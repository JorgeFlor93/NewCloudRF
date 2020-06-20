#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include "main.hh"
#include "inputs.hh"
#include "common.h"
#include <iostream>
/*
* Frequency: Any MHz
* Distance: Any km
*/

int main(int argc, char *argv[]){

    double Ploss = 0.0; //Ldb
    float f = 5000, d, height_Base = 100, height_Mobile = 2; //Tx_h y Rx_h, freq, distance
    int num_model = 0, mode = 2/*mode SUBURBAN*/;
    double wide_area = 0.0, height_area = 0.0;
    int error_if_1 = 0;
    
    /* SDF atributtes 
    int debug = 0, result, nortRxHin, west_min, west_max, z = 0; int nortRxHax, min_lat = 0, min_lon = 0, max_lat = 0, max_lon = 0, rxlat = 0, rxlon = 0, txlat = 0, txlon = 0;
    double tx_range = 0.0; double rx_range = 0.0; double deg_range = 0.0; double max_range = 1.0; double deg_limit = 0.0; double deg_range_lon = 0.0; double altitudeLR = 0.0; double altitude = 0.0;
    unsigned char area_mode = 0; unsigned char topomap = 0; unsigned char max_txsites = 30; unsigned char txsites = 0; unsigned char LRmap = 1;
    */

    /* CONVERSIONES
    char *tx_char_input;
    // defrees minutes seconds
    double tx_degrees = 55.895436, degrees_numeric = 0.0;
    std::cout.precision(6);
    std::cout << "Grados decimales: " << std::fixed << tx_degrees << "\n";
    tx_char_input = dec2dms(tx_degrees);
    degrees_numeric = ReadBearing(tx_char_input);
    std::cout << "char* d m s: " << tx_char_input << "\n";
    std::cout << "vuelta a grados decimales: " << std::fixed << degrees_numeric << "\n";
    */

    struct site antennas[6];
    /*Tx*/  
    antennas[0] = {55.895436, -3.257968, height_Base}; // TX
    //antennas[1] = {55.881092, -3.288290, height_Base};
    //antennas[2] = {55.876001, -3.226494, height_Base};

    /*Rx*/
    antennas[1] = {55.895017, -3.255537, height_Mobile}; //point to point receptor
    antennas[2] = {55.893994, -3.248520, height_Mobile}; //bot right area point
    antennas[3] = {55.895017, -3.255537, height_Mobile}; // top left area point 

    /* Load Digital Elevation Models Data*/
      // max and min lat and lon:

    // min_lat = 70;
	// max_lat = -70;

	// min_lon = (int)floor(antennas[0].lon);
	// max_lon = (int)floor(antennas[0].lon);

	// txlat = (int)floor(antennas[0].lat);
	// txlon = (int)floor(antennas[0].lon);

	// if (txlat < min_lat)
	// 	min_lat = txlat;

	// if (txlat > max_lat)
	// 	max_lat = txlat;

	// if (LonDiff(txlon, min_lon) < 0.0)
	// 	min_lon = txlon;

	// if (LonDiff(txlon, max_lon) >= 0.0)
	// 	max_lon = txlon;
    
    // rxlat = (int)floor(antennas[1].lat);
    // rxlon = (int)floor(antennas[1].lon);

    // if (rxlat < min_lat)
    //     min_lat = rxlat;

    // if (rxlat > max_lat)
    //     max_lat = rxlat;

    // if (LonDiff(rxlon, min_lon) < 0.0)
    //     min_lon = rxlon;

    // if (LonDiff(rxlon, max_lon) >= 0.0)
    //     max_lon = rxlon;
    

    // if(debug){
	// 		fprintf(stderr,"%.4f,%.4f,%.4f,%.4f,%.4f,%.4f\n",max_north,min_west,min_north,max_west,max_lon,min_lon);
	// }

    // if( (result = LoadTopoData(max_lon, min_lon, max_lat, min_lat)) != 0 ){
    //     // This only fails on errors loading SDF tiles
    //     fprintf(stderr, "Error loading topo data\n");
    //     return result;
    // }

    // if (area_mode || topomap) {
    //     for (z = 0; z < txsites && z < max_txsites; z++) {
    //         /* "Ball park" estimates used to load any additional
    //             SDF files required to conduct this analysis. */

    //         tx_range =sqrt(1.5 * (antennas[z].alt + GetElevation(antennas[z])));

    //         if (LRmap)
    //             rx_range = sqrt(1.5 * altitudeLR);
    //         else
    //             rx_range = sqrt(1.5 * altitude);

    //         /* deg_range determines the maximum
    //             amount of topo data we read */

    //         deg_range = (tx_range + rx_range) / 57.0;

    //         /* max_range regulates the size of the
    //             analysis.  A small, non-zero amount can
    //             be used to shrink the size of the analysis
    //             and limit the amount of topo data read by
    //             ss  A large number will increase the
    //             width of the analysis and the size of
    //             the map. */

    //         if (max_range == 0.0)
    //             max_range = tx_range + rx_range;

    //         deg_range = max_range / 57.0;

    //         // No more than 8 degs
    //         deg_limit = 3.5;

    //         if (fabs(antennas[z].lat) < 70.0)
    //             deg_range_lon =
    //                 deg_range / cos(DEG2RAD * antennas[z].lat);
    //         else
    //             deg_range_lon = deg_range / cos(DEG2RAD * 70.0);

    //         /* Correct for squares in degrees not being square in miles */

    //         if (deg_range > deg_limit)
    //             deg_range = deg_limit;

    //         if (deg_range_lon > deg_limit)
    //             deg_range_lon = deg_limit;

    //         nortRxHin = (int)floor(antennas[z].lat - deg_range);
    //         nortRxHax = (int)floor(antennas[z].lat + deg_range);

    //         west_min = (int)floor(antennas[z].lon - deg_range_lon);

    //         while (west_min < 0)
    //             west_min += 360;

    //         while (west_min >= 360)
    //             west_min -= 360;

    //         west_max = (int)floor(antennas[z].lon + deg_range_lon);

    //         while (west_max < 0)
    //             west_max += 360;

    //         while (west_max >= 360)
    //             west_max -= 360;

    //         if (nortRxHin < min_lat)
    //             min_lat = nortRxHin;

    //         if (nortRxHax > max_lat)
    //             max_lat = nortRxHax;

    //         if (LonDiff(west_min, min_lon) < 0.0)
    //             min_lon = west_min;

    //         if (LonDiff(west_max, max_lon) >= 0.0)
    //             max_lon = west_max;
    //     }

    //     /* Load any additional SDF files, if required */

    //     if( (result = LoadTopoData(max_lon, min_lon, max_lat, min_lat)) != 0 ){
    //         // This only fails on errors loading SDF tiles
    //         fprintf(stderr, "Error loading topo data\n");
    //         return result;
    //     }
    // }
    // ppd=(double)ippd;
    // yppd=ppd; 

    // width = (unsigned)(ippd * ReduceAngle(max_west - min_west));
    // height = (unsigned)(ippd * ReduceAngle(max_north - min_north));

    //std::cout << "max_lat: " << max_lat << ", min_lat: " << min_lat << ", max_lon: " << max_lon << ", min_lon: " << min_lon << "\n";

    /* SIMPLIFICANDO SDF
    snprintf(string, 19, "%s", "55:56:3:4.sdf");
	error_if_1 = LoadSDF_SDF(string);
    //error_if_1 = LoadTopoData(max_lon, min_lon, max_lat, min_lat);
    std::cout << "error_if_1: " << error_if_1 << "\n";
    //ReadPath(antennas[0], antennas[0]);
    */

    /* 
    * AREA.
    * min-right: 55.893994, -3.248520.
    * max-left: 55.895017, -3.255537.
    * Area from antenna[0] to this min and max area.
    */

    //Wide and height area 
    antennas[4] = {antennas[2].lat, antennas[3].lon, height_Mobile}; // bot left point
    antennas[5] = {antennas[3].lat, antennas[2].lon, height_Mobile}; // top right point
    height_area = Distance(antennas[3], antennas[4]); //km
    wide_area = Distance(antennas[3], antennas[5]); //km
    //std::cout << "Height: " << height_area << ", Ancho: " << wide_area << "\n";


    /*
    std::cout << "1. fspl" << "\n";
    std::cout << "2. Hata" << "\n";
    std::cout << "Choose propagation model(1,2): ";
    std::cin >> num_model;
    


    d = Distance(antennas[0], antennas[0]);

    if(num_model == 1){
        Ploss = FSPLpathLoss(f, d);
    }
    else if(num_model == 2){
        Ploss = HATApathLoss(f, antennas[0].alt, antennas[0].alt, d, mode);
    }
    else{
        std::cout << "Wrong number input" << "\n";
        return 0;
    }
    std::cout << "Distance (km): " << d << "\n";
    std::cout << "Pathloss are: " << Ploss << "\n";
    */
    
    return 0;
}






