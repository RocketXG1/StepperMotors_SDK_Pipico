#ifndef RECIPE_LOADER_H
#define RECIPE_LOADER_H

#include "MachineRecipe.h"

enum class RecipeLoadStatus {
    Ok,
    DefaultRecipeInvalid
};

const char* getRecipeLoadStatusMessage(RecipeLoadStatus status);

RecipeLoadStatus loadRecipeFromMicroSdOrFallback(MachineRecipe& recipe);

#endif
