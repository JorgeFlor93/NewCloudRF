double version = 3.10;
/****************************************************************************\
*  Signal Server: Radio propagation simulator by Alex Farrant QCVS, 2E0TDW   *
******************************************************************************
*    SPLAT! Project started in 1997 by John A. Magliacane, KD2BD             *
******************************************************************************
*         Please consult the SPLAT! documentation for a complete list of     *
*         individuals who have contributed to this project.                  *
******************************************************************************
*                                                                            *
*  This program is free software; you can redistribute it and/or modify it   *
*  under the terms of the GNU General Public License as published by the     *
*  Free Software Foundation; either version 2 of the License or any later    *
*  version.                                                                  *
*                                                                            *
*  This program is distributed in the hope that it will useful, but WITHOUT  *
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or     *
*  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License     *
*  for more details.                                                         *
*                                                                            *
\****************************************************************************/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "common.h"
#include "inputs.hh"
#include "outputs.hh"
#include "models/itwom3.0.hh"
#include "models/los.hh"
#include "models/pel.hh"
#include "image.hh"
#include "auxFuncts.hh"

/*RESOLUTION SDF*/
int MAXPAGES = 10*10;
int IPPD = 1200; // or 3600
int ARRAYSIZE = (MAXPAGES * IPPD) + 10;

char string[255], sdf_path[255], udt_file[255], opened = 0, gpsav =
    0, ss_name[16], dashes[80];

double earthradius, max_range = 0.0, forced_erp, dpp, ppd, yppd,
    fzone_clearance = 0.6, forced_freq, clutter, lat, lon, txh, tercon, terdic,
    north, east, south, west, dBm, loss, field_strength,
    min_north = 90, max_north = -90, min_west = 360, max_west = -1, westoffset=180, eastoffset=-180, delta=0, rxGain=0,
    cropLat=-70, cropLon=0,cropLonNeg=0;

int ippd, mpi, 
    max_elevation = -32768, min_elevation = 32768, bzerror, contour_threshold,
    pred, pblue, pgreen, ter, multiplier = 256, debug = 0, loops = 100, jgets =
    0, hottest = 0, height, width, resample = 0;

unsigned char got_elevation_pattern, got_azimuth_pattern, metric = 0, dbm = 0;

bool to_stdout = false, cropping = true;

__thread double *elev;
__thread struct path path;
struct site tx_site[2];
struct dem *dem;

struct LR LR;
struct region region;

double arccos(double x, double y)
{
	/* This function implements the arc cosine function,
	   returning a value between 0 and TWOPI. */

	double result = 0.0;

	if (y > 0.0)
		result = acos(x / y);

	if (y < 0.0)
		result = PI + acos(x / y);

	return result;
}

int ReduceAngle(double angle)
{
	/* This function normalizes the argument to
	   an integer angle between 0 and 180 degrees */

	double temp;

	temp = acos(cos(angle * DEG2RAD));

	return (int)rint(temp / DEG2RAD);
}

double LonDiff(double lon1, double lon2)
{
	/* This function returns the short path longitudinal
	   difference between longitude1 and longitude2
	   as an angle between -180.0 and +180.0 degrees.
	   If lon1 is west of lon2, the result is positive.
	   If lon1 is east of lon2, the result is negative. */

	double diff;

	diff = lon1 - lon2;

	if (diff <= -180.0)
		diff += 360.0;

	if (diff >= 180.0)
		diff -= 360.0;

	return diff;
}

char *dec2dms(double decimal)
{
	/* Converts decimal degrees to degrees, minutes, seconds,
	   (DMS) and returns the result as a character string. */

	char sign;
	int degrees, minutes, seconds;
	double a, b, c, d;

	if (decimal < 0.0) {
		decimal = -decimal;
		sign = -1;
	}

	else
		sign = 1;

	a = floor(decimal);
	b = 60.0 * (decimal - a);
	c = floor(b);
	d = 60.0 * (b - c);

	degrees = (int)a;
	minutes = (int)c;
	seconds = (int)d;

	if (seconds < 0)
		seconds = 0;

	if (seconds > 59)
		seconds = 59;

	string[0] = 0;
	snprintf(string, 250, "%d %d %d", degrees * sign, minutes,
		 seconds);
	return (string);
}

