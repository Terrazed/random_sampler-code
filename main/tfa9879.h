#ifndef TFA9879_H
#define TFA9879_H

#include <stdint.h>
#include "driver/i2s_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "hal/i2s_types.h"
#include <stdint.h>
#include "esp_err.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_8             /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           GPIO_NUM_9             /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              I2C_NUM_0              /*!< I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ          400000                 /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                      /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000                   /*!< I2C master timeout in milliseconds */

#define TFA9879_SENSOR_ADDR         0x6C                   /*!< Address of the TFA9879 amplifier : 1 1 0 1 1 A2 A1 R/_W  A2=A1=0 */

#define I2S_CLK_IO                  GPIO_NUM_18            /*!< GPIO number used for I2S clock */
#define I2S_WS_IO                   GPIO_NUM_10            /*!< GPIO number used for I2S WS */
#define I2S_DOUT_IO                 GPIO_NUM_19            /*!< GPIO number used for I2S DATA OUT */


/**
 * @brief Read a sequence of bytes from a TFA9879 registers
 */
esp_err_t tfa9879_register_read(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint8_t *data, size_t len);

/**
 * @brief Write a sequence of bytes to a TFA9879 registers
 */
esp_err_t tfa9879_register_write_byte(i2c_master_dev_handle_t dev_handle, uint8_t reg_addr, uint16_t data);

/**
 * @brief i2c master initialization
 */
void i2c_master_init(i2c_master_bus_handle_t *bus_handle, i2c_master_dev_handle_t *dev_handle);

/**
 * @brief Initialize the TFA9879 amplifier
 */
esp_err_t tfa9879_init(void);


/**
 * @brief i2s master initialization
 */
void i2s_init(i2s_chan_handle_t * tx_chan);

/**
 * @brief tasks that send audio to tfa9879
 */
 void tfa9879_play(const uint8_t* const array, const uint32_t buffer_size);



#endif
