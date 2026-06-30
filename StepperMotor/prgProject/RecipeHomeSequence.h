#ifndef RECIPE_HOME_SEQUENCE_H
#define RECIPE_HOME_SEQUENCE_H

#include "AdvanceMotionController.h"
#include "MachineRecipe.h"

int executeRecipeHomeSequence(
    AdvanceMotionController& motion,
    const MachineRecipe& recipe
);

#endif
