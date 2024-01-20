// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <libavutil/avutil.h>
#include <libavutil/pixfmt.h>

#include "imageprocess/blit.h"
#include "imageprocess/pixel.h"
#include "tools.h"
#include "unpaper.h"

/**
 * Allocates a memory block for storing image data and fills the IMAGE-struct
 * with the specified values.
 */
void initImage(AVFrame **image, int width, int height, int pixel_format,
               bool fill) {
  int ret;

  (*image) = av_frame_alloc();
  (*image)->width = width;
  (*image)->height = height;
  (*image)->format = pixel_format;

  ret = av_frame_get_buffer(*image, 8);
  if (ret < 0) {
    char errbuff[1024];
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to allocate buffer: %s", errbuff);
  }

  if (fill) {
    wipe_rectangle(*image, RECT_FULL_IMAGE, sheetBackgroundPixel,
                   absBlackThreshold);
  }
}

/**
 * Centers one area of an image inside an area of another image.
 * If the source area is smaller than the target area, is is equally
 * surrounded by a white border, if it is bigger, it gets equally cropped
 * at the edges.
 */
static void centerImageArea(int x, int y, int w, int h, AVFrame *source,
                            int toX, int toY, int ww, int hh, AVFrame *target) {
  if ((w < ww) || (h < hh)) { // white rest-border will remain, so clear first
    wipe_rectangle(target,
                   (Rectangle){{{toX, toY}, {toX + ww - 1, toY + hh - 1}}},
                   sheetBackgroundPixel, absBlackThreshold);
  }
  if (w < ww) {
    toX += (ww - w) / 2;
  }
  if (h < hh) {
    toY += (hh - h) / 2;
  }
  if (w > ww) {
    x += (w - ww) / 2;
    w = ww;
  }
  if (h > hh) {
    y += (h - hh) / 2;
    h = hh;
  }
  copy_rectangle(source, target, (Rectangle){{{x, y}, {x + w, y + h}}},
                 (Point){toX, toY}, absBlackThreshold);
}

/**
 * Centers a whole image inside an area of another image.
 */
void centerImage(AVFrame *source, int toX, int toY, int ww, int hh,
                 AVFrame *target) {
  centerImageArea(0, 0, source->width, source->height, source, toX, toY, ww, hh,
                  target);
}
