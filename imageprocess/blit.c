// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/blit.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "tools.h"

/**
 * Wipe a rectangular area of pixels with the defined color.
 * @return The number of pixels actually changed.
 */
uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  Rectangle area = clip_rectangle(image, input_area);

  scan_rectangle(area) {
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

  scan_rectangle(area) {
    grayscale += get_pixel_grayscale(image, (Point){x, y});
  }

  return 0xFF - (grayscale / count);
}

/**
 * Returns the inverse average lightness of a rectangular area.
 */
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t lightness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  scan_rectangle(area) {
    lightness += get_pixel_lightness(image, (Point){x, y});
  }

  return 0xFF - (lightness / count);
}

/**
 * Returns the average darkness of a rectangular area.
 */
uint8_t darkness_rect(AVFrame *image, Rectangle input_area) {
  uint64_t darkness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  scan_rectangle(area) {
    darkness += get_pixel_darkness_inverse(image, (Point){x, y});
  }

  return 0xFF - (darkness / count);
}

uint64_t count_pixels_within_brightness(AVFrame *image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  scan_rectangle(area) {
    Point p = {x, y};
    uint8_t brightness = get_pixel_grayscale(image, p);
    if (brightness < min_brightness || brightness > max_brightness) {
      continue;
    }

    if (clear) {
      set_pixel(image, p, PIXEL_WHITE, abs_black_threshold);
    }
    count++;
  }

  return count;
}

void flip_rotate_90(AVFrame **pImage, RotationDirection direction,
                    uint8_t abs_black_threshold) {
  AVFrame *newimage;

  // exchanged width and height
  initImage(&newimage, (*pImage)->height, (*pImage)->width, (*pImage)->format,
            false);

  for (int y = 0; y < (*pImage)->height; y++) {
    const int xx =
        ((direction > 0) ? (*pImage)->height - 1 : 0) - y * direction;
    for (int x = 0; x < (*pImage)->width; x++) {
      const int yy =
          ((direction < 0) ? (*pImage)->width - 1 : 0) + x * direction;

      Point point1 = {x, y};
      Point point2 = {xx, yy};

      set_pixel(newimage, point2, get_pixel(*pImage, point1),
                abs_black_threshold);
    }
  }
  replaceImage(pImage, &newimage);
}

void mirror(AVFrame *image, bool horizontal, bool vertical,
            uint8_t abs_black_threshold) {
  Rectangle source = {{POINT_ORIGIN, POINT_INFINITY}};

  if (horizontal && !vertical) {
    source.vertex[1].x = (image->width - 1) / 2;
  }

  if (vertical) {
    source.vertex[1].y = (image->height - 1) / 2;
  }

  source = clip_rectangle(image, source);

  // Cannot use scan_rectangle() because of the midpoint turn.
  for (int32_t y = source.vertex[0].y; y <= source.vertex[1].y; y++) {
    int32_t yy = vertical ? image->height - y - 1 : y;
    // Special case: the last middle line in odd-lined images that are
    // to be mirrored both horizontally and vertically.
    if (vertical && horizontal && y == yy) {
      source.vertex[1].x = (image->width - 1) / 2;
    }

    for (int32_t x = 0; x <= source.vertex[1].x; x++) {
      int32_t xx = horizontal ? image->width - x - 1 : x;

      Point point1 = {x, y};
      Point point2 = {xx, yy};
      Pixel pixel1 = get_pixel(image, point1);
      Pixel pixel2 = get_pixel(image, point2);
      set_pixel(image, point1, pixel2, abs_black_threshold);
      set_pixel(image, point2, pixel1, abs_black_threshold);
    }
  }
}

void shift_image(AVFrame **pImage, Delta d, uint8_t abs_black_threshold) {
  AVFrame *newimage;

  // allocate new buffer's memory
  initImage(&newimage, (*pImage)->width, (*pImage)->height, (*pImage)->format,
            true);

  copy_rectangle(*pImage, newimage, RECT_FULL_IMAGE,
                 shift_point(POINT_ORIGIN, d), abs_black_threshold);
  replaceImage(pImage, &newimage);
}
