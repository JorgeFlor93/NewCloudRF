
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "models/fspl.hh"
#include "models/hata.hh"
#include "auxFuncts.hh"
#include "inputs.hh"
#include "outputs.hh"
#include "common.h"
#include "distance.hh"
#include "coordinate.hh"
#include "txrx.hh"
#include <iostream>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <vector>

/*
* Frequency: Any MHz
* Distance: Any km
* Heights in meters
*/

void readCoord(double& lat, double& lon, float& alt);
void displayCoord(Coord& coord);
void displayPar(Pairtxrx& vc);
void load_resvar();
void getTopoData(double minlon, double maxlon, double minlat, double maxlat);
void displayPar(std::vector<Pairtxrx> vpairs, std::vector<Coord> vtx);

int main(int argc, char *argv[]){

    /* Tx_h y Rx_h, freq, distance */
    float f = 5000, height_Mobile = 2; 
    /* SDF atributtes */  
    int result, pmenv = 3; // OPEN
    
    /*Variables del programa*/
    int modeChoosen = 0;
    int model = 0;
    int num_antennas = 0;
    double inlat;
    double inlon;
    float inalt;
    char next;

    std::vector<Eigen::Matrix<double, 1, 2>> point_list; //vector almacen puntos area

    // /*Vector que cubre la zona límite a cargar para las alturas reales del terreno .sdf*/
    // std::vector<int> limits;

    /* Vector tipo struct site que almacena cada punto del área con su pérdida*/
    std::vector<struct site> vloss;

    struct site antennas[1];

    /* Inicializamos vaiables*/ 
    std::vector<Coord> vtx;
    Coord tx, p, rx, tl, br;
    Pairtxrx vtrx;
    std::vector<Pairtxrx> vpairs; // vector te pares tx-rx para el mode 1
    // readCoord(inlat, inlon, inalt);
    // tx2.assignCoord(inlat, inlon, inalt);
    // tx.assignCoord(51.849,-2.2299, 10);
    // vc.assignPar(tx, tx2);
    //displayCoord(vc.getRx());     
   
    getDefaults(); //Inicializa variables con sus valores por defecto
    do_allocs(); //Reserva memoria para las estructuras de common.h según el valor de resolución IPPD(en auxFuncts.cc)
    LR.frq_mhz = f; // Assign freq

    /* -lat 51.849 -lon -2.2299 
    * significado de signos:
    * latitud + -> NORTH
    * latitud - -> SOUTH
    * longitud + -> EAST
    * longitud - -> WEST
    */
    
    //antennas[0] = {51.349, -2.2299, height_Base}; 
    // antennas[1] = {51.449000, -2.239900, height_Mobile}; // top-left
    // antennas[2] = {51.446709, -2.235768, height_Mobile}; //bot-right

    bool run = true;
    while(run){
        std::cout << "Tx Number: " << "\n";
        std::cin >> num_antennas;
        for(int i = 0; i < num_antennas; i++){

            std::cout << "Tx Data: ["<< i + 1 << "]" << "\n";
            readCoord(inlat, inlon, inalt);
            p.assignCoord(inlat, inlon, inalt);
            vtx.push_back(p);
        }

        std::cout << "1. Vector Antennas to point Rx" << "\n";
        std::cout << "2. Area" << "\n";
        std::cout << "3. Line" << "\n";
        std::cout << "Choose mode: " << "\n";
        std::cin >> modeChoosen;

        std::cout << "1. fspl" << "\n";
        std::cout << "2. Hata" << "\n";
        std::cout << "Choose propagation model(1,2): ";
        std::cin >> model;

        if(modeChoosen == 1){
            std::cout << "Rx Data: "; std::cout << "\n";
            readCoord(inlat, inlon, inalt);
            rx.assignCoord(inlat, inlon, inalt);

            getTopoData(vtx.at(0).getLon(), rx.getLon(), vtx.at(0).getLat(), rx.getLat()); //min max lon, min max lat

            for(int i = 0; i < num_antennas; i++){
                vtrx.assignPar(vtx.at(i), rx);
                vtrx.setLoss(PathReport(vtx.at(i).getStruct(), rx.getStruct(), model, pmenv));
                vpairs.push_back(vtrx);
            }
            
            for(auto vectorit = vpairs.begin(); vectorit != vpairs.end(); ++vectorit){
                std::cout << "Tx[" << vectorit->getTx().getLat() << ", "<< vectorit->getTx().getdisLon() << 
                "], Rx[" << vectorit->getRx().getLat() << ", " << vectorit->getRx().getdisLon() << "], Loss: " << vectorit->getLoss() << std::endl;
            }
                                
        }
        else if(modeChoosen == 2){
            std::vector<Coord> varea; //vector con todas las coordenadas del área.

            std::cout << "Top-left area point: "; std::cout << "\n";
            readCoord(inlat, inlon, inalt);
            tl.assignCoord(inlat, inlon, inalt);

            std::cout << "Bot-right area point: "; std::cout << "\n";
            readCoord(inlat, inlon, inalt);
            br.assignCoord(inlat, inlon, inalt);

            getTopoData(vtx.at(0).getLon(), br.getLon(), vtx.at(0).getLat(), tl.getLat());  

            point_list = get_area(tl.getLat(), tl.getLon(), br.getLat(), br.getLon()); 
            for (int i = 0; i < point_list.size(); i++){   
                p.assignCoord(point_list[i](0,0), -1*point_list[i](0,1), tl.getAlt());  
                varea.push_back(p);         
            }  
            for(int i = 0; i < num_antennas; i++){ //Por cada antena se calculan las pérdidas al área
                for(int j = 0; j < varea.size(); j++){
                    vtrx.assignPar(vtx.at(i), varea.at(j));
                    vtrx.setLoss(PathReport(vtx.at(i).getStruct(), varea.at(j).getStruct(), model, pmenv));
                    vpairs.push_back(vtrx);
                }  
            }
            displayPar(vpairs, vtx);
            varea.clear();
        }
        vtx.clear();// delete and resize 0.
        vpairs.clear();
        std::cout << "Continue Pathloss? (y/n)" << std::endl;
        std::cin >> next;
        if(next == 'n'){
            run = false;
        }
        next = 'y';
        free_dem();
        free_elev();
        free_path();
        fflush(stderr);
    }
    return 0;
}

