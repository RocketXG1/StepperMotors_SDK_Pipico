#include "MachineRecipe.h"

#include <cstdlib>
#include <cstring>
#include <cstdio>

void MachineRecipe::reset() {
    version = 1;
    axisCount = 0;
    homeOrderCount = 0;

    for (uint axisIndex = 0; axisIndex < MACHINE_RECIPE_MAX_AXES; axisIndex++) {
        axes[axisIndex].enabled = false;
        axes[axisIndex].config = StepperAxisConfig::createBase();
        axes[axisIndex].beltPitchMm = 1.0f;
        axes[axisIndex].pulleyTeeth = 1;
    }

    for (uint orderIndex = 0; orderIndex < MACHINE_RECIPE_MAX_HOME_ORDER; orderIndex++) {
        homeOrder[orderIndex][0] = '\0';
    }
}

MachineRecipeParser::MachineRecipeParser() {
    clearError();
}

void MachineRecipeParser::clearError() {
    lastError[0] = '\0';
}

bool MachineRecipeParser::fail(const char* message) {
    std::snprintf(lastError, sizeof(lastError), "%s", message);
    return false;
}

const char* MachineRecipeParser::getLastError() const {
    return lastError;
}

void MachineRecipeParser::trim(char* text) {
    if (text == nullptr) {
        return;
    }

    char* start = text;
    while (*start == ' ' || *start == '\t' || *start == '\r' || *start == '\n') {
        start++;
    }

    if (start != text) {
        std::memmove(text, start, std::strlen(start) + 1);
    }

    size_t length = std::strlen(text);
    while (length > 0) {
        char current = text[length - 1];
        if (current != ' ' && current != '\t' && current != '\r' && current != '\n') {
            break;
        }
        text[length - 1] = '\0';
        length--;
    }
}

bool MachineRecipeParser::textEqualsIgnoreCase(const char* left, const char* right) {
    if (left == nullptr || right == nullptr) {
        return false;
    }

    uint index = 0;
    while (left[index] != '\0' && right[index] != '\0') {
        char leftChar = left[index];
        char rightChar = right[index];

        if (leftChar >= 'a' && leftChar <= 'z') {
            leftChar = leftChar - ('a' - 'A');
        }
        if (rightChar >= 'a' && rightChar <= 'z') {
            rightChar = rightChar - ('a' - 'A');
        }

        if (leftChar != rightChar) {
            return false;
        }

        index++;
    }

    return left[index] == '\0' && right[index] == '\0';
}

bool MachineRecipeParser::parseBool(const char* text, bool& outputValue) {
    if (textEqualsIgnoreCase(text, "true") || textEqualsIgnoreCase(text, "yes") || std::strcmp(text, "1") == 0) {
        outputValue = true;
        return true;
    }

    if (textEqualsIgnoreCase(text, "false") || textEqualsIgnoreCase(text, "no") || std::strcmp(text, "0") == 0) {
        outputValue = false;
        return true;
    }

    return false;
}

bool MachineRecipeParser::parseUint(const char* text, uint& outputValue) {
    char* end = nullptr;
    unsigned long parsed = std::strtoul(text, &end, 10);
    if (end == text || *end != '\0') {
        return false;
    }
    outputValue = (uint)parsed;
    return true;
}

bool MachineRecipeParser::parseFloat(const char* text, float& outputValue) {
    char* end = nullptr;
    float parsed = std::strtof(text, &end);
    if (end == text || *end != '\0') {
        return false;
    }
    outputValue = parsed;
    return true;
}

bool MachineRecipeParser::parseAxisSectionName(
    const char* sectionName,
    char outputAxisName[],
    uint outputSize
) {
    const char* prefix = "axis ";
    size_t prefixLength = std::strlen(prefix);

    if (std::strncmp(sectionName, prefix, prefixLength) != 0) {
        return false;
    }

    StepperAxisConfig temp = StepperAxisConfig::createBase();
    if (!temp.setAxisName(sectionName + prefixLength)) {
        return false;
    }

    std::snprintf(outputAxisName, outputSize, "%s", temp.axisName);
    return true;
}

