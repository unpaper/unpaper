// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdint.h>

#include "imageprocess/primitives.h"

uint64_t noisefilter(AVFrame *image, uint64_t intensity,
                     uint8_t min_white_level, uint8_t abs_black_threshold);

typedef struct {
  struct {
    uint32_t horizontal;
    uint32_t vertical;
  } scan_size;

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

uint64_t grayfilter(AVFrame *image, GrayfilterParameters params,
                    uint8_t abs_black_threshold);
