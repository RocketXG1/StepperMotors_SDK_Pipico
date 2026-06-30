#include "RecipeLoader.h"

#include <stdio.h>

#include "DefaultRecipe.h"
#include "SdRecipeStorage.h"

const char* getRecipeLoadStatusMessage(RecipeLoadStatus status) {
    switch (status) {
        case RecipeLoadStatus::Ok:
            return "recipe loaded";
        case RecipeLoadStatus::DefaultRecipeInvalid:
            return "embedded default recipe is invalid";
        default:
            return "unknown recipe load status";
    }
}

RecipeLoadStatus loadRecipeFromMicroSdOrFallback(MachineRecipe& recipe) {
    static char recipeText[MACHINE_RECIPE_MAX_TEXT_LENGTH];

    SdRecipeStorage storage(SdSpiConfig::createDefault());
    MachineRecipeParser parser;

    printf("Loading machine recipe from microSD: %s\n", DEFAULT_RECIPE_PATH);

    if (storage.begin() && storage.readTextFile(DEFAULT_RECIPE_PATH, recipeText, sizeof(recipeText))) {
        storage.end();

        if (parser.parse(recipeText, recipe)) {
            printf("Recipe loaded from microSD. Axes: %u\n", recipe.axisCount);
            return RecipeLoadStatus::Ok;
        }

        printf("ERROR: microSD recipe is invalid: %s\n", parser.getLastError());
    } else {
        printf("WARNING: microSD recipe not available: %s\n", storage.getLastError());
        storage.end();
    }

    printf("Using embedded default recipe as a safe demonstration fallback.\n");

    if (!parser.parse(DEFAULT_MACHINE_RECIPE_TEXT, recipe)) {
        printf("ERROR: embedded default recipe is invalid: %s\n", parser.getLastError());
        return RecipeLoadStatus::DefaultRecipeInvalid;
    }

    return RecipeLoadStatus::Ok;
}
