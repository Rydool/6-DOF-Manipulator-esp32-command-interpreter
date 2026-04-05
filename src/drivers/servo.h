#pragma once
#include <Arduino.h>

// Domyślne parametry serwa 180°
#define SERVO_PIN 27
#define SERVO_MIN_US 500     // impuls dla 0°
#define SERVO_MAX_US 2500    // impuls dla 180°
#define SERVO_FREQ 50        // 50 Hz = standardowe serwo

class ServoDriver {
public:
    void begin();
    void setAngle(float deg);   // 0–180°
    float getAngle();

private:
    float currentAngle = 90.0f; // start neutralnie
};

extern ServoDriver servoDriver;
