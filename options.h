#pragma once

#include "constants.h"

typedef struct {
  LAYOUTS layout;
} Options;

void optionsInit(Options *o);
