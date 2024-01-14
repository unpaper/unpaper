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

/**
 * Wipe a rectangular area of pixels with the defined color.
 * @return The number of pixels actually changed.
 */
int wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                   uint8_t abs_black_threshold) {
  int count = 0;

  Rectangle area = clip_rectangle(image, input_area);

  for (uint32_t y = area.vertex[0].y; y <= area.vertex[1].y; y++) {
    for (uint32_t x = area.vertex[0].x; x <= area.vertex[1].x; x++) {
      if (set_pixel(image, (Point){x, y}, color, abs_black_threshold)) {
        count++;
      }
    }
  }

  return count;
}
