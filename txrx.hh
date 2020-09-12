#include "coordinate.hh"
#include <vector>

class Pairtxrx: public Coord{
public:
    Pairtxrx(){};
    void assignPar(Coord tx, Coord rx);
    void setLoss(double loss);
    double getLoss();
    Coord& getTx();
    Coord& getRx();
private:
    std::vector<Coord> vp;
    double loss;
};