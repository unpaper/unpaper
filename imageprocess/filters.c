// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <inttypes.h>
#include <stdint.h>

#include "constants.h"
#include "imageprocess/blit.h"
#include "imageprocess/fill.h"
#include "imageprocess/filters.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

/***************
 * Blackfilter *
 ***************/

BlackfilterParameters validate_blackfilter_parameters(
    uint32_t scan_size_h, uint32_t scan_size_v, uint32_t scan_step_h,
    uint32_t scan_step_v, uint32_t scan_depth_h, uint32_t scan_depth_v,
    int8_t scan_directions, float threshold, int32_t intensity,
    size_t exclusions_count, Rectangle *exclusions) {
  return (BlackfilterParameters){
      .scan_size =
          {
              .width = scan_size_h,
              .height = scan_size_v,
          },
      .scan_step =
          {
              .horizontal = scan_step_h,
              .vertical = scan_step_v,
          },
      .scan_depth =
          {
              .horizontal = scan_depth_h,
              .vertical = scan_depth_v,
          },

      .scan_horizontal = !!(scan_directions & 1 << HORIZONTAL),
      .scan_vertical = !!(scan_directions & 1 << VERTICAL),

      .abs_threshold = UINT8_MAX * threshold,
      .intensity = intensity,

      .exclusions_count = exclusions_count,
      .exclusions = exclusions,
  };
}

static void blackfilter_scan(Image image, BlackfilterParameters params,
                             Delta step, RectangleSize stripe_size,
                             Delta shift) {
  if (step.horizontal != 0 && step.vertical != 0) {
    errOutput("blackfilter_scan() called with diagonal steps, impossible! "
              "(%" PRId32 ", %" PRId32 ")",
              step.horizontal, step.vertical);
  }

  const Rectangle image_area = full_image(image);

  Rectangle area = rectangle_from_size(POINT_ORIGIN, stripe_size);
  while (point_in_rectangle(area.vertex[0], image_area)) {
    // Make sure last stripe does not reach outside the sheet, shift back
    // inside. We don't use clipping to avoid changing the filter size!
    if (!point_in_rectangle(area.vertex[1], image_area)) {
      Delta d = distance_between(area.vertex[1], image_area.vertex[1]);

      area = shift_rectangle(area, d);
    }

    bool already_excluded_logged = false;

    do {
      uint8_t blackness = darkness_rect(image, area);

      // If we find a solidly black area.
      if (blackness >= params.abs_threshold) {
        if (!rectangle_overlap_any(area, params.exclusions_count,
                                   params.exclusions)) {
          verboseLog(VERBOSE_NORMAL, "black-area flood-fill: [%d,%d,%d,%d]\n",
                     area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
                     area.vertex[1].y);
          already_excluded_logged = false;
          // start flood-fill in this area (on each pixel to make sure we get
          // everything, in most cases first flood-fill from first pixel will
          // delete all other black pixels in the area already)
          scan_rectangle(area) {
            flood_fill(image, (Point){x, y}, PIXEL_WHITE, 0,
                       image.abs_black_threshold, params.intensity);
          }
        } else if (!already_excluded_logged) {
          verboseLog(VERBOSE_NORMAL, "black-area EXCLUDED: [%d,%d,%d,%d]\n",
                     area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
                     area.vertex[1].y);
          already_excluded_logged = true; // do this only once per scan-stripe,
                                          // otherwise too many messages}
        }
      }

      area = shift_rectangle(area, step);
    } while (point_in_rectangle(area.vertex[0], image_area));

    area = shift_rectangle(area, shift);
  }
}

/**
 * Filters out solidly black areas, as appearing on bad photocopies.
 * A virtual bar of width 'size' and height 'depth' is horizontally moved
 * above the middle of the sheet (or the full sheet, if depth ==-1).
 */
void blackfilter(Image image, BlackfilterParameters params) {
  // Left-to-Right scan.
  if (params.scan_horizontal) {
    blackfilter_scan(
        image, params, (Delta){params.scan_step.horizontal, 0},
        (RectangleSize){params.scan_size.width, params.scan_depth.vertical},
        (Delta){0, params.scan_depth.vertical});
  }

  // To-to-Bottom scan.
  if (params.scan_vertical) {
    blackfilter_scan(
        image, params, (Delta){0, params.scan_step.vertical},
        (RectangleSize){params.scan_depth.horizontal, params.scan_size.height},
        (Delta){params.scan_depth.horizontal, 0});
  }
}

