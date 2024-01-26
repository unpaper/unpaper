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
#include "imageprocess/interpolate.h"
#include "imageprocess/masks.h"
#include "imageprocess/pixel.h"
#include "lib/options.h"
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

Interpolation interpolateType = INTERP_CUBIC;

Pixel sheetBackgroundPixel;
unsigned int absBlackThreshold;
unsigned int absWhiteThreshold;

int sheetSize[DIMENSIONS_COUNT] = {-1, -1};
int sheetBackground = WHITE24;
int preRotate = 0;
int postRotate = 0;
int preMirror = 0;
int postMirror = 0;
int preShift[DIRECTIONS_COUNT] = {0, 0};
int postShift[DIRECTIONS_COUNT] = {0, 0};
int size[DIRECTIONS_COUNT] = {-1, -1};
int postSize[DIRECTIONS_COUNT] = {-1, -1};
int stretchSize[DIRECTIONS_COUNT] = {-1, -1};
int postStretchSize[DIRECTIONS_COUNT] = {-1, -1};
float zoomFactor = 1.0;
float postZoomFactor = 1.0;
Border preBorder = {0, 0, 0, 0};
Border postBorder = {0, 0, 0, 0};
Border border = {0, 0, 0, 0};
bool maskValid[MAX_MASKS];
float whiteThreshold = 0.9;
float blackThreshold = 0.33;
bool writeoutput = true;
bool multisheets = true;

