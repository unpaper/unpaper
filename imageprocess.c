// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// Copyright © 2013 Michael McMaster <michael@codesrc.com>
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

/* --- image processing --------------------------------------------------- */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "imageprocess.h"
#include "imageprocess/blit.h"
#include "imageprocess/fill.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"
#include "tools.h"
#include "unpaper.h"

/****************************************************************************
 * image processing functions                                               *
 ****************************************************************************/

static inline bool inMask(int x, int y, const Mask mask) {
  return (x >= mask[LEFT]) && (x <= mask[RIGHT]) && (y >= mask[TOP]) &&
         (y <= mask[BOTTOM]);
}

/**
 * Tests if at least one mask in masks overlaps with m.
 */
static bool masksOverlapAny(const Mask m, const Mask *masks, int masksCount) {
  Rectangle area = maskToRectangle(m);

  for (int i = 0; i < masksCount; i++)
    if (rectangles_overlap(area, maskToRectangle(masks[i])))
      return true;

  return false;
}

/* --- stretching / resizing / shifting ------------------------------------ */

static void stretchTo(AVFrame *source, AVFrame *target) {
  const float xRatio = source->width / (float)target->width;
  const float yRatio = source->height / (float)target->height;

  verboseLog(VERBOSE_MORE, "stretching %dx%d -> %dx%d\n", source->width,
             source->height, target->width, target->height);

  for (int y = 0; y < target->height; y++) {
    for (int x = 0; x < target->width; x++) {
      // calculate average pixel value in source matrix
      const Pixel pxl = interpolate(
          source, (FloatPoint){x * xRatio, y * yRatio}, interpolateType);
      set_pixel(target, (Point){x, y}, pxl, absBlackThreshold);
    }
  }
}

void stretch(int w, int h, AVFrame **image) {
  AVFrame *newimage;

  if ((*image)->width == w && (*image)->height == h)
    return;

  // allocate new buffer's memory
  initImage(&newimage, w, h, (*image)->format, false);

  stretchTo(*image, newimage);

  replaceImage(image, &newimage);
}

/**
 * Resizes the image so that the resulting sheet has a new size and the image
 * content is zoomed to fit best into the sheet, while keeping it's aspect
 * ration.
 *
 * @param w the new width to resize to
 * @param h the new height to resize to
 */
void resize(int w, int h, AVFrame **image) {
  AVFrame *stretched, *resized;
  int ww;
  int hh;
  float wRat = (float)w / (*image)->width;
  float hRat = (float)h / (*image)->height;

  verboseLog(VERBOSE_NORMAL, "resizing %dx%d -> %dx%d\n", (*image)->width,
             (*image)->height, w, h);

  if (wRat < hRat) { // horizontally more shrinking/less enlarging is needed:
                     // fill width fully, adjust height
    ww = w;
    hh = (*image)->height * w / (*image)->width;
  } else if (hRat < wRat) {
    ww = (*image)->width * h / (*image)->height;
    hh = h;
  } else { // wRat == hRat
    ww = w;
    hh = h;
  }
  initImage(&stretched, ww, hh, (*image)->format, true);
  stretchTo(*image, stretched);

  // Check if any centering needs to be done, otherwise make a new
  // copy, center and return that.  Check for the stretched
  // width/height to be the same rather than comparing the ratio, as
  // it is possible that the ratios are just off enough that they
  // generate the same width/height.
  if ((ww == w) && (hh = h)) {
    // don't create one more buffer if the size is the same.
    resized = stretched;
  } else {
    initImage(&resized, w, h, (*image)->format, true);
    centerImage(stretched, 0, 0, w, h, resized);
    av_frame_free(&stretched);
  }
  replaceImage(image, &resized);
}

/**
 * Shifts the image.
 *
 * @param shiftX horizontal shifting
 * @param shiftY vertical shifting
 */
void shift(int shiftX, int shiftY, AVFrame **image) {
  AVFrame *newimage;

  // allocate new buffer's memory
  initImage(&newimage, (*image)->width, (*image)->height, (*image)->format,
            true);

  copy_rectangle(*image, newimage, RECT_FULL_IMAGE, POINT_ORIGIN,
                 absBlackThreshold);
  replaceImage(image, &newimage);
}

