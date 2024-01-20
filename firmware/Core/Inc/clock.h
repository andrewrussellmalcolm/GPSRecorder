/*
 * clock.h
 *
 *  Created on: 11 Dec 2023
 *      Author: andrew
 */

#ifndef INC_CLOCK_H_
#define INC_CLOCK_H_

void set_clock(int hours, int minutes, int seconds, int date, int month, int year);
void get_clock(int *hours, int *minutes, int *seconds, int *date, int *month, int *year);

#endif /* INC_CLOCK_H_ */