/**************
 * Blurfilter *
 **************/

BlurfilterParameters validate_blurfilter_parameters(uint32_t scan_size_h,
                                                    uint32_t scan_size_v,
                                                    uint32_t scan_step_h,
                                                    uint32_t scan_step_v,
                                                    float intensity) {
  return (BlurfilterParameters){
      .scan_size =
          {
              .width = scan_size_h,
              .height = scan_size_v,
          },
      .scan_step =
          {
              .horizontal = scan_step_h,
              .vertical = scan_step_v,
          },
      .intensity = intensity,
  };
}

uint64_t blurfilter(Image image, BlurfilterParameters params,
                    uint8_t abs_white_threshold) {
  RectangleSize image_size = size_of_image(image);
  const uint32_t blocks_per_row = image_size.width / params.scan_size.width;
  const uint64_t total_pixels_in_block =
      params.scan_size.width * params.scan_size.height;
  uint64_t result = 0;

  // allocate one extra block left and right
  uint64_t count_buffers[3][blocks_per_row + 2];

  // Number of dark pixels in previous row
  uint64_t *prevCounts = &count_buffers[0][0];
  // Number of dark pixels in current row
  uint64_t *curCounts = &count_buffers[0][1];
  // Number of dark pixels in next row
  uint64_t *nextCounts = &count_buffers[0][2];

  // Left and Right.
  curCounts[0] = total_pixels_in_block;
  curCounts[blocks_per_row] = total_pixels_in_block;
  nextCounts[0] = total_pixels_in_block;
  nextCounts[blocks_per_row] = total_pixels_in_block;

  const int32_t max_left = image_size.width - params.scan_size.width;
  for (int32_t left = 0, block = 1; left <= max_left;
       left += params.scan_size.width) {
    curCounts[block++] = count_pixels_within_brightness(
        image, rectangle_from_size((Point){left, 0}, params.scan_size), 0,
        abs_white_threshold, false);
  }

  // Loop through all blocks. For a block calculate the number of dark pixels in
  // this block, the number of dark pixels in the block in the top-left corner
  // and similarly for the block in the top-right, bottom-left and bottom-right
  // corner. Take the maximum of these values. Clear the block if this number is
  // not large enough compared to the total number of pixels in a block.
  int32_t max_top = image_size.height - params.scan_size.height;
  for (int32_t top = 0; top <= max_top; top += params.scan_size.height) {
    nextCounts[0] = count_pixels_within_brightness(
        image,
        rectangle_from_size((Point){0, top + params.scan_step.vertical},
                            params.scan_size),
        0, abs_white_threshold, false);

    for (int32_t left = 0, block = 1; left <= max_left;
         left += params.scan_size.width) {

      // bottom right (has still to be calculated)
      nextCounts[block + 1] = count_pixels_within_brightness(
          image,
          rectangle_from_size((Point){left + params.scan_size.width,
                                      top + params.scan_step.vertical},
                              params.scan_size),
          0, abs_white_threshold, false);

      uint64_t max = max3(
          nextCounts[block - 1], nextCounts[block + 1],
          max3(prevCounts[block - 1], prevCounts[block + 1], curCounts[block]));

      // Not enough dark pixels
      if ((((float)max) / total_pixels_in_block) <= params.intensity) {
        wipe_rectangle(
            image, rectangle_from_size((Point){left, top}, params.scan_size),
            PIXEL_WHITE);
        result += curCounts[block];
        curCounts[block] = total_pixels_in_block; // Update information
      }

      block++;
    }

    // Switch Buffers
    uint64_t *tmpCounts;
    tmpCounts = prevCounts;
    prevCounts = curCounts;
    curCounts = nextCounts;
    nextCounts = tmpCounts;
  }
  return result;
}

/***************
 * Noisefilter *
 ***************/

static bool noisefilter_compare_and_clear(Image image, Point p, bool clear,
                                          uint8_t min_white_level) {
  uint8_t lightness = get_pixel_lightness(image, p);
  if (lightness >= min_white_level) {
    return false;
  }

  if (clear) {
    set_pixel(image, p, PIXEL_WHITE);
  }
  return true;
}