/* --- mask-detection ----------------------------------------------------- */

/**
 * Finds one edge of non-black pixels heading from one starting point towards
 * edge direction.
 *
 * @return number of shift-steps until blank edge found
 */
static int detectEdge(int startX, int startY, int shiftX, int shiftY,
                      int maskScanSize, int maskScanDepth,
                      float maskScanThreshold, AVFrame *image) {
  // either shiftX or shiftY is 0, the other value is -i|+i
  int left;
  int top;
  int right;
  int bottom;

  const int half = maskScanSize / 2;
  unsigned int total = 0;
  unsigned int count = 0;

  if (shiftY ==
      0) { // vertical border is to be detected, horizontal shifting of scan-bar
    if (maskScanDepth == -1) {
      maskScanDepth = image->height;
    }
    const int halfDepth = maskScanDepth / 2;
    left = startX - half;
    top = startY - halfDepth;
    right = startX + half;
    bottom = startY + halfDepth;
  } else { // horizontal border is to be detected, vertical shifting of scan-bar
    if (maskScanDepth == -1) {
      maskScanDepth = image->width;
    }
    const int halfDepth = maskScanDepth / 2;
    left = startX - halfDepth;
    top = startY - half;
    right = startX + halfDepth;
    bottom = startY + half;
  }

  while (true) { // !
    const uint8_t blackness = inverse_brightness_rect(
        image, (Rectangle){{{left, top}, {right, bottom}}});
    total += blackness;
    count++;
    // is blackness below threshold*average?
    if ((blackness < ((maskScanThreshold * total) / count)) ||
        (blackness ==
         0)) { // this will surely become true when pos reaches the outside of
               // the actual image area and blacknessRect() will deliver 0
               // because all pixels outside are considered white
      return count; // ! return here, return absolute value of shifting
                    // difference
    }
    left += shiftX;
    right += shiftX;
    top += shiftY;
    bottom += shiftY;
  }
}

/**
 * Detects a mask of white borders around a starting point.
 * The result is returned via call-by-reference parameters left, top, right,
 * bottom.
 *
 * @return the detected mask in left, top, right, bottom; or -1, -1, -1, -1 if
 * no mask could be detected
 */
