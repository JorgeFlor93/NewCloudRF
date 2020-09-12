#include <iostream>
#include "common.h"
#include "coordinate.hh"

Coord::Coord(double lat, double lon, float alt) {
    this->point.lat = lat;
    this->point.lon = lon;
    this->point.alt = alt;
}

struct site Coord::getStruct(){
    return this->point;
}

std::vector<double> Coord::getPos(){
    std::vector<double> Pos;
    Pos.push_back(this->point.lat);
    Pos.push_back(this->point.lon);
}

double Coord::getLat(){
    return this->point.lat;    
}

double Coord::getLon(){
    return this->point.lon;
}

float Coord::getAlt(){
    return this->point.alt;
}

bool Coord::assignCoord(double lat, double lon, float alt){
    if(!validCoord(lat, lon, alt)){
        std::cout << "Incorrect coordinate" << "\n";
        return false;
    }
    this->point.lat = lat;
    this->point.lon = lon;
    this->point.alt = alt;
    return true;
}

bool Coord::validCoord(double lat, double lon, float alt){
    bool correctlat, correctlon, correctalt;
    correctlat = (lat > -70) && (lat < 70);
    correctlon = (lon > -180) && (lon < 180);
    correctalt = (alt > 0);
    return correctalt && correctlon && correctalt;
}
