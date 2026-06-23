#ifndef TRAPEZOID_PROFILE_H
#define TRAPEZOID_PROFILE_H

#include "pico/stdlib.h"

struct TrapezoidProfileData {
    uint32_t accelerationSteps;
    uint32_t cruiseSteps;
    uint32_t decelerationSteps;

    float entrySpeedMmS;
    float exitSpeedMmS;

    float requestedSpeedMmS;
    float reachableSpeedMmS;
    float accelerationMmS2;

    float accelerationDistanceMm;
    float cruiseDistanceMm;
    float decelerationDistanceMm;

    bool isTriangular;
    bool valid;
};

class TrapezoidProfile {
public:
    static TrapezoidProfileData calculate(
        uint32_t totalSteps,
        float distanceMm,
        float requestedSpeedMmS,
        float accelerationMmS2,
        float entrySpeedMmS,
        float exitSpeedMmS
    );
};

#endif
