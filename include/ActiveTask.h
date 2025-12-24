#pragma once

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/**
 * @brief Base class for creating object-oriented FreeRTOS tasks on ESP32
 */
class ActiveTask {
public:
    /**
     * @param name      Task name (visible in FreeRTOS)
     * @param stackSize Stack size in words
     * @param priority  Task priority
     * @param core      Core affinity (tskNO_AFFINITY, 0 or 1)
     */
    ActiveTask(
        const char* name,
        uint32_t stackSize = 4096,
        UBaseType_t priority = 1,
        BaseType_t core = tskNO_AFFINITY
    );

    virtual ~ActiveTask() = default;

    /**
     * @brief Called once when the task starts
     */
    virtual void setup() {}

    /**
     * @brief Main task loop (must be overridden)
     */
    virtual void loop() = 0;

    /**
     * @brief Creates and starts the FreeRTOS task
     */
    void start();

protected:
    TaskHandle_t taskHandle = nullptr;

private:
    /**
     * @brief Static FreeRTOS task entry point
     */
    static void taskEntryPoint(void* param);

    const char* taskName;
    uint32_t taskStackSize;
    UBaseType_t taskPriority;
    BaseType_t taskCore;
};