int PutMask(double lat, double lon, int value)
{
	/* Lines, text, markings, and coverage areas are stored in a
	   mask that is combined with topology data when topographic
	   maps are generated by ss.  This function sets and resets
	   bits in the mask based on the latitude and longitude of the
	   area pointed to. */

	int x = 0, y = 0, indx;
	char found;

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		x = (int)rint(ppd * (lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd * (LonDiff(dem[indx].max_west, lon)));

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found) {
		dem[indx].mask[x][y] = value;
		return ((int)dem[indx].mask[x][y]);
	}

	else
		return -1;
}

int OrMask(double lat, double lon, int value)
{
	/* Lines, text, markings, and coverage areas are stored in a
	   mask that is combined with topology data when topographic
	   maps are generated by ss.  This function sets bits in
	   the mask based on the latitude and longitude of the area
	   pointed to. */

	int x = 0, y = 0, indx;
	char found;

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		x = (int)rint(ppd * (lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd * (LonDiff(dem[indx].max_west, lon)));

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found) {
		dem[indx].mask[x][y] |= value;
		return ((int)dem[indx].mask[x][y]);
	}

	else
		return -1;
}

int GetMask(double lat, double lon)
{
	/* This function returns the mask bits based on the latitude
	   and longitude given. */

	return (OrMask(lat, lon, 0));
}

int PutSignal(double lat, double lon, unsigned char signal)
{
	/* This function writes a signal level (0-255)
	   at the specified location for later recall. */
	char dotfile[255];
	snprintf(dotfile, 80, "%s.dot%c", tx_site[0].filename, 0);
	snprintf(dotfile, 80, "%s.dot%c", tx_site[0].filename, 0);

	int x = 0, y = 0, indx;
	char found;
	if (signal > hottest)	// dBm, dBuV
		hottest = signal;

	//lookup x/y for this co-ord
	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		x = (int)rint(ppd * (lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd * (LonDiff(dem[indx].max_west, lon)));

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found) {
		// Write values to file
		dem[indx].signal[x][y] = signal;

		return (dem[indx].signal[x][y]);
	}

	else
		return 0;
}

unsigned char GetSignal(double lat, double lon)
{
	/* This function reads the signal level (0-255) at the
	   specified location that was previously written by the
	   complimentary PutSignal() function. */

	int x = 0, y = 0, indx;
	char found;

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		x = (int)rint(ppd * (lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd * (LonDiff(dem[indx].max_west, lon)));

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found)
		return (dem[indx].signal[x][y]);
	else
		return 0;
}

double GetElevation(struct site location)
{
	/* This function returns the elevation (in feet) of any location
	   represented by the digital elevation model data in memory.
	   Function returns -5000.0 for locations not found in memory. */

	char found;
	int x = 0, y = 0, indx;
	double elevation;

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		// std::cout << "loc_lat" << location.lat << std::endl;
		x = (int)rint(ppd * (location.lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd *
			      (LonDiff(dem[indx].max_west, location.lon)));
		

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found){
		elevation = 3.28084 * dem[indx].data[x][y];
		//std::cout << "elevation: " << elevation << std::endl;
	}
	else
		elevation = -5000.0;

	return elevation;
}

int AddElevation(double lat, double lon, double height, int size)
{
	/* This function adds a user-defined terrain feature
	   (in meters AGL) to the digital elevation model data
	   in memory.  Does nothing and returns 0 for locations
	   not found in memory. */

	char found;
	int i,j,x = 0, y = 0, indx;

	for (indx = 0, found = 0; indx < MAXPAGES && found == 0;) {
		x = (int)rint(ppd * (lat - dem[indx].min_north));
		y = mpi - (int)rint(yppd * (LonDiff(dem[indx].max_west, lon)));

		if (x >= 0 && x <= mpi && y >= 0 && y <= mpi)
			found = 1;
		else
			indx++;
	}

	if (found && size<2)
		dem[indx].data[x][y] += (short)rint(height);

	// Make surrounding area bigger for wide area landcover. Should enhance 3x3 pixels including c.p
	if (found && size>1){
		for(i=size*-1; i <= size; i=i+1){
			for(j=size*-1; j <= size; j=j+1){
				if(x+j >= 0 && x+j <=IPPD && y+i >= 0 && y+i <=IPPD)
					dem[indx].data[x+j][y+i] += (short)rint(height);
			}

		}
	}


	return found;
}

