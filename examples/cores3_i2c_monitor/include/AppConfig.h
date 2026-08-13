#pragma once

#include <Arduino.h>

// CoreS3 external Port A defaults. Verify these pins against the device label.
constexpr int I2C_SLAVE_SDA_PIN = 2;
constexpr int I2C_SLAVE_SCL_PIN = 1;
constexpr uint8_t I2C_SLAVE_ADDRESS = 0x42;
constexpr uint32_t I2C_SLAVE_FREQUENCY = 400000;

constexpr uint16_t ANALOG_RAW_MAX = 1023;
constexpr uint32_t LINK_STALE_MS = 5000;

// Set these values before uploading. Keep real credentials out of source control.
constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
constexpr char MQTT_HOST[] = "192.168.1.10";
constexpr uint16_t MQTT_PORT = 1883;
constexpr char MQTT_USERNAME[] = "homeassistant";
constexpr char MQTT_PASSWORD[] = "YOUR_MQTT_PASSWORD";

constexpr char MQTT_CLIENT_ID[] = "m5stack-cores3-a320-ovhd";
constexpr char MQTT_BASE_TOPIC[] = "mobiflight/a320_ovhd";
constexpr char HOME_ASSISTANT_DISCOVERY_PREFIX[] = "homeassistant";
constexpr uint32_t NETWORK_RECONNECT_MS = 5000;
