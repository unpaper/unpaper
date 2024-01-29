// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "imageprocess/blit.h"
#include "imageprocess/masks.h"
#include "imageprocess/pixel.h"
#include "imageprocess/primitives.h"
#include "lib/logging.h"

MaskDetectionParameters
validate_mask_detection_parameters(int scan_directions,
                                   const int scan_size[DIRECTIONS_COUNT],
                                   const int scan_depth[DIRECTIONS_COUNT],
                                   const int scan_step[DIRECTIONS_COUNT],
                                   const float scan_threshold[DIRECTIONS_COUNT],
                                   const int scan_mininum[DIMENSIONS_COUNT],
                                   const int scan_maximum[DIMENSIONS_COUNT]) {
  return (MaskDetectionParameters){
      .scan_size =
          {
              .width = scan_size[HORIZONTAL],
              .height = scan_size[VERTICAL],
          },
      .scan_step =
          {
              .horizontal = scan_step[HORIZONTAL],
              .vertical = scan_step[VERTICAL],
          },
      .scan_depth =
          {
              .horizontal = scan_depth[HORIZONTAL],
              .vertical = scan_depth[VERTICAL],
          },
      .scan_threshold =
          {
              .horizontal = scan_threshold[HORIZONTAL],
              .vertical = scan_threshold[VERTICAL],
          },

      .scan_horizontal = !!(scan_directions & 1 << HORIZONTAL),
      .scan_vertical = !!(scan_directions & 1 << VERTICAL),

      .minimum_width = scan_mininum[HORIZONTAL],
      .maximum_width = scan_maximum[HORIZONTAL],

      .minimum_height = scan_mininum[VERTICAL],
      .maximum_height = scan_maximum[VERTICAL],
  };
}

/**
 * Finds one edge of non-black pixels heading from one starting point towards
 * edge direction.
 *
 * @return number of shift-steps until blank edge found
 */
static uint32_t detect_edge(Image image, Point origin, Delta step,
                            int32_t scan_size, int32_t scan_depth,
                            float threshold) {
  Rectangle scan_area;
  RectangleSize image_size = size_of_image(image);

  // either shiftX or shiftY is 0, the other value is -i|+i
  if (step.vertical == 0) {
    // vertical border is to be detected, horizontal shifting of scan-bar
    if (scan_depth == -1) {
      scan_depth = image_size.height;
    }

    scan_area = rectangle_from_size(
        shift_point(origin, (Delta){-scan_size / 2, -scan_depth / 2}),
        (RectangleSize){scan_size, scan_depth});
  } else if (step.horizontal == 0) {
    // horizontal border is to be detected, vertical shifting of scan-bar
    if (scan_depth == -1) {
      scan_depth = image_size.width;
    }

    scan_area = rectangle_from_size(
        shift_point(origin, (Delta){-scan_depth / 2, -scan_size / 2}),
        (RectangleSize){scan_depth, scan_size});
  } else {
    errOutput("detect_edge() called with diagonal steps, impossible! "
              "(%" PRId32 ", %" PRId32 ")",
              step.horizontal, step.vertical);
  }

  uint32_t total = 0;
  uint32_t count = 0;
  uint8_t blackness;
  do {
    blackness = inverse_brightness_rect(image, scan_area);
    total += blackness;
    count++;
    scan_area = shift_rectangle(scan_area, step);
    // is blackness below threshold*average?
    // this will surely become true when pos reaches the outside of
    // the actual image area and blacknessRect() will deliver 0
    // because all pixels outside are considered white
  } while ((blackness >= ((threshold * total) / count)) && blackness != 0);

  return count;
}

/**
 * Detects a mask of white borders around a starting point.
 * The result is returned via call-by-reference parameters left, top, right,
 * bottom.
 */
