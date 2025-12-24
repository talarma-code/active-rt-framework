#include "ActiveTask.h"

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

    instance->setup();

    while (true) {
        instance->loop();
        taskYIELD(); // or vTaskDelay(...)
    }
}
