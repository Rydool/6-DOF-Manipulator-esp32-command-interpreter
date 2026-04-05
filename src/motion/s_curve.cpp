#include "s_curve.h"
#include <math.h>

// Uproszczony 7‑segmentowy profil S‑curve
// Zakładamy: v_start = 0, v_end = 0 (jak w Twoim plannerze)

void scurve_compute(const SCurveProfile& p, SCurveState& out) {
    // Bardzo uproszczony model:
    // - brak pełnego wyliczania wszystkich 7 segmentów
    // - traktujemy to jako "trapez" z wygładzonymi narożami

    float dist = fabsf(p.distance_deg);
    if (dist <= 0.0f || p.v_max <= 0.0f || p.accel <= 0.0f) {
        out.totalTime = 0;
        return;
    }

    // Czas rozpędzania do v_max przy danej accel
    float t_acc = p.v_max / p.accel;
    float d_acc = 0.5f * p.accel * t_acc * t_acc;

    // Sprawdź, czy osiągamy v_max
    if (2.0f * d_acc > dist) {
        // Ruch "trójkątny" (nie osiągamy v_max)
        t_acc = sqrtf(dist / p.accel);
        float t_total = 2.0f * t_acc;

        out.t1 = t_acc;
        out.t2 = t_acc;   // brak pełnej fazy stałej prędkości
        out.t3 = t_total;
        out.totalTime = t_total;
    } else {
        // Ruch "trapezowy" (osiągamy v_max)
        float d_flat = dist - 2.0f * d_acc;
        float t_flat = d_flat / p.v_max;

        float t_total = 2.0f * t_acc + t_flat;

        out.t1 = t_acc;
        out.t2 = t_acc + t_flat;
        out.t3 = t_total;
        out.totalTime = t_total;
    }
}

// Zwraca prędkość w czasie t (bardzo uproszczony S‑curve → w praktyce "trapez")
float scurve_speed_at(const SCurveState& s, const SCurveProfile& p, float t) {
    if (t <= 0.0f || s.totalTime <= 0.0f) return 0.0f;
    if (t >= s.totalTime) return 0.0f;

    float t1 = s.t1;
    float t2 = s.t2;
    float t3 = s.t3;

    // Faza przyspieszania
    if (t <= t1) {
        return p.accel * t;
    }

    // Faza stałej prędkości
    if (t <= t2) {
        return p.v_max;
    }

    // Faza hamowania
    float t_dec = t3 - t;
    return p.accel * t_dec;
}
