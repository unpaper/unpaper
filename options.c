// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <string.h>

#include "options.h"

void optionsInit(Options *o) {
  memset(o, 0, sizeof(Options));

  o->layout = LAYOUT_SINGLE;

  // default: process all between start-sheet and end-sheet
  o->sheetMultiIndex = (struct MultiIndex){.count = -1, .indexes = NULL};
}
