// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include "imageprocess/blit.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/math_util.h"
#include "imageprocess/pixel.h"
#include "lib/logging.h"

/**
 * Wipe a rectangular area of pixels with the defined color.
 * @return The number of pixels actually changed.
 */
uint64_t wipe_rectangle(Image image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  Rectangle area = clip_rectangle(image, input_area);

  scan_rectangle(area) {
    if (set_pixel(image, (Point){x, y}, color, abs_black_threshold)) {
      count++;
    }
  }

  return count;
}

void copy_rectangle(Image source, Image target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold) {
  Rectangle area = clip_rectangle(source, source_area);

  // naive but generic implementation
  for (int32_t sY = area.vertex[0].y, tY = target_coords.y;
       sY <= area.vertex[1].y; sY++, tY++) {
    for (int32_t sX = area.vertex[0].x, tX = target_coords.x;
         sX <= area.vertex[1].x; sX++, tX++) {
      set_pixel(target, (Point){tX, tY}, get_pixel(source, (Point){sX, sY}),
                abs_black_threshold);
    }
  }
}

/**
 * Returns the average brightness of a rectangular area.
 */
uint8_t inverse_brightness_rect(Image image, Rectangle input_area) {
  uint64_t grayscale = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    grayscale += get_pixel_grayscale(image, (Point){x, y});
  }

  return 0xFF - (grayscale / count);
}

/**
 * Returns the inverse average lightness of a rectangular area.
 */
uint8_t inverse_lightness_rect(Image image, Rectangle input_area) {
  uint64_t lightness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    lightness += get_pixel_lightness(image, (Point){x, y});
  }

  return 0xFF - (lightness / count);
}

/**
 * Returns the average darkness of a rectangular area.
 */
uint8_t darkness_rect(Image image, Rectangle input_area) {
  uint64_t darkness = 0;

  Rectangle area = clip_rectangle(image, input_area);
  uint64_t count = count_pixels(area);

  if (count == 0) {
    return 0;
  }

  scan_rectangle(area) {
    darkness += get_pixel_darkness_inverse(image, (Point){x, y});
  }

  return 0xFF - (darkness / count);
}

uint64_t count_pixels_within_brightness(Image image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold) {
  uint64_t count = 0;

  scan_rectangle(area) {
    Point p = {x, y};
    uint8_t brightness = get_pixel_grayscale(image, p);
    if (brightness < min_brightness || brightness > max_brightness) {
      continue;
    }

    if (clear) {
      set_pixel(image, p, PIXEL_WHITE, abs_black_threshold);
    }
    count++;
  }

  return count;
}

/**
 * Centers one area of an image inside an area of another image.
 * If the source area is smaller than the target area, is is equally
 * surrounded by a white border, if it is bigger, it gets equally cropped
 * at the edges.
 */
void center_image(Image source, Image target, Point target_origin,
                  RectangleSize target_size, Pixel sheet_background,
                  uint8_t abs_black_threshold) {
  Point source_origin = POINT_ORIGIN;
  RectangleSize source_size = size_of_image(source);

  if (source_size.width < target_size.width ||
      source_size.height < target_size.height) {
    wipe_rectangle(target, rectangle_from_size(target_origin, target_size),
                   sheet_background, abs_black_threshold);
  }

  if (source_size.width <= target_size.width) {
    target_origin.x += (target_size.width - source_size.width) / 2;
  } else {
    source_origin.x += (source_size.width - target_size.width) / 2;
    source_size.width = target_size.width;
  }
  if (source_size.height <= target_size.height) {
    target_origin.y += (target_size.height - source_size.height) / 2;
  } else {
    source_origin.y += (source_size.height - target_size.height) / 2;
    source_size.height = target_size.height;
  }

  copy_rectangle(source, target,
                 rectangle_from_size(source_origin, source_size), target_origin,
                 abs_black_threshold);
}

static void stretch_frame(Image source, Image target,
                          Interpolation interpolate_type,
                          uint8_t abs_black_threshold) {
  RectangleSize source_size = size_of_image(source),
                target_size = size_of_image(target);
  const float horizontal_ratio =
      (float)source_size.width / (float)target_size.width;
  const float vertical_ratio =
      (float)source_size.height / (float)target_size.height;

  verboseLog(VERBOSE_MORE, "stretching %dx%d -> %dx%d\n", source_size.width,
             source_size.height, target_size.width, target_size.height);

  Rectangle target_area = full_image(target);
  scan_rectangle(target_area) {
    const Point target_coords = {x, y};
    const FloatPoint source_coords = {x * horizontal_ratio, y * vertical_ratio};
    set_pixel(target, target_coords,
              interpolate(source, source_coords, interpolate_type),
              abs_black_threshold);
  }
}

