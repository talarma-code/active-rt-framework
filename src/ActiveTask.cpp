#include "ActiveTask.h"
#include "esp_task_wdt.h"

bool ActiveTask::watchdogInitialized = false;
uint32_t ActiveTask::watchdogTimeoutSeconds = 0;

ActiveTask::ActiveTask(
    const char* name,
    uint32_t stackSize,
    UBaseType_t priority,
    BaseType_t core
)
    : taskName(name),
      taskStackSize(stackSize),
      taskPriority(priority),
      taskCore(core)
{
}

void ActiveTask::initWatchdog(uint32_t timeoutSeconds) {
    if (!watchdogInitialized) {
        esp_task_wdt_init(timeoutSeconds, true);
        watchdogTimeoutSeconds = timeoutSeconds;
        watchdogInitialized = true;
    }
}

void ActiveTask::enableWatchdog(bool enable) {
    watchdogEnabled = enable;
}

void ActiveTask::resetWatchdog() {
    if (watchdogInitialized && watchdogEnabled) {
        esp_task_wdt_reset();
    }
}

void ActiveTask::start() {
    if (taskHandle != nullptr) {
        return; // Task already started
    }

    if (taskCore == tskNO_AFFINITY) {
        xTaskCreate(
            taskEntryPoint,
            taskName,
            taskStackSize,
            this,
            taskPriority,
            &taskHandle
        );
    } else {
        xTaskCreatePinnedToCore(
            taskEntryPoint,
            taskName,
            taskStackSize,
            this,
            taskPriority,
            &taskHandle,
            taskCore
        );
    }
}

void ActiveTask::taskEntryPoint(void* param) {
    ActiveTask* instance = static_cast<ActiveTask*>(param);

    if (watchdogInitialized && instance->watchdogEnabled) {
        // Register current FreeRTOS task in Task Watchdog
        esp_task_wdt_add(nullptr);
    }

    instance->setup();

    while (true) {
        instance->loop();

        taskYIELD(); // or vTaskDelay(...)
    }
}
