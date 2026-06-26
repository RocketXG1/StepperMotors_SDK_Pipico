#include "pico/stdlib.h"
#include <new>
#include <stdio.h>

#include "StepperAxisConfig.h"
#include "AdvanceStepperAxis.h"
#include "StepperPioPulseEngine.h"
#include "AdvanceMotionController.h"
#include "HomeSequenceFunctionBlock.h"
#include "MachineRecipe.h"
#include "SdRecipeStorage.h"
#include "DefaultRecipe.h"

static bool loadRecipeFromMicroSdOrFallback(MachineRecipe& recipe) {
    static char recipeText[MACHINE_RECIPE_MAX_TEXT_LENGTH];

    SdRecipeStorage storage(SdSpiConfig::createDefault());
    MachineRecipeParser parser;

    printf("Loading machine recipe from microSD: %s\n", DEFAULT_RECIPE_PATH);

    if (storage.begin() && storage.readTextFile(DEFAULT_RECIPE_PATH, recipeText, sizeof(recipeText))) {
        storage.end();

        if (parser.parse(recipeText, recipe)) {
            printf("Recipe loaded from microSD. Axes: %u\n", recipe.axisCount);
            return true;
        }

        printf("ERROR: microSD recipe is invalid: %s\n", parser.getLastError());
    } else {
        printf("WARNING: microSD recipe not available: %s\n", storage.getLastError());
        storage.end();
    }

    printf("Using embedded default recipe as a safe demonstration fallback.\n");

    if (!parser.parse(DEFAULT_MACHINE_RECIPE_TEXT, recipe)) {
        printf("ERROR: embedded default recipe is invalid: %s\n", parser.getLastError());
        return false;
    }

    return true;
}

static int executeRecipeHomeSequence(
    AdvanceMotionController& motion,
    const MachineRecipe& recipe
) {
    switch (recipe.homeOrderCount) {
        case 1:
            return executeHomeSequence(motion, { recipe.homeOrder[0] });
        case 2:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1] });
        case 3:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2] });
        case 4:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2], recipe.homeOrder[3] });
        case 5:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2], recipe.homeOrder[3], recipe.homeOrder[4] });
        case 6:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2], recipe.homeOrder[3], recipe.homeOrder[4], recipe.homeOrder[5] });
        case 7:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2], recipe.homeOrder[3], recipe.homeOrder[4], recipe.homeOrder[5], recipe.homeOrder[6] });
        case 8:
            return executeHomeSequence(motion, { recipe.homeOrder[0], recipe.homeOrder[1], recipe.homeOrder[2], recipe.homeOrder[3], recipe.homeOrder[4], recipe.homeOrder[5], recipe.homeOrder[6], recipe.homeOrder[7] });
        default:
            printf("WARNING: recipe has no homeOrder. Using controller default HOME order.\n");
            motion.init();
            return motion.homeAll() ? HOME_SEQUENCE_OK : HOME_SEQUENCE_ERROR_HOMING_FAILED;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    printf("\nStarting motion project with microSD recipe loader...\n");

    MachineRecipe recipe;
    if (!loadRecipeFromMicroSdOrFallback(recipe)) {
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

    alignas(AdvanceStepperAxis) static unsigned char axisStorage[MACHINE_RECIPE_MAX_AXES][sizeof(AdvanceStepperAxis)];
    AdvanceStepperAxis* axes[MACHINE_RECIPE_MAX_AXES];

    for (uint axisIndex = 0; axisIndex < recipe.axisCount; axisIndex++) {
        axes[axisIndex] = new (axisStorage[axisIndex]) AdvanceStepperAxis(recipe.axes[axisIndex].config);
        printf(
            "Axis %u loaded: %s STEP=%u DIR=%u EN=%u\n",
            axisIndex,
            axes[axisIndex]->getAxisName(),
            axes[axisIndex]->getStepPin(),
            axes[axisIndex]->getDirPin(),
            axes[axisIndex]->getEnablePin()
        );
    }

    AdvanceMotionController motion(
        axes,
        recipe.axisCount,
        pulseEngine
    );

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
