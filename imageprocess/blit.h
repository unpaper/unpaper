// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdint.h>

#include "imageprocess/image.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/primitives.h"

uint64_t wipe_rectangle(Image image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold);
void copy_rectangle(Image source, Image target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold);
uint8_t inverse_brightness_rect(Image image, Rectangle input_area);
uint8_t inverse_lightness_rect(Image image, Rectangle input_area);
uint8_t darkness_rect(Image image, Rectangle input_area);
uint64_t count_pixels_within_brightness(Image image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold);

typedef int8_t RotationDirection;
static const RotationDirection ROTATE_CLOCKWISE = 1;
static const RotationDirection ROTATE_ANTICLOCKWISE = -1;

void center_image(Image source, Image target, Point target_origin,
                  RectangleSize target_size, Pixel sheet_background,
                  uint8_t abs_black_threshold);

void stretch_and_replace(Image *pImage, RectangleSize size,
                         Interpolation interpolate_type,
                         uint8_t abs_black_threshold);

void resize_and_replace(Image *pImage, RectangleSize size,
                        Interpolation interpolate_type, Pixel sheet_background,
                        uint8_t abs_black_threshold);

// Rotates an image clockwise or anti-clockwise in 90-degrees.
void flip_rotate_90(Image *pImage, RotationDirection direction,
                    uint8_t abs_black_threshold);

// Mirrors an image either horizontally, vertically, or both.
void mirror(Image image, bool horizontal, bool vertical,
            uint8_t abs_black_threshold);

// Shifts the image.
void shift_image(Image *pImage, Delta d, Pixel sheet_background,
                 uint8_t abs_black_threshold);
