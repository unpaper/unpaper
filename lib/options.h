// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>

#include "constants.h"
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
} Options;

void options_init(Options *o);

bool parse_rectangle(const char *str, Rectangle *rect);
int print_rectangle(Rectangle rect);
