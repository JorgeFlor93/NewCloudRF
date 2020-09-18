#include "lossreport.hh"

double LossReport(struct site source, struct site destination, int propmodel, int pmenv){
    
    double dkm;

    dkm = Distance(source, destination);

    switch (propmodel) {	
        case 1:
            //HATA 
            loss =
                HATApathLoss(LR.frq_mhz, source.alt, destination.alt, dkm, pmenv);
        
        case 2:
            // ITU-R P.525 Free space path loss
            loss = FSPLpathLoss(LR.frq_mhz, dkm);
            break;
        }
	return loss;
}