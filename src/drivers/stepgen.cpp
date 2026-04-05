#include "stepgen.h"
#include "driver/gpio.h"
#include "soc/gpio_struct.h"

StepGen stepgen;

hw_timer_t* stepTimer = nullptr;
static const int STEPGEN_ISR_FREQ = 20000;
static const int FP_SHIFT = 16;
static const int32_t FP_ONE = 1 << FP_SHIFT;

// ISR wrapper
void IRAM_ATTR stepgen_timer_isr() {
    stepgen.updateISR();
}

void StepGen::begin() {
    setupPins();

    for (int i = 0; i < AXIS_COUNT; i++) {
        ram_steps_per_deg[i] = machineConfig.steps_per_deg[i];
        axes[i].stepsPerTick_fp = 0;
        axes[i].accumulator_fp  = 0;
        axes[i].position_steps  = 0;
        axes[i].dir             = 1;
    }

    stepTimer = timerBegin(0, 80, true); // 1 MHz
    timerAttachInterrupt(stepTimer, &stepgen_timer_isr, true);
    timerAlarmWrite(stepTimer, 50, true); // 20 kHz
    timerAlarmEnable(stepTimer);
}

void StepGen::setupPins() {
    int stepPins[AXIS_COUNT] = {X_STEP_PIN, Y_STEP_PIN, Z_STEP_PIN, A_STEP_PIN, B_STEP_PIN, C_STEP_PIN};
    int dirPins [AXIS_COUNT] = {X_DIR_PIN,  Y_DIR_PIN,  Z_DIR_PIN,  A_DIR_PIN,  B_DIR_PIN,  C_DIR_PIN};

    for (int i = 0; i < AXIS_COUNT; i++) {
        axes[i].stepPin = stepPins[i];
        axes[i].dirPin  = dirPins[i];

        pinMode(stepPins[i], OUTPUT);
        pinMode(dirPins[i], OUTPUT);

        digitalWrite(stepPins[i], LOW);
        digitalWrite(dirPins[i], LOW);
    }
}

void StepGen::setSpeedDeg(int axis, float deg_s) {
    if (axis < 0 || axis >= AXIS_COUNT) return;

    float stepsPerDeg = ram_steps_per_deg[axis];
    float stepsPerSec = deg_s * stepsPerDeg;
    float stepsPerTick = stepsPerSec / (float)STEPGEN_ISR_FREQ;

    int32_t fp = (int32_t)(stepsPerTick * (float)FP_ONE);

    axes[axis].stepsPerTick_fp = fp;
    axes[axis].dir = (deg_s >= 0.0f) ? 1 : -1;
}

float StepGen::getPositionDeg(int axis) {
    if (axis < 0 || axis >= AXIS_COUNT) return 0.0f;
    int32_t steps = axes[axis].position_steps;
    float stepsPerDeg = ram_steps_per_deg[axis];
    return (float)steps / stepsPerDeg;
}

void StepGen::setPositionDeg(int axis, float deg) {
    if (axis < 0 || axis >= AXIS_COUNT) return;
    float stepsPerDeg = ram_steps_per_deg[axis];
    int32_t steps = (int32_t)(deg * stepsPerDeg);
    axes[axis].position_steps = steps;
    axes[axis].accumulator_fp = 0;
}

void StepGen::stopAll() {
    for (int i = 0; i < AXIS_COUNT; i++) {
        axes[i].stepsPerTick_fp = 0;
    }
}

// ISR — integer only + GPIO
void IRAM_ATTR StepGen::updateISR() {
    for (int i = 0; i < AXIS_COUNT; i++) {
        StepAxis& ax = axes[i];

        int32_t acc = ax.accumulator_fp + ax.stepsPerTick_fp;
        ax.accumulator_fp = acc;

        while (acc >= FP_ONE || acc <= -FP_ONE) {

            // Ustaw kierunek
            int dirPin = ax.dirPin;
            if (ax.dir > 0) {
                if (dirPin < 32) GPIO.out_w1ts = (1UL << dirPin);
                else GPIO.out1_w1ts.data = (1UL << (dirPin - 32));
            } else {
                if (dirPin < 32) GPIO.out_w1tc = (1UL << dirPin);
                else GPIO.out1_w1tc.data = (1UL << (dirPin - 32));
            }

            // Wygeneruj krok
            int stepPin = ax.stepPin;
            if (stepPin < 32) {
                GPIO.out_w1ts = (1UL << stepPin);
                GPIO.out_w1tc = (1UL << stepPin);
            } else {
                GPIO.out1_w1ts.data = (1UL << (stepPin - 32));
                GPIO.out1_w1tc.data = (1UL << (stepPin - 32));
            }

            // Aktualizacja pozycji
            if (acc >= FP_ONE) {
                acc -= FP_ONE;
                ax.position_steps += ax.dir;
            } else if (acc <= -FP_ONE) {
                acc += FP_ONE;
                ax.position_steps -= ax.dir;
            }
        }

        ax.accumulator_fp = acc;
    }
}