double dist(double lat1, double lon1, double lat2, double lon2)
{
	//ENHANCED HAVERSINE FORMULA WITH RADIUS SLIDER
	double dx, dy, dz;
	int polarRadius=6357;
	int equatorRadius=6378;
	int delta = equatorRadius-polarRadius; // 21km
	float earthRadius = equatorRadius - ((lat1/100) * delta);
	lon1 -= lon2;
	lon1 *= DEG2RAD, lat1 *= DEG2RAD, lat2 *= DEG2RAD;
 
	dz = sin(lat1) - sin(lat2);
	dx = cos(lon1) * cos(lat1) - cos(lat2);
	dy = sin(lon1) * cos(lat1);
	return asin(sqrt(dx * dx + dy * dy + dz * dz) / 2) * 2 * earthRadius;
}

double miles_to_km(double miles){   
    return miles*1.60934;
}

double Distance(struct site site1, struct site site2)
{
	/* This function returns the great circle distance
	   in miles between any two site locations. */

	double lat1, lon1, lat2, lon2, distance;

	lat1 = site1.lat * DEG2RAD;
	lon1 = site1.lon * DEG2RAD;
	lat2 = site2.lat * DEG2RAD;
	lon2 = site2.lon * DEG2RAD;

	distance =
	    3959.0 * acos(sin(lat1) * sin(lat2) +
			  cos(lat1) * cos(lat2) * cos((lon1) - (lon2)));

	distance = miles_to_km(distance);
	return distance;
}

double Azimuth(struct site source, struct site destination)
{
	/* This function returns the azimuth (in degrees) to the
	   destination as seen from the location of the source. */

	double dest_lat, dest_lon, src_lat, src_lon,
	    beta, azimuth, diff, num, den, fraction;

	dest_lat = destination.lat * DEG2RAD;
	dest_lon = destination.lon * DEG2RAD;

	src_lat = source.lat * DEG2RAD;
	src_lon = source.lon * DEG2RAD;

	/* Calculate Surface Distance */

	beta =
	    acos(sin(src_lat) * sin(dest_lat) +
		 cos(src_lat) * cos(dest_lat) * cos(src_lon - dest_lon));

	/* Calculate Azimuth */

	num = sin(dest_lat) - (sin(src_lat) * cos(beta));
	den = cos(src_lat) * sin(beta);
	fraction = num / den;

	/* Trap potential problems in acos() due to rounding */

	if (fraction >= 1.0)
		fraction = 1.0;

	if (fraction <= -1.0)
		fraction = -1.0;

	/* Calculate azimuth */

	azimuth = acos(fraction);

	/* Reference it to True North */

	diff = dest_lon - src_lon;

	if (diff <= -PI)
		diff += TWOPI;

	if (diff >= PI)
		diff -= TWOPI;

	if (diff > 0.0)
		azimuth = TWOPI - azimuth;

	return (azimuth / DEG2RAD);
}

double ElevationAngle(struct site source, struct site destination)
{
	/* This function returns the angle of elevation (in degrees)
	   of the destination as seen from the source location.
	   A positive result represents an angle of elevation (uptilt),
	   while a negative result represents an angle of depression
	   (downtilt), as referenced to a normal to the center of
	   the earth. */

	register double a, b, dx;

	a = GetElevation(destination) + destination.alt + earthradius;
	b = GetElevation(source) + source.alt + earthradius;

	dx = FEET_PER_MILE * Distance(source, destination);

	/* Apply the Law of Cosines */

	return ((180.0 *
		 (acos(((b * b) + (dx * dx) - (a * a)) / (2.0 * b * dx))) /
		 PI) - 90.0);
}

