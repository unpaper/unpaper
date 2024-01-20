// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdint.h>

#include "primitives.h"

uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold);
void copy_rectangle(AVFrame *source, AVFrame *target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold);
uint8_t inverse_brightness_rect(AVFrame *image, Rectangle input_area);
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area);
uint8_t darkness_rect(AVFrame *image, Rectangle input_area);
uint64_t count_pixels_within_brightness(AVFrame *image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold);

typedef int8_t RotationDirection;
static const RotationDirection ROTATE_CLOCKWISE = 1;
static const RotationDirection ROTATE_ANTICLOCKWISE = -1;

// Rotates an image clockwise or anti-clockwise in 90-degrees.
void flip_rotate_90(AVFrame **pImage, RotationDirection direction,
                    uint8_t abs_black_threshold);

// Mirrors an image either horizontally, vertically, or both.
void mirror(AVFrame *image, bool horizontal, bool vertical,
            uint8_t abs_black_threshold);
