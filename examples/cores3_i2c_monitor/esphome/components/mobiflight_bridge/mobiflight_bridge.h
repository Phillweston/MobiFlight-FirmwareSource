#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <M5Unified.h>
#include <vector>
#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/network/ip_address.h"
#include "esphome/core/component.h"

namespace esphome::mobiflight_bridge
{
class MobiFlightBridge : public Component
{
  public:
    void set_i2c(uint8_t sda, uint8_t scl, uint8_t address)
    {
        sda_ = sda;
        scl_ = scl;
        address_ = address;
    }
    void set_analog_raw_max(uint16_t value) { analog_raw_max_ = value; }
    void set_segment_module(uint8_t value) { segment_module_ = value; }
    void set_setup_ap_password(const std::string &value) { setup_ap_password_ = value; }
    void set_bat_displays(text_sensor::TextSensor *bat1, text_sensor::TextSensor *bat2)
    {
        bat1_ = bat1;
        bat2_ = bat2;
    }
    void add_digital_input(uint8_t, uint8_t, uint8_t, bool, binary_sensor::BinarySensor *);
    void add_analog_input(uint8_t, sensor::Sensor *, sensor::Sensor *);
    void add_digital_output(uint8_t, uint8_t, uint8_t, binary_sensor::BinarySensor *);
    void setup() override;
    void loop() override;
    void dump_config() override;
    float get_setup_priority() const override { return setup_priority::HARDWARE; }

  protected:
    struct DigitalBinding
    {
        uint8_t type, module, channel;
        bool press_only;
        uint32_t release_at;
        binary_sensor::BinarySensor *sensor;
    };
    struct AnalogBinding
    {
        uint8_t module;
        sensor::Sensor *percent, *raw;
    };
    struct Frame
    {
        uint8_t length;
        uint8_t data[32];
    };
    static void receive_callback_(int);
    void process_frame_(const Frame &);
    bool pop_frame_(Frame &);
    void draw_screen_();
    void handle_touch_();
    static MobiFlightBridge *instance_;
    static constexpr uint8_t QUEUE_SIZE = 16;
    Frame queue_[QUEUE_SIZE]{};
    volatile uint8_t head_{0}, tail_{0};
    portMUX_TYPE queue_mux_ = portMUX_INITIALIZER_UNLOCKED;
    std::vector<DigitalBinding> inputs_, outputs_;
    std::vector<AnalogBinding> analog_;
    uint8_t sda_{2}, scl_{1}, address_{0x42};
    uint16_t analog_raw_max_{1023};
    uint8_t segment_module_{0};
    text_sensor::TextSensor *bat1_{nullptr}, *bat2_{nullptr};
    char segment_digits_[9]{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', '\0'};
    char segment_chunk_[32]{};
    uint8_t segment_total_{0}, segment_submodule_{0}, segment_points_{0}, segment_mask_{0};
    uint32_t valid_frames_{0}, invalid_frames_{0}, dropped_frames_{0};
    M5Canvas canvas_{&M5.Display};
    std::string setup_ap_password_;
    uint32_t boot_at_{0}, last_status_check_{0};
    bool screen_dirty_{true};
    bool previous_wifi_connected_{false}, previous_api_connected_{false};
    network::IPAddress previous_ip_;
    uint16_t scroll_{0};
    int16_t touch_y_{-1};
    uint8_t page_{0};
};
} // namespace esphome::mobiflight_bridge
