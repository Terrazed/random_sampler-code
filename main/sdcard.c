#include "sdcard.h"

esp_err_t sdcard_init(void){
    esp_err_t ret;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = VFS_FAT_MOUNT_DEFAULT_CONFIG();
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;

    ESP_LOGI("SD", "Initializing SD card");
    ESP_LOGI("SD", "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    //sd_pwr_ctrl_ldo_config_t ldo_config = {
    //    .ldo_chan_id = SD_PWR_IO
    //};
    //sd_pwr_ctrl_handle_t pwr_ctrl_handle = NULL;

    //ret = sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_ctrl_handle);
    //if (ret != ESP_OK) {
    //    ESP_LOGE("SD", "Failed to create a new on-chip LDO power control driver");
    //    return ret;
    //}
    //host.pwr_ctrl_handle = pwr_ctrl_handle;

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
