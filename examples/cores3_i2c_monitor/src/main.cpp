#include <Arduino.h>
#include <M5Unified.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <WiFi.h>
#include "AppConfig.h"

namespace {
constexpr uint8_t FRAME_MAGIC = 0xA5;
constexpr uint8_t FRAME_VERSION = 0x02;
constexpr uint8_t EVENT_ANALOG = 0x05;
constexpr size_t MAX_FRAME_SIZE = 32;
constexpr size_t RX_QUEUE_SIZE = 16;
constexpr size_t ANALOG_COUNT = 6;
constexpr size_t MQTT_QUEUE_SIZE = 32;
constexpr size_t MQTT_PAYLOAD_SIZE = 224;

struct RxFrame {
    uint8_t length;
    uint8_t data[MAX_FRAME_SIZE];
};

struct AnalogState {
    const char *name;
    const char *objectId;
    uint16_t value;
    uint8_t arduinoPin;
    uint32_t updatedAt;
    bool valid;
};

struct MqttMessage {
    char topic[96];
    char payload[MQTT_PAYLOAD_SIZE];
    bool retain;
};

RxFrame rxQueue[RX_QUEUE_SIZE];
volatile uint8_t rxHead = 0;
volatile uint8_t rxTail = 0;
portMUX_TYPE rxMux = portMUX_INITIALIZER_UNLOCKED;

AnalogState analogStates[ANALOG_COUNT] = {
    {"OVHD LIGHT", "ovhd_light", 0, 0, 0, false},
    {"LDG ELEV", "ldg_elev", 0, 0, 0, false},
    {"COCKPIT TEMP", "cockpit_temp", 0, 0, 0, false},
    {"FWD CABIN", "fwd_cabin", 0, 0, 0, false},
    {"AFT CABIN", "aft_cabin", 0, 0, 0, false},
    {"CARGO AFT", "cargo_aft", 0, 0, 0, false},
};

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);
MqttMessage mqttQueue[MQTT_QUEUE_SIZE];
uint8_t mqttHead = 0;
uint8_t mqttTail = 0;

volatile uint32_t rxFrames = 0;
volatile uint32_t rxDrops = 0;
uint32_t invalidFrames = 0;
uint32_t unknownFrames = 0;
uint32_t mqttDrops = 0;
uint32_t lastValidFrameAt = 0;
bool screenDirty = true;

uint16_t percentFor(uint16_t value);

const char *eventName(uint8_t type)
{
    switch (type) {
    case 0x01: return "button";
    case 0x02: return "output";
    case 0x03: return "input_shifter";
    case 0x04: return "input_mux";
    case 0x05: return "analog";
    case 0x06: return "output_shifter";
    case 0x07: return "segment_display";
    case 0x08: return "segment_brightness";
    case 0x09: return "segment_single";
    case 0x0A: return "lcd_text";
    default: return "unknown";
    }
}

bool enqueueMqtt(const char *topic, const char *payload, bool retain)
{
    const uint8_t next = (mqttHead + 1) % MQTT_QUEUE_SIZE;
    if (next == mqttTail) {
        ++mqttDrops;
        screenDirty = true;
        return false;
    }

    MqttMessage &message = mqttQueue[mqttHead];
    strlcpy(message.topic, topic, sizeof(message.topic));
    strlcpy(message.payload, payload, sizeof(message.payload));
    message.retain = retain;
    mqttHead = next;
    return true;
}

void enqueueFrameEvent(const RxFrame &frame)
{
    char hexPayload[MAX_FRAME_SIZE * 2 + 1];
    hexPayload[0] = '\0';
    for (uint8_t i = 0; i < frame.data[3]; ++i)
        snprintf(&hexPayload[i * 2], 3, "%02X", frame.data[4 + i]);

    char payload[MQTT_PAYLOAD_SIZE];
    snprintf(payload, sizeof(payload),
             "{\"type\":\"%s\",\"type_id\":%u,\"length\":%u,\"payload_hex\":\"%s\"}",
             eventName(frame.data[2]), frame.data[2], frame.data[3], hexPayload);

    char topic[96];
    snprintf(topic, sizeof(topic), "%s/event", MQTT_BASE_TOPIC);
    enqueueMqtt(topic, payload, false);
}

void enqueueAnalogState(uint8_t index)
{
    const AnalogState &state = analogStates[index];
    char topic[96];
    char payload[48];

    snprintf(topic, sizeof(topic), "%s/analog/%s/percent", MQTT_BASE_TOPIC, state.objectId);
    snprintf(payload, sizeof(payload), "%u", percentFor(state.value));
    enqueueMqtt(topic, payload, true);

    snprintf(topic, sizeof(topic), "%s/analog/%s/raw", MQTT_BASE_TOPIC, state.objectId);
    snprintf(payload, sizeof(payload), "%u", state.value);
    enqueueMqtt(topic, payload, true);
}

