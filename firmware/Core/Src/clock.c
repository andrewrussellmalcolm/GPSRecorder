/*
 * clock.c
 *
 *  Created on: 11 Dec 2023
 *      Author: andrew
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "main.h"
#include "clock.h"

/** */
void set_clock(int hours, int minutes, int seconds, int date, int month, int year)
{
    RTC_TimeTypeDef sTime = {.Hours = hours, .Minutes = minutes, .Seconds = seconds};
    RTC_DateTypeDef sDate = {.Date = date, .Month = month, .Year = year};

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/** */
void get_clock(int *hours, int *minutes, int *seconds, int *date, int *month, int *year)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    *hours = sTime.Hours;
    *minutes = sTime.Minutes;
    *seconds = sTime.Seconds;
    *date = sDate.Date;
    *month = sDate.Month;
    *year = sDate.Year;
}

