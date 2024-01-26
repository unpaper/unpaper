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

#define POINT_ORIGIN                                                           \
  (Point) { 0, 0 }
#define POINT_INFINITY                                                         \
  (Point) { INT32_MAX, INT32_MAX }

typedef struct {
  int32_t horizontal;
  int32_t vertical;
} Delta;

Delta distance_between(Point a, Point b);
Point shift_point(Point p, Delta d);

#define DELTA_UPWARD                                                           \
  (Delta) { 0, -1 }
#define DELTA_DOWNWARD                                                         \
  (Delta) { 0, 1 }
#define DELTA_LEFTWARD                                                         \
  (Delta) { -1, 0 }
#define DELTA_RIGHTWARD                                                        \
  (Delta) { 1, 0 }

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Pixel;

#define PIXEL_WHITE                                                            \
  (Pixel) { 0xFF, 0xFF, 0xFF }

typedef struct {
  Point vertex[2];
} Rectangle;

typedef struct {
  int32_t width;
  int32_t height;
} RectangleSize;

#define scan_rectangle(area)                                                   \
  for (int32_t y = area.vertex[0].y; y <= area.vertex[1].y; y++)               \
    for (int32_t x = area.vertex[0].x; x <= area.vertex[1].x; x++)

Rectangle rectangle_from_size(Point origin, RectangleSize size);
RectangleSize size_of_rectangle(Rectangle rect);
Rectangle normalize_rectangle(Rectangle input);
Rectangle shift_rectangle(Rectangle rect, Delta d);

RectangleSize size_of_image(AVFrame *image);
Rectangle full_image(AVFrame *image);
Rectangle clip_rectangle(AVFrame *image, Rectangle area);

uint64_t count_pixels(Rectangle area);

bool point_in_rectangle(Point p, Rectangle input_area);
bool point_in_rectangles_any(Point p, size_t count,
                             const Rectangle rectangles[]);
bool rectangle_in_rectangle(Rectangle inner, Rectangle outer);
bool rectangles_overlap(Rectangle first_input, Rectangle second_input);
bool rectangle_overlap_any(Rectangle first_input, size_t count,
                           Rectangle *rectangles);
