/*
 * filter.c
 *
 *  Created on: Dec 19, 2023
 *      Author: andrew
 */

#include <math.h>

const double R = 6371e3; // metres

double distance_between_points(double lat1, double lon1, double lat2, double lon2)
{
	double phi1 = lat1 * M_PI/180; // phi, lambda in radians
	double phi2 = lat2 * M_PI/180;
	double deltaphi = (lat2-lat1) * M_PI/180;
	double deltalambda = (lon2-lon1) * M_PI/180;

	double a = sin(deltaphi/2) * sin(deltaphi/2) + cos(phi1) * cos(phi2) * sin(deltalambda/2) * sin(deltalambda/2);
	double c = 2 * atan2(sqrt(a), sqrt(1-a));

	double d = R * c; // in metres

	return d;
}

