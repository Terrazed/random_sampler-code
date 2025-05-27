#include "audio_pipeline.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "sdcard.h"
#include "tfa9879.h"
#include "wav_decoder.h"
#include <string.h>


struct audio_buffer_t  buffer[2];
SemaphoreHandle_t sem_pipeline;
SemaphoreHandle_t sem_sd;
SemaphoreHandle_t sem_i2s;

QueueHandle_t msgq_sd;
QueueHandle_t msgq_i2s;

TaskHandle_t i2s_task = NULL;
TaskHandle_t sd_task = NULL;

bool pipeline_initialized = false;

esp_err_t audio_pipeline_init(void){
    esp_err_t ret;
    if(!pipeline_initialized){
        // init pipeline
        ESP_LOGI("PIPELINE", "Creating semaphores");
        for(uint8_t i=0; i<2; i++){
            buffer[i].semaphore = xSemaphoreCreateBinary();
            xSemaphoreGive(buffer[i].semaphore);
        }

        sem_pipeline = xSemaphoreCreateBinary();
        if(sem_pipeline == NULL){
            ESP_LOGE("PIPELINE", "sem_pipeline not created - insufficient heap");
        }
        xSemaphoreGive(sem_pipeline);

        sem_sd = xSemaphoreCreateBinary();
        if(sem_sd == NULL){
            ESP_LOGE("PIPELINE", "sem_sd not created - insufficient heap");
        }
        xSemaphoreGive(sem_sd);

        sem_i2s = xSemaphoreCreateBinary();
        if(sem_i2s == NULL){
            ESP_LOGE("PIPELINE", "sem_i2s not created - insufficient heap");
        }
        xSemaphoreGive(sem_i2s);

        msgq_sd = xQueueCreate(MSGQ_SIZE, sizeof(struct audio_buffer_t*));
        if (msgq_sd == NULL)
        {
            ESP_LOGE("PIPELINE","Failed to create msgq_sd = %p", msgq_sd);
        }
        msgq_i2s = xQueueCreate(MSGQ_SIZE, sizeof(struct audio_buffer_t*));
        if (msgq_i2s == NULL)
        {
            ESP_LOGE("PIPELINE","Failed to create msgq_i2s = %p", msgq_i2s);
        }

        // init SD card
        ret = sdcard_init();
        if(ret != ESP_OK){
            ESP_LOGE("PIPELINE", "Failed to initialize SD card");
            return ret;
        }
        sdcard_power_up();

        // init tfa9879
        ret = tfa9879_init();
        if(ret != ESP_OK){
            ESP_LOGE("PIPELINE", "Failed to initialize tfa9879 card");
            return ret;
        }
        tfa9879_power_up();

        pipeline_initialized = true;
    }

    return ESP_OK;
}


void audio_pipeline_play_file(const char *path){
    // check if pipeline is initilized
    if(!pipeline_initialized){
        ESP_LOGE("PIPELINE", "Pipeline not initialized !");
        return;
    }

    ESP_LOGI("PIPELINE","lock pipeline");
    xSemaphoreTake(sem_pipeline, portMAX_DELAY);



    buffer[0].next_buffer = &(buffer[1]);
    buffer[1].next_buffer = &(buffer[0]);

    ESP_LOGI("PIPELINE","open sd");
    sdcard_open(path);

    ESP_LOGI("PIPELINE","reading header");
    uint8_t wav_info[88];
    sdcard_read(wav_info, sizeof(wav_info));

    struct wav_data_t wav_data;
    set_wav_data_from_wav_header_array(&wav_data, wav_info, sizeof(wav_info));

    //ESP_LOGI("PIPELINE", "%s :", path);
    //for(int i = 0; i<sizeof(wav_info); i++){
    //    printf("%2u: %02x/%2c\n", i+1, wav_info[i], wav_info[i]);
    //}
    //printf("\r\n");

    xSemaphoreTake(sem_sd, portMAX_DELAY);
    xTaskCreate(audio_pipeline_sd_task, "pipe_sd_task", 16384, &sem_sd, 5, &sd_task);

    xSemaphoreTake(sem_i2s, portMAX_DELAY);
    xTaskCreate(audio_pipeline_i2s_task, "pipe_i2s_task", 16384, &sem_i2s, 5, &i2s_task);

    if(xQueueSend(msgq_sd, &(buffer[1].next_buffer), 0) == errQUEUE_FULL){
        ESP_LOGE("PIPELINE", "no space in buffer !");
    }


    ESP_LOGI("PIPELINE","wait sd loop to end");
    xSemaphoreTake(sem_sd, portMAX_DELAY);
    xSemaphoreGive(sem_sd);

    ESP_LOGI("PIPELINE","close sd");
    sdcard_close();

    ESP_LOGI("PIPELINE","wait i2s loop to end");
    xSemaphoreTake(sem_i2s, portMAX_DELAY);
    xSemaphoreGive(sem_i2s);

    ESP_LOGI("PIPELINE","free pipeline");
    xSemaphoreGive(sem_pipeline);
}

void audio_pipeline_i2s_task(void* sem_end_task){
    SemaphoreHandle_t* sem = sem_end_task;

    ESP_ERROR_CHECK(i2s_channel_enable(i2s_chan_handle));

    bool continue_loop = true;
    while(continue_loop){
        struct audio_buffer_t* currentbuffer;
        xQueueReceive(msgq_i2s, &currentbuffer, portMAX_DELAY);

        xSemaphoreTake(currentbuffer->semaphore, portMAX_DELAY);

        if(currentbuffer->next_buffer == NULL){
            continue_loop = false;
        }
        else{
            if(xQueueSend(msgq_sd, &(currentbuffer->next_buffer), 0) == errQUEUE_FULL){
                ESP_LOGE("I2S_TASK", "no space in buffer !");
            }
            tfa9879_play(currentbuffer->array, sizeof(currentbuffer->array));
        }

        xSemaphoreGive(currentbuffer->semaphore);


    }

    ESP_ERROR_CHECK(i2s_channel_disable(i2s_chan_handle));

    //notify task end
    xSemaphoreGive(*sem);
    vTaskDelete(i2s_task);
}

void audio_pipeline_sd_task(void* sem_end_task){
    SemaphoreHandle_t* sem = sem_end_task;

    bool continue_loop = true;
    while(continue_loop){
        struct audio_buffer_t* currentbuffer;
        xQueueReceive(msgq_sd, &currentbuffer, portMAX_DELAY);

        xSemaphoreTake(currentbuffer->semaphore, portMAX_DELAY);
        bool eof = sdcard_read(currentbuffer->array, sizeof(currentbuffer->array));


        if(eof != 0){
            continue_loop = false;
            currentbuffer->next_buffer = NULL;
        }

        xSemaphoreGive(currentbuffer->semaphore);

        if(xQueueSend(msgq_i2s, &currentbuffer, 0) == errQUEUE_FULL){
            ESP_LOGE("SD_TASK", "no space in buffer !");
        }
    }

    //notify task end
    xSemaphoreGive(*sem);
    vTaskDelete(sd_task);
}
