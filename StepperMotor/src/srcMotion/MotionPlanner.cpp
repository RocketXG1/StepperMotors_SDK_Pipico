#include "MotionPlanner.h"

#include <math.h>

MotionPlanner::MotionPlanner() {
    axisCount = 0;
    configured = false;

    config.blendEnabled = false;
    config.blendDistanceMm = 1.0f;
    config.blendSegmentLengthMm = 0.5f;
    config.maxBlendSegments = 8;
    config.maxTrimRatio = 0.35f;
    config.minMovementMm = 0.01f;
    config.blendSpeedScale = 0.70f;

    rawCount = 0;

    plannedHead = 0;
    plannedTail = 0;
    plannedCount = 0;

    for (uint i = 0; i < MAX_AXES; i++) {
        startPositionMm[i] = 0.0f;
    }

    for (uint i = 0; i < RAW_QUEUE_SIZE; i++) {
        rawQueue[i].reset();
    }

    for (uint i = 0; i < PLANNED_QUEUE_SIZE; i++) {
        plannedQueue[i].reset();
    }
}

void MotionPlanner::configure(
    uint numberOfAxes,
    const MotionPlannerConfig& plannerConfig
) {
    if (numberOfAxes == 0) {
        configured = false;
        return;
    }

    if (numberOfAxes > MAX_AXES) {
        configured = false;
        return;
    }

    axisCount = numberOfAxes;
    config = plannerConfig;

    if (config.blendDistanceMm < 0.0f) {
        config.blendDistanceMm = 0.0f;
    }

    if (config.blendSegmentLengthMm <= 0.0f) {
        config.blendSegmentLengthMm = 0.5f;
    }

    if (config.maxBlendSegments < 2) {
        config.maxBlendSegments = 2;
    }

    if (config.maxBlendSegments > 32) {
        config.maxBlendSegments = 32;
    }

    if (config.maxTrimRatio <= 0.0f) {
        config.maxTrimRatio = 0.35f;
    }

    if (config.maxTrimRatio > 0.45f) {
        config.maxTrimRatio = 0.45f;
    }

    if (config.minMovementMm <= 0.0f) {
        config.minMovementMm = 0.01f;
    }

    if (config.blendSpeedScale <= 0.0f) {
        config.blendSpeedScale = 0.70f;
    }

    if (config.blendSpeedScale > 1.0f) {
        config.blendSpeedScale = 1.0f;
    }

    configured = true;
}

void MotionPlanner::setStartPosition(
    const float currentPositionMm[],
    uint numberOfAxes
) {
    if (currentPositionMm == nullptr) {
        return;
    }

    if (numberOfAxes != axisCount) {
        return;
    }

    for (uint i = 0; i < axisCount; i++) {
        startPositionMm[i] = currentPositionMm[i];
    }
}

void MotionPlanner::setBlendEnabled(bool enabled) {
    config.blendEnabled = enabled;
}

void MotionPlanner::setBlendParameters(
    float blendDistanceMm,
    float blendSegmentLengthMm,
    uint maxBlendSegments,
    float blendSpeedScale
) {
    config.blendDistanceMm = blendDistanceMm;
    config.blendSegmentLengthMm = blendSegmentLengthMm;
    config.maxBlendSegments = maxBlendSegments;
    config.blendSpeedScale = blendSpeedScale;

    MotionPlannerConfig copy = config;

    configure(
        axisCount,
        copy
    );
}

bool MotionPlanner::pushLinearTo(
    std::initializer_list<float> targetPositionsMm,
    float requestedSpeedMmS
) {
    if (!configured) {
        return false;
    }

    if (targetPositionsMm.size() != axisCount) {
        return false;
    }

    if (requestedSpeedMmS <= 0.0f) {
        return false;
    }

    if (rawCount >= RAW_QUEUE_SIZE) {
        return false;
    }

    RawLinearMove& move = rawQueue[rawCount];

    move.reset();

    move.axisCount = axisCount;
    move.requestedSpeedMmS = requestedSpeedMmS;

    uint index = 0;

    for (float targetMm : targetPositionsMm) {
        move.targetPositionMm[index] = targetMm;
        index++;
    }

    move.valid = true;

    rawCount++;

    return true;
}