/*
muchas antenas a un receptor:
for(int i = 0; i < 3; i++){

        d = Distance(antennas[i], antennas[0]);

        if(num_model == 1){
            Ploss = FSPLpathLoss(f, d);
        }
        else if(num_model == 2){
            Ploss = HATApathLoss(f, antennas[i].alt, antennas[0].alt, d, mode);
        }
        else{
            std::cout << "Wrong number input" << "\n";
            return 0;
        }
        std::cout << "Distance (km): " << d << "\n";
        std::cout << "Pathloss are: " << Ploss << "\n";
    }

     -lat Tx Latitude (decimal degrees) -70/+70
     -lon Tx Longitude (decimal degrees) -180/+180
     -txh Tx Height (above ground)
     -rla (Optional) Rx Latitude for PPA (decimal degrees) -70/+70
     -rlo (Optional) Rx Longitude for PPA (decimal degrees) -180/+180
     -f Tx Frequency (MHz) 20MHz to 100GHz (LOS after 20GHz)
     -erp Tx Effective Radiated Power (Watts) including Tx+Rx gain
     -rxh Rx Height(s) (optional. Default=0.1)
     -rxg Rx gain dBi (optional for text report)
     -hp Horizontal Polarisation (default=vertical)
     -gc Random ground clutter (feet/meters)
     -m Metric units of measurement
     -te Terrain code 1-6 (optional)
     -terdic Terrain dielectric value 2-80 (optional)
     -tercon Terrain conductivity 0.01-0.0001 (optional)
     -cl Climate code 1-6 (optional)
     -rel Reliability for ITM model 50 to 99 (optional)
     -resample Resample Lidar input to specified resolution in meters (optional)
*/