static bool detectMask(int startX, int startY, int maskScanDirections,
                       const int maskScanSize[DIRECTIONS_COUNT],
                       const int maskScanDepth[DIRECTIONS_COUNT],
                       const int maskScanStep[DIRECTIONS_COUNT],
                       const float maskScanThreshold[DIRECTIONS_COUNT],
                       const int maskScanMinimum[DIMENSIONS_COUNT],
                       const int maskScanMaximum[DIMENSIONS_COUNT], int *left,
                       int *top, int *right, int *bottom, AVFrame *image) {
  int width;
  int height;
  int half[DIRECTIONS_COUNT];
  bool success;

  half[HORIZONTAL] = maskScanSize[HORIZONTAL] / 2;
  half[VERTICAL] = maskScanSize[VERTICAL] / 2;
  if ((maskScanDirections & 1 << HORIZONTAL) != 0) {
    *left = startX -
            maskScanStep[HORIZONTAL] *
                detectEdge(startX, startY, -maskScanStep[HORIZONTAL], 0,
                           maskScanSize[HORIZONTAL], maskScanDepth[HORIZONTAL],
                           maskScanThreshold[HORIZONTAL], image) -
            half[HORIZONTAL];
    *right = startX +
             maskScanStep[HORIZONTAL] *
                 detectEdge(startX, startY, maskScanStep[HORIZONTAL], 0,
                            maskScanSize[HORIZONTAL], maskScanDepth[HORIZONTAL],
                            maskScanThreshold[HORIZONTAL], image) +
             half[HORIZONTAL];
  } else { // full range of sheet
    *left = 0;
    *right = image->width - 1;
  }
  if ((maskScanDirections & 1 << VERTICAL) != 0) {
    *top = startY -
           maskScanStep[VERTICAL] *
               detectEdge(startX, startY, 0, -maskScanStep[VERTICAL],
                          maskScanSize[VERTICAL], maskScanDepth[VERTICAL],
                          maskScanThreshold[VERTICAL], image) -
           half[VERTICAL];
    *bottom = startY +
              maskScanStep[VERTICAL] *
                  detectEdge(startX, startY, 0, maskScanStep[VERTICAL],
                             maskScanSize[VERTICAL], maskScanDepth[VERTICAL],
                             maskScanThreshold[VERTICAL], image) +
              half[VERTICAL];
  } else { // full range of sheet
    *top = 0;
    *bottom = image->height - 1;
  }

  // if below minimum or above maximum, set to maximum
  width = *right - *left;
  height = *bottom - *top;
  success = true;
  if (((maskScanMinimum[WIDTH] != -1) && (width < maskScanMinimum[WIDTH])) ||
      ((maskScanMaximum[WIDTH] != -1) && (width > maskScanMaximum[WIDTH]))) {
    width = maskScanMaximum[WIDTH] / 2;
    *left = startX - width;
    *right = startX + width;
    success = false;
    ;
  }
  if (((maskScanMinimum[HEIGHT] != -1) && (height < maskScanMinimum[HEIGHT])) ||
      ((maskScanMaximum[HEIGHT] != -1) && (height > maskScanMaximum[HEIGHT]))) {
    height = maskScanMaximum[HEIGHT] / 2;
    *top = startY - height;
    *bottom = startY + height;
    success = false;
  }
  return success;
}

/**
 * Detects masks around the points specified in point[].
 *
 * @param mask point to array into which detected masks will be stored
 * @return number of masks stored in mask[][]
 */
void detectMasks(AVFrame *image) {
  int left;
  int top;
  int right;
  int bottom;

  maskCount = 0;
  if (maskScanDirections != 0) {
    for (int i = 0; i < pointCount; i++) {
      maskValid[i] = detectMask(
          point[i][X], point[i][Y], maskScanDirections, maskScanSize,
          maskScanDepth, maskScanStep, maskScanThreshold, maskScanMinimum,
          maskScanMaximum, &left, &top, &right, &bottom, image);
      if (!(left == -1 || top == -1 || right == -1 || bottom == -1)) {
        mask[maskCount][LEFT] = left;
        mask[maskCount][TOP] = top;
        mask[maskCount][RIGHT] = right;
        mask[maskCount][BOTTOM] = bottom;
        maskCount++;
        verboseLog(VERBOSE_NORMAL, "auto-masking (%d,%d): %d,%d,%d,%d",
                   point[i][X], point[i][Y], left, top, right, bottom);
        if (maskValid[i] ==
            false) { // (mask had been auto-set to full page size)
          verboseLog(VERBOSE_NORMAL,
                     " (invalid detection, using full page size)");
        }
        verboseLog(VERBOSE_NORMAL, "\n");
      } else {
        verboseLog(VERBOSE_NORMAL, "auto-masking (%d,%d): NO MASK FOUND\n",
                   point[i][X], point[i][Y]);
      }
    }
  }
}

/**
 * Permanently applies image masks. Each pixel which is not covered by at least
 * one mask is set to maskColor.
 */
void applyMasks(const Mask *masks, const int masksCount, AVFrame *image) {
  if (masksCount <= 0) {
    return;
  }

  Pixel maskPixelColor = pixelValueToPixel(maskColor);
  Rectangle full_image = clip_rectangle(image, RECT_FULL_IMAGE);

  scan_rectangle(full_image) {
    // in any mask?
    bool m = false;
    for (int i = 0; i < masksCount; i++) {
      m = m || inMask(x, y, masks[i]);
    }
    if (m == false) {
      set_pixel(image, (Point){x, y}, maskPixelColor, absBlackThreshold);
    }
  }
}

/* --- wiping ------------------------------------------------------------- */

