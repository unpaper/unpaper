// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdbool.h>
#include <stdint.h>

#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>

#include "pixel.h"

void errOutput(const char *fmt, ...);

#define WHITE 0xFF
#define BLACK 0x00

#define max3(a, b, c)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    __typeof__(c) _c = (c);                                                    \
    (_a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c));                     \
  })

#define min3(a, b, c)                                                          \
  ({                                                                           \
    __typeof__(a) _a = (a);                                                    \
    __typeof__(b) _b = (b);                                                    \
    __typeof__(c) _c = (c);                                                    \
    (_a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c));                     \
  })

static inline uint8_t pixel_grayscale(Pixel pixel) {
  return (pixel.r + pixel.g + pixel.b) / 3;
}

static Pixel get_pixel_components(AVFrame *image, Point coords,
                                  uint8_t defval) {
  Pixel pixel = {defval, defval, defval};
  uint8_t *pix;

  if ((coords.x < 0) || (coords.x >= image->width) || (coords.y < 0) ||
      (coords.y >= image->height)) {
    return pixel;
  }

  switch (image->format) {
  case AV_PIX_FMT_GRAY8:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x);
    pixel.r = pixel.g = pixel.b = *pix;
    break;
  case AV_PIX_FMT_Y400A:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x * 2);
    pixel.r = pixel.g = pixel.b = *pix;
    break;
  case AV_PIX_FMT_RGB24:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x * 3);
    pixel.r = pix[0];
    pixel.g = pix[1];
    pixel.b = pix[2];
    break;
  case AV_PIX_FMT_MONOWHITE:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x / 8);
    if (*pix & (128 >> (coords.x % 8)))
      pixel.r = pixel.g = pixel.b = BLACK;
    else
      pixel.r = pixel.g = pixel.b = WHITE;
    break;
  case AV_PIX_FMT_MONOBLACK:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x / 8);
    if (*pix & (128 >> (coords.x % 8)))
      pixel.r = pixel.g = pixel.b = WHITE;
    else
      pixel.r = pixel.g = pixel.b = BLACK;
    break;
  default:
    errOutput("unknown pixel format.");
  }

  return pixel;
}

/* Returns the color or grayscale value of a single pixel.
 * Always returns a color-compatible value (which may be interpreted as 8-bit
 * grayscale)
 *
 * @return color or grayscale-value of the requested pixel, or WHITE if the
 * coordinates are outside the image
 */
Pixel get_pixel(AVFrame *image, Point coords) {
  return get_pixel_components(image, coords, WHITE);
}

/**
 * Returns the grayscale (=brightness) value of a single pixel.
 *
 * @return grayscale-value of the requested pixel, or WHITE if the coordinates
 * are outside the image
 */
uint8_t get_pixel_grayscale(AVFrame *image, Point coords) {
  return pixel_grayscale(get_pixel(image, coords));
}

/**
 * Returns the 'lightness' value of a single pixel. For color images, this
 * value denotes the minimum brightness of a single color-component in the
 * total color, which means that any color is considered 'dark' which has
 * either the red, the green or the blue component (or, of course, several
 * of them) set to a high value. In some way, this is a measure how close a
 * color is to white.
 * For grayscale images, this value is equal to the pixel brightness.
 *
 * @return lightness-value (the higher, the lighter) of the requested pixel, or
 * WHITE if the coordinates are outside the image
 */
uint8_t get_pixel_lightness(AVFrame *image, Point coords) {
  Pixel p = get_pixel_components(image, coords, WHITE);
  return min3(p.r, p.g, p.b);
}

/**
 * Returns the 'inverse-darkness' value of a single pixel. For color images,
 * this value denotes the maximum brightness of a single color-component in the
 * total color, which means that any color is considered 'light' which has
 * either the red, the green or the blue component (or, of course, several
 * of them) set to a high value. In some way, this is a measure how far away a
 * color is to black.
 * For grayscale images, this value is equal to the pixel brightness.
 *
 * @return inverse-darkness-value (the LOWER, the darker) of the requested
 * pixel, or WHITE if the coordinates are outside the image
 */
uint8_t get_pixel_darkness_inverse(AVFrame *image, Point coords) {
  Pixel p = get_pixel_components(image, coords, WHITE);
  return max3(p.r, p.g, p.b);
}

/**
 * Sets the color/grayscale value of a single pixel.
 *
 * @return true if the pixel has been changed, false if the original color was
 * the one to set
 */
bool set_pixel(AVFrame *image, Point coords, Pixel pixel,
               uint8_t abs_black_threshold) {
  uint8_t *pix;

  if ((coords.x < 0) || (coords.x >= image->width) || (coords.y < 0) ||
      (coords.y >= image->height)) {
    return false; // nop
  }

  uint8_t pixelbw =
      pixel_grayscale(pixel) < abs_black_threshold ? BLACK : WHITE;

  switch (image->format) {
  case AV_PIX_FMT_GRAY8:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x);
    *pix = pixel_grayscale(pixel);
    break;
  case AV_PIX_FMT_Y400A:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x * 2);
    pix[0] = pixel_grayscale(pixel);
    pix[1] = 0xFF; // no alpha.
    break;
  case AV_PIX_FMT_RGB24:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x * 3);
    pix[0] = pixel.r;
    pix[1] = pixel.g;
    pix[2] = pixel.b;
    break;
  case AV_PIX_FMT_MONOWHITE:
    pixelbw = ~pixelbw; // reverse compared to following case
  case AV_PIX_FMT_MONOBLACK:
    pix = image->data[0] + (coords.y * image->linesize[0] + coords.x / 8);
    if (pixelbw == WHITE) {
      *pix = *pix | (128 >> (coords.x % 8));
    } else if (pixelbw == BLACK) {
      *pix = *pix & ~(128 >> (coords.x % 8));
    }
    break;
  default:
    errOutput("unknown pixel format.");
  }
  return true;
}
