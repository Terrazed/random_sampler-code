#include "sdcard.h"
#include "esp_err.h"
#include <stdio.h>

sdmmc_card_t *card;

esp_err_t sdcard_init(void){
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = VFS_FAT_MOUNT_DEFAULT_CONFIG();

    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI("SD", "Initializing SD card");
    ESP_LOGI("SD", "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = MOSI_SD_IO,
        .miso_io_num = MISO_SD_IO,
        .sclk_io_num = CLK_SD_IO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0
    };

    ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE("SD", "Failed to initialize bus.");
        return ret;
    }

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CS_SD_IO;
    slot_config.host_id = host.slot;

    ESP_LOGI("SD", "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
    if(ret != ESP_OK){
        if (ret == ESP_FAIL) {
            ESP_LOGE("SD", "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        }
        else {
            ESP_LOGE("SD", "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI("SD", "Filesystem mounted");

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    return ret;
}

esp_err_t sdcard_open_file(FILE *file, const char *path, const char *rw){

    ESP_LOGI("SD", "Opening file %s in mode %s", path, rw);
    file = fopen(path, rw);
    if (file == NULL) {
        ESP_LOGE("SD", "Failed to open file");
        return ESP_FAIL;
    }
    return ESP_OK;
}
esp_err_t sdcard_close_file(FILE *file){

    if(fclose(file) != 0){
        ESP_LOGE("SD", "Failed to close file");
        return ESP_FAIL;
    }
    return ESP_OK;
}


esp_err_t sdcard_read_bytes(FILE *file, char * output_buffer, uint32_t n_byte, bool offset_file_after){

    ESP_LOGI("SD", "caca");
    fgets(output_buffer, n_byte, file);
    ESP_LOGI("SD", "pipi");
    if(offset_file_after){
        fseek(file, n_byte, SEEK_CUR);
    }
    ESP_LOGI("SD", "popo");

    return ESP_OK;
}

esp_err_t s_example_read_file(const char *path)
{
    ESP_LOGI("SD", "Reading file %s", path);
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        ESP_LOGE("SD", "Failed to open file for reading");
        return ESP_FAIL;
    }
    //char line[10][10];
    //for (int16_t j = 0; j<10; j++){
    //    fgets(line[j], 10, f);
    //}
    ESP_LOGI("SD", "Read from file: ");
    uint32_t counter = 0;
    while(feof(f) == 0){
        char a[16];
        fread(a,1,16,f);
        for (int16_t i = 0; i<16; i++){
            printf("0x%02x, ",a[i]);
        }
        printf("\n");
        counter += 16;
    }
    ESP_LOGI("SD", "end read, counter: %u", counter);

    fclose(f);

    //ESP_LOGI(TAG, "Read from file: '%s'", line);

    //ESP_LOGI("SD", "Read from file: ");
    //for (int16_t i = 0; i<10; i++){
    //    for (int16_t i2 = 0; i2<10; i2++){
    //        printf("%02X ", line[i][i2]);
    //    }
    //}

    return ESP_OK;
}
