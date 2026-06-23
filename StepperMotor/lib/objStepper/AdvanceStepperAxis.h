#ifndef ADVANCE_STEPPER_AXIS_H
#define ADVANCE_STEPPER_AXIS_H

#include "pico/stdlib.h"
#include "StepperAxisConfig.h"

class AdvanceStepperAxis {
private:
    StepperAxisConfig config;

    float stepsPerMm;

    int32_t currentPositionSteps;
    int32_t targetPositionSteps;

    bool enabled;
    bool currentDirection;
    bool homed;

    bool simulatedHomeActive;

    void calculateStepsPerMm();

    void configureInputPin(
        uint pin,
        bool usePullup
    );

    bool readInputActive(
        uint pin,
        bool activeLow
    ) const;

public:
    AdvanceStepperAxis(const StepperAxisConfig& axisConfig);

    void init();

    const char* getAxisName() const;

    void enable();
    void disable();

    void setDirection(bool positiveDirection);
    void setDirectionByDelta(int32_t deltaSteps);

    void setCurrentPositionSteps(int32_t positionSteps);
    void setCurrentPositionMm(float positionMm);

    int32_t getCurrentPositionSteps() const;
    float getCurrentPositionMm() const;

    int32_t mmToSteps(float mm) const;
    float stepsToMm(int32_t steps) const;

    float getStepsPerMm() const;

    float getMaxSpeedMmS() const;
    float getAccelerationMmS2() const;

    uint getStepPin() const;
    uint getDirPin() const;
    uint getEnablePin() const;

    bool isEnabled() const;

    void addPositionSteps(int32_t deltaSteps);

    bool isHomeEnabled() const;
    bool isHomeSensorActive() const;

    bool isMinLimitEnabled() const;
    bool isMaxLimitEnabled() const;

    bool isMinLimitActive() const;
    bool isMaxLimitActive() const;

    bool getHomeDirectionPositive() const;

    float getHomingSeekSpeedMmS() const;
    float getHomingLatchSpeedMmS() const;
    float getHomingBackoffMm() const;
    float getMaxHomingTravelMm() const;
    float getHomePositionMm() const;

    bool isHomeSimulationEnabled() const;
    void setHomeSimulationEnabled(bool enabled);
    void setSimulatedHomeActive(bool active);

    bool isHomed() const;
    void setHomed(bool state);

    bool areSoftLimitsEnabled() const;
    bool isTargetWithinSoftLimits(float targetMm) const;
    float getSoftLimitMinMm() const;
    float getSoftLimitMaxMm() const;
};

#endif
