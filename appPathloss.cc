
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
* Frequency:  MHz
* Distance:  km
* Heights in meters
*/

void readCoord(double& lat, double& lon, float& alt);
void readRxCoord(double& lat, double& lon); //no alt
void displayCoord(Coord& coord);
void ShowPar(Pairtxrx& pair);
void getTopoData(double minlon, double maxlon, double minlat, double maxlat);
void load_resvar();

int main(int argc, char *argv[]){


    float f = 100;  
    int pmenv; // pmenv = 1-urban 2-suburban 3-open(rural) 
    
    bool result; // result de pérdida > 0
    int modeChoosen = 0;
    int model = 0;
    int num_antennas = 0;
    double inlat;
    double inlon;
    float inalt;
    char next = 'n';
    float rx_height; // altura del area
    double min_dkm = 99999;
    double curr_dkm;
    double minloss = 99999;

    std::vector<Eigen::Matrix<double, 1, 2>> point_list; //vector almacen puntos area  
    std::vector<Coord> varea; //vector con todas las coordenadas del área.
    std::vector<Coord> vtx; //vector de antenas tx
    std::vector<Pairtxrx> vrx; // punto introducido
    std::vector<Pairtxrx> vpairs; // vector te pares tx-rx para el mode 1
    Pairtxrx vtrx;
    Coord p, tl, br;    
    Coord nearRx; //Punto introducido más cercano a un punto rojo.

    getDefaults(); //Inicializa variables con sus valores por defecto
    LR.frq_mhz = f; // Assign freq
    do_allocs(); //Reserva memoria para las estructuras de common.h según el valor de resolución IPPD(en auxFuncts.cc)
    

    /* significado de signos:
    * latitud + -> NORTH
    * latitud - -> SOUTH
    * longitud + -> EAST
    * longitud - -> WEST
    */
    // antennas[0] = {51.450488, -2.244628, height_Base}; // tx
    // antennas[1] = {51.449000, -2.239900, height_Mobile}; // top-left
    // antennas[2] = {51.446709, -2.235768, height_Mobile}; //bot-right
   
    int result_sdf;
    snprintf(string, 16,"%d:%d:%d:%d",51, 52, 2, 3);
    result_sdf = LoadSDF_SDF(string);
    if(result_sdf < 0){
        std::cout << "ERROR SDF FILE" << std::endl;
        return -1;
    }
    load_resvar(); 
    bool run = true;
    while(run){                
        std::cout << "1. Antennas Tx vector" << "\n";
        std::cout << "2. antennas vector to AREA" << "\n";
        std::cout << "3. antennas vector to POINT" << "\n";
        std::cout << "4. Clear AREA" << "\n";  
        std::cout << "Choose mode(1,2,3,4): ";
        std::cin >> modeChoosen;

        if(modeChoosen == 1){
            int a = 0; //Genero dos Tx para testeo
            std::cout << "Tx Number: " << "\n";
            std::cin >> num_antennas;
            for(int i = 0; i < num_antennas; i++){
                std::cout << "Tx Data: ["<< i + 1 << "]" << "\n";
                if(a == 1){
                    p.assignCoord(51.455488, -2.234628, 30);
                    p.setAlt();
                }
                else {
                    // readCoord(inlat, inlon, inalt);
                    p.assignCoord(51.450488, -2.244628, 30);
                    p.setAlt();
                }
                displayCoord(p);
                vtx.push_back(p);
                a++;
            }
        }
        else if(modeChoosen == 2){
            std::cout << "Height area: ";
            std::cin >> rx_height;
            std::cout << "Top-left area point: "; std::cout << "\n";
            // readRxCoord(inlat, inlon);
            tl.assignCoord(51.449000, -2.239900, rx_height);
            tl.setAlt();
            displayCoord(tl);
            std::cout << "Bot-right area point: "; std::cout << "\n";
            // readRxCoord(inlat, inlon);
            br.assignCoord(51.446709, -2.235768, rx_height);
            br.setAlt();
            displayCoord(br);

            point_list = get_area(tl.getLat(), tl.getLon(), br.getLat(), br.getLon()); // area generada por top-left y bot-right.

            for (int i = 0; i < point_list.size(); i++){ // Introduzco las coordenadas del area en un vector, con las alturas reales
                p.assignCoord(point_list[i](0,0), point_list[i](0,1), rx_height); 
                p.setAlt(); 
                varea.push_back(p);       
            }         
        }
        else if(modeChoosen == 3){
            if(varea.size() == 0){
                std::cout << "No area.." << "\n";
                break;
            }
            std::cout << "Rx Data: "; std::cout << "\n";
            // readCoord(inlat, inlon, inalt);
            p.assignCoord(51.446789,-2.256874,2);         
            displayCoord(p);
            std::cout << "1. Hata" << "\n";
            std::cout << "2. fspl" << "\n";
            std::cout << "Choose propagation model(1,2): ";
            std::cin >> model;

            std::cout << "1. URBAN" << "\n";
            std::cout << "2. SUBURBAN" << "\n";
            std::cout << "3. RURAL" << "\n";
            std::cout << "Choose environment(1,2,3): ";
            std::cin >> pmenv;
            std::cout << "\n";

            for(int j = 0; j < varea.size(); j++){
                curr_dkm = Distance(p.getStruct(), varea.at(j).getStruct());
                if(curr_dkm < min_dkm){
                    min_dkm = curr_dkm;
                    nearRx = varea.at(j);
                }
            }  
            for(auto it = vtx.begin(); it != vtx.end(); ++it){ //Por cada antena se calculan las pérdidas al área
                    //vtrx.assignPar(*it, nearRx);
                    // std::cout << it->getLat() << "\n";
                    vtrx.setTx(it->getStruct());
                    vtrx.setRx(nearRx.getStruct());
                    //ShowPar(vtrx);
                    //std::cout << vtrx.getTx().getLat() << std::endl;   
                    vtrx.setLoss(model, pmenv);
                    vpairs.push_back(vtrx); 
                    if(vtrx.getLoss() < minloss){
                        minloss = vtrx.getLoss();
                        result = nearRx.assignTx(minloss, it->getStruct());
                    }
            }    
            if(!result)
                std::cout << "Loss Error" << "\n";
            else{
                bool contin = true;
                char n2 = 'n';
                int option = 0;
                while(contin){
                    std::cout << "1. Show height point" << "\n";
                    std::cout << "2. Best antenna coord" << "\n";
                    std::cout << "3. Best prop" << "\n";
                    std::cout << "4. All antennas PathLoss" << "\n";
                    std::cout << "5. Exit" << "\n";
                    std::cout << "Choose Option(1,2,3,4,5): ";
                    std::cin >> option;
                    if(option == 1){
                        std::cout << "Total height Coord{" << nearRx.getLat() << ", " << nearRx.getdisLon() << "}: " << nearRx.getAlt() << "\n";
                    }
                    else if(option == 2){
                        std::cout << "Tx{" << nearRx.getBtx().lat << ", " << nearRx.getBtx().dislon << "}: " << "\n";
                    }
                    else if(option == 3){
                        std::cout << "Best Tx-Rx loss: "<< nearRx.getBtxloss() << "\n";
                    }
                    else if(option == 4){
                        for(auto vectoritx = vpairs.begin(); vectoritx != vpairs.end(); ++vectoritx){
                            std::cout << "Antenna: {" << vectoritx->getstx().lat << ", " << vectoritx->getstx().lon << ", " << vectoritx->getstx().alt << "}" << "\n"; 
                            std::cout << "\t" << "loss: " << vectoritx->getLoss() << "\n";
                        }
                    }           
                    else if(option == 5){
                        std::cout << "Exit.." << "\n";
                        contin = false;
                    }
                }
            } 
        }       
        else if(modeChoosen == 4){
            varea.clear();
            vpairs.clear();
        }
        std::cout << "Continue Pathloss? (y/n)" << std::endl;
        std::cin >> next;
        if(next == 'n')
            run = false; 
        else if(next == 'y'){
            run = true;
        }
        else{
            std::cout << "Input Error" << "\n";
            return -1;
        }
        fflush(stdin);
    }
    vtx.clear();// delete and resize 0.
    vpairs.clear();
    varea.clear();
    free_dem();
    free_elev();
    free_path();
    fflush(stderr);
    return 0;
}

