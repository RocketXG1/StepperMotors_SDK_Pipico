#include "AdvanceMotionController.h"

#include <math.h>
#include <stdio.h>

AdvanceMotionController::AdvanceMotionController(
    AdvanceStepperAxis* axisList[],
    uint numberOfAxes,
    StepperPioPulseEngine& selectedPulseEngine
) {
    axisCount = 0;

    pulseEngine = &selectedPulseEngine;

    validController = false;
    busy = false;

    dominantStepCounter = 0;

    nextStepTime = get_absolute_time();

    homeOrderCount = 0;
    homeOrderConfigured = false;

    for (uint i = 0; i < MAX_AXES; i++) {
        axes[i] = nullptr;
        bresenhamError[i] = 0;
        homeOrder[i] = 0;
    }

    currentBlock.reset();

    if (loadAxes(axisList, numberOfAxes)) {
        validController = true;
    }

    if (validController) {
        validController =
            validateAxesStepPins();
    }

    if (validController) {
        validController =
            validateAxisNames();
    }

    MotionPlannerConfig plannerConfig;

    plannerConfig.blendEnabled = false;
    plannerConfig.blendDistanceMm = 1.0f;
    plannerConfig.blendSegmentLengthMm = 0.5f;
    plannerConfig.maxBlendSegments = 8;
    plannerConfig.maxTrimRatio = 0.35f;
    plannerConfig.minMovementMm = 0.01f;
    plannerConfig.blendSpeedScale = 0.70f;

    if (validController) {
        planner.configure(
            axisCount,
            plannerConfig
        );
    }
}

bool AdvanceMotionController::loadAxes(
    AdvanceStepperAxis* axisList[],
    uint numberOfAxes
) {
    if (axisList == nullptr) {
        return false;
    }

    if (numberOfAxes < MIN_AXES) {
        return false;
    }

    if (numberOfAxes > MAX_AXES) {
        return false;
    }

    for (uint i = 0; i < numberOfAxes; i++) {
        if (axisList[i] == nullptr) {
            return false;
        }

        axes[i] = axisList[i];
    }

    axisCount = numberOfAxes;

    return true;
}

bool AdvanceMotionController::validateAxesStepPins() {
    if (pulseEngine == nullptr) {
        return false;
    }

    bool usedOffsets[MAX_AXES];

    for (uint i = 0; i < MAX_AXES; i++) {
        usedOffsets[i] = false;
    }

    for (uint i = 0; i < axisCount; i++) {
        uint stepPin =
            axes[i]->getStepPin();

        if (!pulseEngine->pinBelongsToStepGroup(stepPin)) {
            return false;
        }

        uint offset =
            pulseEngine->getPinOffset(stepPin);

        if (offset >= MAX_AXES) {
            return false;
        }

        if (usedOffsets[offset]) {
            return false;
        }

        usedOffsets[offset] = true;
    }

    return true;
}

bool AdvanceMotionController::textEquals(
    const char* firstText,
    const char* secondText
) const {
    if (firstText == nullptr || secondText == nullptr) {
        return false;
    }

    uint index = 0;

    while (
        firstText[index] != '\0' ||
        secondText[index] != '\0'
    ) {
        if (firstText[index] != secondText[index]) {
            return false;
        }

        index++;
    }

    return true;
}

bool AdvanceMotionController::isReservedAxisName(
    const char* axisName
) const {
    if (axisName == nullptr) {
        return true;
    }

    if (axisName[0] == '\0') {
        return true;
    }

    if (textEquals(axisName, "UNNAMED")) {
        return true;
    }

    return false;
}

bool AdvanceMotionController::validateAxisNames() const {
    for (uint i = 0; i < axisCount; i++) {
        const char* currentName =
            axes[i]->getAxisName();

        if (isReservedAxisName(currentName)) {
            return false;
        }

        for (uint j = i + 1; j < axisCount; j++) {
            const char* compareName =
                axes[j]->getAxisName();

            if (textEquals(currentName, compareName)) {
                return false;
            }
        }
    }

    return true;
}

