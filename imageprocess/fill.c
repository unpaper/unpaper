// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/fill.h"
#include "imageprocess/pixel.h"

/**
 * Solidly fills a line of pixels heading towards a specified direction
 * until color-changes in the pixels to overwrite exceed the 'intensity'
 * parameter.
 *
 * @param step_x either -1 or 1, if step_y is 0, else 0
 * @param step_y either -1 or 1, if step_x is 0, else 0
 */
static uint64_t fill_line(AVFrame *image, Point p, int8_t step_x, int8_t step_y,
                          Pixel color, uint8_t mask_min, uint8_t mask_max,
                          uint64_t intensity, uint8_t abs_black_threshold) {
  uint64_t distance = 0;
  uint64_t intensityCount =
      1; // first pixel must match, otherwise directly exit

  Rectangle area = clip_rectangle(image, RECT_FULL_IMAGE);

  while (true) {
    p.x += step_x;
    p.y += step_y;
    uint8_t pixel = get_pixel_grayscale(image, p);

    if ((pixel >= mask_min) && (pixel <= mask_max)) {
      intensityCount = intensity; // reset counter
    } else {
      intensityCount--; // allow maximum of 'intensity' pixels to be bright,
      // until stop
    }

    if (intensityCount <= 0 || !point_in_rectangle(p, area)) {
      return distance;
    }

    set_pixel(image, p, color, abs_black_threshold);
    distance++;
  }
}

/**
 * Start flood-filling around the edges of a line which has previously been
 * filled using fillLine(). Here, the flood-fill algorithm performs its
 * indirect recursion.
 *
 * @param step_x either -1 or 1, if step_y is 0, else 0
 * @param step_y either -1 or 1, if step_x is 0, else 0
 * @see fillLine()
 */
static void flood_fill_around_line(AVFrame *image, Point p, int8_t step_x,
                                   int8_t step_y, uint64_t distance,
                                   Pixel color, uint8_t mask_min,
                                   uint8_t mask_max, uint64_t intensity,
                                   uint8_t abs_black_threshold) {
  for (uint64_t d = 0; d < distance; d++) {
    if (step_x != 0) {
      p.x += step_x;
      // indirect recursion
      flood_fill(image, (Point){p.x, p.y + 1}, color, mask_min, mask_max,
                 intensity, abs_black_threshold);
      flood_fill(image, (Point){p.x, p.y - 1}, color, mask_min, mask_max,
                 intensity, abs_black_threshold);
    } else { // step_y != 0
      p.y += step_y;
      flood_fill(image, (Point){p.x + 1, p.y}, color, mask_min, mask_max,
                 intensity, abs_black_threshold);
      flood_fill(image, (Point){p.x - 1, p.y}, color, mask_min, mask_max,
                 intensity, abs_black_threshold);
    }
  }
}

/**
 * Flood-fill an area of pixels. (Naive implementation, optimizable.)
 *
 * @see earlier header-declaration to enable indirect recursive calls
 */
void flood_fill(AVFrame *image, Point p, Pixel color, uint8_t mask_min,
                uint8_t mask_max, uint64_t intensity,
                uint8_t abs_black_threshold) {
  // is current pixel to be filled?
  uint8_t pixel = get_pixel_grayscale(image, p);
  if ((pixel >= mask_min) && (pixel <= mask_max)) {
    // first, fill a 'cross' (both vertical, horizontal line)
    set_pixel(image, p, color, abs_black_threshold);
    const uint64_t left = fill_line(image, p, -1, 0, color, mask_min, mask_max,
                                    intensity, abs_black_threshold);
    const uint64_t top = fill_line(image, p, 0, -1, color, mask_min, mask_max,
                                   intensity, abs_black_threshold);
    const uint64_t right = fill_line(image, p, 1, 0, color, mask_min, mask_max,
                                     intensity, abs_black_threshold);
    const uint64_t bottom = fill_line(image, p, 0, 1, color, mask_min, mask_max,
                                      intensity, abs_black_threshold);
    // now recurse on each neighborhood-pixel of the cross (most recursions will
    // immediately return)
    flood_fill_around_line(image, p, -1, 0, left, color, mask_min, mask_max,
                           intensity, abs_black_threshold);
    flood_fill_around_line(image, p, 0, -1, top, color, mask_min, mask_max,
                           intensity, abs_black_threshold);
    flood_fill_around_line(image, p, 1, 0, right, color, mask_min, mask_max,
                           intensity, abs_black_threshold);
    flood_fill_around_line(image, p, 0, 1, bottom, color, mask_min, mask_max,
                           intensity, abs_black_threshold);
  }
}
