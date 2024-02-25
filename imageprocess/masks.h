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

  Delta scan_step;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_depth;

  Direction scan_direction;

  struct {
    float horizontal;
    float vertical;
  } scan_threshold;

  int32_t minimum_width;
  int32_t maximum_width;

  int32_t minimum_height;
  int32_t maximum_height;
} MaskDetectionParameters;

bool validate_mask_detection_parameters(
    MaskDetectionParameters *params, Direction scan_direction,
    RectangleSize scan_size, const int32_t scan_depth[DIRECTIONS_COUNT],
    Delta scan_step, const float scan_threshold[DIRECTIONS_COUNT],
    const int scan_mininum[DIMENSIONS_COUNT],
    const int scan_maximum[DIMENSIONS_COUNT]);

size_t detect_masks(Image image, MaskDetectionParameters params,
                    const Point points[], size_t points_count,
                    Rectangle masks[]);

void center_mask(Image image, const Point center, const Rectangle area);

typedef struct {
  Edges alignment;
  Delta margin;
} MaskAlignmentParameters;

bool validate_mask_alignment_parameters(MaskAlignmentParameters *params,
                                        Edges alignment, Delta margin);

void align_mask(Image image, const Rectangle inside_area,
                const Rectangle outside, MaskAlignmentParameters params);

void apply_masks(Image image, const Rectangle masks[], size_t masks_count,
                 Pixel color);

#define MAX_WIPES MAX_MASKS

typedef struct {
  size_t count;
  Rectangle areas[MAX_WIPES];
} Wipes;

void apply_wipes(Image image, Wipes wipes, Pixel color);

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
  Delta scan_step;

  struct {
    int32_t horizontal;
    int32_t vertical;
  } scan_threshold;

  Direction scan_direction;
} BorderScanParameters;

bool validate_border_scan_parameters(
    BorderScanParameters *params, Direction scan_direction,
    RectangleSize scan_size, Delta scan_step,
    const int32_t scan_threshold[DIRECTIONS_COUNT]);

Border detect_border(Image image, BorderScanParameters params,
                     const Rectangle outside_mask);