bool AdvanceMotionController::normalizeAxisName(
    const char* inputName,
    char outputName[],
    uint outputSize
) const {
    if (inputName == nullptr) {
        return false;
    }

    if (outputName == nullptr) {
        return false;
    }

    if (outputSize == 0) {
        return false;
    }

    uint writeIndex = 0;
    bool hasValidCharacter = false;

    for (
        uint readIndex = 0;
        inputName[readIndex] != '\0' &&
        writeIndex < (outputSize - 1);
        readIndex++
    ) {
        char currentChar =
            inputName[readIndex];

        if (
            currentChar == ' ' ||
            currentChar == '\t' ||
            currentChar == '\r' ||
            currentChar == '\n'
        ) {
            continue;
        }

        if (currentChar >= 'a' && currentChar <= 'z') {
            currentChar =
                currentChar - ('a' - 'A');
        }

        outputName[writeIndex] =
            currentChar;

        hasValidCharacter = true;
        writeIndex++;
    }

    outputName[writeIndex] = '\0';

    return hasValidCharacter;
}

bool AdvanceMotionController::findAxisIndexByName(
    const char* axisName,
    uint& outputAxisIndex
) const {
    char normalizedName[StepperAxisConfig::AXIS_NAME_MAX_LENGTH];

    if (!normalizeAxisName(
            axisName,
            normalizedName,
            StepperAxisConfig::AXIS_NAME_MAX_LENGTH
        )) {
        return false;
    }

    for (uint i = 0; i < axisCount; i++) {
        if (textEquals(
                axes[i]->getAxisName(),
                normalizedName
            )) {
            outputAxisIndex = i;
            return true;
        }
    }

    return false;
}

void AdvanceMotionController::init() {
    if (!validController) {
        return;
    }

    if (pulseEngine != nullptr) {
        pulseEngine->init();
    }

    for (uint i = 0; i < axisCount; i++) {
        axes[i]->init();
    }
}

bool AdvanceMotionController::isValid() const {
    return validController;
}

uint AdvanceMotionController::getAxisCount() const {
    return axisCount;
}

const char* AdvanceMotionController::getAxisName(
    uint axisIndex
) const {
    if (axisIndex >= axisCount) {
        return "INVALID";
    }

    if (axes[axisIndex] == nullptr) {
        return "NULL";
    }

    return axes[axisIndex]->getAxisName();
}

void AdvanceMotionController::printAxisMap() const {
    printf("\n===== AXIS MAP =====\n");

    if (!validController) {
        printf("Controller invalid.\n");
        printf("====================\n\n");
        return;
    }

    for (uint i = 0; i < axisCount; i++) {
        printf(
            "Index [%u] = Axis %s | STEP GPIO %u | DIR GPIO %u | ENABLE GPIO %u\n",
            i,
            axes[i]->getAxisName(),
            axes[i]->getStepPin(),
            axes[i]->getDirPin(),
            axes[i]->getEnablePin()
        );
    }

    printf("====================\n\n");
}

void AdvanceMotionController::printHomeOrder() const {
    printf("\n===== HOME ORDER =====\n");

    if (!homeOrderConfigured) {
        printf("Home order not configured.\n");
        printf("======================\n\n");
        return;
    }

    for (uint i = 0; i < homeOrderCount; i++) {
        uint axisIndex =
            homeOrder[i];

        printf(
            "Home step %u: Axis index [%u] = %s\n",
            i + 1,
            axisIndex,
            getAxisName(axisIndex)
        );
    }

    printf("======================\n\n");
}

float AdvanceMotionController::calculateDistanceFromDeltas(
    const float deltas[],
    uint count
) {
    float sum = 0.0f;

    for (uint i = 0; i < count; i++) {
        sum += deltas[i] * deltas[i];
    }

    return sqrtf(sum);
}

