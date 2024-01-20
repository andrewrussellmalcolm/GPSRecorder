/*
 * persistance.c
 *
 *  Created on: Jan 1, 2024
 *      Author: andrew
 */

#include <stdbool.h>
#include </usr/include/gtk-3.0/gtk/gtk.h>

#define INI_FILENAME "gps_recorder_host.ini"

// default coordinates  @ Greenwich time museum, London
#define DEFAULT_LATITUDE  51.47787684087464
#define DEFAULT_LONGITUDE -0.0014739948731443415
static GKeyFile *ini_file;

/** */
bool ini_load(void)
{
	GError *error = NULL;

	ini_file = g_key_file_new();
	if (!g_key_file_load_from_file(ini_file, INI_FILENAME, G_KEY_FILE_NONE, &error))
	{
		g_key_file_free(ini_file);
		return false;
	}

	return true;
}

/** */
bool ini_save(void)
{
	GError *error = NULL;
	return g_key_file_save_to_file (ini_file, INI_FILENAME, &error);
}

/** */
bool ini_create_default(void)
{
	GError *error = NULL;

	if (!g_file_set_contents_full (INI_FILENAME, "", 0, G_FILE_SET_CONTENTS_NONE, 0666, &error))
	{
		return false;
	}

	return true;
}

/** */
double ini_get_home_latitude(void)
{
	GError *error = NULL;

	double latitude = g_key_file_get_double (ini_file, "COORDINATES", "LATITUDE", &error);

	if(latitude == 0.0)
	{
		latitude = DEFAULT_LATITUDE;
		g_key_file_set_double (ini_file,"COORDINATES", "LATITUDE", latitude);
		ini_save();
	}

	return latitude;
}

/** */
double ini_get_home_longitude(void)
{
	GError *error = NULL;

	double longitude = g_key_file_get_double (ini_file, "COORDINATES", "LONGITUDE", &error);

	if(longitude == 0.0)
	{
		longitude = DEFAULT_LONGITUDE;
		g_key_file_set_double (ini_file,"COORDINATES", "LONGITUDE", longitude);
		ini_save();
	}

	return longitude;
}

/** */
void ini_set_home_latitude(double latitude)
{
	g_key_file_set_double (ini_file,"COORDINATES", "LATITUDE", latitude);
	ini_save();
}

/** */
void ini_set_home_longitude(double longitude)
{
	g_key_file_set_double (ini_file,"COORDINATES", "LONGITUDE", longitude);
	ini_save();
}

/** */
char * ini_get_device_name(void)
{
	GError *error = NULL;

	char * device_name = g_key_file_get_string (ini_file, "DEVICE", "NAME", &error);

	if(device_name == NULL)
	{
		device_name = "/dev/ttyACM0";
		g_key_file_set_string (ini_file,"DEVICE", "NAME", device_name);
		ini_save();
	}

	return device_name;

}
