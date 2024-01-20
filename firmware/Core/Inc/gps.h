/*
 * gps.h
 *
 *  Created on: Nov 29, 2023
 *      Author: andrew
 */

#ifndef INC_GPS_H_
#define INC_GPS_H_

#define GPS_MSG_MAX 90

void gps_init(void);
void gps_process_msg(void);
void gps_set_standby(bool standby);
bool gps_get_standby(void);
bool gps_get_fix_obtained(void);
void gps_set_debug(bool debug);
bool gps_get_debug(void);

#endif /* INC_GPS_H_ */
