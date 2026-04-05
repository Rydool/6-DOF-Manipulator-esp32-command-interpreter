#include "homing.h"
#include "../drivers/stepgen.h"
#include "../config/machine_config.h"

HomingState homingState;

// -----------------------------------------
// Sprawdzenie krańcówki
// -----------------------------------------
bool homing_limitHit(int axis) {
    switch (axis) {
        case 0: return digitalRead(X_LIMIT_PIN) == LOW;
        case 1: return digitalRead(Y_LIMIT_PIN) == LOW;
        case 2: return digitalRead(Z_LIMIT_PIN) == LOW;
        case 3: return digitalRead(A_LIMIT_PIN) == LOW;
        case 4: return digitalRead(B_LIMIT_PIN) == LOW;
        case 5: return digitalRead(C_LIMIT_PIN) == LOW;
    }
    return false;
}

// -----------------------------------------
// Start procedury homingu
// -----------------------------------------
void homing_start(const MotionBlock& b) {
    homingState.active = true;
    homingState.stage1 = true;
    homingState.stage2 = false;
    homingState.stage3 = false;

    // Parametry homingu
    homingState.params.axis = b.target_deg[0];   // w MotionBlock target_deg[0] = numer osi
    homingState.params.speed_deg_s = 60;         // szybki dojazd
    homingState.params.backoff_deg = 5;          // odbicie
    homingState.params.latch_speed = 10;         // wolny dojazd

    // Ustaw szybki ruch w stronę krańcówki
    stepgen_setSpeedDeg(homingState.params.axis, -homingState.params.speed_deg_s);
}

// -----------------------------------------
// Czy homing trwa?
// -----------------------------------------
bool homing_isActive() {
    return homingState.active;
}

// -----------------------------------------
// Aktualizacja homingu (wywoływana z planner.update())
// -----------------------------------------
void homing_update(float dt) {
    if (!homingState.active) return;

    int ax = homingState.params.axis;

    // -----------------------------
    // ETAP 1 — szybki dojazd
    // -----------------------------
    if (homingState.stage1) {
        if (homing_limitHit(ax)) {
            homingState.stage1 = false;
            homingState.stage2 = true;

            // Odbicie
            stepgen_setSpeedDeg(ax, homingState.params.speed_deg_s);
        }
        return;
    }

    // -----------------------------
    // ETAP 2 — odbicie
    // -----------------------------
    if (homingState.stage2) {
        float pos = stepgen_getPositionDeg(ax);
        if (pos >= homingState.params.backoff_deg) {
            homingState.stage2 = false;
            homingState.stage3 = true;

            // Wolny dojazd
            stepgen_setSpeedDeg(ax, -homingState.params.latch_speed);
        }
        return;
    }

    // -----------------------------
    // ETAP 3 — wolny dojazd
    // -----------------------------
    if (homingState.stage3) {
        if (homing_limitHit(ax)) {
            // Ustaw pozycję osi na 0°
			stepgen_setPositionDeg(ax, 0.0f);


            // Zatrzymaj oś
            stepgen_setSpeedDeg(ax, 0);

            homingState.active = false;
            homingState.stage3 = false;
        }
        return;
    }
}
