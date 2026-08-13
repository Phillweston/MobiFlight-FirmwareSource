//
// LedSegment.cpp
//
// (C) MobiFlight Project 2022
//

#include "commandmessenger.h"
#include "allocateMem.h"
#include "MFSegments.h"
#include "LedSegment.h"
#include "I2CBridge.h"

namespace LedSegment
{
    MFSegments *ledSegments;
    uint8_t     ledSegmentsRegistered  = 0;
    uint8_t     ledSegmentsRegistereds = 0;

    bool setupArray(uint16_t count)
    {
        if (!count) return true;
        ledSegments = static_cast<MFSegments *>(MF_ALLOC_TYPE(MFSegments, count));
        if (!ledSegments) return false;

        ledSegmentsRegistereds = count;
        return true;
    }

    void Add(uint8_t type, uint8_t dataPin, uint8_t csPin, uint8_t clkPin, uint8_t numDevices, uint8_t brightness)
    {
        if (ledSegmentsRegistered == ledSegmentsRegistereds)
            return;

        new (&ledSegments[ledSegmentsRegistered]) MFSegments();

        if (!ledSegments[ledSegmentsRegistered].attach(type, dataPin, csPin, clkPin, numDevices, brightness)) {
            cmdMessenger.sendCmd(kStatus, F("Led Segment array does not fit into Memory"));
            return;
        }

        ledSegmentsRegistered++;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added Led Segment"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < ledSegmentsRegistered; i++) {
            ledSegments[i].detach();
        }
        ledSegmentsRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared segments"));
#endif
    }

    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < ledSegmentsRegistered; ++i) {
            ledSegments[i].powerSavingMode(state);
        }
    }

    void OnInitModule()
    {
        int module     = cmdMessenger.readInt16Arg();
        int subModule  = cmdMessenger.readInt16Arg();
        int brightness = cmdMessenger.readInt16Arg();
        ledSegments[module].setBrightness(subModule, brightness);
        const uint8_t payload[] = {
            static_cast<uint8_t>(module),
            static_cast<uint8_t>(subModule),
            static_cast<uint8_t>(brightness),
        };
        I2CBridge::send(I2CBridge::kSegmentBrightness, payload, sizeof(payload));
    }

    void OnSetModule()
    {
        int     module    = cmdMessenger.readInt16Arg();
        int     subModule = cmdMessenger.readInt16Arg();
        char   *value     = cmdMessenger.readStringArg();
        uint8_t points    = (uint8_t)cmdMessenger.readInt16Arg();
        uint8_t mask      = (uint8_t)cmdMessenger.readInt16Arg();
        ledSegments[module].display(subModule, value, points, mask);
        const uint8_t metadata[] = {
            static_cast<uint8_t>(subModule),
            points,
            mask,
        };
        I2CBridge::sendTextChunks(I2CBridge::kSegmentDisplay, static_cast<uint8_t>(module), metadata, sizeof(metadata), value);
    }

    void OnSetModuleBrightness()
    {
        int module     = cmdMessenger.readInt16Arg();
        int subModule  = cmdMessenger.readInt16Arg();
        int brightness = cmdMessenger.readInt16Arg();
        ledSegments[module].setBrightness(subModule, brightness);
        const uint8_t payload[] = {
            static_cast<uint8_t>(module),
            static_cast<uint8_t>(subModule),
            static_cast<uint8_t>(brightness),
        };
        I2CBridge::send(I2CBridge::kSegmentBrightness, payload, sizeof(payload));
    }

    void OnSetModuleSingleSegment()
    {
        uint8_t module    = (uint8_t)cmdMessenger.readInt16Arg();
        uint8_t subModule = (uint8_t)cmdMessenger.readInt16Arg();
        char   *segment   = cmdMessenger.readStringArg();         // 0 to 63, multiple segments deliminited by '|'
        uint8_t on_off    = (uint8_t)cmdMessenger.readInt16Arg(); // 0 or 1

        char *pinTokens = strtok(segment, "|");
        while (pinTokens != 0) {
            uint8_t num = (uint8_t)atoi(pinTokens);
            ledSegments[module].setSingleSegment(subModule, num, on_off);
            const uint8_t payload[] = {module, subModule, num, on_off};
            I2CBridge::send(I2CBridge::kSegmentSingle, payload, sizeof(payload));
            pinTokens = strtok(0, "|");
        }
    }

} // namespace

// LedSegment.cpp
