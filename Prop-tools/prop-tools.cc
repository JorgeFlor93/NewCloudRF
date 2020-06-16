#include "prop-tools.hh"

char string_to_dms[20];

double Distance(struct site site1, struct site site2)
{
	// This function returns the great circle distance
	//   in miles between any two site locations. 

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

double miles_to_km(double miles){   
    return miles*1.60934;
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

	string_to_dms[0] = 0;
	snprintf(string_to_dms, 250, "%d %d %d", degrees * sign, minutes,
		 seconds);
	return (string_to_dms);
}