#pragma once
#include "common.h"
#include "rds.h"
#include "lib.h"

void encoder_saveToFile(RDSEncoder *emp);
int encoder_loadFromFile(RDSEncoder *emp);