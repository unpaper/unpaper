// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#define MAX_MULTI_INDEX                                                        \
  10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_ROTATION_SCAN_SIZE                                                 \
  10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_MASKS 100
#define MAX_POINTS 100
#define MAX_FILES 100
#define MAX_PAGES 2
#define WHITE 0xFF
#define GRAY 0x1F
#define BLACK 0x00
#define WHITE24 0xFFFFFF
#define GRAY24 0x1F1F1F
#define BLACK24 0x000000
#define BLANK_TEXT "<blank>"

typedef enum {
  VERBOSE_QUIET = -1,
  VERBOSE_NONE = 0,
  VERBOSE_NORMAL = 1,
  VERBOSE_MORE = 2,
  VERBOSE_DEBUG = 3,
  VERBOSE_DEBUG_SAVE = 4
} VERBOSE_LEVEL;

typedef enum { X, Y, COORDINATES_COUNT } COORDINATES;

typedef enum { WIDTH, HEIGHT, DIMENSIONS_COUNT } DIMENSIONS;

typedef enum { HORIZONTAL, VERTICAL, DIRECTIONS_COUNT } DIRECTIONS;

typedef enum { LEFT, TOP, RIGHT, BOTTOM, EDGES_COUNT } EDGES;

typedef enum {
  LAYOUT_NONE,
  LAYOUT_SINGLE,
  LAYOUT_DOUBLE,
  LAYOUTS_COUNT
} LAYOUTS;

typedef enum {
  INTERP_NN,
  INTERP_LINEAR,
  INTERP_CUBIC,
  INTERP_FUNCTIONS_COUNT
} INTERP_FUNCTIONS;
