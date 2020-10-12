#ifndef _PROPTOOLS_HH_
#define _PROPTOOLS_HH_

#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DEG2RAD		1.74532925199e-02

struct site {
	double lat;
	double lon;
	float alt;
};

double Distance(struct site site1, struct site site2);
double miles_to_km(double miles);
double ReadBearing(char *input);
char *dec2dms(double decimal);

#endif