void MachineRecipeParser::parseHomeOrder(
    const char* value,
    MachineRecipe& recipe
) {
    char buffer[160];
    std::snprintf(buffer, sizeof(buffer), "%s", value);

    recipe.homeOrderCount = 0;

    char* token = std::strtok(buffer, ",");
    while (token != nullptr && recipe.homeOrderCount < MACHINE_RECIPE_MAX_HOME_ORDER) {
        trim(token);
        StepperAxisConfig temp = StepperAxisConfig::createBase();
        if (temp.setAxisName(token)) {
            std::snprintf(
                recipe.homeOrder[recipe.homeOrderCount],
                StepperAxisConfig::AXIS_NAME_MAX_LENGTH,
                "%s",
                temp.axisName
            );
            recipe.homeOrderCount++;
        }
        token = std::strtok(nullptr, ",");
    }
}

bool MachineRecipeParser::applyAxisValue(
    MachineRecipeAxis& axis,
    const char* key,
    const char* value
) {
    bool boolValue = false;
    uint uintValue = 0;
    float floatValue = 0.0f;

    if (textEqualsIgnoreCase(key, "enabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid boolean value.");
        axis.enabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "mechanism")) {
        if (textEqualsIgnoreCase(value, "belt") || textEqualsIgnoreCase(value, "belt_drive")) {
            axis.config.mechanismType = AxisMechanismType::BELT_DRIVE;
        } else if (textEqualsIgnoreCase(value, "lead_screw") || textEqualsIgnoreCase(value, "leadscrew")) {
            axis.config.mechanismType = AxisMechanismType::LEAD_SCREW;
        } else {
            return fail("Invalid mechanism value.");
        }
    } else if (textEqualsIgnoreCase(key, "stepPin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid stepPin value.");
        axis.config.stepPin = uintValue;
    } else if (textEqualsIgnoreCase(key, "dirPin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid dirPin value.");
        axis.config.dirPin = uintValue;
    } else if (textEqualsIgnoreCase(key, "enablePin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid enablePin value.");
        axis.config.enablePin = uintValue;
    } else if (textEqualsIgnoreCase(key, "enableActiveLow")) {
        if (!parseBool(value, boolValue)) return fail("Invalid enableActiveLow value.");
        axis.config.enableActiveLow = boolValue;
    } else if (textEqualsIgnoreCase(key, "directionInvert")) {
        if (!parseBool(value, boolValue)) return fail("Invalid directionInvert value.");
        axis.config.directionInvert = boolValue;
    } else if (textEqualsIgnoreCase(key, "motorStepsPerRev")) {
        if (!parseUint(value, uintValue)) return fail("Invalid motorStepsPerRev value.");
        axis.config.motorStepsPerRev = uintValue;
    } else if (textEqualsIgnoreCase(key, "microsteps")) {
        if (!parseUint(value, uintValue)) return fail("Invalid microsteps value.");
        axis.config.microsteps = uintValue;
    } else if (textEqualsIgnoreCase(key, "travelPerRevolutionMm") || textEqualsIgnoreCase(key, "screwPitchMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid travel value.");
        axis.config.travelPerRevolutionMm = floatValue;
    } else if (textEqualsIgnoreCase(key, "beltPitchMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid beltPitchMm value.");
        axis.beltPitchMm = floatValue;
        axis.config.travelPerRevolutionMm = axis.beltPitchMm * (float)axis.pulleyTeeth;
    } else if (textEqualsIgnoreCase(key, "pulleyTeeth")) {
        if (!parseUint(value, uintValue)) return fail("Invalid pulleyTeeth value.");
        axis.pulleyTeeth = uintValue;
        axis.config.travelPerRevolutionMm = axis.beltPitchMm * (float)axis.pulleyTeeth;
    } else if (textEqualsIgnoreCase(key, "gearRatio")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid gearRatio value.");
        axis.config.gearRatio = floatValue;
    } else if (textEqualsIgnoreCase(key, "maxSpeedMmS")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid maxSpeedMmS value.");
        axis.config.maxSpeedMmS = floatValue;
    } else if (textEqualsIgnoreCase(key, "accelerationMmS2")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid accelerationMmS2 value.");
        axis.config.accelerationMmS2 = floatValue;
    } else if (textEqualsIgnoreCase(key, "homeEnabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid homeEnabled value.");
        axis.config.homeEnabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "homePin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid homePin value.");
        axis.config.homePin = uintValue;
    } else if (textEqualsIgnoreCase(key, "homeActiveLow")) {
        if (!parseBool(value, boolValue)) return fail("Invalid homeActiveLow value.");
        axis.config.homeActiveLow = boolValue;
    } else if (textEqualsIgnoreCase(key, "homeUsePullup")) {
        if (!parseBool(value, boolValue)) return fail("Invalid homeUsePullup value.");
        axis.config.homeUsePullup = boolValue;
    } else if (textEqualsIgnoreCase(key, "homeDirectionPositive")) {
        if (!parseBool(value, boolValue)) return fail("Invalid homeDirectionPositive value.");
        axis.config.homeDirectionPositive = boolValue;
    } else if (textEqualsIgnoreCase(key, "homeSimulationEnabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid homeSimulationEnabled value.");
        axis.config.homeSimulationEnabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "homingSeekSpeedMmS")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid homingSeekSpeedMmS value.");
        axis.config.homingSeekSpeedMmS = floatValue;
    } else if (textEqualsIgnoreCase(key, "homingLatchSpeedMmS")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid homingLatchSpeedMmS value.");
        axis.config.homingLatchSpeedMmS = floatValue;
    } else if (textEqualsIgnoreCase(key, "homingBackoffMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid homingBackoffMm value.");
        axis.config.homingBackoffMm = floatValue;
    } else if (textEqualsIgnoreCase(key, "maxHomingTravelMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid maxHomingTravelMm value.");
        axis.config.maxHomingTravelMm = floatValue;
    } else if (textEqualsIgnoreCase(key, "homePositionMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid homePositionMm value.");
        axis.config.homePositionMm = floatValue;
    } else if (textEqualsIgnoreCase(key, "minLimitEnabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid minLimitEnabled value.");
        axis.config.minLimitEnabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "minLimitPin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid minLimitPin value.");
        axis.config.minLimitPin = uintValue;
    } else if (textEqualsIgnoreCase(key, "minLimitActiveLow")) {
        if (!parseBool(value, boolValue)) return fail("Invalid minLimitActiveLow value.");
        axis.config.minLimitActiveLow = boolValue;
    } else if (textEqualsIgnoreCase(key, "minLimitUsePullup")) {
        if (!parseBool(value, boolValue)) return fail("Invalid minLimitUsePullup value.");
        axis.config.minLimitUsePullup = boolValue;
    } else if (textEqualsIgnoreCase(key, "maxLimitEnabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid maxLimitEnabled value.");
        axis.config.maxLimitEnabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "maxLimitPin")) {
        if (!parseUint(value, uintValue)) return fail("Invalid maxLimitPin value.");
        axis.config.maxLimitPin = uintValue;
    } else if (textEqualsIgnoreCase(key, "maxLimitActiveLow")) {
        if (!parseBool(value, boolValue)) return fail("Invalid maxLimitActiveLow value.");
        axis.config.maxLimitActiveLow = boolValue;
    } else if (textEqualsIgnoreCase(key, "maxLimitUsePullup")) {
        if (!parseBool(value, boolValue)) return fail("Invalid maxLimitUsePullup value.");
        axis.config.maxLimitUsePullup = boolValue;
    } else if (textEqualsIgnoreCase(key, "softLimitsEnabled")) {
        if (!parseBool(value, boolValue)) return fail("Invalid softLimitsEnabled value.");
        axis.config.softLimitsEnabled = boolValue;
    } else if (textEqualsIgnoreCase(key, "softLimitMinMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid softLimitMinMm value.");
        axis.config.softLimitMinMm = floatValue;
    } else if (textEqualsIgnoreCase(key, "softLimitMaxMm")) {
        if (!parseFloat(value, floatValue)) return fail("Invalid softLimitMaxMm value.");
        axis.config.softLimitMaxMm = floatValue;
    }

    return true;
}

