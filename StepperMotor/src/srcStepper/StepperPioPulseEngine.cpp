#include "StepperPioPulseEngine.h"
#include "stepper.pio.h"

static bool stepperProgramLoadedPio0 = false;
static bool stepperProgramLoadedPio1 = false;

static uint stepperProgramOffsetPio0 = 0;
static uint stepperProgramOffsetPio1 = 0;

StepperPioPulseEngine::StepperPioPulseEngine(
    PIO selectedPioBlock,
    uint selectedStateMachine,
    uint firstStepGpio,
    uint32_t minStepIntervalUs
) {
    pioBlock = selectedPioBlock;
    stateMachine = selectedStateMachine;

    firstStepPin = firstStepGpio;

    minimumStepIntervalUs = minStepIntervalUs;

    if (minimumStepIntervalUs < 6) {
        minimumStepIntervalUs = 6;
    }

    programOffset = 0;
    initialized = false;
}

void StepperPioPulseEngine::loadProgramIfNeeded() {
    if (pioBlock == pio0) {
        if (!stepperProgramLoadedPio0) {
            stepperProgramOffsetPio0 =
                pio_add_program(pioBlock, &stepper_multi_program);

            stepperProgramLoadedPio0 = true;
        }

        programOffset = stepperProgramOffsetPio0;
    } else {
        if (!stepperProgramLoadedPio1) {
            stepperProgramOffsetPio1 =
                pio_add_program(pioBlock, &stepper_multi_program);

            stepperProgramLoadedPio1 = true;
        }

        programOffset = stepperProgramOffsetPio1;
    }
}

void StepperPioPulseEngine::init() {
    loadProgramIfNeeded();

    stepper_multi_program_init(
        pioBlock,
        stateMachine,
        programOffset,
        firstStepPin,
        1000000.0f
    );

    initialized = true;
}

bool StepperPioPulseEngine::isInitialized() const {
    return initialized;
}

uint StepperPioPulseEngine::getFirstStepPin() const {
    return firstStepPin;
}

uint StepperPioPulseEngine::getStepGroupPinCount() const {
    return STEP_GROUP_PIN_COUNT;
}

bool StepperPioPulseEngine::pinBelongsToStepGroup(uint gpioPin) const {
    if (gpioPin < firstStepPin) {
        return false;
    }

    if (gpioPin >= (firstStepPin + STEP_GROUP_PIN_COUNT)) {
        return false;
    }

    return true;
}

uint StepperPioPulseEngine::getPinOffset(uint gpioPin) const {
    if (!pinBelongsToStepGroup(gpioPin)) {
        return 0;
    }

    return gpioPin - firstStepPin;
}

uint32_t StepperPioPulseEngine::getMinimumStepIntervalUs() const {
    return minimumStepIntervalUs;
}

float StepperPioPulseEngine::getMaxStepEventsPerSecond() const {
    return 1000000.0f / (float)minimumStepIntervalUs;
}

void StepperPioPulseEngine::sendStepMaskBlocking(uint32_t stepMask) {
    if (!initialized) {
        return;
    }

    stepMask = stepMask & 0xFFu;

    if (stepMask == 0) {
        return;
    }

    pio_sm_put_blocking(
        pioBlock,
        stateMachine,
        stepMask
    );
}

bool StepperPioPulseEngine::trySendStepMask(uint32_t stepMask) {
    if (!initialized) {
        return false;
    }

    stepMask = stepMask & 0xFFu;

    if (stepMask == 0) {
        return true;
    }

    if (pio_sm_is_tx_fifo_full(pioBlock, stateMachine)) {
        return false;
    }

    pio_sm_put(
        pioBlock,
        stateMachine,
        stepMask
    );

    return true;
}