/**
 * Permanently wipes out areas of an images. Each pixel covered by a wipe-area
 * is set to wipeColor.
 */
void applyWipes(const Mask *area, int areaCount, AVFrame *image) {
  Pixel maskPixelColor = pixelValueToPixel(maskColor);

  for (int i = 0; i < areaCount; i++) {
    uint64_t count = 0;
    Rectangle rect = maskToRectangle(area[i]);

    scan_rectangle(rect) {
      if (set_pixel(image, (Point){x, y}, maskPixelColor, absBlackThreshold)) {
        count++;
      }
    }

    verboseLog(VERBOSE_MORE, "wipe [%d,%d,%d,%d]: %" PRIu64 " pixels\n",
               rect.vertex[0].x, rect.vertex[0].y, rect.vertex[1].x,
               rect.vertex[1].y, count);
  }
}

/* --- mirroring ---------------------------------------------------------- */

/**
 * Mirrors an image either horizontally, vertically, or both.
 */
void mirror(int directions, AVFrame *image) {
  const bool horizontal = !!((directions & 1 << HORIZONTAL) != 0);
  const bool vertical = !!((directions & 1 << VERTICAL) != 0);
  int untilX = ((horizontal == true) && (vertical == false))
                   ? ((image->width - 1) >> 1)
                   : (image->width - 1); // w>>1 == (int)(w-0.5)/2
  int untilY =
      (vertical == true) ? ((image->height - 1) >> 1) : image->height - 1;

  for (int y = 0; y <= untilY; y++) {
    const int yy = (vertical == true) ? (image->height - y - 1) : y;
    if ((vertical == true) && (horizontal == true) &&
        (y ==
         yy)) { // last middle line in odd-lined image mirrored both h and v
      untilX = ((image->width - 1) >> 1);
    }
    for (int x = 0; x <= untilX; x++) {
      const int xx = (horizontal == true) ? (image->width - x - 1) : x;
      Point point1 = {x, y};
      Point point2 = {xx, yy};
      Pixel pixel1 = get_pixel(image, point1);
      Pixel pixel2 = get_pixel(image, point2);
      set_pixel(image, point1, pixel2, absBlackThreshold);
      set_pixel(image, point2, pixel1, absBlackThreshold);
    }
  }
}

/* --- flip-rotating ------------------------------------------------------ */

/**
 * Rotates an image clockwise or anti-clockwise in 90-degrees.
 *
 * @param direction either -1 (rotate anti-clockwise) or 1 (rotate clockwise)
 */
void flipRotate(int direction, AVFrame **image) {
  AVFrame *newimage;

  // exchanged width and height
  initImage(&newimage, (*image)->height, (*image)->width, (*image)->format,
            false);

  for (int y = 0; y < (*image)->height; y++) {
    const int xx = ((direction > 0) ? (*image)->height - 1 : 0) - y * direction;
    for (int x = 0; x < (*image)->width; x++) {
      const int yy =
          ((direction < 0) ? (*image)->width - 1 : 0) + x * direction;

      Point point1 = {x, y};
      Point point2 = {xx, yy};

      set_pixel(newimage, point2, get_pixel(*image, point1), absBlackThreshold);
    }
  }
  replaceImage(image, &newimage);
}

/* --- blackfilter -------------------------------------------------------- */

/**
 * Filters out solidly black areas scanning to one direction.
 *
 * @param stepX is 0 if stepY!=0
 * @param stepY is 0 if stepX!=0
 * @see blackfilter()
 */
