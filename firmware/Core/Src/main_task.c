/*
 * main_task.c
 *
 *  Created on: Oct 20, 2023
 *      Author: andrew
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

#include "main.h"
#include "console.h"
#include "gps.h"
#include "fs.h"

/** */
void main_task(void)
{
	printf(CLS "GPSRecorder built at %s on %s\n\n", __TIME__, __DATE__);

	console_init();
	fs_init();
	gps_init();

    while (true)
    {
    	console_process_key();
    	gps_process_msg();
    }
}
