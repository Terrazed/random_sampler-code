#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include "esp_err.h"

#define AUDIO_BUFFER_SIZE 256

uint8_t audio_buffer[2][AUDIO_BUFFER_SIZE];

esp_err_t audio_pipeline_init(void);

esp_err_t audio_pipeline_start(/*FILE FileToRead */);


#endif //AUDIO_PIPELINE_H
