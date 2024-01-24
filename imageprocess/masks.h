// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "imageprocess/primitives.h"

typedef struct {
  RectangleSize scan_size;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_step;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_depth;

  bool scan_horizontal;
  bool scan_vertical;

  struct {
    float horizontal;
    float vertical;
  } scan_threshold;

  int32_t minimum_width;
  int32_t maximum_width;

  int32_t minimum_height;
  int32_t maximum_height;
} MaskDetectionParameters;

MaskDetectionParameters
validate_mask_detection_parameters(int scan_directions,
                                   const int scan_size[DIRECTIONS_COUNT],
                                   const int scan_depth[DIRECTIONS_COUNT],
                                   const int scan_step[DIRECTIONS_COUNT],
                                   const float scan_threshold[DIRECTIONS_COUNT],
                                   const int scan_mininum[DIMENSIONS_COUNT],
                                   const int scan_maximum[DIMENSIONS_COUNT]);

size_t detect_masks(AVFrame *image, MaskDetectionParameters params,
                    const Point points[], size_t points_count,
                    bool mask_valid[], Rectangle masks[]);

void center_mask(AVFrame *image, const Point center, const Rectangle area,
                 Pixel sheet_background, uint8_t abs_black_threshold);

typedef struct {
  struct {
    bool left;
    bool top;
    bool right;
    bool bottom;
  } alignment;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } margin;
} MaskAlignmentParameters;

MaskAlignmentParameters
validate_mask_alignment_parameters(int border_align,
                                   const int margin[DIRECTIONS_COUNT]);

void align_mask(AVFrame *image, const Rectangle inside_area,
                const Rectangle outside, MaskAlignmentParameters params,
                Pixel sheet_background, uint8_t abs_black_threshold);

void apply_masks(AVFrame *image, const Rectangle masks[], size_t masks_count,
                 Pixel color, uint8_t abs_black_threshold);

void apply_wipes(AVFrame *image, Rectangle wipes[], size_t wipes_count,
                 Pixel color, uint8_t abs_black_threshold);

typedef struct {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
} Border;

static const Border BORDER_NULL = {0, 0, 0, 0};

Rectangle border_to_mask(AVFrame *image, const Border border);
void apply_border(AVFrame *image, const Border border, Pixel color,
                  uint8_t abs_black_threshold);

typedef struct {
  RectangleSize scan_size;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_step;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_threshold;

  bool scan_horizontal;
  bool scan_vertical;
} BorderScanParameters;

BorderScanParameters
validate_border_scan_parameters(int scan_directions,
                                const int scan_size[DIRECTIONS_COUNT],
                                const int scan_step[DIRECTIONS_COUNT],
                                const int scan_threshold[DIRECTIONS_COUNT]);

Border detect_border(AVFrame *image, BorderScanParameters params,
                     const Rectangle outside_mask, uint8_t abs_black_threshold);
