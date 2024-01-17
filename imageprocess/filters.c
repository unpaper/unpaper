// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdint.h>

#include "constants.h"
#include "imageprocess/pixel.h"

static bool noisefilter_compare_and_clear(AVFrame *image, Point p, bool clear,
                                          uint8_t min_white_level,
                                          uint8_t abs_black_threshold) {
  uint8_t lightness = get_pixel_lightness(image, p);
  if (lightness >= min_white_level) {
    return false;
  }

  if (clear) {
    set_pixel(image, p, PIXEL_WHITE, abs_black_threshold);
  }
  return true;
}

static uint64_t
noisefilter_count_pixel_neighbors_level(AVFrame *image, Point p, uint32_t level,
                                        bool clear, uint8_t min_white_level,
                                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  // upper and lower rows
  for (int32_t xx = p.x - level; xx <= p.x + level; xx++) {
    Point upper = {xx, p.y - level}, lower = {xx, p.y + level};

    count += noisefilter_compare_and_clear(image, upper, clear, min_white_level,
                                           abs_black_threshold)
                 ? 1
                 : 0;
    count += noisefilter_compare_and_clear(image, lower, clear, min_white_level,
                                           abs_black_threshold)
                 ? 1
                 : 0;
  }

  // middle rows
  for (int32_t yy = p.y - (level - 1); yy <= p.y + (level - 1); yy++) {
    Point first = {p.x - level, yy}, last = {p.x + level, yy};
    count += noisefilter_compare_and_clear(image, first, clear, min_white_level,
                                           abs_black_threshold)
                 ? 1
                 : 0;
    count += noisefilter_compare_and_clear(image, last, clear, min_white_level,
                                           abs_black_threshold)
                 ? 1
                 : 0;
  }

  return count;
}

static uint64_t noisefilter_count_pixel_neighbors(AVFrame *image, Point p,
                                                  uint64_t intensity,
                                                  uint8_t min_white_level,
                                                  uint8_t abs_black_threshold) {
  // can finish when one level is completely zero
  uint64_t count = 1; // assume self as set
  uint64_t lCount;
  uint32_t level = 1;
  do {
    lCount = noisefilter_count_pixel_neighbors_level(
        image, p, level, false, min_white_level, abs_black_threshold);
    count += lCount;
    level++;
  } while (lCount != 0 && (level <= intensity));

  return count;
}

static void noisefilter_clear_pixel_neighbors(AVFrame *image, Point p,
                                              uint8_t min_white_level,
                                              uint8_t abs_black_threshold) {
  set_pixel(image, p, PIXEL_WHITE, abs_black_threshold);

  // lCount will become 0, otherwise countPixelNeighbors() would previously have
  // delivered a bigger value (and this here would not have been called)
  uint64_t lCount;
  uint32_t level = 1;
  do {
    lCount = noisefilter_count_pixel_neighbors_level(
        image, p, level, true, min_white_level, abs_black_threshold);
    level++;
  } while (lCount != 0);
}

/**
 * Applies a simple noise filter to the image.
 *
 * @param intensity maximum cluster size to delete
 */
uint64_t noisefilter(AVFrame *image, uint64_t intensity,
                     uint8_t min_white_level, uint8_t abs_black_threshold) {
  uint64_t count = 0;

  for (int32_t y = 0; y < image->height; y++) {
    for (int32_t x = 0; x < image->width; x++) {
      Point p = {x, y};

      uint8_t darkness = get_pixel_darkness_inverse(image, p);
      if (darkness < min_white_level) { // one dark pixel found
        // get number of non-light pixels in neighborhood
        uint64_t neighbors = noisefilter_count_pixel_neighbors(
            image, p, intensity, min_white_level, abs_black_threshold);

        // If not more than 'intensity', delete area.
        if (neighbors <= intensity) {
          noisefilter_clear_pixel_neighbors(image, p, min_white_level,
                                            abs_black_threshold);
          count++;
        }
      }
    }
  }
  return count;
}