void ReadPath(struct site source, struct site destination)
{
	/* This function generates a sequence of latitude and
	   longitude positions between source and destination
	   locations along a great circle path, and stores
	   elevation and distance information for points
	   along that path in the "path" structure. */

	int c;
	double azimuth, distance, lat1, lon1, beta, den, num,
	    lat2, lon2, total_distance, dx, dy, path_length,
	    miles_per_sample, samples_per_radian = 68755.0;
	struct site tempsite;

	lat1 = source.lat * DEG2RAD;
	lon1 = source.lon * DEG2RAD;
	lat2 = destination.lat * DEG2RAD;
	lon2 = destination.lon * DEG2RAD;
	samples_per_radian = ppd * 57.295833;
	azimuth = Azimuth(source, destination) * DEG2RAD;

	total_distance = Distance(source, destination);
	//std::cout << "ppd: " << ppd << ", total_distance: "<< total_distance << std::endl;
	if (total_distance > (30.0 / ppd)) {
		//std::cout << "in" << std::endl;
		dx = samples_per_radian * acos(cos(lon1 - lon2));
		dy = samples_per_radian * acos(cos(lat1 - lat2));
		path_length = sqrt((dx * dx) + (dy * dy));
		miles_per_sample = total_distance / path_length;
	}

	else {
		//std::cout << "1" << std::endl;
		c = 0;
		dx = 0.0;
		dy = 0.0;
		path_length = 0.0;
		miles_per_sample = 0.0;
		total_distance = 0.0;

		lat1 = lat1 / DEG2RAD;
		lon1 = lon1 / DEG2RAD;

		path.lat[c] = lat1;
		path.lon[c] = lon1;
		path.elevation[c] = GetElevation(source);
		//std::cout << "path lat, lon, elevation: " << path.lat[c] << " - " << path.lon[c] << " - "<< path.elevation[c] << std::endl;
		path.distance[c] = 0.0;
	}

	for (distance = 0.0, c = 0;
	     (total_distance != 0.0 && distance <= total_distance
	      && c < ARRAYSIZE); c++, distance = miles_per_sample * (double)c) {
		beta = distance / 3959.0;
		lat2 =
		    asin(sin(lat1) * cos(beta) +
			 cos(azimuth) * sin(beta) * cos(lat1));
		num = cos(beta) - (sin(lat1) * sin(lat2));
		den = cos(lat1) * cos(lat2);

		if (azimuth == 0.0 && (beta > HALFPI - lat1))
			lon2 = lon1 + PI;

		else if (azimuth == HALFPI && (beta > HALFPI + lat1))
			lon2 = lon1 + PI;

		else if (fabs(num / den) > 1.0)
			lon2 = lon1;

		else {
			if ((PI - azimuth) >= 0.0)
				lon2 = lon1 - arccos(num, den);
			else
				lon2 = lon1 + arccos(num, den);
		}

		while (lon2 < 0.0)
			lon2 += TWOPI;

		while (lon2 > TWOPI)
			lon2 -= TWOPI;

		lat2 = lat2 / DEG2RAD;
		lon2 = lon2 / DEG2RAD;

		path.lat[c] = lat2;
		path.lon[c] = lon2;
		tempsite.lat = lat2;
		tempsite.lon = lon2;
		//std::cout << "lat: " << tempsite.lat << "lon: " << tempsite.lon << std::endl;
		path.elevation[c] = GetElevation(tempsite);
		//std::cout << "path lat, lon, elevation: " << path.lat[c] << " - " << path.lon[c] << " - "<< path.elevation[c] << std::endl;
		// fix for tile gaps in multi-tile LIDAR plots
		if(path.elevation[c]==0 && path.elevation[c-1] > 10)
			path.elevation[c]=path.elevation[c-1];
		path.distance[c] = distance;
	}

	/* Make sure exact destination point is recorded at path.length-1 */

	if (c < ARRAYSIZE) {
		path.lat[c] = destination.lat;
		path.lon[c] = destination.lon;
		path.elevation[c] = GetElevation(destination); 
		path.distance[c] = total_distance;
		c++;
	}

	if (c < ARRAYSIZE)
		path.length = c;
	else
		path.length = ARRAYSIZE - 1;
}

