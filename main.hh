#ifndef _MAIN_HH_
#define _MAIN_HH_

#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "common.h"

int ReduceAngle(double angle);
double LonDiff(double lon1, double lon2);
int PutMask(double lat, double lon, int value);
int OrMask(double lat, double lon, int value);
int GetMask(double lat, double lon);
int PutSignal(double lat, double lon, unsigned char signal);
unsigned char GetSignal(double lat, double lon);
double GetElevation(struct site location);
int AddElevation(double lat, double lon, double height, int size);
double Distance(struct site site1, struct site site2);
double Azimuth(struct site source, struct site destination);
double ElevationAngle(struct site source, struct site destination);
void ReadPath(struct site source, struct site destination);
double ElevationAngle2(struct site source, struct site destination, double er);
double ReadBearing(char *input);
void ObstructionAnalysis(struct site xmtr, struct site rcvr, double f, FILE *outfile);

#define DEG2RAD	1.74532925199e-02
/*
struct site {
	double lat;
	double lon;
	float alt;
};*/

char *dec2dms(double decimal);
double miles_to_km(double miles);



void free_elev(void);
void free_path(void);
void alloc_elev(void);
void alloc_path(void);
void do_allocs(void);

#endif /* _MAIN_HH_ */
