// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <libavutil/frame.h>

#include "imageprocess/blit.h"
#include "imageprocess/image.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

/**
 * Allocates a memory block for storing image data and fills the AVFrame-struct
 * with the specified values.
 */
Image create_image(RectangleSize size, int pixel_format, bool fill,
                   Pixel sheet_background, uint8_t abs_black_threshold) {
  Image image = {
      .frame = av_frame_alloc(),
      .background = sheet_background,
      .abs_black_threshold = abs_black_threshold,
  };

  image.frame->width = size.width;
  image.frame->height = size.height;
  image.frame->format = pixel_format;

  int ret = av_frame_get_buffer(image.frame, 8);
  if (ret < 0) {
    char errbuff[1024];
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to allocate buffer: %s", errbuff);
  }

  if (fill) {
    wipe_rectangle(image, full_image(image), sheet_background,
                   abs_black_threshold);
  }

  return image;
}

void replace_image(Image *image, Image *new_image) {
  free_image(image);
  image->frame = new_image->frame;
  image->background = new_image->background;
  image->abs_black_threshold = new_image->abs_black_threshold;
  new_image->frame = NULL;
}

void free_image(Image *image) { av_frame_free(&image->frame); }

Image create_compatible_image(Image source, RectangleSize size, bool fill) {
  return create_image(size, source.frame->format, fill, source.background,
                      source.abs_black_threshold);
}

RectangleSize size_of_image(Image image) {
  return (RectangleSize){
      .width = image.frame->width,
      .height = image.frame->height,
  };
}

Rectangle full_image(Image image) {
  return rectangle_from_size(POINT_ORIGIN, size_of_image(image));
}

Rectangle clip_rectangle(Image image, Rectangle area) {
  Rectangle normal_area = normalize_rectangle(area);

  return (Rectangle){
      .vertex =
          {
              {
                  .x = max(normal_area.vertex[0].x, 0),
                  .y = max(normal_area.vertex[0].y, 0),
              },
              {
                  .x = min(normal_area.vertex[1].x, (image.frame->width - 1)),
                  .y = min(normal_area.vertex[1].y, (image.frame->height - 1)),
              },
          },
  };
}
