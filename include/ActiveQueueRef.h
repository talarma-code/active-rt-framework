#pragma once

#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


/**
 * @brief Non-owning reference to an existing FreeRTOS queue
 * @tparam T element type (copied by value)
 */
template<typename T>
class ActiveQueueRef {
public:
    explicit ActiveQueueRef(QueueHandle_t queueHandle)
        : handle(queueHandle)
    {
    }

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

    QueueHandle_t nativeHandle() const {
        return handle;
    }

private:
    QueueHandle_t handle = nullptr;
};

