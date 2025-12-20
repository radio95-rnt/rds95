#include "common.h"
#include "rds.h"
#include "modulator.h"
#include "lib.h"

void encoder_saveToFile(RDSEncoder *emp);
void encoder_loadFromFile(RDSEncoder *emp);
int encoder_saved();

void Modulator_saveToFile(RDSModulatorParameters *emp);
void Modulator_loadFromFile(RDSModulatorParameters *emp);
int modulatorsaved();