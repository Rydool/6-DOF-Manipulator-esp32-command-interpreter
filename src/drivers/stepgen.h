#pragma once
#include <Arduino.h>
#include "../config/machine_config.h"
#include "pins.h"

#define AXIS_COUNT 6

struct StepAxis {
    int stepPin;
    int dirPin;

    volatile int32_t stepsPerTick_fp = 0;   // Q16.16
    volatile int32_t accumulator_fp  = 0;   // Q16.16
    volatile int32_t position_steps  = 0;   // w krokach
    volatile int8_t  dir             = 1;   // +1 / -1
};

class StepGen {
public:
    void begin();
    void updateISR();  // ISR 20 kHz

    void setSpeedDeg(int axis, float deg_s);
    float getPositionDeg(int axis);
    void setPositionDeg(int axis, float deg);
    void stopAll();

private:
    StepAxis axes[AXIS_COUNT];
    float ram_steps_per_deg[AXIS_COUNT];

    void setupPins();
};

extern StepGen stepgen;

inline void stepgen_setSpeedDeg(int axis, float s) { stepgen.setSpeedDeg(axis, s); }
inline float stepgen_getPositionDeg(int axis) { return stepgen.getPositionDeg(axis); }
inline void stepgen_setPositionDeg(int axis, float d) { stepgen.setPositionDeg(axis, d); }
inline void stepgen_stopAll() { stepgen.stopAll(); }
