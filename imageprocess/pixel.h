// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "imageprocess/image.h"
#include "imageprocess/primitives.h"

Pixel pixel_from_value(uint32_t value);
int compare_pixel(Pixel a, Pixel b);
Pixel get_pixel(Image image, Point coords);
uint8_t get_pixel_grayscale(Image image, Point coords);
uint8_t get_pixel_lightness(Image image, Point coords);
uint8_t get_pixel_darkness_inverse(Image image, Point coords);
bool set_pixel(Image image, Point coords, Pixel pixel,
               uint8_t abs_black_threshold);