bool AdvanceMotionController::buildLinearBlockToArray(
    const float targetPositionsMm[],
    uint targetCount,
    float requestedSpeedMmS,
    float entrySpeedMmS,
    float exitSpeedMmS,
    MotionBlock& block
) {
    block.reset();

    if (!validController) {
        return false;
    }

    if (targetPositionsMm == nullptr) {
        return false;
    }

    if (targetCount != axisCount) {
        return false;
    }

    if (requestedSpeedMmS <= 0.0f) {
        return false;
    }

    block.axisCount = axisCount;
    block.requestedSpeedMmS = requestedSpeedMmS;
    block.entrySpeedMmS = entrySpeedMmS;
    block.exitSpeedMmS = exitSpeedMmS;

    uint32_t dominantSteps = 0;

    for (uint i = 0; i < axisCount; i++) {
        float currentMm =
            axes[i]->getCurrentPositionMm();

        float targetMm =
            targetPositionsMm[i];

        if (!axes[i]->isTargetWithinSoftLimits(targetMm)) {
            return false;
        }

        block.deltaMm[i] =
            targetMm - currentMm;

        int32_t targetSteps =
            axes[i]->mmToSteps(targetMm);

        int32_t currentSteps =
            axes[i]->getCurrentPositionSteps();

        block.deltaSteps[i] =
            targetSteps - currentSteps;

        block.direction[i] =
            block.deltaSteps[i] >= 0;

        if (block.deltaSteps[i] >= 0) {
            block.absSteps[i] =
                (uint32_t)block.deltaSteps[i];
        } else {
            block.absSteps[i] =
                (uint32_t)(-block.deltaSteps[i]);
        }

        if (block.absSteps[i] > dominantSteps) {
            dominantSteps =
                block.absSteps[i];
        }
    }

    block.dominantSteps =
        dominantSteps;

    if (block.dominantSteps == 0) {
        return false;
    }

    block.distanceMm =
        calculateDistanceFromDeltas(
            block.deltaMm,
            axisCount
        );

    if (block.distanceMm <= 0.0f) {
        return false;
    }

    float limitedSpeed =
        limitSpeedByAxisLimits(
            block,
            requestedSpeedMmS
        );

    limitedSpeed =
        limitSpeedByPulseEngine(
            block,
            limitedSpeed
        );

    block.nominalSpeedMmS =
        limitedSpeed;

    block.accelerationMmS2 =
        limitAccelerationByAxisLimits(block);

    if (block.nominalSpeedMmS <= 0.0f) {
        return false;
    }

    if (block.accelerationMmS2 <= 0.0f) {
        return false;
    }

    block.valid = true;

    return true;
}

bool AdvanceMotionController::buildLinearBlockTo(
    std::initializer_list<float> targetPositionsMm,
    float requestedSpeedMmS,
    MotionBlock& block
) {
    if (targetPositionsMm.size() != axisCount) {
        return false;
    }

    float targetArray[MAX_AXES];

    uint index = 0;

    for (float value : targetPositionsMm) {
        targetArray[index] = value;
        index++;
    }

    return buildLinearBlockToArray(
        targetArray,
        axisCount,
        requestedSpeedMmS,
        0.0f,
        0.0f,
        block
    );
}

float AdvanceMotionController::limitSpeedByAxisLimits(
    const MotionBlock& block,
    float requestedSpeedMmS
) {
    float limitedSpeed =
        requestedSpeedMmS;

    for (uint i = 0; i < axisCount; i++) {
        if (block.absSteps[i] == 0) {
            continue;
        }

        float axisDistanceMm =
            fabsf(block.deltaMm[i]);

        if (axisDistanceMm <= 0.0f) {
            continue;
        }

        float axisRatio =
            axisDistanceMm /
            block.distanceMm;

        if (axisRatio <= 0.0f) {
            continue;
        }

        float maxPathSpeedByAxis =
            axes[i]->getMaxSpeedMmS() /
            axisRatio;

        if (maxPathSpeedByAxis < limitedSpeed) {
            limitedSpeed =
                maxPathSpeedByAxis;
        }
    }

    return limitedSpeed;
}

