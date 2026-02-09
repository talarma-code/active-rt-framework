#pragma once

#include <cstddef>
#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Owning wrapper for FreeRTOS queue
 * @tparam T element type (copied by value)
 */
template<typename T>
class ActiveQueue {
public:
    // Special timeout values (in milliseconds domain)
    // kWaitForeverMs is a sentinel mapped to portMAX_DELAY in FreeRTOS ticks
    static constexpr uint32_t kWaitForeverMs    = UINT32_MAX;
    static constexpr uint32_t kDefaultTimeoutMs = 500; // 500 ms

    /**
     * @param length number of elements in queue
     */
    explicit ActiveQueue(size_t length)
        : queueLength(length)
    {
        handle = xQueueCreate(length, sizeof(T));
    }

    ~ActiveQueue() {
        if (handle) {
            vQueueDelete(handle);
        }
    }

    ActiveQueue(const ActiveQueue&) = delete;
    ActiveQueue& operator=(const ActiveQueue&) = delete;

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

    // -------- Helpers --------

    bool isEmpty() const {
        return uxQueueMessagesWaiting(handle) == 0;
    }

    bool isFull() const {
        return uxQueueSpacesAvailable(handle) == 0;
    }

    size_t size() const {
        return uxQueueMessagesWaiting(handle);
    }

    size_t capacity() const {
        return queueLength;
    }

    QueueHandle_t nativeHandle() const {
        return handle;
    }

private:
    QueueHandle_t handle = nullptr;
    const size_t queueLength;
};

