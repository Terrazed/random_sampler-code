/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdint.h>
#include "driver/i2c_types.h"
#include "esp_log.h"
#include "esp_private/gpio.h"
#include "esp_err.h"
#include "sdcard.h"
#include "soc/gpio_num.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "tfa9879.h"

#define GPIO_POWER_SD GPIO_NUM_5
#define GPIO_POWER_AMPLIFIER GPIO_NUM_4

void app_main(void)
{
    ESP_LOGI("APP","Program starting!");

    esp_err_t err;

    // GPIO ---------------------------------------------------------------------

    ESP_LOGI("GPIO","configuring GPIO_POWER_SD");
    err = gpio_func_sel(GPIO_POWER_SD, 1);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to configure GPIO_POWER_SD");
        return;
    }
    err = gpio_set_direction(GPIO_POWER_SD, GPIO_MODE_OUTPUT);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to set direction for GPIO_POWER_SD");
        return;
    }
    err = gpio_set_level(GPIO_POWER_SD, 1);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to set level for GPIO_POWER_SD");
        return;
    }
    ESP_LOGI("GPIO","done configuring GPIO_POWER_SD");

    ESP_LOGI("GPIO","configuring GPIO_POWER_AMPLIFIER");
    err = gpio_func_sel(GPIO_POWER_AMPLIFIER, 1);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to configure GPIO_POWER_AMPLIFIER");
        return;
    }
    err = gpio_set_direction(GPIO_POWER_AMPLIFIER, GPIO_MODE_OUTPUT);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to set direction for GPIO_POWER_AMPLIFIER");
        return;
    }
    err = gpio_set_level(GPIO_POWER_AMPLIFIER, 1);
    if(err != ESP_OK) {
        ESP_LOGE("GPIO","Failed to set level for GPIO_POWER_AMPLIFIER");
        return;
    }
    ESP_LOGI("GPIO","done configuring GPIO_POWER_AMPLIFIER");

    // I2C ----------------------------------------------------------------------

    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;

    i2c_master_init(&bus_handle, &dev_handle);
    ESP_LOGI("I2C","done initializing I2C bus");

    tfa9879_register_write_byte(dev_handle, 0x00, 0x0008);
    tfa9879_register_write_byte(dev_handle, 0x00, 0x0009);
    tfa9879_register_write_byte(dev_handle, 0x01, 0x08D0);
    tfa9879_register_write_byte(dev_handle, 0x13, 0x1080);

    uint8_t data[2];
    for(int i = 0; i<=0x15; i++){
        ESP_ERROR_CHECK(tfa9879_register_read(dev_handle, i, data, 0x02));
        ESP_LOGI("I2C","reg 0x%02x value 0x%02x%02x", i, data[0], data[1]);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // I2S ----------------------------------------------------------------------

    ESP_LOGI("I2S", "i2s config");
    i2s_chan_handle_t i2s_chan_handle;
    i2s_init(&i2s_chan_handle);
    //ESP_LOGI("I2S", "i2s write task");
    //xTaskCreate(i2s_write_task, "i2s_write_task", 4096, &i2s_chan_handle, 5, NULL); //play the audio in loop



    // SD card ------------------------------------------------------------------

    sdcard_init();


    // MAIN LOOP ----------------------------------------------------------------

    s_example_read_file("/sdcard/samples/sample_0.wav");

/*
    FILE *f = NULL;
    sdcard_open_file(f, "/sdcard/samples/sample_0.wav", "r");

    char buffer[16+1];

    for (int j = 0; j<32; j++){
        sdcard_read_bytes(f, buffer, 16, true);

        ESP_LOGI("read", "%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8], buffer[9], buffer[10], buffer[11], buffer[12], buffer[13], buffer[14], buffer[15]);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    sdcard_close_file(f);
*/
    while (true) {

        ESP_LOGI("main", "main loop");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
