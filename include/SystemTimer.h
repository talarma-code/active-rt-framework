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

// -----------------------------------------------------------------------------
// Templated variant: does not require the framework to know about application types.
// MessageT - type placed on the queue (e.g. std::variant<TimerEvent, SystemMessagePacket>)
// Converter - callable: MessageT operator()(const TimerEvent&), must be ISR-safe and not allocate.
// -----------------------------------------------------------------------------

// forward declaration so we can use it as a default template parameter
template<typename MessageT> class DefaultTimerToMessageConverter;

template<typename MessageT, typename Converter = DefaultTimerToMessageConverter<MessageT>>
class SystemTimerT {
public:
    enum class Mode {
        OneShot,
        Periodic
    };

    SystemTimerT(
        uint32_t timerId,
        uint32_t periodMs,
        Mode mode,
        ActiveQueueRef<MessageT> eventQueue,
        Converter converter = Converter()
    ) : _timerId(timerId),
        _eventQueue(eventQueue),
        _mode(mode),
        _converter(converter)
    {
        const TickType_t ticks = pdMS_TO_TICKS(periodMs);
        const UBaseType_t autoReload =
            (mode == Mode::Periodic) ? pdTRUE : pdFALSE;

        _timer = xTimerCreate(
            "SystemTimerT",
            ticks,
            autoReload,
            this,              // timer ID → pointer to this object
            &SystemTimerT::timerCallback
        );
    }

    ~SystemTimerT() {
        if (_timer) {
            xTimerDelete(_timer, 0);
        }
    }

    bool start() {
        return (_timer && xTimerStart(_timer, 0) == pdPASS);
    }

    bool start(uint32_t ms) {
        if (!_timer) return false;
        const TickType_t ticks = pdMS_TO_TICKS(ms);
        if (xTimerChangePeriod(_timer, ticks, 0) != pdPASS) return false;
        return xTimerStart(_timer, 0) == pdPASS;
    }

    bool stop() {
        return (_timer && xTimerStop(_timer, 0) == pdPASS);
    }

    bool reset() {
        return (_timer && xTimerReset(_timer, 0) == pdPASS);
    }

private:
    static void timerCallback(TimerHandle_t xTimer) {
        auto* self = static_cast<SystemTimerT*>(pvTimerGetTimerID(xTimer));
        if (!self) return;

        TimerEvent te{ .timerId = self->_timerId, .type = TimerEventType::Fired };
        // Convert to MessageT using the provided converter (must be ISR-safe)
        MessageT msg = self->_converter(te);
        self->_eventQueue.sendFromISR(msg);
    }

    TimerHandle_t _timer = nullptr;
    uint32_t _timerId;
    ActiveQueueRef<MessageT> _eventQueue;
    Mode _mode;
    Converter _converter;
};

// Helper default converter: requires MessageT to be constructible from TimerEvent (works for TimerEvent or std::variant)
template<typename MessageT>
class DefaultTimerToMessageConverter {
    public:
    // Default: construct MessageT from TimerEvent (works for MessageT == TimerEvent and for std::variant<TimerEvent,...>)
    MessageT operator()(const TimerEvent& t) const { return MessageT(t); }
};

// Specialization for MessageT == TimerEvent: return a copy directly (no extra construction required)
template<>
class DefaultTimerToMessageConverter<TimerEvent> {
    public:
    TimerEvent operator()(const TimerEvent& t) const { return t; }
};
