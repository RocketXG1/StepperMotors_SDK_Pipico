#ifndef MOTION_SYSTEM_INITIALIZER_H
#define MOTION_SYSTEM_INITIALIZER_H

#include "AdvanceMotionController.h"
#include "AdvanceStepperAxis.h"
#include "MachineRecipe.h"
#include "StepperPioPulseEngine.h"

enum class MotionSystemStatus {
    Ok,
    InvalidAxisCount
};

struct MotionSystem {
    alignas(AdvanceStepperAxis) unsigned char axisStorage[MACHINE_RECIPE_MAX_AXES][sizeof(AdvanceStepperAxis)];
    AdvanceStepperAxis* axes[MACHINE_RECIPE_MAX_AXES] = {};

    alignas(AdvanceMotionController) unsigned char motionStorage[sizeof(AdvanceMotionController)];
    AdvanceMotionController* motion = nullptr;
};

const char* getMotionSystemStatusMessage(MotionSystemStatus status);

MotionSystemStatus initializeMotionSystem(
    const MachineRecipe& recipe,
    StepperPioPulseEngine& pulseEngine,
    MotionSystem& system
);

#endif
