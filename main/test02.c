/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <math.h>
#include <stdint.h>
#include <sys/_timeval.h>
#include "driver/i2c_types.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "esp_log_level.h"
#include "esp_private/gpio.h"
#include "esp_err.h"
#include "soc/gpio_num.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <sys/time.h>
#include "esp_random.h"
#include "bootloader_random.h"

#include "audio_pipeline.h"
#include "esp_sleep.h"


static RTC_DATA_ATTR time_t current_time;


uint64_t get_random_time_us(){

    //get random number
    bootloader_random_enable();
    uint32_t u1 = esp_random();
    uint32_t u2 = esp_random();
    bootloader_random_disable();

    //convert them to float
    float uf1 = (float)u1/(float)UINT32_MAX;
    if(uf1 == 0 || uf1 == 1){
        uf1 = 0.5;
    }
    float uf2 = (float)u2/(float)UINT32_MAX;
    if(uf2 == 0|| uf2 == 1){
        uf2 = 0.5;
    }

    //normal distribution parameters
    const float STD_DEV = 90 * 1000000; // 1.5min
    const float MEAN = 300 * 1000000; // 5min

    //box-muller transform
    float z0 = sqrtf(-2.0f * logf(uf1)) * cosf(2.0f * M_PI * uf2) * STD_DEV + MEAN;

    //limits
    if(z0 <= 0 || z0 >= 600000000){
        z0 = MEAN;
    }

    //convert to uint32_t
    uint64_t output = z0;

    //ESP_LOGI("RANDOM", "output : %u", (uint32_t)output);




    return output;
}


void app_main(void)
{
    ESP_LOGI("APP","Program starting!");
    esp_err_t err;


    time(&current_time);
    struct tm *temp_current_time = localtime(&current_time);
    ESP_LOGW("time", "current time: %uh %umin %usec, %u/%u/%u", temp_current_time->tm_hour, temp_current_time->tm_min, temp_current_time->tm_sec, temp_current_time->tm_mday, temp_current_time->tm_mon, temp_current_time->tm_year);



    //for(uint16_t i = 0; i<1000; i++){
    //    get_random_time_us();
    //}




    err = audio_pipeline_init();
    if(err != ESP_OK){
        ESP_LOGE("MAIN", "Failed to initialize pipeline!");
        return;
    }
    //vTaskDelay(1000 / portTICK_PERIOD_MS);



    while (true) {




        //audio_pipeline_play_file("/sdcard/samples/veridis_quo_16000.wav");
        //audio_pipeline_play_file("/sdcard/samples/mockingbird_16000.wav");
        //audio_pipeline_play_file("/sdcard/samples/king_of_speed_16000.wav");
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo.wav");
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo(1).wav");
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo(2).wav");
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo(3).wav");
        audio_pipeline_play_file("/sdcard/samples/sample_0.wav");

        uint64_t time_to_sleep_us;

        struct tm *temp_current_time = localtime(&current_time);
        if(temp_current_time->tm_hour < 12){

            time_to_sleep_us = get_random_time_us();
        }
        else {
            time_to_sleep_us = 43200000000; //12h
        }

        ESP_LOGI("DEEPSLEEP", "sleep for %u seconds...", (uint32_t)((uint64_t)time_to_sleep_us/(uint64_t)1000000));
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(time_to_sleep_us));
        esp_deep_sleep_start();
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo_auto.wav");
        //audio_pipeline_play_file("/sdcard/samples/veridis_quo_24bit_auto.wav");
        //audio_pipeline_play_file("/sdcard/samples/mockingbird_auto.wav");
        //audio_pipeline_play_file("/sdcard/samples/king_of_speed_auto.wav");





        //vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
