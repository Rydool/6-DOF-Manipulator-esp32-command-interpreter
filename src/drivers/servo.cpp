#include "servo.h"

ServoDriver servoDriver;

void ServoDriver::begin() {
    ledcSetup(0, SERVO_FREQ, 16);   // kanał 0, 50 Hz, 16-bit
    ledcAttachPin(SERVO_PIN, 0);
    setAngle(currentAngle);
}

void ServoDriver::setAngle(float deg) {
    if (deg < 0) deg = 0;
    if (deg > 180) deg = 180;

    currentAngle = deg;

    // mapowanie stopni → mikrosekundy
    float us = SERVO_MIN_US + (SERVO_MAX_US - SERVO_MIN_US) * (deg / 180.0f);

    // konwersja µs → wartość PWM 16-bit
    uint32_t duty = (uint32_t)((us / 20000.0f) * 65535.0f);

    ledcWrite(0, duty);
}

float ServoDriver::getAngle() {
    return currentAngle;
}
