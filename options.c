// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <string.h>

#include "options.h"

static struct MultiIndex multiIndexEmpty(void) {
  return (struct MultiIndex){.count = 0, .indexes = NULL};
}

void optionsInit(Options *o) {
  memset(o, 0, sizeof(Options));

  o->layout = LAYOUT_SINGLE;

  // default: process all between start-sheet and end-sheet
  // This does not use .count = 0 because we use the -1 as a sentinel for "all
  // sheets".
  o->sheetMultiIndex = (struct MultiIndex){.count = -1, .indexes = NULL};

  o->excludeMultiIndex = multiIndexEmpty();
  o->ignoreMultiIndex = multiIndexEmpty();
  o->insertBlank = multiIndexEmpty();
  o->replaceBlank = multiIndexEmpty();

  o->noBlackfilterMultiIndex = multiIndexEmpty();
  o->noNoisefilterMultiIndex = multiIndexEmpty();
  o->noBlurfilterMultiIndex = multiIndexEmpty();
  o->noGrayfilterMultiIndex = multiIndexEmpty();
  o->noMaskScanMultiIndex = multiIndexEmpty();
  o->noMaskCenterMultiIndex = multiIndexEmpty();
  o->noDeskewMultiIndex = multiIndexEmpty();
  o->noWipeMultiIndex = multiIndexEmpty();
  o->noBorderMultiIndex = multiIndexEmpty();
  o->noBorderScanMultiIndex = multiIndexEmpty();
  o->noBorderAlignMultiIndex = multiIndexEmpty();
}
