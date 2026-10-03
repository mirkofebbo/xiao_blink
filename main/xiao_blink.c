#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define BOOT_BUTTON_GPIO 9 // XIAO ESP32-C6 boot button
#define BLINK_GPIO 15      // XIAO ESP32-C6 user LED

// ==== QUEUE AND TASK DEFINITIONS ====
QueueHandle_t blink_delay_queue;

// ==== BUTTON TASK ====
void button_task(void *pvParameter)
{

    // Initialize the boot button GPIO
    gpio_reset_pin(BOOT_BUTTON_GPIO);
    gpio_set_direction(BOOT_BUTTON_GPIO, GPIO_MODE_INPUT);

    gpio_set_pull_mode(BOOT_BUTTON_GPIO, GPIO_PULLUP_ONLY);

    int current_delay = 1000;
    int last_button_state = 1; // need to double check this

    while (1)
    {
        int button_state = gpio_get_level(BOOT_BUTTON_GPIO);

        // Failing edge detection
        if (button_state == 0 && last_button_state == 1)
        {

            current_delay = (current_delay == 1000) ? 100 : 1000;
            printf("[BUTTON_TASK] set delay: %d\n", current_delay);

            // Sending to the queue
            xQueueSend(blink_delay_queue, &current_delay, 0); // params: queue to write to, pointer to data, block time

            // Software debounce
            vTaskDelay(pdMS_TO_TICKS(50));
        }

        last_button_state = button_state;

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ==== BLINK TASK ====
void blink_task(void *pvParameter)
{
    // Hardware initialization inside the task.
    // It is often cleaner for a task to configure the peripherals it exclusively owns.
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;
    int task_delay = 1000;

    // The task execution loop
    while (1)
    {

        // Check button queue for new delay value
        if (xQueueReceive(blink_delay_queue, &task_delay, 0) == pdTRUE)
        {
            printf("[BLINK_TASK] new delay: %d ms\n", task_delay);
        }

        gpio_set_level(BLINK_GPIO, led_state);

        // Print out the state. This helps us verify the scheduler is running this task.
        printf("Blink Task: LED State %s\n", led_state == 0 ? "ON" : "OFF");

        led_state = !led_state;

        // Yield to the scheduler. Without this delay, the task would starve lower-priority tasks.
        vTaskDelay(pdMS_TO_TICKS(task_delay));
    }

    // Safety net: If we ever broke out of the while(1) loop, we must delete the task.
    // NULL tells the OS "delete the task that is currently running".
    vTaskDelete(NULL);
}

// ==== APP MAIN ====
void app_main(void)
{
    printf("System booted. Spawning blink task...\n");

    blink_delay_queue = xQueueCreate(5, sizeof(int));

    if (blink_delay_queue == NULL)
    {
        printf("[APP_MAIN] Failed to create blink delay queue\n");
        return;
    }

    xTaskCreate(button_task, "button_task", 2048, NULL, 1, NULL);
    // Tell FreeRTOS to allocate memory and schedule our new task
    xTaskCreate(
        blink_task,   // Pointer to the function implementing the task
        "blink_task", // Task name (strictly for debugging purposes)
        2048,         // Stack size (in bytes for ESP-IDF) allocated for this task
        NULL,         // Parameters passed to the task (none needed right now)
        1,            // Task priority (1 is a standard low priority)
        NULL          // Task handle (used to suspend/delete the task later; we don't need it)
    );

    printf("Task successfully passed to scheduler. app_main is now exiting.\n");
    // app_main ends here. The RTOS cleans up the boot task, but blink_task runs forever.
}