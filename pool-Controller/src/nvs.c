/* Non-Volatile Storage (NVS) Read and Write a Value - Example

   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "nvsLocal.h"
esp_err_t change_nvs_value(char * ,int16_t, nvs_handle_t );
void get_Pool_timer_times(nvs_timer_t *times);
esp_err_t err;
esp_err_t write_timer_to_mvs(nvs_times_t * timer)
{
    nvs_handle_t my_handle;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else 
    {
        if(timer->pump)
        {
            err = change_nvs_value("pumpOnHr",timer->timerOnHour,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("pumpOnMin",timer->timerOnMinute,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("pumpOnAmPm",timer->timerOnAmPm,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("pumpOffHr",timer->timerOffHour,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("pumpOffMin",timer->timerOffMinute,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("pumpOffAmPm",timer->timerOffAmPm,my_handle);
        }
        else
        {
            err = change_nvs_value("cleanerOnHr",timer->timerOnHour,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("cleanerOnMin",timer->timerOnMinute,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("cleanerOnAmPm",timer->timerOnAmPm,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("cleanerOffHr",timer->timerOffHour,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("cleanerOffMin",timer->timerOffMinute,my_handle);
            if(err == ESP_OK)
               err = change_nvs_value("cleanerOffAmPm",timer->timerOffAmPm,my_handle);
        }
        switch (err) {
            case ESP_OK:
                printf("timer changed\n");
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error %x changing timer settings!\n", err);
        }

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
        // Close
        nvs_close(my_handle);
    }
    return err;
}



esp_err_t change_nvs_value(char * name,int16_t newValue,nvs_handle_t my_handle)
{
   int16_t value;
   err = nvs_get_i16(my_handle, name, &value);
   if(err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND)
   {
        err = nvs_set_i16(my_handle, name, newValue);
        if (err != ESP_OK)
        {
            printf("Error %x setting %s handle!\n", err,name);
        }
    }
    else
    {
        printf("Error (%d) changing %s!\n", err,name);
    }
   return err;
}

void get_pool_timer_times(nvs_timer_t * times)
{
    nvs_handle_t my_handle;
    int16_t value;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else 
    {
        err = nvs_get_i16(my_handle, "pumpOnHr", (int16_t*)&times->pumpTimerOnHour);
        if(err == ESP_OK)
           err = nvs_get_i16(my_handle, "pumpOnMin", &times->pumpTimerOnMinute);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle, "pumpOnAmPm", &times->pumpTimerOnAmPm);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"pumpOffHr",&times->pumpTimerOffHour);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"pumpOffMin",&times->pumpTimerOffMinute);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"pumpOffAmPm",&times->pumpTimerOffAmPm);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOnHr",&times->cleanerTimerOnHour);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOnMin",&times->cleanerTimerOnMinute);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOnAmPm",&times->cleanerTimerOnAmPm);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOffHr",&times->cleanerTimerOffHour);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOffMin",&times->cleanerTimerOffMinute);
        if(err == ESP_OK)
            err = nvs_get_i16(my_handle,"cleanerOffAmPm",&times->cleanerTimerOffAmPm);
        }
       
    nvs_close(my_handle);
    if(err != ESP_OK)
    {
       printf("read failed with err = %x\n",err);
       return;
    }
    /* printf("pump on hr = %d\n",times->pumpTimerOnHour);
    printf("pumpOnMin = %d\n",times->pumpTimerOnMinute);
    printf("pumpOnAmPm = %d\n",times->pumpTimerOnAmPm);
    printf("pumpOffHr = %d\n",times->pumpTimerOffHour);
    printf("pumpOffMin = %d\n",times->pumpTimerOffMinute);
    printf("pumpOffAmPm = %d\n",times->pumpTimerOffAmPm);
    printf("cleanerOnHr = %d\n",times->cleanerTimerOnHour);
    printf("cleanerOnMin = %d\n",times->cleanerTimerOnMinute);
    printf("cleanerOnAmPm = %d\n",times->cleanerTimerOnAmPm);
    printf("cleanerOffHr = %d\n",times->cleanerTimerOffHour);
    printf("cleanerOffMin = %d\n",times->cleanerTimerOffMinute);
    printf("cleanerOffAmPm = %d\n",times->cleanerTimerOffAmPm); */
}


