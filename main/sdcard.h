#ifndef SDCARD_H
#define SDCARD_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "esp_log.h"
#include "sd_pwr_ctrl_by_on_chip_ldo.h"
#include <sys/unistd.h>


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

esp_err_t sdcard_open_file(FILE *file, const char *path, const char *rw);
esp_err_t sdcard_close_file(FILE *file);

esp_err_t sdcard_read_bytes(FILE *file, char * output_buffer, uint32_t n_byte, bool offset_file_after);

esp_err_t s_example_read_file(const char *path);

#endif
