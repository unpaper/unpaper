// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "constants.h"

typedef struct {
  LAYOUTS layout;
} Options;

void optionsInit(Options *o);
