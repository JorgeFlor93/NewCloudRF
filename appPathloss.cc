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
#include <vector>

/*
* Frequency: Any MHz
* Distance: Any km
*/

int main(int argc, char *argv[]){

    /* Tx_h y Rx_h, freq, distance */
    float f = 5000, d, height_Base = 100, height_Mobile = 2; 
    int num_model = 0, mode = 2/*mode SUBURBAN*/;
    int x; int y;
    double loss = 0.0;
    /* SDF atributtes */  
    int result;
    
    std::vector<int> limits;

    std::vector<struct site> vloss;

    struct site antennas[3];

    /* Inicializamos vaiables*/ 
    MAXRAD = 200; 
    antennas[0].lat = 91.0;
	antennas[0].lon = 361.0;
	antennas[1].lat = 91.0;
	antennas[1].lon = 361.0;
    antennas[2].lat = 91.0;
	antennas[2].lon = 361.0;
    
    getMain();
    do_allocs();
    LR.frq_mhz = f;

    /* -lat 51.849 -lon -2.2299 
     negativos:
           latitud + -> NORTH
           latitud - -> SOUTH
           longitud + -> EAST
           longitud - -> WEST
    */
    /*Tx*/  
    antennas[0] = {51.349, -2.2299, height_Base}; 

    /*Rx AREA*/
    antennas[1] = {51.449000, -2.239900, height_Mobile}; // top-left
    antennas[2] = {51.446709, -2.235768, height_Mobile}; //bot-right

    /* CALCULAR PUNTOS AREA*/

    std::vector<Eigen::Matrix<double, 1, 2>> point_list;

    point_list = get_area(antennas[1].lat, antennas[1].lon, antennas[2].lat, antennas[2].lon); 
    
        /* LOS CONVIERTO A struct site
           y aplico el *= -1 para q pueda ser leidos los datos sdf
        */
    struct site converted_point[point_list.size()];  
    std::cout.precision(8);
    for (int i = 0; i < point_list.size(); i++){   
        converted_point[i].lat = point_list[i](0,0);
        converted_point[i].lon = point_list[i](0,1); 
        converted_point[i].lon *= -1; //SIEMPRE
        if (converted_point[i].lon < 0.0)
            converted_point[i].lon *= 360.0; 
        converted_point[i].alt = height_Mobile;
        //std::cout << "point " << i+1 << ": [" << converted_point[i].lat << ", " << converted_point[i].lon << "]" << std::endl;   
    }

    /* LOAD SDF INFO */
        /*quito longitud negativa en tx*/
    antennas[0].lon *= -1;
    if (antennas[0].lon < 0.0)
        antennas[0].lon += 360.0; 

        /* 
        Límites del cálculo, max and min lat and lon
        Comparo tx con las esquinas del área
        Realmente solo es útil cuando hay q cargar mas de un archivo sdf por ser una zona extensa o ser límite de dos zonas
        antennas[1] siempre tiene la latitud mas alta del área y antennas[2] la longitud mas alta, 
        revisar si es necesario porq está incompleto
        */
    
    limits = LatLongMinMax(antennas[0].lon, antennas[2].lon, antennas[0].lat, antennas[1].lat);

	    /* Cargo las alturas del terreno*/
	
    result = LoadTopoData(limits[0], limits[1], limits[2], limits[3]);
   
    /*load variables de resolución*/
    ppd = (double)ippd;
    yppd=ppd; 
    dpp = 1 / ppd;
 	mpi = ippd-1;

    strncpy(antennas[0].filename, "gnuplot", 253); /* fichero con info de propagación recogida en pathreport (Deshabilitado)*/
    
    /* Calculate PATH LOSS*/ 
    for(int i = 0; i < point_list.size(); i++){
        converted_point[i].loss = PathReport(antennas[0], converted_point[i], antennas[0].filename, 1, 3, 0);
        //std::cout << converted_point[i].loss << std::endl;
        vloss.push_back(converted_point[i]);
    }
    

    for(auto vectorit = vloss.begin(); vectorit != vloss.end(); ++vectorit){
        std::cout << "{"<<vectorit->lat <<", " << vectorit->lon << ", "<< vectorit->loss << "}"<< std::endl;
    }
   
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
