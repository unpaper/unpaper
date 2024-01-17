// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "pixel.h"

typedef struct {
  Point vertex[2];
} Rectangle;

#define RECT_FULL_IMAGE (Rectangle){{POINT_ORIGIN, POINT_INFINITY}}

uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold);
void copy_rectangle(AVFrame *source, AVFrame *target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold);
uint8_t inverse_brightness_rect(AVFrame *image, Rectangle input_area);
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area);
uint8_t darkness_rect(AVFrame *image, Rectangle input_area);
