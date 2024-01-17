// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <libavutil/frame.h>

typedef struct {
  int32_t x;
  int32_t y;
} Point;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Pixel;

#define PIXEL_WHITE                                                            \
  (Pixel) { 0xFF, 0xFF, 0xFF }

Pixel get_pixel(AVFrame *image, Point coords);
uint8_t get_pixel_grayscale(AVFrame *image, Point coords);
uint8_t get_pixel_lightness(AVFrame *image, Point coords);
uint8_t get_pixel_darkness_inverse(AVFrame *image, Point coords);
bool set_pixel(AVFrame *image, Point coords, Pixel pixel,
               uint8_t abs_black_threshold);
