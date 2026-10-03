#include <stdio.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// The XIAO ESP32-C6
#define BLINK_GPIO 15
#define ANALOG_GPIO 0

void app_main(void)
{
    // 1. Hardware Initialization
    // Reset the pin to its default state, clearing any previous configurations
    gpio_reset_pin(BLINK_GPIO);
    gpio_reset_pin(ANALOG_GPIO);

    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT); // Set the GPIO pad to output mode so we can drive voltage high/low
    gpio_set_direction(ANALOG_GPIO, GPIO_MODE_INPUT); // Set the GPIO pad to input mode so we can read voltage levels

    adc_oneshot_unit_handle_t adc1;
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc1));

    adc_oneshot_chan_cfg_t channel_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc1, ADC_CHANNEL_0, &channel_cfg));

    int led_state = 0;

    // 2. FreeRTOS Task Loop
    // This is a standard FreeRTOS infinite loop. Because this runs inside a task,
    // it will run forever while the rest of the OS operates in the background.
    while (1)
    {
        // Write the current state to the pin.
        // Note: The XIAO LED is active-low (0 = ON, 1 = OFF).
        gpio_set_level(BLINK_GPIO, led_state);
        int raw;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1, ADC_CHANNEL_0, &raw));
         // Print to the ESP-IDF monitor (UART console)
        printf("LED State: %s, Analog Level: %d\n", led_state == 0 ? "ON" : "OFF", raw);

        // Toggle state for the next iteration
        led_state = !led_state;

        // 3. Task Yielding (CRITICAL CONCEPT)
        // Never use a standard C busy-wait delay (like a huge for-loop) in FreeRTOS.
        // vTaskDelay tells the FreeRTOS scheduler: "Put this task to sleep,
        // and let other tasks (like Wi-Fi or Bluetooth) use the CPU."
        // pdMS_TO_TICKS() converts our desired milliseconds into FreeRTOS OS ticks.
        vTaskDelay(pdMS_TO_TICKS(raw));
    }
}