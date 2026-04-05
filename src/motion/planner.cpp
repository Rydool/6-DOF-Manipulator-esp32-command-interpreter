#include "planner.h"
#include "../drivers/stepgen.h"
#include "../motion/homing.h"

MotionPlanner motionPlanner;

void MotionPlanner::begin() {
    for (int i = 0; i < AXIS_COUNT; i++) {
        motion.start_deg[i] = 0;
    }
    motion.active = false;
}

bool MotionPlanner::isBusy() const {
    return motion.active;
}

float MotionPlanner::getAxisPosition(int axis) const {
    return motion.start_deg[axis] + (motion.time / motion.duration) * motion.delta_deg[axis];
}

void MotionPlanner::stopAll() {
    motion.active = false;
    stepgen_stopAll();
}

void MotionPlanner::update(float dt) {
    // Jeśli nie ma aktywnego ruchu — pobierz kolejny blok
    if (!motion.active) {
        startNextBlock();
        return;
    }

    // Aktualizuj czas
    motion.time += dt;
    if (motion.time > motion.duration) {
        motion.time = motion.duration;
    }

    // Oblicz prędkość z profilu S-curve
    float speed = scurve_speed_at(motion.state, motion.profile, motion.time);

    // Zastosuj prędkość do generatora kroków
    applyStepgen(speed, dt);

    // Jeśli ruch zakończony
    if (motion.time >= motion.duration) {

        // *** WYRÓWNANIE DO CELU — KONIECZNIE ***
        for (int i = 0; i < AXIS_COUNT; i++) {
            stepgen_setPositionDeg(i, motion.block.target_deg[i]);
        }

        // zatrzymaj ruch
        stepgen_stopAll();
        motion.active = false;
    }
}

void MotionPlanner::startNextBlock() {
    MotionBlock b;
    if (!motionQueue.pop(b)) {
        return; // kolejka pusta
    }

    // Homing ma własną logikę
    if (b.homing) {
        homing_start(b);
        return;
    }

    // Ustaw pozycję początkową
    for (int i = 0; i < AXIS_COUNT; i++) {
        motion.start_deg[i] = stepgen_getPositionDeg(i);
        motion.delta_deg[i] = b.target_deg[i] - motion.start_deg[i];
    }

    motion.block = b;

    computeProfile(b);
    motion.time = 0;
    motion.active = true;
}

void MotionPlanner::computeProfile(const MotionBlock& b) {
    // Oblicz dystans największej osi (dominującej)
    float maxDist = 0;
    for (int i = 0; i < AXIS_COUNT; i++) {
        float d = fabsf(motion.delta_deg[i]);
        if (d > maxDist) maxDist = d;
    }

    // Ustaw parametry profilu
    motion.profile.distance_deg = maxDist;
    motion.profile.v_start = 0;
    motion.profile.v_end = 0;
    motion.profile.v_max = b.feed_deg_s;
    motion.profile.accel =  machineConfig.max_accel_deg_s2[0]; // uproszczenie
    motion.profile.jerk  =  machineConfig.max_accel_deg_s2[0] * 2.0f;

    // Oblicz czasy segmentów
    scurve_compute(motion.profile, motion.state);

    motion.duration = motion.state.totalTime;
}

void MotionPlanner::applyStepgen(float speed_deg_s, float dt) {
    // Wylicz proporcje ruchu dla każdej osi
    float maxDist = 0;
    for (int i = 0; i < AXIS_COUNT; i++) {
        float d = fabsf(motion.delta_deg[i]);
        if (d > maxDist) maxDist = d;
    }

    if (maxDist == 0) return;

    for (int i = 0; i < AXIS_COUNT; i++) {
        float ratio = motion.delta_deg[i] / maxDist;
        float axisSpeed = speed_deg_s * ratio;

        stepgen_setSpeedDeg(i, axisSpeed);
    }
}