void publishDiscoverySensor(size_t index, bool raw)
{
    const AnalogState &state = analogStates[index];
    char topic[128];
    char stateTopic[96];
    char payload[512];
    const char *suffix = raw ? "raw" : "percent";
    const char *unit = raw ? "" : "%";

    snprintf(topic, sizeof(topic), "%s/sensor/%s/%s_%s/config",
             HOME_ASSISTANT_DISCOVERY_PREFIX, MQTT_CLIENT_ID, state.objectId, suffix);
    snprintf(stateTopic, sizeof(stateTopic), "%s/analog/%s/%s",
             MQTT_BASE_TOPIC, state.objectId, suffix);
    snprintf(payload, sizeof(payload),
             "{\"name\":\"%s %s\",\"unique_id\":\"%s_%s_%s\","
             "\"state_topic\":\"%s\",\"availability_topic\":\"%s/status\","
             "\"payload_available\":\"online\",\"payload_not_available\":\"offline\","
             "\"unit_of_measurement\":\"%s\",\"state_class\":\"measurement\","
             "\"device\":{\"identifiers\":[\"%s\"],\"name\":\"A320 OVHD CoreS3\","
             "\"manufacturer\":\"M5Stack\",\"model\":\"CoreS3 SE\"}}",
             state.name, raw ? "Raw" : "Position", MQTT_CLIENT_ID, state.objectId,
             suffix, stateTopic, MQTT_BASE_TOPIC, unit, MQTT_CLIENT_ID);
    mqttClient.publish(topic, payload, true);
}

void publishDiscovery()
{
    for (size_t i = 0; i < ANALOG_COUNT; ++i) {
        publishDiscoverySensor(i, false);
        publishDiscoverySensor(i, true);
    }
}

void maintainNetwork()
{
    static uint32_t lastWifiAttempt = 0;
    static uint32_t lastMqttAttempt = 0;

    if (WiFi.status() != WL_CONNECTED) {
        if (millis() - lastWifiAttempt >= NETWORK_RECONNECT_MS) {
            lastWifiAttempt = millis();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        return;
    }

    if (!mqttClient.connected()) {
        if (millis() - lastMqttAttempt < NETWORK_RECONNECT_MS)
            return;
        lastMqttAttempt = millis();

        char statusTopic[96];
        snprintf(statusTopic, sizeof(statusTopic), "%s/status", MQTT_BASE_TOPIC);
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD,
                               statusTopic, 1, true, "offline")) {
            mqttClient.publish(statusTopic, "online", true);
            publishDiscovery();
            for (size_t i = 0; i < ANALOG_COUNT; ++i)
                if (analogStates[i].valid)
                    enqueueAnalogState(i);
            screenDirty = true;
        }
        return;
    }

    mqttClient.loop();
    uint8_t publishBudget = 4;
    while (mqttTail != mqttHead && publishBudget--) {
        MqttMessage &message = mqttQueue[mqttTail];
        if (!mqttClient.publish(message.topic, message.payload, message.retain))
            break;
        mqttTail = (mqttTail + 1) % MQTT_QUEUE_SIZE;
    }
}

void drainWire()
{
    while (Wire1.available())
        Wire1.read();
}

void onI2cReceive(int byteCount)
{
    if (byteCount <= 0 || byteCount > static_cast<int>(MAX_FRAME_SIZE)) {
        drainWire();
        ++rxDrops;
        return;
    }

    RxFrame incoming;
    incoming.length = 0;
    while (Wire1.available() && incoming.length < MAX_FRAME_SIZE)
        incoming.data[incoming.length++] = static_cast<uint8_t>(Wire1.read());

    if (incoming.length != byteCount) {
        ++rxDrops;
        return;
    }

    portENTER_CRITICAL_ISR(&rxMux);
    const uint8_t next = (rxHead + 1) % RX_QUEUE_SIZE;
    if (next == rxTail) {
        portEXIT_CRITICAL_ISR(&rxMux);
        ++rxDrops;
        return;
    }

    rxQueue[rxHead] = incoming;
    rxHead = next;
    ++rxFrames;
    portEXIT_CRITICAL_ISR(&rxMux);
}

bool popFrame(RxFrame &frame)
{
    bool available = false;
    portENTER_CRITICAL(&rxMux);
    if (rxTail != rxHead) {
        frame = rxQueue[rxTail];
        rxTail = (rxTail + 1) % RX_QUEUE_SIZE;
        available = true;
    }
    portEXIT_CRITICAL(&rxMux);
    return available;
}

bool validateFrame(const RxFrame &frame)
{
    if (frame.length < 5 || frame.data[0] != FRAME_MAGIC ||
        frame.data[1] != FRAME_VERSION ||
        frame.length != static_cast<uint8_t>(frame.data[3] + 5))
        return false;

    uint8_t checksum = 0;
    for (uint8_t i = 0; i < frame.length; ++i)
        checksum ^= frame.data[i];
    return checksum == 0;
}

