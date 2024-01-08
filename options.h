#pragma once

#include "constants.h"
#include "parse.h"

typedef struct {
  LAYOUTS layout;
  struct MultiIndex sheetMultiIndex;
} Options;

void optionsInit(Options *o);
