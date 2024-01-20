/*
 * gps.c
 *
 *  Created on: Nov 29, 2023
 *      Author: andrew
 */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "queue.h"
#include "gps.h"
#include "fs.h"
#include "clock.h"

static uint8_t rx_buffer[512];
static queue_t * queue;
static bool debug = false;

// configuration messages for GPS module
const char * fix_interval = "$PMTK220,10000*2F\r\n";
const char * nmea_output = "$PMTK314,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29\r\n";

/** */
static float nmea_to_decimal_degrees(char *nmea,char nsew)
{
	float decimal = 0;
	if(strlen(nmea)>5)
	{
		char integer_part[3+1];
		int digit_count = (nmea[4] == '.' ? 2 : 3);
		memcpy(integer_part, nmea, digit_count);
		integer_part[digit_count] = 0;
		nmea += digit_count;

		decimal = atoi(integer_part) + atof(nmea)/60.0f;
		if(nsew == 'W' || nsew == 'S')
		{
			decimal = -decimal;
		}
	}
	return decimal;
}

/** */
static void set_time_and_date(char* time, char* date)
{
	int day, month,year;
	sscanf(date,"%2d%2d%2d",&day, &month, &year);
	int hours, minutes, seconds;
	sscanf(time,"%2d%2d%2d",&hours, &minutes, &seconds);
	set_clock(hours, minutes, seconds, day, month, year);
}

/** */
static void rx_event_callback(UART_HandleTypeDef *huart, uint16_t size)
{
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, sizeof(rx_buffer));
	queue_put(queue, rx_buffer,size);
}

/** */
static bool check_crc(uint8_t * buffer, size_t length)
{
	uint8_t recv_crc = 0;
	uint8_t calc_crc = 0;
	uint8_t i;

	for(i = 0; i < length - 2; i++)
	{
		if(buffer[i] == '$')
		{
			calc_crc = 0;
		}
		else if(buffer[i] == '*')
		{
			break;
		}
		else
		{
			calc_crc ^= buffer[i];
		}
	}

	recv_crc = strtoul((char*)&buffer[i+1], NULL, 16);

	return calc_crc == recv_crc;
}

/** */
static void rx_error_callback(UART_HandleTypeDef *huart)
{
	HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, sizeof(rx_buffer));
}

/** */
void gps_init(void)
{
	HAL_GPIO_WritePin(GPS_RST_GPIO_Port, GPS_RST_Pin, 0);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GPS_RST_GPIO_Port, GPS_RST_Pin, 1);
	HAL_Delay(500);
	gps_set_standby(false);

	queue = queue_create(4, 512);
	HAL_UART_RegisterRxEventCallback(&huart1, rx_event_callback);

	HAL_UART_RegisterCallback(&huart1, HAL_UART_ERROR_CB_ID, rx_error_callback);

	HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buffer, sizeof(rx_buffer));

	// configure GPS output format and frequency
	HAL_UART_Transmit(&huart1, (uint8_t*)nmea_output, strlen(nmea_output), -1);
	HAL_UART_Transmit(&huart1, (uint8_t*)fix_interval, strlen(fix_interval), -1);
}

/** */
void gps_process_msg(void)
{
	static uint8_t buffer[512];

	static bool fix_valid = false;
	while(queue_size(queue))
	{
		queue_get(queue,buffer, sizeof(buffer));

		char time[16];
		char date[16];
		char latitude[16];
		char longitude[16];
		char speed[16];
		char course[16];
		char valid, ns,ew;

		if(debug)
		{
			printf("%s\n$ ", buffer);
		}

		if(check_crc(buffer, sizeof(buffer)))
		{
			int n = sscanf((char *)buffer,"$GNRMC,%[^,],%c,%[^,],%c,%[^,],%c,%[^,],%[^,],%[^,]",time, &valid, latitude, &ns,longitude, &ew, speed, course, date);

			if(n >= 2 && valid == 'A')
			{
				if(!fix_valid)
				{
					fix_valid = true;
					set_time_and_date(time, date);

					if(debug)
					{
						printf("fix data became valid time and date set to %s on %s\n$ ", time, date );
					}
				}

				float decimal_latitude = nmea_to_decimal_degrees(latitude, ns);
				float decimal_longitude = nmea_to_decimal_degrees(longitude, ew);
				float decimal_speed = atof(speed);
				float decimal_course = atof(course);
				char buffer[330];
				int length = sprintf(buffer,"%.6s\t%12.8f\t%12.8f\t%12.8f\t%12.8f\n", time, decimal_latitude, decimal_longitude, decimal_speed, decimal_course);

				append_to_current_file(buffer, length);
			}

			if(n < 2 || valid != 'A')
			{
				if(fix_valid)
				{
					fix_valid = false;
					if(debug)
					{
						printf("fix no longer valid\n$ ");
					}
				}
			}

			HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 0);
			HAL_Delay(50);
			HAL_GPIO_WritePin(LED0_GPIO_Port, LED0_Pin, 1);
		}
	}

	// turn on an LED when fix obtained
	HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, !gps_get_fix_obtained());
}

/** */
void gps_set_standby(bool standby)
{
	HAL_GPIO_WritePin(GPS_SB_GPIO_Port, GPS_SB_Pin, !standby);
}

/** */
bool gps_get_standby(void)
{
	return !HAL_GPIO_ReadPin(GPS_SB_GPIO_Port, GPS_SB_Pin);
}
/** */
bool gps_get_fix_obtained(void)
{
	return HAL_GPIO_ReadPin(GPS_FIX_GPIO_Port, GPS_FIX_Pin);
}

/** */
void gps_set_debug(bool dbg)
{
	debug = dbg;
}

/** */
bool gps_get_debug(void)
{
	return debug;
}
