#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// XIAO ESP32-C6 user LED
#define BLINK_GPIO 15

// 1. Define the Task Function
void blink_task(void *pvParameter)
{
    // Hardware initialization inside the task. 
    // It is often cleaner for a task to configure the peripherals it exclusively owns.
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;

    // The task execution loop
    while (1) {
        gpio_set_level(BLINK_GPIO, led_state);
        
        // Print out the state. This helps us verify the scheduler is running this task.
        printf("Blink Task: LED State %s\n", led_state == 0 ? "ON" : "OFF");
        
        led_state = !led_state; 
        
        // Yield to the scheduler. Without this delay, the task would starve lower-priority tasks.
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }

    // Safety net: If we ever broke out of the while(1) loop, we must delete the task.
    // NULL tells the OS "delete the task that is currently running".
    vTaskDelete(NULL);
}

// 2. The Entry Point
void app_main(void)
{
    printf("System booted. Spawning blink task...\n");

    // Tell FreeRTOS to allocate memory and schedule our new task
    xTaskCreate(
        blink_task,       // Pointer to the function implementing the task
        "blink_task",     // Task name (strictly for debugging purposes)
        2048,             // Stack size (in bytes for ESP-IDF) allocated for this task
        NULL,             // Parameters passed to the task (none needed right now)
        1,                // Task priority (1 is a standard low priority)
        NULL              // Task handle (used to suspend/delete the task later; we don't need it)
    );

    printf("Task successfully passed to scheduler. app_main is now exiting.\n");
    // app_main ends here. The RTOS cleans up the boot task, but blink_task runs forever.
}