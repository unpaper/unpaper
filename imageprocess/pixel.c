// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdbool.h>
#include <stdint.h>

#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/pixfmt.h>

#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

static inline uint8_t pixel_grayscale(Pixel pixel) {
  return (pixel.r + pixel.g + pixel.b) / 3;
}

static Pixel get_pixel_components(Image image, Point coords) {
  uint8_t *pix;

  if ((coords.x < 0) || (coords.x >= image.frame->width) || (coords.y < 0) ||
      (coords.y >= image.frame->height)) {
    return PIXEL_WHITE;
  }

  switch (image.frame->format) {
  case AV_PIX_FMT_GRAY8:
    pix =
        image.frame->data[0] + (coords.y * image.frame->linesize[0] + coords.x);
    return (Pixel){*pix, *pix, *pix};
  case AV_PIX_FMT_Y400A:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x * 2);
    return (Pixel){*pix, *pix, *pix};
  case AV_PIX_FMT_RGB24:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x * 3);
    return (Pixel){
        .r = pix[0],
        .g = pix[1],
        .b = pix[2],
    };
  case AV_PIX_FMT_MONOWHITE:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x / 8);
    if (*pix & (128 >> (coords.x % 8)))
      return PIXEL_BLACK;
    else
      return PIXEL_WHITE;
  case AV_PIX_FMT_MONOBLACK:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x / 8);
    if (*pix & (128 >> (coords.x % 8)))
      return PIXEL_WHITE;
    else
      return PIXEL_BLACK;
  default:
    errOutput("unknown pixel format.");
    return PIXEL_WHITE; // technically unreachable
  }
}

Pixel pixel_from_value(uint32_t value) {
  return (Pixel){
      .r = (value >> 16) & 0xff,
      .g = (value >> 8) & 0xff,
      .b = value & 0xff,
  };
}

int compare_pixel(Pixel a, Pixel b) {
  if (memcmp(&a, &b, sizeof(Pixel)) == 0) {
    return 0;
  }
  return pixel_grayscale(a) < pixel_grayscale(b) ? -1 : 1;
}

/* Returns the color or grayscale value of a single pixel.
 * Always returns a color-compatible value (which may be interpreted as 8-bit
 * grayscale)
 */
Pixel get_pixel(Image image, Point coords) {
  return get_pixel_components(image, coords);
}

/**
 * Returns the grayscale (=brightness) value of a single pixel.
 */
uint8_t get_pixel_grayscale(Image image, Point coords) {
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
 * UINT8_MAX if the coordinates are outside the image
 */
uint8_t get_pixel_lightness(Image image, Point coords) {
  Pixel p = get_pixel_components(image, coords);
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
 * pixel, or UINT8_MAX if the coordinates are outside the image
 */
uint8_t get_pixel_darkness_inverse(Image image, Point coords) {
  Pixel p = get_pixel_components(image, coords);
  return max3(p.r, p.g, p.b);
}

/**
 * Sets the color/grayscale value of a single pixel.
 *
 * @return true if the pixel has been changed, false if the original color was
 * the one to set
 */
bool set_pixel(Image image, Point coords, Pixel pixel) {
  uint8_t *pix;

  if ((coords.x < 0) || (coords.x >= image.frame->width) || (coords.y < 0) ||
      (coords.y >= image.frame->height)) {
    return false; // nop
  }

  bool pixel_black = pixel_grayscale(pixel) < image.abs_black_threshold;

  switch (image.frame->format) {
  case AV_PIX_FMT_GRAY8:
    pix =
        image.frame->data[0] + (coords.y * image.frame->linesize[0] + coords.x);
    *pix = pixel_grayscale(pixel);
    break;
  case AV_PIX_FMT_Y400A:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x * 2);
    pix[0] = pixel_grayscale(pixel);
    pix[1] = 0xFF; // no alpha.
    break;
  case AV_PIX_FMT_RGB24:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x * 3);
    pix[0] = pixel.r;
    pix[1] = pixel.g;
    pix[2] = pixel.b;
    break;
  case AV_PIX_FMT_MONOWHITE:
    pixel_black = !pixel_black; // reverse compared to following case
  case AV_PIX_FMT_MONOBLACK:
    pix = image.frame->data[0] +
          (coords.y * image.frame->linesize[0] + coords.x / 8);
    if (!pixel_black) {
      *pix = *pix | (128 >> (coords.x % 8));
    } else if (pixel_black) {
      *pix = *pix & ~(128 >> (coords.x % 8));
    }
    break;
  default:
    errOutput("unknown pixel format.");
  }
  return true;
}
