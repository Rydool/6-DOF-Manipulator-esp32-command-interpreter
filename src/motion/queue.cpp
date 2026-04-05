#include "queue.h"

// Globalna instancja kolejki
MotionQueue motionQueue;

MotionQueue::MotionQueue() {
    head = 0;
    tail = 0;
    count = 0;
}

bool MotionQueue::push(const MotionBlock& b) {
    if (count >= QUEUE_SIZE) {
        return false; // kolejka pełna
    }

    buffer[tail] = b;
    tail = (tail + 1) % QUEUE_SIZE;
    count++;
    return true;
}

bool MotionQueue::pop(MotionBlock& b) {
    if (count == 0) {
        return false; // kolejka pusta
    }

    b = buffer[head];
    head = (head + 1) % QUEUE_SIZE;
    count--;
    return true;
}

bool MotionQueue::empty() const {
    return count == 0;
}

bool MotionQueue::full() const {
    return count >= QUEUE_SIZE;
}

int MotionQueue::size() const {
    return count;
}
