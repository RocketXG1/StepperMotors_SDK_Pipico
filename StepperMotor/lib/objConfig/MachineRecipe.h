#ifndef MACHINE_RECIPE_H
#define MACHINE_RECIPE_H

#include "pico/stdlib.h"
#include "StepperAxisConfig.h"

static const uint MACHINE_RECIPE_MAX_AXES = 8;
static const uint MACHINE_RECIPE_MAX_HOME_ORDER = 8;
static const uint MACHINE_RECIPE_MAX_TEXT_LENGTH = 8192;

struct MachineRecipeAxis {
    bool enabled;
    StepperAxisConfig config;
    float beltPitchMm;
    uint pulleyTeeth;
};

struct MachineRecipe {
    int version;
    MachineRecipeAxis axes[MACHINE_RECIPE_MAX_AXES];
    uint axisCount;
    char homeOrder[MACHINE_RECIPE_MAX_HOME_ORDER][StepperAxisConfig::AXIS_NAME_MAX_LENGTH];
    uint homeOrderCount;

    void reset();
};

class MachineRecipeParser {
private:
    char lastError[160];

    void clearError();
    bool fail(const char* message);
    static void trim(char* text);
    static bool textEqualsIgnoreCase(const char* left, const char* right);
    static bool parseBool(const char* text, bool& outputValue);
    static bool parseUint(const char* text, uint& outputValue);
    static bool parseFloat(const char* text, float& outputValue);
    static bool parseAxisSectionName(
        const char* sectionName,
        char outputAxisName[],
        uint outputSize
    );
    static void parseHomeOrder(
        const char* value,
        MachineRecipe& recipe
    );
    bool applyAxisValue(
        MachineRecipeAxis& axis,
        const char* key,
        const char* value
    );
    bool validateRecipe(const MachineRecipe& recipe);

public:
    MachineRecipeParser();

    bool parse(
        const char* recipeText,
        MachineRecipe& outputRecipe
    );

    const char* getLastError() const;
};

#endif
