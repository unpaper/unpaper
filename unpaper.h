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
#include "imageprocess/pixel.h"
#include "lib/logging.h"
#include "lib/math_util.h"

/* --- preprocessor macros ------------------------------------------------ */

#define pluralS(i) ((i > 1) ? "s" : "")

/* --- tool function for file handling ------------------------------------ */

void loadImage(const char *filename, Image *image, Pixel sheet_background,
               uint8_t abs_black_threshold);

void saveImage(char *filename, Image image, int outputPixFmt);

void saveDebug(char *filenameTemplate, int index, Image image)
    __attribute__((format(printf, 1, 0)));

/* --- arithmetic tool functions ------------------------------------------ */

static inline void limit(int *i, int max) {
  if (*i > max) {
    *i = max;
  }
}
