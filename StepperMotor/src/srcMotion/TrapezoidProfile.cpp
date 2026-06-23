#include "TrapezoidProfile.h"

#include <math.h>

TrapezoidProfileData TrapezoidProfile::calculate(
    uint32_t totalSteps,
    float distanceMm,
    float requestedSpeedMmS,
    float accelerationMmS2,
    float entrySpeedMmS,
    float exitSpeedMmS
) {
    TrapezoidProfileData profile;

    profile.accelerationSteps = 0;
    profile.cruiseSteps = 0;
    profile.decelerationSteps = 0;

    profile.entrySpeedMmS = entrySpeedMmS;
    profile.exitSpeedMmS = exitSpeedMmS;

    profile.requestedSpeedMmS = requestedSpeedMmS;
    profile.reachableSpeedMmS = 0.0f;
    profile.accelerationMmS2 = accelerationMmS2;

    profile.accelerationDistanceMm = 0.0f;
    profile.cruiseDistanceMm = 0.0f;
    profile.decelerationDistanceMm = 0.0f;

    profile.isTriangular = false;
    profile.valid = false;

    if (totalSteps == 0) {
        return profile;
    }

    if (distanceMm <= 0.0f) {
        return profile;
    }

    if (requestedSpeedMmS <= 0.0f) {
        return profile;
    }

    if (accelerationMmS2 <= 0.0f) {
        return profile;
    }

    if (profile.entrySpeedMmS < 0.0f) {
        profile.entrySpeedMmS = 0.0f;
    }

    if (profile.exitSpeedMmS < 0.0f) {
        profile.exitSpeedMmS = 0.0f;
    }

    if (profile.entrySpeedMmS > requestedSpeedMmS) {
        profile.entrySpeedMmS = requestedSpeedMmS;
    }

    if (profile.exitSpeedMmS > requestedSpeedMmS) {
        profile.exitSpeedMmS = requestedSpeedMmS;
    }

    float requestedSquared =
        requestedSpeedMmS * requestedSpeedMmS;

    float entrySquared =
        profile.entrySpeedMmS * profile.entrySpeedMmS;

    float exitSquared =
        profile.exitSpeedMmS * profile.exitSpeedMmS;

    float accelerationDistance =
        (requestedSquared - entrySquared) /
        (2.0f * accelerationMmS2);

    float decelerationDistance =
        (requestedSquared - exitSquared) /
        (2.0f * accelerationMmS2);

    if (accelerationDistance < 0.0f) {
        accelerationDistance = 0.0f;
    }

    if (decelerationDistance < 0.0f) {
        decelerationDistance = 0.0f;
    }

    float totalAccelDecelDistance =
        accelerationDistance + decelerationDistance;

    if (totalAccelDecelDistance <= distanceMm) {
        profile.isTriangular = false;

        profile.reachableSpeedMmS = requestedSpeedMmS;

        profile.accelerationDistanceMm = accelerationDistance;
        profile.decelerationDistanceMm = decelerationDistance;

        profile.cruiseDistanceMm =
            distanceMm -
            accelerationDistance -
            decelerationDistance;
    } else {
        profile.isTriangular = true;

        float peakSquared =
            (
                2.0f * accelerationMmS2 * distanceMm +
                entrySquared +
                exitSquared
            ) /
            2.0f;

        if (peakSquared < entrySquared) {
            peakSquared = entrySquared;
        }

        if (peakSquared < exitSquared) {
            peakSquared = exitSquared;
        }

        profile.reachableSpeedMmS =
            sqrtf(peakSquared);

        if (profile.reachableSpeedMmS > requestedSpeedMmS) {
            profile.reachableSpeedMmS = requestedSpeedMmS;
        }

        float peakSquaredFinal =
            profile.reachableSpeedMmS *
            profile.reachableSpeedMmS;

        profile.accelerationDistanceMm =
            (peakSquaredFinal - entrySquared) /
            (2.0f * accelerationMmS2);

        profile.decelerationDistanceMm =
            (peakSquaredFinal - exitSquared) /
            (2.0f * accelerationMmS2);

        if (profile.accelerationDistanceMm < 0.0f) {
            profile.accelerationDistanceMm = 0.0f;
        }

        if (profile.decelerationDistanceMm < 0.0f) {
            profile.decelerationDistanceMm = 0.0f;
        }

        profile.cruiseDistanceMm = 0.0f;
    }

    profile.accelerationSteps =
        (uint32_t)(
            (profile.accelerationDistanceMm / distanceMm) *
            (float)totalSteps
        );

    profile.decelerationSteps =
        (uint32_t)(
            (profile.decelerationDistanceMm / distanceMm) *
            (float)totalSteps
        );

    if (
        profile.accelerationSteps +
        profile.decelerationSteps >
        totalSteps
    ) {
        profile.accelerationSteps = totalSteps / 2;

        profile.decelerationSteps =
            totalSteps - profile.accelerationSteps;

        profile.cruiseSteps = 0;
    } else {
        profile.cruiseSteps =
            totalSteps -
            profile.accelerationSteps -
            profile.decelerationSteps;
    }

    profile.valid = true;

    return profile;
}
