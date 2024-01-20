// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdint.h>

#include "primitives.h"

uint64_t wipe_rectangle(AVFrame *image, Rectangle input_area, Pixel color,
                        uint8_t abs_black_threshold);
void copy_rectangle(AVFrame *source, AVFrame *target, Rectangle source_area,
                    Point target_coords, uint8_t abs_black_threshold);
uint8_t inverse_brightness_rect(AVFrame *image, Rectangle input_area);
uint8_t inverse_lightness_rect(AVFrame *image, Rectangle input_area);
uint8_t darkness_rect(AVFrame *image, Rectangle input_area);
uint64_t count_pixels_within_brightness(AVFrame *image, Rectangle area,
                                        uint8_t min_brightness,
                                        uint8_t max_brightness, bool clear,
                                        uint8_t abs_black_threshold);
