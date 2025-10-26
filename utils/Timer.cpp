#include "Timer.h"

Timer::Timer(unsigned long interval) {
    this->interval = interval;
    this->startTime = 0;
    this->running = false;
}

Timer::Timer(unsigned long interval, bool autoStart) {
    this->interval = interval;
    this->running = autoStart;
    this->startTime = autoStart ? millis() : 0;
}

void Timer::start() {
    startTime = millis();
    running = true;
}

void Timer::stop() {
    running = false;
}

void Timer::reset() {
    startTime = millis();
}

bool Timer::isExpired() {
    if (!running) {
        return false;
    }

    return (millis() - startTime >= interval);
}

bool Timer::isRunning() {
    return running;
}

unsigned long Timer::getElapsed() {
    if (!running) {
        return 0;
    }

    return millis() - startTime;
}

unsigned long Timer::getRemaining() {
    if (!running) {
        return 0;
    }

    unsigned long elapsed = getElapsed();
    if (elapsed >= interval) {
        return 0;
    }

    return interval - elapsed;
}

void Timer::setInterval(unsigned long newInterval) {
    interval = newInterval;
}

unsigned long Timer::getInterval() {
    return interval;
}