float AdvanceMotionController::limitSpeedByPulseEngine(
    const MotionBlock& block,
    float currentLimitedSpeed
) {
    if (pulseEngine == nullptr) {
        return currentLimitedSpeed;
    }

    if (block.distanceMm <= 0.0f) {
        return currentLimitedSpeed;
    }

    if (block.dominantSteps == 0) {
        return currentLimitedSpeed;
    }

    float maxEventsPerSecond =
        pulseEngine->getMaxStepEventsPerSecond();

    float maxPathSpeedByPulse =
        (
            maxEventsPerSecond *
            block.distanceMm
        ) /
        (float)block.dominantSteps;

    if (maxPathSpeedByPulse < currentLimitedSpeed) {
        return maxPathSpeedByPulse;
    }

    return currentLimitedSpeed;
}

float AdvanceMotionController::limitAccelerationByAxisLimits(
    const MotionBlock& block
) {
    float limitedAcceleration =
        1000000.0f;

    for (uint i = 0; i < axisCount; i++) {
        if (block.absSteps[i] == 0) {
            continue;
        }

        float axisDistanceMm =
            fabsf(block.deltaMm[i]);

        if (axisDistanceMm <= 0.0f) {
            continue;
        }

        float axisRatio =
            axisDistanceMm /
            block.distanceMm;

        if (axisRatio <= 0.0f) {
            continue;
        }

        float maxPathAccelerationByAxis =
            axes[i]->getAccelerationMmS2() /
            axisRatio;

        if (maxPathAccelerationByAxis < limitedAcceleration) {
            limitedAcceleration =
                maxPathAccelerationByAxis;
        }
    }

    if (limitedAcceleration == 1000000.0f) {
        limitedAcceleration = 1.0f;
    }

    return limitedAcceleration;
}

void AdvanceMotionController::prepareDirections(
    const MotionBlock& block
) {
    for (uint i = 0; i < axisCount; i++) {
        if (block.absSteps[i] == 0) {
            continue;
        }

        axes[i]->setDirection(
            block.direction[i]
        );
    }
}

void AdvanceMotionController::prepareBresenham(
    const MotionBlock& block
) {
    for (uint i = 0; i < axisCount; i++) {
        bresenhamError[i] = 0;
    }

    dominantStepCounter = 0;
}

bool AdvanceMotionController::startMotionBlock(
    const MotionBlock& block
) {
    if (!validController) {
        return false;
    }

    if (!block.valid) {
        return false;
    }

    if (busy) {
        return false;
    }

    currentBlock =
        block;

    currentProfile =
        TrapezoidProfile::calculate(
            currentBlock.dominantSteps,
            currentBlock.distanceMm,
            currentBlock.nominalSpeedMmS,
            currentBlock.accelerationMmS2,
            currentBlock.entrySpeedMmS,
            currentBlock.exitSpeedMmS
        );

    if (!currentProfile.valid) {
        return false;
    }

    prepareDirections(currentBlock);
    prepareBresenham(currentBlock);

    nextStepTime =
        get_absolute_time();

    busy = true;

    return true;
}

bool AdvanceMotionController::startPlannedLinearMove(
    const PlannedLinearMove& plannedMove
) {
    if (!plannedMove.valid) {
        return false;
    }

    MotionBlock block;

    if (!buildLinearBlockToArray(
            plannedMove.targetPositionMm,
            plannedMove.axisCount,
            plannedMove.requestedSpeedMmS,
            plannedMove.entrySpeedMmS,
            plannedMove.exitSpeedMmS,
            block
        )) {
        return false;
    }

    return startMotionBlock(block);
}

bool AdvanceMotionController::moveLinearTo(
    std::initializer_list<float> targetPositionsMm,
    float requestedSpeedMmS
) {
    if (busy) {
        return false;
    }

    MotionBlock block;

    if (!buildLinearBlockTo(
            targetPositionsMm,
            requestedSpeedMmS,
            block
        )) {
        return false;
    }

    return startMotionBlock(block);
}

