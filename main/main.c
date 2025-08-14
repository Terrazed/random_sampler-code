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


#define TIME_RECTIFICATION_MULT 0.005 // -5ms per second (based on measurement run_7)
#define MICROSECONDS_IN_SECOND (uint64_t)1000000


//static RTC_DATA_ATTR time_t current_time = 0;
static RTC_DATA_ATTR struct timeval current_time = {
    .tv_sec = 0,
    .tv_usec = 0,
};
static RTC_DATA_ATTR uint16_t n_sample = 0;


uint64_t get_random_time_us(){

    //get random number
    bootloader_random_enable();
    uint32_t u1 = esp_random();
    uint32_t u2 = esp_random();
    bootloader_random_disable();

    //convert them to float from 0 to 1
    float uf1 = (float)u1/(float)UINT32_MAX;
    if(uf1 == 0 || uf1 == 1){
        uf1 = 0.5;
    }
    float uf2 = (float)u2/(float)UINT32_MAX;
    if(uf2 == 0 || uf2 == 1){
        uf2 = 0.5;
    }

    //normal distribution parameters
    const float STD_DEV = 90 * MICROSECONDS_IN_SECOND; // 1.5min
    const float MEAN = 300 * MICROSECONDS_IN_SECOND; // 5min

    //box-muller transform
    float z0 = sqrtf(-2.0f * logf(uf1)) * cosf(2.0f * M_PI * uf2) * STD_DEV + MEAN;

    //limits 0-10min
    if(z0 <= 0 || z0 >= 600 * MICROSECONDS_IN_SECOND){
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
                ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(300 * MICROSECONDS_IN_SECOND)); // sleep 5min
                esp_deep_sleep_start();
            }
        }
    }

    //get a random number
    bootloader_random_enable();
    uint32_t random_number = esp_random();
    //bootloader_random_disable(); // leave bootloader_random enabled for i2s

    random_number = random_number % n_sample;

    char sample_name[256];
    sprintf(sample_name, "/sdcard/samples/sample_%u.wav", random_number);

    ESP_LOGE("PLAY", "playing %s", sample_name);

    audio_pipeline_play_file(sample_name);
}


void app_main(void)
{
    ESP_LOGI("APP","Program starting!");
    esp_err_t err;

    gettimeofday(&current_time, NULL);
    //time(&current_time);
    struct tm *temp_current_time_log = localtime(&(current_time.tv_sec));
    ESP_LOGE("TIME", "current time: %uh %umin %usec, %u/%u/%u", temp_current_time_log->tm_hour, temp_current_time_log->tm_min, temp_current_time_log->tm_sec, temp_current_time_log->tm_mday, temp_current_time_log->tm_mon, temp_current_time_log->tm_year);


    err = audio_pipeline_init();
    if(err != ESP_OK){
        ESP_LOGE("MAIN", "Failed to initialize pipeline!");
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1 * MICROSECONDS_IN_SECOND));
        esp_deep_sleep_start();
        return;
    }


    while (true) {
        play_random_sample();

        uint64_t time_to_sleep_us;

        gettimeofday(&current_time, NULL);
        struct tm *temp_current_time = localtime(&(current_time.tv_sec));

        if(temp_current_time->tm_hour >= 12 && temp_current_time->tm_hour < 13){ // sleeps 12h each day ("temp_current_time->tm_hour < 13" is for when time is rectified it doesn't sleep an other 12h)
            time_to_sleep_us = 12 * 60 * 60 * MICROSECONDS_IN_SECOND;
        }
        else {
            time_to_sleep_us = get_random_time_us();
        }

        uint64_t rectification_time_usec = time_to_sleep_us * TIME_RECTIFICATION_MULT;
        gettimeofday(&current_time, NULL);
        uint64_t rectified_time_usec = current_time.tv_sec * MICROSECONDS_IN_SECOND + current_time.tv_usec - rectification_time_usec;

        const struct timeval rectified_timeval = {
            .tv_sec = rectified_time_usec / MICROSECONDS_IN_SECOND,
            .tv_usec = rectified_time_usec % MICROSECONDS_IN_SECOND,
        };

        //ESP_LOGE("RECTIFICATION", "time_to_sleep_us: %llu", time_to_sleep_us);
        //ESP_LOGE("RECTIFICATION", "current_time s: %llu, current_time us: %lu", current_time.tv_sec, current_time.tv_usec);
        //ESP_LOGE("RECTIFICATION", "rectified_timeval s: %llu, rectified_timeval us: %lu", rectified_timeval.tv_sec, rectified_timeval.tv_usec);

        if(settimeofday(&rectified_timeval, NULL) != 0){
            ESP_LOGE("SETTIME", "Failed to set new time");
        }

        ESP_LOGE("DEEPSLEEP", "sleep for %u seconds...", (uint32_t)((uint64_t)time_to_sleep_us/(uint64_t)1000000));
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(time_to_sleep_us));
        esp_deep_sleep_start();
    }
}
