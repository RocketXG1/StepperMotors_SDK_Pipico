#include "pico/stdlib.h"
#include <stdio.h>
#include <cstring>

#include "StepperPioPulseEngine.h"
#include "AdvanceMotionController.h"
#include "MachineRecipe.h"
#include "MotionSystemInitializer.h"
#include "RecipeLoader.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("\nStarting single-axis simulated HOME example...\n");

    MachineRecipe recipe;
    RecipeLoadStatus recipeLoadStatus = loadRecipeFromMicroSdOrFallback(recipe);

    if (recipeLoadStatus != RecipeLoadStatus::Ok) {
        printf(
            "ERROR: Could not load machine recipe: %s\n",
            getRecipeLoadStatusMessage(recipeLoadStatus)
        );
        while (true) {
            tight_loop_contents();
        }
    }

    StepperPioPulseEngine pulseEngine(
        /* selectedPioBlock */      pio0,
        /* selectedStateMachine */  0,
        /* firstStepGpio */         2,
        /* minStepIntervalUs */     10
    );

    static MotionSystem motionSystem;
    MotionSystemStatus motionSystemStatus = initializeMotionSystem(recipe, pulseEngine, motionSystem);

    if (motionSystemStatus != MotionSystemStatus::Ok) {
        printf(
            "ERROR: Could not initialize motion system: %s\n",
            getMotionSystemStatusMessage(motionSystemStatus)
        );
        while (true) {
            tight_loop_contents();
        }
    }

    AdvanceMotionController& motion = *motionSystem.motion;

    const char* targetHomeAxisName = "Z";
    bool axisFound = false;
    uint targetAxisIndex = 0;

    for (uint axisIndex = 0; axisIndex < motion.getAxisCount(); axisIndex++) {
        if (std::strcmp(motion.getAxisName(axisIndex), targetHomeAxisName) == 0) {
            axisFound = true;
            targetAxisIndex = axisIndex;
            break;
        }
    }

    if (!axisFound) {
        printf("ERROR: Axis %s was not found in the loaded recipe.\n", targetHomeAxisName);
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    motion.init();
    motion.printAxisMap();

    if (!motionSystem.axes[targetAxisIndex]->isHomeSimulationEnabled()) {
        printf(
            "ERROR: Axis %s at index %u is not configured for HOME simulation.\n",
            targetHomeAxisName,
            targetAxisIndex
        );
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    motionSystem.axes[targetAxisIndex]->setSimulatedHomeActive(true);

    bool homeOk = motion.homeAxis(targetAxisIndex);

    if (!homeOk) {
        printf(
            "ERROR: Simulated HOME failed for axis %s at index %u.\n",
            targetHomeAxisName,
            targetAxisIndex
        );
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    printf(
        "Simulated HOME completed for axis %s at index %u.\n",
        targetHomeAxisName,
        targetAxisIndex
    );

    while (true) {
        tight_loop_contents();
    }
}
