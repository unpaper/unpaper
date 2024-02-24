// SPDX-FileCopyrightText: 2024 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "imageprocess/primitives.h"
#include "parse.h"

// The following two structures contain temporary values provided as parameters
// from the user, received (possibly) as physical dimensions in centimetres or
// inches, and converted to thousands of an inch (mils) for ease of processing
// without using floating points.
//
// This uses imperial measurements for ease of conversion of PPI
// (Pixels-per-Inch) values as that is de-facto the industry standard.
//
// The explicit ppi value attached to these allows setting it to `1` if the user
// provided the dimension in pixels, rather than with physical units.

typedef struct {
  int32_t width;
  int32_t height;
  bool physical;
} MilsSize;

typedef struct {
  int32_t horizontal;
  int32_t vertical;
  bool physical;
} MilsDelta;

RectangleSize mils_size_to_pixels(MilsSize size, int16_t ppi);
Delta mils_delta_to_pixels(MilsDelta delta, int16_t ppi);

bool parse_physical_size(const char *str, MilsSize *size);
bool parse_physical_delta(const char *str, MilsDelta *delta);
