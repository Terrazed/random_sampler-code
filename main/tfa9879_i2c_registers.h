#ifndef TFA9879_I2C_REGISTERS_H
#define TFA9879_I2C_REGISTERS_H

#include <stdint.h>
#include <sys/cdefs.h>

//https://www.nxp.com/docs/en/data-sheet/TFA9879.pdf

/*--------------Register Addresses--------------*/
enum {
    TFA9879_DEVICE_CONTROL_REGISTER = 0x00,
    TFA9879_INTERFACE_CONTROL_REGISTER = 0x01,
    TFA9879_VOLUME_CONTROL_REGISTER = 0x13,
};


/*--------------Device control register (address 00h)--------------*/
typedef enum {
    SERIAL_INTERFACE_INPUT_1 = 0,
    SERIAL_INTERFACE_INPUT_2 = 1,
}tfa9879_input_sel_t;

typedef enum {
    OFF_MODE = 0,
    AMPLIFIER_MODE = 1,
}tfa9879_opmode_t;

typedef enum {
    RESET_INACTIVE = 0,
    RESET_ACTIVE = 1, // 1 is written to generate a reset, after which the RESET bit is automatically reset to 0
}tfa9879_reset_t;

typedef enum {
    POWER_DOWN_MODE = 0,
    OPERATING_MODE = 1, // dependent on opmode
}tfa9879_powerup_t;

union tfa9879_reg_serial_control{
    struct __packed{
        tfa9879_powerup_t powerup : 1;
        tfa9879_reset_t reset : 1;
        uint16_t reserved1 : 1;
        tfa9879_opmode_t opmode : 1;
        tfa9879_input_sel_t input_sel : 1;
        uint16_t reserved0 : 11;
    };
    uint16_t data;
};

/*--------------Serial interface control registers (addresses 01h and 03h )--------------*/

typedef enum {
    LEFT_CHANNEL = 0,
    RIGHT_CHANNEL = 1,
    LEFT_RIGHT_CHANNEL = 2,
}tfa9879_mono_sel_t;

typedef enum {
    SAMPLE_RATE_8K = 0,
    SAMPLE_RATE_11K025 = 1,
    SAMPLE_RATE_12K = 2,
    SAMPLE_RATE_16K = 3,
    SAMPLE_RATE_22K05 = 4,
    SAMPLE_RATE_24K = 5,
    SAMPLE_RATE_32K = 6,
    SAMPLE_RATE_44K1 = 7,
    SAMPLE_RATE_48K = 8,
    SAMPLE_RATE_64K = 9,
    SAMPLE_RATE_88K2 = 10,
    SAMPLE_RATE_96K = 11,
}tfa9879_i2s_fs_t;

typedef enum {
    MSB_JUSTIFED_UP_TO_24BITS = 2,
    I2S_DATA_UP_TO_24BITS = 3,
    LSB_JUSTIFED_16BITS = 4,
    LSB_JUSTIFED_18BITS = 5,
    LSB_JUSTIFED_20BITS = 6,
    LSB_JUSTIFED_24BITS = 7,
}tfa9879_i2s_set_t;

typedef enum {
    NO_SCK_INVERSION = 0,
    SCK_INVERSION = 1,
}tfa9879_sck_pol_t;

typedef enum {
    I2S_MODE = 0,
    PCM_IOM2_SHORT = 1,
    PCM_IOM2_LONG = 2,
}tfa9879_i_mode_t;

union tfa9879_reg_interface_control{
    struct __packed{
        tfa9879_i_mode_t i_mode : 2;
        tfa9879_sck_pol_t sck_pol : 1;
        tfa9879_i2s_set_t i2s_set : 3;
        tfa9879_i2s_fs_t i2s_fs : 4;
        tfa9879_mono_sel_t mono_sel : 2;
        uint16_t reserved0 : 4;
    };
    uint16_t data;
};


/*--------------Volume control register (address 13h)--------------*/

typedef enum {
    ZERO_CROSSING_VOLUME_CONTROL_DISABLED = 0,
    ZERO_CROSSING_VOLUME_CONTROL_ENABLED = 1,
}tfa9879_zr_crss_t;

union tfa9879_reg_volume_control{
    struct __packed{
        uint8_t volume : 8; //00h = +24dB, 01h = +23.5dB, ... BCh = -70dB, BDh = mute, ... FFh = mute
        uint16_t reserved1 : 4;
        tfa9879_zr_crss_t zr_crss : 1;
        uint16_t reserved0 : 3;
    };
    uint16_t data;
};



#endif //TFA9879_I2C_REGISTERS_H