bool MotionPlanner::buildPlannedQueue() {
    if (!configured) {
        return false;
    }

    clearPlannedQueueOnly();

    if (rawCount == 0) {
        return true;
    }

    float points[RAW_QUEUE_SIZE + 1][MAX_AXES];

    for (uint axis = 0; axis < axisCount; axis++) {
        points[0][axis] = startPositionMm[axis];
    }

    for (uint i = 0; i < rawCount; i++) {
        for (uint axis = 0; axis < axisCount; axis++) {
            points[i + 1][axis] =
                rawQueue[i].targetPositionMm[axis];
        }
    }

    bool cornerHasBlend[RAW_QUEUE_SIZE + 1];

    float blendEntryPoint[RAW_QUEUE_SIZE + 1][MAX_AXES];
    float blendExitPoint[RAW_QUEUE_SIZE + 1][MAX_AXES];

    for (uint i = 0; i < RAW_QUEUE_SIZE + 1; i++) {
        cornerHasBlend[i] = false;

        for (uint axis = 0; axis < MAX_AXES; axis++) {
            blendEntryPoint[i][axis] = 0.0f;
            blendExitPoint[i][axis] = 0.0f;
        }
    }

    /*
        El suavizado geométrico requiere mínimo 2 ejes.

        Si axisCount == 1:
        - se permite mover el eje
        - se permite cola de movimientos
        - no se generan curvas
    */
    if (config.blendEnabled && rawCount >= 2 && axisCount >= 2) {
        for (uint cornerIndex = 1; cornerIndex < rawCount; cornerIndex++) {
            float unitIn[MAX_AXES];
            float unitOut[MAX_AXES];

            float lengthIn = 0.0f;
            float lengthOut = 0.0f;

            bool validIn =
                normalizeVector(
                    points[cornerIndex - 1],
                    points[cornerIndex],
                    unitIn,
                    axisCount,
                    lengthIn
                );

            bool validOut =
                normalizeVector(
                    points[cornerIndex],
                    points[cornerIndex + 1],
                    unitOut,
                    axisCount,
                    lengthOut
                );

            if (!validIn || !validOut) {
                continue;
            }

            float dot =
                dotProduct(
                    unitIn,
                    unitOut,
                    axisCount
                );

            if (dot > 0.999f) {
                continue;
            }

            if (dot < -0.95f) {
                continue;
            }

            float angleFactor =
                (1.0f - dot) * 0.5f;

            float trimDistance =
                config.blendDistanceMm * angleFactor;

            float maxTrimIn =
                lengthIn * config.maxTrimRatio;

            float maxTrimOut =
                lengthOut * config.maxTrimRatio;

            if (trimDistance > maxTrimIn) {
                trimDistance = maxTrimIn;
            }

            if (trimDistance > maxTrimOut) {
                trimDistance = maxTrimOut;
            }

            if (trimDistance <= config.minMovementMm) {
                continue;
            }

            calculatePointBeforeCorner(
                points[cornerIndex],
                unitIn,
                trimDistance,
                blendEntryPoint[cornerIndex],
                axisCount
            );

            calculatePointAfterCorner(
                points[cornerIndex],
                unitOut,
                trimDistance,
                blendExitPoint[cornerIndex],
                axisCount
            );

            cornerHasBlend[cornerIndex] = true;
        }
    }

    for (uint rawIndex = 0; rawIndex < rawCount; rawIndex++) {
        uint cornerAtEnd =
            rawIndex + 1;

        float lineEnd[MAX_AXES];

        if (cornerHasBlend[cornerAtEnd]) {
            copyPoint(
                lineEnd,
                blendEntryPoint[cornerAtEnd],
                axisCount
            );
        } else {
            copyPoint(
                lineEnd,
                points[cornerAtEnd],
                axisCount
            );
        }

        if (!addPlannedMove(
                lineEnd,
                axisCount,
                rawQueue[rawIndex].requestedSpeedMmS,
                false
            )) {
            return false;
        }

        if (cornerHasBlend[cornerAtEnd]) {
            float entryPoint[MAX_AXES];
            float cornerPoint[MAX_AXES];
            float exitPoint[MAX_AXES];

            copyPoint(
                entryPoint,
                blendEntryPoint[cornerAtEnd],
                axisCount
            );

            copyPoint(
                cornerPoint,
                points[cornerAtEnd],
                axisCount
            );

            copyPoint(
                exitPoint,
                blendExitPoint[cornerAtEnd],
                axisCount
            );

            float curveDistance =
                distanceBetween(
                    entryPoint,
                    exitPoint,
                    axisCount
                );

            uint blendSegments =
                (uint)ceilf(
                    curveDistance /
                    config.blendSegmentLengthMm
                );

            if (blendSegments < 2) {
                blendSegments = 2;
            }

            if (blendSegments > config.maxBlendSegments) {
                blendSegments = config.maxBlendSegments;
            }

            float currentSpeed =
                rawQueue[rawIndex].requestedSpeedMmS;

            float nextSpeed =
                rawQueue[rawIndex + 1].requestedSpeedMmS;

            float blendSpeed = currentSpeed;

            if (nextSpeed < blendSpeed) {
                blendSpeed = nextSpeed;
            }

            blendSpeed =
                blendSpeed *
                config.blendSpeedScale;

            for (uint segment = 1; segment <= blendSegments; segment++) {
                float t =
                    (float)segment /
                    (float)blendSegments;

                float bezierPoint[MAX_AXES];

                calculateBezierPoint(
                    entryPoint,
                    cornerPoint,
                    exitPoint,
                    t,
                    bezierPoint,
                    axisCount
                );

                if (!addPlannedMove(
                        bezierPoint,
                        axisCount,
                        blendSpeed,
                        true
                    )) {
                    return false;
                }
            }
        }
    }

    assignPlannedSpeeds();

    return true;
}