bool MachineRecipeParser::validateRecipe(const MachineRecipe& recipe) {
    if (recipe.axisCount < 1) {
        return fail("Recipe must define at least one enabled axis.");
    }

    for (uint i = 0; i < recipe.axisCount; i++) {
        const StepperAxisConfig& config = recipe.axes[i].config;

        if (std::strcmp(config.axisName, "UNNAMED") == 0 || config.axisName[0] == '\0') {
            return fail("Recipe contains an unnamed axis.");
        }

        if (config.stepPin == StepperAxisConfig::PIN_NOT_USED ||
            config.dirPin == StepperAxisConfig::PIN_NOT_USED ||
            config.enablePin == StepperAxisConfig::PIN_NOT_USED) {
            return fail("Enabled axis is missing step, dir or enable pin.");
        }

        if (config.motorStepsPerRev == 0 || config.microsteps == 0) {
            return fail("Enabled axis has invalid motor steps or microsteps.");
        }

        if (config.travelPerRevolutionMm <= 0.0f || config.gearRatio <= 0.0f) {
            return fail("Enabled axis has invalid mechanical travel or gear ratio.");
        }

        if (config.maxSpeedMmS <= 0.0f || config.accelerationMmS2 <= 0.0f) {
            return fail("Enabled axis has invalid speed or acceleration.");
        }

        if (config.softLimitsEnabled && config.softLimitMinMm >= config.softLimitMaxMm) {
            return fail("Enabled axis has invalid software limits.");
        }

        for (uint j = i + 1; j < recipe.axisCount; j++) {
            if (std::strcmp(config.axisName, recipe.axes[j].config.axisName) == 0) {
                return fail("Recipe contains duplicated axis names.");
            }
            if (config.stepPin == recipe.axes[j].config.stepPin) {
                return fail("Recipe contains duplicated STEP pins.");
            }
        }
    }

    return true;
}