static void blackfilterScan(int stepX, int stepY, int size, int dep,
                            unsigned int absBlackfilterScanThreshold,
                            const Mask *exclude, int excludeCount,
                            int intensity, AVFrame *image) {
  int left;
  int top;
  int right;
  int bottom;
  int shiftX;
  int shiftY;
  int l, t, r, b;
  int diffX;
  int diffY;
  bool alreadyExcludedMessage;

  if (stepX != 0) { // horizontal scanning
    left = 0;
    top = 0;
    right = size - 1;
    bottom = dep - 1;
    shiftX = 0;
    shiftY = dep;
  } else { // vertical scanning
    left = 0;
    top = 0;
    right = dep - 1;
    bottom = size - 1;
    shiftX = dep;
    shiftY = 0;
  }
  while (
      (left < image->width) &&
      (top <
       image->height)) { // individual scanning "stripes" over the whole sheet
    l = left;
    t = top;
    r = right;
    b = bottom;
    // make sure last stripe does not reach outside sheet, shift back inside
    // (next +=shift will exit while-loop)
    if (r >= image->width || b >= image->height) {
      diffX = r - image->width + 1;
      diffY = b - image->height + 1;
      l -= diffX;
      t -= diffY;
      r -= diffX;
      b -= diffY;
    }
    alreadyExcludedMessage = false;
    while ((l < image->width) &&
           (t < image->height)) { // single scanning "stripe"
      uint8_t blackness = darkness_rect(image, (Rectangle){{{l, t}, {r, b}}});
      if (blackness >=
          absBlackfilterScanThreshold) { // found a solidly black area
        const Mask mask = {l, t, r, b};
        const Rectangle area = maskToRectangle(mask);
        if (!masksOverlapAny(mask, exclude, excludeCount)) {
          verboseLog(VERBOSE_NORMAL, "black-area flood-fill: [%d,%d,%d,%d]\n",
                     l, t, r, b);
          alreadyExcludedMessage = false;
          // start flood-fill in this area (on each pixel to make sure we get
          // everything, in most cases first flood-fill from first pixel will
          // delete all other black pixels in the area already)
          scan_rectangle(area) {
            flood_fill(image, (Point){x, y}, PIXEL_WHITE, 0, absBlackThreshold,
                       intensity, absBlackThreshold);
          }
        } else {
          if (!alreadyExcludedMessage) {
            verboseLog(VERBOSE_NORMAL, "black-area EXCLUDED: [%d,%d,%d,%d]\n",
                       l, t, r, b);
            alreadyExcludedMessage = true; // do this only once per scan-stripe,
                                           // otherwise too many messages
          }
        }
      }
      l += stepX;
      t += stepY;
      r += stepX;
      b += stepY;
    }
    left += shiftX;
    top += shiftY;
    right += shiftX;
    bottom += shiftY;
  }
}

/**
 * Filters out solidly black areas, as appearing on bad photocopies.
 * A virtual bar of width 'size' and height 'depth' is horizontally moved
 * above the middle of the sheet (or the full sheet, if depth ==-1).
 */
void blackfilter(AVFrame *image) {
  if ((blackfilterScanDirections & 1 << HORIZONTAL) !=
      0) { // left-to-right scan
    blackfilterScan(blackfilterScanStep[HORIZONTAL], 0,
                    blackfilterScanSize[HORIZONTAL],
                    blackfilterScanDepth[HORIZONTAL],
                    absBlackfilterScanThreshold, blackfilterExclude,
                    blackfilterExcludeCount, blackfilterIntensity, image);
  }
  if ((blackfilterScanDirections & 1 << VERTICAL) != 0) { // top-to-bottom scan
    blackfilterScan(0, blackfilterScanStep[VERTICAL],
                    blackfilterScanSize[VERTICAL],
                    blackfilterScanDepth[VERTICAL], absBlackfilterScanThreshold,
                    blackfilterExclude, blackfilterExcludeCount,
                    blackfilterIntensity, image);
  }
}

/* --- blurfilter --------------------------------------------------------- */

/**
 * Removes noise using a kind of blurfilter, as alternative to the noise
 * filter. This algorithm counts pixels while 'shaking' the area to detect,
 * and clears the area if the amount of white pixels exceeds whiteTreshold.
 */
