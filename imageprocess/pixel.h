// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <libavutil/frame.h>

#include "imageprocess/primitives.h"

Pixel pixel_from_value(uint32_t value);
int compare_pixel(Pixel a, Pixel b);
Pixel get_pixel(AVFrame *image, Point coords);
uint8_t get_pixel_grayscale(AVFrame *image, Point coords);
uint8_t get_pixel_lightness(AVFrame *image, Point coords);
uint8_t get_pixel_darkness_inverse(AVFrame *image, Point coords);
bool set_pixel(AVFrame *image, Point coords, Pixel pixel,
               uint8_t abs_black_threshold);