void AdvanceMotionController::configurePathSmoothing(
    bool enabled,
    float blendDistanceMm,
    float blendSegmentLengthMm,
    uint maxBlendSegments,
    float blendSpeedScale
) {
    planner.setBlendEnabled(enabled);

    planner.setBlendParameters(
        blendDistanceMm,
        blendSegmentLengthMm,
        maxBlendSegments,
        blendSpeedScale
    );
}

void AdvanceMotionController::clearMotionQueue() {
    planner.clear();
}

bool AdvanceMotionController::queueLinearTo(
    std::initializer_list<float> targetPositionsMm,
    float requestedSpeedMmS
) {
    return planner.pushLinearTo(
        targetPositionsMm,
        requestedSpeedMmS
    );
}

bool AdvanceMotionController::prepareQueuedMoves() {
    float currentPositions[MAX_AXES];

    for (uint i = 0; i < axisCount; i++) {
        currentPositions[i] =
            axes[i]->getCurrentPositionMm();
    }

    planner.setStartPosition(
        currentPositions,
        axisCount
    );

    return planner.buildPlannedQueue();
}

uint AdvanceMotionController::getRawQueuedMoveCount() const {
    return planner.getRawMoveCount();
}

uint AdvanceMotionController::getPlannedMoveCount() const {
    return planner.getPlannedMoveCount();
}

void AdvanceMotionController::loadNextPlannedMoveIfAvailable() {
    if (busy) {
        return;
    }

    PlannedLinearMove plannedMove;

    if (!planner.popPlannedMove(plannedMove)) {
        return;
    }

    startPlannedLinearMove(plannedMove);
}

uint32_t AdvanceMotionController::calculateCurrentStepDelayUs() {
    if (currentBlock.dominantSteps == 0) {
        return pulseEngine->getMinimumStepIntervalUs();
    }

    if (currentBlock.distanceMm <= 0.0f) {
        return pulseEngine->getMinimumStepIntervalUs();
    }

    float distancePerDominantStep =
        currentBlock.distanceMm /
        (float)currentBlock.dominantSteps;

    float traveledDistanceMm =
        distancePerDominantStep *
        (float)dominantStepCounter;

    float speedMmS =
        currentProfile.reachableSpeedMmS;

    float entrySquared =
        currentProfile.entrySpeedMmS *
        currentProfile.entrySpeedMmS;

    float exitSquared =
        currentProfile.exitSpeedMmS *
        currentProfile.exitSpeedMmS;

    if (dominantStepCounter < currentProfile.accelerationSteps) {
        float speedSquared =
            entrySquared +
            2.0f *
            currentProfile.accelerationMmS2 *
            traveledDistanceMm;

        speedMmS =
            sqrtf(speedSquared);
    } else if (
        dominantStepCounter >=
        (
            currentProfile.accelerationSteps +
            currentProfile.cruiseSteps
        )
    ) {
        float remainingDistanceMm =
            currentBlock.distanceMm -
            traveledDistanceMm;

        if (remainingDistanceMm < 0.0f) {
            remainingDistanceMm = 0.0f;
        }

        float speedSquared =
            exitSquared +
            2.0f *
            currentProfile.accelerationMmS2 *
            remainingDistanceMm;

        speedMmS =
            sqrtf(speedSquared);
    } else {
        speedMmS =
            currentProfile.reachableSpeedMmS;
    }

    if (speedMmS < 0.1f) {
        speedMmS = 0.1f;
    }

    float stepEventsPerSecond =
        (
            speedMmS *
            (float)currentBlock.dominantSteps
        ) /
        currentBlock.distanceMm;

    if (stepEventsPerSecond <= 0.0f) {
        return pulseEngine->getMinimumStepIntervalUs();
    }

    uint32_t delayUs =
        (uint32_t)(
            1000000.0f /
            stepEventsPerSecond
        );

    if (delayUs < pulseEngine->getMinimumStepIntervalUs()) {
        delayUs =
            pulseEngine->getMinimumStepIntervalUs();
    }

    return delayUs;
}

