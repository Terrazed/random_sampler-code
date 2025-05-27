#include "wav_decoder.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdbool.h>
#include <string.h>
#include <strings.h>

esp_err_t set_wav_data_from_wav_header_array(struct wav_data_t* output, const uint8_t* wav_header_array, const uint32_t wav_header_array_size){
    const uint32_t riff = *(uint32_t*)"RIFF";
    bool riff_found = false;
    const uint32_t wave = *(uint32_t*)"WAVE";
    bool wave_found = false;
    const uint32_t data = *(uint32_t*)"data";
    bool data_found = false;

    struct wav_data_t temp_output;

    if(wav_header_array_size<44){
        ESP_LOGE("WAV", "wav header cannot be less than 44 bytes long");
        return ESP_ERR_INVALID_ARG;
    }

    for(int current_position = 0; (current_position < wav_header_array_size-4) || !(riff_found && wave_found && data_found); current_position++){

        //ESP_LOGI("WAV", "current pos : %u", current_position);

        if(!riff_found && riff == (*((uint32_t*)(&(wav_header_array[current_position]))))){
            //match riff
            //ESP_LOGI("WAV", "RIFF position : %u", current_position+4);
            riff_found = true;
            temp_output.file_size =
                (wav_header_array[current_position+7]<<24)+
                (wav_header_array[current_position+6]<<16)+
                (wav_header_array[current_position+5]<<8)+
                (wav_header_array[current_position+4]);
        }
        else if(!wave_found && wave == (*((uint32_t*)(&(wav_header_array[current_position]))))){
            //match wave
            //ESP_LOGI("WAV", "wave position : %u", current_position+4);
            wave_found = true;
            temp_output.format_type =
                (wav_header_array[current_position+13]<<8)+
                (wav_header_array[current_position+12]);
            temp_output.channel_number =
                (wav_header_array[current_position+15]<<8)+
                (wav_header_array[current_position+14]);
            temp_output.sample_rate =
                (wav_header_array[current_position+19]<<24)+
                (wav_header_array[current_position+18]<<16)+
                (wav_header_array[current_position+17]<<8)+
                (wav_header_array[current_position+16]);
            temp_output.bits_per_sample =
                (wav_header_array[current_position+27]<<8)+
                (wav_header_array[current_position+26]);

            //ESP_LOGI("WAV", "format_type: %u, channel_number : %u, sample_rate : %u, bits_per_sample : %u, ", temp_output.format_type, temp_output.channel_number, temp_output.sample_rate, temp_output.bits_per_sample);
        }
        else if(!data_found && data == (*((uint32_t*)(&(wav_header_array[current_position]))))){
            //match data
            ESP_LOGI("WAV", "data position : %u", current_position+4);
            data_found = true;
            temp_output.data_size =
                (wav_header_array[current_position+7]<<24)+
                (wav_header_array[current_position+6]<<16)+
                (wav_header_array[current_position+5]<<8)+
                (wav_header_array[current_position+4]);
            ESP_LOGI("WAV", "data size : %u", temp_output.data_size);
        }
    }

    if(riff_found && wave_found && data_found){
        memcpy(output, &temp_output, sizeof(temp_output));
        return ESP_OK;
    }
    ESP_LOGE("WAV", "wav header did not contain all needed informations");
    return ESP_ERR_NOT_FOUND;




}
