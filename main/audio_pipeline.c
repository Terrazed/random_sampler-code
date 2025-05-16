#include "audio_pipeline.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/idf_additions.h"
#include "sdcard.h"
#include "tfa9879.h"


struct audio_buffer_t  buffer[2];
SemaphoreHandle_t sem_pipeline;
SemaphoreHandle_t sem_sd;
SemaphoreHandle_t sem_i2s;
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

    ESP_LOGI("PIPELINE","open sd");
    sdcard_open(path);

    xSemaphoreTake(buffer[0].semaphore, portMAX_DELAY);
    ESP_LOGI("PIPELINE","sd load in buffer 0");
    sdcard_read(buffer[0].array, sizeof(buffer[0].array));
    xSemaphoreGive(buffer[0].semaphore);
    xSemaphoreTake(buffer[1].semaphore, portMAX_DELAY);

    xSemaphoreTake(sem_i2s, portMAX_DELAY);
    ESP_LOGI("PIPELINE","launch i2s loop");
    xTaskCreate(audio_pipeline_i2s_task, "audio_pipeline_i2s_task", 16384, &sem_i2s, 5, NULL);

    xSemaphoreTake(sem_sd, portMAX_DELAY);
    ESP_LOGI("PIPELINE","launch sd loop");
    xTaskCreate(audio_pipeline_sd_task, "audio_pipeline_sd_task", 16384, &sem_sd, 5, NULL);

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

    vTaskDelete(NULL);
}

void audio_pipeline_i2s_task(void* sem_end_task){
    SemaphoreHandle_t* sem = sem_end_task;

    xSemaphoreTake(buffer[0].semaphore, portMAX_DELAY);
    ESP_ERROR_CHECK(i2s_channel_enable(i2s_chan_handle));

    while(true){
        ESP_LOGI("PIPELINE","i2s play buffer 0");
        tfa9879_play(buffer[0].array, sizeof(buffer[0].array));
        xSemaphoreTake(buffer[1].semaphore, portMAX_DELAY);
        xSemaphoreGive(buffer[0].semaphore);

        ESP_LOGI("PIPELINE","i2s play buffer 1");
        tfa9879_play(buffer[1].array, sizeof(buffer[1].array));
        xSemaphoreTake(buffer[0].semaphore, portMAX_DELAY);
        xSemaphoreGive(buffer[1].semaphore);
    }
    ESP_ERROR_CHECK(i2s_channel_disable(i2s_chan_handle));

    //notify task end
    xSemaphoreGive(*sem);
}

void audio_pipeline_sd_task(void* sem_end_task){
    SemaphoreHandle_t* sem = sem_end_task;

    while(true){
        ESP_LOGI("PIPELINE","sd load in buffer 1");
        sdcard_read(buffer[1].array, sizeof(buffer[1].array));
        xSemaphoreGive(buffer[1].semaphore);
        xSemaphoreTake(buffer[0].semaphore, portMAX_DELAY);

        ESP_LOGI("PIPELINE","sd load in buffer 0");
        sdcard_read(buffer[0].array, sizeof(buffer[0].array));
        xSemaphoreGive(buffer[0].semaphore);
        xSemaphoreTake(buffer[1].semaphore, portMAX_DELAY);
    }

    //notify task end
    xSemaphoreGive(*sem);
    vTaskDelete(NULL);
}
