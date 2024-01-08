#pragma once

#include "constants.h"
#include "parse.h"

typedef struct {
  LAYOUTS layout;

  struct MultiIndex sheetMultiIndex;
  struct MultiIndex excludeMultiIndex;
} Options;

void optionsInit(Options *o);
