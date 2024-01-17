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

bool setPixel(int pixel, const int x, const int y, AVFrame *image);

int getPixel(int x, int y, AVFrame *image);

void centerImage(AVFrame *source, int toX, int toY, int ww, int hh,
                 AVFrame *target);

int countPixelsRect(int left, int top, int right, int bottom, int minColor,
                    int maxBrightness, bool clear, AVFrame *image);

int countPixelNeighbors(int x, int y, int intensity, int whiteMin,
                        AVFrame *image);

void clearPixelNeighbors(int x, int y, int whiteMin, AVFrame *image);

void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity,
               AVFrame *image);
