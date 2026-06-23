#ifndef STEPPER_PIO_PULSE_ENGINE_H
#define STEPPER_PIO_PULSE_ENGINE_H

#include "pico/stdlib.h"
#include "hardware/pio.h"

class StepperPioPulseEngine {
private:
    static const uint STEP_GROUP_PIN_COUNT = 8;

    PIO pioBlock;
    uint stateMachine;
    uint programOffset;

    uint firstStepPin;

    uint32_t minimumStepIntervalUs;

    bool initialized;

    void loadProgramIfNeeded();

public:
    StepperPioPulseEngine(
        PIO selectedPioBlock,
        uint selectedStateMachine,
        uint firstStepGpio,
        uint32_t minStepIntervalUs = 10
    );

    void init();

    bool isInitialized() const;

    uint getFirstStepPin() const;
    uint getStepGroupPinCount() const;

    bool pinBelongsToStepGroup(uint gpioPin) const;
    uint getPinOffset(uint gpioPin) const;

    uint32_t getMinimumStepIntervalUs() const;
    float getMaxStepEventsPerSecond() const;

    void sendStepMaskBlocking(uint32_t stepMask);
    bool trySendStepMask(uint32_t stepMask);
};

#endif
