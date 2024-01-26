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

void errOutput(const char *fmt, ...);

#define WHITE_COMPONENT 0xFF
#define BLACK_COMPONENT 0x00

uint8_t pixel_grayscale(Pixel pixel) {
  return (pixel.r + pixel.g + pixel.b) / 3;
}

static Pixel get_pixel_components(Image image, Point coords, uint8_t defval) {
  if ((coords.x < 0) || (coords.x >= image.frame->width) || (coords.y < 0) ||
      (coords.y >= image.frame->height)) {
    return (Pixel){defval, defval, defval};
  }

  return image._get_pixel(image, coords);
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
 *
 * @return color or grayscale-value of the requested pixel, or WHITE_COMPONENT
 * if the coordinates are outside the image
 */
Pixel get_pixel(Image image, Point coords) {
  return get_pixel_components(image, coords, WHITE_COMPONENT);
}

/**
 * Returns the grayscale (=brightness) value of a single pixel.
 *
 * @return grayscale-value of the requested pixel, or WHITE_COMPONENT if the
 * coordinates are outside the image
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
 * WHITE_COMPONENT if the coordinates are outside the image
 */
uint8_t get_pixel_lightness(Image image, Point coords) {
  Pixel p = get_pixel_components(image, coords, WHITE_COMPONENT);
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
 * pixel, or WHITE_COMPONENT if the coordinates are outside the image
 */
uint8_t get_pixel_darkness_inverse(Image image, Point coords) {
  Pixel p = get_pixel_components(image, coords, WHITE_COMPONENT);
  return max3(p.r, p.g, p.b);
}

/**
 * Sets the color/grayscale value of a single pixel.
 *
 * @return true if the pixel has been changed, false if the original color was
 * the one to set
 */
bool set_pixel(Image image, Point coords, Pixel pixel) {
  if ((coords.x < 0) || (coords.x >= image.frame->width) || (coords.y < 0) ||
      (coords.y >= image.frame->height)) {
    return false; // nop
  }

  image._set_pixel(image, coords, pixel);

  return true;
}