bool MotionPlanner::addPlannedMove(
    const float targetPositionMm[],
    uint targetAxisCount,
    float requestedSpeedMmS,
    bool isBlendSegment
) {
    if (targetPositionMm == nullptr) {
        return false;
    }

    if (targetAxisCount != axisCount) {
        return false;
    }

    if (requestedSpeedMmS <= 0.0f) {
        return false;
    }

    if (plannedCount >= PLANNED_QUEUE_SIZE) {
        return false;
    }

    PlannedLinearMove& move =
        plannedQueue[plannedHead];

    move.reset();

    move.axisCount = axisCount;
    move.requestedSpeedMmS = requestedSpeedMmS;
    move.isBlendSegment = isBlendSegment;

    for (uint i = 0; i < axisCount; i++) {
        move.targetPositionMm[i] =
            targetPositionMm[i];
    }

    move.valid = true;

    plannedHead =
        (plannedHead + 1) %
        PLANNED_QUEUE_SIZE;

    plannedCount++;

    return true;
}

void MotionPlanner::assignPlannedSpeeds() {
    if (plannedCount == 0) {
        return;
    }

    for (uint i = 0; i < plannedCount; i++) {
        plannedQueue[i].entrySpeedMmS = 0.0f;
        plannedQueue[i].exitSpeedMmS = 0.0f;
    }

    for (uint i = 0; i < plannedCount; i++) {
        if (i == 0) {
            plannedQueue[i].entrySpeedMmS = 0.0f;
        } else {
            float junctionSpeed = 0.0f;

            bool previousBlend =
                plannedQueue[i - 1].isBlendSegment;

            bool currentBlend =
                plannedQueue[i].isBlendSegment;

            if (previousBlend || currentBlend) {
                junctionSpeed =
                    plannedQueue[i - 1].requestedSpeedMmS;

                if (plannedQueue[i].requestedSpeedMmS < junctionSpeed) {
                    junctionSpeed =
                        plannedQueue[i].requestedSpeedMmS;
                }
            }

            plannedQueue[i].entrySpeedMmS =
                junctionSpeed;

            plannedQueue[i - 1].exitSpeedMmS =
                junctionSpeed;
        }
    }

    plannedQueue[plannedCount - 1].exitSpeedMmS = 0.0f;
}