double ElevationAngle2(struct site source, struct site destination, double er)
{
	/* This function returns the angle of elevation (in degrees)
	   of the destination as seen from the source location, UNLESS
	   the path between the sites is obstructed, in which case, the
	   elevation angle to the first obstruction is returned instead.
	   "er" represents the earth radius. */

	int x;
	char block = 0;
	double source_alt, destination_alt, cos_xmtr_angle,
	    cos_test_angle, test_alt, elevation, distance,
	    source_alt2, first_obstruction_angle = 0.0;
	struct path temp;

	temp = path;

	ReadPath(source, destination);

	distance = FEET_PER_MILE * Distance(source, destination);
	source_alt = er + source.alt + GetElevation(source);
	destination_alt = er + destination.alt + GetElevation(destination);
	source_alt2 = source_alt * source_alt;

	/* Calculate the cosine of the elevation angle of the
	   destination (receiver) as seen by the source (transmitter). */

	cos_xmtr_angle =
	    ((source_alt2) + (distance * distance) -
	     (destination_alt * destination_alt)) / (2.0 * source_alt *
						     distance);

	/* Test all points in between source and destination locations to
	   see if the angle to a topographic feature generates a higher
	   elevation angle than that produced by the destination.  Begin
	   at the source since we're interested in identifying the FIRST
	   obstruction along the path between source and destination. */

	for (x = 2, block = 0; x < path.length && block == 0; x++) {
		distance = FEET_PER_MILE * path.distance[x];

		test_alt =
		    earthradius + (path.elevation[x] ==
				   0.0 ? path.elevation[x] : path.elevation[x] +
				   clutter);

		cos_test_angle =
		    ((source_alt2) + (distance * distance) -
		     (test_alt * test_alt)) / (2.0 * source_alt * distance);

		/* Compare these two angles to determine if
		   an obstruction exists.  Since we're comparing
		   the cosines of these angles rather than
		   the angles themselves, the sense of the
		   following "if" statement is reversed from
		   what it would be if the angles themselves
		   were compared. */

		if (cos_xmtr_angle >= cos_test_angle) {
			block = 1;
			first_obstruction_angle =
			    ((acos(cos_test_angle)) / DEG2RAD) - 90.0;
		}
	}

	if (block)
		elevation = first_obstruction_angle;

	else
		elevation = ((acos(cos_xmtr_angle)) / DEG2RAD) - 90.0;

	path = temp;

	return elevation;
}

double ReadBearing(char *input)
{
	/* This function takes numeric input in the form of a character
	   string, and returns an equivalent bearing in degrees as a
	   decimal number (double).  The input may either be expressed
	   in decimal format (40.139722) or degree, minute, second
	   format (40 08 23).  This function also safely handles
	   extra spaces found either leading, trailing, or
	   embedded within the numbers expressed in the
	   input string.  Decimal seconds are permitted. */

	double seconds, bearing = 0.0;
	char string[20];
	int a, b, length, degrees, minutes;

	/* Copy "input" to "string", and ignore any extra
	   spaces that might be present in the process. */

	string[0] = 0;
	length = strlen(input);

	for (a = 0, b = 0; a < length && a < 18; a++) {
		if ((input[a] != 32 && input[a] != '\n')
		    || (input[a] == 32 && input[a + 1] != 32
			&& input[a + 1] != '\n' && b != 0)) {
			string[b] = input[a];
			b++;
		}
	}

	string[b] = 0;

	/* Count number of spaces in the clean string. */

	length = strlen(string);

	for (a = 0, b = 0; a < length; a++)
		if (string[a] == 32)
			b++;

	if (b == 0)		/* Decimal Format (40.139722) */
		sscanf(string, "%lf", &bearing);

	if (b == 2) {		/* Degree, Minute, Second Format (40 08 23.xx) */
		sscanf(string, "%d %d %lf", &degrees, &minutes, &seconds);

		bearing = fabs((double)degrees);
		bearing += fabs(((double)minutes) / 60.0);
		bearing += fabs(seconds / 3600.0);

		if ((degrees < 0) || (minutes < 0) || (seconds < 0.0))
			bearing = -bearing;
	}

	/* Anything else returns a 0.0 */

	if (bearing > 360.0 || bearing < -360.0)
		bearing = 0.0;

	return bearing;
}

