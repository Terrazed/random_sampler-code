/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
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
#include "esp_sleep.h"

#include "audio_pipeline.h"
#include "sdcard.h"



static RTC_DATA_ATTR time_t current_time = 0;
static RTC_DATA_ATTR uint16_t n_sample = 0;


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

void play_random_sample(){
    if(n_sample == 0){ // count the number of samples in the /samples folder
        n_sample = sdcard_count_samples("/samples");
        if(n_sample == 0){
            ESP_LOGE("MAIN", "could not find any samples, check if the samples are in \"/sdcard/samples\" and named \"sample_x.wav\" where x is a number");
            while(true){
                ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(300000000)); // sleep 5min
                esp_deep_sleep_start();
            }
        }
    }

    //get a random number
    bootloader_random_enable();
    uint32_t random_number = esp_random();
    //bootloader_random_disable(); // leave bootloader_random enabled for i2s

    random_number = (uint32_t)((uint64_t)random_number * (uint64_t)n_sample / (uint64_t)UINT32_MAX);

    char sample_name[256];
    sprintf(sample_name, "/sdcard/samples/sample_%u.wav", random_number);

    //ESP_LOGE("test", "playing %s", sample_name);

    audio_pipeline_play_file(sample_name);
}


void app_main(void)
{
    ESP_LOGI("APP","Program starting!");
    esp_err_t err;


    time(&current_time);
    //struct tm *temp_current_time = localtime(&current_time);
    //ESP_LOGW("time", "current time: %uh %umin %usec, %u/%u/%u", temp_current_time->tm_hour, temp_current_time->tm_min, temp_current_time->tm_sec, temp_current_time->tm_mday, temp_current_time->tm_mon, temp_current_time->tm_year);


    err = audio_pipeline_init();
    if(err != ESP_OK){
        ESP_LOGE("MAIN", "Failed to initialize pipeline!");
        return;
    }


    while (true) {

        play_random_sample();

        uint64_t time_to_sleep_us;

        struct tm *temp_current_time = localtime(&current_time);
        if(temp_current_time->tm_hour < 12){

            time_to_sleep_us = get_random_time_us();
        }
        else {
            time_to_sleep_us = 43200000000; //12h in us
        }

        ESP_LOGI("DEEPSLEEP", "sleep for %u seconds...", (uint32_t)((uint64_t)time_to_sleep_us/(uint64_t)1000000));
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(time_to_sleep_us));
        esp_deep_sleep_start();
    }
}
