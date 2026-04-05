#pragma once
#include <Arduino.h>

// --- Piny osi (bez zmian) ---

#define X_STEP_PIN    16
#define X_DIR_PIN     17
#define X_ENABLE_PIN  14
#define X_LIMIT_PIN   34

#define Y_STEP_PIN    18
#define Y_DIR_PIN     19
#define Y_ENABLE_PIN  14
#define Y_LIMIT_PIN   35

#define Z_STEP_PIN    21
#define Z_DIR_PIN     22
#define Z_ENABLE_PIN  14
#define Z_LIMIT_PIN   13

#define A_STEP_PIN    5
#define A_DIR_PIN     23
#define A_ENABLE_PIN  14
#define A_LIMIT_PIN   4

#define B_STEP_PIN    32
#define B_DIR_PIN     33
#define B_ENABLE_PIN  14
#define B_LIMIT_PIN   2

#define C_STEP_PIN    25
#define C_DIR_PIN     26
#define C_ENABLE_PIN  14
#define C_LIMIT_PIN   15

// --- Serwo chwytaka ---
#define SERVO_PIN 27

inline void setAllEnable(bool enable) {
    pinMode(X_ENABLE_PIN, OUTPUT);
    digitalWrite(X_ENABLE_PIN, enable ? LOW : HIGH);
}