bool MachineRecipeParser::parse(
    const char* recipeText,
    MachineRecipe& outputRecipe
) {
    clearError();
    outputRecipe.reset();

    if (recipeText == nullptr || recipeText[0] == '\0') {
        return fail("Recipe text is empty.");
    }

    char line[240];
    char currentSection[48] = "";
    MachineRecipeAxis* currentAxis = nullptr;
    const char* cursor = recipeText;

    while (*cursor != '\0') {
        uint lineLength = 0;
        while (cursor[lineLength] != '\0' && cursor[lineLength] != '\n' && lineLength < sizeof(line) - 1) {
            line[lineLength] = cursor[lineLength];
            lineLength++;
        }
        line[lineLength] = '\0';

        while (cursor[lineLength] != '\0' && cursor[lineLength] != '\n') {
            lineLength++;
        }
        cursor += lineLength;
        if (*cursor == '\n') {
            cursor++;
        }

        char* comment = std::strchr(line, '#');
        if (comment != nullptr) {
            *comment = '\0';
        }
        comment = std::strchr(line, ';');
        if (comment != nullptr) {
            *comment = '\0';
        }

        trim(line);
        if (line[0] == '\0') {
            continue;
        }

        if (line[0] == '[') {
            char* sectionEnd = std::strchr(line, ']');
            if (sectionEnd == nullptr) {
                return fail("Invalid section header.");
            }
            *sectionEnd = '\0';
            std::snprintf(currentSection, sizeof(currentSection), "%s", line + 1);
            trim(currentSection);
            currentAxis = nullptr;

            char axisName[StepperAxisConfig::AXIS_NAME_MAX_LENGTH];
            if (parseAxisSectionName(currentSection, axisName, sizeof(axisName))) {
                if (outputRecipe.axisCount >= MACHINE_RECIPE_MAX_AXES) {
                    return fail("Recipe has more axes than supported.");
                }
                currentAxis = &outputRecipe.axes[outputRecipe.axisCount];
                currentAxis->enabled = true;
                currentAxis->config = StepperAxisConfig::createBase();
                currentAxis->beltPitchMm = 1.0f;
                currentAxis->pulleyTeeth = 1;
                currentAxis->config.setAxisName(axisName);
                outputRecipe.axisCount++;
            }
            continue;
        }

        char* equals = std::strchr(line, '=');
        if (equals == nullptr) {
            return fail("Invalid key-value line.");
        }

        *equals = '\0';
        char* key = line;
        char* value = equals + 1;
        trim(key);
        trim(value);

        if (textEqualsIgnoreCase(currentSection, "project")) {
            if (textEqualsIgnoreCase(key, "version")) {
                uint parsedVersion = 0;
                if (!parseUint(value, parsedVersion)) {
                    return fail("Invalid project version.");
                }
                outputRecipe.version = (int)parsedVersion;
            } else if (textEqualsIgnoreCase(key, "homeOrder")) {
                parseHomeOrder(value, outputRecipe);
            }
        } else if (currentAxis != nullptr) {
            if (!applyAxisValue(*currentAxis, key, value)) {
                return false;
            }
        }
    }

    return validateRecipe(outputRecipe);
}
