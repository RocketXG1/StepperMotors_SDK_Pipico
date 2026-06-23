#ifndef MOTION_BLOCK_H
#define MOTION_BLOCK_H

#include "pico/stdlib.h"

struct MotionBlock {
    static const uint MAX_AXES = 8;

    uint axisCount;

    int32_t deltaSteps[MAX_AXES];
    uint32_t absSteps[MAX_AXES];
    bool direction[MAX_AXES];

    uint32_t dominantSteps;

    float deltaMm[MAX_AXES];

    float distanceMm;

    float requestedSpeedMmS;
    float nominalSpeedMmS;

    float entrySpeedMmS;
    float exitSpeedMmS;

    float accelerationMmS2;

    bool valid;

    void reset() {
        axisCount = 0;

        dominantSteps = 0;

        distanceMm = 0.0f;

        requestedSpeedMmS = 0.0f;
        nominalSpeedMmS = 0.0f;

        entrySpeedMmS = 0.0f;
        exitSpeedMmS = 0.0f;

        accelerationMmS2 = 0.0f;

        valid = false;

        for (uint i = 0; i < MAX_AXES; i++) {
            deltaSteps[i] = 0;
            absSteps[i] = 0;
            direction[i] = true;
            deltaMm[i] = 0.0f;
        }
    }
};

#endif
