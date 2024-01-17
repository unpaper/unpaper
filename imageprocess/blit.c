// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "blit.h"

#define max(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a > _b ? _a : _b;                                                         \
  })

#define min(a, b)                                                              \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    _a < _b ? _a : _b;                                                         \
  })

#define scan_rect(area)                                                        \
  for (int32_t y = area.vertex[0].y; y <= area.vertex[1].y; y++)               \
    for (int32_t x = area.vertex[0].x; x <= area.vertex[1].x; x++)

static Rectangle normalize_rectangle(Rectangle input) {
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

static Rectangle clip_rectangle(AVFrame *image, Rectangle area) {
  Rectangle normal_area = normalize_rectangle(area);

  return (Rectangle){
      .vertex =
          {
              {
                  .x = max(normal_area.vertex[0].x, 0),
                  .y = max(normal_area.vertex[0].y, 0),
              },
              {
                  .x = min(normal_area.vertex[1].x, (image->width - 1)),
                  .y = min(normal_area.vertex[1].y, (image->height - 1)),
              },
          },
  };
}

static uint64_t count_pixels(Rectangle area) {
  return ((abs(area.vertex[0].x - area.vertex[1].x) + 1) *
          (abs(area.vertex[0].y - area.vertex[1].y) + 1));
}

/**
 * Wipe a rectangular area of pixels with the defined color.
 * @return The number of pixels actually changed.
 */
uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  Rectangle area = clip_rectangle(image, input_area);

  scan_rect(area) {
    if (set_pixel(image, (Point){x, y}, color, abs_black_threshold)) {
      count++;
    }
  }

  return count;
}

void copy_rectangle(AVFrame *source, AVFrame *target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold) {
  Rectangle area = clip_rectangle(source, source_area);

  // naive but generic implementation
  for (int32_t sY = area.vertex[0].y, tY = target_coords.y;
       sY <= area.vertex[1].y; sY++, tY++) {
    for (int32_t sX = area.vertex[0].x, tX = target_coords.x;
         sX <= area.vertex[1].x; sX++, tX++) {
      set_pixel(target, (Point){tX, tY}, get_pixel(source, (Point){sX, sY}),
                abs_black_threshold);
    }
  }
}

/**
 * Returns the average brightness of a rectangular area.
 */
uint8_t inverse_brightness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t grayscale = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  scan_rect(area) { grayscale += get_pixel_grayscale(image, (Point){x, y}); }

  return 0xFF - (grayscale / count);
}

/**
 * Returns the inverse average lightness of a rectangular area.
 */
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t lightness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  scan_rect(area) { lightness += get_pixel_lightness(image, (Point){x, y}); }

  return 0xFF - (lightness / count);
}

/**
 * Returns the average darkness of a rectangular area.
 */
uint8_t darkness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t darkness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  scan_rect(area) {
    darkness += get_pixel_darkness_inverse(image, (Point){x, y});
  }

  return 0xFF - (darkness / count);
}