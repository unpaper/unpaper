// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdint.h>

#include "imageprocess/image.h"
#include "imageprocess/primitives.h"

typedef struct {
  RectangleSize scan_size;

  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_step;

  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_depth;

  bool scan_horizontal;
  bool scan_vertical;

  uint8_t abs_threshold;
  int32_t intensity;

  size_t exclusions_count;
  Rectangle *exclusions;
} BlackfilterParameters;

void blackfilter(Image image, BlackfilterParameters params,
                 uint8_t abs_black_threshold);

BlackfilterParameters validate_blackfilter_parameters(
    uint32_t scan_size_h, uint32_t scan_size_v, uint32_t scan_step_h,
    uint32_t scan_step_v, uint32_t scan_depth_h, uint32_t scan_depth_v,
    int8_t scan_directions, float threshold, int32_t intensity,
    size_t exclusions_count, Rectangle *exclusions);

typedef struct {
  RectangleSize scan_size;

  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_step;

  float intensity;
} BlurfilterParameters;

BlurfilterParameters validate_blurfilter_parameters(uint32_t scan_size_h,
                                                    uint32_t scan_size_v,
                                                    uint32_t scan_step_h,
                                                    uint32_t scan_step_v,
                                                    float intensity);

uint64_t blurfilter(Image image, BlurfilterParameters params,
                    uint8_t abs_white_threshold, uint8_t abs_black_threshold);

uint64_t noisefilter(Image image, uint64_t intensity, uint8_t min_white_level,
                     uint8_t abs_black_threshold);

typedef struct {
  RectangleSize scan_size;

  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_step;

  uint8_t abs_threshold;
} GrayfilterParameters;

GrayfilterParameters validate_grayfilter_parameters(uint32_t scan_size_h,
                                                    uint32_t scan_size_v,
                                                    uint32_t scan_step_h,
                                                    uint32_t scan_step_v,
                                                    float threshold);

uint64_t grayfilter(Image image, GrayfilterParameters params,
                    uint8_t abs_black_threshold);
