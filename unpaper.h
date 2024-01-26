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

/* --- tool function for file handling ------------------------------------ */

void loadImage(const char *filename, AVFrame **image, Pixel sheet_background,
               uint8_t abs_black_threshold);

void saveImage(char *filename, AVFrame *image, int outputPixFmt,
               Pixel sheet_background, uint8_t abs_black_threshold);

void saveDebug(char *filenameTemplate, int index, AVFrame *image,
               Pixel sheet_background, uint8_t abs_black_threshold)
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

static inline Pixel pixelValueToPixel(uint32_t pixelValue) {
  return (Pixel){
      .r = (pixelValue >> 16) & 0xff,
      .g = (pixelValue >> 8) & 0xff,
      .b = pixelValue & 0xff,
  };
}