static uint64_t
noisefilter_count_pixel_neighbors_level(Image image, Point p, uint32_t level,
                                        bool clear, uint8_t min_white_level) {
  uint64_t count = 0;

  // upper and lower rows
  for (int32_t xx = p.x - level; xx <= p.x + level; xx++) {
    Point upper = {xx, p.y - level}, lower = {xx, p.y + level};

    count += noisefilter_compare_and_clear(image, upper, clear, min_white_level)
                 ? 1
                 : 0;
    count += noisefilter_compare_and_clear(image, lower, clear, min_white_level)
                 ? 1
                 : 0;
  }

  // middle rows
  for (int32_t yy = p.y - (level - 1); yy <= p.y + (level - 1); yy++) {
    Point first = {p.x - level, yy}, last = {p.x + level, yy};
    count += noisefilter_compare_and_clear(image, first, clear, min_white_level)
                 ? 1
                 : 0;
    count += noisefilter_compare_and_clear(image, last, clear, min_white_level)
                 ? 1
                 : 0;
  }

  return count;
}

static uint64_t noisefilter_count_pixel_neighbors(Image image, Point p,
                                                  uint64_t intensity,
                                                  uint8_t min_white_level) {
  // can finish when one level is completely zero
  uint64_t count = 1; // assume self as set
  uint64_t lCount;
  uint32_t level = 1;
  do {
    lCount = noisefilter_count_pixel_neighbors_level(image, p, level, false,
                                                     min_white_level);
    count += lCount;
    level++;
  } while (lCount != 0 && (level <= intensity));

  return count;
}

static void noisefilter_clear_pixel_neighbors(Image image, Point p,
                                              uint8_t min_white_level) {
  set_pixel(image, p, PIXEL_WHITE);

  // lCount will become 0, otherwise countPixelNeighbors() would previously have
  // delivered a bigger value (and this here would not have been called)
  uint64_t lCount;
  uint32_t level = 1;
  do {
    lCount = noisefilter_count_pixel_neighbors_level(image, p, level, true,
                                                     min_white_level);
    level++;
  } while (lCount != 0);
}

/**
 * Applies a simple noise filter to the image.
 *
 * @param intensity maximum cluster size to delete
 */
uint64_t noisefilter(Image image, uint64_t intensity, uint8_t min_white_level) {
  uint64_t count = 0;
  Rectangle area = full_image(image);

  scan_rectangle(area) {
    Point p = {x, y};

    uint8_t darkness = get_pixel_darkness_inverse(image, p);
    if (darkness < min_white_level) { // one dark pixel found
      // get number of non-light pixels in neighborhood
      uint64_t neighbors = noisefilter_count_pixel_neighbors(
          image, p, intensity, min_white_level);

      // If not more than 'intensity', delete area.
      if (neighbors <= intensity) {
        noisefilter_clear_pixel_neighbors(image, p, min_white_level);
        count++;
      }
    }
  }
  return count;
}

/***************
 * Grayfilter *
 ***************/

GrayfilterParameters validate_grayfilter_parameters(uint32_t scan_size_h,
                                                    uint32_t scan_size_v,
                                                    uint32_t scan_step_h,
                                                    uint32_t scan_step_v,
                                                    float threshold) {
  return (GrayfilterParameters){
      .scan_size =
          {
              .width = scan_size_h,
              .height = scan_size_v,
          },
      .scan_step =
          {
              .horizontal = scan_step_h,
              .vertical = scan_step_v,
          },
      .abs_threshold = UINT8_MAX * threshold,
  };
}

uint64_t grayfilter(Image image, GrayfilterParameters params) {
  RectangleSize image_size = size_of_image(image);
  Point filter_origin = POINT_ORIGIN;
  uint64_t result = 0;

  do {
    Rectangle area = rectangle_from_size(filter_origin, params.scan_size);
    uint64_t count = count_pixels_within_brightness(
        image, area, 0, image.abs_black_threshold, false);

    if (count == 0) {
      uint8_t lightness = inverse_lightness_rect(image, area);
      // (lower threshold->more deletion)
      if (lightness < params.abs_threshold) {
        result += wipe_rectangle(image, area, PIXEL_WHITE);
      }
    }

    // Continue on the same row unless we reached the end of the row.
    if (filter_origin.x < image_size.width) {
      filter_origin.x += params.scan_step.horizontal;
    } else {
      // next row:
      filter_origin.x = 0;
      filter_origin.y += params.scan_step.vertical;
    }
  } while (filter_origin.y <= image_size.height);

  return result;
}
