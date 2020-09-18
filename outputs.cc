#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "common.h"
#include "auxFuncts.hh"
#include "inputs.hh"
#include "models/cost.hh"
#include "models/ecc33.hh"
#include "models/ericsson.hh"
#include "models/fspl.hh"
#include "models/hata.hh"
#include "models/itwom3.0.hh"
#include "models/sui.hh"

/*Usamos esta función para calcular las pérdidas. 
Esta función inicialmente escribe en un fichero las caracteristicas de propagacion entre afuente-destino luego puede servirnos para obtener más parámetros.*/

double PathReport(struct site source, struct site destination, int propmodel, int pmenv)
{

	int x, y, z, errnum;
	char basename[255], term[30], ext[15], strmode[100],
	    report_name[80], block = 0;
	double maxloss = -100000.0, minloss = 100000.0, angle1, angle2,
	    azimuth, pattern = 1.0, patterndB = 0.0,
	    total_loss = 0.0, cos_xmtr_angle, cos_test_angle = 0.0,
	    source_alt, test_alt, dest_alt, source_alt2, dest_alt2,
	    distance, elevation, four_thirds_earth,
	    free_space_loss = 0.0, eirp =
	    0.0, voltage, rxp, power_density, dkm;
	

	
	four_thirds_earth = FOUR_THIRDS * EARTHRADIUS;
	
	azimuth = Azimuth(source, destination);
	angle1 = ElevationAngle(source, destination);
	angle2 = ElevationAngle2(source, destination, earthradius);

	x = (int)rint(10.0 * (10.0 - angle2));

	if (x >= 0 && x <= 1000){
		pattern =(double)LR.antenna_pattern[(int)rint(azimuth)][x];
	}
	patterndB = 20.0 * log10(pattern);

	azimuth = Azimuth(destination, source);

	angle1 = ElevationAngle(destination, source);
	angle2 = ElevationAngle2(destination, source, earthradius);

	if (LR.erp != 0.0) {

		dBm = 10.0 * (log10(LR.erp * 1000.0));		

		eirp = LR.erp * 1.636816521;
	
		dBm = 10.0 * (log10(eirp * 1000.0));
		
	}

	ReadPath(source, destination);	/* source=TX, destination=RX */
	/* Copy elevations plus clutter along
		path into the elev[] array. */

	for (x = 1; x < path.length - 1; x++){
		elev[x + 2] =
			METERS_PER_FOOT * (path.elevation[x] ==
						0.0 ? path.
						elevation[x] : (clutter +
								path.
								elevation[x]));
		//std::cout << "path_elevation: " << path.elevation[x] << ", elev: " << elev[x+2] << std::endl;
	}
	/* Copy ending points without clutter */

	elev[2] = path.elevation[0] * METERS_PER_FOOT;
	elev[path.length + 1] = path.elevation[path.length - 1] * METERS_PER_FOOT;
	
	azimuth = rint(Azimuth(source, destination));

	/* FUERZO A QUE ME CALCULE 1 PUNTO, EL QUE CAE DENTRO DEL AREA, Y ME QUITO LOS INTERMEDIOS*/
	y = path.length - 1;/* path.length-1 avoids LR error */
	distance = FEET_PER_MILE * path.distance[y];
	source_alt = four_thirds_earth + source.alt + path.elevation[0];
	dest_alt = four_thirds_earth + destination.alt +
		path.elevation[y];
	dest_alt2 = dest_alt * dest_alt;
	source_alt2 = source_alt * source_alt;

	/* Calculate the cosine of the elevation of
		the receiver as seen by the transmitter. */

	cos_xmtr_angle =
		((source_alt2) + (distance * distance) -
			(dest_alt2)) / (2.0 * source_alt * distance);

	if (got_elevation_pattern) {
		/* If an antenna elevation pattern is available, the
			following code determines the elevation angle to
			the first obstruction along the path. */

		for (x = 2, block = 0; x < y && block == 0; x++) {
			distance =
				FEET_PER_MILE * (path.distance[y] -
						path.distance[x]);
			test_alt =
				four_thirds_earth +
				path.elevation[x];

			/* Calculate the cosine of the elevation
				angle of the terrain (test point)
				as seen by the transmitter. */

			cos_test_angle =
				((source_alt2) +
					(distance * distance) -
					(test_alt * test_alt)) / (2.0 *
								source_alt
								*
								distance);

			/* Compare these two angles to determine if
				an obstruction exists.  Since we're comparing
				the cosines of these angles rather than
				the angles themselves, the sense of the
				following "if" statement is reversed from
				what it would be if the angles themselves
				were compared. */

			if (cos_xmtr_angle >= cos_test_angle)
				block = 1;
		}

		/* At this point, we have the elevation angle
			to the first obstruction (if it exists). */
	}

	/* Determine path loss for each point along the
		path using Longley-Rice's point_to_point mode
		starting at x=2 (number_of_points = 1), the
		shortest distance terrain can play a role in
		path loss. */
	
	elev[0] = y - 1;	/* (number of points - 1) */

	/* Distance between elevation samples */

	elev[1] =
		METERS_PER_MILE * (path.distance[y] -
					path.distance[y - 1]);

	/*
		point_to_point(elev, source.alt*METERS_PER_FOOT,
		destination.alt*METERS_PER_FOOT, LR.eps_dielect,
		LR.sgm_conductivity, LR.eno_ns_surfref, LR.frq_mhz,
		LR.radio_climate, LR.pol, LR.conf, LR.rel, loss,
		strmode, errnum);
		*/
	dkm = (elev[1] * elev[0]) / 1000;	// km
	//std::cout <<  "ELEV: " << path.elevation[y] << std::endl;
	// std::cout <<  "h_tx: " << source.alt * METERS_PER_FOOT << ", h_M: " << 
	// 			 (path.elevation[y] * METERS_PER_FOOT) +
	// 			 (destination.alt * METERS_PER_FOOT) << std::endl <<" d: "<<  dkm << ", pmenv: "<<  pmenv << std::endl;
	
	//std::cout << "last: [" << path.lat[y] << ", " << path.lon[y] << "], elev: " << path.elevation[y] << std::endl;
	switch (propmodel) {	
	case 1:
		//HATA 1, 2 & 3
		loss =
			HATApathLoss(LR.frq_mhz, source.alt * METERS_PER_FOOT,
					(path.elevation[y] * METERS_PER_FOOT) + (destination.alt * METERS_PER_FOOT), 
					dkm, pmenv);
		break;
	
	case 2:
		// ITU-R P.525 Free space path loss
		loss = FSPLpathLoss(LR.frq_mhz, dkm);
		break;
	}
	return loss;
}