int blurfilter(AVFrame *image) {
  const int blocksPerRow = image->width / blurfilterScanSize[HORIZONTAL];
  const int total = blurfilterScanSize[HORIZONTAL] *
                    blurfilterScanSize[VERTICAL]; // Number of pixels in a block
  int top = 0;
  int right = blurfilterScanSize[HORIZONTAL] - 1;
  int bottom = blurfilterScanSize[VERTICAL] - 1;
  int maxLeft = image->width - blurfilterScanSize[HORIZONTAL];
  int maxTop = image->height - blurfilterScanSize[VERTICAL];
  int result = 0;

  // Number of dark pixels in previous row
  // allocate one extra block left and right
  int *prevCounts = calloc(blocksPerRow + 2, sizeof(int));
  // Number of dark pixels in current row
  int *curCounts = calloc(blocksPerRow + 2, sizeof(int));
  // Number of dark pixels in next row
  int *nextCounts = calloc(blocksPerRow + 2, sizeof(int));

  for (int left = 0, block = 1; left <= maxLeft;
       left += blurfilterScanSize[HORIZONTAL]) {
    curCounts[block] = countPixelsRect(left, top, right, bottom, 0,
                                       absWhiteThreshold, false, image);
    block++;
    right += blurfilterScanSize[HORIZONTAL];
  }
  curCounts[0] = total;
  curCounts[blocksPerRow] = total;
  nextCounts[0] = total;
  nextCounts[blocksPerRow] = total;

  // Loop through all blocks. For a block calculate the number of dark pixels in
  // this block, the number of dark pixels in the block in the top-left corner
  // and similarly for the block in the top-right, bottom-left and bottom-right
  // corner. Take the maximum of these values. Clear the block if this number is
  // not large enough compared to the total number of pixels in a block.
  for (int top = 0; top <= maxTop; top += blurfilterScanSize[HORIZONTAL]) {
    right = blurfilterScanSize[HORIZONTAL] - 1;
    nextCounts[0] =
        countPixelsRect(0, top + blurfilterScanStep[VERTICAL], right,
                        bottom + blurfilterScanSize[VERTICAL], 0,
                        absWhiteThreshold, false, image);

    for (int left = 0, block = 1; left <= maxLeft;
         left += blurfilterScanSize[HORIZONTAL]) {
      // bottom right (has still to be calculated)
      nextCounts[block + 1] =
          countPixelsRect(left + blurfilterScanSize[HORIZONTAL],
                          top + blurfilterScanStep[VERTICAL],
                          right + blurfilterScanSize[HORIZONTAL],
                          bottom + blurfilterScanSize[VERTICAL], 0,
                          absWhiteThreshold, false, image);

      int max = max3(
          nextCounts[block - 1], nextCounts[block + 1],
          max3(prevCounts[block - 1], prevCounts[block + 1], curCounts[block]));

      if ((((float)max) / total) <=
          blurfilterIntensity) { // Not enough dark pixels
        wipe_rectangle(image, (Rectangle){{{left, top}, {right, bottom}}},
                       PIXEL_WHITE, absBlackThreshold);
        result += curCounts[block];
        curCounts[block] = total; // Update information
      }

      right += blurfilterScanSize[HORIZONTAL];
      block++;
    }

    bottom += blurfilterScanSize[VERTICAL];
    // Switch Buffers
    const int *tmpCounts;
    tmpCounts = prevCounts;
    prevCounts = curCounts;
    curCounts = nextCounts;
    nextCounts = (int *)tmpCounts;
  }
  free(prevCounts);
  free(curCounts);
  free(nextCounts);

  return result;
}

/* --- border-detection --------------------------------------------------- */

/**
 * Moves a rectangular area of pixels to be centered above the centerX, centerY
 * coordinates.
 */
