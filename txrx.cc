
#include <iostream>
#include "txrx.hh"
#include "coordinate.hh"

void Pairtxrx::assignPar(Coord tx, Coord rx){
    this->vp.push_back(tx);
    this->vp.push_back(rx);
}

Coord& Pairtxrx::getTx(){
    return this->vp.front();
}

Coord& Pairtxrx::getRx(){
    return this->vp.back();
}

void Pairtxrx::setLoss(double loss){
    this->loss = loss;
}

double Pairtxrx::getLoss(){
    return this->loss;
}