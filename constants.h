// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#define MAX_MULTI_INDEX 10000
#define MAX_MASKS 100
#define MAX_POINTS 100
#define MAX_PAGES 2
#define WHITE 0xFF
#define BLANK_TEXT "<blank>"

typedef enum { WIDTH, HEIGHT, DIMENSIONS_COUNT } DIMENSIONS;

typedef enum { HORIZONTAL, VERTICAL, DIRECTIONS_COUNT } DIRECTIONS;

typedef enum {
  LAYOUT_NONE,
  LAYOUT_SINGLE,
  LAYOUT_DOUBLE,
  LAYOUTS_COUNT
} Layout;
