#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include <iostream>
/*
* Frequency: Any MHz
* Distance: Any km
*/

int main(int argc, char *argv[]){

    double lat_tx = 55.895436, lon_tx = -3.257968, Ploss = 0.0; //tx site
    double lat_rx = 55.828814, lon_rx = -3.332724; // rx site
    float f = 5000, d = 10, erp = 20, h_B = 100, h_M = 2;
    std::string model = "";
    int num_model = 0, mode = 1;

    std::cout << "1. fspl" << "\n";
    std::cout << "2. Hata" << "\n";
    std::cout << "Choose propagation model(1,2): ";
    
    std::cin >> num_model;

    if(num_model == 1){
        Ploss = FSPLpathLoss(f, d);
    }
    else if(num_model == 2){
        Ploss = HATApathLoss(f, h_B, h_M, d, mode);
    }
    else{
        std::cout << "Wrong number input" << "\n";
        return 0;
    }
    std::cout << "Pathloss are: " << Ploss << "\n";
    
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