#ifndef _PROPTOOLS_HH_
#define _PROPTOOLS_HH_

#include <math.h>
#include <stdlib.h>
#include <string.h>
#define DEG2RAD		1.74532925199e-02

struct site {
	double lat;
	double lon;
	float alt;
};

double Distance(struct site site1, struct site site2);
double miles_to_km(double miles);

#endif