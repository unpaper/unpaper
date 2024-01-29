// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "constants.h"
#include "imageprocess/image.h"
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

size_t detect_masks(Image image, MaskDetectionParameters params,
                    const Point points[], size_t points_count,
                    Rectangle masks[]);

void center_mask(Image image, const Point center, const Rectangle area);

typedef struct {
  struct {
    bool left;
    bool top;
    bool right;
    bool bottom;
  } alignment;

  Delta margin;
} MaskAlignmentParameters;

MaskAlignmentParameters validate_mask_alignment_parameters(int border_align,
                                                           Delta margin);

void align_mask(Image image, const Rectangle inside_area,
                const Rectangle outside, MaskAlignmentParameters params);

void apply_masks(Image image, const Rectangle masks[], size_t masks_count,
                 Pixel color);

void apply_wipes(Image image, Rectangle wipes[], size_t wipes_count,
                 Pixel color);

typedef struct {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
} Border;

static const Border BORDER_NULL = {0, 0, 0, 0};

Rectangle border_to_mask(Image image, const Border border);
void apply_border(Image image, const Border border, Pixel color);

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

Border detect_border(Image image, BorderScanParameters params,
                     const Rectangle outside_mask);
