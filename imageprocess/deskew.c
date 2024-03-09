// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <math.h>

#include <libavutil/mathematics.h> // for M_PI

#include "constants.h"
#include "imageprocess/blit.h"
#include "imageprocess/deskew.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"
#include "lib/math_util.h"

// maximum pixel count of virtual line to detect rotation with
#define MAX_ROTATION_SCAN_SIZE 10000

static inline float degreesToRadians(float d) { return d * M_PI / 180.0; }

bool validate_deskew_parameters(DeskewParameters *params, float deskewScanRange,
                                float deskewScanStep, float deskewScanDeviation,
                                int deskewScanSize, float deskewScanDepth,
                                Edges deskewScanEdges) {
  *params = (DeskewParameters){
      .deskewScanRangeRad = degreesToRadians(deskewScanRange),
      .deskewScanStepRad = degreesToRadians(deskewScanStep),
      .deskewScanDeviationRad = degreesToRadians(deskewScanDeviation),
      .deskewScanSize = deskewScanSize,
      .deskewScanDepth = deskewScanDepth,
      .scan_edges = deskewScanEdges};

  return true;
}

/**
 * Returns the maximum peak value that occurs when shifting a rotated virtual
 * line above the image, starting from one edge of an area and moving towards
 * the middle point of the area. The peak value is calculated by the absolute
 * difference in the average blackness of pixels that occurs between two single
 * shifting steps.
 *
 * @param m ascending slope of the virtually shifted (m=tan(angle)). Mind that
 * this is negative for negative radians.
 */
static int detect_edge_rotation_peak(Image image, const Rectangle mask,
                                     const DeskewParameters params, Delta shift,
                                     float m) {
  RectangleSize size = size_of_rectangle(mask);
  int mid;
  int half;
  int sideOffset;
  int outerOffset;
  float X; // unrounded coordinates
  float Y;
  float stepX;
  float stepY;
  int dep;
  int pixel;
  int blackness;
  int lastBlackness = 0;
  int diff = 0;
  int maxDiff = 0;
  int maxBlacknessAbs = 255 * params.deskewScanSize * params.deskewScanDepth;
  int maxDepth;
  int accumulatedBlackness = 0;
  int deskewScanSize = params.deskewScanSize;

  if (shift.vertical == 0) { // horizontal detection
    if (deskewScanSize == -1) {
      deskewScanSize = size.height;
    }
    deskewScanSize = min3(deskewScanSize, MAX_ROTATION_SCAN_SIZE, size.height);

    maxDepth = size.width / 2;
    half = deskewScanSize / 2;
    outerOffset = (int)(fabsf(m) * half);
    mid = size.height / 2;
    sideOffset = shift.horizontal > 0 ? mask.vertex[0].x - outerOffset
                                      : mask.vertex[1].x + outerOffset;
    X = sideOffset + half * m;
    Y = mask.vertex[0].y + mid - half;
    stepX = -m;
    stepY = 1.0;
  } else { // vertical detection
    if (deskewScanSize == -1) {
      deskewScanSize = size.width;
    }
    deskewScanSize = min3(deskewScanSize, MAX_ROTATION_SCAN_SIZE, size.width);
    maxDepth = size.height / 2;
    half = deskewScanSize / 2;
    outerOffset = (int)(fabsf(m) * half);
    mid = size.width / 2;
    sideOffset = shift.vertical > 0 ? mask.vertex[0].x - outerOffset
                                    : mask.vertex[1].x + outerOffset;
    X = mask.vertex[0].x + mid - half;
    Y = sideOffset - (half * m);
    stepX = 1.0;
    stepY = -m; // (line goes upwards for negative degrees)
  }

  Point p[deskewScanSize];

  // fill buffer with coordinates for rotated line in first unshifted position
  for (int lineStep = 0; lineStep < deskewScanSize; lineStep++) {
    p[lineStep].x = (int)X;
    p[lineStep].y = (int)Y;
    X += stepX;
    Y += stepY;
  }

  // now scan for edge, modify coordinates in buffer to shift line into search
  // direction (towards the middle point of the area) stop either when
  // detectMaxDepth steps are shifted, or when diff falls back to less than
  // detectThreshold*maxDiff
  for (dep = 0; (accumulatedBlackness < maxBlacknessAbs) && (dep < maxDepth);
       dep++) {
    // calculate blackness of virtual line
    blackness = 0;
    for (int lineStep = 0; lineStep < deskewScanSize; lineStep++) {
      Point pt = p[lineStep];
      p[lineStep] = shift_point(pt, shift);
      if (point_in_rectangle(pt, mask)) {
        pixel = get_pixel_darkness_inverse(image, pt);
        blackness += (255 - pixel);
      }
    }
    diff = blackness - lastBlackness;
    lastBlackness = blackness;
    if (diff >= maxDiff) {
      maxDiff = diff;
    }
    accumulatedBlackness += blackness;
  }
  if (dep < maxDepth) { // has not terminated only because middle was reached
    return maxDiff;
  } else {
    return 0;
  }
}

/**
 * Detects rotation at one edge of the area specified by left, top, right,
 * bottom. Which of the four edges to take depends on whether shiftX or shiftY
 * is non-zero, and what sign this shifting value has.
 */
