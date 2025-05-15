#include "tfa9879.h"

#include "esp_log.h"

#include "test_audio.h" // audio file for testing bc the sd card doesnt work yet
#include <stdint.h>

esp_err_t tfa9879_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len){
    return i2c_master_transmit_receive(dev_handle, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

esp_err_t tfa9879_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint16_t data){
    uint8_t write_buf[3] = {reg_addr, (uint8_t)(data>>8), (uint8_t)(data)};
    return i2c_master_transmit(dev_handle, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle){
    static i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_MASTER_NUM,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false, //TODO: Check if pullup are strong enough to handle high speed I2C
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, bus_handle));

    static i2c_device_config_t dev_config = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = TFA9879_SENSOR_ADDR,
            .scl_speed_hz = I2C_MASTER_FREQ_HZ,
        };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(*bus_handle, &dev_config, dev_handle));
}

esp_err_t tfa9879_init(void){

    //TODO
    return ESP_OK;
}


void i2s_init(i2s_chan_handle_t * tx_chan){
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&tx_chan_cfg, tx_chan, NULL));

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(16000),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = I2S_CLK_IO,
            .ws   = I2S_WS_IO,
            .dout = I2S_DOUT_IO,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(*tx_chan, &tx_std_cfg));
}


void tfa9879_play(const uint8_t* const array, const uint32_t buffer_size){

    // TODO

}

void i2s_write_task(void *args){

    i2s_chan_handle_t *tx_chan = (i2s_chan_handle_t *)args;

    size_t w_bytes = 0;

    uint8_t *w_buf = (uint8_t *)(test_audio+80);


    //while (w_bytes == DATA_SIZE) {
    //    /* Here we load the target buffer repeatedly, until all the DMA buffers are preloaded */
    //    ESP_ERROR_CHECK(i2s_channel_preload_data(*tx_chan, w_buf, DATA_SIZE, &w_bytes));
    //}

    /* Enable the TX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(*tx_chan));

    while (1) {
        /* Write i2s data */
        if (i2s_channel_write(*tx_chan, w_buf, DATA_SIZE, &w_bytes, 10000) == ESP_OK) {
            ESP_LOGI("I2S", "Write Task: i2s write %d bytes", w_bytes);
        } else {
            ESP_LOGE("I2S", "Write Task: i2s write failed");
        }
    }

    ESP_ERROR_CHECK(i2s_channel_disable(*tx_chan));
    vTaskDelete(NULL);
}
