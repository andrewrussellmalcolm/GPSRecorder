/*
 * persistence.h
 *
 *  Created on: Jan 1, 2024
 *      Author: andrew
 */

#ifndef PERSISTENCE_H_
#define PERSISTENCE_H_


bool ini_load(void);
bool ini_create_default(void);
double ini_get_home_latitude(void);
double ini_get_home_longitude(void);
void ini_set_home_latitude(double latitude);
void ini_set_home_longitude(double longitude);
char * ini_get_device_name(void);

#endif /* PERSISTENCE_H_ */