void readCoord(double& lat, double& lon, float& alt){
    std::cout << "Latitude: "; std::cin >> lat;
    std::cout << "Longitude: "; std::cin >> lon;
    std::cout << "Height: "; std::cin >> alt;
}

void readRxCoord(double& lat, double& lon){
    std::cout << "Latitude: "; std::cin >> lat;
    std::cout << "Longitude: "; std::cin >> lon;
}

void ShowPar(Pairtxrx& pair){
    std::cout << "tx{" << pair.getstx().lat << ", " << pair.getstx().dislon << ", " << pair.getstx().alt << "}" << "\n";
    // std::cout << "tx{" << pair.getTx().getLat() << ", " << pair.getTx().getdisLon() << ", " << pair.getTx().getAlt() << "}, rx{" << 
    //                         pair.getRx().getLat() << ", " << pair.getRx().getdisLon() << ", " << pair.getRx().getAlt() << "\n";
}

void displayCoord(Coord& coord){
    double lat = coord.getLat();
    double lon = coord.getdisLon();
    float alt = coord.getAlt();
    std::cout << "{" << lat << ", "<< lon << ", "<< alt << "}" << "\n"; 
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
    limits = LatLongMinMax(minlon, maxlon, minlat, maxlat);	/*Vector que cubre la zona límite a cargar para las alturas reales del terreno .sdf*/
    result = LoadTopoData(limits[0], limits[1], limits[2], limits[3]); // Se cargan las estructuras dem según la info del archivo del terreno
    load_resvar();
    //std::cout << "result: " << result << std::endl;
}

