#ifndef STEPPER_AXIS_CONFIG_H
#define STEPPER_AXIS_CONFIG_H

#include "pico/stdlib.h"

enum class AxisMechanismType {
    LEAD_SCREW,
    BELT_DRIVE
};

struct StepperAxisConfig {
    static const uint PIN_NOT_USED = 255;
    static const uint AXIS_NAME_MAX_LENGTH = 16;

    /*
        Nombre obligatorio del eje.

        Ejemplo:
        "X", "Y", "Z", "A", "ROTARY", etc.

        setAxisName() convierte automáticamente el nombre a MAYÚSCULAS.
        El controlador rechazará el nombre por defecto "UNNAMED".
    */
    char axisName[AXIS_NAME_MAX_LENGTH];

    uint stepPin;
    uint dirPin;
    uint enablePin;

    bool enableActiveLow;
    bool directionInvert;

    uint motorStepsPerRev;
    uint microsteps;

    AxisMechanismType mechanismType;

    float travelPerRevolutionMm;
    float gearRatio;

    float maxSpeedMmS;
    float accelerationMmS2;

    /*
        HOME.

        homeDirectionPositive:
        true  = busca HOME hacia dirección positiva.
        false = busca HOME hacia dirección negativa.

        directionInvert:
        Invierte físicamente el pin DIR si el motor gira al revés.
    */
    bool homeEnabled;
    uint homePin;
    bool homeActiveLow;
    bool homeUsePullup;
    bool homeDirectionPositive;
    bool homeSimulationEnabled;

    float homingSeekSpeedMmS;
    float homingLatchSpeedMmS;
    float homingBackoffMm;
    float maxHomingTravelMm;
    float homePositionMm;

    /*
        Límites físicos.
    */
    bool minLimitEnabled;
    uint minLimitPin;
    bool minLimitActiveLow;
    bool minLimitUsePullup;

    bool maxLimitEnabled;
    uint maxLimitPin;
    bool maxLimitActiveLow;
    bool maxLimitUsePullup;

    /*
        Límites por software.
    */
    bool softLimitsEnabled;
    float softLimitMinMm;
    float softLimitMaxMm;

    /*
        Asigna nombre al eje.

        Parámetro:
        name:
        Texto con el nombre deseado del eje.

        Reglas:
        - Convierte minúsculas a MAYÚSCULAS.
        - Ignora espacios, tabuladores y saltos de línea.
        - Limita el nombre a AXIS_NAME_MAX_LENGTH - 1 caracteres.
    */
    bool setAxisName(const char* name) {
        if (name == nullptr) {
            return false;
        }

        uint writeIndex = 0;
        bool hasValidCharacter = false;

        for (
            uint readIndex = 0;
            name[readIndex] != '\0' &&
            writeIndex < (AXIS_NAME_MAX_LENGTH - 1);
            readIndex++
        ) {
            char currentChar = name[readIndex];

            if (
                currentChar == ' ' ||
                currentChar == '\t' ||
                currentChar == '\r' ||
                currentChar == '\n'
            ) {
                continue;
            }

            if (currentChar >= 'a' && currentChar <= 'z') {
                currentChar =
                    currentChar - ('a' - 'A');
            }

            axisName[writeIndex] =
                currentChar;

            hasValidCharacter = true;
            writeIndex++;
        }

        axisName[writeIndex] = '\0';

        return hasValidCharacter;
    }

    static StepperAxisConfig createBase() {
        StepperAxisConfig config = {};

        /*
            Nombre por defecto.

            Este nombre NO será aceptado por el controlador.
            Es obligatorio reemplazarlo usando:

            configX.setAxisName("X");
        */
        config.setAxisName("UNNAMED");

        config.stepPin = PIN_NOT_USED;
        config.dirPin = PIN_NOT_USED;
        config.enablePin = PIN_NOT_USED;

        config.enableActiveLow = true;
        config.directionInvert = false;

        config.motorStepsPerRev = 200;
        config.microsteps = 1;

        config.mechanismType = AxisMechanismType::BELT_DRIVE;
        config.travelPerRevolutionMm = 1.0f;
        config.gearRatio = 1.0f;

        config.maxSpeedMmS = 10.0f;
        config.accelerationMmS2 = 50.0f;

        config.homeEnabled = false;
        config.homePin = PIN_NOT_USED;
        config.homeActiveLow = true;
        config.homeUsePullup = true;
        config.homeDirectionPositive = false;
        config.homeSimulationEnabled = false;

        config.homingSeekSpeedMmS = 10.0f;
        config.homingLatchSpeedMmS = 2.0f;
        config.homingBackoffMm = 3.0f;
        config.maxHomingTravelMm = 300.0f;
        config.homePositionMm = 0.0f;

        config.minLimitEnabled = false;
        config.minLimitPin = PIN_NOT_USED;
        config.minLimitActiveLow = true;
        config.minLimitUsePullup = true;

        config.maxLimitEnabled = false;
        config.maxLimitPin = PIN_NOT_USED;
        config.maxLimitActiveLow = true;
        config.maxLimitUsePullup = true;

        config.softLimitsEnabled = false;
        config.softLimitMinMm = 0.0f;
        config.softLimitMaxMm = 100.0f;

        return config;
    }

    static StepperAxisConfig createLeadScrew(
        uint stepPin,
        uint dirPin,
        uint enablePin,
        bool enableActiveLow,
        bool directionInvert,
        uint motorStepsPerRev,
        uint microsteps,
        float screwPitchMm,
        float gearRatio,
        float maxSpeedMmS,
        float accelerationMmS2
    ) {
        StepperAxisConfig config = createBase();

        config.stepPin = stepPin;
        config.dirPin = dirPin;
        config.enablePin = enablePin;

        config.enableActiveLow = enableActiveLow;
        config.directionInvert = directionInvert;

        config.motorStepsPerRev = motorStepsPerRev;
        config.microsteps = microsteps;

        config.mechanismType = AxisMechanismType::LEAD_SCREW;
        config.travelPerRevolutionMm = screwPitchMm;

        if (gearRatio <= 0.0f) {
            gearRatio = 1.0f;
        }

        config.gearRatio = gearRatio;

        config.maxSpeedMmS = maxSpeedMmS;
        config.accelerationMmS2 = accelerationMmS2;

        return config;
    }

    static StepperAxisConfig createBeltDrive(
        uint stepPin,
        uint dirPin,
        uint enablePin,
        bool enableActiveLow,
        bool directionInvert,
        uint motorStepsPerRev,
        uint microsteps,
        float beltPitchMm,
        uint pulleyTeeth,
        float gearRatio,
        float maxSpeedMmS,
        float accelerationMmS2
    ) {
        StepperAxisConfig config = createBase();

        config.stepPin = stepPin;
        config.dirPin = dirPin;
        config.enablePin = enablePin;

        config.enableActiveLow = enableActiveLow;
        config.directionInvert = directionInvert;

        config.motorStepsPerRev = motorStepsPerRev;
        config.microsteps = microsteps;

        config.mechanismType = AxisMechanismType::BELT_DRIVE;
        config.travelPerRevolutionMm = beltPitchMm * (float)pulleyTeeth;

        if (gearRatio <= 0.0f) {
            gearRatio = 1.0f;
        }

        config.gearRatio = gearRatio;

        config.maxSpeedMmS = maxSpeedMmS;
        config.accelerationMmS2 = accelerationMmS2;

        return config;
    }
};

#endif
