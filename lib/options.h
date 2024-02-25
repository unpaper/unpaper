// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>

#include "constants.h"
#include "imageprocess/masks.h"
#include "imageprocess/primitives.h"
#include "parse.h"

typedef struct {
  LAYOUTS layout;
  int start_sheet;
  int end_sheet;
  int start_input;
  int start_output;
  int input_count;
  int output_count;

  struct MultiIndex sheet_multi_index;
  struct MultiIndex exclude_multi_index;
  struct MultiIndex ignore_multi_index;
  struct MultiIndex insert_blank;
  struct MultiIndex replace_blank;

  // 0: allow all, -1: disable all, n: individual entries
  struct MultiIndex no_blackfilter_multi_index;
  struct MultiIndex no_noisefilter_multi_index;
  struct MultiIndex no_blurfilter_multi_index;
  struct MultiIndex no_grayfilter_multi_index;
  struct MultiIndex no_mask_scan_multi_index;
  struct MultiIndex no_mask_center_multi_index;
  struct MultiIndex no_deskew_multi_index;
  struct MultiIndex no_wipe_multi_index;
  struct MultiIndex no_border_multi_index;
  struct MultiIndex no_border_scan_multi_index;
  struct MultiIndex no_border_align_multi_index;

  Delta pre_shift;
  Delta post_shift;

  RectangleSize sheet_size;
  RectangleSize page_size;
  RectangleSize post_page_size;
  RectangleSize stretch_size;
  RectangleSize post_stretch_size;

  uint8_t abs_black_threshold;
  uint8_t abs_white_threshold;
} Options;

void options_init(Options *o);

bool parse_symmetric_integers(const char *str, int32_t *value_1,
                              int32_t *value_2);
bool parse_symmetric_floats(const char *str, float *value_1, float *value_2);

bool parse_rectangle(const char *str, Rectangle *rect);
int print_rectangle(Rectangle rect);

bool parse_rectangle_size(const char *str, RectangleSize *size);
int print_rectangle_size(RectangleSize size);

bool parse_delta(const char *str, Delta *delta);
bool parse_scan_step(const char *str, Delta *delta);
int print_delta(Delta delta);

bool parse_wipe(const char *optname, const char *str, Wipes *wipes);

bool parse_border(const char *str, Border *rect);
int print_border(Border rect);

bool parse_color(const char *str, Pixel *color);
int print_color(Pixel color);
