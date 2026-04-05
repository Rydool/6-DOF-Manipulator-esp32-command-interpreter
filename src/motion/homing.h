#pragma once
#include <Arduino.h>
#include "../drivers/stepgen.h"
#include "../drivers/pins.h"
#include "queue.h"

// Struktura opisująca parametry homingu
struct HomingParams {
    int axis;               // która oś
    float speed_deg_s;      // prędkość dojazdu
    float backoff_deg;      // cofnięcie po trafieniu w krańcówkę
    float latch_speed;      // wolny dojazd po odbiciu
};

// API homingu
void homing_start(const MotionBlock& b);
void homing_update(float dt);
bool homing_isActive();

// Funkcja pomocnicza do sprawdzania krańcówek
bool homing_limitHit(int axis);

// Globalny stan homingu
struct HomingState {
    bool active = false;
    bool stage1 = false;    // szybki dojazd
    bool stage2 = false;    // odbicie
    bool stage3 = false;    // wolny dojazd

    HomingParams params;
};

extern HomingState homingState;
