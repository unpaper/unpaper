// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <string.h>

#include "options.h"

static struct MultiIndex multi_index_empty(void) {
  return (struct MultiIndex){.count = 0, .indexes = NULL};
}

void options_init(options *o) {
  memset(o, 0, sizeof(options));

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
}