bool overwrite = false;
int dpi = 300;

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
  OPT_DPI,
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
  int w = -1;
  int h = -1;
  int left;
  int top;
  int right;
  int bottom;
  int previousWidth = -1;
  int previousHeight = -1;
  AVFrame *sheet = NULL;
  AVFrame *page = NULL;
  int inputNr;
  int outputNr;
  int option_index = 0;
  int outputPixFmt = -1;
  Options options;

  int deskewScanEdges = (1 << LEFT) | (1 << RIGHT);
  int deskewScanSize = 1500;
  float deskewScanDepth = 0.5;
  float deskewScanRange = 5.0;
  float deskewScanStep = 0.1;
  float deskewScanDeviation = 1.0;
  DeskewParameters deskewParams;
  int maskScanDirections = (1 << HORIZONTAL);
  int maskScanSize[DIRECTIONS_COUNT] = {50, 50};
  int maskScanDepth[DIRECTIONS_COUNT] = {-1, -1};
  int maskScanStep[DIRECTIONS_COUNT] = {5, 5};
  float maskScanThreshold[DIRECTIONS_COUNT] = {0.1, 0.1};
  int maskScanMinimum[DIMENSIONS_COUNT] = {100, 100};
  int maskScanMaximum[DIMENSIONS_COUNT] = {-1, -1}; // set default later
  MaskDetectionParameters maskDetectionParams;
  MaskAlignmentParameters maskAlignmentParams;
  int borderScanDirections = (1 << VERTICAL);
  int borderScanSize[DIRECTIONS_COUNT] = {5, 5};
  int borderScanStep[DIRECTIONS_COUNT] = {5, 5};
  int borderScanThreshold[DIRECTIONS_COUNT] = {5, 5};
  BorderScanParameters borderScanParams;
  int borderAlign = 0;                              // center
  int borderAlignMargin[DIRECTIONS_COUNT] = {0, 0}; // center
  Pixel maskColorPixel;
  size_t pointCount = 0;
  Point points[MAX_POINTS];
  size_t maskCount = 0;
  Rectangle masks[MAX_MASKS];
  size_t preMaskCount = 0;
  Rectangle preMasks[MAX_MASKS];
  int maskColor = WHITE24;
  size_t wipeCount = 0;
  Rectangle wipe[MAX_MASKS];
  int middleWipe[2] = {0, 0};
  size_t preWipeCount = 0;
  Rectangle preWipe[MAX_MASKS];
  size_t postWipeCount = 0;
  Rectangle postWipe[MAX_MASKS];
  Rectangle outsideBorderscanMask[MAX_PAGES]; // set by --layout
  size_t outsideBorderscanMaskCount = 0;
  int blackfilterScanDirections = (1 << HORIZONTAL) | (1 << VERTICAL);
  int blackfilterScanSize[DIRECTIONS_COUNT] = {20, 20};
  int blackfilterScanDepth[DIRECTIONS_COUNT] = {500, 500};
  int blackfilterScanStep[DIRECTIONS_COUNT] = {5, 5};
  float blackfilterScanThreshold = 0.95;
  int blackfilterExcludeCount = 0;
  int blackfilterExclude[MAX_MASKS][EDGES_COUNT];
  int blackfilterIntensity = 20;
  BlackfilterParameters blackfilterParams;
  int blurfilterScanSize[DIRECTIONS_COUNT] = {100, 100};
  int blurfilterScanStep[DIRECTIONS_COUNT] = {50, 50};
  float blurfilterIntensity = 0.01;
  BlurfilterParameters blurfilterParams;
  int grayfilterScanSize[DIRECTIONS_COUNT] = {50, 50};
  int grayfilterScanStep[DIRECTIONS_COUNT] = {20, 20};
  float grayfilterThreshold = 0.5;
  GrayfilterParameters grayfilterParams;
  int noisefilterIntensity = 4;

  // -------------------------------------------------------------------
  // --- parse parameters                                            ---
  // -------------------------------------------------------------------

  options_init(&options);

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
        {"dpi", required_argument, NULL, OPT_DPI},
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
      if (strcmp(optarg, "single") == 0) {
        options.layout = LAYOUT_SINGLE;
      } else if (strcmp(optarg, "double") == 0) {
        options.layout = LAYOUT_DOUBLE;
      } else if (strcmp(optarg, "none") == 0) {
        options.layout = LAYOUT_NONE;
      } else {
        errOutput("unknown layout mode '%s'.", optarg);
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
      parseSize(optarg, sheetSize, dpi);
      break;

    case OPT_SHEET_BACKGROUND:
      sheetBackground = parseColor(optarg);
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
      sscanf(optarg, "%d", &preRotate);
      if ((preRotate != 0) && (abs(preRotate) != 90)) {
        fprintf(
            stderr,
            "cannot set --pre-rotate value other than -90 or 90, ignoring.\n");
        preRotate = 0;
      }
      break;

    case OPT_POST_ROTATE:
      sscanf(optarg, "%d", &postRotate);
      if ((postRotate != 0) && (abs(postRotate) != 90)) {
        fprintf(
            stderr,
            "cannot set --post-rotate value other than -90 or 90, ignoring.\n");
        postRotate = 0;
      }
      break;

    case 'M':
      preMirror =
          parseDirections(optarg); // s = "v", "v,h", "vertical,horizontal", ...
      break;

    case OPT_POST_MIRROR:
      postMirror = parseDirections(optarg);
      break;

    case OPT_PRE_SHIFT:
      parseSize(optarg, preShift, dpi);
      break;

    case OPT_POST_SHIFT:
      parseSize(optarg, postShift, dpi);
      break;

    case OPT_PRE_MASK:
      if (preMaskCount < MAX_MASKS) {
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        preMasks[preMaskCount++] = (Rectangle){{{left, top}, {right, bottom}}};
      } else {
        fprintf(stderr,
                "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                MAX_MASKS, optarg);
      }
      break;

    case 's':
      parseSize(optarg, size, dpi);
      break;

    case OPT_POST_SIZE:
      parseSize(optarg, postSize, dpi);
      break;

    case OPT_STRETCH:
      parseSize(optarg, stretchSize, dpi);
      break;

    case OPT_POST_STRETCH:
      parseSize(optarg, postStretchSize, dpi);
      break;

    case 'z':
      sscanf(optarg, "%f", &zoomFactor);
      break;

    case OPT_POST_ZOOM:
      sscanf(optarg, "%f", &postZoomFactor);
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
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        masks[maskCount] = (Rectangle){{{left, top}, {right, bottom}}};
        maskValid[maskCount] = true;
        maskCount++;
      } else {
        fprintf(stderr,
                "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                MAX_MASKS, optarg);
      }
      break;

    case 'W':
      if (wipeCount < MAX_MASKS) {
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        wipe[wipeCount++] = (Rectangle){{{left, top}, {right, bottom}}};
      } else {
        fprintf(stderr,
                "maximum number of wipes (%d) exceeded, ignoring mask %s\n",
                MAX_MASKS, optarg);
      }
      break;

    case OPT_PRE_WIPE:
      if (preWipeCount < MAX_MASKS) {
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        preWipe[preWipeCount++] = (Rectangle){{{left, top}, {right, bottom}}};
      } else {
        fprintf(stderr,
                "maximum number of pre-wipes (%d) exceeded, ignoring mask %s\n",
                MAX_MASKS, optarg);
      }
      break;

    case OPT_POST_WIPE:
      if (postWipeCount < MAX_MASKS) {
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        postWipe[postWipeCount++] = (Rectangle){{{left, top}, {right, bottom}}};
      } else {
        fprintf(
            stderr,
            "maximum number of post-wipes (%d) exceeded, ignoring mask %s\n",
            MAX_MASKS, optarg);
      }
      break;

    case OPT_MIDDLE_WIPE:
      parseInts(optarg, middleWipe);
      break;

    case 'B':
      sscanf(optarg, "%d,%d,%d,%d", &border.left, &border.top, &border.right,
             &border.bottom);
      break;

    case OPT_PRE_BORDER:
      sscanf(optarg, "%d,%d,%d,%d", &preBorder.left, &preBorder.top,
             &preBorder.right, &preBorder.bottom);
      break;

    case OPT_POST_BORDER:
      sscanf(optarg, "%d,%d,%d,%d", &postBorder.left, &postBorder.top,
             &postBorder.right, &postBorder.bottom);
      break;

    case OPT_NO_BLACK_FILTER:
      parseMultiIndex(optarg, &options.no_blackfilter_multi_index);
      break;

    case OPT_BLACK_FILTER_SCAN_DIRECTION:
      blackfilterScanDirections = parseDirections(optarg);
      break;

    case OPT_BLACK_FILTER_SCAN_SIZE:
      parseInts(optarg, blackfilterScanSize);
      break;

    case OPT_BLACK_FILTER_SCAN_DEPTH:
      parseInts(optarg, blackfilterScanDepth);
      break;

    case OPT_BLACK_FILTER_SCAN_STEP:
      parseInts(optarg, blackfilterScanStep);
      break;

    case OPT_BLACK_FILTER_SCAN_THRESHOLD:
      sscanf(optarg, "%f", &blackfilterScanThreshold);
      break;

    case OPT_BLACK_FILTER_SCAN_EXCLUDE:
      if (blackfilterExcludeCount < MAX_MASKS) {
        left = -1;
        top = -1;
        right = -1;
        bottom = -1;
        sscanf(optarg, "%d,%d,%d,%d", &left, &top, &right,
               &bottom); // x1, y1, x2, y2
        blackfilterExclude[blackfilterExcludeCount][LEFT] = left;
        blackfilterExclude[blackfilterExcludeCount][TOP] = top;
        blackfilterExclude[blackfilterExcludeCount][RIGHT] = right;
        blackfilterExclude[blackfilterExcludeCount][BOTTOM] = bottom;
        blackfilterExcludeCount++;
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
      sscanf(optarg, "%d", &noisefilterIntensity);
      break;

    case OPT_NO_BLUR_FILTER:
      parseMultiIndex(optarg, &options.no_blurfilter_multi_index);
      break;

    case OPT_BLUR_FILTER_SIZE:
      parseInts(optarg, blurfilterScanSize);
      break;

    case OPT_BLUR_FILTER_STEP:
      parseInts(optarg, blurfilterScanStep);
      break;

    case OPT_BLUR_FILTER_INTENSITY:
      sscanf(optarg, "%f", &blurfilterIntensity);
      break;

    case OPT_NO_GRAY_FILTER:
      parseMultiIndex(optarg, &options.no_grayfilter_multi_index);
      break;

    case OPT_GRAY_FILTER_SIZE:
      parseInts(optarg, grayfilterScanSize);
      break;

    case OPT_GRAY_FILTER_STEP:
      parseInts(optarg, grayfilterScanStep);
      break;

    case OPT_GRAY_FILTER_THRESHOLD:
      sscanf(optarg, "%f", &grayfilterThreshold);
      break;

    case OPT_NO_MASK_SCAN:
      parseMultiIndex(optarg, &options.no_mask_scan_multi_index);
      break;

    case OPT_MASK_SCAN_DIRECTION:
      maskScanDirections = parseDirections(optarg);
      break;

    case OPT_MASK_SCAN_SIZE:
      parseInts(optarg, maskScanSize);
      break;

    case OPT_MASK_SCAN_DEPTH:
      parseInts(optarg, maskScanDepth);
      break;

    case OPT_MASK_SCAN_STEP:
      parseInts(optarg, maskScanStep);
      break;

    case OPT_MASK_SCAN_THRESHOLD:
      parseFloats(optarg, maskScanThreshold);
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
      sscanf(optarg, "%d", &maskColor);
      break;

    case OPT_NO_MASK_CENTER:
      parseMultiIndex(optarg, &options.no_mask_center_multi_index);
      break;

    case OPT_NO_DESKEW:
      parseMultiIndex(optarg, &options.no_deskew_multi_index);
      break;

    case OPT_DESKEW_SCAN_DIRECTION:
      deskewScanEdges = parseEdges(optarg);
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
      borderScanDirections = parseDirections(optarg);
      break;

    case OPT_BORDER_SCAN_SIZE:
      parseInts(optarg, borderScanSize);
      break;

    case OPT_BORDER_SCAN_STEP:
      parseInts(optarg, borderScanStep);
      break;

    case OPT_BORDER_SCAN_THRESHOLD:
      parseInts(optarg, borderScanThreshold);
      break;

    case OPT_BORDER_ALIGN:
      borderAlign = parseEdges(optarg);
      break;

    case OPT_BORDER_MARGIN:
      parseSize(optarg, borderAlignMargin, dpi);
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
          "--input-file-sequence and --output-file-sequence are deprecated and "
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
      writeoutput = false;
      break;

    case OPT_NO_MULTI_PAGES:
      multisheets = false;
      break;

    case OPT_DPI:
      sscanf(optarg, "%d", &dpi);
      break;

    case 't':
      if (strcmp(optarg, "pbm") == 0) {
        outputPixFmt = AV_PIX_FMT_MONOWHITE;
      } else if (strcmp(optarg, "pgm") == 0) {
        outputPixFmt = AV_PIX_FMT_GRAY8;
      } else if (strcmp(optarg, "ppm") == 0) {
        outputPixFmt = AV_PIX_FMT_RGB24;
      }
      break;

    case 'q':
      verbose = VERBOSE_QUIET;
      break;

    case OPT_OVERWRITE:
      overwrite = true;
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
      if (strcmp(optarg, "nearest") == 0) {
        interpolateType = INTERP_NN;
      } else if (strcmp(optarg, "linear") == 0) {
        interpolateType = INTERP_LINEAR;
      } else if (strcmp(optarg, "cubic") == 0) {
        interpolateType = INTERP_CUBIC;
      } else {
        fprintf(stderr,
                "Could not parse --interpolate, using cubic as default.\n");
        interpolateType = INTERP_CUBIC;
      }
      break;
    }
  }

  /* make sure we have at least two arguments after the options, as
     that's the minimum amount of parameters we need (one input and
     one output, or a wildcard of inputs and a wildcard of
     outputs.
  */
  if (optind + 2 > argc)
    errOutput("no input or output files given.\n");

  verboseLog(VERBOSE_NORMAL, WELCOME); // welcome message

  if (options.start_input == -1)
    options.start_input = (options.start_sheet - 1) * options.input_count + 1;
  if (options.start_output == -1)
    options.start_output = (options.start_sheet - 1) * options.output_count + 1;

  inputNr = options.start_input;
  outputNr = options.start_output;

  if (!multisheets && options.end_sheet == -1)
    options.end_sheet = options.start_sheet;

  // Calculate the constant absolute values based on the relative parameters.
  sheetBackgroundPixel = pixelValueToPixel(sheetBackground);
  maskColorPixel = pixelValueToPixel(maskColor);
  absBlackThreshold = WHITE * (1.0 - blackThreshold);
  absWhiteThreshold = WHITE * (whiteThreshold);

  deskewParams = validate_deskew_parameters(deskewScanRange, deskewScanStep,
                                            deskewScanDeviation, deskewScanSize,
                                            deskewScanDepth, deskewScanEdges);
  maskDetectionParams = validate_mask_detection_parameters(
      maskScanDirections, maskScanSize, maskScanDepth, maskScanStep,
      maskScanThreshold, maskScanMinimum, maskScanMaximum);
  maskAlignmentParams =
      validate_mask_alignment_parameters(borderAlign, borderAlignMargin);
  borderScanParams =
      validate_border_scan_parameters(borderScanDirections, borderScanSize,
                                      borderScanStep, borderScanThreshold);
  grayfilterParams = validate_grayfilter_parameters(
      grayfilterScanSize[HORIZONTAL], grayfilterScanSize[VERTICAL],
      grayfilterScanStep[HORIZONTAL], grayfilterScanStep[VERTICAL],
      grayfilterThreshold);
  // This will be reachable memory at the end of the program, we don't need to
  // free it.
  Rectangle blackfilterExclusionAreas[blackfilterExcludeCount];
  for (size_t i = 0; i < blackfilterExcludeCount; i++) {
    blackfilterExclusionAreas[i] = maskToRectangle(blackfilterExclude[i]);
  }
  blackfilterParams = validate_blackfilter_parameters(
      blackfilterScanSize[HORIZONTAL], blackfilterScanSize[VERTICAL],
      blackfilterScanStep[HORIZONTAL], blackfilterScanStep[VERTICAL],
      blackfilterScanDepth[HORIZONTAL], blackfilterScanDepth[VERTICAL],
      blackfilterScanDirections, blackfilterScanThreshold, blackfilterIntensity,
      blackfilterExcludeCount, blackfilterExclusionAreas);
  blurfilterParams = validate_blurfilter_parameters(
      blurfilterScanSize[HORIZONTAL], blurfilterScanSize[VERTICAL],
      blurfilterScanStep[HORIZONTAL], blurfilterScanStep[VERTICAL],
      blurfilterIntensity);

  for (int nr = options.start_sheet;
       (options.end_sheet == -1) || (nr <= options.end_sheet); nr++) {
    char inputFilesBuffer[2][255];
    char outputFilesBuffer[2][255];
    char *inputFileNames[2];
    char *outputFileNames[2];

    // -------------------------------------------------------------------
    // --- begin processing                                            ---
    // -------------------------------------------------------------------

    bool inputWildcard = multisheets && (strchr(argv[optind], '%') != NULL);
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
    outputWildcard = multisheets && (strchr(argv[optind], '%') != NULL);
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

      if (!overwrite) {
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

      if (multisheets) {
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

          loadImage(inputFileNames[j], &page);
          saveDebug("_loaded_%d.pnm", inputNr - options.input_count + j, page);

          if (outputPixFmt == -1 && page != NULL) {
            outputPixFmt = page->format;
          }

          // pre-rotate
          if (preRotate != 0) {
            verboseLog(VERBOSE_NORMAL, "pre-rotating %d degrees.\n", preRotate);

            flip_rotate_90(&page, preRotate / 90, absBlackThreshold);
          }

          // if sheet-size is not known yet (and not forced by --sheet-size),
          // set now based on size of (first) input image
          if (w == -1) {
            if (sheetSize[WIDTH] != -1) {
              w = sheetSize[WIDTH];
            } else {
              w = page->width * options.input_count;
            }
          }
          if (h == -1) {
            if (sheetSize[HEIGHT] != -1) {
              h = sheetSize[HEIGHT];
            } else {
              h = page->height;
            }
          }
        } else { // inputFiles[j] == NULL
          page = NULL;
        }

        // place image into sheet buffer
        // allocate sheet-buffer if not done yet
        if ((sheet == NULL) && (w != -1) && (h != -1)) {
          sheet = create_image((RectangleSize){w, h}, AV_PIX_FMT_RGB24, true,
                               sheetBackgroundPixel, absBlackThreshold);
        }
        if (page != NULL) {
          saveDebug("_page%d.pnm", inputNr - options.input_count + j, page);
          saveDebug("_before_center_page%d.pnm",
                    inputNr - options.input_count + j, sheet);

          center_image(page, sheet, (Point){(w * j / options.input_count), 0},
                       (RectangleSize){(w / options.input_count), h},
                       sheetBackgroundPixel, absBlackThreshold);

          saveDebug("_after_center_page%d.pnm",
                    inputNr - options.input_count + j, sheet);
        }
      }

      // the only case that buffer is not yet initialized is if all blank pages
      // have been inserted
      if (sheet == NULL) {
        // last chance: try to get previous (unstretched/not zoomed) sheet size
        w = previousWidth;
        h = previousHeight;
        verboseLog(VERBOSE_NORMAL,
                   "need to guess sheet size from previous sheet: %dx%d\n", w,
                   h);

        if ((w == -1) || (h == -1)) {
          errOutput("sheet size unknown, use at least one input file per "
                    "sheet, or force using --sheet-size.");
        } else {
          sheet = create_image((RectangleSize){w, h}, AV_PIX_FMT_RGB24, true,
                               sheetBackgroundPixel, absBlackThreshold);
        }
      }

      previousWidth = w;
      previousHeight = h;

      // pre-mirroring
      if (preMirror != 0) {
        verboseLog(VERBOSE_NORMAL, "pre-mirroring %s\n",
                   getDirections(preMirror));

        mirror(sheet, !!(preMirror & (1 << HORIZONTAL)),
               !!(preMirror & (1 << VERTICAL)), absBlackThreshold);
      }

      // pre-shifting
      if ((preShift[WIDTH] != 0) || ((preShift[HEIGHT] != 0))) {
        verboseLog(VERBOSE_NORMAL, "pre-shifting [%d,%d]\n", preShift[WIDTH],
                   preShift[HEIGHT]);

        shift_image(&sheet, (Delta){preShift[WIDTH], preShift[HEIGHT]},
                    sheetBackgroundPixel, absBlackThreshold);
      }

      // pre-masking
      if (preMaskCount > 0) {
        verboseLog(VERBOSE_NORMAL, "pre-masking\n ");

        apply_masks(sheet, preMasks, preMaskCount, maskColorPixel,
                    absBlackThreshold);
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

        if (preRotate != 0) {
          printf("pre-rotate: %d\n", preRotate);
        }
        if (preMirror != 0) {
          printf("pre-mirror: %s\n", getDirections(preMirror));
        }
        if ((preShift[WIDTH] != 0) || ((preShift[HEIGHT] != 0))) {
          printf("pre-shift: [%d,%d]\n", preShift[WIDTH], preShift[HEIGHT]);
        }
        if (preWipeCount > 0) {
          printf("pre-wipe: ");
          for (int i = 0; i < preWipeCount; i++) {
            printf("[%d,%d,%d,%d] ", preWipe[i].vertex[0].x,
                   preWipe[i].vertex[0].y, preWipe[i].vertex[1].x,
                   preWipe[i].vertex[1].y);
          }
          printf("\n");
        }
        if (memcmp(&preBorder, &BORDER_NULL, sizeof(BORDER_NULL)) != 0) {
          printf("pre-border: [%d,%d,%d,%d]\n", preBorder.left, preBorder.top,
                 preBorder.right, preBorder.bottom);
        }
        if (preMaskCount > 0) {
          printf("pre-masking: ");
          for (int i = 0; i < preMaskCount; i++) {
            printf("[%d,%d,%d,%d] ", preMasks[i].vertex[0].x,
                   preMasks[i].vertex[0].y, preMasks[i].vertex[1].x,
                   preMasks[i].vertex[1].y);
          }
          printf("\n");
        }
        if ((stretchSize[WIDTH] != -1) || (stretchSize[HEIGHT] != -1)) {
          printf("stretch to: %dx%d\n", stretchSize[WIDTH],
                 stretchSize[HEIGHT]);
        }
        if ((postStretchSize[WIDTH] != -1) || (postStretchSize[HEIGHT] != -1)) {
          printf("post-stretch to: %dx%d\n", postStretchSize[WIDTH],
                 postStretchSize[HEIGHT]);
        }
        if (zoomFactor != 1.0) {
          printf("zoom: %f\n", zoomFactor);
        }
        if (postZoomFactor != 1.0) {
          printf("post-zoom: %f\n", postZoomFactor);
        }
        if (options.no_blackfilter_multi_index.count != -1) {
          printf("blackfilter-scan-direction: %s\n",
                 getDirections(blackfilterScanDirections));
          printf("blackfilter-scan-size: [%d,%d]\n", blackfilterScanSize[0],
                 blackfilterScanSize[1]);
          printf("blackfilter-scan-depth: [%d,%d]\n", blackfilterScanDepth[0],
                 blackfilterScanDepth[1]);
          printf("blackfilter-scan-step: [%d,%d]\n", blackfilterScanStep[0],
                 blackfilterScanStep[1]);
          printf("blackfilter-scan-threshold: %f\n", blackfilterScanThreshold);
          if (blackfilterExcludeCount > 0) {
            printf("blackfilter-scan-exclude: ");
            for (int i = 0; i < blackfilterExcludeCount; i++) {
              printf("[%d,%d,%d,%d] ", blackfilterExclude[i][LEFT],
                     blackfilterExclude[i][TOP], blackfilterExclude[i][RIGHT],
                     blackfilterExclude[i][BOTTOM]);
            }
            printf("\n");
          }
          printf("blackfilter-intensity: %d\n", blackfilterIntensity);
          if (options.no_blackfilter_multi_index.count > 0) {
            printf("blackfilter DISABLED for sheets: ");
            printMultiIndex(options.no_blackfilter_multi_index);
          }
        } else {
          printf("blackfilter DISABLED for all sheets.\n");
        }
        if (options.no_noisefilter_multi_index.count != -1) {
          printf("noisefilter-intensity: %d\n", noisefilterIntensity);
          if (options.no_noisefilter_multi_index.count > 0) {
            printf("noisefilter DISABLED for sheets: ");
            printMultiIndex(options.no_noisefilter_multi_index);
          }
        } else {
          printf("noisefilter DISABLED for all sheets.\n");
        }
        if (options.no_blurfilter_multi_index.count != -1) {
          printf("blurfilter-size: [%d,%d]\n", blurfilterScanSize[0],
                 blurfilterScanSize[1]);
          printf("blurfilter-step: [%d,%d]\n", blurfilterScanStep[0],
                 blurfilterScanStep[1]);
          printf("blurfilter-intensity: %f\n", blurfilterIntensity);
          if (options.no_blurfilter_multi_index.count > 0) {
            printf("blurfilter DISABLED for sheets: ");
            printMultiIndex(options.no_blurfilter_multi_index);
          }
        } else {
          printf("blurfilter DISABLED for all sheets.\n");
        }
        if (options.no_grayfilter_multi_index.count != -1) {
          printf("grayfilter-size: [%d,%d]\n", grayfilterScanSize[0],
                 grayfilterScanSize[1]);
          printf("grayfilter-step: [%d,%d]\n", grayfilterScanStep[0],
                 grayfilterScanStep[1]);
          printf("grayfilter-threshold: %f\n", grayfilterThreshold);
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
                 getDirections(maskScanDirections));
          printf("mask-scan-size: [%d,%d]\n", maskScanSize[0], maskScanSize[1]);
          printf("mask-scan-depth: [%d,%d]\n", maskScanDepth[0],
                 maskScanDepth[1]);
          printf("mask-scan-step: [%d,%d]\n", maskScanStep[0], maskScanStep[1]);
          printf("mask-scan-threshold: [%f,%f]\n", maskScanThreshold[0],
                 maskScanThreshold[1]);
          printf("mask-scan-minimum: [%d,%d]\n", maskScanMinimum[0],
                 maskScanMinimum[1]);
          printf("mask-scan-maximum: [%d,%d]\n", maskScanMaximum[0],
                 maskScanMaximum[1]);
          printf("mask-color: %d\n", maskColor);
          if (options.no_mask_scan_multi_index.count > 0) {
            printf("mask-scan DISABLED for sheets: ");
            printMultiIndex(options.no_mask_scan_multi_index);
          }
        } else {
          printf("mask-scan DISABLED for all sheets.\n");
        }
        if (options.no_deskew_multi_index.count != -1) {
          printf("deskew-scan-direction: ");
          printEdges(deskewScanEdges);
          printf("deskew-scan-size: %d\n", deskewScanSize);
          printf("deskew-scan-depth: %f\n", deskewScanDepth);
          printf("deskew-scan-range: %f\n", deskewScanRange);
          printf("deskew-scan-step: %f\n", deskewScanStep);
          printf("deskew-scan-deviation: %f\n", deskewScanDeviation);
          if (options.no_deskew_multi_index.count > 0) {
            printf("deskew-scan DISABLED for sheets: ");
            printMultiIndex(options.no_deskew_multi_index);
          }
        } else {
          printf("deskew-scan DISABLED for all sheets.\n");
        }
        if (options.no_wipe_multi_index.count != -1) {
          if (wipeCount > 0) {
            printf("wipe areas: ");
            for (int i = 0; i < wipeCount; i++) {
              printf("[%d,%d,%d,%d] ", wipe[i].vertex[0].x, wipe[i].vertex[0].y,
                     wipe[i].vertex[1].x, wipe[i].vertex[1].y);
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
          if (memcmp(&border, &BORDER_NULL, sizeof(BORDER_NULL)) != 0) {
            printf("explicit border: [%d,%d,%d,%d]\n", border.left, border.top,
                   border.right, border.bottom);
          }
        } else {
          printf("border DISABLED for all sheets.\n");
        }
        if (options.no_border_scan_multi_index.count != -1) {
          printf("border-scan-direction: %s\n",
                 getDirections(borderScanDirections));
          printf("border-scan-size: [%d,%d]\n", borderScanSize[0],
                 borderScanSize[1]);
          printf("border-scan-step: [%d,%d]\n", borderScanStep[0],
                 borderScanStep[1]);
          printf("border-scan-threshold: [%d,%d]\n", borderScanThreshold[0],
                 borderScanThreshold[1]);
          if (options.no_border_scan_multi_index.count > 0) {
            printf("border-scan DISABLED for sheets: ");
            printMultiIndex(options.no_border_scan_multi_index);
          }
          printf("border-align: ");
          printEdges(borderAlign);
          printf("border-margin: [%d,%d]\n", borderAlignMargin[0],
                 borderAlignMargin[1]);
        } else {
          printf("border-scan DISABLED for all sheets.\n");
        }
        if (postWipeCount > 0) {
          printf("post-wipe: ");
          for (int i = 0; i < postWipeCount; i++) {
            printf("[%d,%d,%d,%d] ", postWipe[i].vertex[0].x,
                   postWipe[i].vertex[0].y, postWipe[i].vertex[1].x,
                   postWipe[i].vertex[1].y);
          }
          printf("\n");
        }
        if (memcmp(&postBorder, &BORDER_NULL, sizeof(BORDER_NULL)) != 0) {
          printf("post-border: [%d,%d,%d,%d]\n", postBorder.left,
                 postBorder.top, postBorder.right, postBorder.bottom);
        }
        if (postMirror != 0) {
          printf("post-mirror: %s\n", getDirections(postMirror));
        }
        if ((postShift[WIDTH] != 0) || ((postShift[HEIGHT] != 0))) {
          printf("post-shift: [%d,%d]\n", postShift[WIDTH], postShift[HEIGHT]);
        }
        if (postRotate != 0) {
          printf("post-rotate: %d\n", postRotate);
        }
        // if (options.ignoreMultiIndex.count > 0) {
        //    printf("EXCLUDE sheets: ");
        //    printMultiIndex(options.ignoreMultiIndex);
        //}
        printf("white-threshold: %f\n", whiteThreshold);
        printf("black-threshold: %f\n", blackThreshold);
        printf("sheet-background: %s %6x\n",
               ((sheetBackground == BLACK24) ? "black" : "white"),
               sheetBackground);
        printf("dpi: %d\n", dpi);
        printf("input-files per sheet: %d\n", options.input_count);
        printf("output-files per sheet: %d\n", options.output_count);
        if ((sheetSize[WIDTH] != -1) || (sheetSize[HEIGHT] != -1)) {
          printf("sheet size forced to: %d x %d pixels\n", sheetSize[WIDTH],
                 sheetSize[HEIGHT]);
        }
        printf("input-file-sequence:  %s\n",
               implode(s1, (const char **)inputFileNames, options.input_count));
        printf(
            "output-file-sequence: %s\n",
            implode(s1, (const char **)outputFileNames, options.output_count));
        if (overwrite) {
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
      verboseLog(VERBOSE_NORMAL, "sheet size: %dx%d\n", sheet->width,
                 sheet->height);
      verboseLog(VERBOSE_NORMAL, "...\n");

      // -------------------------------------------------------
      // --- process image data                              ---
      // -------------------------------------------------------

      // stretch
      if (stretchSize[WIDTH] != -1) {
        w = stretchSize[WIDTH];
      } else {
        w = sheet->width;
      }
      if (stretchSize[HEIGHT] != -1) {
        h = stretchSize[HEIGHT];
      } else {
        h = sheet->height;
      }

      w *= zoomFactor;
      h *= zoomFactor;

      saveDebug("_before-stretch%d.pnm", nr, sheet);
      stretch_and_replace(&sheet, (RectangleSize){w, h}, interpolateType,
                          absBlackThreshold);
      saveDebug("_after-stretch%d.pnm", nr, sheet);

      // size
      if ((size[WIDTH] != -1) || (size[HEIGHT] != -1)) {
        if (size[WIDTH] != -1) {
          w = size[WIDTH];
        } else {
          w = sheet->width;
        }
        if (size[HEIGHT] != -1) {
          h = size[HEIGHT];
        } else {
          h = sheet->height;
        }
        saveDebug("_before-resize%d.pnm", nr, sheet);
        resize_and_replace(&sheet, (RectangleSize){w, h}, interpolateType,
                           sheetBackgroundPixel, absBlackThreshold);
        saveDebug("_after-resize%d.pnm", nr, sheet);
      }

      // handle sheet layout

      // LAYOUT_SINGLE
      if (options.layout == LAYOUT_SINGLE) {
        // set middle of sheet as single starting point for mask detection
        if (pointCount == 0) { // no manual settings, use auto-values
          points[pointCount++] = (Point){sheet->width / 2, sheet->height / 2};
        }
        if (maskScanMaximum[WIDTH] == -1) {
          maskScanMaximum[WIDTH] = sheet->width;
        }
        if (maskScanMaximum[HEIGHT] == -1) {
          maskScanMaximum[HEIGHT] = sheet->height;
        }
        // avoid inner half of the sheet to be blackfilter-detectable
        if (blackfilterExcludeCount ==
            0) { // no manual settings, use auto-values
          blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet->width / 4;
          blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
          blackfilterExclude[blackfilterExcludeCount][RIGHT] =
              sheet->width / 2 + sheet->width / 4;
          blackfilterExclude[blackfilterExcludeCount][BOTTOM] =
              sheet->height / 2 + sheet->height / 4;
          blackfilterExcludeCount++;
        }
        // set single outside border to start scanning for final border-scan
        if (outsideBorderscanMaskCount ==
            0) { // no manual settings, use auto-values
          outsideBorderscanMask[outsideBorderscanMaskCount++] =
              clip_rectangle(sheet, RECT_FULL_IMAGE);
        }

        // LAYOUT_DOUBLE
      } else if (options.layout == LAYOUT_DOUBLE) {
        // set two middle of left/right side of sheet as starting points for
        // mask detection
        if (pointCount == 0) { // no manual settings, use auto-values
          points[pointCount++] = (Point){sheet->width / 4, sheet->height / 2};
          points[pointCount++] =
              (Point){sheet->width - sheet->width / 4, sheet->height / 2};
        }
        if (maskScanMaximum[WIDTH] == -1) {
          maskScanMaximum[WIDTH] = sheet->width / 2;
        }
        if (maskScanMaximum[HEIGHT] == -1) {
          maskScanMaximum[HEIGHT] = sheet->height;
        }
        if (middleWipe[0] > 0 || middleWipe[1] > 0) { // left, right
          wipe[wipeCount++] = (Rectangle){{
              {sheet->width / 2 - middleWipe[0], 0},
              {sheet->width / 2 + middleWipe[1], sheet->height - 1},
          }};
        }
        // avoid inner half of each page to be blackfilter-detectable
        if (blackfilterExcludeCount ==
            0) { // no manual settings, use auto-values
          blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet->width / 8;
          blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
          blackfilterExclude[blackfilterExcludeCount][RIGHT] =
              sheet->width / 4 + sheet->width / 8;
          blackfilterExclude[blackfilterExcludeCount][BOTTOM] =
              sheet->height / 2 + sheet->height / 4;
          blackfilterExcludeCount++;
          blackfilterExclude[blackfilterExcludeCount][LEFT] =
              sheet->width / 2 + sheet->width / 8;
          blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
          blackfilterExclude[blackfilterExcludeCount][RIGHT] =
              sheet->width / 2 + sheet->width / 4 + sheet->width / 8;
          blackfilterExclude[blackfilterExcludeCount][BOTTOM] =
              sheet->height / 2 + sheet->height / 4;
          blackfilterExcludeCount++;
        }
        // set two outside borders to start scanning for final border-scan
        if (outsideBorderscanMaskCount ==
            0) { // no manual settings, use auto-values
          outsideBorderscanMask[outsideBorderscanMaskCount++] = (Rectangle){
              {POINT_ORIGIN, {sheet->width / 2, sheet->height - 1}}};
          outsideBorderscanMask[outsideBorderscanMaskCount++] = (Rectangle){
              {{sheet->width / 2, 0}, {sheet->width - 1, sheet->height - 1}}};
        }
      }
      // if maskScanMaximum still unset (no --layout specified), set to full
      // sheet size now
      if (maskScanMinimum[WIDTH] == -1) {
        maskScanMaximum[WIDTH] = sheet->width;
      }
      if (maskScanMinimum[HEIGHT] == -1) {
        maskScanMaximum[HEIGHT] = sheet->height;
      }

      // pre-wipe
      if (!isExcluded(nr, options.no_wipe_multi_index,
                      options.ignore_multi_index)) {
        apply_wipes(sheet, preWipe, preWipeCount, maskColorPixel,
                    absBlackThreshold);
      }

      // pre-border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, preBorder, maskColorPixel, absBlackThreshold);
      }

      // black area filter
      if (!isExcluded(nr, options.no_blackfilter_multi_index,
                      options.ignore_multi_index)) {
        saveDebug("_before-blackfilter%d.pnm", nr, sheet);
        blackfilter(sheet, blackfilterParams, absBlackThreshold);
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
            sheet, noisefilterIntensity, absWhiteThreshold, absBlackThreshold);
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
        uint64_t filterResult = blurfilter(
            sheet, blurfilterParams, absWhiteThreshold, absBlackThreshold);
        saveDebug("_after-blurfilter%d.pnm", nr, sheet);
        verboseLog(VERBOSE_NORMAL, " deleted %" PRIu64 " pixels.\n",
                   filterResult);

      } else {
        verboseLog(VERBOSE_MORE, "+ blurfilter DISABLED for sheet %d\n", nr);
      }

      // mask-detection
      if (!isExcluded(nr, options.no_mask_scan_multi_index,
                      options.ignore_multi_index)) {
        detect_masks(sheet, maskDetectionParams, points, pointCount, maskValid,
                     masks);
      } else {
        verboseLog(VERBOSE_MORE, "+ mask-scan DISABLED for sheet %d\n", nr);
      }

      // permanently apply masks
      if (maskCount > 0) {
        saveDebug("_before-masking%d.pnm", nr, sheet);
        apply_masks(sheet, masks, maskCount, maskColorPixel, absBlackThreshold);
        saveDebug("_after-masking%d.pnm", nr, sheet);
      }

      // gray filter
      if (!isExcluded(nr, options.no_grayfilter_multi_index,
                      options.ignore_multi_index)) {
        verboseLog(VERBOSE_NORMAL, "gray-filter...");

        saveDebug("_before-grayfilter%d.pnm", nr, sheet);
        uint64_t filterResult =
            grayfilter(sheet, grayfilterParams, absBlackThreshold);
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
          maskCount = detect_masks(sheet, maskDetectionParams, points,
                                   pointCount, maskValid, masks);
        } else {
          verboseLog(VERBOSE_MORE, "(mask-scan before deskewing disabled)\n");
        }

        // auto-deskew each mask
        for (int i = 0; i < maskCount; i++) {
          saveDebug("_before-deskew-detect%d.pnm", nr * maskCount + i, sheet);
          float rotation = detect_rotation(sheet, masks[i], deskewParams);
          saveDebug("_after-deskew-detect%d.pnm", nr * maskCount + i, sheet);

          verboseLog(VERBOSE_NORMAL, "rotate (%d,%d): %f\n", points[i].x,
                     points[i].y, rotation);

          if (rotation != 0.0) {
            AVFrame *rect =
                create_image(size_of_rectangle(masks[i]), sheet->format, false,
                             sheetBackgroundPixel, absBlackThreshold);
            AVFrame *rectTarget = create_image(
                (RectangleSize){rect->width, rect->height}, sheet->format, true,
                sheetBackgroundPixel, absBlackThreshold);

            // copy area to rotate into rSource
            copy_rectangle(sheet, rect,
                           (Rectangle){{masks[i].vertex[0], POINT_INFINITY}},
                           POINT_ORIGIN, absBlackThreshold);

            // rotate
            rotate(rect, rectTarget, -rotation, absBlackThreshold,
                   interpolateType);

            // copy result back into whole image
            copy_rectangle(rectTarget, sheet, RECT_FULL_IMAGE,
                           masks[i].vertex[0], absBlackThreshold);

            av_frame_free(&rect);
            av_frame_free(&rectTarget);
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
          maskCount = detect_masks(sheet, maskDetectionParams, points,
                                   pointCount, maskValid, masks);
        } else {
          verboseLog(VERBOSE_MORE, "(mask-scan before centering disabled)\n");
        }

        saveDebug("_before-centering%d.pnm", nr, sheet);
        // center masks on the sheet, according to their page position
        for (int i = 0; i < maskCount; i++) {
          center_mask(sheet, points[i], masks[i], sheetBackgroundPixel,
                      absBlackThreshold);
        }
        saveDebug("_after-centering%d.pnm", nr, sheet);
      } else {
        verboseLog(VERBOSE_MORE, "+ auto-centering DISABLED for sheet %d\n",
                   nr);
      }

      // explicit wipe
      if (!isExcluded(nr, options.no_wipe_multi_index,
                      options.ignore_multi_index)) {
        apply_wipes(sheet, wipe, wipeCount, maskColorPixel, absBlackThreshold);
      } else {
        verboseLog(VERBOSE_MORE, "+ wipe DISABLED for sheet %d\n", nr);
      }

      // explicit border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, border, maskColorPixel, absBlackThreshold);
      } else {
        verboseLog(VERBOSE_MORE, "+ border DISABLED for sheet %d\n", nr);
      }

      // border-detection
      if (!isExcluded(nr, options.no_border_scan_multi_index,
                      options.ignore_multi_index)) {
        Rectangle autoborderMask[outsideBorderscanMaskCount];
        saveDebug("_before-border%d.pnm", nr, sheet);
        for (int i = 0; i < outsideBorderscanMaskCount; i++) {
          autoborderMask[i] =
              border_to_mask(sheet, detect_border(sheet, borderScanParams,
                                                  outsideBorderscanMask[i],
                                                  absBlackThreshold));
        }
        apply_masks(sheet, autoborderMask, outsideBorderscanMaskCount,
                    maskColorPixel, absBlackThreshold);
        for (int i = 0; i < outsideBorderscanMaskCount; i++) {
          // border-centering
          if (!isExcluded(nr, options.no_border_align_multi_index,
                          options.ignore_multi_index)) {
            align_mask(sheet, autoborderMask[i], outsideBorderscanMask[i],
                       maskAlignmentParams, sheetBackgroundPixel,
                       absBlackThreshold);
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
        apply_wipes(sheet, postWipe, postWipeCount, maskColorPixel,
                    absBlackThreshold);
      }

      // post-border
      if (!isExcluded(nr, options.no_border_multi_index,
                      options.ignore_multi_index)) {
        apply_border(sheet, postBorder, maskColorPixel, absBlackThreshold);
      }

      // post-mirroring
      if (postMirror != 0) {
        verboseLog(VERBOSE_NORMAL, "post-mirroring %s\n",
                   getDirections(postMirror));
        mirror(sheet, !!(postMirror & (1 << HORIZONTAL)),
               !!(postMirror & (1 << VERTICAL)), absBlackThreshold);
      }

      // post-shifting
      if ((postShift[WIDTH] != 0) || ((postShift[HEIGHT] != 0))) {
        verboseLog(VERBOSE_NORMAL, "post-shifting [%d,%d]\n", postShift[WIDTH],
                   postShift[HEIGHT]);

        shift_image(&sheet, (Delta){postShift[WIDTH], postShift[HEIGHT]},
                    sheetBackgroundPixel, absBlackThreshold);
      }

      // post-rotating
      if (postRotate != 0) {
        verboseLog(VERBOSE_NORMAL, "post-rotating %d degrees.\n", postRotate);
        flip_rotate_90(&sheet, postRotate / 90, absBlackThreshold);
      }

      // post-stretch
      if (postStretchSize[WIDTH] != -1) {
        w = postStretchSize[WIDTH];
      } else {
        w = sheet->width;
      }
      if (postStretchSize[HEIGHT] != -1) {
        h = postStretchSize[HEIGHT];
      } else {
        h = sheet->height;
      }

      w *= postZoomFactor;
      h *= postZoomFactor;

      stretch_and_replace(&sheet, (RectangleSize){w, h}, interpolateType,
                          absBlackThreshold);

      // post-size
      if ((postSize[WIDTH] != -1) || (postSize[HEIGHT] != -1)) {
        if (postSize[WIDTH] != -1) {
          w = postSize[WIDTH];
        } else {
          w = sheet->width;
        }
        if (postSize[HEIGHT] != -1) {
          h = postSize[HEIGHT];
        } else {
          h = sheet->height;
        }
        resize_and_replace(&sheet, (RectangleSize){w, h}, interpolateType,
                           sheetBackgroundPixel, absBlackThreshold);
      }

      // --- write output file ---

      // write split pages output

      if (writeoutput == true) {
        verboseLog(VERBOSE_NORMAL, "writing output.\n");
        // write files
        saveDebug("_before-save%d.pnm", nr, sheet);

        if (outputPixFmt == -1) {
          outputPixFmt = sheet->format;
        }

        for (int j = 0; j < options.output_count; j++) {
          // get pagebuffer
          page = create_image(
              (RectangleSize){sheet->width / options.output_count,
                              sheet->height},
              sheet->format, false, sheetBackgroundPixel, absBlackThreshold);
          copy_rectangle(
              sheet, page,
              (Rectangle){{{page->width * j, 0},
                           {page->width * j + page->width, page->height}}},
              POINT_ORIGIN, absBlackThreshold);

          verboseLog(VERBOSE_MORE, "saving file %s.\n", outputFileNames[j]);

          saveImage(outputFileNames[j], page, outputPixFmt);

          av_frame_free(&page);
        }

        av_frame_free(&sheet);
        sheet = NULL;
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
