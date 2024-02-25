// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#pragma once

#include <stdint.h>

#include "imageprocess/image.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/primitives.h"

typedef struct {
  float deskewScanRangeRad;
  float deskewScanStepRad;
  float deskewScanDeviationRad;
  int deskewScanSize;
  float deskewScanDepth;
  Edges scan_edges;
} DeskewParameters;

bool validate_deskew_parameters(DeskewParameters *params, float deskewScanRange,
                                float deskewScanStep, float deskewScanDeviation,
                                int deskewScanSize, float deskewScanDepth,
                                Edges deskewScanEdges);

float detect_rotation(Image image, Rectangle mask,
                      const DeskewParameters params);

void rotate(Image source, Image target, const float radians,
            Interpolation interpolate_type);
