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

    bool start();
    bool stop();
    bool reset();

private:
    static void timerCallback(TimerHandle_t xTimer);

    TimerHandle_t _timer;
    uint32_t _timerId;
    ActiveQueueRef<TimerEvent> _eventQueue;
};
