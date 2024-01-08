#pragma once

#include "constants.h"
#include "parse.h"

typedef struct {
  LAYOUTS layout;
  int startSheet;
  int endSheet;

  struct MultiIndex sheetMultiIndex;
  struct MultiIndex excludeMultiIndex;
  struct MultiIndex ignoreMultiIndex;
  struct MultiIndex insertBlank;
  struct MultiIndex replaceBlank;

  // 0: allow all, -1: disable all, n: individual entries
  struct MultiIndex noBlackfilterMultiIndex;
  struct MultiIndex noNoisefilterMultiIndex;
  struct MultiIndex noBlurfilterMultiIndex;
  struct MultiIndex noGrayfilterMultiIndex;
  struct MultiIndex noMaskScanMultiIndex;
  struct MultiIndex noMaskCenterMultiIndex;
  struct MultiIndex noDeskewMultiIndex;
  struct MultiIndex noWipeMultiIndex;
  struct MultiIndex noBorderMultiIndex;
  struct MultiIndex noBorderScanMultiIndex;
  struct MultiIndex noBorderAlignMultiIndex;
} Options;

void optionsInit(Options *o);
