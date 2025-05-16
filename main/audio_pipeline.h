#ifndef AUDIO_PIPELINE_H
#define AUDIO_PIPELINE_H

#include "esp_err.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "sdcard.h"
#include "tfa9879.h"

#define AUDIO_BUFFER_SIZE 16000

struct audio_buffer_t {
    uint8_t array[AUDIO_BUFFER_SIZE];
    SemaphoreHandle_t semaphore;
};

extern struct audio_buffer_t  buffer[2];

extern SemaphoreHandle_t sem_pipeline;
extern SemaphoreHandle_t sem_sd;
extern SemaphoreHandle_t sem_i2s;

extern bool pipeline_initialized;


esp_err_t audio_pipeline_init(void);
void audio_pipeline_play_file(const char *path);
void audio_pipeline_i2s_task(void* sem_end_task);
void audio_pipeline_sd_task(void* sem_end_task);




#endif //AUDIO_PIPELINE_H
