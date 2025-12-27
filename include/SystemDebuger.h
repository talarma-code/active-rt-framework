#pragma once

#include <Arduino.h>

/**
 * @brief System debugging utilities for FreeRTOS (ESP32).
 *
 * Provides task list and heap usage diagnostics.
 * Intended for debug / monitoring tasks.
 */
class SystemDebuger {
public:
    /// Print FreeRTOS task list (name, state, priority, stack watermark)
    static void printTaskList();

    /// Print current free heap size
    static void printFreeHeap();

    /// Print minimum-ever free heap size (heap high-water mark)
    static void printHeapWatermark();

    /// Print combined system diagnostics (tasks + heap)
    static void printSystemStats();
};