void displayPar(std::vector<Pairtxrx> vpairs, std::vector<Coord> vtx){
    int i = 0;
    for(auto vectoritx = vtx.begin(); vectoritx != vtx.end(); ++vectoritx){
        std::cout << "Tx" << i << "[" << vectoritx->getLat() << ", "<< vectoritx->getdisLon() << "]" << std::endl;
        for(auto vectorit = vpairs.begin(); vectorit != vpairs.end(); ++vectorit){
            std::cout << "[" << vectorit->getRx().getLat() << ", " << vectorit->getRx().getdisLon() << "], Loss: " << vectorit->getLoss() << std::endl;
        }
        ++i;
        std::cout << std::endl;
    }
}

void readCoord(double& lat, double& lon, float& alt){
    std::cout << "Latitude: "; std::cin >> lat;
    std::cout << "Longitude: "; std::cin >> lon;
    std::cout << "Height: "; std::cin >> alt;
}

void displayCoord(Coord& coord){
    double lat = coord.getLat();
    double lon = coord.getdisLon();
    float alt = coord.getAlt();
    std::cout << "{" << lat << ", "<< lon << ", "<< alt <<"}" << "\n"; 
}

void load_resvar(){
    /*load variables de resolución*/
    ppd = (double)ippd;
    yppd=ppd; 
    dpp = 1 / ppd;
 	mpi = ippd-1;
}
    
void getTopoData(double minlon, double maxlon, double minlat, double maxlat){
    std::vector<int> limits;
    int result;
    limits = LatLongMinMax(minlon, maxlon, minlat, maxlat);	
    result = LoadTopoData(limits[0], limits[1], limits[2], limits[3]); // Se cargan las estructuras dem según la info del archivo del terreno
    load_resvar();
    std::cout << "result: " << result << std::endl;
}
    /* CALCULAR PUNTOS AREA
    * Libreria Eigen para manejar matrices de vectores. 
    * Posteriormente se convierte a struct site para poder ser manipulados
    */

    

    // point_list = get_area(antennas[1].lat, antennas[1].lon, antennas[2].lat, antennas[2].lon); 
    
    //     /* LOS CONVIERTO A struct site
    //        y aplico el *= -1 para q pueda ser leidos los datos sdf
    //     */
    // struct site converted_point[point_list.size()];  
    // std::cout.precision(8);
    // for (int i = 0; i < point_list.size(); i++){   
    //     converted_point[i].lat = point_list[i](0,0);
    //     converted_point[i].lon = point_list[i](0,1); 
    //     converted_point[i].lon *= -1; //SIEMPRE
    //     if (converted_point[i].lon < 0.0)
    //         converted_point[i].lon *= 360.0; 
    //     converted_point[i].alt = height_Mobile;
    //     //std::cout << "point " << i+1 << ": [" << converted_point[i].lat << ", " << converted_point[i].lon << "]" << std::endl;   
    // }

    // /* LOAD SDF INFO */
    //     /*quito longitud negativa en tx*/
    // antennas[0].lon *= -1;
    // if (antennas[0].lon < 0.0)
    //     antennas[0].lon += 360.0; //Necesario para leer las longitudes ESTE.

        /* 
        Límites del cálculo, max and min lat and lon
        Comparo tx con las esquinas del área
        Realmente solo es útil cuando hay q cargar mas de un archivo sdf por ser una zona extensa o ser límite de dos zonas
        antennas[1] siempre tiene la latitud mas alta del área y antennas[2] la longitud mas alta, 
        funciona para un archivo sdf al mismo tiempo. REVISAR
        */
    
    // limits = LatLongMinMax(antennas[0].lon, antennas[2].lon, antennas[0].lat, antennas[1].lat);

	    /* Cargo las alturas del terreno*/
	
    // result = LoadTopoData(limits[0], limits[1], limits[2], limits[3]); // Se cargan las estructuras dem según la info del archivo del terreno
    
    // /*load variables de resolución*/
    // ppd = (double)ippd;
    // yppd=ppd; 
    // dpp = 1 / ppd;
 	// mpi = ippd-1;

    /* Calculate PATH LOSS */ 
    // for(int i = 0; i < point_list.size(); i++){
    //     converted_point[i].loss = PathReport(antennas[0], converted_point[i], 1, 1);
    //     vloss.push_back(converted_point[i]);
    // }
    
    // /*Acceder posición concreta*/
    // std::cout << vloss.at(10).loss << std::endl;

    /*Recorrer el vector*/
    // for(auto vectorit = vloss.begin(); vectorit != vloss.end(); ++vectorit){
    //     std::cout << "{"<<vectorit->lat <<", " << vectorit->lon << ", "<< vectorit->loss << "}"<< std::endl;
    // }

//  std::cout << "max_lon: " << max_lon << ", min_lon: " << min_lon << ", max_lat: " << max_lat << ", min_lat: " << min_lat << std::endl;

// muchas antenas a un receptor:
// for(int i = 0; i < 3; i++){

//         d = Distance(antennas[i], antennas[0]);

//         if(model == 1){
//             Ploss = FSPLpathLoss(f, d);
//         }
//         else if(model == 2){
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