bool AdvanceMotionController::isHardwareLimitBlockingDirection(
    uint axisIndex,
    bool positiveDirection
) const {
    if (axisIndex >= axisCount) {
        return true;
    }

    AdvanceStepperAxis* axis =
        axes[axisIndex];

    if (positiveDirection) {
        if (axis->isMaxLimitEnabled() && axis->isMaxLimitActive()) {
            return true;
        }
    } else {
        if (axis->isMinLimitEnabled() && axis->isMinLimitActive()) {
            return true;
        }
    }

    return false;
}

void AdvanceMotionController::executeBresenhamStep() {
    if (!busy) {
        return;
    }

    if (dominantStepCounter >= currentBlock.dominantSteps) {
        busy = false;
        currentBlock.reset();
        loadNextPlannedMoveIfAvailable();
        return;
    }

    uint32_t stepMask = 0;

    int32_t positionDelta[MAX_AXES];

    for (uint i = 0; i < MAX_AXES; i++) {
        positionDelta[i] = 0;
    }

    for (uint i = 0; i < axisCount; i++) {
        if (currentBlock.absSteps[i] == 0) {
            continue;
        }

        bresenhamError[i] +=
            (int32_t)currentBlock.absSteps[i];

        if (bresenhamError[i] >= (int32_t)currentBlock.dominantSteps) {
            bresenhamError[i] -=
                (int32_t)currentBlock.dominantSteps;

            bool positiveDirection =
                currentBlock.direction[i];

            if (isHardwareLimitBlockingDirection(
                    i,
                    positiveDirection
                )) {
                emergencyStop();
                return;
            }

            uint stepPin =
                axes[i]->getStepPin();

            uint pinOffset =
                pulseEngine->getPinOffset(stepPin);

            stepMask |=
                (1u << pinOffset);

            if (positiveDirection) {
                positionDelta[i] = 1;
            } else {
                positionDelta[i] = -1;
            }
        }
    }

    if (stepMask != 0) {
        pulseEngine->sendStepMaskBlocking(stepMask);

        for (uint i = 0; i < axisCount; i++) {
            if (positionDelta[i] != 0) {
                axes[i]->addPositionSteps(
                    positionDelta[i]
                );
            }
        }
    }

    dominantStepCounter++;

    if (dominantStepCounter >= currentBlock.dominantSteps) {
        busy = false;
        currentBlock.reset();
        loadNextPlannedMoveIfAvailable();
    }
}

void AdvanceMotionController::update() {
    if (!validController) {
        return;
    }

    if (!busy) {
        loadNextPlannedMoveIfAvailable();
        return;
    }

    absolute_time_t now =
        get_absolute_time();

    if (absolute_time_diff_us(now, nextStepTime) <= 0) {
        executeBresenhamStep();

        uint32_t delayUs =
            calculateCurrentStepDelayUs();

        nextStepTime =
            delayed_by_us(
                nextStepTime,
                delayUs
            );
    }
}

bool AdvanceMotionController::isBusy() const {
    return busy;
}

void AdvanceMotionController::stop() {
    busy = false;
    currentBlock.reset();

    planner.clear();
}

void AdvanceMotionController::emergencyStop() {
    stop();

    for (uint i = 0; i < axisCount; i++) {
        axes[i]->disable();
    }
}

void AdvanceMotionController::setCurrentPosition(
    std::initializer_list<float> currentPositionsMm
) {
    if (currentPositionsMm.size() != axisCount) {
        return;
    }

    uint index = 0;

    for (float positionMm : currentPositionsMm) {
        axes[index]->setCurrentPositionMm(positionMm);
        index++;
    }
}

bool AdvanceMotionController::getCurrentPosition(
    float outputPositionsMm[],
    uint outputCount
) const {
    if (outputPositionsMm == nullptr) {
        return false;
    }

    if (outputCount != axisCount) {
        return false;
    }

    for (uint i = 0; i < axisCount; i++) {
        outputPositionsMm[i] =
            axes[i]->getCurrentPositionMm();
    }

    return true;
}

