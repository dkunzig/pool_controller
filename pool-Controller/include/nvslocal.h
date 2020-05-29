

#include <stdio.h>

typedef struct nvs_timer_t {
    int16_t pumpTimerOnHour; 
    int16_t pumpTimerOnMinute;
    int16_t pumpTimerOnAmPm;
    int16_t pumpTimerOffHour; 
    int16_t pumpTimerOffMinute;
    int16_t pumpTimerOffAmPm;
    int16_t cleanerTimerOnHour;
    int16_t cleanerTimerOnMinute;
    int16_t cleanerTimerOnAmPm;
    int16_t cleanerTimerOffHour;
    int16_t cleanerTimerOffMinute;
    int16_t cleanerTimerOffAmPm;
    bool err;
}nvs_timer_t; 

typedef struct nvs_times_t {
    bool cleaner;
    bool pump;
    int16_t timerOnHour; 
    int16_t timerOnMinute;
    int16_t timerOnAmPm; 
    int16_t timerOffHour; 
    int16_t timerOffMinute;
    int16_t timerOffAmPm; 
}nvs_times_t; 



