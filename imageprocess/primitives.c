// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2023 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/primitives.h"
#include "imageprocess/math_util.h"

Delta distance_between(Point a, Point b) {
  return (Delta){
      b.x - a.x,
      b.y - a.y,
  };
}

Point shift_point(Point p, Delta d) {
  return (Point){
      p.x + d.horizontal,
      p.y + d.vertical,
  };
}

Rectangle rectangle_from_size(Point origin, RectangleSize size) {
  return (Rectangle){
      .vertex =
          {
              origin,
              {
                  .x = origin.x + size.width - 1,
                  .y = origin.y + size.height - 1,
              },
          },
  };
}

RectangleSize size_of_rectangle(Rectangle rect) {
  return (RectangleSize){
      .width = abs(rect.vertex[0].x - rect.vertex[1].x) + 1,
      .height = abs(rect.vertex[0].y - rect.vertex[1].y) + 1,
  };
}

Rectangle normalize_rectangle(Rectangle input) {
  return (Rectangle){
      .vertex =
          {
              {
                  .x = min(input.vertex[0].x, input.vertex[1].x),
                  .y = min(input.vertex[0].y, input.vertex[1].y),
              },
              {
                  .x = max(input.vertex[0].x, input.vertex[1].x),
                  .y = max(input.vertex[0].y, input.vertex[1].y),
              },
          },
  };
}

Rectangle shift_rectangle(Rectangle rect, Delta d) {
  return (Rectangle){{
      shift_point(rect.vertex[0], d),
      shift_point(rect.vertex[1], d),
  }};
}

int compare_sizes(RectangleSize a, RectangleSize b) {
  if (a.height == b.height && a.width == b.width) {
    return 0;
  }

  if (min(a.height, a.width) < min(b.height, b.width)) {
    return -1;
  } else {
    return 1;
  }
}

uint64_t count_pixels(Rectangle area) {
  RectangleSize size = size_of_rectangle(area);

  return size.width * size.height;
}

bool point_in_rectangle(Point p, Rectangle input_area) {
  Rectangle area = normalize_rectangle(input_area);

  return (p.x >= area.vertex[0].x && p.x <= area.vertex[1].x &&
          p.y >= area.vertex[0].y && p.y <= area.vertex[1].y);
}

bool point_in_rectangles_any(Point p, size_t count,
                             const Rectangle rectangles[]) {
  for (size_t n = 0; n < count; n++) {
    if (point_in_rectangle(p, rectangles[n])) {
      return true;
    }
  }

  return false;
}

bool rectangle_in_rectangle(Rectangle inner, Rectangle outer) {
  return point_in_rectangle(inner.vertex[0], outer) &&
         point_in_rectangle(inner.vertex[1], outer);
}

bool rectangles_overlap(Rectangle first_input, Rectangle second_input) {
  Rectangle first = normalize_rectangle(first_input);
  Rectangle second = normalize_rectangle(second_input);

  return point_in_rectangle(first.vertex[0], second) ||
         point_in_rectangle(first.vertex[1], second);
}

bool rectangle_overlap_any(Rectangle first_input, size_t count,
                           Rectangle *rectangles) {
  for (size_t n = 0; n < count; n++) {
    if (rectangles_overlap(first_input, rectangles[n])) {
      return true;
    }
  }

  return false;
}
