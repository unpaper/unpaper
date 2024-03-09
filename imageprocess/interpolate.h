// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "imageprocess/image.h"
#include "imageprocess/primitives.h"

typedef enum {
  INTERP_NN,
  INTERP_LINEAR,
  INTERP_CUBIC,
  INTERP_FUNCTIONS_COUNT
} Interpolation;

Pixel interpolate(Image image, FloatPoint coords, Interpolation function);
