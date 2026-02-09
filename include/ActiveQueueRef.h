#pragma once

#include <cstddef>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


/**
 * @brief Non-owning reference to an existing FreeRTOS queue
 * @tparam T element type (copied by value)
 */
template<typename T>
class ActiveQueueRef {
public:
    // Special timeout values (in milliseconds domain)
    // kWaitForeverMs is a sentinel mapped to portMAX_DELAY in FreeRTOS ticks
    static constexpr uint32_t kWaitForeverMs    = UINT32_MAX;
    static constexpr uint32_t kDefaultTimeoutMs = 500; // 500 ms

    ActiveQueueRef() = default;

    explicit ActiveQueueRef(QueueHandle_t queueHandle)
        : handle(queueHandle)
    {
    }

    // -------- Task context API --------

    // timeoutMs is expressed in milliseconds. Use kWaitForeverMs to wait forever.
    bool send(const T& item, uint32_t timeoutMs = kDefaultTimeoutMs) {
        TickType_t ticks;
        if (timeoutMs == kWaitForeverMs) {
            ticks = portMAX_DELAY;
        } else {
            ticks = pdMS_TO_TICKS(timeoutMs);
        }
        return xQueueSend(handle, &item, ticks) == pdTRUE;
    }

    // timeoutMs is expressed in milliseconds. Use kWaitForeverMs to wait forever.
    bool receive(T& out, uint32_t timeoutMs = kDefaultTimeoutMs) {
        TickType_t ticks;
        if (timeoutMs == kWaitForeverMs) {
            ticks = portMAX_DELAY;
        } else {
            ticks = pdMS_TO_TICKS(timeoutMs);
        }
        return xQueueReceive(handle, &out, ticks) == pdTRUE;
    }

    // -------- ISR context API --------

    bool sendFromISR(const T& item,
                     BaseType_t* higherPriorityTaskWoken = nullptr)
    {
        return xQueueSendFromISR(handle, &item, higherPriorityTaskWoken) == pdTRUE;
    }

    bool receiveFromISR(T& out,
                        BaseType_t* higherPriorityTaskWoken = nullptr)
    {
        return xQueueReceiveFromISR(handle, &out, higherPriorityTaskWoken) == pdTRUE;
    }

    QueueHandle_t nativeHandle() const {
        return handle;
    }

private:
    QueueHandle_t handle = nullptr;
};

