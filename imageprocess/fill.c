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
static uint64_t fill_line(Image image, Point p, Delta step, Pixel color,
                          uint8_t mask_min, uint8_t mask_max,
                          uint64_t intensity, uint8_t abs_black_threshold) {
  uint64_t distance = 0;
  uint64_t intensityCount =
      1; // first pixel must match, otherwise directly exit

  Rectangle area = full_image(image);

  while (true) {
    p = shift_point(p, step);
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
static void flood_fill_around_line(Image image, Point p, Delta step,
                                   uint64_t distance, Pixel color,
                                   uint8_t mask_min, uint8_t mask_max,
                                   uint64_t intensity,
                                   uint8_t abs_black_threshold) {
  for (uint64_t d = 0; d < distance; d++) {
    if (step.horizontal != 0) {
      p.x += step.horizontal;
      // indirect recursion
      flood_fill(image, shift_point(p, DELTA_DOWNWARD), color, mask_min,
                 mask_max, intensity, abs_black_threshold);
      flood_fill(image, shift_point(p, DELTA_UPWARD), color, mask_min, mask_max,
                 intensity, abs_black_threshold);
    } else { // step.vertical != 0
      p.y += step.vertical;
      flood_fill(image, shift_point(p, DELTA_RIGHTWARD), color, mask_min,
                 mask_max, intensity, abs_black_threshold);
      flood_fill(image, shift_point(p, DELTA_LEFTWARD), color, mask_min,
                 mask_max, intensity, abs_black_threshold);
    }
  }
}

/**
 * Flood-fill an area of pixels. (Naive implementation, optimizable.)
 *
 * @see earlier header-declaration to enable indirect recursive calls
 */
void flood_fill(Image image, Point p, Pixel color, uint8_t mask_min,
                uint8_t mask_max, uint64_t intensity,
                uint8_t abs_black_threshold) {
  // is current pixel to be filled?
  uint8_t pixel = get_pixel_grayscale(image, p);
  if ((pixel >= mask_min) && (pixel <= mask_max)) {
    // first, fill a 'cross' (both vertical, horizontal line)
    set_pixel(image, p, color, abs_black_threshold);
    const uint64_t left = fill_line(image, p, DELTA_LEFTWARD, color, mask_min,
                                    mask_max, intensity, abs_black_threshold);
    const uint64_t top = fill_line(image, p, DELTA_UPWARD, color, mask_min,
                                   mask_max, intensity, abs_black_threshold);
    const uint64_t right = fill_line(image, p, DELTA_RIGHTWARD, color, mask_min,
                                     mask_max, intensity, abs_black_threshold);
    const uint64_t bottom = fill_line(image, p, DELTA_DOWNWARD, color, mask_min,
                                      mask_max, intensity, abs_black_threshold);
    // now recurse on each neighborhood-pixel of the cross (most recursions will
    // immediately return)
    flood_fill_around_line(image, p, DELTA_LEFTWARD, left, color, mask_min,
                           mask_max, intensity, abs_black_threshold);
    flood_fill_around_line(image, p, DELTA_UPWARD, top, color, mask_min,
                           mask_max, intensity, abs_black_threshold);
    flood_fill_around_line(image, p, DELTA_RIGHTWARD, right, color, mask_min,
                           mask_max, intensity, abs_black_threshold);
    flood_fill_around_line(image, p, DELTA_DOWNWARD, bottom, color, mask_min,
                           mask_max, intensity, abs_black_threshold);
  }
}
