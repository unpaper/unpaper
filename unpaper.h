// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

/* --- global declarations ------------------------------------------------ */

#pragma once

#include <math.h>
#include <stdbool.h>

#include <libavutil/frame.h>

#include "constants.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

/* --- preprocessor macros ------------------------------------------------ */

#define pluralS(i) ((i > 1) ? "s" : "")

/* --- global variable ---------------------------------------------------- */

extern Interpolation interpolateType;

extern Pixel sheetBackgroundPixel;
extern unsigned int absBlackThreshold;
extern unsigned int absWhiteThreshold;
extern unsigned int absBlackfilterScanThreshold;

extern int sheetSize[DIMENSIONS_COUNT];
extern int sheetBackground;
extern int preRotate;
extern int postRotate;
extern int preMirror;
extern int postMirror;
extern int preShift[DIRECTIONS_COUNT];
extern int postShift[DIRECTIONS_COUNT];
extern int size[DIRECTIONS_COUNT];
extern int postSize[DIRECTIONS_COUNT];
extern int stretchSize[DIRECTIONS_COUNT];
extern int postStretchSize[DIRECTIONS_COUNT];
extern float zoomFactor;
extern float postZoomFactor;
extern int pointCount;
extern int point[MAX_POINTS][COORDINATES_COUNT];
extern int maskCount;
extern int mask[MAX_MASKS][EDGES_COUNT];
extern int wipeCount;
extern int wipe[MAX_MASKS][EDGES_COUNT];
extern int middleWipe[2];
extern int preWipeCount;
extern int preWipe[MAX_MASKS][EDGES_COUNT];
extern int postWipeCount;
extern int postWipe[MAX_MASKS][EDGES_COUNT];
extern int preBorder[EDGES_COUNT];
extern int postBorder[EDGES_COUNT];
extern int border[EDGES_COUNT];
extern bool maskValid[MAX_MASKS];
extern int preMaskCount;
extern int preMask[MAX_MASKS][EDGES_COUNT];
extern int blackfilterScanDirections;
extern int blackfilterScanSize[DIRECTIONS_COUNT];
extern int blackfilterScanDepth[DIRECTIONS_COUNT];
extern int blackfilterScanStep[DIRECTIONS_COUNT];
extern float blackfilterScanThreshold;
extern int blackfilterExcludeCount;
extern int blackfilterExclude[MAX_MASKS][EDGES_COUNT];
extern int blackfilterIntensity;
extern int maskScanDirections;
extern int maskScanSize[DIRECTIONS_COUNT];
extern int maskScanDepth[DIRECTIONS_COUNT];
extern int maskScanStep[DIRECTIONS_COUNT];
extern float maskScanThreshold[DIRECTIONS_COUNT];
extern int maskScanMinimum[DIMENSIONS_COUNT];
extern int maskScanMaximum[DIMENSIONS_COUNT]; // set default later
extern int maskColor;
extern int borderScanDirections;
extern int borderScanSize[DIRECTIONS_COUNT];
extern int borderScanStep[DIRECTIONS_COUNT];
extern int borderScanThreshold[DIRECTIONS_COUNT];
extern int borderAlign;                                   // center
extern int borderAlignMargin[DIRECTIONS_COUNT];           // center
extern int outsideBorderscanMask[MAX_PAGES][EDGES_COUNT]; // set by --layout
extern int outsideBorderscanMaskCount;
extern float whiteThreshold;
extern float blackThreshold;
extern bool writeoutput;
extern bool multisheets;

extern int autoborder[MAX_MASKS][EDGES_COUNT];
extern int autoborderMask[MAX_MASKS][EDGES_COUNT];
extern bool overwrite;
extern int dpi;

/* --- tool function for file handling ------------------------------------ */

void loadImage(const char *filename, AVFrame **image);

void saveImage(char *filename, AVFrame *image, int outputPixFmt);

void saveDebug(char *filenameTemplate, int index, AVFrame *image)
    __attribute__((format(printf, 1, 0)));

/* --- arithmetic tool functions ------------------------------------------ */

static inline void limit(int *i, int max) {
  if (*i > max) {
    *i = max;
  }
}

#define red(pixel) ((pixel >> 16) & 0xff)
#define green(pixel) ((pixel >> 8) & 0xff)
#define blue(pixel) (pixel & 0xff)

static inline int pixelValue(uint8_t r, uint8_t g, uint8_t b) {
  return (r) << 16 | (g) << 8 | (b);
}

/* Conversion functions from old types to new types */
static inline Rectangle maskToRectangle(const Mask mask) {
  return (Rectangle){
      .vertex =
          {
              {
                  .x = mask[LEFT],
                  .y = mask[TOP],
              },
              {
                  .x = mask[RIGHT],
                  .y = mask[BOTTOM],
              },
          },
  };
}

static inline Pixel pixelValueToPixel(uint32_t pixelValue) {
  return (Pixel){
      .r = (pixelValue >> 16) & 0xff,
      .g = (pixelValue >> 8) & 0xff,
      .b = pixelValue & 0xff,
  };
}
