//
// I2CBridge.cpp
//

#include <Wire.h>
#include <string.h>
#include "I2CBridge.h"

namespace I2CBridge
{
    namespace
    {
        const uint8_t FRAME_MAGIC       = 0xA5;
        const uint8_t FRAME_VERSION     = 0x02;
        const uint8_t MAX_FRAME_SIZE    = 32;
        const uint8_t FRAME_OVERHEAD    = 5;
        const uint8_t MAX_PAYLOAD_SIZE  = MAX_FRAME_SIZE - FRAME_OVERHEAD;
        const uint8_t TEXT_CHUNK_FIELDS = 4;
    }

    void begin()
    {
        Wire.begin();
        Wire.setClock(400000);
    }

    bool send(EventType type, const uint8_t *payload, uint8_t payloadLength)
    {
        if (payloadLength > MAX_PAYLOAD_SIZE || (payloadLength && !payload))
            return false;

        uint8_t frame[MAX_FRAME_SIZE];
        frame[0] = FRAME_MAGIC;
        frame[1] = FRAME_VERSION;
        frame[2] = static_cast<uint8_t>(type);
        frame[3] = payloadLength;
        if (payloadLength)
            memcpy(&frame[4], payload, payloadLength);

        uint8_t checksum = 0;
        for (uint8_t i = 0; i < payloadLength + 4; ++i)
            checksum ^= frame[i];
        frame[payloadLength + 4] = checksum;

        Wire.beginTransmission(MF_I2C_BRIDGE_ADDRESS);
        Wire.write(frame, payloadLength + FRAME_OVERHEAD);
        return Wire.endTransmission() == 0;
    }

    bool sendBinary(EventType type, uint8_t id, bool state)
    {
        const uint8_t payload[] = {id, static_cast<uint8_t>(state ? 1 : 0)};
        return send(type, payload, sizeof(payload));
    }

    bool sendModuleBinary(EventType type, uint8_t module, uint8_t channel, bool state)
    {
        const uint8_t payload[] = {module, channel, static_cast<uint8_t>(state ? 1 : 0)};
        return send(type, payload, sizeof(payload));
    }

    bool sendAnalog(uint8_t module, uint8_t pin, uint16_t value)
    {
        const uint8_t payload[] = {
            module,
            pin,
            static_cast<uint8_t>(value & 0xFF),
            static_cast<uint8_t>(value >> 8),
        };
        return send(kAnalog, payload, sizeof(payload));
    }

    bool sendTextChunks(EventType type, uint8_t module, const uint8_t *metadata,
                        uint8_t metadataLength, const char *text)
    {
        if (!text || metadataLength + TEXT_CHUNK_FIELDS > MAX_PAYLOAD_SIZE)
            return false;

        const size_t textLength = strlen(text);
        if (textLength > 255)
            return false;

        const uint8_t maxChunk = MAX_PAYLOAD_SIZE - metadataLength - TEXT_CHUNK_FIELDS;
        size_t        offset   = 0;
        bool          success  = true;

        do {
            const size_t  remaining   = textLength - offset;
            const uint8_t chunkLength = static_cast<uint8_t>(remaining < maxChunk ? remaining : maxChunk);
            uint8_t       payload[MAX_PAYLOAD_SIZE];
            payload[0] = module;
            payload[1] = static_cast<uint8_t>(offset);
            payload[2] = static_cast<uint8_t>(textLength);
            payload[3] = chunkLength;
            if (metadataLength)
                memcpy(&payload[TEXT_CHUNK_FIELDS], metadata, metadataLength);
            if (chunkLength)
                memcpy(&payload[TEXT_CHUNK_FIELDS + metadataLength], &text[offset], chunkLength);

            success = send(type, payload, TEXT_CHUNK_FIELDS + metadataLength + chunkLength) && success;
            offset += chunkLength;
        } while (offset < textLength);

        return success;
    }
}
