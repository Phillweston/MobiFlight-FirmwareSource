#include "mobiflight_bridge.h"
#include <algorithm>
#include "esphome/components/api/api_server.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/log.h"

namespace esphome::mobiflight_bridge
{
static const char *const TAG = "mobiflight_bridge";
MobiFlightBridge *MobiFlightBridge::instance_ = nullptr;
void MobiFlightBridge::add_digital_input(uint8_t t, uint8_t m, uint8_t c, bool p, binary_sensor::BinarySensor *s)
{
    inputs_.push_back({t, m, c, p, 0, s});
}
void MobiFlightBridge::add_analog_input(uint8_t m, sensor::Sensor *p, sensor::Sensor *r)
{
    analog_.push_back({m, p, r});
}
void MobiFlightBridge::add_digital_output(uint8_t t, uint8_t m, uint8_t c, binary_sensor::BinarySensor *s)
{
    outputs_.push_back({t, m, c, false, 0, s});
}
void MobiFlightBridge::setup()
{
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextColor(TFT_CYAN);
    M5.Display.setTextSize(3);
    M5.Display.setCursor(48, 70);
    M5.Display.print("A320 OVHD");
    M5.Display.setTextSize(2);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setCursor(68, 112);
    M5.Display.print("ESPHome CoreS3");
    canvas_.setPsram(true);
    if (!canvas_.createSprite(320, 240))
        ESP_LOGW(TAG, "Display back buffer allocation failed; using direct drawing");
    boot_at_ = millis();
    instance_ = this;
    Wire.onReceive(receive_callback_);
    if (!Wire.begin(address_, sda_, scl_, 400000))
    {
        ESP_LOGE(TAG, "I2C slave init failed");
        mark_failed();
        return;
    }
    ESP_LOGI(TAG, "I2C slave 0x%02X ready", address_);
}
void MobiFlightBridge::receive_callback_(int count)
{
    if (!instance_)
        return;
    auto *s = instance_;
    Frame f{};
    while (Wire.available() && f.length < sizeof(f.data))
        f.data[f.length++] = Wire.read();
    while (Wire.available())
        Wire.read();
    if (count <= 0 || count > 32)
    {
        s->dropped_frames_++;
        return;
    }
    portENTER_CRITICAL_ISR(&s->queue_mux_);
    uint8_t n = (s->head_ + 1) % QUEUE_SIZE;
    if (n == s->tail_)
        s->dropped_frames_++;
    else
    {
        s->queue_[s->head_] = f;
        s->head_ = n;
    }
    portEXIT_CRITICAL_ISR(&s->queue_mux_);
}
bool MobiFlightBridge::pop_frame_(Frame &f)
{
    bool ok = false;
    portENTER_CRITICAL(&queue_mux_);
    if (tail_ != head_)
    {
        f = queue_[tail_];
        tail_ = (tail_ + 1) % QUEUE_SIZE;
        ok = true;
    }
    portEXIT_CRITICAL(&queue_mux_);
    return ok;
}
void MobiFlightBridge::process_frame_(const Frame &f)
{
    if (f.length < 5 || f.data[0] != 0xA5 || f.data[1] != 2 || f.length != f.data[3] + 5)
    {
        invalid_frames_++;
        return;
    }
    uint8_t sum = 0;
    for (uint8_t i = 0; i < f.length; i++)
        sum ^= f.data[i];
    if (sum)
    {
        invalid_frames_++;
        return;
    }
    valid_frames_++;
    screen_dirty_ = true;
    uint8_t t = f.data[2], len = f.data[3];
    const uint8_t *p = f.data + 4;
    if ((t == 3 || t == 4) && len == 3)
    {
        for (auto &b : inputs_)
            if (b.type == t && b.module == p[0] && b.channel == p[1])
            {
                bool state = p[2] != 0;
                b.sensor->publish_state(state);
                if (state && b.press_only)
                    b.release_at = millis() + 250;
                return;
            }
    }
    else if (t == 5 && len == 4)
    {
        uint16_t raw = p[2] | (uint16_t(p[3]) << 8);
        for (auto &b : analog_)
            if (b.module == p[0])
            {
                b.raw->publish_state(raw);
                b.percent->publish_state(std::min<uint32_t>(raw, analog_raw_max_) * 100.0f / analog_raw_max_);
                return;
            }
    }
    else if (t == 2 && len == 2)
    {
        for (auto &b : outputs_)
            if (b.type == t && b.channel == p[0])
            {
                b.sensor->publish_state(p[1] != 0);
                return;
            }
    }
    else if (t == 6 && len >= 4 && len == p[2] + 3)
    {
        for (uint8_t by = 0; by < p[2]; by++)
            for (uint8_t bit = 0; bit < 8; bit++)
            {
                if (!(p[3 + by] & (1U << bit)))
                    continue;
                uint8_t c = by * 8 + bit;
                for (auto &b : outputs_)
                    if (b.type == t && b.module == p[0] && b.channel == c)
                        b.sensor->publish_state(p[1] != 0);
            }
    }
    else if (t == 7 && len >= 7 && p[0] == segment_module_ && p[3] + 7 == len &&
             p[1] + p[3] <= sizeof(segment_chunk_) - 1 && p[2] < sizeof(segment_chunk_))
    {
        if (p[1] == 0)
        {
            segment_total_ = p[2];
            segment_submodule_ = p[4];
            segment_points_ = p[5];
            segment_mask_ = p[6];
            memset(segment_chunk_, 0, sizeof(segment_chunk_));
        }
        if (p[2] != segment_total_ || p[4] != segment_submodule_ || p[5] != segment_points_ || p[6] != segment_mask_)
        {
            invalid_frames_++;
            return;
        }
        memcpy(segment_chunk_ + p[1], p + 7, p[3]);
        if (p[1] + p[3] == segment_total_)
        {
            uint8_t pos = 0;
            for (int8_t digit = 7; digit >= 0; digit--)
                if (segment_mask_ & (1U << digit))
                    segment_digits_[digit] = pos < segment_total_ ? segment_chunk_[pos++] : ' ';
            char bat1[4] = {segment_digits_[5], segment_digits_[4], segment_digits_[3], 0};
            char bat2[4] = {segment_digits_[2], segment_digits_[1], segment_digits_[0], 0};
            if (bat1_)
                bat1_->publish_state(bat1);
            if (bat2_)
                bat2_->publish_state(bat2);
        }
    }
}
void MobiFlightBridge::handle_touch_()
{
    M5.update();
    auto d = M5.Touch.getDetail();
    if (d.wasPressed())
    {
        if (d.y >= 208)
        {
            page_ = std::min<uint8_t>(4, d.x / 64);
            scroll_ = 0;
            screen_dirty_ = true;
        }
        else
            touch_y_ = d.y;
    }
    if (d.wasReleased() && touch_y_ >= 0)
    {
        int delta = d.y - touch_y_;
        if (abs(delta) >= 18)
        {
            int next = int(scroll_) + (delta < 0 ? 3 : -3);
            scroll_ = std::max(0, next);
            screen_dirty_ = true;
        }
        touch_y_ = -1;
    }
}
void MobiFlightBridge::draw_screen_()
{
    auto &display =
        canvas_.getBuffer() ? static_cast<lgfx::LGFXBase &>(canvas_) : static_cast<lgfx::LGFXBase &>(M5.Display);
    display.fillScreen(0x0841);
    display.setTextWrap(false);
    display.setTextSize(1);
    display.setTextColor(TFT_WHITE);
    display.setCursor(8, 7);
    display.print("A320 OVHD  ESPHome");
    display.setCursor(190, 7);
    display.printf("I2C %s", valid_frames_ ? "RX" : "WAIT");
    const char *tabs[] = {"STATUS", "INPUT", "ANALOG", "OUTPUT", "SETUP"};
    for (uint8_t i = 0; i < 5; i++)
    {
        uint16_t c = i == page_ ? 0x2C8B : 0x18C3;
        display.fillRect(i * 64, 208, 63, 32, c);
        display.setTextColor(TFT_WHITE);
        display.setCursor(i * 64 + 5, 220);
        display.print(tabs[i]);
    }
    display.setTextSize(2);
    display.setTextColor(TFT_CYAN);
    display.setCursor(8, 30);
    display.print(tabs[page_]);
    display.setTextSize(1);
    int y = 58;
    if (page_ == 0)
    {
        auto *wifi_component = wifi::global_wifi_component;
        const bool wifi_connected = wifi_component && wifi_component->is_connected();
        char ssid[wifi::SSID_BUFFER_SIZE]{};
        char ip[network::IP_ADDRESS_BUFFER_SIZE]{};
        if (wifi_connected)
        {
            wifi_component->wifi_ssid_to(ssid);
            wifi_component->wifi_sta_ip_addresses()[0].str_to(ip);
        }
        display.setTextColor(TFT_WHITE);
        display.setCursor(10, y);
        display.printf("Wi-Fi: %s", wifi_connected ? ssid : "disconnected");
        y += 22;
        display.setCursor(10, y);
        display.printf("IP: %s", wifi_connected ? ip : "--");
        y += 22;
        display.setCursor(10, y);
        display.printf("Home Assistant API: %s",
                       api::global_api_server && api::global_api_server->is_connected() ? "connected" : "waiting");
        y += 22;
        display.setCursor(10, y);
        display.printf("I2C frames: %lu valid / %lu bad / %lu dropped", static_cast<unsigned long>(valid_frames_),
                       static_cast<unsigned long>(invalid_frames_), static_cast<unsigned long>(dropped_frames_));
    }
    else if (page_ == 1)
    {
        for (size_t i = scroll_; i < inputs_.size() && y < 202; i++, y += 17)
        {
            auto &b = inputs_[i];
            display.setTextColor(b.sensor->has_state() ? (b.sensor->state ? TFT_GREEN : 0x7BEF) : 0x7BEF);
            display.setCursor(8, y);
            display.printf("%-28.28s %s", b.sensor->get_name().c_str(),
                           b.sensor->has_state() ? (b.sensor->state ? "ON" : "OFF") : "--");
        }
    }
    else if (page_ == 2)
    {
        for (size_t i = scroll_; i < analog_.size() && y < 202; i++, y += 24)
        {
            auto &b = analog_[i];
            display.setTextColor(TFT_WHITE);
            display.setCursor(8, y);
            display.printf("%-24.24s %5.1f%%", b.percent->get_name().c_str(),
                           b.percent->has_state() ? b.percent->state : 0);
        }
    }
    else if (page_ == 3)
    {
        size_t count = outputs_.size() + 2;
        for (size_t i = scroll_; i < count && y < 202; i++, y += 17)
        {
            display.setCursor(8, y);
            if (i < outputs_.size())
            {
                auto &b = outputs_[i];
                display.setTextColor(b.sensor->has_state() ? (b.sensor->state ? TFT_GREEN : 0x7BEF) : 0x7BEF);
                display.printf("%-28.28s %s", b.sensor->get_name().c_str(),
                               b.sensor->has_state() ? (b.sensor->state ? "ON" : "OFF") : "--");
            }
            else
            {
                auto *s = i == outputs_.size() ? bat1_ : bat2_;
                display.setTextColor(TFT_CYAN);
                display.printf("%-28.28s %s", s->get_name().c_str(), s->has_state() ? s->state.c_str() : "--");
            }
        }
    }
    else
    {
        auto *wifi_component = wifi::global_wifi_component;
        const bool wifi_connected = wifi_component && wifi_component->is_connected();
        char ssid[wifi::SSID_BUFFER_SIZE]{};
        char ip[network::IP_ADDRESS_BUFFER_SIZE]{};
        if (wifi_connected)
        {
            wifi_component->wifi_ssid_to(ssid);
            wifi_component->wifi_sta_ip_addresses()[0].str_to(ip);
        }
        display.setTextColor(TFT_WHITE);
        display.setCursor(10, y);
        if (wifi_connected)
        {
            display.setTextColor(TFT_GREEN);
            display.print("Network: connected");
            y += 24;
            display.setTextColor(TFT_WHITE);
            display.setCursor(10, y);
            display.printf("Wi-Fi: %s", ssid);
            y += 24;
            display.setCursor(10, y);
            display.printf("IP: %s", ip);
            y += 24;
            display.setCursor(10, y);
            display.printf("Web: http://%s", ip);
            y += 24;
            display.setCursor(10, y);
            display.print("Host: a320-ovhd-cores3.local");
        }
        else
        {
            display.print("Provision: captive portal / Improv Serial");
            y += 24;
            display.setCursor(10, y);
            display.print("AP: A320 OVHD Setup");
            y += 24;
            display.setCursor(10, y);
            display.printf("Password: %s", setup_ap_password_.c_str());
            y += 24;
            display.setCursor(10, y);
            display.print("Setup: http://192.168.4.1");
        }
    }
    if (canvas_.getBuffer())
        canvas_.pushSprite(0, 0);
}
void MobiFlightBridge::loop()
{
    Frame f;
    while (pop_frame_(f))
        process_frame_(f);
    uint32_t now = millis();
    for (auto &b : inputs_)
        if (b.release_at && int32_t(now - b.release_at) >= 0)
        {
            b.release_at = 0;
            b.sensor->publish_state(false);
            screen_dirty_ = true;
        }
    handle_touch_();
    if (now - last_status_check_ >= 500)
    {
        last_status_check_ = now;
        auto *wifi_component = wifi::global_wifi_component;
        const bool wifi_connected = wifi_component && wifi_component->is_connected();
        const bool api_connected = api::global_api_server && api::global_api_server->is_connected();
        const network::IPAddress ip =
            wifi_connected ? wifi_component->wifi_sta_ip_addresses()[0] : network::IPAddress();
        if (wifi_connected != previous_wifi_connected_ || api_connected != previous_api_connected_ ||
            ip != previous_ip_)
        {
            previous_wifi_connected_ = wifi_connected;
            previous_api_connected_ = api_connected;
            previous_ip_ = ip;
            screen_dirty_ = true;
        }
    }
    if (now - boot_at_ >= 1500 && screen_dirty_)
    {
        screen_dirty_ = false;
        draw_screen_();
    }
}
void MobiFlightBridge::dump_config()
{
    ESP_LOGCONFIG(TAG, "MobiFlight bridge 0x%02X SDA%u/SCL%u: %u inputs, %u analog, %u outputs; frames %u/%u/%u",
                  address_, sda_, scl_, inputs_.size(), analog_.size(), outputs_.size(), valid_frames_, invalid_frames_,
                  dropped_frames_);
}
} // namespace esphome::mobiflight_bridge
