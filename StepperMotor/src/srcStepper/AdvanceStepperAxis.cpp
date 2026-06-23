#include "AdvanceStepperAxis.h"

#include <math.h>

AdvanceStepperAxis::AdvanceStepperAxis(const StepperAxisConfig& axisConfig) {
    config = axisConfig;

    stepsPerMm = 0.0f;

    currentPositionSteps = 0;
    targetPositionSteps = 0;

    enabled = false;
    currentDirection = true;
    homed = false;

    simulatedHomeActive = false;

    calculateStepsPerMm();
}

void AdvanceStepperAxis::calculateStepsPerMm() {
    if (config.travelPerRevolutionMm <= 0.0f) {
        stepsPerMm = 0.0f;
        return;
    }

    if (config.gearRatio <= 0.0f) {
        stepsPerMm = 0.0f;
        return;
    }

    stepsPerMm =
        ((float)config.motorStepsPerRev *
         (float)config.microsteps *
         config.gearRatio) /
        config.travelPerRevolutionMm;
}

void AdvanceStepperAxis::configureInputPin(
    uint pin,
    bool usePullup
) {
    if (pin == StepperAxisConfig::PIN_NOT_USED) {
        return;
    }

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);

    if (usePullup) {
        gpio_pull_up(pin);
    } else {
        gpio_disable_pulls(pin);
    }
}

bool AdvanceStepperAxis::readInputActive(
    uint pin,
    bool activeLow
) const {
    if (pin == StepperAxisConfig::PIN_NOT_USED) {
        return false;
    }

    bool rawState = gpio_get(pin);

    if (activeLow) {
        return !rawState;
    }

    return rawState;
}

void AdvanceStepperAxis::init() {
    gpio_init(config.dirPin);
    gpio_set_dir(config.dirPin, GPIO_OUT);
    gpio_put(config.dirPin, 0);

    gpio_init(config.enablePin);
    gpio_set_dir(config.enablePin, GPIO_OUT);

    disable();

    if (config.homeEnabled && !config.homeSimulationEnabled) {
        configureInputPin(
            config.homePin,
            config.homeUsePullup
        );
    }

    if (config.minLimitEnabled) {
        configureInputPin(
            config.minLimitPin,
            config.minLimitUsePullup
        );
    }

    if (config.maxLimitEnabled) {
        configureInputPin(
            config.maxLimitPin,
            config.maxLimitUsePullup
        );
    }
}

const char* AdvanceStepperAxis::getAxisName() const {
    return config.axisName;
}

void AdvanceStepperAxis::enable() {
    if (config.enableActiveLow) {
        gpio_put(config.enablePin, 0);
    } else {
        gpio_put(config.enablePin, 1);
    }

    enabled = true;
}

void AdvanceStepperAxis::disable() {
    if (config.enableActiveLow) {
        gpio_put(config.enablePin, 1);
    } else {
        gpio_put(config.enablePin, 0);
    }

    enabled = false;
}

void AdvanceStepperAxis::setDirection(bool positiveDirection) {
    currentDirection = positiveDirection;

    bool pinState = positiveDirection;

    if (config.directionInvert) {
        pinState = !pinState;
    }

    gpio_put(config.dirPin, pinState ? 1 : 0);
}

void AdvanceStepperAxis::setDirectionByDelta(int32_t deltaSteps) {
    setDirection(deltaSteps >= 0);
}

void AdvanceStepperAxis::setCurrentPositionSteps(int32_t positionSteps) {
    currentPositionSteps = positionSteps;
}

void AdvanceStepperAxis::setCurrentPositionMm(float positionMm) {
    currentPositionSteps = mmToSteps(positionMm);
}

int32_t AdvanceStepperAxis::getCurrentPositionSteps() const {
    return currentPositionSteps;
}

float AdvanceStepperAxis::getCurrentPositionMm() const {
    return stepsToMm(currentPositionSteps);
}

int32_t AdvanceStepperAxis::mmToSteps(float mm) const {
    return (int32_t)lroundf(mm * stepsPerMm);
}

