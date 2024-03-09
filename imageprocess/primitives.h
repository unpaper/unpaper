// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2023 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>
#include <stddef.h>
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
  bool horizontal;
  bool vertical;
} Direction;

#define DIRECTION_NONE                                                         \
  (Direction) { .horizontal = false, .vertical = false }
#define DIRECTION_HORIZONTAL                                                   \
  (Direction) { .horizontal = true, .vertical = false }
#define DIRECTION_VERTICAL                                                     \
  (Direction) { .horizontal = false, .vertical = true }
#define DIRECTION_BOTH                                                         \
  (Direction) { .horizontal = true, .vertical = true }

typedef struct {
  bool left;
  bool top;
  bool right;
  bool bottom;
} Edges;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} Pixel;

#define PIXEL_WHITE                                                            \
  (Pixel) { UINT8_MAX, UINT8_MAX, UINT8_MAX }
#define PIXEL_BLACK                                                            \
  (Pixel) { 0, 0, 0 }

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

int compare_sizes(RectangleSize a, RectangleSize b);
RectangleSize coerce_size(RectangleSize size, RectangleSize default_size);

uint64_t count_pixels(Rectangle area);

bool point_in_rectangle(Point p, Rectangle input_area);
bool point_in_rectangles_any(Point p, size_t count,
                             const Rectangle rectangles[]);
bool rectangle_in_rectangle(Rectangle inner, Rectangle outer);
bool rectangles_overlap(Rectangle first_input, Rectangle second_input);
bool rectangle_overlap_any(Rectangle first_input, size_t count,
                           Rectangle *rectangles);

typedef struct {
  float x;
  float y;
} FloatPoint;

FloatPoint center_of_rectangle(Rectangle area);
