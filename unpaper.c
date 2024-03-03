// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

/* --- The main program  -------------------------------------------------- */

#include <assert.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>

#include <libavutil/avutil.h>

#include "imageprocess/blit.h"
#include "imageprocess/deskew.h"
#include "imageprocess/filters.h"
#include "imageprocess/image.h"
#include "imageprocess/interpolate.h"
#include "imageprocess/masks.h"
#include "imageprocess/pixel.h"
#include "lib/options.h"
#include "lib/physical.h"
#include "parse.h"
#include "unpaper.h"
#include "version.h"

#define WELCOME                                                                \
  "unpaper " VERSION_STR "\n"                                                  \
  "License GPLv2: GNU GPL version 2.\n"                                        \
  "This is free software: you are free to change and redistribute it.\n"       \
  "There is NO WARRANTY, to the extent permitted by law.\n"

#define USAGE                                                                  \
  WELCOME "\n"                                                                 \
          "Usage: unpaper [options] <input-file(s)> <output-file(s)>\n"        \
          "\n"                                                                 \
          "Filenames may contain a formatting placeholder starting with '%%' " \
          "to insert a\n"                                                      \
          "page counter for multi-page processing. E.g.: 'scan%%03d.pbm' to "  \
          "process files\n"                                                    \
          "scan001.pbm, scan002.pbm, scan003.pbm etc.\n"                       \
          "\n"                                                                 \
          "See 'man unpaper' for options details\n"                            \
          "Report bugs at https://github.com/unpaper/unpaper/issues\n"

/* --- global variable ---------------------------------------------------- */

// We use these for the "val" field in struct option, for getopt_long_only().
// These are for the options that do not have single characters as short
// options.
//
// The values start at 0x7e because this is above all the values for the
// short-option characters (e.g. 0x7e is '~', but there is no '~" short option,
// so we start with that).
enum LONG_OPTION_VALUES {
  OPT_START_SHEET = 0x7e,
  OPT_END_SHEET,
  OPT_START_INPUT,
  OPT_START_OUTPUT,
  OPT_SHEET_BACKGROUND,
  OPT_PRE_ROTATE,
  OPT_POST_ROTATE,
  OPT_POST_MIRROR,
  OPT_PRE_SHIFT,
  OPT_POST_SHIFT,
  OPT_PRE_MASK,
  OPT_POST_SIZE,
  OPT_STRETCH,
  OPT_POST_STRETCH,
  OPT_POST_ZOOM,
  OPT_PRE_WIPE,
  OPT_POST_WIPE,
  OPT_MIDDLE_WIPE,
  OPT_PRE_BORDER,
  OPT_POST_BORDER,
  OPT_NO_BLACK_FILTER,
  OPT_BLACK_FILTER_SCAN_DIRECTION,
  OPT_BLACK_FILTER_SCAN_SIZE,
  OPT_BLACK_FILTER_SCAN_DEPTH,
  OPT_BLACK_FILTER_SCAN_STEP,
  OPT_BLACK_FILTER_SCAN_THRESHOLD,
  OPT_BLACK_FILTER_SCAN_EXCLUDE,
  OPT_BLACK_FILTER_INTENSITY,
  OPT_NO_NOISE_FILTER,
  OPT_NOISE_FILTER_INTENSITY,
  OPT_NO_BLUR_FILTER,
  OPT_BLUR_FILTER_SIZE,
  OPT_BLUR_FILTER_STEP,
  OPT_BLUR_FILTER_INTENSITY,
  OPT_NO_GRAY_FILTER,
  OPT_GRAY_FILTER_SIZE,
  OPT_GRAY_FILTER_STEP,
  OPT_GRAY_FILTER_THRESHOLD,
  OPT_NO_MASK_SCAN,
  OPT_MASK_SCAN_DIRECTION,
  OPT_MASK_SCAN_SIZE,
  OPT_MASK_SCAN_DEPTH,
  OPT_MASK_SCAN_STEP,
  OPT_MASK_SCAN_THRESHOLD,
  OPT_MASK_SCAN_MINIMUM,
  OPT_MASK_SCAN_MAXIMUM,
  OPT_MASK_COLOR,
  OPT_NO_MASK_CENTER,
  OPT_NO_DESKEW,
  OPT_DESKEW_SCAN_DIRECTION,
  OPT_DESKEW_SCAN_SIZE,
  OPT_DESKEW_SCAN_DEPTH,
  OPT_DESKEW_SCAN_RANGE,
  OPT_DESKEW_SCAN_STEP,
  OPT_DESKEW_SCAN_DEVIATION,
  OPT_NO_BORDER_SCAN,
  OPT_BORDER_SCAN_DIRECTION,
  OPT_BORDER_SCAN_SIZE,
  OPT_BORDER_SCAN_STEP,
  OPT_BORDER_SCAN_THRESHOLD,
  OPT_BORDER_ALIGN,
  OPT_BORDER_MARGIN,
  OPT_NO_BORDER_ALIGN,
  OPT_NO_WIPE,
  OPT_NO_BORDER,
  OPT_INPUT_PAGES,
  OPT_OUTPUT_PAGES,
  OPT_INPUT_FILE_SEQUENCE,
  OPT_OUTPUT_FILE_SEQUENCE,
  OPT_INSERT_BLANK,
  OPT_REPLACE_BLANK,
  OPT_NO_MULTI_PAGES,
  OPT_PPI,
  OPT_OVERWRITE,
  OPT_VERBOSE_MORE,
  OPT_DEBUG,
  OPT_DEBUG_SAVE,
  OPT_INTERPOLATE,
};

/****************************************************************************
 * MAIN()                                                                   *
 ****************************************************************************/

/**
 * The main program.
 */
