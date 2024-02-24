// SPDX-FileCopyrightText: 2024 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "lib/logging.h"
#include "lib/math_util.h"
#include "lib/physical.h"

static int32_t mils_to_pixel(int32_t mils, int16_t ppi) {
  return (int32_t)roundf(mils / 1000.0 * ppi);
}

RectangleSize mils_size_to_pixels(MilsSize size, int16_t ppi) {
  if (!size.physical) {
    return (RectangleSize){
        .width = size.width,
        .height = size.height,
    };
  }

  return (RectangleSize){
      .width = mils_to_pixel(size.width, ppi),
      .height = mils_to_pixel(size.height, ppi),
  };
}

Delta mils_delta_to_pixels(MilsDelta delta, int16_t ppi) {
  if (!delta.physical) {
    return (Delta){
        .horizontal = delta.horizontal,
        .vertical = delta.vertical,
    };
  }

  return (Delta){
      .horizontal = mils_to_pixel(delta.horizontal, ppi),
      .vertical = mils_to_pixel(delta.vertical, ppi),
  };
}

// The official definition of an inch is 25.4mm.
#define IN2MILS 1000.0
#define MM2MILS IN2MILS / 25.4
#define CM2MILS MM2MILS * 10.0

static const struct {
  char unit[8];
  float factor;
} UNIT_TO_MILS[] = {
    {"in", IN2MILS},
    {"cm", CM2MILS},
    {"mm", MM2MILS},
    {"mils", 1.0},
};

static bool parse_physical_dimension(const char *str, int32_t *mils,
                                     bool *physical) {
  char *unit; // actually `const char *`, strtof prototype is incorrect.

  float value = strtof(str, &unit);

  if (fabs(value) == HUGE_VALF || str == unit) {
    errOutput("unable to parse dimension '%s': invalid input", str);
  }

  // If there is no unit, then the value was provided in pixels, rather than
  // in physical unit, so make this known to the caller.
  if (strlen(unit) == 0) {
    *mils = (int32_t)roundf(value);
    *physical = false;
  }

  *physical = true;

  for (size_t j = 0; j < sizeof(UNIT_TO_MILS) / sizeof(UNIT_TO_MILS[0]); j++) {
    if (strcmp(unit, UNIT_TO_MILS[j].unit) == 0) {
      *mils = (int32_t)roundf(value * UNIT_TO_MILS[j].factor);
      return true;
    }
  }

  errOutput("unable to parse dimension '%s': unknown unit %s", str, unit);
  return false;
}

static bool parse_physical_2_dimensions(const char *str, int32_t *mils_1,
                                        int32_t *mils_2, bool *physical) {
  // find comma in size string, if there
  const char *comma = strchr(str, ',');

  if (comma == NULL) {
    if (parse_physical_dimension(str, mils_1, physical)) {
      *mils_2 = *mils_1;
      return true;
    } else {
      return false;
    }
  }

  // If there is a comma we have separate width and height dimensions which are
  // slighlty easier to parse if we copy it in and make it two separate strings.
  size_t comma_position = comma - str;
  char *copy = strdup(str);
  copy[comma_position] = '\0';

  const char *str_1 = &copy[0], *str_2 = &copy[comma_position + 1];

  bool physical_1, physical_2;

  bool result = parse_physical_dimension(str_1, mils_1, &physical_1) &&
                parse_physical_dimension(str_2, mils_2, &physical_2);

  if (result && physical_1 != physical_2) {
    errOutput("unable to parse size %s: mixed physical and logical sizes are "
              "not allowed.",
              str);
    result = false;
  }

  *physical = physical_1 && physical_2;

  free(copy);
  return result;
}

#define MM_SIZE(w, h)                                                          \
  { .width = w * MM2MILS, .height = h * MM2MILS, .physical = true }
#define IN_SIZE(w, h)                                                          \
  { .width = w * IN2MILS, .height = h * IN2MILS, .physical = true }

#define ISO_SIZE_AND_FLIP(name, w, h)                                          \
  {name, MM_SIZE(w, h)}, { name "-landscape", MM_SIZE(h, w) }

#define US_SIZE_AND_FLIP(name, w, h)                                           \
  {name, IN_SIZE(w, h)}, { name "-landscape", IN_SIZE(h, w) }

static const struct {
  char name[24];
  MilsSize size;
} PAPERSIZES[] = {
    // The table is scanned linearly, so put more uncommon paper sizes later in
    // the scan.

    // ISO 216 A-series
    ISO_SIZE_AND_FLIP("a3", 297.0, 420.0),
    ISO_SIZE_AND_FLIP("a4", 210.0, 297.0),
    ISO_SIZE_AND_FLIP("a5", 148.0, 210.0),

    // US paper sizes
    US_SIZE_AND_FLIP("letter", 8.5, 11.0),
    US_SIZE_AND_FLIP("legal", 8.5, 14.0),

    // ISO 216 A-series (cont.)
    ISO_SIZE_AND_FLIP("a0", 841.0, 1189.0),
    ISO_SIZE_AND_FLIP("a1", 594.0, 841.0),
    ISO_SIZE_AND_FLIP("a2", 420.0, 594.0),
    ISO_SIZE_AND_FLIP("a6", 105.0, 148.0),
    ISO_SIZE_AND_FLIP("a7", 74.0, 105.0),
    ISO_SIZE_AND_FLIP("a8", 52.0, 74.0),
    ISO_SIZE_AND_FLIP("a9", 37.0, 52.0),
    ISO_SIZE_AND_FLIP("a10", 26.0, 37.0),
};

bool parse_physical_size(const char *str, MilsSize *size) {
  // First try if the string is a clean paper size name.
  for (size_t j = 0; j < sizeof(PAPERSIZES) / sizeof(PAPERSIZES[0]); j++) {
    if (strcasecmp(str, PAPERSIZES[j].name) == 0) {
      *size = PAPERSIZES[j].size;
      return true;
    }
  }

  // If not expect one (square) or two (rectangle) physical dimensions.
  return parse_physical_2_dimensions(str, &size->width, &size->height,
                                     &size->physical);
}

bool parse_physical_delta(const char *str, MilsDelta *delta) {
  return parse_physical_2_dimensions(str, &delta->horizontal, &delta->vertical,
                                     &delta->physical);
}
