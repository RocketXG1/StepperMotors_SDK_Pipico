#include "MotionSystemInitializer.h"

#include <new>
#include <stdio.h>

const char* getMotionSystemStatusMessage(MotionSystemStatus status) {
    switch (status) {
        case MotionSystemStatus::Ok:
            return "motion system initialized";
        case MotionSystemStatus::InvalidAxisCount:
            return "invalid recipe axis count";
        default:
            return "unknown motion system status";
    }
}

MotionSystemStatus initializeMotionSystem(
    const MachineRecipe& recipe,
    StepperPioPulseEngine& pulseEngine,
    MotionSystem& system
) {
    if (recipe.axisCount == 0 || recipe.axisCount > MACHINE_RECIPE_MAX_AXES) {
        printf(
            "ERROR: Invalid axis count in recipe: %u (max=%u)\n",
            recipe.axisCount,
            MACHINE_RECIPE_MAX_AXES
        );
        return MotionSystemStatus::InvalidAxisCount;
    }

    for (uint axisIndex = 0; axisIndex < recipe.axisCount; axisIndex++) {
        system.axes[axisIndex] = new (system.axisStorage[axisIndex])
            AdvanceStepperAxis(recipe.axes[axisIndex].config);
        printf(
            "Axis %u loaded: %s STEP=%u DIR=%u EN=%u\n",
            axisIndex,
            system.axes[axisIndex]->getAxisName(),
            system.axes[axisIndex]->getStepPin(),
            system.axes[axisIndex]->getDirPin(),
            system.axes[axisIndex]->getEnablePin()
        );
    }

    system.motion = new (system.motionStorage) AdvanceMotionController(
        system.axes,
        recipe.axisCount,
        pulseEngine
    );

    return MotionSystemStatus::Ok;
}
