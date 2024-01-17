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

bool setPixel(int pixel, int x, int y, AVFrame *image) {
  Pixel p = {
      .r = (pixel >> 16) & 0xff,
      .g = (pixel >> 8) & 0xff,
      .b = pixel & 0xff,
  };

  return set_pixel(image, (Point){x, y}, p, absBlackThreshold);
}

int getPixel(int x, int y, AVFrame *image) {
  Pixel p = get_pixel(image, (Point){x, y});

  return pixelValue(p.r, p.g, p.b);
}

/**
 * Sets the color/grayscale value of a single pixel to white.
 *
 * @return true if the pixel has been changed, false if the original color was
 * the one to set
 */
static bool clearPixel(int x, int y, AVFrame *image) {
  return set_pixel(image, (Point){x, y}, (Pixel){WHITE, WHITE, WHITE},
                   absBlackThreshold);
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
      const int pixel = get_pixel_grayscale(image, (Point){x, y});
      if ((pixel >= minColor) && (pixel <= maxBrightness)) {
        if (clear == true) {
          clearPixel(x, y, image);
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
    // upper row
    uint8_t pixel = get_pixel_lightness(image, (Point){xx, y - level});
    if (pixel < whiteMin) { // non-light pixel found
      if (clear == true) {
        clearPixel(xx, y - level, image);
      }
      count++;
    }
    // lower row
    pixel = get_pixel_lightness(image, (Point){xx, y + level});
    if (pixel < whiteMin) {
      if (clear == true) {
        clearPixel(xx, y + level, image);
      }
      count++;
    }
  }
  // middle rows
  for (int yy = y - (level - 1); yy <= y + (level - 1); yy++) {
    // first col
    uint8_t pixel = get_pixel_lightness(image, (Point){x - level, yy});
    if (pixel < whiteMin) {
      if (clear == true) {
        clearPixel(x - level, yy, image);
      }
      count++;
    }
    // last col
    pixel = get_pixel_lightness(image, (Point){x + level, yy});
    if (pixel < whiteMin) {
      if (clear == true) {
        clearPixel(x + level, yy, image);
      }
      count++;
    }
  }
  /* old version, not optimized:
  for (yy = y-level; yy <= y+level; yy++) {
      for (xx = x-level; xx <= x+level; xx++) {
          if (abs(xx-x)==level || abs(yy-y)==level) {
              pixel = getPixelLightness(xx, yy, image);
              if (pixel < whiteMin) {
                  if (clear==true) {
                      clearPixel(xx, yy, image);
                  }
                  count++;
              }
          }
      }
  }*/
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

  clearPixel(x, y, image);

  // lCount will become 0, otherwise countPixelNeighbors() would previously have
  // delivered a bigger value (and this here would not have been called)
  for (int level = 1; lCount != 0; level++) {
    lCount = countPixelNeighborsLevel(x, y, true, level, whiteMin, image);
  }
}

/**
 * Solidly fills a line of pixels heading towards a specified direction
 * until color-changes in the pixels to overwrite exceed the 'intensity'
 * parameter.
 *
 * @param stepX either -1 or 1, if stepY is 0, else 0
 * @param stepY either -1 or 1, if stepX is 0, else 0
 */
static int fillLine(int x, int y, int stepX, int stepY, int color,
                    uint8_t maskMin, uint8_t maskMax, int intensity,
                    AVFrame *image) {
  int distance = 0;
  int intensityCount = 1; // first pixel must match, otherwise directly exit
  const int w = image->width;
  const int h = image->height;

  while (true) {
    x += stepX;
    y += stepY;
    uint8_t pixel = get_pixel_grayscale(image, (Point){x, y});
    if ((pixel >= maskMin) && (pixel <= maskMax)) {
      intensityCount = intensity; // reset counter
    } else {
      intensityCount--; // allow maximum of 'intensity' pixels to be bright,
      // until stop
    }
    if ((intensityCount > 0) && (x >= 0) && (x < w) && (y >= 0) && (y < h)) {
      setPixel(color, x, y, image);
      distance++;
    } else {
      return distance; // exit here
    }
  }
}

/**
 * Start flood-filling around the edges of a line which has previously been
 * filled using fillLine(). Here, the flood-fill algorithm performs its
 * indirect recursion.
 *
 * @param stepX either -1 or 1, if stepY is 0, else 0
 * @param stepY either -1 or 1, if stepX is 0, else 0
 * @see fillLine()
 */
static void floodFillAroundLine(int x, int y, int stepX, int stepY,
                                int distance, int color, int maskMin,
                                int maskMax, int intensity, AVFrame *image) {
  for (int d = 0; d < distance; d++) {
    if (stepX != 0) {
      x += stepX;
      floodFill(x, y + 1, color, maskMin, maskMax, intensity,
                image); // indirect recursion
      floodFill(x, y - 1, color, maskMin, maskMax, intensity,
                image); // indirect recursion
    } else {            // stepY != 0
      y += stepY;
      floodFill(x + 1, y, color, maskMin, maskMax, intensity,
                image); // indirect recursion
      floodFill(x - 1, y, color, maskMin, maskMax, intensity,
                image); // indirect recursion
    }
  }
}

/**
 * Flood-fill an area of pixels. (Naive implementation, optimizable.)
 *
 * @see earlier header-declaration to enable indirect recursive calls
 */
void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity,
               AVFrame *image) {
  // is current pixel to be filled?
  uint8_t pixel = get_pixel_grayscale(image, (Point){x, y});
  if ((pixel >= maskMin) && (pixel <= maskMax)) {
    // first, fill a 'cross' (both vertical, horizontal line)
    setPixel(color, x, y, image);
    const int left =
        fillLine(x, y, -1, 0, color, maskMin, maskMax, intensity, image);
    const int top =
        fillLine(x, y, 0, -1, color, maskMin, maskMax, intensity, image);
    const int right =
        fillLine(x, y, 1, 0, color, maskMin, maskMax, intensity, image);
    const int bottom =
        fillLine(x, y, 0, 1, color, maskMin, maskMax, intensity, image);
    // now recurse on each neighborhood-pixel of the cross (most recursions will
    // immediately return)
    floodFillAroundLine(x, y, -1, 0, left, color, maskMin, maskMax, intensity,
                        image);
    floodFillAroundLine(x, y, 0, -1, top, color, maskMin, maskMax, intensity,
                        image);
    floodFillAroundLine(x, y, 1, 0, right, color, maskMin, maskMax, intensity,
                        image);
    floodFillAroundLine(x, y, 0, 1, bottom, color, maskMin, maskMax, intensity,
                        image);
  }
}
