#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include "esp_err.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdcard.h"
#include "tfa9879.h"

#define AUDIO_BUFFER_SIZE 256

struct audio_buffer_t {
    uint8_t array[AUDIO_BUFFER_SIZE];
    SemaphoreHandle_t semaphore;
};

struct audio_buffer_t  buffer[2];

SemaphoreHandle_t sem_pipeline;
SemaphoreHandle_t sem_sd;
SemaphoreHandle_t sem_i2s;

bool pipeline_initialized = false;


esp_err_t audio_pipeline_init(void);
void audio_pipeline_play_file_task(const char *path);
void audio_pipeline_i2s_task(void* sem_end_task);
void audio_pipeline_sd_task(void* sem_end_task);




#endif //AUDIO_PIPELINE_H
