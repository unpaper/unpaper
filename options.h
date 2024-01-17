// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "constants.h"
#include "parse.h"

typedef struct {
  LAYOUTS layout;

  struct MultiIndex sheetMultiIndex;
  struct MultiIndex excludeMultiIndex;
  struct MultiIndex ignoreMultiIndex;
  struct MultiIndex insertBlank;
  struct MultiIndex replaceBlank;
} Options;

void optionsInit(Options *o);
