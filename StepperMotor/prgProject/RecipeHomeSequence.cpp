#include "RecipeHomeSequence.h"

#include <stdio.h>

#include "HomeSequenceFunctionBlock.h"

int executeRecipeHomeSequence(
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
