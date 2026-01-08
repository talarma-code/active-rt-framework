#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>

#include "ActiveQueueRef.h"

enum class TimerEventType : uint8_t {
    Fired
};

struct TimerEvent {
    uint32_t timerId;
    TimerEventType type;
};

class SystemTimer {
public:
    enum class Mode {
        OneShot,
        Periodic
    };

    SystemTimer(
        uint32_t timerId,
        uint32_t periodMs,
        Mode mode,
        ActiveQueueRef<TimerEvent> eventQueue
    );

    ~SystemTimer();

    // Start using the mode configured in the constructor.
    bool start();
    // Change the period (milliseconds) and start the timer. Note: this does NOT change the mode (one-shot vs periodic).
    bool start(uint32_t ms);
    bool stop();
    bool reset();

private:
    static void timerCallback(TimerHandle_t xTimer);

    TimerHandle_t _timer;
    uint32_t _timerId;
    ActiveQueueRef<TimerEvent> _eventQueue;
    Mode _mode;
};
