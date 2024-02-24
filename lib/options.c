// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "imageprocess/pixel.h"
#include "lib/options.h"

static struct MultiIndex multi_index_empty(void) {
  return (struct MultiIndex){.count = 0, .indexes = NULL};
}

void options_init(Options *o) {
  memset(o, 0, sizeof(Options));

  o->layout = LAYOUT_SINGLE;
  o->start_sheet = 1;
  o->end_sheet = -1;
  o->start_input = -1;
  o->start_output = -1;
  o->input_count = 1;
  o->output_count = 1;

  // default: process all between start-sheet and end-sheet
  // this does not use .count = 0 because we use the -1 as a sentinel for "all
  // sheets".
  o->sheet_multi_index = (struct MultiIndex){.count = -1, .indexes = NULL};

  o->exclude_multi_index = multi_index_empty();
  o->ignore_multi_index = multi_index_empty();
  o->insert_blank = multi_index_empty();
  o->replace_blank = multi_index_empty();

  o->no_blackfilter_multi_index = multi_index_empty();
  o->no_noisefilter_multi_index = multi_index_empty();
  o->no_blurfilter_multi_index = multi_index_empty();
  o->no_grayfilter_multi_index = multi_index_empty();
  o->no_mask_scan_multi_index = multi_index_empty();
  o->no_mask_center_multi_index = multi_index_empty();
  o->no_deskew_multi_index = multi_index_empty();
  o->no_wipe_multi_index = multi_index_empty();
  o->no_border_multi_index = multi_index_empty();
  o->no_border_scan_multi_index = multi_index_empty();
  o->no_border_align_multi_index = multi_index_empty();

  o->pre_shift = (Delta){0, 0};
  o->post_shift = (Delta){0, 0};

  o->sheet_size = (RectangleSize){-1, -1};
  o->page_size = (RectangleSize){-1, -1};
  o->post_page_size = (RectangleSize){-1, -1};
  o->stretch_size = (RectangleSize){-1, -1};
  o->post_stretch_size = (RectangleSize){-1, -1};
}

bool parse_rectangle(const char *str, Rectangle *rect) {
  if (sscanf(str, "%" SCNd32 ",%" SCNd32 ",%" SCNd32 ",%" SCNd32 "",
             &rect->vertex[0].x, &rect->vertex[0].y, &rect->vertex[1].x,
             &rect->vertex[1].y) != 4) {
    return false;
  }

  // only return true if the rectangle is valid!
  return count_pixels(*rect) > 0;
}

int print_rectangle(Rectangle rect) {
  return printf("[%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 "] ",
                rect.vertex[0].x, rect.vertex[0].y, rect.vertex[1].x,
                rect.vertex[1].y);
}

/**
 * Parses either a single integer as a size in pixel, or a pair of two
 * integers, separated by a comma. If the second integer is missing,
 * use the one integer for both width and height.
 */
bool parse_symmetric_integers(const char *str, int32_t *value_1,
                              int32_t *value_2) {
  switch (sscanf(str, "%" SCNd32 ",%" SCNd32 "", value_1, value_2)) {
  case 1:
    // only one value, copy over to the second.
    *value_2 = *value_1;
    // intentional fall-through.
  case 2:
    // both values provided (or fall-through.)
    return true;
  default:
    // not enough / unparseable.
    return false;
  }
}

/**
 * As above, but with floats.
 */
bool parse_symmetric_floats(const char *str, float *value_1, float *value_2) {
  switch (sscanf(str, "%f,%f", value_1, value_2)) {
  case 1:
    // only one value, copy over to the second.
    *value_2 = *value_1;
    // intentional fall-through.
  case 2:
    // both values provided (or fall-through.)
    return true;
  default:
    // not enough / unparseable.
    return false;
  }
}
bool parse_rectangle_size(const char *str, RectangleSize *size) {
  if (!parse_symmetric_integers(str, &size->width, &size->height)) {
    return false;
  }

  // only return true if the size is non-negative!
  return size->width >= 0 && size->height >= 0;
}

int print_rectangle_size(RectangleSize size) {
  return printf("[%" PRId32 ",%" PRId32 "] ", size.width, size.height);
}

bool parse_delta(const char *str, Delta *delta) {
  return parse_symmetric_integers(str, &delta->horizontal, &delta->vertical);
}

/**
 * Special case of parse_delta that validates the delta is positive.
 */
bool parse_scan_step(const char *str, Delta *delta) {
  if (!parse_delta(str, delta)) {
    return false;
  }

  return delta->horizontal > 0 && delta->vertical > 0;
}

int print_delta(Delta delta) {
  return printf("[%" PRId32 ",%" PRId32 "] ", delta.horizontal, delta.vertical);
}

bool parse_border(const char *str, Border *border) {
  if (sscanf(str, "%" SCNd32 ",%" SCNd32 ",%" SCNd32 ",%" SCNd32 "",
             &border->left, &border->top, &border->right,
             &border->bottom) != 4) {
    return false;
  }

  // only return true if the border is valid!
  return (border->left >= 0 && border->top >= 0 && border->right >= 0 &&
          border->bottom >= 0);
}

int print_border(Border border) {
  return printf("[%" PRId32 ",%" PRId32 ",%" PRId32 ",%" PRId32 "] ",
                border.left, border.top, border.right, border.bottom);
}

bool parse_color(const char *str, Pixel *color) {
  if (strcmp(str, "black") == 0) {
    *color = PIXEL_BLACK;
    return true;
  }
  if (strcmp(str, "white") == 0) {
    *color = PIXEL_WHITE;
    return true;
  }

  uint32_t colorValue;
  if (sscanf(str, "%" SCNd32, &colorValue) != 1) {
    return false;
  }

  *color = pixel_from_value(colorValue);
  return true;
}

int print_color(Pixel color) {
  if (compare_pixel(color, PIXEL_BLACK) == 0) {
    return printf("black");
  }
  if (compare_pixel(color, PIXEL_WHITE) == 0) {
    return printf("white");
  }

  return printf("#%02x%02x%02x", color.r, color.g, color.b);
}
