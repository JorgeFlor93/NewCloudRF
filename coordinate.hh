#pragma once
#include "common.h"
#include <vector>

class Coord{
    public:
    Coord(){}; // Constructor sin parámetros
    Coord(double lat, double lon, float alt); //Constructor
    bool assignCoord(double lat, double lon, float txalt);
    bool validCoord(double lat, double lon, float alt);
    std::vector<double> getPos();
    double getLat();
    double getdisLon();
    double getLon();
    float setAlt(); 
    float getAlt();
    float getTxalt();
    struct site getStruct();
    bool assignTx(double minloss, struct site tx);
    double getBtxloss();
    struct site getBtx();
    private:
    struct site point; // lat, lon, alt , loss , dislon del punto
    double loss;
    struct site btx;
};