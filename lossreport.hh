
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "common.h"
#include "auxFuncts.hh"
#include "inputs.hh"
#include "auxFuncts.hh"
#include "coordinate.hh"
#include "models/cost.hh"
#include "models/ecc33.hh"
#include "models/ericsson.hh"
#include "models/fspl.hh"
#include "models/hata.hh"
#include "models/itwom3.0.hh"
#include "models/sui.hh"

double LossReport(struct site source, struct site destination, int propmodel, int pmenv);