void centerMask(AVFrame *image, const int center[COORDINATES_COUNT],
                const Mask mask) {
  AVFrame *newimage;
  const Rectangle area = maskToRectangle(mask);
  const RectangleSize size = size_of_rectangle(area);

  const Point target = {center[X] - size.width / 2,
                        center[Y] - size.height / 2};

  Rectangle new_area = rectangle_from_size(target, size);
  Rectangle clipped_area = clip_rectangle(image, new_area);

  if (memcmp(&new_area, &clipped_area, sizeof(new_area)) == 0) {
    verboseLog(VERBOSE_NORMAL, "centering mask [%d,%d,%d,%d] (%d,%d): %d, %d\n",
               area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
               area.vertex[1].y, center[X], center[Y],
               target.x - area.vertex[0].x, target.y - area.vertex[0].y);
    initImage(&newimage, size.width, size.height, image->format, false);
    copy_rectangle(image, newimage, area, POINT_ORIGIN, absBlackThreshold);
    wipe_rectangle(image, area, sheetBackgroundPixel, absBlackThreshold);
    copy_rectangle(newimage, image, RECT_FULL_IMAGE, target, absBlackThreshold);
    av_frame_free(&newimage);
  } else {
    verboseLog(VERBOSE_NORMAL,
               "centering mask [%d,%d,%d,%d] (%d,%d): %d, %d - NO CENTERING "
               "(would shift area outside visible image)\n",
               area.vertex[0].x, area.vertex[0].y, area.vertex[1].x,
               area.vertex[1].y, center[X], center[Y],
               target.x - area.vertex[0].x, target.y - area.vertex[0].y);
  }
}

/**
 * Moves a rectangular area of pixels to be centered inside a specified area
 * coordinates.
 */
void alignMask(const Mask mask, const Mask outside, AVFrame *image) {
  AVFrame *newimage;
  const Rectangle inside_area = maskToRectangle(mask);
  const RectangleSize inside_size = size_of_rectangle(inside_area);

  Point target;

  if (borderAlign & 1 << LEFT) {
    target.x = outside[LEFT] + borderAlignMargin[HORIZONTAL];
  } else if (borderAlign & 1 << RIGHT) {
    target.x =
        outside[RIGHT] - inside_size.width - borderAlignMargin[HORIZONTAL];
  } else {
    target.x = (outside[LEFT] + outside[RIGHT] - inside_size.width) / 2;
  }
  if (borderAlign & 1 << TOP) {
    target.y = outside[TOP] + borderAlignMargin[VERTICAL];
  } else if (borderAlign & 1 << BOTTOM) {
    target.y =
        outside[BOTTOM] - inside_size.height - borderAlignMargin[VERTICAL];
  } else {
    target.y = (outside[TOP] + outside[BOTTOM] - inside_size.height) / 2;
  }
  verboseLog(VERBOSE_NORMAL, "aligning mask [%d,%d,%d,%d] (%d,%d): %d, %d\n",
             inside_area.vertex[0].x, inside_area.vertex[0].y,
             inside_area.vertex[1].x, inside_area.vertex[1].y, target.x,
             target.y, target.x - inside_area.vertex[0].x,
             target.y - inside_area.vertex[0].y);
  initImage(&newimage, inside_size.width, inside_size.height, image->format,
            true);
  copy_rectangle(image, newimage, inside_area, POINT_ORIGIN, absBlackThreshold);
  wipe_rectangle(image, inside_area, sheetBackgroundPixel, absBlackThreshold);
  copy_rectangle(newimage, image, RECT_FULL_IMAGE, target, absBlackThreshold);
  av_frame_free(&newimage);
}

/**
 * Find the size of one border edge.
 *
 * @param x1..y2 area inside of which border is to be detected
 */
static int detectBorderEdge(const Mask outsideMask, int stepX, int stepY,
                            int size, int threshold, AVFrame *image) {
  int left;
  int top;
  int right;
  int bottom;
  int max;
  int cnt;
  int result;

  if (stepY == 0) { // horizontal detection
    if (stepX > 0) {
      left = outsideMask[LEFT];
      top = outsideMask[TOP];
      right = outsideMask[LEFT] + size;
      bottom = outsideMask[BOTTOM];
    } else {
      left = outsideMask[RIGHT] - size;
      top = outsideMask[TOP];
      right = outsideMask[RIGHT];
      bottom = outsideMask[BOTTOM];
    }
    max = (outsideMask[RIGHT] - outsideMask[LEFT]);
  } else { // vertical detection
    if (stepY > 0) {
      left = outsideMask[LEFT];
      top = outsideMask[TOP];
      right = outsideMask[RIGHT];
      bottom = outsideMask[TOP] + size;
    } else {
      left = outsideMask[LEFT];
      top = outsideMask[BOTTOM] - size;
      right = outsideMask[RIGHT];
      bottom = outsideMask[BOTTOM];
    }
    max = (outsideMask[BOTTOM] - outsideMask[TOP]);
  }
  result = 0;
  while (result < max) {
    cnt = countPixelsRect(left, top, right, bottom, 0, absBlackThreshold, false,
                          image);
    if (cnt >= threshold) {
      return result; // border has been found: regular exit here
    }
    left += stepX;
    top += stepY;
    right += stepX;
    bottom += stepY;
    result += abs(stepX + stepY); // (either stepX or stepY is 0)
  }
  return 0; // no border found between 0..max
}

