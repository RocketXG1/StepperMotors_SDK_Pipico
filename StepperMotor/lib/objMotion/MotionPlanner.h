#ifndef MOTION_PLANNER_H
#define MOTION_PLANNER_H

#include <initializer_list>

#include "pico/stdlib.h"

struct MotionPlannerConfig {
    bool blendEnabled;

    float blendDistanceMm;
    float blendSegmentLengthMm;

    uint maxBlendSegments;

    float maxTrimRatio;
    float minMovementMm;

    float blendSpeedScale;
};

struct RawLinearMove {
    static const uint MAX_AXES = 8;

    uint axisCount;
    float targetPositionMm[MAX_AXES];
    float requestedSpeedMmS;
    bool valid;

    void reset() {
        axisCount = 0;
        requestedSpeedMmS = 0.0f;
        valid = false;

        for (uint i = 0; i < MAX_AXES; i++) {
            targetPositionMm[i] = 0.0f;
        }
    }
};

struct PlannedLinearMove {
    static const uint MAX_AXES = 8;

    uint axisCount;

    float targetPositionMm[MAX_AXES];

    float requestedSpeedMmS;
    float entrySpeedMmS;
    float exitSpeedMmS;

    bool isBlendSegment;
    bool valid;

    void reset() {
        axisCount = 0;

        requestedSpeedMmS = 0.0f;
        entrySpeedMmS = 0.0f;
        exitSpeedMmS = 0.0f;

        isBlendSegment = false;
        valid = false;

        for (uint i = 0; i < MAX_AXES; i++) {
            targetPositionMm[i] = 0.0f;
        }
    }
};

class MotionPlanner {
private:
    static const uint MAX_AXES = 8;

    static const uint RAW_QUEUE_SIZE = 24;
    static const uint PLANNED_QUEUE_SIZE = 128;

    uint axisCount;
    bool configured;

    MotionPlannerConfig config;

    RawLinearMove rawQueue[RAW_QUEUE_SIZE];
    uint rawCount;

    PlannedLinearMove plannedQueue[PLANNED_QUEUE_SIZE];
    uint plannedHead;
    uint plannedTail;
    uint plannedCount;

    float startPositionMm[MAX_AXES];

    bool addPlannedMove(
        const float targetPositionMm[],
        uint targetAxisCount,
        float requestedSpeedMmS,
        bool isBlendSegment
    );

    void assignPlannedSpeeds();

    void clearPlannedQueueOnly();

    float distanceBetween(
        const float a[],
        const float b[],
        uint count
    ) const;

    bool normalizeVector(
        const float start[],
        const float end[],
        float outputUnit[],
        uint count,
        float& outputDistance
    ) const;

    float dotProduct(
        const float a[],
        const float b[],
        uint count
    ) const;

    void copyPoint(
        float destination[],
        const float source[],
        uint count
    ) const;

    void calculatePointBeforeCorner(
        const float corner[],
        const float unitIn[],
        float trimDistance,
        float outputPoint[],
        uint count
    ) const;

    void calculatePointAfterCorner(
        const float corner[],
        const float unitOut[],
        float trimDistance,
        float outputPoint[],
        uint count
    ) const;

    void calculateBezierPoint(
        const float start[],
        const float control[],
        const float end[],
        float t,
        float outputPoint[],
        uint count
    ) const;

public:
    MotionPlanner();

    void configure(
        uint numberOfAxes,
        const MotionPlannerConfig& plannerConfig
    );

    void setStartPosition(
        const float currentPositionMm[],
        uint numberOfAxes
    );

    void setBlendEnabled(bool enabled);

    void setBlendParameters(
        float blendDistanceMm,
        float blendSegmentLengthMm,
        uint maxBlendSegments,
        float blendSpeedScale
    );

    bool pushLinearTo(
        std::initializer_list<float> targetPositionsMm,
        float requestedSpeedMmS
    );

    bool buildPlannedQueue();

    bool popPlannedMove(PlannedLinearMove& outputMove);

    bool hasPlannedMoves() const;
    bool hasRawMoves() const;

    uint getRawMoveCount() const;
    uint getPlannedMoveCount() const;

    void clear();
};

#endif
