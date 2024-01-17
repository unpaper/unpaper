// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "imageprocess/primitives.h"

typedef struct {
  float x;
  float y;
} FloatPoint;

typedef enum {
  INTERP_NN,
  INTERP_LINEAR,
  INTERP_CUBIC,
  INTERP_FUNCTIONS_COUNT
} Interpolation;

Pixel interpolate(AVFrame *image, FloatPoint coords, Interpolation function);