/**
 * Detects a border of completely non-black pixels around the area
 * outsideBorder[LEFT],outsideBorder[TOP]-outsideBorder[RIGHT],outsideBorder[BOTTOM].
 */
void detectBorder(int border[EDGES_COUNT], const Mask outsideMask,
                  AVFrame *image) {
  border[LEFT] = outsideMask[LEFT];
  border[TOP] = outsideMask[TOP];
  border[RIGHT] = image->width - outsideMask[RIGHT];
  border[BOTTOM] = image->height - outsideMask[BOTTOM];

  if (borderScanDirections & 1 << HORIZONTAL) {
    border[LEFT] += detectBorderEdge(outsideMask, borderScanStep[HORIZONTAL], 0,
                                     borderScanSize[HORIZONTAL],
                                     borderScanThreshold[HORIZONTAL], image);
    border[RIGHT] += detectBorderEdge(outsideMask, -borderScanStep[HORIZONTAL],
                                      0, borderScanSize[HORIZONTAL],
                                      borderScanThreshold[HORIZONTAL], image);
  }
  if (borderScanDirections & 1 << VERTICAL) {
    border[TOP] += detectBorderEdge(outsideMask, 0, borderScanStep[VERTICAL],
                                    borderScanSize[VERTICAL],
                                    borderScanThreshold[VERTICAL], image);
    border[BOTTOM] += detectBorderEdge(
        outsideMask, 0, -borderScanStep[VERTICAL], borderScanSize[VERTICAL],
        borderScanThreshold[VERTICAL], image);
  }
  verboseLog(VERBOSE_NORMAL,
             "border detected: (%d,%d,%d,%d) in [%d,%d,%d,%d]\n", border[LEFT],
             border[TOP], border[RIGHT], border[BOTTOM], outsideMask[LEFT],
             outsideMask[TOP], outsideMask[RIGHT], outsideMask[BOTTOM]);
}

/**
 * Converts a border-tuple to a mask-tuple.
 */
void borderToMask(const int border[EDGES_COUNT], Mask mask, AVFrame *image) {
  mask[LEFT] = border[LEFT];
  mask[TOP] = border[TOP];
  mask[RIGHT] = image->width - border[RIGHT] - 1;
  mask[BOTTOM] = image->height - border[BOTTOM] - 1;
  verboseLog(VERBOSE_DEBUG, "border [%d,%d,%d,%d] -> mask [%d,%d,%d,%d]\n",
             border[LEFT], border[TOP], border[RIGHT], border[BOTTOM],
             mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM]);
}

/**
 * Applies a border to the whole image. All pixels in the border range at the
 * edges of the sheet will be cleared.
 */
void applyBorder(const int border[EDGES_COUNT], AVFrame *image) {
  Mask mask;

  if (border[LEFT] != 0 || border[TOP] != 0 || border[RIGHT] != 0 ||
      border[BOTTOM] != 0) {
    borderToMask(border, mask, image);
    verboseLog(VERBOSE_NORMAL, "applying border (%d,%d,%d,%d) [%d,%d,%d,%d]\n",
               border[LEFT], border[TOP], border[RIGHT], border[BOTTOM],
               mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM]);
    applyMasks(&mask, 1, image);
  }
}
