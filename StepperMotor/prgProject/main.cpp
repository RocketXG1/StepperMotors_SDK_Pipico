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

    printf("\nStarting motion project with microSD recipe loader...\n");

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

    executeRecipeHomeSequence(motion, recipe);

    bool safeMoveOk =
        motion.moveToSafePositionAfterHome(
            {
                0.0f,
                0.0f,
                10.0f
            },
            /* speedMmS */ 20.0f
        );

    if (!safeMoveOk) {
        printf("ERROR: Safe move failed.\n");
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    while (motion.isBusy()) {
        motion.update();
    }

    printf("Safe position reached.\n");

    bool directMoveOk =
        motion.moveLinearTo(
            {
                50.0f,
                50.0f,
                10.0f
            },
            /* requestedSpeedMmS */ 30.0f
        );

    if (!directMoveOk) {
        printf("ERROR: Direct move failed.\n");
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    while (motion.isBusy()) {
        motion.update();
    }

    printf("Direct move completed.\n");

    motion.configurePathSmoothing(
        /* enabled */              true,
        /* blendDistanceMm */      2.0f,
        /* blendSegmentLengthMm */ 0.5f,
        /* maxBlendSegments */     12,
        /* blendSpeedScale */      0.70f
    );

    motion.clearMotionQueue();

    motion.queueLinearTo(
        {
            100.0f,
            0.0f,
            10.0f
        },
        /* requestedSpeedMmS */ 40.0f
    );

    motion.queueLinearTo(
        {
            100.0f,
            100.0f,
            10.0f
        },
        /* requestedSpeedMmS */ 40.0f
    );

    motion.queueLinearTo(
        {
            200.0f,
            100.0f,
            10.0f
        },
        /* requestedSpeedMmS */ 40.0f
    );

    bool queueOk = motion.prepareQueuedMoves();

    if (!queueOk) {
        printf("ERROR: Could not prepare movement queue.\n");
        motion.emergencyStop();
        while (true) {
            tight_loop_contents();
        }
    }

    printf("Movement queue ready.\n");

    while (true) {
        motion.update();
    }
}
