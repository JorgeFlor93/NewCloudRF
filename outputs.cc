#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "common.h"
#include "main.hh"
#include "inputs.hh"
#include "models/cost.hh"
#include "models/ecc33.hh"
#include "models/ericsson.hh"
#include "models/fspl.hh"
#include "models/hata.hh"
#include "models/itwom3.0.hh"
#include "models/sui.hh"

void PathReport(struct site source, struct site destination, char *name, int propmodel, int pmenv, double rxGain)
{
	/* This function writes a PPA Path Report (name.txt) to
	   the filesystem.  If (graph_it == 1), then gnuplot is invoked
	   to generate an appropriate output file indicating the Longley-Rice
	   model loss between the source and destination locations.
	   "filename" is the name assigned to the output file generated
	   by gnuplot.  The filename extension is used to set gnuplot's
	   terminal setting and output file type.  If no extension is
	   found, .png is assumed. */

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
	FILE *fd = NULL, *fd2 = NULL;

	snprintf(report_name, 80, "%s.txt%c", name, 0);
	four_thirds_earth = FOUR_THIRDS * EARTHRADIUS;

	fd2 = fopen(report_name, "w");

	fprintf(fd2, "\n\t\t--==[ Path Profile Analysis ]==--\n\n");
	fprintf(fd2, "Transmitter site: %s\n", source.name);

	if (source.lat >= 0.0) {

		if (source.lon <= 180){
			fprintf(fd2, "Site location: %.4f, -%.4f\n",source.lat, source.lon);
		}else{
			fprintf(fd2, "Site location: %.4f, %.4f\n",source.lat, 360 - source.lon);
		}
	}

	else {

		if (source.lon <= 180){
			fprintf(fd2, "Site location: %.4f, -%.4f\n",source.lat, source.lon);
		}else{
			fprintf(fd2, "Site location: %.4f, %.4f\n",source.lat, 360 - source.lon);
		}
	}

	if (metric) {
		fprintf(fd2, "Ground elevation: %.2f meters AMSL\n",
			METERS_PER_FOOT * GetElevation(source));
		fprintf(fd2,
			"Antenna height: %.2f meters AGL / %.2f meters AMSL\n",
			METERS_PER_FOOT * source.alt,
			METERS_PER_FOOT * (source.alt + GetElevation(source)));
	}
	else { // se guarda en un puntero a fichero datos elevación de tx
		fprintf(fd2, "Ground elevation: %.2f feet AMSL\n",
			GetElevation(source));
		fprintf(fd2, "Antenna height: %.2f feet AGL / %.2f feet AMSL\n",
			source.alt, source.alt + GetElevation(source));
	}

	azimuth = Azimuth(source, destination);
	angle1 = ElevationAngle(source, destination);
	angle2 = ElevationAngle2(source, destination, earthradius);

		x = (int)rint(10.0 * (10.0 - angle2));

		if (x >= 0 && x <= 1000)
			pattern =
			    (double)LR.antenna_pattern[(int)rint(azimuth)][x];

		patterndB = 20.0 * log10(pattern);

	if (metric)
		fprintf(fd2, "Distance to %s: %.2f kilometers\n",
			destination.name, KM_PER_MILE * Distance(source,
								 destination));

	else
		fprintf(fd2, "Distance to %s: %.2f miles\n", destination.name,
			Distance(source, destination));

	fprintf(fd2, "Azimuth to %s: %.2f degrees grid\n", destination.name,
		azimuth);


	fprintf(fd2, "Downtilt angle to %s: %+.4f degrees\n",
		destination.name, angle1);



	/* Receiver */

	fprintf(fd2, "\nReceiver site: %s\n", destination.name);

	if (destination.lat >= 0.0) {

		if (destination.lon <= 180){
			fprintf(fd2, "Site location: %.4f, -%.4f\n",destination.lat, destination.lon);
		}else{
			fprintf(fd2, "Site location: %.4f, %.4f\n",destination.lat, 360 - destination.lon);
		}
	}

	else {

		if (destination.lon <= 180){
			fprintf(fd2, "Site location: %.4f, -%.4f\n",destination.lat, destination.lon);
		}else{
			fprintf(fd2, "Site location: %.4f, %.4f\n",destination.lat, 360 - destination.lon);
		}
	}

	if (metric) {
		fprintf(fd2, "Ground elevation: %.2f meters AMSL\n",
			METERS_PER_FOOT * GetElevation(destination));
		fprintf(fd2,
			"Antenna height: %.2f meters AGL / %.2f meters AMSL\n",
			METERS_PER_FOOT * destination.alt,
			METERS_PER_FOOT * (destination.alt +
					   GetElevation(destination)));
	}

	else {
		fprintf(fd2, "Ground elevation: %.2f feet AMSL\n",
			GetElevation(destination));
		fprintf(fd2, "Antenna height: %.2f feet AGL / %.2f feet AMSL\n",
			destination.alt,
			destination.alt + GetElevation(destination));
	}

	if (metric)
		fprintf(fd2, "Distance to %s: %.2f kilometers\n", source.name,
			KM_PER_MILE * Distance(source, destination));

	else
		fprintf(fd2, "Distance to %s: %.2f miles\n", source.name,
			Distance(source, destination));

	azimuth = Azimuth(destination, source);

	angle1 = ElevationAngle(destination, source);
	angle2 = ElevationAngle2(destination, source, earthradius);

	fprintf(fd2, "Azimuth to %s: %.2f degrees grid\n", source.name, azimuth);


	fprintf(fd2, "Downtilt angle to %s: %+.4f degrees\n",
		source.name, angle1);

	if (LR.frq_mhz > 0.0) {
		fprintf(fd2, "\n\nPropagation model: ");

		switch (propmodel) {
		case 1:
			fprintf(fd2, "Okumura-Hata\n");
			break;
		case 2:
			fprintf(fd2, "Free space path loss (ITU-R.525)\n");
			break;
		}

		fprintf(fd2, "Model sub-type: ");

		switch (pmenv) {
		case 1:
			fprintf(fd2, "City / Conservative\n");
			break;
		case 2:
			fprintf(fd2, "Suburban / Average\n");
			break;
		case 3:
			fprintf(fd2, "Rural / Optimistic\n");
			break;
		}
		// fprintf(fd2, "Earth's Dielectric Constant: %.3lf\n",
		// 	LR.eps_dielect);
		// fprintf(fd2, "Earth's Conductivity: %.3lf Siemens/meter\n",
		// 	LR.sgm_conductivity);
		// fprintf(fd2,
		// 	"Atmospheric Bending Constant (N-units): %.3lf ppm\n",
		// 	LR.eno_ns_surfref);
		fprintf(fd2, "Frequency: %.3lf MHz\n", LR.frq_mhz);
		// fprintf(fd2, "Radio Climate: %d (", LR.radio_climate);

		switch (LR.radio_climate) {
		case 1:
			fprintf(fd2, "Equatorial");
			break;

		case 2:
			fprintf(fd2, "Continental Subtropical");
			break;

		case 3:
			fprintf(fd2, "Maritime Subtropical");
			break;

		case 4:
			fprintf(fd2, "Desert");
			break;

		case 5:
			fprintf(fd2, "Continental Temperate");
			break;

		case 6:
			fprintf(fd2, "Maritime Temperate, Over Land");
			break;

		case 7:
			fprintf(fd2, "Maritime Temperate, Over Sea");
			break;

		default:
			fprintf(fd2, "Unknown");
		}

		fprintf(fd2, ")\nPolarisation: %d (", LR.pol);

		if (LR.pol == 0)
			fprintf(fd2, "Horizontal");

		if (LR.pol == 1)
			fprintf(fd2, "Vertical");

		fprintf(fd2, ")\nFraction of Situations: %.1lf%c\n",
			LR.conf * 100.0, 37);
		fprintf(fd2, "Fraction of Time: %.1lf%c\n", LR.rel * 100.0, 37);

		if (LR.erp != 0.0) {
			fprintf(fd2, "\nReceiver gain: %.1f dBd / %.1f dBi\n", rxGain, rxGain+2.14);
			fprintf(fd2, "Transmitter ERP plus Receiver gain: ");

			if (LR.erp < 1.0)
				fprintf(fd2, "%.1lf milliwatts",
					1000.0 * LR.erp);

			if (LR.erp >= 1.0 && LR.erp < 10.0)
				fprintf(fd2, "%.1lf Watts", LR.erp);

			if (LR.erp >= 10.0 && LR.erp < 10.0e3)
				fprintf(fd2, "%.0lf Watts", LR.erp);

			if (LR.erp >= 10.0e3)
				fprintf(fd2, "%.3lf kilowatts", LR.erp / 1.0e3);

			dBm = 10.0 * (log10(LR.erp * 1000.0));
			fprintf(fd2, " (%+.2f dBm)\n", dBm);
			fprintf(fd2, "Transmitter ERP minus Receiver gain: %.2f dBm\n", dBm-rxGain);

			/* EIRP = ERP + 2.14 dB */

			fprintf(fd2, "Transmitter EIRP plus Receiver gain: ");

			eirp = LR.erp * 1.636816521;

			if (eirp < 1.0)
				fprintf(fd2, "%.1lf milliwatts", 1000.0 * eirp);

			if (eirp >= 1.0 && eirp < 10.0)
				fprintf(fd2, "%.1lf Watts", eirp);

			if (eirp >= 10.0 && eirp < 10.0e3)
				fprintf(fd2, "%.0lf Watts", eirp);

			if (eirp >= 10.0e3)
				fprintf(fd2, "%.3lf kilowatts", eirp / 1.0e3);

			dBm = 10.0 * (log10(eirp * 1000.0));
			fprintf(fd2, " (%+.2f dBm)\n", dBm);

			// Rx gain
			fprintf(fd2, "Transmitter EIRP minus Receiver gain: %.2f dBm\n", dBm-rxGain);
		}

		fprintf(fd2, "\nSummary for the link between %s and %s:\n\n",
			source.name, destination.name);

		fprintf(fd2, "%s antenna pattern towards %s: %.3f (%.2f dB)\n",
				source.name, destination.name, pattern,
				patterndB);

		ReadPath(source, destination);	/* source=TX, destination=RX */
		std::cout << "path lenght: " << path.length - 1 << std::endl;
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
		//std::cout << "ending_points, elev[2]: " << elev[2] << std::endl; 
		azimuth = rint(Azimuth(source, destination));
		
		for (y = 2; y < (path.length - 1); y++) {	/* path.length-1 avoids LR error */
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

			if (block)
				elevation =
				    ((acos(cos_test_angle)) / DEG2RAD) - 90.0;
			else
				elevation =
				    ((acos(cos_xmtr_angle)) / DEG2RAD) - 90.0;

			/* Integrate the antenna's radiation
			   pattern into the overall path loss. */

			x = (int)rint(10.0 * (10.0 - elevation));

			if (x >= 0 && x <= 1000) {
				pattern =
				    (double)LR.antenna_pattern[(int)azimuth][x];

				if (pattern != 0.0){
					patterndB = 20.0 * log10(pattern);
				}else{
					patterndB = 0.0;
				}
			}

			else
				patterndB = 0.0;

			total_loss = loss - patterndB;

			if (total_loss > maxloss)
				maxloss = total_loss;

			if (total_loss < minloss)
				minloss = total_loss;
			
			std::cout << "loss: " << loss << ", total loss: " << total_loss << std::endl;
		}

		distance = Distance(source, destination);

		if (distance != 0.0) {
			free_space_loss =
			    36.6 + (20.0 * log10(LR.frq_mhz)) +
			    (20.0 * log10(distance));
			fprintf(fd2, "Free space path loss: %.2f dB\n",
				free_space_loss);
		}

		fprintf(fd2, "Computed path loss: %.2f dB\n", loss);


                if((loss*1.5) < free_space_loss){
			fprintf(fd2,"Model error! Computed loss of %.1fdB is greater than free space loss of %.1fdB. Check your inuts for model %d\n",loss,free_space_loss,propmodel);
                        fprintf(stderr,"Model error! Computed loss of %.1fdB is greater than free space loss of %.1fdB. Check your inuts for model %d\n",loss,free_space_loss,propmodel);
                        return;
                }

		if (free_space_loss != 0.0)
			fprintf(fd2,
				"Attenuation due to terrain shielding: %.2f dB\n",
				loss - free_space_loss);

		fprintf(fd2,"Total path loss including %s antenna pattern: %.2f dB\n",
				source.name, total_loss);

		if (LR.erp != 0.0) {
			field_strength =
			    (139.4 + (20.0 * log10(LR.frq_mhz)) - total_loss) +
			    (10.0 * log10(LR.erp / 1000.0));

			/* dBm is referenced to EIRP */

			rxp = eirp / (pow(10.0, (total_loss / 10.0)));
			dBm = 10.0 * (log10(rxp * 1000.0));
			power_density =
			    (eirp /
			     (pow
			      (10.0, (total_loss - free_space_loss) / 10.0)));
			/* divide by 4*PI*distance_in_meters squared */
			power_density /= (4.0 * PI * distance * distance *
					  2589988.11);

			fprintf(fd2, "Field strength at %s: %.2f dBuV/meter\n",
				destination.name, field_strength);
			fprintf(fd2, "Signal power level at %s: %+.2f dBm\n",
				destination.name, dBm);
			fprintf(fd2,
				"Signal power density at %s: %+.2f dBW per square meter\n",
				destination.name, 10.0 * log10(power_density));
			voltage =
			    1.0e6 * sqrt(50.0 *
					 (eirp /
					  (pow
					   (10.0,
					    (total_loss - 2.14) / 10.0))));
			fprintf(fd2,
				"Voltage across 50 ohm dipole at %s: %.2f uV (%.2f dBuV)\n",
				destination.name, voltage,
				20.0 * log10(voltage));

			voltage =
			    1.0e6 * sqrt(75.0 *
					 (eirp /
					  (pow
					   (10.0,
					    (total_loss - 2.14) / 10.0))));
			fprintf(fd2,
				"Voltage across 75 ohm dipole at %s: %.2f uV (%.2f dBuV)\n",
				destination.name, voltage,
				20.0 * log10(voltage));
		}

		if (propmodel == 1) {
			fprintf(fd2, "Longley-Rice model error number: %d",
				errnum);

			switch (errnum) {
			case 0:
				fprintf(fd2, " (No error)\n");
				break;

			case 1:
				fprintf(fd2,
					"\n  Warning: Some parameters are nearly out of range.\n");
				fprintf(fd2,
					"  Results should be used with caution.\n");
				break;

			case 2:
				fprintf(fd2,
					"\n  Note: Default parameters have been substituted for impossible ones.\n");
				break;

			case 3:
				fprintf(fd2,
					"\n  Warning: A combination of parameters is out of range for this model.\n");
				fprintf(fd2,
					"  Results should be used with caution.\n");
				break;

			default:
				fprintf(fd2,
					"\n  Warning: Some parameters are out of range for this model.\n");
				fprintf(fd2,
					"  Results should be used with caution.\n");
			}
		}

	}

	ObstructionAnalysis(source, destination, LR.frq_mhz, fd2);
	fclose(fd2);

	// fprintf(stderr,
	// 	"Path loss (dB), Received Power (dBm), Field strength (dBuV):\n%.1f\n%.1f\n%.1f",
	// 	loss, dBm, field_strength);
	std::cout << std::endl;
	// /* Skip plotting the graph if ONLY a path-loss report is needed. */

	// if (graph_it) {
	// 	if (name[0] == '.') {
	// 		/* Default filename and output file type */

	// 		strncpy(basename, "profile\0", 8);
	// 		strncpy(term, "png\0", 4);
	// 		strncpy(ext, "png\0", 4);
	// 	}

	// 	else {
	// 		/* Extract extension and terminal type from "name" */

	// 		ext[0] = 0;
	// 		y = strlen(name);
	// 		strncpy(basename, name, 254);

	// 		for (x = y - 1; x > 0 && name[x] != '.'; x--) ;

	// 		if (x > 0) {	/* Extension found */
	// 			for (z = x + 1; z <= y && (z - (x + 1)) < 10;
	// 			     z++) {
	// 				ext[z - (x + 1)] = tolower(name[z]);
	// 				term[z - (x + 1)] = name[z];
	// 			}

	// 			ext[z - (x + 1)] = 0;	/* Ensure an ending 0 */
	// 			term[z - (x + 1)] = 0;
	// 			basename[x] = 0;
	// 		}
	// 	}

	// 	if (ext[0] == 0) {	/* No extension -- Default is png */
	// 		strncpy(term, "png\0", 4);
	// 		strncpy(ext, "png\0", 4);
	// 	}

	// 	/* Either .ps or .postscript may be used
	// 	   as an extension for postscript output. */

	// 	if (strncmp(term, "postscript", 10) == 0)
	// 		strncpy(ext, "ps\0", 3);

	// 	else if (strncmp(ext, "ps", 2) == 0)
	// 		strncpy(term, "postscript enhanced color\0", 26);

	// 	fd = fopen("ppa.gp", "w");

	// 	fprintf(fd, "set grid\n");
	// 	fprintf(fd, "set yrange [%2.3f to %2.3f]\n", minloss, maxloss);
	// 	fprintf(fd, "set encoding iso_8859_1\n");
	// 	fprintf(fd, "set term %s\n", term);
	// 	fprintf(fd,
	// 		"set title \"Path Loss Profile Along Path Between %s and %s (%.2f%c azimuth)\"\n",
	// 		destination.name, source.name, Azimuth(destination,
	// 						       source), 176);

	// 	if (metric)
	// 		fprintf(fd,
	// 			"set xlabel \"Distance Between %s and %s (%.2f kilometers)\"\n",
	// 			destination.name, source.name,
	// 			KM_PER_MILE * Distance(destination, source));
	// 	else
	// 		fprintf(fd,
	// 			"set xlabel \"Distance Between %s and %s (%.2f miles)\"\n",
	// 			destination.name, source.name,
	// 			Distance(destination, source));

	// 	if (got_azimuth_pattern || got_elevation_pattern)
	// 		fprintf(fd,
	// 			"set ylabel \"Total Path Loss (including TX antenna pattern) (dB)");
	// 	else
	// 		fprintf(fd, "set ylabel \"Longley-Rice Path Loss (dB)");

	// 	fprintf(fd, "\"\nset output \"%s.%s\"\n", basename, ext);
	// 	fprintf(fd,
	// 		"plot \"profile.gp\" title \"Path Loss\" with lines\n");

	// 	fclose(fd);

	// 	x = system("gnuplot ppa.gp");

	// 	if (x != -1) {
	// 		if (gpsav == 0) {
	// 			//unlink("ppa.gp");
	// 			//unlink("profile.gp");
	// 			//unlink("reference.gp");
	// 		}

	// 	}

	// 	else
	// 		fprintf(stderr,
	// 			"\n*** ERROR: Error occurred invoking gnuplot!\n");
	// }

}