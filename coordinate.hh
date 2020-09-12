#pragma once
#include "common.h"
#include <vector>

class Coord{
    public:
    Coord(){}; // Constructor sin parámetros
    Coord(double lat, double lon, float alt); //Constructor
    bool assignCoord(double lat, double lon, float alt);
    bool validCoord(double lat, double lon, float alt);
    std::vector<double> getPos();
    double getLat();
    double getLon();
    float getAlt();
    struct site getStruct();
    private:
    struct site point; // lat, lon, alt , loss del punto
};