void stretch_and_replace(Image *pImage, RectangleSize size,
                         Interpolation interpolate_type,
                         uint8_t abs_black_threshold) {
  if (compare_sizes(size_of_image(*pImage), size) == 0)
    return;

  Image target = create_compatible_image(*pImage, size, false);

  stretch_frame(*pImage, target, interpolate_type, abs_black_threshold);
  replace_image(pImage, &target);
}

void resize_and_replace(Image *pImage, RectangleSize size,
                        Interpolation interpolate_type, Pixel sheet_background,
                        uint8_t abs_black_threshold) {
  RectangleSize image_size = size_of_image(*pImage);
  if (compare_sizes(image_size, size) == 0)
    return;

  verboseLog(VERBOSE_NORMAL, "resizing %dx%d -> %dx%d\n", image_size.width,
             image_size.height, size.width, size.height);

  const float horizontal_ratio = (float)size.width / (float)image_size.width;
  const float vertical_ratio = (float)size.height / (float)image_size.height;

  RectangleSize stretch_size;
  if (horizontal_ratio < vertical_ratio) {
    // horizontally more shrinking/less enlarging is needed:
    // fill width fully, adjust height
    stretch_size =
        (RectangleSize){size.width, image_size.height * horizontal_ratio};
  } else if (vertical_ratio < horizontal_ratio) {
    stretch_size =
        (RectangleSize){image_size.width * vertical_ratio, size.height};
  } else { // wRat == hRat
    stretch_size = size;
  }
  stretch_and_replace(pImage, stretch_size, interpolate_type,
                      abs_black_threshold);

  // Check if any centering needs to be done, otherwise make a new
  // copy, center and return that.  Check for the stretched
  // width/height to be the same rather than comparing the ratio, as
  // it is possible that the ratios are just off enough that they
  // generate the same width/height.
  if (size.width == stretch_size.width && size.height == stretch_size.height) {
    return;
  }

  Image resized = create_compatible_image(*pImage, size, true);
  center_image(*pImage, resized, POINT_ORIGIN, size, sheet_background,
               abs_black_threshold);
  replace_image(pImage, &resized);
}

void flip_rotate_90(Image *pImage, RotationDirection direction,
                    uint8_t abs_black_threshold) {
  RectangleSize image_size = size_of_image(*pImage);

  // exchanged width and height
  Image newimage = create_compatible_image(
      *pImage,
      (RectangleSize){.width = image_size.height, .height = image_size.width},
      false);

  for (int y = 0; y < image_size.height; y++) {
    const int xx =
        ((direction > 0) ? image_size.height - 1 : 0) - y * direction;
    for (int x = 0; x < image_size.width; x++) {
      const int yy =
          ((direction < 0) ? image_size.width - 1 : 0) + x * direction;

      Point point1 = {x, y};
      Point point2 = {xx, yy};

      set_pixel(newimage, point2, get_pixel(*pImage, point1),
                abs_black_threshold);
    }
  }
  replace_image(pImage, &newimage);
}

void mirror(Image image, bool horizontal, bool vertical,
            uint8_t abs_black_threshold) {
  Rectangle source = {{POINT_ORIGIN, POINT_INFINITY}};
  RectangleSize image_size = size_of_image(image);

  if (horizontal && !vertical) {
    source.vertex[1].x = (image_size.width - 1) / 2;
  }

  if (vertical) {
    source.vertex[1].y = (image_size.height - 1) / 2;
  }

  source = clip_rectangle(image, source);

  // Cannot use scan_rectangle() because of the midpoint turn.
  for (int32_t y = source.vertex[0].y; y <= source.vertex[1].y; y++) {
    int32_t yy = vertical ? image_size.height - y - 1 : y;
    // Special case: the last middle line in odd-lined images that are
    // to be mirrored both horizontally and vertically.
    if (vertical && horizontal && y == yy) {
      source.vertex[1].x = (image_size.width - 1) / 2;
    }

    for (int32_t x = 0; x <= source.vertex[1].x; x++) {
      int32_t xx = horizontal ? image_size.width - x - 1 : x;

      Point point1 = {x, y};
      Point point2 = {xx, yy};
      Pixel pixel1 = get_pixel(image, point1);
      Pixel pixel2 = get_pixel(image, point2);
      set_pixel(image, point1, pixel2, abs_black_threshold);
      set_pixel(image, point2, pixel1, abs_black_threshold);
    }
  }
}

void shift_image(Image *pImage, Delta d, Pixel sheet_background,
                 uint8_t abs_black_threshold) {
  // allocate new buffer's memory
  Image newimage =
      create_compatible_image(*pImage, size_of_image(*pImage), true);

  copy_rectangle(*pImage, newimage, full_image(*pImage),
                 shift_point(POINT_ORIGIN, d), abs_black_threshold);
  replace_image(pImage, &newimage);
}
