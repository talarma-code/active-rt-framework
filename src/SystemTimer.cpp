#include "SystemTimer.h"

SystemTimer::SystemTimer(
    uint32_t timerId,
    uint32_t periodMs,
    Mode mode,
    ActiveQueueRef<TimerEvent> eventQueue
)
    : _timerId(timerId),
      _eventQueue(eventQueue),
      _mode(mode)
{
    const TickType_t ticks = pdMS_TO_TICKS(periodMs);
    const UBaseType_t autoReload =
        (mode == Mode::Periodic) ? pdTRUE : pdFALSE;

    _timer = xTimerCreate(
        "SystemTimer",
        ticks,
        autoReload,
        this,              // timer ID → pointer to this object
        &SystemTimer::timerCallback
    );
}

SystemTimer::~SystemTimer() {
    if (_timer) {
        xTimerDelete(_timer, 0);
    }
}

bool SystemTimer::start() {
    return (_timer && xTimerStart(_timer, 0) == pdPASS);
}

bool SystemTimer::start(uint32_t ms) {
    if (!_timer) return false;
    const TickType_t ticks = pdMS_TO_TICKS(ms);
    // Change period; does NOT change auto-reload mode set in constructor.
    if (xTimerChangePeriod(_timer, ticks, 0) != pdPASS) return false;
    return xTimerStart(_timer, 0) == pdPASS;
}

bool SystemTimer::stop() {
    return (_timer && xTimerStop(_timer, 0) == pdPASS);
}

bool SystemTimer::reset() {
    return (_timer && xTimerReset(_timer, 0) == pdPASS);
}

void SystemTimer::timerCallback(TimerHandle_t xTimer) {
    auto* self = static_cast<SystemTimer*>(
        pvTimerGetTimerID(xTimer)
    );

    if (!self) {
        return;
    }

    TimerEvent evt{
        .timerId = self->_timerId,
        .type = TimerEventType::Fired
    };

    // Timer callback runs in Tmr Svc task
    // Must be non-blocking
    self->_eventQueue.sendFromISR(evt);
}
