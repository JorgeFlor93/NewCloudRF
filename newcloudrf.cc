#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include "main.hh"
#include "inputs.hh"
#include "outputs.hh"
#include "common.h"
#include "distance.hh"
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>

/*
* Frequency: Any MHz
* Distance: Any km
*/

int main(int argc, char *argv[]){

    double Ploss = 0.0; //Ldb
    float f = 5000, d, height_Base = 100, height_Mobile = 2; /*Tx_h y Rx_h, freq, distance*/
    int num_model = 0, mode = 2/*mode SUBURBAN*/;
    int x; int y;
    
    /* SDF atributtes */  
    int min_lat = 0, min_lon = 0, max_lat = 0, max_lon = 0, rxlat = 0, rxlon = 0, txlat = 0, txlon = 0;
    int debug = 0, result, nortRxHin, west_min, west_max, z = 0;
    
    struct site antennas[2];

    /* Inicializamos vaiables*/ 
    MAXRAD = 200; 
    antennas[0].lat = 91.0;
	antennas[0].lon = 361.0;
	antennas[1].lat = 91.0;
	antennas[1].lon = 361.0;
    
    getMain();
    do_allocs();
    LR.frq_mhz = f;

    /* -lat 51.849 -lon -2.2299 */
    /*Tx*/  
    antennas[0] = {51.349, -2.2299, height_Base}; 
    /*Rx*/
    antennas[1] = {51.449, -2.2399, height_Mobile}; //point to point receptor

    /* LOAD SDF INFO */

    antennas[0].lon *= -1;
    if (antennas[0].lon < 0.0)
        antennas[0].lon += 360.0; 
    antennas[1].lon *= -1;
    if (antennas[1].lon < 0.0)
        antennas[1].lon += 360.0;

    if (sdf_path[0]) {
    x = strlen(sdf_path);

        if (sdf_path[x - 1] != '/' && x != 0) {
            sdf_path[x] = '/';
            sdf_path[x + 1] = 0;
        }
    }

	x = 0;
	y = 0;

	min_lat = 70;
	max_lat = -70;

    min_lon = (int)floor(antennas[0].lon);
	max_lon = (int)floor(antennas[0].lon);

	txlat = (int)floor(antennas[0].lat);
	txlon = (int)floor(antennas[0].lon);

	if (txlat < min_lat)
		min_lat = txlat;

	if (txlat > max_lat)
		max_lat = txlat;

	if (LonDiff(txlon, min_lon) < 0.0)
		min_lon = txlon;

	if (LonDiff(txlon, max_lon) >= 0.0)
		max_lon = txlon;

    rxlat = (int)floor(antennas[1].lat);
    rxlon = (int)floor(antennas[1].lon);

    if (rxlat < min_lat)
        min_lat = rxlat;

    if (rxlat > max_lat)
        max_lat = rxlat;

    if (LonDiff(rxlon, min_lon) < 0.0)
        min_lon = rxlon;

    if (LonDiff(rxlon, max_lon) >= 0.0)
        max_lon = rxlon;
	
    result = LoadTopoData(max_lon, min_lon, max_lat, min_lat);
    ppd = (double)ippd;
    yppd=ppd; 
    dpp = 1 / ppd;
 	mpi = ippd-1;

    /* Calculate PATH LOSS*/ 
    
    PathReport(antennas[0], antennas[1], antennas[0].filename, 1, 3, 0);

    free_dem();
    free_elev();
    free_path();
    fflush(stderr);
    return 0;
}





//  std::cout << "max_lon: " << max_lon << ", min_lon: " << min_lon << ", max_lat: " << max_lat << ", min_lat: " << min_lat << std::endl;

// muchas antenas a un receptor:
// for(int i = 0; i < 3; i++){

//         d = Distance(antennas[i], antennas[0]);

//         if(num_model == 1){
//             Ploss = FSPLpathLoss(f, d);
//         }
//         else if(num_model == 2){
//             Ploss = HATApathLoss(f, antennas[i].alt, antennas[0].alt, d, mode);
//         }
//         else{
//             std::cout << "Wrong number input" << "\n";
//             return 0;
//         }
//         std::cout << "Distance (km): " << d << "\n";
//         std::cout << "Pathloss are: " << Ploss << "\n";
//     }

    //  Accediendo al elemento 7->long: */
    // std::cout << std::endl;
    // std::cout << point_list[7](0,0);
    // std::cout << std::endl;

    // antennas[4] = {antennas[2].lat, antennas[3].lon, height_Mobile}; // bot left point
    // antennas[5] = {antennas[3].lat, antennas[2].lon, height_Mobile}; // top right point
    // height_area = Distance(antennas[3], antennas[4]); //km
    // wide_area = Distance(antennas[3], antennas[5]); //km
    //std::cout << "Height: " << height_area << ", Ancho: " << wide_area << "\n";
     
    // std::cout.precision(12);
    // for(int j = 0; j < point_list.size(); j++)
    //     std::cout <<"[" << std::fixed << point_list[j] << "] ";

//      -lat Tx Latitude (decimal degrees) -70/+70
//      -lon Tx Longitude (decimal degrees) -180/+180
//      -txh Tx Height (above ground)
//      -rla (Optional) Rx Latitude for PPA (decimal degrees) -70/+70
//      -rlo (Optional) Rx Longitude for PPA (decimal degrees) -180/+180
//      -f Tx Frequency (MHz) 20MHz to 100GHz (LOS after 20GHz)
//      -erp Tx Effective Radiated Power (Watts) including Tx+Rx gain
//      -rxh Rx Height(s) (optional. Default=0.1)
//      -rxg Rx gain dBi (optional for text report)
//      -hp Horizontal Polarisation (default=vertical)
//      -gc Random ground clutter (feet/meters)
//      -m Metric units of measurement
//      -te Terrain code 1-6 (optional)
//      -terdic Terrain dielectric value 2-80 (optional)
//      -tercon Terrain conductivity 0.01-0.0001 (optional)
//      -cl Climate code 1-6 (optional)
//      -rel Reliability for ITM model 50 to 99 (optional)
//      -resample Resample Lidar input to specified resolution in meters (optional) 
