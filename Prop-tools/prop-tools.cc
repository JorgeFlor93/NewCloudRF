#include "prop-tools.hh"

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