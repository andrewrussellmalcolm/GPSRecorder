/*
 * gps_recorder_device.h
 *
 *  Created on: Dec 13, 2023
 *      Author: andrew
 */

#ifndef GPS_RECORDER_DEVICE_H_
#define GPS_RECORDER_DEVICE_H_

typedef void (*pf_add_point_t)( long time, double latitude, double longitude, double speed, double course);
typedef void (*pf_add_file_t)(char * name, int size);
typedef void (*pf_add_status_t)(bool fix_obtained, char * info);

FILE * gps_recorder_device_open(char * device_name);
void gps_recorder_device_close(FILE * device);
void gps_recorder_device_list_file_points(FILE * device, char *filename, pf_add_point_t add_point);
void gps_recorder_list_files(FILE * device,pf_add_file_t add_file);
void gps_recorder_get_status(FILE * device,pf_add_status_t add_status);
void gps_recorder_delete_file(FILE * device, char *filename);
#endif /* GPS_RECORDER_DEVICE_H_ */