void ObstructionAnalysis(struct site xmtr, struct site rcvr, double f,
			 FILE *outfile)
{
	/* Perform an obstruction analysis along the
	   path between receiver and transmitter. */

	int x;
	struct site site_x;
	double h_r, h_t, h_x, h_r_orig, cos_tx_angle, cos_test_angle,
	    cos_tx_angle_f1, cos_tx_angle_fpt6, d_tx, d_x,
	    h_r_f1, h_r_fpt6, h_f, h_los, lambda = 0.0;
	char string[255], string_fpt6[255], string_f1[255];

	ReadPath(xmtr, rcvr);
	h_r = GetElevation(rcvr) + rcvr.alt + earthradius;
	h_r_f1 = h_r;
	h_r_fpt6 = h_r;
	h_r_orig = h_r;
	h_t = GetElevation(xmtr) + xmtr.alt + earthradius;
	d_tx = FEET_PER_MILE * Distance(rcvr, xmtr);
	cos_tx_angle =
	    ((h_r * h_r) + (d_tx * d_tx) - (h_t * h_t)) / (2.0 * h_r * d_tx);
	cos_tx_angle_f1 = cos_tx_angle;
	cos_tx_angle_fpt6 = cos_tx_angle;

	if (f)
		lambda = 9.8425e8 / (f * 1e6);

	if (clutter > 0.0) {
		fprintf(outfile, "Terrain has been raised by");

		if (metric)
			fprintf(outfile, " %.2f meters",
				METERS_PER_FOOT * clutter);
		else
			fprintf(outfile, " %.2f feet", clutter);

		fprintf(outfile, " to account for ground clutter.\n\n");
	}

	/* At each point along the path calculate the cosine
	   of a sort of "inverse elevation angle" at the receiver.
	   From the antenna, 0 deg. looks at the ground, and 90 deg.
	   is parallel to the ground.

	   Start at the receiver.  If this is the lowest antenna,
	   then terrain obstructions will be nearest to it.  (Plus,
	   that's the way ppa!'s original los() did it.)

	   Calculate cosines only.  That's sufficient to compare
	   angles and it saves the extra computational burden of
	   acos().  However, note the inverted comparison: if
	   acos(A) > acos(B), then B > A. */

	for (x = path.length - 1; x > 0; x--) {
		site_x.lat = path.lat[x];
		site_x.lon = path.lon[x];
		site_x.alt = 0.0;

		h_x = GetElevation(site_x) + earthradius + clutter;
		d_x = FEET_PER_MILE * Distance(rcvr, site_x);

		/* Deal with the LOS path first. */

		cos_test_angle =
		    ((h_r * h_r) + (d_x * d_x) -
		     (h_x * h_x)) / (2.0 * h_r * d_x);

		if (cos_tx_angle > cos_test_angle) {
			if (h_r == h_r_orig)
				fprintf(outfile,
					"Between %s and %s, obstructions were detected at:\n\n",
					rcvr.name, xmtr.name);

			if (site_x.lat >= 0.0) {
				if (metric)
					fprintf(outfile,
						"   %8.4f N,%9.4f W, %5.2f kilometers, %6.2f meters AMSL\n",
						site_x.lat, site_x.lon,
						KM_PER_MILE * (d_x / FEET_PER_MILE),
						METERS_PER_FOOT * (h_x -
								   earthradius));
				else
					fprintf(outfile,
						"   %8.4f N,%9.4f W, %5.2f miles, %6.2f feet AMSL\n",
						site_x.lat, site_x.lon,
						d_x / FEET_PER_MILE,
						h_x - earthradius);
			}

			else {
				if (metric)
					fprintf(outfile,
						"   %8.4f S,%9.4f W, %5.2f kilometers, %6.2f meters AMSL\n",
						-site_x.lat, site_x.lon,
						KM_PER_MILE * (d_x / FEET_PER_MILE),
						METERS_PER_FOOT * (h_x -
								   earthradius));
				else
					fprintf(outfile,
						"   %8.4f S,%9.4f W, %5.2f miles, %6.2f feet AMSL\n",
						-site_x.lat, site_x.lon,
						d_x / FEET_PER_MILE,
						h_x - earthradius);
			}
		}

		while (cos_tx_angle > cos_test_angle) {
			h_r += 1;
			cos_test_angle =
			    ((h_r * h_r) + (d_x * d_x) -
			     (h_x * h_x)) / (2.0 * h_r * d_x);
			cos_tx_angle =
			    ((h_r * h_r) + (d_tx * d_tx) -
			     (h_t * h_t)) / (2.0 * h_r * d_tx);
		}

		if (f) {
			/* Now clear the first Fresnel zone... */

			cos_tx_angle_f1 =
			    ((h_r_f1 * h_r_f1) + (d_tx * d_tx) -
			     (h_t * h_t)) / (2.0 * h_r_f1 * d_tx);
			h_los =
			    sqrt(h_r_f1 * h_r_f1 + d_x * d_x -
				 2 * h_r_f1 * d_x * cos_tx_angle_f1);
			h_f = h_los - sqrt(lambda * d_x * (d_tx - d_x) / d_tx);

			while (h_f < h_x) {
				h_r_f1 += 1;
				cos_tx_angle_f1 =
				    ((h_r_f1 * h_r_f1) + (d_tx * d_tx) -
				     (h_t * h_t)) / (2.0 * h_r_f1 * d_tx);
				h_los =
				    sqrt(h_r_f1 * h_r_f1 + d_x * d_x -
					 2 * h_r_f1 * d_x * cos_tx_angle_f1);
				h_f =
				    h_los -
				    sqrt(lambda * d_x * (d_tx - d_x) / d_tx);
			}

			/* and clear the 60% F1 zone. */

			cos_tx_angle_fpt6 =
			    ((h_r_fpt6 * h_r_fpt6) + (d_tx * d_tx) -
			     (h_t * h_t)) / (2.0 * h_r_fpt6 * d_tx);
			h_los =
			    sqrt(h_r_fpt6 * h_r_fpt6 + d_x * d_x -
				 2 * h_r_fpt6 * d_x * cos_tx_angle_fpt6);
			h_f =
			    h_los -
			    fzone_clearance * sqrt(lambda * d_x * (d_tx - d_x) /
						   d_tx);

			while (h_f < h_x) {
				h_r_fpt6 += 1;
				cos_tx_angle_fpt6 =
				    ((h_r_fpt6 * h_r_fpt6) + (d_tx * d_tx) -
				     (h_t * h_t)) / (2.0 * h_r_fpt6 * d_tx);
				h_los =
				    sqrt(h_r_fpt6 * h_r_fpt6 + d_x * d_x -
					 2 * h_r_fpt6 * d_x *
					 cos_tx_angle_fpt6);
				h_f =
				    h_los -
				    fzone_clearance * sqrt(lambda * d_x *
							   (d_tx - d_x) / d_tx);
			}
		}
	}

	if (h_r > h_r_orig) {
		if (metric)
			snprintf(string, 150,
				 "\nAntenna at %s must be raised to at least %.2f meters AGL\nto clear all obstructions detected.\n",
				 rcvr.name,
				 METERS_PER_FOOT * (h_r - GetElevation(rcvr) -
						    earthradius));
		else
			snprintf(string, 150,
				 "\nAntenna at %s must be raised to at least %.2f feet AGL\nto clear all obstructions detected.\n",
				 rcvr.name,
				 h_r - GetElevation(rcvr) - earthradius);
	}

	else
		snprintf(string, 150,
			 "\nNo obstructions to LOS path due to terrain were detected\n");

	if (f) {
		if (h_r_fpt6 > h_r_orig) {
			if (metric)
				snprintf(string_fpt6, 150,
					 "\nAntenna at %s must be raised to at least %.2f meters AGL\nto clear %.0f%c of the first Fresnel zone.\n",
					 rcvr.name,
					 METERS_PER_FOOT * (h_r_fpt6 -
							    GetElevation(rcvr) -
							    earthradius),
					 fzone_clearance * 100.0, 37);

			else
				snprintf(string_fpt6, 150,
					 "\nAntenna at %s must be raised to at least %.2f feet AGL\nto clear %.0f%c of the first Fresnel zone.\n",
					 rcvr.name,
					 h_r_fpt6 - GetElevation(rcvr) -
					 earthradius, fzone_clearance * 100.0,
					 37);
		}

		else
			snprintf(string_fpt6, 150,
				 "\n%.0f%c of the first Fresnel zone is clear.\n",
				 fzone_clearance * 100.0, 37);

		if (h_r_f1 > h_r_orig) {
			if (metric)
				snprintf(string_f1, 150,
					 "\nAntenna at %s must be raised to at least %.2f meters AGL\nto clear the first Fresnel zone.\n",
					 rcvr.name,
					 METERS_PER_FOOT * (h_r_f1 -
							    GetElevation(rcvr) -
							    earthradius));

			else
				snprintf(string_f1, 150,
					 "\nAntenna at %s must be raised to at least %.2f feet AGL\nto clear the first Fresnel zone.\n",
					 rcvr.name,
					 h_r_f1 - GetElevation(rcvr) -
					 earthradius);

		}

		else
			snprintf(string_f1, 150,
				 "\nThe first Fresnel zone is clear.\n");
	}

	fprintf(outfile, "%s", string);

	if (f) {
		fprintf(outfile, "%s", string_f1);
		fprintf(outfile, "%s", string_fpt6);
	}

}

