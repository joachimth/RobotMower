#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

/**
 * Timer klasse - Non-blocking timer til tidsbaserede operationer
 *
 * Denne klasse erstatter blocking delay() kald med non-blocking timers.
 */
class Timer {
public:
    /**
     * Constructor
     * @param interval Timer interval i millisekunder
     */
    Timer(unsigned long interval);

    /**
     * Constructor med auto-start
     * @param interval Timer interval i millisekunder
     * @param autoStart Start timer automatisk
     */
    Timer(unsigned long interval, bool autoStart);

    /**
     * Starter timer
     */
    void start();

    /**
     * Stopper timer
     */
    void stop();

    /**
     * Nulstiller timer til start
     */
    void reset();

    /**
     * Tjek om timer er udløbet
     * @return true hvis tiden er gået
     */
    bool isExpired();

    /**
     * Tjek om timer kører
     * @return true hvis timer er aktiv
     */
    bool isRunning();

    /**
     * Hent tid siden timer start
     * @return Millisekunder siden start
     */
    unsigned long getElapsed();

    /**
     * Hent resterende tid
     * @return Millisekunder tilbage (0 hvis udløbet)
     */
    unsigned long getRemaining();

    /**
     * Sæt nyt interval
     * @param newInterval Nyt interval i millisekunder
     */
    void setInterval(unsigned long newInterval);

    /**
     * Hent nuværende interval
     * @return Interval i millisekunder
     */
    unsigned long getInterval();

private:
    unsigned long interval;
    unsigned long startTime;
    bool running;
};

#endif // TIMER_H
