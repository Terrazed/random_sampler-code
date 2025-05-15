#ifndef SDCARD_H
#define SDCARD_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include <stdint.h>
#include <sys/unistd.h>
#include "soc/gpio_num.h"
#include "esp_private/gpio.h"


// SD SPI pins
#define CLK_SD_IO GPIO_NUM_6
#define CS_SD_IO GPIO_NUM_3
#define MISO_SD_IO GPIO_NUM_7
#define MOSI_SD_IO GPIO_NUM_2

// SD power pin
#define SD_PWR_IO GPIO_NUM_5

// SD card mount name
#define MOUNT_POINT "/sdcard"

esp_err_t sdcard_init(void);

esp_err_t sdcard_open(const char *path);
esp_err_t sdcard_read(uint8_t* const buffer, const uint32_t buffer_size);
esp_err_t sdcard_close();

esp_err_t sdcard_power_up();
esp_err_t sdcard_power_down();

esp_err_t s_example_read_file(const char *path);

#endif
