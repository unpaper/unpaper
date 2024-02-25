// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdint.h>

#include "imageprocess/image.h"
#include "imageprocess/primitives.h"

typedef struct {
  RectangleSize scan_size;

  Delta scan_step;

  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_depth;

  Direction scan_direction;

  uint8_t abs_threshold;
  int32_t intensity;

  size_t exclusions_count;
  Rectangle *exclusions;
} BlackfilterParameters;

void blackfilter(Image image, BlackfilterParameters params);

bool validate_blackfilter_parameters(BlackfilterParameters *params,
                                     RectangleSize scan_size, Delta scan_step,
                                     uint32_t scan_depth_h,
                                     uint32_t scan_depth_v,
                                     Direction scan_direction, float threshold,
                                     int32_t intensity, size_t exclusions_count,
                                     Rectangle *exclusions);

typedef struct {
  RectangleSize scan_size;

  Delta scan_step;

  float intensity;
} BlurfilterParameters;

bool validate_blurfilter_parameters(BlurfilterParameters *params,
                                    RectangleSize scan_size, Delta scan_step,
                                    float intensity);

uint64_t blurfilter(Image image, BlurfilterParameters params,
                    uint8_t abs_white_threshold);

uint64_t noisefilter(Image image, uint64_t intensity, uint8_t min_white_level);

typedef struct {
  RectangleSize scan_size;

  Delta scan_step;

  uint8_t abs_threshold;
} GrayfilterParameters;

bool validate_grayfilter_parameters(GrayfilterParameters *params,
                                    RectangleSize scan_size, Delta scan_step,
                                    float threshold);

uint64_t grayfilter(Image image, GrayfilterParameters params);
