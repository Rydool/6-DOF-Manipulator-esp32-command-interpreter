#pragma once
#include <Arduino.h>

// Ten moduł implementuje uproszczony profil S-curve (jerk-limited)
// Działa w jednostkach fizycznych: stopnie, stopnie/s, stopnie/s²

struct SCurveProfile {
    float distance_deg;      // dystans ruchu w stopniach
    float v_start;           // prędkość początkowa (deg/s)
    float v_end;             // prędkość końcowa (deg/s)
    float v_max;             // prędkość maksymalna (deg/s)
    float accel;             // przyspieszenie (deg/s²)
    float jerk;              // jerk (deg/s³)
};

// Wyniki obliczeń profilu
struct SCurveState {
    float t1, t2, t3;        // fazy przyspieszania
    float t4;                // faza stałej prędkości
    float t5, t6, t7;        // fazy hamowania
    float totalTime;         // całkowity czas ruchu
};

// API
void scurve_compute(const SCurveProfile& p, SCurveState& out);
float scurve_speed_at(const SCurveState& s, const SCurveProfile& p, float t);
