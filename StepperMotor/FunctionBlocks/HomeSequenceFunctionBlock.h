#ifndef HOME_SEQUENCE_FUNCTION_BLOCK_H
#define HOME_SEQUENCE_FUNCTION_BLOCK_H

#include <initializer_list>

#include "AdvanceMotionController.h"

enum HomeSequenceResult {
    HOME_SEQUENCE_OK = 0,
    HOME_SEQUENCE_ERROR_INVALID_CONTROLLER = 1,
    HOME_SEQUENCE_ERROR_INVALID_ORDER = 2,
    HOME_SEQUENCE_ERROR_HOMING_FAILED = 3
};

enum HomeSequenceStep {
    HOME_STEP_VALIDATE_CONTROLLER = 0,
    HOME_STEP_PRINT_AXIS_MAP,
    HOME_STEP_CONFIGURE_HOME_ORDER,
    HOME_STEP_PRINT_HOME_ORDER,
    HOME_STEP_INITIALIZE_HARDWARE,
    HOME_STEP_EXECUTE_HOME,
    HOME_STEP_DONE
};

int executeHomeSequence(
    AdvanceMotionController& motion,
    std::initializer_list<const char*> orderedAxisNames
);

#endif
