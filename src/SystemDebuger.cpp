#include "SystemDebuger.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_system.h>

void SystemDebuger::printTaskList() {
    // static char buffer[512];

    // Serial.println();
    // Serial.println("Name          State  Prio  Stack  Num");
    // Serial.println("---------------------------------------");

    // vTaskList(buffer);
    // Serial.println(buffer);
}

void SystemDebuger::printFreeHeap() {
    Serial.printf(
        "Free heap: %u bytes\n",
        ESP.getFreeHeap()
    );
}

void SystemDebuger::printHeapWatermark() {
    Serial.printf(
        "Minimum ever free heap: %u bytes\n",
        xPortGetMinimumEverFreeHeapSize()
    );
}

void SystemDebuger::printSystemStats() {
    Serial.println("===== SYSTEM STATS =====");
    printTaskList();
    printFreeHeap();
    printHeapWatermark();
    Serial.println("========================");
}
