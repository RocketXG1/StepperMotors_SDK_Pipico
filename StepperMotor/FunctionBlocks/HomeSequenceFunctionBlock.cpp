#include "HomeSequenceFunctionBlock.h"

#include "pico/stdlib.h"

#include <stdio.h>

int executeHomeSequence(
    AdvanceMotionController& motion,
    std::initializer_list<const char*> orderedAxisNames
) {
    HomeSequenceStep currentStep =
        HOME_STEP_VALIDATE_CONTROLLER;

    int homeResult =
        HOME_SEQUENCE_OK;

    while (
        currentStep != HOME_STEP_DONE &&
        homeResult == HOME_SEQUENCE_OK
    ) {
        switch (currentStep) {
            case HOME_STEP_VALIDATE_CONTROLLER:
                if (!motion.isValid()) {
                    printf("ERROR: Motion controller invalid.\n");
                    printf("Check axis names, duplicate names or STEP pins.\n");
                    homeResult =
                        HOME_SEQUENCE_ERROR_INVALID_CONTROLLER;
                    break;
                }

                currentStep =
                    HOME_STEP_PRINT_AXIS_MAP;
                break;

            case HOME_STEP_PRINT_AXIS_MAP:
                motion.printAxisMap();

                currentStep =
                    HOME_STEP_CONFIGURE_HOME_ORDER;
                break;

            case HOME_STEP_CONFIGURE_HOME_ORDER:
                if (!motion.setHomeOrderByName(orderedAxisNames)) {
                    printf("ERROR: Invalid home order.\n");
                    motion.emergencyStop();
                    homeResult =
                        HOME_SEQUENCE_ERROR_INVALID_ORDER;
                    break;
                }

                currentStep =
                    HOME_STEP_PRINT_HOME_ORDER;
                break;

            case HOME_STEP_PRINT_HOME_ORDER:
                motion.printHomeOrder();

                currentStep =
                    HOME_STEP_INITIALIZE_HARDWARE;
                break;

            case HOME_STEP_INITIALIZE_HARDWARE:
                motion.init();

                currentStep =
                    HOME_STEP_EXECUTE_HOME;
                break;

            case HOME_STEP_EXECUTE_HOME: {
                printf("Starting HOME...\n");

                bool homingOk =
                    motion.homeAll();

                if (!homingOk) {
                    printf("ERROR: HOME failed.\n");
                    motion.emergencyStop();
                    homeResult =
                        HOME_SEQUENCE_ERROR_HOMING_FAILED;
                    break;
                }

                printf("HOME completed.\n");

                currentStep =
                    HOME_STEP_DONE;
                break;
            }

            case HOME_STEP_DONE:
                break;

            default:
                motion.emergencyStop();
                homeResult =
                    HOME_SEQUENCE_ERROR_HOMING_FAILED;
                break;
        }
    }

    if (homeResult != HOME_SEQUENCE_OK) {
        printf(
            "ERROR: HOME sequence failed with code %d.\n",
            homeResult
        );

        while (true) {
            tight_loop_contents();
        }
    }

    return homeResult;
}
