#include "pico/stdlib.h"
#include <stdio.h>

#include "StepperAxisConfig.h"
#include "AdvanceStepperAxis.h"
#include "StepperPioPulseEngine.h"
#include "AdvanceMotionController.h"
#include "HomeSequenceFunctionBlock.h"

int main() {
    /*
        Inicializa entrada/salida estándar.

        Permite usar printf() por USB serial.
    */
    stdio_init_all();

    /*
        Pequeña espera para dar tiempo a abrir el monitor serial.
    */
    sleep_ms(2000);

    printf("\nStarting motion project...\n");

    /*
        ------------------------------------------------------------
        PIO PARA PULSOS STEP
        ------------------------------------------------------------

        GPIO 2 = STEP X
        GPIO 3 = STEP Y
        GPIO 4 = STEP Z

        Como el PIO está configurado para 8 pines STEP consecutivos:

        GPIO 2 a GPIO 9 quedan reservados como posibles pines STEP.
    */
    StepperPioPulseEngine pulseEngine(
        /* selectedPioBlock */      pio0,
        /* selectedStateMachine */  0,
        /* firstStepGpio */         2,
        /* minStepIntervalUs */     10
    );

    /*
        ------------------------------------------------------------
        CONFIGURACIÓN EJE X
        ------------------------------------------------------------
    */
    StepperAxisConfig configX =
        StepperAxisConfig::createBeltDrive(
            /* stepPin */            2,
            /* dirPin */             10,
            /* enablePin */          13,
            /* enableActiveLow */    true,
            /* directionInvert */    false,
            /* motorStepsPerRev */   200,
            /* microsteps */         16,
            /* beltPitchMm */        2.0f,
            /* pulleyTeeth */        20,
            /* gearRatio */          1.0f,
            /* maxSpeedMmS */        100.0f,
            /* accelerationMmS2 */   300.0f
        );

    /*
        Nombre obligatorio del eje.

        Aunque escribas "x", internamente se guarda como "X".
        No se permite dejar el nombre como "UNNAMED".
    */
    configX.setAxisName("X");

    /*
        HOME X.

        homeDirectionPositive = false:
        X buscará HOME hacia dirección negativa.
    */
    configX.homeEnabled = true;
    configX.homePin = 16;
    configX.homeActiveLow = true;
    configX.homeUsePullup = true;
    configX.homeDirectionPositive = false;
    configX.homeSimulationEnabled = false;

    /*
        Parámetros HOME X.
    */
    configX.homingSeekSpeedMmS = 30.0f;
    configX.homingLatchSpeedMmS = 5.0f;
    configX.homingBackoffMm = 3.0f;
    configX.maxHomingTravelMm = 350.0f;
    configX.homePositionMm = 0.0f;

    /*
        Límites físicos X.
    */
    configX.minLimitEnabled = true;
    configX.minLimitPin = 17;
    configX.minLimitActiveLow = true;
    configX.minLimitUsePullup = true;

    configX.maxLimitEnabled = true;
    configX.maxLimitPin = 18;
    configX.maxLimitActiveLow = true;
    configX.maxLimitUsePullup = true;

    /*
        Límites por software X.
    */
    configX.softLimitsEnabled = true;
    configX.softLimitMinMm = 0.0f;
    configX.softLimitMaxMm = 300.0f;

    /*
        ------------------------------------------------------------
        CONFIGURACIÓN EJE Y
        ------------------------------------------------------------
    */
    StepperAxisConfig configY =
        StepperAxisConfig::createBeltDrive(
            /* stepPin */            3,
            /* dirPin */             11,
            /* enablePin */          14,
            /* enableActiveLow */    true,
            /* directionInvert */    false,
            /* motorStepsPerRev */   200,
            /* microsteps */         16,
            /* beltPitchMm */        2.0f,
            /* pulleyTeeth */        20,
            /* gearRatio */          1.0f,
            /* maxSpeedMmS */        100.0f,
            /* accelerationMmS2 */   300.0f
        );

    /*
        Nombre obligatorio del eje Y.
    */
    configY.setAxisName("Y");

    /*
        HOME Y.

        homeDirectionPositive = false:
        Y buscará HOME hacia dirección negativa.
    */
    configY.homeEnabled = true;
    configY.homePin = 19;
    configY.homeActiveLow = true;
    configY.homeUsePullup = true;
    configY.homeDirectionPositive = false;
    configY.homeSimulationEnabled = false;

    /*
        Parámetros HOME Y.
    */
    configY.homingSeekSpeedMmS = 30.0f;
    configY.homingLatchSpeedMmS = 5.0f;
    configY.homingBackoffMm = 3.0f;
    configY.maxHomingTravelMm = 350.0f;
    configY.homePositionMm = 0.0f;

    /*
        Límites físicos Y.
    */
    configY.minLimitEnabled = true;
    configY.minLimitPin = 20;
    configY.minLimitActiveLow = true;
    configY.minLimitUsePullup = true;

    configY.maxLimitEnabled = true;
    configY.maxLimitPin = 21;
    configY.maxLimitActiveLow = true;
    configY.maxLimitUsePullup = true;

    /*
        Límites por software Y.
    */
    configY.softLimitsEnabled = true;
    configY.softLimitMinMm = 0.0f;
    configY.softLimitMaxMm = 300.0f;

    /*
        ------------------------------------------------------------
        CONFIGURACIÓN EJE Z
        ------------------------------------------------------------

        Eje Z configurado con husillo.

        screwPitchMm = 8.0f:
        Una vuelta completa del husillo equivale a 8 mm.
    */
    StepperAxisConfig configZ =
        StepperAxisConfig::createLeadScrew(
            /* stepPin */            4,
            /* dirPin */             12,
            /* enablePin */          15,
            /* enableActiveLow */    true,
            /* directionInvert */    false,
            /* motorStepsPerRev */   200,
            /* microsteps */         16,
            /* screwPitchMm */       8.0f,
            /* gearRatio */          1.0f,
            /* maxSpeedMmS */        20.0f,
            /* accelerationMmS2 */   100.0f
        );

    /*
        Nombre obligatorio del eje Z.
    */
    configZ.setAxisName("Z");

    /*
        HOME Z.

        homeDirectionPositive = true:
        Z buscará HOME hacia dirección positiva.

        Si tu HOME de Z está hacia abajo o negativo,
        cambia esto a false.
    */
    configZ.homeEnabled = true;
    configZ.homePin = 22;
    configZ.homeActiveLow = true;
    configZ.homeUsePullup = true;
    configZ.homeDirectionPositive = true;
    configZ.homeSimulationEnabled = false;

    /*
        Parámetros HOME Z.
    */
    configZ.homingSeekSpeedMmS = 10.0f;
    configZ.homingLatchSpeedMmS = 2.0f;
    configZ.homingBackoffMm = 2.0f;
    configZ.maxHomingTravelMm = 120.0f;
    configZ.homePositionMm = 0.0f;

    /*
        Límites físicos Z.
    */
    configZ.minLimitEnabled = true;
    configZ.minLimitPin = 26;
    configZ.minLimitActiveLow = true;
    configZ.minLimitUsePullup = true;

    configZ.maxLimitEnabled = true;
    configZ.maxLimitPin = 27;
    configZ.maxLimitActiveLow = true;
    configZ.maxLimitUsePullup = true;

    /*
        Límites por software Z.
    */
    configZ.softLimitsEnabled = true;
    configZ.softLimitMinMm = 0.0f;
    configZ.softLimitMaxMm = 100.0f;

    /*
        ------------------------------------------------------------
        CREAR INSTANCIAS DE EJES
        ------------------------------------------------------------

        Cada objeto representa un eje físico.
    */
    AdvanceStepperAxis axisX(configX);
    AdvanceStepperAxis axisY(configY);
    AdvanceStepperAxis axisZ(configZ);

    /*
        ------------------------------------------------------------
        SIMULACIÓN DE HOME PARA PRUEBAS SIN SENSORES
        ------------------------------------------------------------

        Para probar sin sensores físicos, puedes descomentar este bloque.

        setHomeSimulationEnabled(true):
        El eje ya no lee el pin HOME físico.

        setSimulatedHomeActive(true):
        El programa cree que el sensor HOME ya está activo.
    */

    /*
    axisX.setHomeSimulationEnabled(true);
    axisY.setHomeSimulationEnabled(true);
    axisZ.setHomeSimulationEnabled(true);

    axisX.setSimulatedHomeActive(true);
    axisY.setSimulatedHomeActive(true);
    axisZ.setSimulatedHomeActive(true);
    */

    /*
        ------------------------------------------------------------
        ARREGLO DE EJES
        ------------------------------------------------------------

        El controlador necesita recibir los ejes en un arreglo.

        Aunque el HOME ya puede configurarse por nombre,
        este arreglo sigue definiendo el orden interno.

        axes[0] = X
        axes[1] = Y
        axes[2] = Z
    */
    AdvanceStepperAxis* axes[] = {
        &axisX,
        &axisY,
        &axisZ
    };

    /*
        ------------------------------------------------------------
        CREAR CONTROLADOR DE MOVIMIENTO
        ------------------------------------------------------------
    */
    AdvanceMotionController motion(
        axes,
        3,
        pulseEngine
    );

    executeHomeSequence(
        motion,
        {
            "Z",
            "X",
            "Y"
        }
    );

    /*
        ------------------------------------------------------------
        POSICIÓN SEGURA DESPUÉS DE HOME
        ------------------------------------------------------------

        Solo se permite si todos los ejes hicieron HOME.

        X = 0 mm
        Y = 0 mm
        Z = 10 mm
    */
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

    /*
        Esperar a que termine el movimiento seguro.
    */
    while (motion.isBusy()) {
        motion.update();
    }

    printf("Safe position reached.\n");

    /*
        ------------------------------------------------------------
        EJEMPLO A: UN SOLO MOVIMIENTO DIRECTO
        ------------------------------------------------------------

        moveLinearTo() es directo:
        - No usa cola.
        - No usa suavizado de trayectoria.
        - Sí usa perfil trapezoidal, aceleración, límites y PIO.
    */
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

    /*
        ------------------------------------------------------------
        EJEMPLO B: MOVIMIENTOS EN COLA CON SUAVIZADO
        ------------------------------------------------------------

        configurePathSmoothing() define los parámetros de suavizado.

        prepareQueuedMoves() sí usa esos parámetros para detectar
        esquinas y generar segmentos suaves.
    */
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

    bool queueOk =
        motion.prepareQueuedMoves();

    if (!queueOk) {
        printf("ERROR: Could not prepare movement queue.\n");

        motion.emergencyStop();

        while (true) {
            tight_loop_contents();
        }
    }

    printf("Movement queue ready.\n");

    /*
        ------------------------------------------------------------
        LOOP PRINCIPAL
        ------------------------------------------------------------

        update() debe llamarse constantemente.

        Aquí se ejecutan los movimientos de la cola.
    */
    while (true) {
        motion.update();
    }
}
