// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <libavutil/frame.h>
#include <stdint.h>

#include "imageprocess/primitives.h"

uint64_t noisefilter(AVFrame *image, uint64_t intensity,
                     uint8_t min_white_level, uint8_t abs_black_threshold);
