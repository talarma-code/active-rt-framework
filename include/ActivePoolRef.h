#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/**
 * @brief Non-owning reference to an existing ActivePool.
 */
template<typename T>
class ActivePoolRef {
public:
    explicit ActivePoolRef(QueueHandle_t queue)
        : queue_(queue) {}

    T* acquire(TickType_t timeout = 0) {
        T* obj = nullptr;
        if (xQueueReceive(queue_, &obj, timeout) == pdTRUE) {
            return obj;
        }
        return nullptr;
    }

    bool release(T* obj) {
        return xQueueSend(queue_, &obj, 0) == pdTRUE;
    }

    T* acquireFromISR(BaseType_t* hpw = nullptr) {
        T* obj = nullptr;
        if (xQueueReceiveFromISR(queue_, &obj, hpw) == pdTRUE) {
            return obj;
        }
        return nullptr;
    }

    bool releaseFromISR(T* obj, BaseType_t* hpw = nullptr) {
        return xQueueSendFromISR(queue_, &obj, hpw) == pdTRUE;
    }

private:
    QueueHandle_t queue_;
};

