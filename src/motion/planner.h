#pragma once
#include <Arduino.h>
#include "queue.h"
#include "s_curve.h"
#include "../config/machine_config.h"

// Stan aktualnie wykonywanego ruchu
struct ActiveMotion {
    bool active = false;

    MotionBlock block;      // aktualny blok ruchu
    SCurveProfile profile;  // parametry profilu
    SCurveState state;      // obliczone czasy segmentów

    float time = 0.0f;      // czas od początku ruchu
    float duration = 0.0f;  // całkowity czas ruchu

    float start_deg[AXIS_COUNT];   // pozycja początkowa
    float delta_deg[AXIS_COUNT];   // różnica pozycji
};

// Główny planner ruchu
class MotionPlanner {
public:
    void begin();
    void update(float dt);     // wywoływane w pętli głównej (co ~1–5 ms)

    bool isBusy() const;
    void stopAll();

    float getAxisPosition(int axis) const;

private:
    ActiveMotion motion;

    void startNextBlock();
    void computeProfile(const MotionBlock& b);
    void applyStepgen(float speed_deg_s, float dt);
};

// globalna instancja
extern MotionPlanner motionPlanner;