uint32_t AdvanceMotionController::speedToStepDelayUs(
    uint axisIndex,
    float speedMmS
) const {
    if (axisIndex >= axisCount) {
        return 1000;
    }

    if (speedMmS <= 0.0f) {
        return 1000;
    }

    float stepsPerSecond =
        speedMmS *
        axes[axisIndex]->getStepsPerMm();

    if (stepsPerSecond <= 0.0f) {
        return 1000;
    }

    uint32_t delayUs =
        (uint32_t)(
            1000000.0f /
            stepsPerSecond
        );

    if (delayUs < pulseEngine->getMinimumStepIntervalUs()) {
        delayUs =
            pulseEngine->getMinimumStepIntervalUs();
    }

    return delayUs;
}

bool AdvanceMotionController::stepAxisOnceBlocking(
    uint axisIndex,
    bool positiveDirection,
    uint32_t delayUs
) {
    if (axisIndex >= axisCount) {
        return false;
    }

    if (isHardwareLimitBlockingDirection(
            axisIndex,
            positiveDirection
        )) {
        emergencyStop();
        return false;
    }

    axes[axisIndex]->setDirection(
        positiveDirection
    );

    uint stepPin =
        axes[axisIndex]->getStepPin();

    uint pinOffset =
        pulseEngine->getPinOffset(stepPin);

    uint32_t stepMask =
        (1u << pinOffset);

    pulseEngine->sendStepMaskBlocking(stepMask);

    if (positiveDirection) {
        axes[axisIndex]->addPositionSteps(1);
    } else {
        axes[axisIndex]->addPositionSteps(-1);
    }

    sleep_us(delayUs);

    return true;
}

bool AdvanceMotionController::moveAxisRelativeBlocking(
    uint axisIndex,
    float distanceMm,
    float speedMmS
) {
    if (axisIndex >= axisCount) {
        return false;
    }

    if (speedMmS <= 0.0f) {
        return false;
    }

    int32_t stepsToMove =
        axes[axisIndex]->mmToSteps(
            fabsf(distanceMm)
        );

    if (stepsToMove <= 0) {
        return true;
    }

    bool positiveDirection =
        distanceMm >= 0.0f;

    uint32_t delayUs =
        speedToStepDelayUs(
            axisIndex,
            speedMmS
        );

    for (int32_t step = 0; step < stepsToMove; step++) {
        if (!stepAxisOnceBlocking(
                axisIndex,
                positiveDirection,
                delayUs
            )) {
            return false;
        }
    }

    return true;
}

bool AdvanceMotionController::setHomeOrder(
    std::initializer_list<uint> orderedAxisIndexes
) {
    if (!validController) {
        return false;
    }

    if (orderedAxisIndexes.size() == 0) {
        return false;
    }

    if (orderedAxisIndexes.size() > axisCount) {
        return false;
    }

    bool axisAlreadyAdded[MAX_AXES];

    for (uint i = 0; i < MAX_AXES; i++) {
        axisAlreadyAdded[i] = false;
        homeOrder[i] = 0;
    }

    uint index = 0;

    for (uint axisIndex : orderedAxisIndexes) {
        if (axisIndex >= axisCount) {
            return false;
        }

        if (axisAlreadyAdded[axisIndex]) {
            return false;
        }

        homeOrder[index] =
            axisIndex;

        axisAlreadyAdded[axisIndex] =
            true;

        index++;
    }

    homeOrderCount = index;
    homeOrderConfigured = true;

    return true;
}

bool AdvanceMotionController::setHomeOrderByName(
    std::initializer_list<const char*> orderedAxisNames
) {
    if (!validController) {
        return false;
    }

    if (orderedAxisNames.size() == 0) {
        return false;
    }

    if (orderedAxisNames.size() > axisCount) {
        return false;
    }

    bool axisAlreadyAdded[MAX_AXES];

    for (uint i = 0; i < MAX_AXES; i++) {
        axisAlreadyAdded[i] = false;
        homeOrder[i] = 0;
    }

    uint orderIndex = 0;

    for (const char* axisName : orderedAxisNames) {
        uint foundAxisIndex = 0;

        if (!findAxisIndexByName(
                axisName,
                foundAxisIndex
            )) {
            return false;
        }

        if (axisAlreadyAdded[foundAxisIndex]) {
            return false;
        }

        homeOrder[orderIndex] =
            foundAxisIndex;

        axisAlreadyAdded[foundAxisIndex] =
            true;

        orderIndex++;
    }

    homeOrderCount = orderIndex;
    homeOrderConfigured = true;

    return true;
}

