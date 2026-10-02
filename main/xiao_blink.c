#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// The XIAO ESP32-C6 user LED is connected to GPIO 15
#define BLINK_GPIO 15

void app_main(void)
{
    // 1. Hardware Initialization
    // Reset the pin to its default state, clearing any previous configurations
    gpio_reset_pin(BLINK_GPIO);
    
    // Set the GPIO pad to output mode so we can drive voltage high/low
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    int led_state = 0;

    // 2. FreeRTOS Task Loop
    // This is a standard FreeRTOS infinite loop. Because this runs inside a task,
    // it will run forever while the rest of the OS operates in the background.
    while (1) {
        // Write the current state to the pin. 
        // Note: The XIAO LED is active-low (0 = ON, 1 = OFF).
        gpio_set_level(BLINK_GPIO, led_state);
        
        // Print to the ESP-IDF monitor (UART console)
        printf("LED State: %s\n", led_state == 0 ? "ON" : "OFF");
        
        // Toggle state for the next iteration
        led_state = !led_state; 

        // 3. Task Yielding (CRITICAL CONCEPT)
        // Never use a standard C busy-wait delay (like a huge for-loop) in FreeRTOS.
        // vTaskDelay tells the FreeRTOS scheduler: "Put this task to sleep, 
        // and let other tasks (like Wi-Fi or Bluetooth) use the CPU."
        // pdMS_TO_TICKS() converts our desired milliseconds into FreeRTOS OS ticks.
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}