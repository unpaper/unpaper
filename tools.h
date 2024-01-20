// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdbool.h>

#include <libavutil/frame.h>

/* --- tool functions for image handling ---------------------------------- */

void initImage(AVFrame **image, int width, int height, int pixel_format,
               bool fill);

static inline void replaceImage(AVFrame **image, AVFrame **newimage) {
  av_frame_free(image);
  *image = *newimage;
}

void centerImage(AVFrame *source, int toX, int toY, int ww, int hh,
                 AVFrame *target);
