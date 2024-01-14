// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2023 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int32_t x;
  int32_t y;
} Point;

static const Point POINT_ORIGIN = {0, 0};
static const Point POINT_INFINITY = {INT32_MAX, INT32_MAX};

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Pixel;

static const Pixel PIXEL_WHITE = {0xFF, 0xFF, 0xFF};

typedef struct {
  Point vertex[2];
} Rectangle;

static const Rectangle RECT_FULL_IMAGE = {{POINT_ORIGIN, POINT_INFINITY}};

typedef struct {
  uint32_t width;
  uint32_t height;
} RectangleSize;

#define scan_rectangle(area)                                                   \
  for (int32_t y = area.vertex[0].y; y <= area.vertex[1].y; y++)               \
    for (int32_t x = area.vertex[0].x; x <= area.vertex[1].x; x++)

Rectangle rectangle_from_size(Point origin, RectangleSize size);
RectangleSize size_of_rectangle(Rectangle rect);
Rectangle normalize_rectangle(Rectangle input);
Rectangle clip_rectangle(AVFrame *image, Rectangle area);
uint64_t count_pixels(Rectangle area);
bool point_in_rectangle(Point p, Rectangle input_area);
bool rectangles_overlap(Rectangle first_input, Rectangle second_input);
