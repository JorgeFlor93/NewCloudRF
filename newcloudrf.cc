#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include "Prop-tools/prop-tools.hh"
#include <iostream>
/*
* Frequency: Any MHz
* Distance: Any km
*/

int main(int argc, char *argv[]){

    double Ploss = 0.0; //Ldb
    float f = 5000, d, height_Base = 100, height_Mobile = 2; //Tx_h y Rx_h, freq, distance
    int num_model = 0, mode = 2; //mode SUBURBAN
    double wide_area = 0.0, height_area = 0.0, diagonal = 0.0;

    /*Tx*/
    struct site antennas[3];  //Constructor Vector de antenas

    antennas[0] = {55.895436, -3.257968, height_Base};

    antennas[1] = {55.881092, -3.288290, height_Base};

    antennas[2] = {55.876001, -3.226494, height_Base};

    /*Rx*/
    struct site receptors[5];

    receptors[0] = {55.835449, -3.380117, height_Mobile};

    receptors[1] = {55.821254, -3.298381, height_Mobile}; //bot right area

    receptors[2] = {55.835449, -3.380117, height_Mobile}; // top left area

    /* AREA
    *  min-right: 55.821254, -3.298381
    *  max-left: 55.835449, -3.380117
    *  Area from antenna[0] to this min and max area.
    */

    // Wide and height area
    receptors[3] = {receptors[1].lat, receptors[2].lon, height_Mobile}; // bot left point
    receptors[4] = {receptors[2].lat, receptors[1].lon, height_Mobile}; //top right point
    height_area = Distance(receptors[2], receptors[3]);
    wide_area = Distance(receptors[2], receptors[4]);
    diagonal = Distance(receptors[2], receptors[1]);
    
    std::cout << "Diagonal: " << diagonal << "\n";
    std::cout << "Height: " << height_area << "\n";
    std::cout << "Ancho: " << wide_area << "\n";
    std::cout << "\n";

    std::cout << "1. fspl" << "\n";
    std::cout << "2. Hata" << "\n";
    std::cout << "Choose propagation model(1,2): ";
    std::cin >> num_model;
    

    for(int i = 0; i < 3; i++){

        d = Distance(antennas[i], receptors[0]);

        if(num_model == 1){
            Ploss = FSPLpathLoss(f, d);
        }
        else if(num_model == 2){
            Ploss = HATApathLoss(f, antennas[i].alt, receptors[0].alt, d, mode);
        }
        else{
            std::cout << "Wrong number input" << "\n";
            return 0;
        }
        std::cout << "Distance (km): " << d << "\n";
        std::cout << "Pathloss are: " << Ploss << "\n";
    }
    
    return 0;
}

/*
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