void processFrame(const RxFrame &frame)
{
    if (!validateFrame(frame)) {
        ++invalidFrames;
        screenDirty = true;
        return;
    }

    lastValidFrameAt = millis();
    const uint8_t type = frame.data[2];
    const uint8_t length = frame.data[3];
    const uint8_t *payload = &frame.data[4];

    enqueueFrameEvent(frame);

    if (type != EVENT_ANALOG) {
        ++unknownFrames;
        return;
    }

    if (length != 4 || payload[0] >= ANALOG_COUNT) {
        ++invalidFrames;
        screenDirty = true;
        return;
    }

    AnalogState &state = analogStates[payload[0]];
    state.arduinoPin = payload[1];
    state.value = payload[2] | (static_cast<uint16_t>(payload[3]) << 8);
    state.updatedAt = millis();
    state.valid = true;
    enqueueAnalogState(payload[0]);
    screenDirty = true;
}

uint16_t percentFor(uint16_t value)
{
    const uint32_t bounded = value > ANALOG_RAW_MAX ? ANALOG_RAW_MAX : value;
    return static_cast<uint16_t>((bounded * 100U + ANALOG_RAW_MAX / 2U) / ANALOG_RAW_MAX);
}

void drawHeader(bool linkOnline)
{
    M5.Display.fillRect(0, 0, 320, 35, linkOnline ? 0x0320 : 0x3000);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 9);
    M5.Display.print("A320 OVHD KNOBS");
    M5.Display.setTextColor(linkOnline ? TFT_GREEN : TFT_RED);
    M5.Display.setCursor(224, 9);
    M5.Display.print(linkOnline ? "I2C" : "---");
    M5.Display.setTextColor(mqttClient.connected() ? TFT_GREEN : TFT_RED);
    M5.Display.setCursor(270, 9);
    M5.Display.print(mqttClient.connected() ? "MQ" : "--");
}

void drawKnob(size_t index)
{
    const AnalogState &state = analogStates[index];
    const int column = index % 2;
    const int row = index / 2;
    const int x = column * 160;
    const int y = 39 + row * 58;
    const int width = 156;

    M5.Display.fillRect(x + 2, y, width - 4, 54, TFT_BLACK);
    M5.Display.drawRect(x + 2, y, width - 4, 54, state.valid ? 0x7BEF : 0x3186);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(state.valid ? TFT_WHITE : 0x7BEF);
    M5.Display.setCursor(x + 9, y + 7);
    M5.Display.print(state.name);

    M5.Display.setTextSize(2);
    M5.Display.setCursor(x + 9, y + 22);
    if (state.valid) {
        M5.Display.printf("%3u%%", percentFor(state.value));
        M5.Display.setTextSize(1);
        M5.Display.setTextColor(0xBDF7);
        M5.Display.setCursor(x + 78, y + 27);
        M5.Display.printf("RAW %u", state.value);
    } else {
        M5.Display.print("--");
    }
}

void drawFooter()
{
    M5.Display.fillRect(0, 216, 320, 24, 0x1082);
    M5.Display.setTextSize(1);
    M5.Display.setTextColor(0xBDF7);
    M5.Display.setCursor(7, 223);
    M5.Display.printf("RX:%lu DROP:%lu BAD:%lu MQD:%lu",
                      static_cast<unsigned long>(rxFrames),
                      static_cast<unsigned long>(rxDrops),
                      static_cast<unsigned long>(invalidFrames),
                      static_cast<unsigned long>(mqttDrops));
}

void drawScreen()
{
    const bool online = lastValidFrameAt != 0 && millis() - lastValidFrameAt <= LINK_STALE_MS;
    drawHeader(online);
    for (size_t i = 0; i < ANALOG_COUNT; ++i)
        drawKnob(i);
    drawFooter();
    screenDirty = false;
}
} // namespace

void setup()
{
    auto config = M5.config();
    M5.begin(config);
    M5.Display.setRotation(1);
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setTextWrap(false);

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    mqttClient.setBufferSize(768);
    Wire1.onReceive(onI2cReceive);
    if (!Wire1.begin(I2C_SLAVE_ADDRESS, I2C_SLAVE_SDA_PIN,
                     I2C_SLAVE_SCL_PIN, I2C_SLAVE_FREQUENCY)) {
        M5.Display.setTextColor(TFT_RED);
        M5.Display.setTextSize(2);
        M5.Display.setCursor(12, 90);
        M5.Display.println("I2C SLAVE INIT FAILED");
        while (true)
            delay(1000);
    }

    drawScreen();
}

void loop()
{
    M5.update();

    RxFrame frame;
    while (popFrame(frame))
        processFrame(frame);

    maintainNetwork();

    static uint32_t lastRefresh = 0;
    if (screenDirty || millis() - lastRefresh >= 500) {
        drawScreen();
        lastRefresh = millis();
    }

    delay(2);
}