bool MotionPlanner::popPlannedMove(
    PlannedLinearMove& outputMove
) {
    outputMove.reset();

    if (plannedCount == 0) {
        return false;
    }

    outputMove = plannedQueue[plannedTail];

    plannedQueue[plannedTail].reset();

    plannedTail =
        (plannedTail + 1) %
        PLANNED_QUEUE_SIZE;

    plannedCount--;

    return outputMove.valid;
}

bool MotionPlanner::hasPlannedMoves() const {
    return plannedCount > 0;
}

bool MotionPlanner::hasRawMoves() const {
    return rawCount > 0;
}

uint MotionPlanner::getRawMoveCount() const {
    return rawCount;
}

uint MotionPlanner::getPlannedMoveCount() const {
    return plannedCount;
}

void MotionPlanner::clear() {
    rawCount = 0;

    for (uint i = 0; i < RAW_QUEUE_SIZE; i++) {
        rawQueue[i].reset();
    }

    clearPlannedQueueOnly();
}

void MotionPlanner::clearPlannedQueueOnly() {
    plannedHead = 0;
    plannedTail = 0;
    plannedCount = 0;

    for (uint i = 0; i < PLANNED_QUEUE_SIZE; i++) {
        plannedQueue[i].reset();
    }
}

float MotionPlanner::distanceBetween(
    const float a[],
    const float b[],
    uint count
) const {
    float sum = 0.0f;

    for (uint i = 0; i < count; i++) {
        float difference =
            b[i] - a[i];

        sum += difference * difference;
    }

    return sqrtf(sum);
}

bool MotionPlanner::normalizeVector(
    const float start[],
    const float end[],
    float outputUnit[],
    uint count,
    float& outputDistance
) const {
    outputDistance =
        distanceBetween(
            start,
            end,
            count
        );

    if (outputDistance <= config.minMovementMm) {
        for (uint i = 0; i < count; i++) {
            outputUnit[i] = 0.0f;
        }

        return false;
    }

    for (uint i = 0; i < count; i++) {
        outputUnit[i] =
            (end[i] - start[i]) /
            outputDistance;
    }

    return true;
}

float MotionPlanner::dotProduct(
    const float a[],
    const float b[],
    uint count
) const {
    float result = 0.0f;

    for (uint i = 0; i < count; i++) {
        result += a[i] * b[i];
    }

    return result;
}

void MotionPlanner::copyPoint(
    float destination[],
    const float source[],
    uint count
) const {
    for (uint i = 0; i < count; i++) {
        destination[i] = source[i];
    }
}

void MotionPlanner::calculatePointBeforeCorner(
    const float corner[],
    const float unitIn[],
    float trimDistance,
    float outputPoint[],
    uint count
) const {
    for (uint i = 0; i < count; i++) {
        outputPoint[i] =
            corner[i] -
            unitIn[i] * trimDistance;
    }
}

void MotionPlanner::calculatePointAfterCorner(
    const float corner[],
    const float unitOut[],
    float trimDistance,
    float outputPoint[],
    uint count
) const {
    for (uint i = 0; i < count; i++) {
        outputPoint[i] =
            corner[i] +
            unitOut[i] * trimDistance;
    }
}

void MotionPlanner::calculateBezierPoint(
    const float start[],
    const float control[],
    const float end[],
    float t,
    float outputPoint[],
    uint count
) const {
    float oneMinusT =
        1.0f - t;

    float firstWeight =
        oneMinusT * oneMinusT;

    float controlWeight =
        2.0f * oneMinusT * t;

    float endWeight =
        t * t;

    for (uint i = 0; i < count; i++) {
        outputPoint[i] =
            firstWeight * start[i] +
            controlWeight * control[i] +
            endWeight * end[i];
    }
}
