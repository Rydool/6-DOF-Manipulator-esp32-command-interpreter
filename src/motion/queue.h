#pragma once
#include <Arduino.h>
#include "../config/machine_config.h"

// Pojedynczy blok ruchu — jeden wpis w kolejce
struct MotionBlock {
    float target_deg[AXIS_COUNT];   // pozycje docelowe w stopniach
    float feed_deg_s;               // prędkość w stopniach/s
    bool homing;                    // czy to ruch homingu
};

// Kolejka FIFO na bloki ruchu
class MotionQueue {
public:
    MotionQueue();

    bool push(const MotionBlock& b);   // dodaj blok
    bool pop(MotionBlock& b);          // pobierz blok
    bool empty() const;
    bool full() const;
    int size() const;

private:
    static const int QUEUE_SIZE = 32;
    MotionBlock buffer[QUEUE_SIZE];

    volatile int head;
    volatile int tail;
    volatile int count;
};

// globalna instancja kolejki
extern MotionQueue motionQueue;