void free_dem(void)
{
	int i;
	int j;

	for (i = 0; i < MAXPAGES; i++) {
		for (j = 0; j < IPPD; j++) {
			delete [] dem[i].data[j];
			delete [] dem[i].mask[j];
			delete [] dem[i].signal[j];
		}
		delete [] dem[i].data;
		delete [] dem[i].mask;
		delete [] dem[i].signal;
	}
	delete [] dem;
}

void free_elev(void) {
  delete [] elev;
}

void free_path(void)
{
	delete [] path.lat;
	delete [] path.lon;
	delete [] path.elevation;
	delete [] path.distance;
}

void alloc_elev(void)
{
  elev  = new double[ARRAYSIZE + 10];
}

static void alloc_dem(void)
{
	int i;
	int j;

	dem = new struct dem[MAXPAGES];
	for (i = 0; i < MAXPAGES; i++) {
		dem[i].data = new short *[IPPD];
		dem[i].mask = new unsigned char *[IPPD];
		dem[i].signal = new unsigned char *[IPPD];
		for (j = 0; j < IPPD; j++) {
			dem[i].data[j] = new short[IPPD];
			dem[i].mask[j] = new unsigned char[IPPD];
			dem[i].signal[j] = new unsigned char[IPPD];
		}
	}
}

