#ifndef WAV_DECODER_H
#define WAV_DECODER_H

#include "esp_err.h"
#include <stdint.h>

struct wav_data_t {
    uint32_t file_size; //Size of the overall file
    uint16_t format_type; //Type of format (1 is PCM)
    uint16_t channel_number; //Number of Channels
    uint32_t sample_rate; //Sample Rate (sample/second)
    uint32_t bits_per_sample; //Number of bits per sample
    uint32_t data_size; //Size of the datas in bytes
};

esp_err_t set_wav_data_from_wav_header_array(struct wav_data_t* output, const uint8_t* wav_header_array, const uint32_t wav_header_array_size);

#endif //WAV_DECODER_H
