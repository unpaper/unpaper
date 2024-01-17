// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdint.h>

#include "primitives.h"

void flood_fill(AVFrame *image, Point p, Pixel color, uint8_t mask_min,
                uint8_t mask_max, uint64_t intensity,
                uint8_t abs_black_threshold);