static bool detect_mask(Image image, MaskDetectionParameters params,
                        Point origin, Rectangle *mask) {
  RectangleSize image_size = size_of_image(image);

  if (params.scan_horizontal) {
    int32_t left_edge =
        detect_edge(image, origin, (Delta){-params.scan_step.horizontal, 0},
                    params.scan_size.width, params.scan_depth.horizontal,
                    params.scan_threshold.horizontal);
    int32_t right_edge =
        detect_edge(image, origin, (Delta){params.scan_step.horizontal, 0},
                    params.scan_size.width, params.scan_depth.horizontal,
                    params.scan_threshold.horizontal);

    mask->vertex[0].x = origin.x - (params.scan_step.horizontal * left_edge) -
                        params.scan_size.width / 2;
    mask->vertex[1].x = origin.x + (params.scan_step.horizontal * right_edge) +
                        params.scan_size.width / 2;
  } else { // full range of sheet
    mask->vertex[0].x = 0;
    mask->vertex[1].x = image_size.width - 1;
  }

  if (params.scan_vertical) {
    int32_t top_edge =
        detect_edge(image, origin, (Delta){0, -params.scan_step.vertical},
                    params.scan_size.height, params.scan_depth.vertical,
                    params.scan_threshold.vertical);
    int32_t bottom_edge =
        detect_edge(image, origin, (Delta){0, params.scan_step.vertical},
                    params.scan_size.height, params.scan_depth.vertical,
                    params.scan_threshold.vertical);

    mask->vertex[0].y = origin.y - (params.scan_step.vertical * top_edge) -
                        params.scan_size.height / 2;
    mask->vertex[1].y = origin.y + (params.scan_step.vertical * bottom_edge) +
                        params.scan_size.height / 2;
  } else {
    mask->vertex[0].y = 0;
    mask->vertex[1].y = image_size.height - 1;
  }

  // Clip to maximum if below minimum or above maximum.
  RectangleSize size = size_of_rectangle(*mask);
  bool success = true;

  if ((params.minimum_width != -1 && size.width < params.minimum_width) ||
      (params.maximum_width != -1 && size.width > params.maximum_width)) {
    verboseLog(VERBOSE_DEBUG, "mask width (%d) not within min/max (%d / %d)\n",
               size.width, params.minimum_width, params.maximum_width);
    mask->vertex[0].x = origin.x - params.maximum_width / 2;
    mask->vertex[1].x = origin.x + params.maximum_width / 2;
    success = false;
  }

  if ((params.minimum_height != -1 && size.height < params.minimum_height) ||
      (params.maximum_height != -1 && size.height > params.maximum_height)) {
    verboseLog(VERBOSE_DEBUG, "mask height (%d) not within min/max (%d / %d)\n",
               size.height, params.minimum_height, params.maximum_height);
    mask->vertex[0].y = origin.y - params.maximum_height / 2;
    mask->vertex[1].y = origin.y + params.maximum_height / 2;
    success = false;
  }

  return success;
}

static const Rectangle INVALID_MASK = {{{-1, -1}, {-1, -1}}};

/**
 * Detects masks around the points specified in point[].
 *
 * @return number of masks stored in mask[][]
 */
size_t detect_masks(Image image, MaskDetectionParameters params,
                    const Point points[], size_t points_count,
                    Rectangle masks[]) {
  size_t masks_count = 0;
  if (!params.scan_horizontal && !params.scan_vertical) {
    return masks_count;
  }

  for (size_t i = 0; i < points_count; i++) {
    bool mask_valid = detect_mask(image, params, points[i], &masks[i]);

    // Compare the newly-detected mask with an invalid mask where all the
    // vertex are (-1, -1)
    if (memcmp(&masks[i], &INVALID_MASK, sizeof(INVALID_MASK)) != 0) {
      masks_count++;

      verboseLog(
          VERBOSE_NORMAL, "auto-masking (%d,%d): %d,%d,%d,%d%s\n", points[i].x,
          points[i].y, masks[i].vertex[0].x, masks[i].vertex[0].y,
          masks[i].vertex[1].x, masks[i].vertex[1].y,
          mask_valid ? "" : " (invalid detection, using full page size)");
    } else {
      verboseLog(VERBOSE_NORMAL, "auto-masking (%d,%d): NO MASK FOUND\n",
                 points[i].x, points[i].y);
    }
  }

  return masks_count;
}

/**
 * Moves a rectangular area of pixels to be centered above the centerX, centerY
 * coordinates.
 */
void center_mask(Image image, const Point center, const Rectangle area) {
  const RectangleSize size = size_of_rectangle(area);
  const Rectangle image_area = full_image(image);

  const Point target =
      shift_point(center, (Delta){-size.width / 2, -size.height / 2});

  Rectangle new_area = rectangle_from_size(target, size);

  if (rectangle_in_rectangle(new_area, image_area)) {
    verboseLog(VERBOSE_NORMAL, "centering mask [%d,%d,%d,%d] (%d,%d): %d, %d\n",
               area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
               area.vertex[1].y, center.x, center.y,
               target.x - area.vertex[0].x, target.y - area.vertex[0].y);
    Image newimage = create_compatible_image(image, size, false);
    copy_rectangle(image, newimage, area, POINT_ORIGIN);
    wipe_rectangle(image, area, image.background);
    copy_rectangle(newimage, image, full_image(newimage), target);
    free_image(&newimage);
  } else {
    verboseLog(VERBOSE_NORMAL,
               "centering mask [%d,%d,%d,%d] (%d,%d): %d, %d - NO CENTERING "
               "(would shift area outside visible image)\n",
               area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
               area.vertex[1].y, center.x, center.y,
               target.x - area.vertex[0].x, target.y - area.vertex[0].y);
  }
}