static float detect_edge_rotation(Image image, const Rectangle mask,
                                  const DeskewParameters params, Delta shift) {
  // either shiftX or shiftY is 0, the other value is -i|+i
  // depending on shiftX/shiftY the start edge for shifting is determined
  int max_peak = 0;
  float detected_rotation = 0.0;

  // iteratively increase test angle, alternating between +/- sign while
  // increasing absolute value
  for (float rotation = 0.0; rotation <= params.deskewScanRangeRad;
       rotation = (rotation >= 0.0) ? -(rotation + params.deskewScanStepRad)
                                    : -rotation) {
    float m = tanf(rotation);
    int peak = detect_edge_rotation_peak(image, mask, params, shift, m);
    if (peak > max_peak) {
      detected_rotation = rotation;
      max_peak = peak;
    }
  }
  return detected_rotation;
}
/**
 * detect rotation of a whole area.
 * angles between -deskew_scan_range and +deskew_scan_range are scanned, at
 * either the horizontal or vertical edges of the area specified by left, top,
 * right, bottom.
 */
float detect_rotation(Image image, const Rectangle mask,
                      const DeskewParameters params) {
  float rotation[4];
  int count = 0;
  float total;
  float average;
  float deviation;

  if (params.scan_edges.left) {
    // left
    rotation[count] =
        detect_edge_rotation(image, mask, params, DELTA_RIGHTWARD);
    verboseLog(VERBOSE_NORMAL, "detected rotation left: [%d,%d,%d,%d]: %f\n",
               mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
               mask.vertex[1].y, rotation[count]);
    count++;
  }
  if (params.scan_edges.top) {
    // top
    rotation[count] =
        -detect_edge_rotation(image, mask, params, DELTA_DOWNWARD);
    verboseLog(VERBOSE_NORMAL, "detected rotation top: [%d,%d,%d,%d]: %f\n",
               mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
               mask.vertex[1].y, rotation[count]);
    count++;
  }
  if (params.scan_edges.right) {
    // right
    rotation[count] = detect_edge_rotation(image, mask, params, DELTA_LEFTWARD);
    verboseLog(VERBOSE_NORMAL, "detected rotation right: [%d,%d,%d,%d]: %f\n",
               mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
               mask.vertex[1].y, rotation[count]);
    count++;
  }
  if (params.scan_edges.bottom) {
    // bottom
    rotation[count] = -detect_edge_rotation(image, mask, params, DELTA_UPWARD);
    verboseLog(VERBOSE_NORMAL, "detected rotation bottom: [%d,%d,%d,%d]: %f\n",
               mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
               mask.vertex[1].y, rotation[count]);
    count++;
  }

  total = 0.0;
  for (int i = 0; i < count; i++) {
    total += rotation[i];
  }
  average = total / count;
  total = 0.0;
  for (int i = 0; i < count; i++) {
    total += powf(rotation[i] - average, 2);
  }
  deviation = sqrtf(total);
  verboseLog(VERBOSE_NORMAL,
             "rotation average: %f  deviation: %f  rotation-scan-deviation "
             "(maximum): %f  [%d,%d,%d,%d]\n",
             average, deviation, params.deskewScanDeviationRad,
             mask.vertex[0].x, mask.vertex[0].y, mask.vertex[1].x,
             mask.vertex[1].y);
  if (deviation <= params.deskewScanDeviationRad) {
    return average;
  } else {
    verboseLog(VERBOSE_NONE, "out of deviation range - NO ROTATING\n");
    return 0.0;
  }
}

/**
 * Rotates a whole image buffer by the specified radians, around its
 * middle-point. (To rotate parts of an image, extract the part with copyBuffer,
 * rotate, and re-paste with copyBuffer.)
 */
static void rotate(Image source, Image target, const float radians,
                   Interpolation interpolate_type) {
  Rectangle source_area = full_image(source);
  FloatPoint source_center = center_of_rectangle(source_area);

  // create 2D rotation matrix
  const float sinval = sinf(radians);
  const float cosval = cosf(radians);

  scan_rectangle(source_area) {
    const float srcX = source_center.x + (x - source_center.x) * cosval +
                       (y - source_center.y) * sinval;
    const float srcY = source_center.y + (y - source_center.y) * cosval -
                       (x - source_center.x) * sinval;
    const Pixel pxl =
        interpolate(source, (FloatPoint){srcX, srcY}, interpolate_type);
    set_pixel(target, (Point){x, y}, pxl);
  }
}

void deskew(Image source, Rectangle mask, float radians,
            Interpolation interpolate_type) {
  Image source_copy =
      create_compatible_image(source, size_of_rectangle(mask), false);
  Image rotated =
      create_compatible_image(source, size_of_rectangle(mask), true);

  // copy area to rotate into source_copy
  copy_rectangle(source, source_copy,
                 (Rectangle){{mask.vertex[0], POINT_INFINITY}}, POINT_ORIGIN);

  // rotate
  rotate(source_copy, rotated, -radians, interpolate_type);

  // copy result back into whole image
  copy_rectangle(rotated, source, full_image(rotated), mask.vertex[0]);

  free_image(&source_copy);
  free_image(&rotated);
}
