//
// OutputShifter.cpp
//
// (C) MobiFlight Project 2022
//

#include "commandmessenger.h"
#include <string.h>
#include "allocateMem.h"
#include "MFOutputShifter.h"
#include "OutputShifter.h"
#include "I2CBridge.h"

namespace OutputShifter
{
    MFOutputShifter *outputShifter;
    uint8_t          outputShifterRegistered = 0;
    uint8_t          maxOutputShifter        = 0;

    bool setupArray(uint16_t count)
    {
        if (!count) return true;
        outputShifter = static_cast<MFOutputShifter *>(MF_ALLOC_TYPE(MFOutputShifter, count));
        if (!outputShifter) return false;

        maxOutputShifter = count;
        return true;
    }

    void Add(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin, uint8_t modules)
    {
        if (outputShifterRegistered == maxOutputShifter)
            return;

        new (&outputShifter[outputShifterRegistered]) MFOutputShifter();
        if (!outputShifter[outputShifterRegistered].attach(latchPin, clockPin, dataPin, modules)) {
            cmdMessenger.sendCmd(kStatus, F("OutputShifter array does not fit into Memory"));
            return;
        }
        outputShifterRegistered++;

#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Added Output Shifter"));
#endif
    }

    void Clear()
    {
        for (uint8_t i = 0; i < outputShifterRegistered; i++) {
            outputShifter[i].detach();
        }

        outputShifterRegistered = 0;
#ifdef DEBUG2CMDMESSENGER
        cmdMessenger.sendCmd(kDebug, F("Cleared Output Shifter"));
#endif
    }

    void OnSet()
    {
        int module               = cmdMessenger.readInt16Arg();
        int number_of_submodules = cmdMessenger.readInt16Arg();
        int value                = cmdMessenger.readInt16Arg();

        if (number_of_submodules <= 0 || number_of_submodules > 24)
            return;

        uint8_t _pins[number_of_submodules] = {0};

        for (uint8_t i = number_of_submodules; i != 0; i--) {
            _pins[i-1] = (uint8_t)cmdMessenger.readInt16Arg();
        }

        if (module < 0 || module >= outputShifterRegistered)
            return;

        outputShifter[module].setPins(_pins, value);

        uint8_t payload[27];
        payload[0] = static_cast<uint8_t>(module);
        payload[1] = static_cast<uint8_t>(value);
        payload[2] = static_cast<uint8_t>(number_of_submodules);
        memcpy(&payload[3], _pins, number_of_submodules);
        I2CBridge::send(I2CBridge::kOutputShifter, payload, number_of_submodules + 3);
    }

    void PowerSave(bool state)
    {
        for (uint8_t i = 0; i < outputShifterRegistered; ++i) {
            outputShifter[i].powerSavingMode(state);
        }
    }
} // namespace

// OutputShifter.cpp