int main(int argc, char *argv[]) {
  // --- local variables ---
  Options options;

  // The variables in the following block need to stay allocated becuse the
  size_t pointCount = 0;
  Point points[MAX_POINTS];
  size_t maskCount = 0;
  Rectangle masks[MAX_MASKS];
  size_t preMaskCount = 0;
  Rectangle preMasks[MAX_MASKS];
  int32_t middleWipe[2] = {0, 0};
  Rectangle outsideBorderscanMask[MAX_PAGES]; // set by --layout
  size_t outsideBorderscanMaskCount = 0;
  Rectangle blackfilterExclude[MAX_MASKS]; // Required to stay allocated!

  // -------------------------------------------------------------------
  // --- parse parameters                                            ---
  // -------------------------------------------------------------------

  {
    // The following variables are no longer visible after argument parsing
    // is complete.
    float whiteThreshold = 0.9;
    float blackThreshold = 0.33;

    Edges deskewScanEdges = {
        .left = true, .top = false, .right = true, .bottom = false};
    int deskewScanSize = 1500;
    float deskewScanDepth = 0.5;
    float deskewScanRange = 5.0;
    float deskewScanStep = 0.1;
    float deskewScanDeviation = 1.0;
    Direction maskScanDirections = DIRECTION_HORIZONTAL;
    RectangleSize maskScanSize = {50, 50};
    int32_t maskScanDepth[DIRECTIONS_COUNT] = {-1, -1};
    Delta maskScanStep = {5, 5};
    float maskScanThreshold[DIRECTIONS_COUNT] = {0.1, 0.1};
    int maskScanMinimum[DIMENSIONS_COUNT] = {100, 100};
    int maskScanMaximum[DIMENSIONS_COUNT] = {-1, -1}; // set default later
    Direction borderScanDirections = DIRECTION_VERTICAL;
    RectangleSize borderScanSize = {5, 5};
    Delta borderScanStep = {5, 5};
    int32_t borderScanThreshold[DIRECTIONS_COUNT] = {5, 5};
    Edges borderAlign = {
        .left = false, .top = false, .right = false, .bottom = false}; // center
    MilsDelta borderAlignMarginPhysical = {0, 0, false};               // center

    int16_t ppi = 300;
    MilsSize sheetSizePhysical = {-1, -1, false};
    MilsDelta preShiftPhysical = {0, 0, false};
    MilsDelta postShiftPhysical = {0, 0, false};
    MilsSize sizePhysical = {-1, -1, false};
    MilsSize postSizePhysical = {-1, -1, false};
    MilsSize stretchSizePhysical = {-1, -1, false};
    MilsSize postStretchSizePhysical = {-1, -1, false};

    Direction blackfilterScanDirections = DIRECTION_BOTH;
    RectangleSize blackfilterScanSize = {20, 20};
    int32_t blackfilterScanDepth[DIRECTIONS_COUNT] = {500, 500};
    Delta blackfilterScanStep = {5, 5};
    float blackfilterScanThreshold = 0.95;
    size_t blackfilterExcludeCount = 0;
    int blackfilterIntensity = 20;
    RectangleSize blurfilterScanSize = {100, 100};
    Delta blurfilterScanStep = {50, 50};
    float blurfilterIntensity = 0.01;
    RectangleSize grayfilterScanSize = {50, 50};
    Delta grayfilterScanStep = {20, 20};
    float grayfilterThreshold = 0.5;

    options_init(&options);

    int option_index = 0;
    while (true) {
      int c;

      static const struct option long_options[] = {
          {"help", no_argument, NULL, 'h'},
          {"?", no_argument, NULL, 'h'},
          {"version", no_argument, NULL, 'V'},
          {"layout", required_argument, NULL, 'l'},
          {"#", required_argument, NULL, '#'},
          {"sheet", required_argument, NULL, '#'},
          {"start", required_argument, NULL, OPT_START_SHEET},
          {"start-sheet", required_argument, NULL, OPT_START_SHEET},
          {"end", required_argument, NULL, OPT_END_SHEET},
          {"end-sheet", required_argument, NULL, OPT_END_SHEET},
          {"start-input", required_argument, NULL, OPT_START_INPUT},
          {"si", required_argument, NULL, OPT_START_INPUT},
          {"start-output", required_argument, NULL, OPT_START_OUTPUT},
          {"so", required_argument, NULL, OPT_START_OUTPUT},
          {"sheet-size", required_argument, NULL, 'S'},
          {"sheet-background", required_argument, NULL, OPT_SHEET_BACKGROUND},
          {"exclude", optional_argument, NULL, 'x'},
          {"no-processing", required_argument, NULL, 'n'},
          {"pre-rotate", required_argument, NULL, OPT_PRE_ROTATE},
          {"post-rotate", required_argument, NULL, OPT_POST_ROTATE},
          {"pre-mirror", required_argument, NULL, 'M'},
          {"post-mirror", required_argument, NULL, OPT_POST_MIRROR},
          {"pre-shift", required_argument, NULL, OPT_PRE_SHIFT},
          {"post-shift", required_argument, NULL, OPT_POST_SHIFT},
          {"pre-mask", required_argument, NULL, OPT_PRE_MASK},
          {"size", required_argument, NULL, 's'},
          {"post-size", required_argument, NULL, OPT_POST_SIZE},
          {"stretch", required_argument, NULL, OPT_STRETCH},
          {"post-stretch", required_argument, NULL, OPT_POST_STRETCH},
          {"zoom", required_argument, NULL, 'z'},
          {"post-zoom", required_argument, NULL, OPT_POST_ZOOM},
          {"mask-scan-point", required_argument, NULL, 'p'},
          {"mask", required_argument, NULL, 'm'},
          {"wipe", required_argument, NULL, 'W'},
          {"pre-wipe", required_argument, NULL, OPT_PRE_WIPE},
          {"post-wipe", required_argument, NULL, OPT_POST_WIPE},
          {"middle-wipe", required_argument, NULL, OPT_MIDDLE_WIPE},
          {"mw", required_argument, NULL, OPT_MIDDLE_WIPE},
          {"border", required_argument, NULL, 'B'},
          {"pre-border", required_argument, NULL, OPT_PRE_BORDER},
          {"post-border", required_argument, NULL, OPT_POST_BORDER},
          {"no-blackfilter", optional_argument, NULL, OPT_NO_BLACK_FILTER},
          {"blackfilter-scan-direction", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_DIRECTION},
          {"bn", required_argument, NULL, OPT_BLACK_FILTER_SCAN_DIRECTION},
          {"blackfilter-scan-size", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_SIZE},
          {"bs", required_argument, NULL, OPT_BLACK_FILTER_SCAN_SIZE},
          {"blackfilter-scan-depth", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_DEPTH},
          {"bd", required_argument, NULL, OPT_BLACK_FILTER_SCAN_DEPTH},
          {"blackfilter-scan-step", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_STEP},
          {"bp", required_argument, NULL, OPT_BLACK_FILTER_SCAN_STEP},
          {"blackfilter-scan-threshold", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_THRESHOLD},
          {"bt", required_argument, NULL, OPT_BLACK_FILTER_SCAN_THRESHOLD},
          {"blackfilter-scan-exclude", required_argument, NULL,
           OPT_BLACK_FILTER_SCAN_EXCLUDE},
          {"bx", required_argument, NULL, OPT_BLACK_FILTER_SCAN_EXCLUDE},
          {"blackfilter-intensity", required_argument, NULL,
           OPT_BLACK_FILTER_INTENSITY},
          {"bi", required_argument, NULL, OPT_BLACK_FILTER_INTENSITY},
          {"no-noisefilter", optional_argument, NULL, OPT_NO_NOISE_FILTER},
          {"noisefilter-intensity", required_argument, NULL,
           OPT_NOISE_FILTER_INTENSITY},
          {"ni", required_argument, NULL, OPT_NOISE_FILTER_INTENSITY},
          {"no-blurfilter", optional_argument, NULL, OPT_NO_BLUR_FILTER},
          {"blurfilter-size", required_argument, NULL, OPT_BLUR_FILTER_SIZE},
          {"ls", required_argument, NULL, OPT_BLUR_FILTER_SIZE},
          {"blurfilter-step", required_argument, NULL, OPT_BLUR_FILTER_STEP},
          {"lp", required_argument, NULL, OPT_BLUR_FILTER_STEP},
          {"blurfilter-intensity", required_argument, NULL,
           OPT_BLUR_FILTER_INTENSITY},
          {"li", required_argument, NULL, OPT_BLUR_FILTER_INTENSITY},
          {"no-grayfilter", optional_argument, NULL, OPT_NO_GRAY_FILTER},
          {"grayfilter-size", required_argument, NULL, OPT_GRAY_FILTER_SIZE},
          {"gs", required_argument, NULL, OPT_GRAY_FILTER_SIZE},
          {"grayfilter-step", required_argument, NULL, OPT_GRAY_FILTER_STEP},
          {"gp", required_argument, NULL, OPT_GRAY_FILTER_STEP},
          {"grayfilter-threshold", required_argument, NULL,
           OPT_GRAY_FILTER_THRESHOLD},
          {"gt", required_argument, NULL, OPT_GRAY_FILTER_THRESHOLD},
          {"no-mask-scan", optional_argument, NULL, OPT_NO_MASK_SCAN},
          {"mask-scan-direction", required_argument, NULL,
           OPT_MASK_SCAN_DIRECTION},
          {"mn", required_argument, NULL, OPT_MASK_SCAN_DIRECTION},
          {"mask-scan-size", required_argument, NULL, OPT_MASK_SCAN_SIZE},
          {"ms", required_argument, NULL, OPT_MASK_SCAN_SIZE},
          {"mask-scan-depth", required_argument, NULL, OPT_MASK_SCAN_DEPTH},
          {"md", required_argument, NULL, OPT_MASK_SCAN_DEPTH},
          {"mask-scan-step", required_argument, NULL, OPT_MASK_SCAN_STEP},
          {"mp", required_argument, NULL, OPT_MASK_SCAN_STEP},
          {"mask-scan-threshold", required_argument, NULL,
           OPT_MASK_SCAN_THRESHOLD},
          {"mt", required_argument, NULL, OPT_MASK_SCAN_THRESHOLD},
          {"mask-scan-minimum", required_argument, NULL, OPT_MASK_SCAN_MINIMUM},
          {"mm", required_argument, NULL, OPT_MASK_SCAN_MINIMUM},
          {"mask-scan-maximum", required_argument, NULL, OPT_MASK_SCAN_MAXIMUM},
          {"mM", required_argument, NULL, OPT_MASK_SCAN_MAXIMUM},
          {"mask-color", required_argument, NULL, OPT_MASK_COLOR},
          {"mc", required_argument, NULL, OPT_MASK_COLOR},
          {"no-mask-center", optional_argument, NULL, OPT_NO_MASK_CENTER},
          {"no-deskew", optional_argument, NULL, OPT_NO_DESKEW},
          {"deskew-scan-direction", required_argument, NULL,
           OPT_DESKEW_SCAN_DIRECTION},
          {"dn", required_argument, NULL, OPT_DESKEW_SCAN_DIRECTION},
          {"deskew-scan-size", required_argument, NULL, OPT_DESKEW_SCAN_SIZE},
          {"ds", required_argument, NULL, OPT_DESKEW_SCAN_SIZE},
          {"deskew-scan-depth", required_argument, NULL, OPT_DESKEW_SCAN_DEPTH},
          {"dd", required_argument, NULL, OPT_DESKEW_SCAN_DEPTH},
          {"deskew-scan-range", required_argument, NULL, OPT_DESKEW_SCAN_RANGE},
          {"dr", required_argument, NULL, OPT_DESKEW_SCAN_RANGE},
          {"deskew-scan-step", required_argument, NULL, OPT_DESKEW_SCAN_STEP},
          {"dp", required_argument, NULL, OPT_DESKEW_SCAN_STEP},
          {"deskew-scan-deviation", required_argument, NULL,
           OPT_DESKEW_SCAN_DEVIATION},
          {"dv", required_argument, NULL, OPT_DESKEW_SCAN_DEVIATION},
          {"no-border-scan", optional_argument, NULL, OPT_NO_BORDER_SCAN},
          {"border-scan-direction", required_argument, NULL,
           OPT_BORDER_SCAN_DIRECTION},
          {"Bn", required_argument, NULL, OPT_BORDER_SCAN_DIRECTION},
          {"border-scan-size", required_argument, NULL, OPT_BORDER_SCAN_SIZE},
          {"Bs", required_argument, NULL, OPT_BORDER_SCAN_SIZE},
          {"border-scan-step", required_argument, NULL, OPT_BORDER_SCAN_STEP},
          {"Bp", required_argument, NULL, OPT_BORDER_SCAN_STEP},
          {"border-scan-threshold", required_argument, NULL,
           OPT_BORDER_SCAN_THRESHOLD},
          {"Bt", required_argument, NULL, OPT_BORDER_SCAN_THRESHOLD},
          {"border-align", required_argument, NULL, OPT_BORDER_ALIGN},
          {"Ba", required_argument, NULL, OPT_BORDER_ALIGN},
          {"border-margin", required_argument, NULL, OPT_BORDER_MARGIN},
          {"Bm", required_argument, NULL, OPT_BORDER_MARGIN},
          {"no-border-align", optional_argument, NULL, OPT_NO_BORDER_ALIGN},
          {"no-wipe", optional_argument, NULL, OPT_NO_WIPE},
          {"no-border", optional_argument, NULL, OPT_NO_BORDER},
          {"white-threshold", required_argument, NULL, 'w'},
          {"black-threshold", required_argument, NULL, 'b'},
          {"input-pages", required_argument, NULL, OPT_INPUT_PAGES},
          {"ip", required_argument, NULL, OPT_INPUT_PAGES},
          {"output-pages", required_argument, NULL, OPT_OUTPUT_PAGES},
          {"op", required_argument, NULL, OPT_OUTPUT_PAGES},
          {"input-file-sequence", required_argument, NULL,
           OPT_INPUT_FILE_SEQUENCE},
          {"if", required_argument, NULL, OPT_INPUT_FILE_SEQUENCE},
          {"output-file-sequence", required_argument, NULL,
           OPT_OUTPUT_FILE_SEQUENCE},
          {"of", required_argument, NULL, OPT_OUTPUT_FILE_SEQUENCE},
          {"insert-blank", required_argument, NULL, OPT_INSERT_BLANK},
          {"replace-blank", required_argument, NULL, OPT_REPLACE_BLANK},
          {"test-only", no_argument, NULL, 'T'},
          {"no-multi-pages", no_argument, NULL, OPT_NO_MULTI_PAGES},
          {"dpi", required_argument, NULL, OPT_PPI},
          {"ppi", required_argument, NULL, OPT_PPI},
          {"type", required_argument, NULL, 't'},
          {"quiet", no_argument, NULL, 'q'},
          {"overwrite", no_argument, NULL, OPT_OVERWRITE},
          {"verbose", no_argument, NULL, 'v'},
          {"vv", no_argument, NULL, OPT_VERBOSE_MORE},
          {"debug", no_argument, NULL, OPT_DEBUG},
          {"vvv", no_argument, NULL, OPT_DEBUG},
          {"debug-save", no_argument, NULL, OPT_DEBUG_SAVE},
          {"vvvv", no_argument, NULL, OPT_DEBUG_SAVE},
          {"interpolate", required_argument, NULL, OPT_INTERPOLATE},
          {NULL, no_argument, NULL, 0}};

      c = getopt_long_only(argc, argv, "hVl:S:x::n::M:s:z:p:m:W:B:w:b:Tt:qv",
                           long_options, &option_index);
      if (c == -1)
        break;

      switch (c) {
      case 'h':
      case '?':
        puts(USAGE);
        return c == '?' ? 1 : 0;

      case 'V':
        puts(VERSION_STR);
        return 0;

      case 'l':
        if (!parse_layout(optarg, &options.layout)) {
          errOutput("unable to parse layout: '%s'", optarg);
        }
        break;

      case '#':
        parseMultiIndex(optarg, &options.sheet_multi_index);
        // allow 0 as start sheet, might be overwritten by --start-sheet again
        if (options.sheet_multi_index.count > 0 &&
            options.start_sheet > options.sheet_multi_index.indexes[0])
          options.start_sheet = options.sheet_multi_index.indexes[0];
        break;

      case OPT_START_SHEET:
        sscanf(optarg, "%d", &options.start_sheet);
        break;

      case OPT_END_SHEET:
        sscanf(optarg, "%d", &options.end_sheet);
        break;

      case OPT_START_INPUT:
        sscanf(optarg, "%d", &options.start_input);
        break;

      case OPT_START_OUTPUT:
        sscanf(optarg, "%d", &options.start_output);
        break;

      case 'S':
        parse_physical_size(optarg, &sheetSizePhysical);
        break;

      case OPT_SHEET_BACKGROUND:
        if (!parse_color(optarg, &options.sheet_background)) {
          errOutput("invalid value for sheet-background: '%s'", optarg);
        }
        break;

      case 'x':
        parseMultiIndex(optarg, &options.exclude_multi_index);
        if (options.exclude_multi_index.count == -1)
          options.exclude_multi_index.count = 0; // 'exclude all' makes no sense
        break;

      case 'n':
        parseMultiIndex(optarg, &options.ignore_multi_index);
        break;

      case OPT_PRE_ROTATE:
        sscanf(optarg, "%hd", &options.pre_rotate);
        if ((options.pre_rotate != 0) && (abs(options.pre_rotate) != 90)) {
          fprintf(stderr, "cannot set --pre-rotate value other than -90 or 90, "
                          "ignoring.\n");
          options.pre_rotate = 0;
        }
        break;

      case OPT_POST_ROTATE:
        sscanf(optarg, "%hd", &options.post_rotate);
        if ((options.post_rotate != 0) && (abs(options.post_rotate) != 90)) {
          fprintf(stderr, "cannot set --post-rotate value other than -90 or "
                          "90, ignoring.\n");
          options.post_rotate = 0;
        }
        break;

      case 'M':
        if (!parse_direction(optarg, &options.pre_mirror)) {
          errOutput("unable to parse pre-mirror directions: '%s'", optarg);
        };
        break;

      case OPT_POST_MIRROR:
        if (!parse_direction(optarg, &options.post_mirror)) {
          errOutput("unable to parse post-mirror directions: '%s'", optarg);
        }
        break;

      case OPT_PRE_SHIFT:
        parse_physical_delta(optarg, &preShiftPhysical);
        break;

      case OPT_POST_SHIFT:
        parse_physical_delta(optarg, &postShiftPhysical);
        break;

      case OPT_PRE_MASK:
        if (preMaskCount < MAX_MASKS) {
          if (parse_rectangle(optarg, &preMasks[preMaskCount])) {
            preMaskCount++;
          }
        } else {
          fprintf(stderr,
                  "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                  MAX_MASKS, optarg);
        }
        break;

      case 's':
        parse_physical_size(optarg, &sizePhysical);
        break;

      case OPT_POST_SIZE:
        parse_physical_size(optarg, &postSizePhysical);
        break;

      case OPT_STRETCH:
        parse_physical_size(optarg, &stretchSizePhysical);
        break;

      case OPT_POST_STRETCH:
        parse_physical_size(optarg, &postStretchSizePhysical);
        break;

      case 'z':
        sscanf(optarg, "%f", &options.pre_zoom_factor);
        break;

      case OPT_POST_ZOOM:
        sscanf(optarg, "%f", &options.post_zoom_factor);
        break;

      case 'p':
        if (pointCount < MAX_POINTS) {
          int x = -1;
          int y = -1;
          sscanf(optarg, "%d,%d", &x, &y);
          points[pointCount++] = (Point){x, y};
        } else {
          fprintf(stderr,
                  "maximum number of scan points (%d) exceeded, ignoring scan "
                  "point %s\n",
                  MAX_POINTS, optarg);
        }
        break;

      case 'm':
        if (maskCount < MAX_MASKS) {
          if (parse_rectangle(optarg, &masks[maskCount])) {
            maskCount++;
          }
        } else {
          fprintf(stderr,
                  "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                  MAX_MASKS, optarg);
        }
        break;

      case 'W':
        parse_wipe("wipe", optarg, options.wipes);
        break;

      case OPT_PRE_WIPE:
        parse_wipe("pre-wipe", optarg, options.pre_wipes);
        break;

      case OPT_POST_WIPE:
        parse_wipe("post-wipe", optarg, options.post_wipes);
        break;

      case OPT_MIDDLE_WIPE:
        if (!parse_symmetric_integers(optarg, &middleWipe[0], &middleWipe[1])) {
          errOutput("unable to parse middle-wipe: '%s'", optarg);
        }
        break;

      case 'B':
        if (!parse_border(optarg, &options.border)) {
          errOutput("unable to parse border: '%s'", optarg);
        }
        break;

      case OPT_PRE_BORDER:
        if (!parse_border(optarg, &options.pre_border)) {
          errOutput("unable to parse pre-border: '%s'", optarg);
        }
        break;

      case OPT_POST_BORDER:
        if (!parse_border(optarg, &options.post_border)) {
          errOutput("unable to parse post-border: '%s'", optarg);
        }
        break;

      case OPT_NO_BLACK_FILTER:
        parseMultiIndex(optarg, &options.no_blackfilter_multi_index);
        break;

      case OPT_BLACK_FILTER_SCAN_DIRECTION:
        if (!parse_direction(optarg, &blackfilterScanDirections)) {
          errOutput("unable to parse blackfilter-scan-direction: '%s'", optarg);
        }
        break;

      case OPT_BLACK_FILTER_SCAN_SIZE:
        if (!parse_rectangle_size(optarg, &blackfilterScanSize)) {
          errOutput("unable to parse blackfilter-scan-size: '%s'", optarg);
        }
        break;

      case OPT_BLACK_FILTER_SCAN_DEPTH:
        if (!parse_symmetric_integers(optarg, &blackfilterScanDepth[0],
                                      &blackfilterScanDepth[1]) ||
            blackfilterScanDepth[0] <= 0 || blackfilterScanDepth[1] <= 0) {
          errOutput("unable to parse blackfilter-scan-depth: '%s'", optarg);
        }
        break;

      case OPT_BLACK_FILTER_SCAN_STEP:
        if (!parse_scan_step(optarg, &blackfilterScanStep)) {
          errOutput("unable to parse blackfilter-scan-step: '%s'", optarg);
        }
        break;

      case OPT_BLACK_FILTER_SCAN_THRESHOLD:
        sscanf(optarg, "%f", &blackfilterScanThreshold);
        break;

      case OPT_BLACK_FILTER_SCAN_EXCLUDE:
        if (blackfilterExcludeCount < MAX_MASKS) {
          if (parse_rectangle(optarg,
                              &blackfilterExclude[blackfilterExcludeCount])) {
            blackfilterExcludeCount++;
          }
        } else {
          fprintf(stderr,
                  "maximum number of blackfilter exclusion (%d) exceeded, "
                  "ignoring mask %s\n",
                  MAX_MASKS, optarg);
        }
        break;

      case OPT_BLACK_FILTER_INTENSITY:
        sscanf(optarg, "%d", &blackfilterIntensity);
        break;

      case OPT_NO_NOISE_FILTER:
        parseMultiIndex(optarg, &options.no_noisefilter_multi_index);
        break;

      case OPT_NOISE_FILTER_INTENSITY:
        sscanf(optarg, "%" SCNu64, &options.noisefilter_intensity);
        break;

      case OPT_NO_BLUR_FILTER:
        parseMultiIndex(optarg, &options.no_blurfilter_multi_index);
        break;

      case OPT_BLUR_FILTER_SIZE:
        if (!parse_rectangle_size(optarg, &blurfilterScanSize)) {
          errOutput("unable to parse blurfilter-scan-size: '%s'", optarg);
        }
        break;

      case OPT_BLUR_FILTER_STEP:
        if (!parse_scan_step(optarg, &blurfilterScanStep)) {
          errOutput("unable to parse blurfilter-scan-step: '%s'", optarg);
        }
        break;

      case OPT_BLUR_FILTER_INTENSITY:
        sscanf(optarg, "%f", &blurfilterIntensity);
        break;

      case OPT_NO_GRAY_FILTER:
        parseMultiIndex(optarg, &options.no_grayfilter_multi_index);
        break;

      case OPT_GRAY_FILTER_SIZE:
        if (!parse_rectangle_size(optarg, &grayfilterScanSize)) {
          errOutput("unable to parse grayfilter-scan-size: '%s'", optarg);
        }
        break;

      case OPT_GRAY_FILTER_STEP:
        if (!parse_scan_step(optarg, &grayfilterScanStep)) {
          errOutput("unable to parse grayfilter-scan-step: '%s'", optarg);
        }
        break;

      case OPT_GRAY_FILTER_THRESHOLD:
        sscanf(optarg, "%f", &grayfilterThreshold);
        break;

      case OPT_NO_MASK_SCAN:
        parseMultiIndex(optarg, &options.no_mask_scan_multi_index);
        break;

      case OPT_MASK_SCAN_DIRECTION:
        if (!parse_direction(optarg, &maskScanDirections)) {
          errOutput("unable to parse mask-scan-direction: '%s'", optarg);
        }
        break;

      case OPT_MASK_SCAN_SIZE:
        if (!parse_rectangle_size(optarg, &maskScanSize)) {
          errOutput("unable to parse mask-scan-size: '%s'", optarg);
        }
        break;

      case OPT_MASK_SCAN_DEPTH:
        if (!parse_symmetric_integers(optarg, &maskScanDepth[0],
                                      &maskScanDepth[1]) ||
            maskScanDepth[0] <= 0 || maskScanDepth[1] <= 0) {
          errOutput("unable to parse mask-scan-depth: '%s'", optarg);
        }
        break;

      case OPT_MASK_SCAN_STEP:
        if (!parse_scan_step(optarg, &maskScanStep)) {
          errOutput("unable to parse mask-scan-step");
        }
        break;

      case OPT_MASK_SCAN_THRESHOLD:
        if (!parse_symmetric_floats(optarg, &maskScanThreshold[0],
                                    &maskScanThreshold[1]) ||
            maskScanThreshold[0] <= 0 || maskScanThreshold[1] <= 0) {
          errOutput("unable to parse mask-scan-threshold: '%s'", optarg);
        }
        break;

      case OPT_MASK_SCAN_MINIMUM:
        sscanf(optarg, "%d,%d", &maskScanMinimum[WIDTH],
               &maskScanMinimum[HEIGHT]);
        break;

      case OPT_MASK_SCAN_MAXIMUM:
        sscanf(optarg, "%d,%d", &maskScanMaximum[WIDTH],
               &maskScanMaximum[HEIGHT]);
        break;

      case OPT_MASK_COLOR:
        if (!parse_color(optarg, &options.mask_color)) {
          errOutput("invalid value for mask-color: '%s'", optarg);
        }
        break;

      case OPT_NO_MASK_CENTER:
        parseMultiIndex(optarg, &options.no_mask_center_multi_index);
        break;

      case OPT_NO_DESKEW:
        parseMultiIndex(optarg, &options.no_deskew_multi_index);
        break;

      case OPT_DESKEW_SCAN_DIRECTION:
        if (!parse_edges(optarg, &deskewScanEdges)) {
          errOutput("uanble to parse deskew-scan-direction: '%s'", optarg);
        }
        break;

      case OPT_DESKEW_SCAN_SIZE:
        sscanf(optarg, "%d", &deskewScanSize);
        break;

      case OPT_DESKEW_SCAN_DEPTH:
        sscanf(optarg, "%f", &deskewScanDepth);
        break;

      case OPT_DESKEW_SCAN_RANGE:
        sscanf(optarg, "%f", &deskewScanRange);
        break;

      case OPT_DESKEW_SCAN_STEP:
        sscanf(optarg, "%f", &deskewScanStep);
        break;

      case OPT_DESKEW_SCAN_DEVIATION:
        sscanf(optarg, "%f", &deskewScanDeviation);
        break;

      case OPT_NO_BORDER_SCAN:
        parseMultiIndex(optarg, &options.no_border_scan_multi_index);
        break;

      case OPT_BORDER_SCAN_DIRECTION:
        if (!parse_direction(optarg, &borderScanDirections)) {
          errOutput("unable to parse border-scan-direction: '%s'", optarg);
        }
        break;

      case OPT_BORDER_SCAN_SIZE:
        if (!parse_rectangle_size(optarg, &borderScanSize)) {
          errOutput("unable to parse border-scan-size: '%s'", optarg);
        }
        break;

      case OPT_BORDER_SCAN_STEP:
        if (!parse_scan_step(optarg, &borderScanStep)) {
          errOutput("unable to parse border-scan-step: '%s'", optarg);
        }
        break;

      case OPT_BORDER_SCAN_THRESHOLD:
        if (!parse_symmetric_integers(optarg, &borderScanThreshold[0],
                                      &borderScanThreshold[1]) ||
            borderScanThreshold[0] <= 0 || borderScanThreshold <= 0) {
          errOutput("unable to parse border-scan-threshold: '%s'", optarg);
        }
        break;

      case OPT_BORDER_ALIGN:
        if (!parse_edges(optarg, &borderAlign)) {
          errOutput("unable to parse border-align: '%s'", optarg);
        }
        break;

      case OPT_BORDER_MARGIN:
        parse_physical_delta(optarg, &borderAlignMarginPhysical);
        break;

      case OPT_NO_BORDER_ALIGN:
        parseMultiIndex(optarg, &options.no_border_align_multi_index);
        break;

      case OPT_NO_WIPE:
        parseMultiIndex(optarg, &options.no_wipe_multi_index);
        break;

      case OPT_NO_BORDER:
        parseMultiIndex(optarg, &options.no_border_multi_index);
        break;

      case 'w':
        sscanf(optarg, "%f", &whiteThreshold);
        break;

      case 'b':
        sscanf(optarg, "%f", &blackThreshold);
        break;

      case OPT_INPUT_PAGES:
        sscanf(optarg, "%d", &options.input_count);
        if (!(options.input_count >= 1 && options.input_count <= 2)) {
          fprintf(
              stderr,
              "cannot set --input-pages value other than 1 or 2, ignoring.\n");
          options.input_count = 1;
        }

        break;

      case OPT_OUTPUT_PAGES:
        sscanf(optarg, "%d", &options.output_count);
        if (!(options.output_count >= 1 && options.output_count <= 2)) {
          fprintf(
              stderr,
              "cannot set --output-pages value other than 1 or 2, ignoring.\n");
          options.output_count = 1;
        }

        break;

      case OPT_INPUT_FILE_SEQUENCE:
      case OPT_OUTPUT_FILE_SEQUENCE:
        errOutput(
            "--input-file-sequence and --output-file-sequence are deprecated "
            "and "
            "unimplemented.\n"
            "Please pass input output pairs as arguments to unpaper instead.");
        break;

      case OPT_INSERT_BLANK:
        parseMultiIndex(optarg, &options.insert_blank);
        break;

      case OPT_REPLACE_BLANK:
        parseMultiIndex(optarg, &options.replace_blank);
        break;

      case 'T':
        options.write_output = false;
        break;

      case OPT_NO_MULTI_PAGES:
        options.multiple_sheets = false;
        break;

      case OPT_PPI:
        sscanf(optarg, "%hd", &ppi);
        break;

      case 't':
        if (strcmp(optarg, "pbm") == 0) {

          options.output_pixel_format = AV_PIX_FMT_MONOWHITE;
        } else if (strcmp(optarg, "pgm") == 0) {
          options.output_pixel_format = AV_PIX_FMT_GRAY8;
        } else if (strcmp(optarg, "ppm") == 0) {
          options.output_pixel_format = AV_PIX_FMT_RGB24;
        }
        break;

      case 'q':
        verbose = VERBOSE_QUIET;
        break;

      case OPT_OVERWRITE:
        options.overwrite_output = true;
        break;

      case 'v':
        verbose = VERBOSE_NORMAL;
        break;

      case OPT_VERBOSE_MORE:
        verbose = VERBOSE_MORE;
        break;

      case OPT_DEBUG:
        verbose = VERBOSE_DEBUG;
        break;

      case OPT_DEBUG_SAVE:
        verbose = VERBOSE_DEBUG_SAVE;
        break;

      case OPT_INTERPOLATE:
        if (!parse_interpolate(optarg, &options.interpolate_type)) {
          errOutput("unable to parse interpolate: '%s'", optarg);
        }
        break;
      }
    }

    // Expand any physical size to their pixel equivalents.
    options.pre_shift = mils_delta_to_pixels(preShiftPhysical, ppi);
    options.post_shift = mils_delta_to_pixels(postShiftPhysical, ppi);

    options.sheet_size = mils_size_to_pixels(sheetSizePhysical, ppi);
    options.page_size = mils_size_to_pixels(sizePhysical, ppi);
    options.post_page_size = mils_size_to_pixels(postSizePhysical, ppi);
    options.stretch_size = mils_size_to_pixels(stretchSizePhysical, ppi);
    options.post_stretch_size =
        mils_size_to_pixels(postStretchSizePhysical, ppi);

    // Calculate the constant absolute values based on the relative parameters.
    options.abs_black_threshold = WHITE * (1.0 - blackThreshold);
    options.abs_white_threshold = WHITE * (whiteThreshold);

    if (!validate_deskew_parameters(&options.deskew_parameters, deskewScanRange,
                                    deskewScanStep, deskewScanDeviation,
                                    deskewScanSize, deskewScanDepth,
                                    deskewScanEdges)) {
      errOutput("deskew parameters are not valid.");
    }
    if (!validate_mask_detection_parameters(
            &options.mask_detection_parameters, maskScanDirections,
            maskScanSize, maskScanDepth, maskScanStep, maskScanThreshold,
            maskScanMinimum, maskScanMaximum)) {
      errOutput("mask detection parameters are not valid.");
    }
    if (!validate_mask_alignment_parameters(
            &options.mask_alignment_parameters, borderAlign,
            mils_delta_to_pixels(borderAlignMarginPhysical, ppi))) {
      errOutput("mask alignment parameters are not valid.");
    };
    if (!validate_border_scan_parameters(&options.border_scan_parameters,
                                         borderScanDirections, borderScanSize,
                                         borderScanStep, borderScanThreshold)) {
      errOutput("border scan parameters are not valid.");
    };
    if (!validate_grayfilter_parameters(&options.grayfilter_parameters,
                                        grayfilterScanSize, grayfilterScanStep,
                                        grayfilterThreshold)) {
      errOutput("grayfilter parameters are not valid.");
    }
    if (!validate_blackfilter_parameters(
            &options.blackfilter_parameters, blackfilterScanSize,
            blackfilterScanStep, blackfilterScanDepth[HORIZONTAL],
            blackfilterScanDepth[VERTICAL], blackfilterScanDirections,
            blackfilterScanThreshold, blackfilterIntensity,
            blackfilterExcludeCount, blackfilterExclude)) {
      errOutput("blackfilter parameters are not valid.");
    }
    if (!validate_blurfilter_parameters(&options.blurfilter_parameters,
                                        blurfilterScanSize, blurfilterScanStep,
                                        blurfilterIntensity)) {
      errOutput("blurfilter parameters are not valid.");
    }

    if (options.start_input == -1)
      options.start_input = (options.start_sheet - 1) * options.input_count + 1;
    if (options.start_output == -1)
      options.start_output =
          (options.start_sheet - 1) * options.output_count + 1;

    if (!options.multiple_sheets && options.end_sheet == -1)
      options.end_sheet = options.start_sheet;
  }

  /* make sure we have at least two arguments after the options, as
     that's the minimum amount of parameters we need (one input and
     one output, or a wildcard of inputs and a wildcard of
     outputs.
  */
  if (optind + 2 > argc)
    errOutput("no input or output files given.\n");

  verboseLog(VERBOSE_NORMAL, WELCOME); // welcome message

  int inputNr = options.start_input;
  int outputNr = options.start_output;

  RectangleSize inputSize = {-1, -1};
  RectangleSize previousSize = {-1, -1};
  Image sheet = EMPTY_IMAGE;
  Image page = EMPTY_IMAGE;

  for (int nr = options.start_sheet;
       (options.end_sheet == -1) || (nr <= options.end_sheet); nr++) {
    char inputFilesBuffer[2][PATH_MAX];
    char outputFilesBuffer[2][PATH_MAX];
    char *inputFileNames[2];
    char *outputFileNames[2];

    // -------------------------------------------------------------------
    // --- begin processing                                            ---
    // -------------------------------------------------------------------

    bool inputWildcard =
        options.multiple_sheets && (strchr(argv[optind], '%') != NULL);
    bool outputWildcard = false;

    for (int i = 0; i < options.input_count; i++) {
      bool ins = isInMultiIndex(inputNr, options.insert_blank);
      bool repl = isInMultiIndex(inputNr, options.replace_blank);

      if (repl) {
        inputFileNames[i] = NULL;
        inputNr++; /* replace */
      } else if (ins) {
        inputFileNames[i] = NULL; /* insert */
      } else if (inputWildcard) {
        sprintf(inputFilesBuffer[i], argv[optind], inputNr++);
        inputFileNames[i] = inputFilesBuffer[i];
      } else if (optind >= argc) {
        if (options.end_sheet == -1) {
          options.end_sheet = nr - 1;
          goto sheet_end;
        } else {
          errOutput("not enough input files given.");
        }
      } else {
        inputFileNames[i] = argv[optind++];
      }
      if (inputFileNames[i] == NULL) {
        verboseLog(VERBOSE_DEBUG, "added blank input file\n");
      } else {
        verboseLog(VERBOSE_DEBUG, "added input file %s\n", inputFileNames[i]);
      }

      if (inputFileNames[i] != NULL) {
        struct stat statBuf;
        if (stat(inputFileNames[i], &statBuf) != 0) {
          if (options.end_sheet == -1) {
            options.end_sheet = nr - 1;
            goto sheet_end;
          } else {
            errOutput("unable to open file %s.", inputFileNames[i]);
          }
        }
      }
    }
    if (inputWildcard)
      optind++;

    if (optind >= argc) { // see if any one of the last two optind++ has pushed
                          // it over the array boundary
      errOutput("not enough output files given.");
    }
    outputWildcard =
        options.multiple_sheets && (strchr(argv[optind], '%') != NULL);
    for (int i = 0; i < options.output_count; i++) {
      if (outputWildcard) {
        sprintf(outputFilesBuffer[i], argv[optind], outputNr++);
        outputFileNames[i] = outputFilesBuffer[i];
      } else if (optind >= argc) {
        errOutput("not enough output files given.");
      } else {
        outputFileNames[i] = argv[optind++];
      }
      verboseLog(VERBOSE_DEBUG, "added output file %s\n", outputFileNames[i]);

      if (!options.overwrite_output) {
        struct stat statbuf;
        if (stat(outputFileNames[i], &statbuf) == 0) {
          errOutput("output file '%s' already present.\n", outputFileNames[i]);
        }
      }
    }
    if (outputWildcard)
      optind++;

    // ---------------------------------------------------------------
    // --- process single sheet                                    ---
    // ---------------------------------------------------------------

    if (isInMultiIndex(nr, options.sheet_multi_index) &&
        (!isInMultiIndex(nr, options.exclude_multi_index))) {
      char s1[1023]; // buffers for result of implode()
      char s2[1023];

      verboseLog(
          VERBOSE_NORMAL,
          "\n-------------------------------------------------------------"
          "------------------\n");

      if (options.multiple_sheets) {
        verboseLog(
            VERBOSE_NORMAL, "Processing sheet #%d: %s -> %s\n", nr,
            implode(s1, (const char **)inputFileNames, options.input_count),
            implode(s2, (const char **)outputFileNames, options.output_count));
      } else {
        verboseLog(
            VERBOSE_NORMAL, "Processing sheet: %s -> %s\n",
            implode(s1, (const char **)inputFileNames, options.input_count),
            implode(s2, (const char **)outputFileNames, options.output_count));
      }

      // load input image(s)
      for (int j = 0; j < options.input_count; j++) {
        if (inputFileNames[j] !=
            NULL) { // may be null if --insert-blank or --replace-blank
          verboseLog(VERBOSE_MORE, "loading file %s.\n", inputFileNames[j]);

          loadImage(inputFileNames[j], &page, options.sheet_background,
                    options.abs_black_threshold);
          saveDebug("_loaded_%d.pnm", inputNr - options.input_count + j, page);

          if (options.output_pixel_format == AV_PIX_FMT_NONE &&
              page.frame != NULL) {
            options.output_pixel_format = page.frame->format;
          }

          // pre-rotate
          if (options.pre_rotate != 0) {
            verboseLog(VERBOSE_NORMAL, "pre-rotating %hd degrees.\n",
                       options.pre_rotate);

            flip_rotate_90(&page, options.pre_rotate / 90);
          }

          // if sheet-size is not known yet (and not forced by --sheet-size),
          // set now based on size of (first) input image
          RectangleSize inputSheetSize = {
              .width = page.frame->width * options.input_count,
              .height = page.frame->height,
          };
          inputSize = coerce_size(
              inputSize, coerce_size(options.sheet_size, inputSheetSize));
        } else { // inputFiles[j] == NULL
          page = EMPTY_IMAGE;
        }

        // place image into sheet buffer
        // allocate sheet-buffer if not done yet
        if ((sheet.frame == NULL) && (inputSize.width != -1) &&
            (inputSize.height != -1)) {
          sheet = create_image(inputSize, AV_PIX_FMT_RGB24, true,
                               options.sheet_background,
                               options.abs_black_threshold);
        }
        if (page.frame != NULL) {
          saveDebug("_page%d.pnm", inputNr - options.input_count + j, page);
          saveDebug("_before_center_page%d.pnm",
                    inputNr - options.input_count + j, sheet);

          center_image(page, sheet,
                       (Point){(inputSize.width * j / options.input_count), 0},
                       (RectangleSize){(inputSize.width / options.input_count),
                                       inputSize.height});

          saveDebug("_after_center_page%d.pnm",
                    inputNr - options.input_count + j, sheet);
        }
      }

      // the only case that buffer is not yet initialized is if all blank pages
      // have been inserted
      if (sheet.frame == NULL) {
        // last chance: try to get previous (unstretched/not zoomed) sheet size
        inputSize = previousSize;
        verboseLog(VERBOSE_NORMAL,
                   "need to guess sheet size from previous sheet: %dx%d\n",
                   inputSize.width, inputSize.height);

        if ((inputSize.width == -1) || (inputSize.height == -1)) {
          errOutput("sheet size unknown, use at least one input file per "
                    "sheet, or force using --sheet-size.");
        } else {
          sheet = create_image(inputSize, AV_PIX_FMT_RGB24, true,
                               options.sheet_background,
                               options.abs_black_threshold);
        }
      }

      previousSize = inputSize;

      // pre-mirroring
      if (options.pre_mirror.horizontal || options.pre_mirror.vertical) {
        verboseLog(VERBOSE_NORMAL, "pre-mirroring %s\n",
                   direction_to_string(options.pre_mirror));

        mirror(sheet, options.pre_mirror);
      }

      // pre-shifting
      if (options.pre_shift.horizontal != 0 ||
          options.pre_shift.vertical != 0) {
        verboseLog(VERBOSE_NORMAL, "pre-shifting [%" PRId32 ",%" PRId32 "]\n",
                   options.pre_shift.horizontal, options.pre_shift.vertical);

        shift_image(&sheet, options.pre_shift);
      }

      // pre-masking
      if (preMaskCount > 0) {
        verboseLog(VERBOSE_NORMAL, "pre-masking\n ");

        apply_masks(sheet, preMasks, preMaskCount, options.mask_color);
      }

      // --------------------------------------------------------------
      // --- verbose parameter output,                              ---
      // --------------------------------------------------------------

      // parameters and size are known now

      if (verbose >= VERBOSE_MORE) {
        switch (options.layout) {
        case LAYOUT_NONE:
          printf("layout: none\n");
          break;
        case LAYOUT_SINGLE:
          printf("layout: single\n");
          break;
        case LAYOUT_DOUBLE:
          printf("layout: double\n");
          break;
        default:
          assert(false); // unreachable
        }

        if (options.pre_rotate != 0) {
          printf("pre-rotate: %d\n", options.pre_rotate);
        }
        printf("pre-mirror: %s\n", direction_to_string(options.pre_mirror));
        if (options.pre_shift.horizontal != 0 ||
            options.pre_shift.vertical != 0) {
          printf("pre-shift: [%" PRId32 ",%" PRId32 "]\n",
                 options.pre_shift.horizontal, options.pre_shift.vertical);
        }
        if (options.pre_wipes->count > 0) {
          printf("pre-wipe: ");
          for (size_t i = 0; i < options.pre_wipes->count; i++) {
            print_rectangle(options.pre_wipes->areas[i]);
          }
          printf("\n");
        }
        if (memcmp(&options.pre_border, &BORDER_NULL, sizeof(BORDER_NULL)) !=
            0) {
          printf("pre-border: ");
          print_border(options.pre_border);
          printf("\n");
        }
        if (preMaskCount > 0) {
          printf("pre-masking: ");
          for (int i = 0; i < preMaskCount; i++) {
            print_rectangle(preMasks[i]);
          }
          printf("\n");
        }
        if (options.stretch_size.width != -1 ||
            options.stretch_size.height != -1) {
          printf("stretch to: %" PRId32 "x%" PRId32 "\n",
                 options.stretch_size.width, options.stretch_size.height);
        }
        if (options.post_stretch_size.width != -1 ||
            options.post_stretch_size.height != -1) {
          printf("post-stretch to: %" PRId32 "x%" PRId32 "d\n",
                 options.post_stretch_size.width,
                 options.post_stretch_size.height);
        }
        if (options.pre_zoom_factor != 1.0) {
          printf("zoom: %f\n", options.pre_zoom_factor);
        }
        if (options.post_zoom_factor != 1.0) {
          printf("post-zoom: %f\n", options.post_zoom_factor);
        }
        if (options.no_blackfilter_multi_index.count != -1) {
          printf("blackfilter-scan-direction: %s\n",
                 direction_to_string(
                     options.blackfilter_parameters.scan_direction));
          printf("blackfilter-scan-size: ");
          print_rectangle_size(options.blackfilter_parameters.scan_size);
          printf("\nblackfilter-scan-depth: [%d,%d]\n",
                 options.blackfilter_parameters.scan_depth.horizontal,
                 options.blackfilter_parameters.scan_depth.vertical);
          printf("blackfilter-scan-step: ");
          print_delta(options.blackfilter_parameters.scan_step);
          printf("\nblackfilter-scan-threshold: %d\n",
                 options.blackfilter_parameters.abs_threshold);
          if (options.blackfilter_parameters.exclusions_count > 0) {
            printf("blackfilter-scan-exclude: ");
            for (size_t i = 0;
                 i < options.blackfilter_parameters.exclusions_count; i++) {
              print_rectangle(options.blackfilter_parameters.exclusions[i]);
            }
            printf("\n");
          }
          printf("blackfilter-intensity: %d\n",
                 options.blackfilter_parameters.intensity);
          if (options.no_blackfilter_multi_index.count > 0) {
            printf("blackfilter DISABLED for sheets: ");
            printMultiIndex(options.no_blackfilter_multi_index);
          }
        } else {
          printf("blackfilter DISABLED for all sheets.\n");
        }
        if (options.no_noisefilter_multi_index.count != -1) {
          printf("noisefilter-intensity: %" PRIu64 "\n",
                 options.noisefilter_intensity);
          if (options.no_noisefilter_multi_index.count > 0) {
            printf("noisefilter DISABLED for sheets: ");
            printMultiIndex(options.no_noisefilter_multi_index);
          }
        } else {
          printf("noisefilter DISABLED for all sheets.\n");
        }
        if (options.no_blurfilter_multi_index.count != -1) {
          printf("blurfilter-size: ");
          print_rectangle_size(options.blurfilter_parameters.scan_size);
          printf("\nblurfilter-step: ");
          print_delta(options.blurfilter_parameters.scan_step);
          printf("\nblurfilter-intensity: %f\n",
                 options.blurfilter_parameters.intensity);
          if (options.no_blurfilter_multi_index.count > 0) {
            printf("blurfilter DISABLED for sheets: ");
            printMultiIndex(options.no_blurfilter_multi_index);
          }
        } else {
          printf("blurfilter DISABLED for all sheets.\n");
        }
        if (options.no_grayfilter_multi_index.count != -1) {
          printf("grayfilter-size: ");
          print_rectangle_size(options.grayfilter_parameters.scan_size);
          printf("\ngrayfilter-step: ");
          print_delta(options.grayfilter_parameters.scan_step);
          printf("\ngrayfilter-threshold: %d\n",
                 options.grayfilter_parameters.abs_threshold);
          if (options.no_grayfilter_multi_index.count > 0) {
            printf("grayfilter DISABLED for sheets: ");
            printMultiIndex(options.no_grayfilter_multi_index);
          }
        } else {
          printf("grayfilter DISABLED for all sheets.\n");
        }
        if (options.no_mask_scan_multi_index.count != -1) {
          printf("mask points: ");
          for (int i = 0; i < pointCount; i++) {
            printf("(%d,%d) ", points[i].x, points[i].y);
          }
          printf("\n");
          printf("mask-scan-direction: %s\n",
                 direction_to_string(
                     options.mask_detection_parameters.scan_direction));
          printf("mask-scan-size: ");
          print_rectangle_size(options.mask_detection_parameters.scan_size);
          printf("\nmask-scan-depth: [%d,%d]\n",
                 options.mask_detection_parameters.scan_depth.horizontal,
                 options.mask_detection_parameters.scan_depth.vertical);
          printf("mask-scan-step: ");
          print_delta(options.mask_detection_parameters.scan_step);
          printf("\nmask-scan-threshold: [%f,%f]\n",
                 options.mask_detection_parameters.scan_threshold.horizontal,
                 options.mask_detection_parameters.scan_threshold.vertical);
          printf("mask-scan-minimum: [%d,%d]\n",
                 options.mask_detection_parameters.minimum_width,
                 options.mask_detection_parameters.minimum_height);
          printf("mask-scan-maximum: [%d,%d]\n",
                 options.mask_detection_parameters.maximum_width,
                 options.mask_detection_parameters.maximum_height);
          printf("mask-color: ");
          print_color(options.mask_color);
          printf("\n");
          if (options.no_mask_scan_multi_index.count > 0) {
            printf("mask-scan DISABLED for sheets: ");
            printMultiIndex(options.no_mask_scan_multi_index);
          }
        } else {
          printf("mask-scan DISABLED for all sheets.\n");
        }
        if (options.no_deskew_multi_index.count != -1) {
          printf("deskew-scan-direction: ");
          print_edges(options.deskew_parameters.scan_edges);
          printf("deskew-scan-size: %d\n",
                 options.deskew_parameters.deskewScanSize);
          printf("deskew-scan-depth: %f\n",
                 options.deskew_parameters.deskewScanDepth);
          printf("deskew-scan-range: %f\n",
                 options.deskew_parameters.deskewScanRangeRad);
          printf("deskew-scan-step: %f\n",
                 options.deskew_parameters.deskewScanStepRad);
          printf("deskew-scan-deviation: %f\n",
                 options.deskew_parameters.deskewScanDeviationRad);
          if (options.no_deskew_multi_index.count > 0) {
            printf("deskew-scan DISABLED for sheets: ");
            printMultiIndex(options.no_deskew_multi_index);
          }
        } else {
          printf("deskew-scan DISABLED for all sheets.\n");
        }
        if (options.no_wipe_multi_index.count != -1) {
          if (options.wipes->count > 0) {
            printf("wipe areas: ");
            for (size_t i = 0; i < options.wipes->count; i++) {
              print_rectangle(options.wipes->areas[i]);
            }
            printf("\n");
          }
        } else {
          printf("wipe DISABLED for all sheets.\n");
        }
        if (middleWipe[0] > 0 || middleWipe[1] > 0) {
          printf("middle-wipe (l,r): %d,%d\n", middleWipe[0], middleWipe[1]);
        }
        if (options.no_border_multi_index.count != -1) {
          if (memcmp(&options.border, &BORDER_NULL, sizeof(BORDER_NULL)) != 0) {
            printf("explicit border: ");
            print_border(options.border);
            printf("\n");
          }
        } else {
          printf("border DISABLED for all sheets.\n");
        }
        if (options.no_border_scan_multi_index.count != -1) {
          printf("border-scan-direction: %s\n",
                 direction_to_string(
                     options.border_scan_parameters.scan_direction));
          printf("border-scan-size: ");
          print_rectangle_size(options.border_scan_parameters.scan_size);
          printf("\nborder-scan-step: ");
          print_delta(options.border_scan_parameters.scan_step);
          printf("\nborder-scan-threshold: [%d,%d]\n",
                 options.border_scan_parameters.scan_threshold.horizontal,
                 options.border_scan_parameters.scan_threshold.vertical);
          if (options.no_border_scan_multi_index.count > 0) {
            printf("border-scan DISABLED for sheets: ");
            printMultiIndex(options.no_border_scan_multi_index);
          }
          printf("border-align: ");
          print_edges(options.mask_alignment_parameters.alignment);
          printf("border-margin: [%d,%d]\n",
                 options.mask_alignment_parameters.margin.horizontal,
                 options.mask_alignment_parameters.margin.vertical);
        } else {
          printf("border-scan DISABLED for all sheets.\n");
        }
        if (options.post_wipes->count > 0) {
          printf("post-wipe: ");
          for (size_t i = 0; i < options.post_wipes->count; i++) {
            print_rectangle(options.post_wipes->areas[i]);
          }
          printf("\n");
        }
        if (memcmp(&options.post_border, &BORDER_NULL, sizeof(BORDER_NULL)) !=
            0) {
          printf("post-border: ");
          print_border(options.post_border);
          printf("\n");
        }
        printf("post-mirror: %s\n", direction_to_string(options.post_mirror));
        if (options.post_shift.horizontal != 0 ||
            options.post_shift.vertical != 0) {
          printf("post-shift: [%" PRId32 ",%" PRId32 "]\n",
                 options.post_shift.horizontal, options.post_shift.vertical);
        }
        if (options.post_rotate != 0) {
          printf("post-rotate: %d\n", options.post_rotate);
        }
        // if (options.ignoreMultiIndex.count > 0) {
        //    printf("EXCLUDE sheets: ");
        //    printMultiIndex(options.ignoreMultiIndex);
        //}
        printf("white-threshold: %d\n", options.abs_white_threshold);
        printf("black-threshold: %d\n", options.abs_black_threshold);
        printf("sheet-background: ");
        print_color(options.sheet_background);
        printf("\n");
        printf("input-files per sheet: %d\n", options.input_count);
        printf("output-files per sheet: %d\n", options.output_count);
        if (options.sheet_size.width != -1 || options.sheet_size.height != -1) {
          printf("sheet size forced to: %" PRId32 " x %" PRId32 " pixels\n",
                 options.sheet_size.width, options.sheet_size.height);
        }
        printf("input-file-sequence:  %s\n",
               implode(s1, (const char **)inputFileNames, options.input_count));
        printf(
            "output-file-sequence: %s\n",
            implode(s1, (const char **)outputFileNames, options.output_count));
        if (options.overwrite_output) {
          printf("OVERWRITING EXISTING FILES\n");
        }
        printf("\n");
      }
      verboseLog(
          VERBOSE_NORMAL, "input-file%s for sheet %d: %s\n",
          pluralS(options.input_count), nr,
          implode(s1, (const char **)inputFileNames, options.input_count));
      verboseLog(
          VERBOSE_NORMAL, "output-file%s for sheet %d: %s\n",
          pluralS(options.output_count), nr,
          implode(s1, (const char **)outputFileNames, options.output_count));
      verboseLog(VERBOSE_NORMAL, "sheet size: %dx%d\n", sheet.frame->width,
                 sheet.frame->height);
      verboseLog(VERBOSE_NORMAL, "...\n");

      // -------------------------------------------------------
      // --- process image data                              ---
      // -------------------------------------------------------

      // stretch
      inputSize = coerce_size(options.stretch_size, size_of_image(sheet));

      inputSize.width *= options.pre_zoom_factor;
      inputSize.height *= options.pre_zoom_factor;

      saveDebug("_before-stretch%d.pnm", nr, sheet);
      stretch_and_replace(&sheet, inputSize, options.interpolate_type);
      saveDebug("_after-stretch%d.pnm", nr, sheet);

      // size
      if (options.page_size.width != -1 || options.page_size.height != -1) {
        inputSize = coerce_size(options.page_size, size_of_image(sheet));
        saveDebug("_before-resize%d.pnm", nr, sheet);
        resize_and_replace(&sheet, inputSize, options.interpolate_type);
        saveDebug("_after-resize%d.pnm", nr, sheet);
      }

      // handle sheet layout

      // LAYOUT_SINGLE
      if (options.layout == LAYOUT_SINGLE) {
        // set middle of sheet as single starting point for mask detection
        if (pointCount == 0) { // no manual settings, use auto-values
          points[pointCount++] =
              (Point){sheet.frame->width / 2, sheet.frame->height / 2};
        }
        if (options.mask_detection_parameters.maximum_width == -1) {
          options.mask_detection_parameters.maximum_width = sheet.frame->width;
        }
        if (options.mask_detection_parameters.maximum_height == -1) {
          options.mask_detection_parameters.maximum_height =
              sheet.frame->height;
        }
        // avoid inner half of the sheet to be blackfilter-detectable
        if (options.blackfilter_parameters.exclusions_count == 0) {
          // no manual settings, use auto-values
          RectangleSize sheetSize = size_of_image(sheet);
          options.blackfilter_parameters
              .exclusions[options.blackfilter_parameters.exclusions_count++] =
              rectangle_from_size(
                  (Point){sheetSize.width / 4, sheetSize.height / 4},
                  (RectangleSize){.width = sheetSize.width / 2,
                                  .height = sheetSize.height / 2});
        }
        // set single outside border to start scanning for final border-scan
        if (outsideBorderscanMaskCount ==
            0) { // no manual settings, use auto-values
          outsideBorderscanMask[outsideBorderscanMaskCount++] =
              full_image(sheet);
        }

        // LAYOUT_DOUBLE
      } else if (options.layout == LAYOUT_DOUBLE) {
        // set two middle of left/right side of sheet as starting points for
        // mask detection
        if (pointCount == 0) { // no manual settings, use auto-values
          points[pointCount++] =
              (Point){sheet.frame->width / 4, sheet.frame->height / 2};
          points[pointCount++] =
              (Point){sheet.frame->width - sheet.frame->width / 4,
                      sheet.frame->height / 2};
        }
        if (options.mask_detection_parameters.maximum_width == -1) {
          options.mask_detection_parameters.maximum_width =
              sheet.frame->width / 2;
        }
        if (options.mask_detection_parameters.maximum_height == -1) {
          options.mask_detection_parameters.maximum_height =
              sheet.frame->height;
        }
        if (middleWipe[0] > 0 || middleWipe[1] > 0) { // left, right
          options.wipes->areas[options.wipes->count++] = (Rectangle){{
              {sheet.frame->width / 2 - middleWipe[0], 0},
              {sheet.frame->width / 2 + middleWipe[1], sheet.frame->height - 1},
          }};
        }
        // avoid inner half of each page to be blackfilter-detectable
        if (options.blackfilter_parameters.exclusions_count == 0) {
          // no manual settings, use auto-values
          RectangleSize sheetSize = size_of_image(sheet);
          RectangleSize filterSize = {
              .width = sheetSize.width / 4,
              .height = sheetSize.height / 2,
          };
          Point firstFilterOrigin = {sheetSize.width / 8, sheetSize.height / 4};
          Point secondFilterOrigin =
              shift_point(firstFilterOrigin, (Delta){sheet.frame->width / 2});

          options.blackfilter_parameters
              .exclusions[options.blackfilter_parameters.exclusions_count++] =
              rectangle_from_size(firstFilterOrigin, filterSize);
          options.blackfilter_parameters
              .exclusions[options.blackfilter_parameters.exclusions_count++] =
              rectangle_from_size(secondFilterOrigin, filterSize);
        }
        // set two outside borders to start scanning for final border-scan
        if (outsideBorderscanMaskCount ==
            0) { // no manual settings, use auto-values
          outsideBorderscanMask[outsideBorderscanMaskCount++] =
              (Rectangle){{POINT_ORIGIN,
                           {sheet.frame->width / 2, sheet.frame->height - 1}}};
          outsideBorderscanMask[outsideBorderscanMaskCount++] =
              (Rectangle){{{sheet.frame->width / 2, 0},
                           {sheet.frame->width - 1, sheet.frame->height - 1}}};
        }
      }
      // if maskScanMaximum still unset (no --layout specified), set to full
      // sheet size now
      if (options.mask_detection_parameters.maximum_width == -1) {
        options.mask_detection_parameters.maximum_width = sheet.frame->width;
      }
      if (options.mask_detection_parameters.maximum_height == -1) {
        options.mask_detection_parameters.maximum_height = sheet.frame->height;
      }

      // pre-wipe
      if (!isExcluded(nr, options.no_wipe_multi_index,
                      options.ignore_multi_index)) {
        apply_wipes(sheet, *options.pre_wipes, options.mask_color);
      }

      // pre-border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, options.pre_border, options.mask_color);
      }

      // black area filter
      if (!isExcluded(nr, options.no_blackfilter_multi_index,
                      options.ignore_multi_index)) {
        saveDebug("_before-blackfilter%d.pnm", nr, sheet);
        blackfilter(sheet, options.blackfilter_parameters);
        saveDebug("_after-blackfilter%d.pnm", nr, sheet);
      } else {
        verboseLog(VERBOSE_MORE, "+ blackfilter DISABLED for sheet %d\n", nr);
      }

      // noise filter
      if (!isExcluded(nr, options.no_noisefilter_multi_index,
                      options.ignore_multi_index)) {
        verboseLog(VERBOSE_NORMAL, "noise-filter ...");

        saveDebug("_before-noisefilter%d.pnm", nr, sheet);
        uint64_t filterResult = noisefilter(
            sheet, options.noisefilter_intensity, options.abs_white_threshold);
        saveDebug("_after-noisefilter%d.pnm", nr, sheet);
        verboseLog(VERBOSE_NORMAL, " deleted %" PRId64 " clusters.\n",
                   filterResult);

      } else {
        verboseLog(VERBOSE_MORE, "+ noisefilter DISABLED for sheet %d\n", nr);
      }

      // blur filter
      if (!isExcluded(nr, options.no_blurfilter_multi_index,
                      options.ignore_multi_index)) {
        verboseLog(VERBOSE_NORMAL, "blur-filter...");

        saveDebug("_before-blurfilter%d.pnm", nr, sheet);
        uint64_t filterResult = blurfilter(sheet, options.blurfilter_parameters,
                                           options.abs_white_threshold);
        saveDebug("_after-blurfilter%d.pnm", nr, sheet);
        verboseLog(VERBOSE_NORMAL, " deleted %" PRIu64 " pixels.\n",
                   filterResult);

      } else {
        verboseLog(VERBOSE_MORE, "+ blurfilter DISABLED for sheet %d\n", nr);
      }

      // mask-detection
      if (!isExcluded(nr, options.no_mask_scan_multi_index,
                      options.ignore_multi_index)) {
        detect_masks(sheet, options.mask_detection_parameters, points,
                     pointCount, masks);
      } else {
        verboseLog(VERBOSE_MORE, "+ mask-scan DISABLED for sheet %d\n", nr);
      }

      // permanently apply masks
      if (maskCount > 0) {
        saveDebug("_before-masking%d.pnm", nr, sheet);
        apply_masks(sheet, masks, maskCount, options.mask_color);
        saveDebug("_after-masking%d.pnm", nr, sheet);
      }

      // gray filter
      if (!isExcluded(nr, options.no_grayfilter_multi_index,
                      options.ignore_multi_index)) {
        verboseLog(VERBOSE_NORMAL, "gray-filter...");

        saveDebug("_before-grayfilter%d.pnm", nr, sheet);
        uint64_t filterResult =
            grayfilter(sheet, options.grayfilter_parameters);
        saveDebug("_after-grayfilter%d.pnm", nr, sheet);
        verboseLog(VERBOSE_NORMAL, " deleted %" PRIu64 " pixels.\n",
                   filterResult);
      } else {
        verboseLog(VERBOSE_MORE, "+ grayfilter DISABLED for sheet %d\n", nr);
      }

      // rotation-detection
      if ((!isExcluded(nr, options.no_deskew_multi_index,
                       options.ignore_multi_index))) {
        saveDebug("_before-deskew%d.pnm", nr, sheet);

        // detect masks again, we may get more precise results now after first
        // masking and grayfilter
        if (!isExcluded(nr, options.no_mask_scan_multi_index,
                        options.ignore_multi_index)) {
          maskCount = detect_masks(sheet, options.mask_detection_parameters,
                                   points, pointCount, masks);
        } else {
          verboseLog(VERBOSE_MORE, "(mask-scan before deskewing disabled)\n");
        }

        // auto-deskew each mask
        for (int i = 0; i < maskCount; i++) {
          saveDebug("_before-deskew-detect%d.pnm", nr * maskCount + i, sheet);
          float rotation =
              detect_rotation(sheet, masks[i], options.deskew_parameters);
          saveDebug("_after-deskew-detect%d.pnm", nr * maskCount + i, sheet);

          verboseLog(VERBOSE_NORMAL, "rotate (%d,%d): %f\n", points[i].x,
                     points[i].y, rotation);

          if (rotation != 0.0) {
            Image rect = create_compatible_image(
                sheet, size_of_rectangle(masks[i]), false);
            Image rectTarget = create_compatible_image(
                sheet, size_of_rectangle(masks[i]), true);

            // copy area to rotate into rSource
            copy_rectangle(sheet, rect,
                           (Rectangle){{masks[i].vertex[0], POINT_INFINITY}},
                           POINT_ORIGIN);

            // rotate
            rotate(rect, rectTarget, -rotation, options.interpolate_type);

            // copy result back into whole image
            copy_rectangle(rectTarget, sheet, full_image(rectTarget),
                           masks[i].vertex[0]);

            free_image(&rect);
            free_image(&rectTarget);
          }
        }

        saveDebug("_after-deskew%d.pnm", nr, sheet);
      } else {
        verboseLog(VERBOSE_MORE, "+ deskewing DISABLED for sheet %d\n", nr);
      }

      // auto-center masks on either single-page or double-page layout
      if (!isExcluded(
              nr, options.no_mask_center_multi_index,
              options.ignore_multi_index)) { // (maskCount==pointCount to
                                             // make sure all masks had
                                             // correctly been detected)
        // perform auto-masking again to get more precise masks after rotation
        if (!isExcluded(nr, options.no_mask_scan_multi_index,
                        options.ignore_multi_index)) {
          maskCount = detect_masks(sheet, options.mask_detection_parameters,
                                   points, pointCount, masks);
        } else {
          verboseLog(VERBOSE_MORE, "(mask-scan before centering disabled)\n");
        }

        saveDebug("_before-centering%d.pnm", nr, sheet);
        // center masks on the sheet, according to their page position
        for (int i = 0; i < maskCount; i++) {
          center_mask(sheet, points[i], masks[i]);
        }
        saveDebug("_after-centering%d.pnm", nr, sheet);
      } else {
        verboseLog(VERBOSE_MORE, "+ auto-centering DISABLED for sheet %d\n",
                   nr);
      }

      // explicit wipe
      if (!isExcluded(nr, options.no_wipe_multi_index,
                      options.ignore_multi_index)) {
        apply_wipes(sheet, *options.wipes, options.mask_color);
      } else {
        verboseLog(VERBOSE_MORE, "+ wipe DISABLED for sheet %d\n", nr);
      }

      // explicit border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, options.border, options.mask_color);
      } else {
        verboseLog(VERBOSE_MORE, "+ border DISABLED for sheet %d\n", nr);
      }

      // border-detection
      if (!isExcluded(nr, options.no_border_scan_multi_index,
                      options.ignore_multi_index)) {
        Rectangle autoborderMask[outsideBorderscanMaskCount];
        saveDebug("_before-border%d.pnm", nr, sheet);
        for (int i = 0; i < outsideBorderscanMaskCount; i++) {
          autoborderMask[i] = border_to_mask(
              sheet, detect_border(sheet, options.border_scan_parameters,
                                   outsideBorderscanMask[i]));
        }
        apply_masks(sheet, autoborderMask, outsideBorderscanMaskCount,
                    options.mask_color);
        for (int i = 0; i < outsideBorderscanMaskCount; i++) {
          // border-centering
          if (!isExcluded(nr, options.no_border_align_multi_index,
                          options.ignore_multi_index)) {
            align_mask(sheet, autoborderMask[i], outsideBorderscanMask[i],
                       options.mask_alignment_parameters);
          } else {
            verboseLog(VERBOSE_MORE,
                       "+ border-centering DISABLED for sheet %d\n", nr);
          }
        }
        saveDebug("_after-border%d.pnm", nr, sheet);
      } else {
        verboseLog(VERBOSE_MORE, "+ border-scan DISABLED for sheet %d\n", nr);
      }

      // post-wipe
      if (!isExcluded(nr, options.no_wipe_multi_index,
                      options.ignore_multi_index)) {
        apply_wipes(sheet, *options.post_wipes, options.mask_color);
      }

      // post-border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, options.post_border, options.mask_color);
      }

      // post-mirroring
      if (options.post_mirror.horizontal || options.post_mirror.vertical) {
        verboseLog(VERBOSE_NORMAL, "post-mirroring %s\n",
                   direction_to_string(options.post_mirror));
        mirror(sheet, options.post_mirror);
      }

      // post-shifting
      if ((options.post_shift.horizontal != 0) ||
          ((options.post_shift.vertical != 0))) {
        verboseLog(VERBOSE_NORMAL, "post-shifting [%" PRId32 ",%" PRId32 "]\n",
                   options.post_shift.horizontal, options.post_shift.vertical);

        shift_image(&sheet, options.post_shift);
      }

      // post-rotating
      if (options.post_rotate != 0) {
        verboseLog(VERBOSE_NORMAL, "post-rotating %d degrees.\n",
                   options.post_rotate);
        flip_rotate_90(&sheet, options.post_rotate / 90);
      }

      // post-stretch
      inputSize = coerce_size(options.post_stretch_size, size_of_image(sheet));

      inputSize.width *= options.post_zoom_factor;
      inputSize.height *= options.post_zoom_factor;

      stretch_and_replace(&sheet, inputSize, options.interpolate_type);

      // post-size
      if (options.post_page_size.width != -1 ||
          options.post_page_size.height != -1) {
        inputSize = coerce_size(options.post_page_size, size_of_image(sheet));
        resize_and_replace(&sheet, inputSize, options.interpolate_type);
      }

      // --- write output file ---

      // write split pages output

      if (options.write_output) {
        verboseLog(VERBOSE_NORMAL, "writing output.\n");
        // write files
        saveDebug("_before-save%d.pnm", nr, sheet);

        if (options.output_pixel_format == AV_PIX_FMT_NONE) {
          options.output_pixel_format = sheet.frame->format;
        }

        for (int j = 0; j < options.output_count; j++) {
          // get pagebuffer
          page = create_compatible_image(
              sheet,
              (RectangleSize){sheet.frame->width / options.output_count,
                              sheet.frame->height},
              false);
          copy_rectangle(
              sheet, page,
              (Rectangle){{{page.frame->width * j, 0},
                           {page.frame->width * j + page.frame->width,
                            page.frame->height}}},
              POINT_ORIGIN);

          verboseLog(VERBOSE_MORE, "saving file %s.\n", outputFileNames[j]);

          saveImage(outputFileNames[j], page, options.output_pixel_format);

          free_image(&page);
        }

        free_image(&sheet);
      }
    }

  sheet_end:
    /* if we're not given an input wildcard, and we finished the
     * arguments, we don't want to keep looping.
     */
    if (optind >= argc && !inputWildcard)
      break;
    else if (inputWildcard && outputWildcard)
      optind -= 2;
  }

  return 0;
}
