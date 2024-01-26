// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include "imageprocess/primitives.h"

typedef struct AVFrame AVFrame;

typedef struct Image Image;

typedef Pixel (*get_pixel_cb)(struct Image image, Point coords);
typedef void (*set_pixel_cb)(struct Image image, Point coords, Pixel pixel);

typedef struct Image {
  AVFrame *frame;
  Pixel background;
  uint8_t abs_black_threshold;

  get_pixel_cb _get_pixel;
  set_pixel_cb _set_pixel;
} Image;

#define EMPTY_IMAGE                                                            \
  (Image) { .frame = NULL }

Image create_image(RectangleSize size, int pixel_format, bool fill,
                   Pixel sheet_background, uint8_t abs_black_threshold);
void replace_image(Image *image, Image *new_image);
void free_image(Image *image);
Image create_compatible_image(Image source, RectangleSize size, bool fill);

RectangleSize size_of_image(Image image);
Rectangle full_image(Image image);
Rectangle clip_rectangle(Image image, Rectangle area);
