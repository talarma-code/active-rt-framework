#pragma once

#include <cstddef>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"


/**
 * @brief Owning, ISR-safe object pool built on top of FreeRTOS queues.
 *
 * ActivePool preallocates a fixed number of objects of type T and manages
 * their ownership using a FreeRTOS queue of pointers.
 *
 * No dynamic allocation is performed after construction.
 */
template<typename T>
class ActivePool {
public:
    /**
     * @param capacity Number of objects in the pool
     *
     * Must be called after FreeRTOS is ready.
     */
    explicit ActivePool(size_t capacity)
        : capacity_(capacity)
    {
        objects_ = new T[capacity_];
        queue_ = xQueueCreate(capacity_, sizeof(T*));

        for (size_t i = 0; i < capacity_; ++i) {
            T* ptr = &objects_[i];
            xQueueSend(queue_, &ptr, 0);
        }
    }

    ~ActivePool() {
        if (queue_) {
            vQueueDelete(queue_);
        }
        delete[] objects_;
    }

    ActivePool(const ActivePool&) = delete;
    ActivePool& operator=(const ActivePool&) = delete;

    // ---------- Task context ----------

    /**
     * @brief Acquire an object from the pool (task context)
     */
    T* acquire(TickType_t timeout = 0) {
        T* obj = nullptr;
        if (xQueueReceive(queue_, &obj, timeout) == pdTRUE) {
            return obj;
        }
        return nullptr;
    }

    /**
     * @brief Return an object back to the pool (task context)
     */
    bool release(T* obj) {
        return xQueueSend(queue_, &obj, 0) == pdTRUE;
    }

    // ---------- ISR context ----------

    /**
     * @brief Acquire an object from the pool (ISR context)
     */
    T* acquireFromISR(BaseType_t* hpw = nullptr) {
        T* obj = nullptr;
        if (xQueueReceiveFromISR(queue_, &obj, hpw) == pdTRUE) {
            return obj;
        }
        return nullptr;
    }

    /**
     * @brief Return an object back to the pool (ISR context)
     */
    bool releaseFromISR(T* obj, BaseType_t* hpw = nullptr) {
        return xQueueSendFromISR(queue_, &obj, hpw) == pdTRUE;
    }

    // ---------- Info ----------

    size_t capacity() const { return capacity_; }

    size_t available() const {
        return uxQueueMessagesWaiting(queue_);
    }

    QueueHandle_t nativeHandle() const {
        return queue_;
    }

private:
    T* objects_ = nullptr;
    QueueHandle_t queue_ = nullptr;
    const size_t capacity_;
};

