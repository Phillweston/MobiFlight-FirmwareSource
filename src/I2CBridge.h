//
// I2CBridge.h
//

#pragma once

#include <Arduino.h>

#ifndef MF_I2C_BRIDGE_ADDRESS
#define MF_I2C_BRIDGE_ADDRESS 0x42
#endif

namespace I2CBridge
{
    enum EventType : uint8_t {
        kButton           = 0x01,
        kOutput           = 0x02,
        kInputShifter     = 0x03,
        kInputMux         = 0x04,
        kAnalog           = 0x05,
        kOutputShifter    = 0x06,
        kSegmentDisplay   = 0x07,
        kSegmentBrightness = 0x08,
        kSegmentSingle    = 0x09,
        kLcdText          = 0x0A,
    };

    void begin();
    bool send(EventType type, const uint8_t *payload, uint8_t payloadLength);
    bool sendBinary(EventType type, uint8_t id, bool state);
    bool sendModuleBinary(EventType type, uint8_t module, uint8_t channel, bool state);
    bool sendAnalog(uint8_t module, uint8_t pin, uint16_t value);
    bool sendTextChunks(EventType type, uint8_t module, const uint8_t *metadata,
                        uint8_t metadataLength, const char *text);
}