float AdvanceStepperAxis::stepsToMm(int32_t steps) const {
    if (stepsPerMm <= 0.0f) {
        return 0.0f;
    }

    return (float)steps / stepsPerMm;
}

float AdvanceStepperAxis::getStepsPerMm() const {
    return stepsPerMm;
}

float AdvanceStepperAxis::getMaxSpeedMmS() const {
    return config.maxSpeedMmS;
}

float AdvanceStepperAxis::getAccelerationMmS2() const {
    return config.accelerationMmS2;
}

uint AdvanceStepperAxis::getStepPin() const {
    return config.stepPin;
}

uint AdvanceStepperAxis::getDirPin() const {
    return config.dirPin;
}

uint AdvanceStepperAxis::getEnablePin() const {
    return config.enablePin;
}

bool AdvanceStepperAxis::isEnabled() const {
    return enabled;
}

void AdvanceStepperAxis::addPositionSteps(int32_t deltaSteps) {
    currentPositionSteps += deltaSteps;
}

bool AdvanceStepperAxis::isHomeEnabled() const {
    return config.homeEnabled;
}

bool AdvanceStepperAxis::isHomeSensorActive() const {
    if (!config.homeEnabled) {
        return false;
    }

    if (config.homeSimulationEnabled) {
        return simulatedHomeActive;
    }

    return readInputActive(
        config.homePin,
        config.homeActiveLow
    );
}

bool AdvanceStepperAxis::isMinLimitEnabled() const {
    return config.minLimitEnabled;
}

bool AdvanceStepperAxis::isMaxLimitEnabled() const {
    return config.maxLimitEnabled;
}

bool AdvanceStepperAxis::isMinLimitActive() const {
    if (!config.minLimitEnabled) {
        return false;
    }

    return readInputActive(
        config.minLimitPin,
        config.minLimitActiveLow
    );
}

bool AdvanceStepperAxis::isMaxLimitActive() const {
    if (!config.maxLimitEnabled) {
        return false;
    }

    return readInputActive(
        config.maxLimitPin,
        config.maxLimitActiveLow
    );
}

bool AdvanceStepperAxis::getHomeDirectionPositive() const {
    return config.homeDirectionPositive;
}

float AdvanceStepperAxis::getHomingSeekSpeedMmS() const {
    return config.homingSeekSpeedMmS;
}

float AdvanceStepperAxis::getHomingLatchSpeedMmS() const {
    return config.homingLatchSpeedMmS;
}

float AdvanceStepperAxis::getHomingBackoffMm() const {
    return config.homingBackoffMm;
}

float AdvanceStepperAxis::getMaxHomingTravelMm() const {
    return config.maxHomingTravelMm;
}

float AdvanceStepperAxis::getHomePositionMm() const {
    return config.homePositionMm;
}

bool AdvanceStepperAxis::isHomeSimulationEnabled() const {
    return config.homeSimulationEnabled;
}

void AdvanceStepperAxis::setHomeSimulationEnabled(bool enabled) {
    config.homeSimulationEnabled = enabled;
}

void AdvanceStepperAxis::setSimulatedHomeActive(bool active) {
    simulatedHomeActive = active;
}

bool AdvanceStepperAxis::isHomed() const {
    return homed;
}

void AdvanceStepperAxis::setHomed(bool state) {
    homed = state;
}

bool AdvanceStepperAxis::areSoftLimitsEnabled() const {
    return config.softLimitsEnabled;
}

bool AdvanceStepperAxis::isTargetWithinSoftLimits(float targetMm) const {
    if (!config.softLimitsEnabled) {
        return true;
    }

    if (targetMm < config.softLimitMinMm) {
        return false;
    }

    if (targetMm > config.softLimitMaxMm) {
        return false;
    }

    return true;
}

float AdvanceStepperAxis::getSoftLimitMinMm() const {
    return config.softLimitMinMm;
}

float AdvanceStepperAxis::getSoftLimitMaxMm() const {
    return config.softLimitMaxMm;
}
