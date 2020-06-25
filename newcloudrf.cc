#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include "main.hh"
#include "inputs.hh"
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
    double wide_area = 0.0, height_area = 0.0;
    int error_if_1 = 0;

    
    std::vector<std::string> type;

    std::vector<Eigen::Matrix<double, 1, 2>> point_list;

    std::cout << "1. fspl" << "\n";
    std::cout << "2. Hata" << "\n";
    std::cout << "Choose propagation model(1,2): ";
    std::cin >> num_model;
    struct site antennas[4];
    /*Tx*/  
    antennas[0] = {55.899785, -3.269813, height_Base}; 
    /*Rx*/
    antennas[1] = {55.895017, -3.255537, height_Mobile}; //point to point receptor
    antennas[2] = {55.903883, -3.268566, height_Mobile}; //bot right area point
    antennas[3] = {55.905278, -3.274574, height_Mobile}; // top left area point 

    /* AREA & LINE*/

    type.push_back(argv[1]);

    if(type[0] == "line")
        point_list = get_line(antennas[3].lat, antennas[3].lon, antennas[2].lat, antennas[2].lon); //distance.cc

    else if(type[0] == "area")
        point_list = get_area(antennas[3].lat, antennas[3].lon, antennas[2].lat, antennas[2].lon); 

    else    
        std::cout << "Error input" << std::endl;
    
    /* Convierto los puntos del área a struct site */
    struct site converted_point[point_list.size()];  
    for (int i = 0; i <= point_list.size(); i++){   
        converted_point[i].lat = point_list[i](0,0);
        converted_point[i].lon = point_list[i](0,1);     
    }

    for (int i = 0; i < point_list.size(); i++){
        d = Distance(antennas[0], converted_point[i]);
        std::cout << "point: lat-> " << converted_point[i].lat << ", lon-> " << converted_point[i].lon <<"\n";
        if(num_model == 1){
            Ploss = FSPLpathLoss(f, d);
        }
        else if(num_model == 2){
            Ploss = HATApathLoss(f, antennas[0].alt, height_Mobile, d, mode);
        }
        else{
            std::cout << "Wrong number input" << "\n";
            return 0;
        }
        //std::cout << "Distance (km): " << d << "\n";
        std::cout << "Pathloss are: " << Ploss << "\n";
    }
    
    return 0;
}







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
// 
