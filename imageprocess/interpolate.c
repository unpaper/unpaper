// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <math.h>
#include <stdint.h>

#include <libavutil/common.h>

#include "imageprocess/interpolate.h"
#include "imageprocess/pixel.h"

Pixel interp_nearest_neighbour(AVFrame *image, FloatPoint coords) {
  // Round to nearest location.
  Point p = {(int)roundf(coords.x), (int)roundf(coords.y)};

  return get_pixel(image, p);
}

/**
 * 1-D cubic interpolation. Clamps the return value between 0 and 255 to
 * support 8-bit colour images.
 */
static uint8_t cubic_scale(float factor, uint8_t a, uint8_t b, uint8_t c,
                           uint8_t d) {
  int result = b + 0.5f * factor *
                       (c - a +
                        factor * (2.0f * a - 5.0f * b + 4.0f * c - d +
                                  factor * (3.0f * (b - c) + d - a)));

  return av_clip_uint8(result);
}

// 1-D cubic interpolation
static Pixel cubic_pixel_interpolation(float factor, Pixel pxls[4]) {
  return (Pixel){
      .r = cubic_scale(factor, pxls[0].r, pxls[1].r, pxls[2].r, pxls[3].r),
      .g = cubic_scale(factor, pxls[0].g, pxls[1].g, pxls[2].g, pxls[3].g),
      .b = cubic_scale(factor, pxls[0].b, pxls[1].b, pxls[2].b, pxls[3].b),
  };
}

// 2-D bicubic interpolation
Pixel interp_bicubic(AVFrame *image, FloatPoint coords) {
  Point p = {(int)coords.x, (int)coords.y};

  Pixel pxls[4];

  for (int i = -1; i < 3; ++i) {
    Pixel quad[4] = {
        get_pixel(image, (Point){p.x - 1, p.y + i}),
        get_pixel(image, (Point){p.x, p.y + i}),
        get_pixel(image, (Point){p.x + 1, p.y + i}),
        get_pixel(image, (Point){p.x + 2, p.y + i}),
    };
    pxls[i + 1] = cubic_pixel_interpolation(coords.x - p.x, quad);
  }

  return cubic_pixel_interpolation(coords.y - p.y, pxls);
}

static uint8_t linear_scale(float x, uint8_t a, uint8_t b) {
  return (1.0f - x) * a + x * b;
}

// 1-D linear interpolation
static Pixel linear_pixel_interpolation(float factor, Pixel a, Pixel b) {
  return (Pixel){
      .r = linear_scale(factor, a.r, b.r),
      .g = linear_scale(factor, a.g, b.g),
      .b = linear_scale(factor, a.b, b.b),
  };
}

// 2-D linear interpolation
Pixel interp_bilinear(AVFrame *image, FloatPoint coords) {
  Rectangle image_area = clip_rectangle(image, RECT_FULL_IMAGE);

  Point p1 = {(int)floorf(coords.x), (int)floorf(coords.y)};
  Point p2 = {(int)ceil(coords.x), (int)ceilf(coords.y)};

  // Check edge conditions to avoid divide-by-zero
  if (!point_in_rectangle(p2, image_area)) {
    return get_pixel(image, p1);
  }

  // Single pixel.
  if (p1.x == p2.x && p1.y == p2.y) {
    return get_pixel(image, p1);
  }

  // 2D vertical interpolation.
  if (p1.x == p2.x) {
    Pixel pxl1 = get_pixel(image, p1);
    Pixel pxl2 = get_pixel(image, p2);

    return linear_pixel_interpolation(coords.x - p1.x, pxl1, pxl2);
  }

  // 2D horizontal interpolation.
  if (p1.y == p2.y) {
    Pixel pxl1 = get_pixel(image, p1);
    Pixel pxl2 = get_pixel(image, p2);

    return linear_pixel_interpolation(coords.y - p1.y, pxl1, pxl2);
  }

  // Get the four pixels in a square.
  Pixel pxl1 = get_pixel(image, (Point){p1.x, p1.y});
  Pixel pxl2 = get_pixel(image, (Point){p2.x, p1.y});
  Pixel pxl3 = get_pixel(image, (Point){p1.x, p2.y});
  Pixel pxl4 = get_pixel(image, (Point){p2.x, p2.y});

  Pixel pxl_h1 = linear_pixel_interpolation(coords.x - p1.x, pxl1, pxl2);
  Pixel pxl_h2 = linear_pixel_interpolation(coords.x - p1.x, pxl3, pxl4);
  return linear_pixel_interpolation(coords.y - p1.y, pxl_h1, pxl_h2);
}

Pixel interpolate(AVFrame *image, FloatPoint coords, Interpolation function) {
  switch (function) {
  case INTERP_NN:
    return interp_nearest_neighbour(image, coords);
  case INTERP_LINEAR:
    return interp_bilinear(image, coords);
  case INTERP_CUBIC:
  default:
    return interp_bicubic(image, coords);
  }
}
