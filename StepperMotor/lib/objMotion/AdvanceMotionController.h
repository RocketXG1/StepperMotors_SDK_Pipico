#ifndef ADVANCE_MOTION_CONTROLLER_H
#define ADVANCE_MOTION_CONTROLLER_H

#include <initializer_list>

#include "pico/stdlib.h"

#include "AdvanceStepperAxis.h"
#include "StepperPioPulseEngine.h"

#include "MotionBlock.h"
#include "TrapezoidProfile.h"
#include "MotionPlanner.h"

class AdvanceMotionController {
private:
    static const uint MAX_AXES = 8;
    static const uint MIN_AXES = 1;

    AdvanceStepperAxis* axes[MAX_AXES];
    uint axisCount;

    StepperPioPulseEngine* pulseEngine;

    MotionPlanner planner;

    MotionBlock currentBlock;
    TrapezoidProfileData currentProfile;

    bool validController;
    bool busy;

    uint32_t dominantStepCounter;

    int32_t bresenhamError[MAX_AXES];

    absolute_time_t nextStepTime;

    /*
        Orden configurable para HOME.

        Se puede configurar por índice:
        setHomeOrder({2, 0, 1});

        O por nombre:
        setHomeOrderByName({"Z", "X", "Y"});
    */
    uint homeOrder[MAX_AXES];
    uint homeOrderCount;
    bool homeOrderConfigured;

    bool loadAxes(
        AdvanceStepperAxis* axisList[],
        uint numberOfAxes
    );

    bool validateAxesStepPins();

    bool textEquals(
        const char* firstText,
        const char* secondText
    ) const;

    bool isReservedAxisName(
        const char* axisName
    ) const;

    bool validateAxisNames() const;

    bool normalizeAxisName(
        const char* inputName,
        char outputName[],
        uint outputSize
    ) const;

    bool findAxisIndexByName(
        const char* axisName,
        uint& outputAxisIndex
    ) const;

    bool buildLinearBlockToArray(
        const float targetPositionsMm[],
        uint targetCount,
        float requestedSpeedMmS,
        float entrySpeedMmS,
        float exitSpeedMmS,
        MotionBlock& block
    );

    bool buildLinearBlockTo(
        std::initializer_list<float> targetPositionsMm,
        float requestedSpeedMmS,
        MotionBlock& block
    );

    bool startMotionBlock(
        const MotionBlock& block
    );

    bool startPlannedLinearMove(
        const PlannedLinearMove& plannedMove
    );

    void prepareDirections(const MotionBlock& block);
    void prepareBresenham(const MotionBlock& block);

    float calculateDistanceFromDeltas(
        const float deltas[],
        uint count
    );

    float limitSpeedByAxisLimits(
        const MotionBlock& block,
        float requestedSpeedMmS
    );

    float limitSpeedByPulseEngine(
        const MotionBlock& block,
        float currentLimitedSpeed
    );

    float limitAccelerationByAxisLimits(
        const MotionBlock& block
    );

    void executeBresenhamStep();

    uint32_t calculateCurrentStepDelayUs();

    void loadNextPlannedMoveIfAvailable();

    uint32_t speedToStepDelayUs(
        uint axisIndex,
        float speedMmS
    ) const;

    bool isHardwareLimitBlockingDirection(
        uint axisIndex,
        bool positiveDirection
    ) const;

    bool stepAxisOnceBlocking(
        uint axisIndex,
        bool positiveDirection,
        uint32_t delayUs
    );

    bool moveAxisRelativeBlocking(
        uint axisIndex,
        float distanceMm,
        float speedMmS
    );

public:
    AdvanceMotionController(
        AdvanceStepperAxis* axisList[],
        uint numberOfAxes,
        StepperPioPulseEngine& selectedPulseEngine
    );

    void init();

    bool isValid() const;

    uint getAxisCount() const;
    const char* getAxisName(uint axisIndex) const;

    void printAxisMap() const;
    void printHomeOrder() const;

    bool moveLinearTo(
        std::initializer_list<float> targetPositionsMm,
        float requestedSpeedMmS
    );

    void configurePathSmoothing(
        bool enabled,
        float blendDistanceMm,
        float blendSegmentLengthMm,
        uint maxBlendSegments,
        float blendSpeedScale
    );

    void clearMotionQueue();

    bool queueLinearTo(
        std::initializer_list<float> targetPositionsMm,
        float requestedSpeedMmS
    );

    bool prepareQueuedMoves();

    uint getRawQueuedMoveCount() const;
    uint getPlannedMoveCount() const;

    void update();

    bool isBusy() const;

    void stop();
    void emergencyStop();

    void setCurrentPosition(
        std::initializer_list<float> currentPositionsMm
    );

    bool getCurrentPosition(
        float outputPositionsMm[],
        uint outputCount
    ) const;

    bool setHomeOrder(
        std::initializer_list<uint> orderedAxisIndexes
    );

    bool setHomeOrderByName(
        std::initializer_list<const char*> orderedAxisNames
    );

    void clearHomeOrder();

    bool homeAxis(uint axisIndex);
    bool homeAll();

    bool moveToSafePositionAfterHome(
        std::initializer_list<float> safePositionsMm,
        float speedMmS
    );
};

#endif
