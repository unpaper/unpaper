// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "pixel.h"

typedef struct {
  Point vertex[2];
} Rectangle;

int wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                   uint8_t abs_black_threshold);