MaskAlignmentParameters validate_mask_alignment_parameters(int border_align,
                                                           Delta margin) {
  return (MaskAlignmentParameters){
      .alignment =
          {
              .left = !!(border_align & (1 << LEFT)),
              .top = !!(border_align & (1 << TOP)),
              .right = !!(border_align & (1 << RIGHT)),
              .bottom = !!(border_align & (1 << BOTTOM)),
          },
      .margin = margin,
  };
}

/**
 * Moves a rectangular area of pixels to be centered inside a specified area
 * coordinates.
 */
void align_mask(Image image, const Rectangle inside_area,
                const Rectangle outside, MaskAlignmentParameters params) {
  const RectangleSize inside_size = size_of_rectangle(inside_area);

  Point target;

  if (params.alignment.left) {
    target.x = outside.vertex[0].x + params.margin.horizontal;
  } else if (params.alignment.right) {
    target.x =
        outside.vertex[1].x - inside_size.width - params.margin.horizontal;
  } else {
    target.x =
        (outside.vertex[0].x + outside.vertex[1].x - inside_size.width) / 2;
  }
  if (params.alignment.top) {
    target.y = outside.vertex[0].y + params.margin.vertical;
  } else if (params.alignment.bottom) {
    target.y =
        outside.vertex[1].y - inside_size.height - params.margin.vertical;
  } else {
    target.y =
        (outside.vertex[0].y + outside.vertex[1].y - inside_size.height) / 2;
  }
  verboseLog(VERBOSE_NORMAL, "aligning mask [%d,%d,%d,%d] (%d,%d): %d, %d\n",
             inside_area.vertex[0].x, inside_area.vertex[0].y,
             inside_area.vertex[1].x, inside_area.vertex[1].y, target.x,
             target.y, target.x - inside_area.vertex[0].x,
             target.y - inside_area.vertex[0].y);

  Image newimage = create_compatible_image(image, inside_size, true);
  copy_rectangle(image, newimage, inside_area, POINT_ORIGIN);
  wipe_rectangle(image, inside_area, image.background);
  copy_rectangle(newimage, image, full_image(newimage), target);
  free_image(&newimage);
}

/**
 * Permanently applies image masks. Each pixel which is not covered by at least
 * one mask is set to maskColor.
 */
void apply_masks(Image image, const Rectangle masks[], size_t masks_count,
                 Pixel color) {
  if (masks_count <= 0) {
    return;
  }

  Rectangle image_area = full_image(image);

  scan_rectangle(image_area) {
    Point p = {x, y};
    if (!point_in_rectangles_any(p, masks_count, masks)) {
      set_pixel(image, p, color);
    }
  }
}

/**
 * Permanently wipes out areas of an images. Each pixel covered by a wipe-area
 * is set to wipeColor.
 */
void apply_wipes(Image image, Rectangle wipes[], size_t wipes_count,
                 Pixel color) {
  for (size_t i = 0; i < wipes_count; i++) {
    scan_rectangle(wipes[i]) { set_pixel(image, (Point){x, y}, color); }

    verboseLog(VERBOSE_MORE, "wipe [%d,%d,%d,%d]\n", wipes[i].vertex[0].x,
               wipes[i].vertex[0].y, wipes[i].vertex[1].x,
               wipes[i].vertex[1].y);
  }
}

Rectangle border_to_mask(Image image, const Border border) {
  RectangleSize image_size = size_of_image(image);

  Rectangle mask = {{
      {border.left, border.top},
      {image_size.width - border.right - 1,
       image_size.height - border.bottom - 1},
  }};
  verboseLog(VERBOSE_DEBUG, "border [%d,%d,%d,%d] -> mask [%d,%d,%d,%d]\n",
             border.left, border.top, border.right, border.bottom,
             mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
             mask.vertex[1].y);

  return mask;
}

