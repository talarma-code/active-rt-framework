#pragma once

#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Owning wrapper for FreeRTOS queue
 * @tparam T element type (copied by value)
 */
template<typename T>
class ActiveQueue {
public:
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

    bool send(const T& item, TickType_t timeout = portMAX_DELAY) {
        return xQueueSend(handle, &item, timeout) == pdTRUE;
    }

    bool receive(T& out, TickType_t timeout = portMAX_DELAY) {
        return xQueueReceive(handle, &out, timeout) == pdTRUE;
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

