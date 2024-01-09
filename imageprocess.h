// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>

#include "constants.h"

/****************************************************************************
 * image processing functions                                               *
 ****************************************************************************/

typedef struct {
  float deskewScanRangeRad;
  float deskewScanStepRad;
  float deskewScanDeviationRad;
} ImageProcessParameters;

ImageProcessParameters imageProcessParameters(float deskewScanRange,
                                              float deskewScanStep,
                                              float deskewScanDeviation);

/* --- deskewing ---------------------------------------------------------- */

float detectRotation(AVFrame *image, Mask mask, const ImageProcessParameters *params);

void rotate(const float radians, AVFrame *source, AVFrame *target);

/* --- stretching / resizing / shifting ------------------------------------ */

void stretch(int w, int h, AVFrame **image);

void resize(int w, int h, AVFrame **image);

void shift(int shiftX, int shiftY, AVFrame **image);

/* --- mask-detection ----------------------------------------------------- */

void detectMasks(AVFrame *image);

void applyMasks(Mask *masks, const int maskCount,
                AVFrame *image);

/* --- wiping ------------------------------------------------------------- */

void applyWipes(Mask *area, int areaCount,
                AVFrame *image);

/* --- mirroring ---------------------------------------------------------- */

void mirror(int directions, AVFrame *image);

/* --- flip-rotating ------------------------------------------------------ */

void flipRotate(int direction, AVFrame **image);

/* --- blackfilter -------------------------------------------------------- */

void blackfilter(AVFrame *image);

/* --- noisefilter -------------------------------------------------------- */

int noisefilter(AVFrame *image);

/* --- blurfilter --------------------------------------------------------- */

int blurfilter(AVFrame *image);

/* --- grayfilter --------------------------------------------------------- */

int grayfilter(AVFrame *image);

/* --- border-detection --------------------------------------------------- */

void centerMask(AVFrame *image, int center[COORDINATES_COUNT],
                Mask mask);

void alignMask(Mask mask, Mask outside, AVFrame *image);

void detectBorder(int border[EDGES_COUNT], Mask outsideMask,
                  AVFrame *image);

void borderToMask(int border[EDGES_COUNT], Mask mask,
                  AVFrame *image);

void applyBorder(int border[EDGES_COUNT], AVFrame *image);
