#include <iostream>
#include "common.h"
#include "coordinate.hh"
#include "auxFuncts.hh"

Coord::Coord(double lat, double lon, float alt) {
    this->point.lat = lat;
    this->point.dislon = lon;
    lon *= -1;
    if (lon < 0.0)
        lon += 360.0; //Necesario para leer las longitudes ESTE y cargar su sdf.
    this->point.lon = lon;
    this->point.alt = alt;
}

struct site Coord::getStruct(){
    return this->point;
}

std::vector<double> Coord::getPos(){
    std::vector<double> Pos;
    Pos.push_back(this->point.lat);
    Pos.push_back(this->point.dislon);
}

double Coord::getLat(){
    return this->point.lat;    
}

double Coord::getdisLon(){
    return this->point.dislon;
}

double Coord::getLon(){
    return this->point.lon;
}

float Coord::getAlt(){
    return this->point.alt;
}
float Coord::getTxalt(){
    return this->point.txalt;
}

float Coord::setAlt(){  
    return this->point.alt = this->point.txalt + float(GetElevation(this->point)); // altura q introduzco + altura elevacion del terreno
}

bool Coord::assignCoord(double lat, double lon, float txalt){
    if(!validCoord(lat, lon, txalt)){
        std::cout << "Incorrect coordinate" << "\n";
        return false;
    }
    this->point.lat = lat;
    this->point.dislon = lon;
    this->point.txalt = txalt;
    lon *= -1;
    if (lon < 0.0)
        lon += 360.0; //Necesario para leer las longitudes ESTE y cargar su sdf.
    this->point.lon = lon;
    return true;
}

bool Coord::validCoord(double lat, double lon, float alt){
    bool correctlat, correctlon, correctalt;
    correctlat = (lat > -70) && (lat < 70);
    correctlon = (lon > -180) && (lon < 180);
    correctalt = (alt >= 0);
    return correctalt && correctlon && correctalt;
}

bool Coord::assignTx(double minloss, struct site tx){
    bool correctloss;
    this->loss = minloss;
    this->btx = tx;
    correctloss = (minloss > 0);
    return correctloss;  
}

double Coord::getBtxloss(){
    return this->loss;
}

struct site Coord::getBtx(){
    return this->btx;
}