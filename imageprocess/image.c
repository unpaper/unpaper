// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <libavutil/frame.h>

#include "imageprocess/blit.h"
#include "imageprocess/image.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

static Pixel _get_pixel_gray8(Image image, Point coords) {
  uint8_t *pix =
      image.frame->data[0] + (coords.y * image.frame->linesize[0] + coords.x);
  return (Pixel){*pix, *pix, *pix};
}

static void _set_pixel_gray8(Image image, Point coords, Pixel color) {
  uint8_t *pix =
      image.frame->data[0] + (coords.y * image.frame->linesize[0] + coords.x);
  *pix = pixel_grayscale(color);
}

static Pixel _get_pixel_y400a(Image image, Point coords) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x * 2);
  return (Pixel){*pix, *pix, *pix};
}

static void _set_pixel_y400a(Image image, Point coords, Pixel color) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x * 2);
  pix[0] = pixel_grayscale(color);
  pix[1] = UINT8_MAX; // no alpha.
}

static Pixel _get_pixel_rgb24(Image image, Point coords) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x * 3);
  return (Pixel){
      .r = pix[0],
      .g = pix[1],
      .b = pix[2],
  };
}

static void _set_pixel_rgb24(Image image, Point coords, Pixel color) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x * 3);
  pix[0] = color.r;
  pix[1] = color.g;
  pix[2] = color.b;
}

static Pixel _get_pixel_monowhite(Image image, Point coords) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x / 8);
  if (*pix & (128 >> (coords.x % 8)))
    return PIXEL_BLACK;
  else
    return PIXEL_WHITE;
}

static void _set_pixel_monowhite(Image image, Point coords, Pixel color) {
  uint8_t black = pixel_grayscale(color) < image.abs_black_threshold;

  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x / 8);
  if (black) {
    *pix = *pix | (128 >> (coords.x % 8));
  } else if (black) {
    *pix = *pix & ~(128 >> (coords.x % 8));
  }
}

static Pixel _get_pixel_monoblack(Image image, Point coords) {
  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x / 8);
  if (*pix & (128 >> (coords.x % 8)))
    return PIXEL_WHITE;
  else
    return PIXEL_BLACK;
}

static void _set_pixel_monoblack(Image image, Point coords, Pixel color) {
  uint8_t black = pixel_grayscale(color) < image.abs_black_threshold;

  uint8_t *pix = image.frame->data[0] +
                 (coords.y * image.frame->linesize[0] + coords.x / 8);
  if (!black) {
    *pix = *pix | (128 >> (coords.x % 8));
  } else if (black) {
    *pix = *pix & ~(128 >> (coords.x % 8));
  }
}

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

  switch (pixel_format) {
  case AV_PIX_FMT_GRAY8:
    image._get_pixel = _get_pixel_gray8;
    image._set_pixel = _set_pixel_gray8;
    break;
  case AV_PIX_FMT_Y400A:
    image._get_pixel = _get_pixel_y400a;
    image._set_pixel = _set_pixel_y400a;
    break;
  case AV_PIX_FMT_RGB24:
    image._get_pixel = _get_pixel_rgb24;
    image._set_pixel = _set_pixel_rgb24;
    break;
  case AV_PIX_FMT_MONOWHITE:
    image._get_pixel = _get_pixel_monowhite;
    image._set_pixel = _set_pixel_monowhite;
    break;
  case AV_PIX_FMT_MONOBLACK:
    image._get_pixel = _get_pixel_monoblack;
    image._set_pixel = _set_pixel_monoblack;
    break;
  default:
    errOutput("unknown pixel format 0x%04x.", pixel_format);
  }

  if (fill) {
    wipe_rectangle(image, full_image(image), image.background);
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
