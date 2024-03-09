// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <ctype.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "imageprocess/pixel.h"
#include "lib/options.h"

static struct MultiIndex multi_index_empty(void) {
  return (struct MultiIndex){.count = 0, .indexes = NULL};
}

void options_init(Options *o) {
  *o = (Options){
      .write_output = true,
      .overwrite_output = false,
      .multiple_sheets = true,
      .output_pixel_format = AV_PIX_FMT_NONE,

      .layout = LAYOUT_SINGLE,
      .start_sheet = 1,
      .end_sheet = -1,
      .start_input = -1,
      .start_output = -1,
      .input_count = 1,
      .output_count = 1,

      // default: process all between start-sheet and end-sheet
      // this does not use .count = 0 because we use the -1 as a sentinel for
      // "all sheets".
      .sheet_multi_index = (struct MultiIndex){.count = -1, .indexes = NULL},

      .exclude_multi_index = multi_index_empty(),
      .ignore_multi_index = multi_index_empty(),
      .insert_blank = multi_index_empty(),
      .replace_blank = multi_index_empty(),

      .no_blackfilter_multi_index = multi_index_empty(),
      .no_noisefilter_multi_index = multi_index_empty(),
      .no_blurfilter_multi_index = multi_index_empty(),
      .no_grayfilter_multi_index = multi_index_empty(),
      .no_mask_scan_multi_index = multi_index_empty(),
      .no_mask_center_multi_index = multi_index_empty(),
      .no_deskew_multi_index = multi_index_empty(),
      .no_wipe_multi_index = multi_index_empty(),
      .no_border_multi_index = multi_index_empty(),
      .no_border_scan_multi_index = multi_index_empty(),
      .no_border_align_multi_index = multi_index_empty(),

      .pre_wipes = (Wipes){.count = 0},
      .wipes = (Wipes){.count = 0},
      .post_wipes = (Wipes){.count = 0},

      .pre_shift = (Delta){0, 0},
      .post_shift = (Delta){0, 0},

      .pre_rotate = 0,
      .post_rotate = 0,

      .pre_mirror = DIRECTION_NONE,
      .post_mirror = DIRECTION_NONE,

      .sheet_size = (RectangleSize){-1, -1},
      .page_size = (RectangleSize){-1, -1},
      .post_page_size = (RectangleSize){-1, -1},
      .stretch_size = (RectangleSize){-1, -1},
      .post_stretch_size = (RectangleSize){-1, -1},

      .pre_zoom_factor = 1.0,
      .post_zoom_factor = 1.0,

      .sheet_background = PIXEL_WHITE,
      .mask_color = PIXEL_WHITE,

      .pre_border = BORDER_NULL,
      .border = BORDER_NULL,
      .post_border = BORDER_NULL,

      .interpolate_type = INTERP_CUBIC,
      .noisefilter_intensity = 4,
  };
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

/**
 * Parse, if space is available, a wipe definition into a list of wipes.
 */
bool parse_wipe(const char *optname, const char *str, Wipes *wipes) {
  if (wipes->count >= (sizeof(wipes->areas) / sizeof(wipes->areas[0]))) {
    fprintf(stderr, "%s: maximum number of wipes (%zd) exceeded, ignoring '%s'",
            optname, (sizeof(wipes->areas) / sizeof(wipes->areas[0])), str);
    return false;
  }

  if (!parse_rectangle(str, &wipes->areas[wipes->count])) {
    fprintf(stderr, "%s: invalid wipe definition, ignoring '%s'", optname, str);
    return false;
  }

  wipes->count++;
  return true;
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

bool parse_direction(const char *str, Direction *direction) {
  // This is a bit of a hack, but since there's no 'h' in "vertical", and
  // no "v" in "horizontal", we can assume that if we find either of the
  // two characters, the corresponding direction is selected.
  direction->horizontal = !!(strchr(str, 'h') || strchr(str, 'H'));
  direction->vertical = !!(strchr(str, 'v') || strchr(str, 'V'));

  // if neither direction was selected, the only valid input is "none".
  return direction->horizontal || direction->vertical ||
         strcasecmp(str, "none") == 0;
}

const char *direction_to_string(Direction direction) {
  if (direction.horizontal && direction.vertical) {
    return "[horizontal,vertical]";
  } else if (direction.horizontal) {
    return "[horizontal]";
  } else if (direction.vertical) {
    return "[vertical]";
  } else {
    return "[none]";
  }
}

static bool parse_edge_character(char token, Edges *edges) {
  token = tolower(token);

  if (token == 'l') {
    edges->left = true;
  } else if (token == 't') {
    edges->top = true;
  } else if (token == 'r') {
    edges->right = true;
  } else if (token == 'b') {
    edges->bottom = true;
  } else {
    return false;
  }

  return true;
}

static bool parse_edge_token(const char *token, Edges *edges) {
  // We accept single letter selections (l t r b)
  if (strlen(token) == 1) {
    return parse_edge_character(token[0], edges);
  }

  if (strcasecmp(token, "left") == 0) {
    edges->left = true;
  } else if (strcasecmp(token, "top") == 0) {
    edges->top = true;
  } else if (strcasecmp(token, "right") == 0) {
    edges->right = true;
  } else if (strcasecmp(token, "bottom") == 0) {
    edges->bottom = true;
  } else {
    return false;
  }

  return true;
}

bool parse_edges(const char *str, Edges *edges) {
  // We accept options in two ways: either a single string of
  // up to four characters from the set "ltrb", or comma-separated
  // tokens, which are either the words "left", "top", "right", "bottom"
  // or the "ltrb" characters.

  // First attempt, if there is no comma, parse the single token.
  if (strchr(str, ',') == NULL) {
    // A single edge.
    if (parse_edge_token(str, edges)) {
      return true;
    }

    // Not a single edge, try instead to parse it as a set of
    // characters.
    while (*str && parse_edge_character(*str++, edges))
      ;

    // Return true if we scanned the full string successfully.
    return *str == '\0';
  }

  // Otherwise go ahead and do the tokenization of the string instead.
  char *buffer = strdup(str); // strtok_r writes to the string.
  char *tmp = NULL;

  char *next_token = strtok_r(buffer, ",", &tmp);
  while (next_token != NULL && parse_edge_token(next_token, edges)) {
    next_token = strtok_r(NULL, ",", &tmp);
  }

  free(buffer);

  // return true only if we interrupted because we ran out of tokens.
  return next_token == NULL;
}

int print_edges(Edges edges) {
  if (!edges.left && !edges.top && !edges.right && !edges.bottom) {
    return printf("[none]");
  }

  return printf(
      "[%s%s%s%s%s%s%s]", edges.left ? "left" : "",
      (edges.left && (edges.top || edges.right || edges.bottom)) ? "," : "",
      edges.top ? "top" : "",
      (edges.top && (edges.right || edges.bottom)) ? "," : "",
      edges.right ? "right" : "", (edges.right && edges.bottom) ? "," : "",
      edges.bottom ? "bottom" : "");
}

static const struct {
  const char name[8];
  Layout layout;
} LAYOUTS[] = {
    {"single", LAYOUT_SINGLE},
    {"double", LAYOUT_DOUBLE},
    {"none", LAYOUT_NONE},
};

bool parse_layout(const char *str, Layout *layout) {
  for (size_t j = 0; j < sizeof(LAYOUTS) / sizeof(LAYOUTS[0]); j++) {
    if (strcasecmp(str, LAYOUTS[j].name) == 0) {
      *layout = LAYOUTS[j].layout;
      return true;
    }
  }

  return false;
}

static const struct {
  const char name[8];
  Interpolation interpolation;
} INTERPOLATIONS[] = {
    {"nearest", INTERP_NN},
    {"linear", INTERP_LINEAR},
    {"cubic", INTERP_CUBIC},
};

bool parse_interpolate(const char *str, Interpolation *interpolation) {
  for (size_t j = 0; j < sizeof(INTERPOLATIONS) / sizeof(INTERPOLATIONS[0]);
       j++) {
    if (strcasecmp(str, INTERPOLATIONS[j].name) == 0) {
      *interpolation = INTERPOLATIONS[j].interpolation;
      return true;
    }
  }

  return false;
}
