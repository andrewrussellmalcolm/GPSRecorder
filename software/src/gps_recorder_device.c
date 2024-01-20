/*
 * gps_recorder_device.c
 *
 *  Created on: Dec 11, 2023
 *      Author: andrew
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>

#include "gps_recorder_device.h"
#include "filter.h"

const float knot_to_ms = 0.514444;

/** */
static bool gps_recorder_device_is_in_use(char * device_name)
{
	FILE *fp;
	char path[1035];
	char command[32];
	bool open = false;

	sprintf(command,"/bin/lsof -t %s", device_name);

	fp = popen(command, "r");
	if (fp == NULL)
	{
		printf("Failed to run lsof\n" );
		exit(1);
	}

	// if lsof outputs anything, the port is is use
	// or not active
	while (fgets(path, sizeof(path), fp) != NULL)
	{
		if(strlen(path) > 0)
		{
			open = true;
		}
	}

	/* close */
	pclose(fp);

	return open;
}

/** */
FILE * gps_recorder_device_open(char * device_name)
{
	FILE * device;

	if(gps_recorder_device_is_in_use(device_name))
	{
		return NULL;
	}

	device = fopen(device_name,"a+");

	if(device == NULL)
	{
		return NULL;
	}

    struct termios tty;

    tcgetattr(fileno(device), &tty);
    cfmakeraw(&tty);
    tcsetattr(fileno(device), TCSANOW, &tty);

	return device;
}

/** */
void gps_recorder_device_close(FILE * device)
{
	fclose(device);
}

/** */
void gps_recorder_device_list_file_points(FILE * device, char *filename, pf_add_point_t add_point)
{
	char buffer[64];
	double latitude;
	double longitude;
	double speed;
	double course;
	long time;

	int points = 0;

	fprintf(device, "cat %s\r", filename);

	while(true)
	{
		fgets(buffer, sizeof(buffer), device);

		if((strlen(buffer) == 1) && (points > 0))
		{
			break;
		}

		int n = sscanf(buffer,"%ld\t%lf\t%lf\t%lf\t%lf", &time, &latitude, &longitude, &speed, &course);

		speed *= knot_to_ms;

		if(n == 5)
		{
			add_point( time, latitude, longitude, speed, course);
			points++;
		}
	}
}

/** */
void gps_recorder_list_files(FILE * device, pf_add_file_t add_file)
{
	char buffer[64];

	fputs("ls\r", device);

	while(true)
	{
		fgets(buffer, sizeof(buffer), device);

		if(strlen(buffer) == 1)
		{
			break;
		}
		else if(strncmp(buffer, ".", 1) == 0)
		{
			continue;
		}
		else if(strncmp(buffer, "..", 2) == 0)
		{
			continue;
		}

		char name[64];
		int size;
		int n = sscanf(buffer,"%s\t\t%d\n", name, &size);

		if(n == 2)
		{
			add_file(name,size);
		}
	}
}

/** */
void gps_recorder_delete_file(FILE * device, char *filename)
{
	char buffer[64];
	fprintf(device, "rm %s\r", filename);

	while(true)
	{
		fgets(buffer, sizeof(buffer), device);

		if(strlen(buffer) == 1)
		{
			break;
		}
	}
}

/** */
void gps_recorder_get_status(FILE * device,pf_add_status_t add_status)
{
	bool fix_obtained =false;
	char buffer[64];

	fputs("gps fix\r", device);

	while(true)
	{
		fgets(buffer, sizeof(buffer), device);

		if(strlen(buffer) == 1)
		{
			break;
		}
		else if(strstr(buffer, "obtained") != NULL)
		{
			fix_obtained = true;
		}
	}

	fputs("info\r", device);

	while(true)
	{
		fgets(buffer, sizeof(buffer), device);

		if(strlen(buffer) == 1)
		{
			break;
		}
		else if(strstr(buffer, "total") != NULL)
		{
			break;
		}
	}

	add_status(fix_obtained, buffer);
}


