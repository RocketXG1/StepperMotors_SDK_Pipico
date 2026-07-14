#include "pico/stdlib.h"
#include <stdio.h>

#include "StepperPioPulseEngine.h"
#include "AdvanceMotionController.h"
#include "MachineRecipe.h"
#include "MotionSystemInitializer.h"
#include "RecipeLoader.h"
#include "RecipeHomeSequence.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("\nStarting general simulated HOME example...\n");

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

    for (uint axisIndex = 0; axisIndex < motion.getAxisCount(); axisIndex++) {
        if (motionSystem.axes[axisIndex]->isHomeSimulationEnabled()) {
            motionSystem.axes[axisIndex]->setSimulatedHomeActive(true);
            printf(
                "Simulated HOME active for axis %s at index %u.\n",
                motion.getAxisName(axisIndex),
                axisIndex
            );
        } else {
            printf(
                "Axis %s at index %u is not configured for HOME simulation.\n",
                motion.getAxisName(axisIndex),
                axisIndex
            );
        }
    }

    int homeResult = executeRecipeHomeSequence(motion, recipe);

    if (homeResult != HOME_SEQUENCE_OK) {
        printf("ERROR: General simulated HOME failed with code %d.\n", homeResult);
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    printf("General simulated HOME completed.\n");

    while (true) {
        tight_loop_contents();
    }
}