void AdvanceMotionController::clearHomeOrder() {
    for (uint i = 0; i < MAX_AXES; i++) {
        homeOrder[i] = 0;
    }

    homeOrderCount = 0;
    homeOrderConfigured = false;
}

bool AdvanceMotionController::homeAxis(
    uint axisIndex
) {
    if (!validController) {
        return false;
    }

    if (busy) {
        return false;
    }

    if (axisIndex >= axisCount) {
        return false;
    }

    AdvanceStepperAxis* axis =
        axes[axisIndex];

    if (!axis->isHomeEnabled()) {
        return false;
    }

    axis->enable();

    if (
        axis->isHomeSimulationEnabled() &&
        axis->isHomeSensorActive()
    ) {
        axis->setCurrentPositionMm(
            axis->getHomePositionMm()
        );

        axis->setHomed(true);

        return true;
    }

    bool homeDirectionPositive =
        axis->getHomeDirectionPositive();

    uint32_t seekDelayUs =
        speedToStepDelayUs(
            axisIndex,
            axis->getHomingSeekSpeedMmS()
        );

    uint32_t latchDelayUs =
        speedToStepDelayUs(
            axisIndex,
            axis->getHomingLatchSpeedMmS()
        );

    int32_t maxTravelSteps =
        axis->mmToSteps(
            axis->getMaxHomingTravelMm()
        );

    if (maxTravelSteps < 0) {
        maxTravelSteps =
            -maxTravelSteps;
    }

    int32_t traveledSteps = 0;

    while (!axis->isHomeSensorActive()) {
        if (traveledSteps >= maxTravelSteps) {
            emergencyStop();
            return false;
        }

        if (!stepAxisOnceBlocking(
                axisIndex,
                homeDirectionPositive,
                seekDelayUs
            )) {
            return false;
        }

        traveledSteps++;
    }

    float backoffDistance =
        axis->getHomingBackoffMm();

    if (homeDirectionPositive) {
        backoffDistance =
            -backoffDistance;
    }

    if (!moveAxisRelativeBlocking(
            axisIndex,
            backoffDistance,
            axis->getHomingSeekSpeedMmS()
        )) {
        return false;
    }

    sleep_ms(100);

    traveledSteps = 0;

    while (!axis->isHomeSensorActive()) {
        if (traveledSteps >= maxTravelSteps) {
            emergencyStop();
            return false;
        }

        if (!stepAxisOnceBlocking(
                axisIndex,
                homeDirectionPositive,
                latchDelayUs
            )) {
            return false;
        }

        traveledSteps++;
    }

    axis->setCurrentPositionMm(
        axis->getHomePositionMm()
    );

    axis->setHomed(true);

    return true;
}

bool AdvanceMotionController::homeAll() {
    if (!validController) {
        return false;
    }

    if (busy) {
        return false;
    }

    if (!homeOrderConfigured) {
        return false;
    }

    if (homeOrderCount == 0) {
        return false;
    }

    for (uint i = 0; i < homeOrderCount; i++) {
        uint axisIndex =
            homeOrder[i];

        if (!homeAxis(axisIndex)) {
            return false;
        }
    }

    return true;
}

bool AdvanceMotionController::moveToSafePositionAfterHome(
    std::initializer_list<float> safePositionsMm,
    float speedMmS
) {
    if (!validController) {
        return false;
    }

    if (safePositionsMm.size() != axisCount) {
        return false;
    }

    for (uint i = 0; i < axisCount; i++) {
        if (!axes[i]->isHomed()) {
            return false;
        }
    }

    return moveLinearTo(
        safePositionsMm,
        speedMmS
    );
}
