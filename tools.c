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

/**
 * Counts the number of pixels in a rectangular area whose grayscale
 * values ranges between minColor and maxBrightness. Optionally, the area can
 * get cleared with white color while counting.
 */
int countPixelsRect(int left, int top, int right, int bottom, int minColor,
                    int maxBrightness, bool clear, AVFrame *image) {
  int count = 0;

  for (int y = top; y <= bottom; y++) {
    for (int x = left; x <= right; x++) {
      Point p = {x, y};
      const int pixel = get_pixel_grayscale(image, p);
      if ((pixel >= minColor) && (pixel <= maxBrightness)) {
        if (clear == true) {
          set_pixel(image, p, PIXEL_WHITE, absBlackThreshold);
        }
        count++;
      }
    }
  }
  return count;
}

/**
 * Counts the number of dark pixels around the pixel at (x,y), who have a
 * square-metric distance of 'level' to the (x,y) (thus, testing the values
 * of 9 pixels if level==1, 16 pixels if level==2 and so on).
 * Optionally, the pixels can get cleared after counting.
 */
static int countPixelNeighborsLevel(int x, int y, bool clear, int level,
                                    int whiteMin, AVFrame *image) {
  int count = 0;

  // upper and lower rows
  for (int xx = x - level; xx <= x + level; xx++) {
    Point upper = {xx, y - level}, lower = {xx, y + level};

    // upper row
    uint8_t pixel = get_pixel_lightness(image, upper);
    if (pixel < whiteMin) { // non-light pixel found
      if (clear == true) {
        set_pixel(image, upper, PIXEL_WHITE, absBlackThreshold);
      }
      count++;
    }
    // lower row
    pixel = get_pixel_lightness(image, lower);
    if (pixel < whiteMin) {
      if (clear == true) {
        set_pixel(image, lower, PIXEL_WHITE, absBlackThreshold);
      }
      count++;
    }
  }
  // middle rows
  for (int yy = y - (level - 1); yy <= y + (level - 1); yy++) {
    Point first = {x - level, yy}, last = {x + level, yy};

    // first col
    uint8_t pixel = get_pixel_lightness(image, first);
    if (pixel < whiteMin) {
      if (clear == true) {
        set_pixel(image, first, PIXEL_WHITE, absBlackThreshold);
      }
      count++;
    }
    // last col
    pixel = get_pixel_lightness(image, last);
    if (pixel < whiteMin) {
      if (clear == true) {
        set_pixel(image, last, PIXEL_WHITE, absBlackThreshold);
      }
      count++;
    }
  }
  return count;
}

/**
 * Count all dark pixels in the distance 0..intensity that are directly
 * reachable from the dark pixel at (x,y), without having to cross bright
 * pixels.
 */
int countPixelNeighbors(int x, int y, int intensity, int whiteMin,
                        AVFrame *image) {
  int count = 1; // assume self as set
  int lCount = -1;

  // can finish when one level is completely zero
  for (int level = 1; (lCount != 0) && (level <= intensity); level++) {
    lCount = countPixelNeighborsLevel(x, y, false, level, whiteMin, image);
    count += lCount;
  }
  return count;
}

/**
 * Clears all dark pixels that are directly reachable from the dark pixel at
 * (x,y). This should be called only if it has previously been detected that
 * the amount of pixels to clear will be reasonable small.
 */
void clearPixelNeighbors(int x, int y, int whiteMin, AVFrame *image) {
  int lCount = -1;

  set_pixel(image, (Point){x, y}, PIXEL_WHITE, absBlackThreshold);

  // lCount will become 0, otherwise countPixelNeighbors() would previously have
  // delivered a bigger value (and this here would not have been called)
  for (int level = 1; lCount != 0; level++) {
    lCount = countPixelNeighborsLevel(x, y, true, level, whiteMin, image);
  }
}