void alloc_path(void)
{
	path.lat = new double[ARRAYSIZE];
	path.lon = new double[ARRAYSIZE];
	path.elevation = new double[ARRAYSIZE];
	path.distance = new double[ARRAYSIZE];
}

void do_allocs(void)
{
	int i;

	alloc_elev();
	alloc_dem();
	alloc_path();

	for (i = 0; i < MAXPAGES; i++) {
		dem[i].min_el = 32768;
		dem[i].max_el = -32768;
		dem[i].min_north = 90;
		dem[i].max_north = -90;
		dem[i].min_west = 360;
		dem[i].max_west = -1;
	}
}

std::vector<int> LatLongMinMax(double tx_lon, double bot_lon, double tx_lat, double top_lat){
	
	int min_lat = 0, min_lon = 0, max_lat = 0, max_lon = 0, rxlat = 0, rxlon = 0, txlat = 0, txlon = 0;
	std::vector<int> limits;
	min_lat = 70;
	max_lat = -70;

    min_lon = (int)floor(tx_lon);
	max_lon = (int)floor(tx_lon);
    
	txlat = (int)floor(tx_lat);
	txlon = (int)floor(tx_lon);

	if (txlat < min_lat)
		min_lat = txlat;

	if (txlat > max_lat)
		max_lat = txlat;

	if (LonDiff(txlon, min_lon) < 0.0)
		min_lon = txlon;

	if (LonDiff(txlon, max_lon) >= 0.0)
		max_lon = txlon;

    //top-left
    rxlat = (int)floor(top_lat);
    rxlon = (int)floor(bot_lon);
    
    if (rxlat < min_lat)
        min_lat = rxlat;

    if (rxlat > max_lat)
        max_lat = rxlat;

    if (LonDiff(rxlon, min_lon) < 0.0)
        min_lon = rxlon;

    if (LonDiff(rxlon, max_lon) >= 0.0)
        max_lon = rxlon;

	limits.push_back(max_lon);
	limits.push_back(min_lon);
	limits.push_back(max_lat);
	limits.push_back(min_lat);

	return limits;
}

void getDefaults(){

	dbm = 0;
	gpsav = 0;
	metric = 0;
	string[0] = 0;
	clutter = 0.0;
	forced_erp = -1.0;
	forced_freq = 0.0;
	sdf_path[0] = 0;

	path.length = 0;
	fzone_clearance = 0.6;
	contour_threshold = 0;
	resample = 0;

	earthradius = EARTHRADIUS;
	max_range = 1.0;
	lat = 0;
	lon = 0;
	txh = 0;
	ippd = IPPD;		// default resolution

	// Defaults
	LR.eps_dielect = 15.0;	// Farmland
	LR.sgm_conductivity = 0.005;	// Farmland
	LR.eno_ns_surfref = 301.0;
	LR.radio_climate = 5;	// continental
	LR.pol = 1;		// vert
	LR.conf = 0.50;
	LR.rel = 0.50;
	LR.erp = 0.0;		// will default to Path Loss


 }