/**
 * Applies a border to the whole image. All pixels in the border range at the
 * edges of the sheet will be cleared.
 */
void apply_border(Image image, const Border border, Pixel color) {
  if (memcmp(&border, &BORDER_NULL, sizeof(BORDER_NULL)) == 0) {
    return;
  }

  Rectangle mask = border_to_mask(image, border);
  verboseLog(VERBOSE_NORMAL, "applying border (%d,%d,%d,%d) [%d,%d,%d,%d]\n",
             border.left, border.top, border.right, border.bottom,
             mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
             mask.vertex[1].y);
  apply_masks(image, &mask, 1, color);
}

BorderScanParameters
validate_border_scan_parameters(int scan_directions,
                                const int scan_size[DIRECTIONS_COUNT],
                                const int scan_step[DIRECTIONS_COUNT],
                                const int scan_threshold[DIRECTIONS_COUNT]) {
  return (BorderScanParameters){
      .scan_size =
          {
              .width = scan_size[HORIZONTAL],
              .height = scan_size[VERTICAL],
          },
      .scan_step =
          {
              .horizontal = scan_step[HORIZONTAL],
              .vertical = scan_step[VERTICAL],
          },
      .scan_threshold =
          {
              .horizontal = scan_threshold[HORIZONTAL],
              .vertical = scan_threshold[VERTICAL],
          },

      .scan_horizontal = !!(scan_directions & 1 << HORIZONTAL),
      .scan_vertical = !!(scan_directions & 1 << VERTICAL),
  };
}

/**
 * Find the size of one border edge.
 */
static uint32_t detect_border_edge(Image image, const Rectangle outside_mask,
                                   Delta step, int32_t size,
                                   int32_t threshold) {
  Rectangle area = outside_mask;
  RectangleSize mask_size = size_of_rectangle(outside_mask);
  int32_t max_step;

  if (step.vertical == 0) { // horizontal detection
    if (step.horizontal > 0) {
      area.vertex[1].x = outside_mask.vertex[0].x + size;
    } else {
      area.vertex[0].x = outside_mask.vertex[1].x - size;
    }
    max_step = mask_size.width;
  } else { // vertical detection
    if (step.vertical > 0) {
      area.vertex[1].y = outside_mask.vertex[0].y + size;
    } else {
      area.vertex[0].y = outside_mask.vertex[1].y - size;
    }
    max_step = mask_size.height;
  }

  uint32_t result = 0;
  while (result < max_step) {
    uint32_t cnt = count_pixels_within_brightness(
        image, area, 0, image.abs_black_threshold, false);
    if (cnt >= threshold) {
      return result; // border has been found: regular exit here
    }

    area = shift_rectangle(area, step);

    // (either stepX or stepY is 0)
    result += abs(step.horizontal + step.vertical);
  }

  return 0; // no border found between 0..max_step
}

/**
 * Detects a border of completely non-black pixels around the area
 * outsideBorder.
 */
Border detect_border(Image image, BorderScanParameters params,
                     const Rectangle outside_mask) {
  RectangleSize image_size = size_of_image(image);

  Border border = {
      .left = outside_mask.vertex[0].x,
      .top = outside_mask.vertex[0].y,
      .right = image_size.width - outside_mask.vertex[1].x,
      .bottom = image_size.height - outside_mask.vertex[1].y,
  };

  if (params.scan_horizontal) {
    border.left += detect_border_edge(
        image, outside_mask, (Delta){params.scan_step.horizontal, 0},
        params.scan_size.width, params.scan_threshold.horizontal);
    border.right += detect_border_edge(
        image, outside_mask, (Delta){-params.scan_step.horizontal, 0},
        params.scan_size.width, params.scan_threshold.horizontal);
  }
  if (params.scan_vertical) {
    border.top += detect_border_edge(
        image, outside_mask, (Delta){0, params.scan_step.vertical},
        params.scan_size.height, params.scan_threshold.vertical);
    border.bottom += detect_border_edge(
        image, outside_mask, (Delta){0, -params.scan_step.vertical},
        params.scan_size.height, params.scan_threshold.vertical);
  }
  verboseLog(VERBOSE_NORMAL,
             "border detected: (%d,%d,%d,%d) in [%d,%d,%d,%d]\n", border.left,
             border.top, border.right, border.bottom, outside_mask.vertex[0].x,
             outside_mask.vertex[0].y, outside_mask.vertex[1].x,
             outside_mask.vertex[1].y);

  return border;
}
