/*
 * This file is part of Unpaper.
 *
 * Copyright © 2005-2007 Jens Gulden
 * Copyright © 2011-2011 Diego Elio Pettenò
 *
 * Unpaper is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * Unpaper is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

/* --- The main program  -------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <getopt.h>

#include <sys/stat.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#include "unpaper.h"
#include "imageprocess.h"
#include "tools.h"
#include "parse.h"

#define WELCOME                                                         \
    PACKAGE_STRING "\n"							\
    "License GPLv2: GNU GPL version 2.\n"				\
    "This is free software: you are free to change and redistribute it.\n" \
    "There is NO WARRANTY, to the extent permitted by law.\n"

#define USAGE                                                           \
    WELCOME "\n"                                                        \
    "Usage: unpaper [options] <input-file(s)> <output-file(s)>\n"	\
    "\n"								\
    "Filenames may contain a formatting placeholder starting with '%%' to insert a\n" \
    "page counter for multi-page processing. E.g.: 'scan%%03d.pbm' to process files\n" \
    "scan001.pbm, scan002.pbm, scan003.pbm etc.\n"			\
    "\n"								\
    "See 'man unpaper' for options details\n"				\
    "Report bugs at " PACKAGE_BUGREPORT "\n"

/* --- global variable ---------------------------------------------------- */

VERBOSE_LEVEL verbose = VERBOSE_NONE;

INTERP_FUNCTIONS interpolateType = INTERP_CUBIC;

unsigned int absBlackThreshold;
unsigned int absWhiteThreshold;
unsigned int absBlackfilterScanThreshold;
unsigned int absGrayfilterThreshold;
double deskewScanRangeRad;
double deskewScanStepRad;
double deskewScanDeviationRad;

int layout = LAYOUT_SINGLE;
int startSheet = 1;
int endSheet = -1;
int startInput = -1;
int startOutput = -1;
int inputCount = 1;
int outputCount = 1;
int sheetSize[DIMENSIONS_COUNT] = { -1, -1 };
int sheetBackground = WHITE24;
int preRotate = 0;
int postRotate = 0;
int preMirror = 0;
int postMirror = 0;
int preShift[DIRECTIONS_COUNT] = { 0, 0 };
int postShift[DIRECTIONS_COUNT] = { 0, 0 };
int size[DIRECTIONS_COUNT] = { -1, -1 };
int postSize[DIRECTIONS_COUNT] = { -1, -1 };
int stretchSize[DIRECTIONS_COUNT] = { -1, -1 };
int postStretchSize[DIRECTIONS_COUNT] = { -1, -1 };
float zoomFactor = 1.0;
float postZoomFactor = 1.0;
int pointCount = 0;
int point[MAX_POINTS][COORDINATES_COUNT];
int maskCount = 0;
int mask[MAX_MASKS][EDGES_COUNT];
int wipeCount = 0;
int wipe[MAX_MASKS][EDGES_COUNT];
int middleWipe[2] = { 0, 0 };
int preWipeCount = 0;
int preWipe[MAX_MASKS][EDGES_COUNT];
int postWipeCount = 0;
int postWipe[MAX_MASKS][EDGES_COUNT];
int preBorder[EDGES_COUNT] = { 0, 0, 0, 0 };
int postBorder[EDGES_COUNT] = { 0, 0, 0, 0 };
int border[EDGES_COUNT] = { 0, 0, 0, 0 };
bool maskValid[MAX_MASKS];
int preMaskCount = 0;
int preMask[MAX_MASKS][EDGES_COUNT];
int blackfilterScanDirections = (1<<HORIZONTAL) | (1<<VERTICAL);
int blackfilterScanSize[DIRECTIONS_COUNT] = { 20, 20 };
int blackfilterScanDepth[DIRECTIONS_COUNT] = { 500, 500 };
int blackfilterScanStep[DIRECTIONS_COUNT] = { 5, 5 };
float blackfilterScanThreshold = 0.95;
int blackfilterExcludeCount = 0;
int blackfilterExclude[MAX_MASKS][EDGES_COUNT];
int blackfilterIntensity = 20;
int noisefilterIntensity = 4;
int blurfilterScanSize[DIRECTIONS_COUNT] = { 100, 100 };
int blurfilterScanStep[DIRECTIONS_COUNT] = { 50, 50 };
float blurfilterIntensity = 0.01;
int grayfilterScanSize[DIRECTIONS_COUNT] = { 50, 50 };
int grayfilterScanStep[DIRECTIONS_COUNT] = { 20, 20 };
float grayfilterThreshold = 0.5;
int maskScanDirections = (1<<HORIZONTAL);
int maskScanSize[DIRECTIONS_COUNT] = { 50, 50 };
int maskScanDepth[DIRECTIONS_COUNT] = { -1, -1 };
int maskScanStep[DIRECTIONS_COUNT] = { 5, 5 };
float maskScanThreshold[DIRECTIONS_COUNT] = { 0.1, 0.1 };
int maskScanMinimum[DIMENSIONS_COUNT] = { 100, 100 };
int maskScanMaximum[DIMENSIONS_COUNT] = { -1, -1 }; // set default later
int maskColor = WHITE24;
int deskewScanEdges = (1<<LEFT) | (1<<RIGHT);
int deskewScanSize = 1500;
float deskewScanDepth = 0.5;
float deskewScanRange = 5.0;
float deskewScanStep = 0.1;
float deskewScanDeviation = 1.0;
int borderScanDirections = (1<<VERTICAL);
int borderScanSize[DIRECTIONS_COUNT] = { 5, 5 };
int borderScanStep[DIRECTIONS_COUNT] = { 5, 5 };
int borderScanThreshold[DIRECTIONS_COUNT] = { 5, 5 };
int borderAlign = 0; // center
int borderAlignMargin[DIRECTIONS_COUNT] = { 0, 0 }; // center
int outsideBorderscanMask[MAX_PAGES][EDGES_COUNT]; // set by --layout
int outsideBorderscanMaskCount = 0;
float whiteThreshold = 0.9;
float blackThreshold = 0.33;
bool writeoutput = true;
bool multisheets = true;

// 0: allow all, -1: disable all, n: individual entries
struct MultiIndex noBlackfilterMultiIndex = { 0, NULL };
struct MultiIndex noNoisefilterMultiIndex = { 0, NULL };
struct MultiIndex noBlurfilterMultiIndex = { 0, NULL };
struct MultiIndex noGrayfilterMultiIndex = { 0, NULL };
struct MultiIndex noMaskScanMultiIndex = { 0, NULL };
struct MultiIndex noMaskCenterMultiIndex = { 0, NULL };
struct MultiIndex noDeskewMultiIndex = { 0, NULL };
struct MultiIndex noWipeMultiIndex = { 0, NULL };
struct MultiIndex noBorderMultiIndex = { 0, NULL };
struct MultiIndex noBorderScanMultiIndex = { 0, NULL };
struct MultiIndex noBorderAlignMultiIndex = { 0, NULL };

// default: process all between start-sheet and end-sheet
struct MultiIndex sheetMultiIndex = { -1, NULL };
struct MultiIndex excludeMultiIndex = { 0, NULL };
struct MultiIndex ignoreMultiIndex = { 0, NULL };
struct MultiIndex insertBlank = { 0, NULL };
struct MultiIndex replaceBlank = { 0, NULL };

bool overwrite = false;
int dpi = 300;

/**
 * Print an error and exit process
 */
void errOutput(const char *fmt, ...) {
    va_list vl;

    fprintf(stderr, "unpaper: error: ");

    va_start(vl, fmt);
    vfprintf(stderr, fmt, vl);
    va_end(vl);

    fprintf(stderr, "\nTry 'man unpaper' for more information.\n");

    exit(1);
}

/****************************************************************************
 * MAIN()                                                                   *
 ****************************************************************************/

/**
 * The main program.
 */
int main(int argc, char* argv[]) {
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

    // -------------------------------------------------------------------
    // --- parse parameters                                            ---
    // -------------------------------------------------------------------

    while(true) {
        int c;

        static const struct option long_options[] = {
            { "help",                       no_argument,       NULL,  'h' },
            { "?",                          no_argument,       NULL,  'h' },
            { "version",                    no_argument,       NULL,  'V' },
            { "layout",                     required_argument, NULL,  'l' },
            { "#",                          required_argument, NULL,  '#' },
            { "sheet",                      required_argument, NULL,  '#' },
            { "start",                      required_argument, NULL, 0x7e },
            { "start-sheet",                required_argument, NULL, 0x7e },
            { "end",                        required_argument, NULL, 0x7f },
            { "end-sheet",                  required_argument, NULL, 0x7f },
            { "start-input",                required_argument, NULL, 0x80 },
            { "si",                         required_argument, NULL, 0x80 },
            { "start-ouput",                required_argument, NULL, 0x81 },
            { "so",                         required_argument, NULL, 0x81 },
            { "sheet-size",                 required_argument, NULL,  'S' },
            { "sheet-background",           required_argument, NULL, 0x82 },
            { "exclude",                    optional_argument, NULL,  'x' },
            { "no-processing",              required_argument, NULL,  'n' },
            { "pre-rotate",                 required_argument, NULL, 0x83 },
            { "post-rotate",                required_argument, NULL, 0x84 },
            { "pre-mirror",                 required_argument, NULL,  'M' },
            { "post-mirror",                required_argument, NULL, 0x85 },
            { "pre-shift",                  required_argument, NULL, 0x86 },
            { "post-shift",                 required_argument, NULL, 0x87 },
            { "pre-mask",                   required_argument, NULL, 0x88 },
            { "size",                       required_argument, NULL,  's' },
            { "post-size",                  required_argument, NULL, 0x89 },
            { "stretch",                    required_argument, NULL, 0x8a },
            { "post-stretch",               required_argument, NULL, 0x8b },
            { "zoom",                       required_argument, NULL,  'z' },
            { "post-zoom",                  required_argument, NULL, 0x8c },
            { "mask-scan-point",            required_argument, NULL,  'p' },
            { "mask",                       required_argument, NULL,  'm' },
            { "wipe",                       required_argument, NULL,  'W' },
            { "pre-wipe",                   required_argument, NULL, 0x8d },
            { "post-wipe",                  required_argument, NULL, 0x8e },
            { "middle-wipe",                required_argument, NULL, 0x8f },
            { "mw",                         required_argument, NULL, 0x8f },
            { "border",                     required_argument, NULL,  'B' },
            { "pre-border",                 required_argument, NULL, 0x90 },
            { "post-border",                required_argument, NULL, 0x91 },
            { "no-blackfilter",             optional_argument, NULL, 0x92 },
            { "blackfilter-scan-direction", required_argument, NULL, 0x93 },
            { "blackfilter-scan-size",      required_argument, NULL, 0x94 },
            { "bs",                         required_argument, NULL, 0x94 },
            { "blackfilter-scan-depth",     required_argument, NULL, 0x95 },
            { "bd",                         required_argument, NULL, 0x95 },
            { "blackfilter-scan-step",      required_argument, NULL, 0x96 },
            { "bp",                         required_argument, NULL, 0x96 },
            { "blackfilter-scan-threshold", required_argument, NULL, 0x97 },
            { "bt",                         required_argument, NULL, 0x97 },
            { "blackfilter-scan-exclude",   required_argument, NULL, 0x98 },
            { "bx",                         required_argument, NULL, 0x98 },
            { "blackfilter-intensity",      required_argument, NULL, 0x99 },
            { "bi",                         required_argument, NULL, 0x99 },
            { "no-noisefilter",             optional_argument, NULL, 0x9a },
            { "noisefilter-intensity",      required_argument, NULL, 0x9b },
            { "ni",                         required_argument, NULL, 0x9b },
            { "no-blurfilter",              optional_argument, NULL, 0x9c },
            { "blurfilter-size",            required_argument, NULL, 0x9d },
            { "ls",                         required_argument, NULL, 0x9d },
            { "blurfilter-step",            required_argument, NULL, 0x9e },
            { "lp",                         required_argument, NULL, 0x9e },
            { "blurfilter-intensity",       required_argument, NULL, 0x9f },
            { "li",                         required_argument, NULL, 0x9f },
            { "no-grayfilter",              optional_argument, NULL, 0xa0 },
            { "grayfilter-size",            required_argument, NULL, 0xa1 },
            { "gs",                         required_argument, NULL, 0xa1 },
            { "grayfilter-step",            required_argument, NULL, 0xa2 },
            { "gp",                         required_argument, NULL, 0xa2 },
            { "grayfilter-threshold",       required_argument, NULL, 0xa3 },
            { "gt",                         required_argument, NULL, 0xa3 },
            { "no-mask-scan",               optional_argument, NULL, 0xa4 },
            { "mask-scan-direction",        required_argument, NULL, 0xa5 },
            { "mn",                         required_argument, NULL, 0xa5 },
            { "mask-scan-size",             required_argument, NULL, 0xa6 },
            { "ms",                         required_argument, NULL, 0xa6 },
            { "mask-scan-depth",            required_argument, NULL, 0xa7 },
            { "md",                         required_argument, NULL, 0xa7 },
            { "mask-scan-step",             required_argument, NULL, 0xa8 },
            { "mp",                         required_argument, NULL, 0xa8 },
            { "mask-scan-threshold",        required_argument, NULL, 0xa9 },
            { "mt",                         required_argument, NULL, 0xa9 },
            { "mask-scan-minimum",          required_argument, NULL, 0xaa },
            { "mm",                         required_argument, NULL, 0xaa },
            { "mask-scan-maximum",          required_argument, NULL, 0xab },
            { "mM",                         required_argument, NULL, 0xab },
            { "mask-color",                 required_argument, NULL, 0xac },
            { "mc",                         required_argument, NULL, 0xac },
            { "no-mask-center",             optional_argument, NULL, 0xad },
            { "no-deskew",                  optional_argument, NULL, 0xae },
            { "deskew-scan-direction",      required_argument, NULL, 0xaf },
            { "dn",                         required_argument, NULL, 0xaf },
            { "deskew-scan-size",           required_argument, NULL, 0xb0 },
            { "ds",                         required_argument, NULL, 0xb0 },
            { "deskew-scan-depth",          required_argument, NULL, 0xb1 },
            { "dd",                         required_argument, NULL, 0xb1 },
            { "deskew-scan-range",          required_argument, NULL, 0xb2 },
            { "dr",                         required_argument, NULL, 0xb2 },
            { "deskew-scan-step",           required_argument, NULL, 0xb3 },
            { "dp",                         required_argument, NULL, 0xb3 },
            { "deskew-scan-deviation",      required_argument, NULL, 0xb4 },
            { "dv",                         required_argument, NULL, 0xb4 },
            { "no-border-scan",             optional_argument, NULL, 0xb5 },
            { "border-scan-direction",      required_argument, NULL, 0xb6 },
            { "Bn",                         required_argument, NULL, 0xb6 },
            { "border-scan-size",           required_argument, NULL, 0xb7 },
            { "Bs",                         required_argument, NULL, 0xb7 },
            { "border-scan-step",           required_argument, NULL, 0xb8 },
            { "Bp",                         required_argument, NULL, 0xb8 },
            { "border-scan-threshold",      required_argument, NULL, 0xb9 },
            { "Bt",                         required_argument, NULL, 0xb9 },
            { "border-align",               required_argument, NULL, 0xba },
            { "Ba",                         required_argument, NULL, 0xba },
            { "border-margin",              required_argument, NULL, 0xbb },
            { "Bm",                         required_argument, NULL, 0xbb },
            { "no-border-align",            optional_argument, NULL, 0xbc },
            { "no-wipe",                    optional_argument, NULL, 0xbd },
            { "no-border",                  optional_argument, NULL, 0xbe },
            { "white-threshold",            required_argument, NULL,  'w' },
            { "black-threshold",            required_argument, NULL,  'b' },
            { "input-pages",                required_argument, NULL, 0xbf },
            { "ip",                         required_argument, NULL, 0xbf },
            { "output-pages",               required_argument, NULL, 0xc0 },
            { "op",                         required_argument, NULL, 0xc0 },
            { "input-file-sequence",        required_argument, NULL, 0xc1 },
            { "if",                         required_argument, NULL, 0xc1 },
            { "output-file-sequence",       required_argument, NULL, 0xc2 },
            { "of",                         required_argument, NULL, 0xc2 },
            { "insert-blank",               required_argument, NULL, 0xc3 },
            { "replace-blank",              required_argument, NULL, 0xc4 },
            { "test-only",                  no_argument,       NULL,  'T' },
            { "no-multi-pages",             no_argument,       NULL, 0xc6 },
            { "dpi",                        required_argument, NULL, 0xc7 },
            { "type",                       required_argument, NULL,  't' },
            { "quiet",                      no_argument,       NULL,  'q' },
            { "overwrite",                  no_argument,       NULL, 0xc8 },
            { "verbose",                    no_argument,       NULL,  'v' },
            { "vv",                         no_argument,       NULL, 0xca },
            { "debug",                      no_argument,       NULL, 0xcb },
            { "vvv",                        no_argument,       NULL, 0xcb },
            { "debug-save",                 no_argument,       NULL, 0xcc },
            { "vvvv",                       no_argument,       NULL, 0xcc },
            { "interpolate",                required_argument, NULL, 0xcd },
            { NULL,                         no_argument,       NULL, 0    }
        };

        c = getopt_long_only(argc, argv, "hVl:S:x::n::M:s:z:p:m:W:B:w:b:Tt:qv",
                             long_options, &option_index);
        if (c == -1)
            break;

        switch(c) {
        case 'h':
        case '?':
            puts(USAGE);
            return c == '?' ? 1 : 0;

        case 'V':
            puts(VERSION);
            return 0;

        case 'l':
            if (strcmp(optarg, "single")==0) {
                layout = LAYOUT_SINGLE;
            } else if (strcmp(optarg, "double")==0) {
                layout = LAYOUT_DOUBLE;
            } else if (strcmp(optarg, "none")==0) {
                layout = LAYOUT_NONE;
            } else {
                errOutput("unknown layout mode '%s'.", optarg);
            }
            break;

        case '#':
            parseMultiIndex(optarg, &sheetMultiIndex);
            // allow 0 as start sheet, might be overwritten by --start-sheet again
            if (sheetMultiIndex.count > 0 && startSheet > sheetMultiIndex.indexes[0] )
                startSheet = sheetMultiIndex.indexes[0];
            break;

        case 0x7e:
            sscanf(optarg,"%d", &startSheet);
            break;

        case 0x7f:
            sscanf(optarg,"%d", &endSheet);
            break;

        case 0x80:
            sscanf(optarg,"%d", &startInput);
            break;

        case 0x81:
            sscanf(optarg,"%d", &startOutput);
            break;

        case 'S':
            parseSize(optarg, sheetSize, dpi);
            break;

        case 0x82:
            sheetBackground = parseColor(optarg);
            break;

        case 'x':
            parseMultiIndex(optarg, &excludeMultiIndex);
            if (excludeMultiIndex.count == -1)
                excludeMultiIndex.count = 0; // 'exclude all' makes no sense
            break;

        case 'n':
            parseMultiIndex(optarg, &ignoreMultiIndex);
            break;

        case 0x83:
            sscanf(optarg, "%d", &preRotate);
            if ((preRotate != 0) && (abs(preRotate) != 90)) {
                fprintf(stderr, "cannot set --pre-rotate value other than -90 or 90, ignoring.\n");
                preRotate = 0;
            }
            break;

        case 0x84:
            sscanf(optarg,"%d", &postRotate);
            if ((postRotate != 0) && (abs(postRotate) != 90)) {
                fprintf(stderr, "cannot set --post-rotate value other than -90 or 90, ignoring.\n");
                postRotate = 0;
            }
            break;

        case 'M':
            preMirror = parseDirections(optarg); // s = "v", "v,h", "vertical,horizontal", ...
            break;

        case 0x85:
            postMirror = parseDirections(optarg);
            break;

        case 0x86:
            parseSize(optarg, preShift, dpi);
            break;

        case 0x87:
            parseSize(optarg, postShift, dpi);
            break;

        case 0x88:
            if ( preMaskCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                preMask[preMaskCount][LEFT] = left;
                preMask[preMaskCount][TOP] = top;
                preMask[preMaskCount][RIGHT] = right;
                preMask[preMaskCount][BOTTOM] = bottom;
                preMaskCount++;
            } else {
                fprintf(stderr, "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 's':
            parseSize(optarg, size, dpi);
            break;

        case 0x89:
            parseSize(optarg, postSize, dpi);
            break;

        case 0x8a:
            parseSize(optarg, stretchSize, dpi);
            break;

        case 0x8b:
            parseSize(optarg, postStretchSize, dpi);
            break;

        case 'z':
            sscanf(optarg,"%f", &zoomFactor);
            break;

        case 0x8c:
            sscanf(optarg,"%f", &postZoomFactor);
            break;

        case 'p':
            if ( pointCount < MAX_POINTS ) {
                int x = -1;
                int y = -1;
                sscanf(optarg,"%d,%d", &x, &y);
                point[pointCount][X] = x;
                point[pointCount][Y] = y;
                pointCount++;
            } else {
                fprintf(stderr, "maximum number of scan points (%d) exceeded, ignoring scan point %s\n",
                        MAX_POINTS, optarg);
            }
            break;

        case 'm':
            if ( maskCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                mask[maskCount][LEFT] = left;
                mask[maskCount][TOP] = top;
                mask[maskCount][RIGHT] = right;
                mask[maskCount][BOTTOM] = bottom;
                maskValid[maskCount] = true;
                maskCount++;
            } else {
                fprintf(stderr, "maximum number of masks (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 'W':
            if ( wipeCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                wipe[wipeCount][LEFT] = left;
                wipe[wipeCount][TOP] = top;
                wipe[wipeCount][RIGHT] = right;
                wipe[wipeCount][BOTTOM] = bottom;
                wipeCount++;
            } else {
                fprintf(stderr, "maximum number of wipes (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 0x8d:
            if ( preWipeCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                preWipe[preWipeCount][LEFT] = left;
                preWipe[preWipeCount][TOP] = top;
                preWipe[preWipeCount][RIGHT] = right;
                preWipe[preWipeCount][BOTTOM] = bottom;
                preWipeCount++;
            } else {
                fprintf(stderr, "maximum number of pre-wipes (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 0x8e:
            if ( postWipeCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                postWipe[postWipeCount][LEFT] = left;
                postWipe[postWipeCount][TOP] = top;
                postWipe[postWipeCount][RIGHT] = right;
                postWipe[postWipeCount][BOTTOM] = bottom;
                postWipeCount++;
            } else {
                fprintf(stderr, "maximum number of post-wipes (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 0x8f:
            parseInts(optarg, middleWipe);
            break;

        case 'B':
            sscanf(optarg,"%d,%d,%d,%d", &border[LEFT], &border[TOP], &border[RIGHT], &border[BOTTOM]);
            break;

        case 0x90:
            sscanf(optarg,"%d,%d,%d,%d", &preBorder[LEFT], &preBorder[TOP], &preBorder[RIGHT], &preBorder[BOTTOM]);
            break;

        case 0x91:
            sscanf(optarg,"%d,%d,%d,%d", &postBorder[LEFT], &postBorder[TOP], &postBorder[RIGHT], &postBorder[BOTTOM]);
            break;

        case 0x92:
            parseMultiIndex(optarg, &noBlackfilterMultiIndex);
            break;

        case 0x93:
            blackfilterScanDirections = parseDirections(optarg);
            break;

        case 0x94:
            parseInts(optarg, blackfilterScanSize);
            break;

        case 0x95:
            parseInts(optarg, blackfilterScanDepth);
            break;

        case 0x96:
            parseInts(optarg, blackfilterScanStep);
            break;

        case 0x97:
            sscanf(optarg, "%f", &blackfilterScanThreshold);
            break;

        case 0x98:
            if ( blackfilterExcludeCount < MAX_MASKS ) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(optarg,"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                blackfilterExclude[blackfilterExcludeCount][LEFT] = left;
                blackfilterExclude[blackfilterExcludeCount][TOP] = top;
                blackfilterExclude[blackfilterExcludeCount][RIGHT] = right;
                blackfilterExclude[blackfilterExcludeCount][BOTTOM] = bottom;
                blackfilterExcludeCount++;
            } else {
                fprintf(stderr, "maximum number of blackfilter exclusion (%d) exceeded, ignoring mask %s\n",
                        MAX_MASKS, optarg);
            }
            break;

        case 0x99:
            sscanf(optarg, "%d", &blackfilterIntensity);
            break;

        case 0x9a:
            parseMultiIndex(optarg, &noNoisefilterMultiIndex);
            break;

        case 0x9b:
            sscanf(optarg, "%d", &noisefilterIntensity);
            break;

        case 0x9c:
            parseMultiIndex(optarg, &noBlurfilterMultiIndex);
            break;

        case 0x9d:
            parseInts(optarg, blurfilterScanSize);
            break;

        case 0x9e:
            parseInts(optarg, blurfilterScanStep);
            break;

        case 0x9f:
            sscanf(optarg, "%f", &blurfilterIntensity);
            break;

        case 0xa0:
            parseMultiIndex(optarg, &noGrayfilterMultiIndex);
            break;

        case 0xa1:
            parseInts(optarg, grayfilterScanSize);
            break;

        case 0xa2:
            parseInts(optarg, grayfilterScanStep);
            break;

        case 0xa3:
            sscanf(optarg, "%f", &grayfilterThreshold);
            break;

        case 0xa4:
            parseMultiIndex(optarg, &noMaskScanMultiIndex);
            break;

        case 0xa5:
            maskScanDirections = parseDirections(optarg);
            break;

        case 0xa6:
            parseInts(optarg, maskScanSize);
            break;

        case 0xa7:
            parseInts(optarg, maskScanDepth);
            break;

        case 0xa8:
            parseInts(optarg, maskScanStep);
            break;

        case 0xa9:
            parseFloats(optarg, maskScanThreshold);
            break;

        case 0xaa:
            sscanf(optarg,"%d,%d", &maskScanMinimum[WIDTH], &maskScanMinimum[HEIGHT]);
            break;

        case 0xab:
            sscanf(optarg,"%d,%d", &maskScanMaximum[WIDTH], &maskScanMaximum[HEIGHT]);
            break;

        case 0xac:
            sscanf(optarg,"%d", &maskColor);
            break;

        case 0xad:
            parseMultiIndex(optarg, &noMaskCenterMultiIndex);
            break;

        case 0xae:
            parseMultiIndex(optarg, &noDeskewMultiIndex);
            break;

        case 0xaf:
            deskewScanEdges = parseEdges(optarg);
            break;

        case 0xb0:
            sscanf(optarg,"%d", &deskewScanSize);
            break;

        case 0xb1:
            sscanf(optarg,"%f", &deskewScanDepth);
            break;

        case 0xb2:
            sscanf(optarg,"%f", &deskewScanRange);
            break;

        case 0xb3:
            sscanf(optarg,"%f", &deskewScanStep);
            break;

        case 0xb4:
            sscanf(optarg,"%f", &deskewScanDeviation);
            break;

        case 0xb5:
            parseMultiIndex(optarg, &noBorderScanMultiIndex);
            break;

        case 0xb6:
            borderScanDirections = parseDirections(optarg);
            break;

        case 0xb7:
            parseInts(optarg, borderScanSize);
            break;

        case 0xb8:
            parseInts(optarg, borderScanStep);
            break;

        case 0xb9:
            parseInts(optarg, borderScanThreshold);
            break;

        case 0xba:
            borderAlign = parseEdges(optarg);
            break;

        case 0xbb:
            parseSize(optarg, borderAlignMargin, dpi);
            break;

        case 0xbc:
            parseMultiIndex(optarg, &noBorderAlignMultiIndex);
            break;

        case 0xbd:
            parseMultiIndex(optarg, &noWipeMultiIndex);
            break;

        case 0xbe:
            parseMultiIndex(optarg, &noBorderMultiIndex);
            break;

        case 'w':
            sscanf(optarg,"%f", &whiteThreshold);
            break;

        case 'b':
            sscanf(optarg,"%f", &blackThreshold);
            break;

        case 0xbf:
            sscanf(optarg,"%d", &inputCount);
            if ( ! (inputCount >= 1 && inputCount <= 2 ) ) {
                fprintf(stderr, "cannot set --input-pages value other than 1 or 2, ignoring.\n");
                inputCount = 1;
            }

            break;

        case 0xc0:
            sscanf(optarg,"%d", &outputCount);
            if ( ! (outputCount >= 1 && outputCount <= 2 ) ) {
                fprintf(stderr, "cannot set --output-pages value other than 1 or 2, ignoring.\n");
                outputCount = 1;
            }

            break;

        case 0xc1:
        case 0xc2:
            errOutput("--input-file-sequence and --output-file sequence are deprecated and unimplemented.\n"
                    "Please pass input output pairs as arguments to unpaper instead.");
            break;

        case 0xc3:
            parseMultiIndex(optarg, &insertBlank);
            break;

        case 0xc4:
            parseMultiIndex(optarg, &replaceBlank);
            break;

        case 'T':
            writeoutput = false;
            break;

        case 0xc5:
            // Deprecated function, ignore.
            break;

        case 0xc6:
            multisheets = false;
            break;

        case 0xc7:
            sscanf(optarg,"%d", &dpi);
            break;

        case 't':
            if ( strcmp(optarg, "pbm") == 0 ) {
                outputPixFmt = AV_PIX_FMT_MONOWHITE;
            } else if ( strcmp(optarg, "pgm") == 0 ) {
                outputPixFmt = AV_PIX_FMT_GRAY8;
            } else if ( strcmp(optarg, "ppm") == 0 ) {
                outputPixFmt = AV_PIX_FMT_RGB24;
            }
            break;

        case 'q':
            verbose = VERBOSE_QUIET;
            break;

        case 0xc8:
            overwrite = true;
            break;

        case 'v':
            verbose = VERBOSE_NORMAL;
            break;

        case 0xca:
            verbose = VERBOSE_MORE;
            break;

        case 0xcb:
            verbose = VERBOSE_DEBUG;
            break;

        case 0xcc:
            verbose = VERBOSE_DEBUG_SAVE;
            break;

        case 0xcd:
            if (strcmp(optarg, "nearest") == 0) {
                interpolateType = INTERP_NN;
            } else if (strcmp(optarg, "linear") == 0) {
                interpolateType = INTERP_LINEAR;
            } else if (strcmp(optarg, "cubic") == 0) {
                interpolateType = INTERP_CUBIC;
            }
            else
            {
                fprintf(stderr, "Could not parse --interpolate, using cubic as default.\n");
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
    if ( optind + 2 > argc )
        errOutput("no input or output files given.\n");

    if ( verbose >= VERBOSE_NORMAL )
        printf(WELCOME); // welcome message

    if (startInput == -1)
        startInput = (startSheet - 1) * inputCount + 1;
    if (startOutput == -1)
        startOutput = (startSheet - 1) * outputCount + 1;

    inputNr = startInput;
    outputNr = startOutput;

    if ( ! multisheets && endSheet == -1)
        endSheet = startSheet;

    // Calculate the constant absolute values based on the relative parameters.
    absBlackThreshold = WHITE * (1.0 - blackThreshold);
    absWhiteThreshold = WHITE * (whiteThreshold);
    absBlackfilterScanThreshold = WHITE * (blackfilterScanThreshold);
    absGrayfilterThreshold = WHITE * (grayfilterThreshold);
    deskewScanRangeRad = degreesToRadians(deskewScanRange);
    deskewScanStepRad = degreesToRadians(deskewScanStep);
    deskewScanDeviationRad = degreesToRadians(deskewScanDeviation);

    avcodec_register_all();
    av_register_all();

    for (int nr = startSheet; (endSheet == -1) || (nr <= endSheet); nr++) {
        char inputFilesBuffer[2][255];
        char outputFilesBuffer[2][255];
        char *inputFileNames[2];
        char *outputFileNames[2];

        // -------------------------------------------------------------------
        // --- begin processing                                            ---
        // -------------------------------------------------------------------

        bool inputWildcard = multisheets && (strchr(argv[optind], '%') != NULL);
        for (int i = 0; i < inputCount; i++) {
            bool ins = isInMultiIndex(inputNr, insertBlank);
            bool repl = isInMultiIndex(inputNr, replaceBlank);

            if ( repl ) {
                inputFileNames[i] = NULL;
                inputNr++; /* replace */
            } else if ( ins ) {
                inputFileNames[i] = NULL; /* insert */
            } else if ( inputWildcard ) {
                sprintf(inputFilesBuffer[i], argv[optind], inputNr++);
                inputFileNames[i] = inputFilesBuffer[i];
            } else if ( optind >= argc ) {
                if ( endSheet == -1 ) {
                    endSheet = nr-1;
                    goto sheet_end;
                } else {
                    errOutput("not enough input files given.");
                }
            } else {
                inputFileNames[i] = argv[optind++];
            }

            if ( inputFileNames[i] != NULL ) {
                struct stat statBuf;
                if ( stat(inputFileNames[i], &statBuf) != 0 ) {
                    if ( endSheet == -1 ) {
                        endSheet = nr-1;
                        goto sheet_end;
                    } else {
                        errOutput("unable to open file %s.",
                                  inputFileNames[i]);
                    }
                }
            }
        }
        if ( inputWildcard )
            optind++;

        bool outputWildcard = multisheets && (strchr(argv[optind], '%') != NULL);
        for(int i = 0; i < outputCount; i++) {
            if ( outputWildcard ) {
                sprintf(outputFilesBuffer[i], argv[optind], outputNr++);
                outputFileNames[i] = outputFilesBuffer[i];
            } else if ( optind >= argc ) {
                errOutput("not enough output files given.");
                return -1;
            } else {
                outputFileNames[i] = argv[optind++];
            }

            if ( ! overwrite ) {
                struct stat statbuf;
                if ( stat(outputFileNames[i], &statbuf) == 0 ) {
                    errOutput("output file '%s' already present.\n",
                              outputFileNames[i]);
                }
            }
        }
        if ( outputWildcard )
            optind++;

        // ---------------------------------------------------------------
        // --- process single sheet                                    ---
        // ---------------------------------------------------------------

        if (isInMultiIndex(nr, sheetMultiIndex) && (!isInMultiIndex(nr, excludeMultiIndex))) {
            char s1[1023]; // buffers for result of implode()
            char s2[1023];

            if (verbose >= VERBOSE_NORMAL) {
                printf("\n-------------------------------------------------------------------------------\n");
            }
            if (verbose > VERBOSE_QUIET) {
                if (multisheets) {
                    printf("Processing sheet #%d: %s -> %s\n", nr, implode(s1, (const char **)inputFileNames, inputCount), implode(s2, (const char **)outputFileNames, outputCount));
                } else {
                    printf("Processing sheet: %s -> %s\n", implode(s1, (const char **)inputFileNames, inputCount), implode(s2, (const char **)outputFileNames, outputCount));
                }
            }

            // load input image(s)
            for (int j = 0; j < inputCount; j++) {
                if (inputFileNames[j] != NULL) { // may be null if --insert-blank or --replace-blank
                    if (verbose >= VERBOSE_MORE)
                        printf("loading file %s.\n", inputFileNames[j]);

                    loadImage(inputFileNames[j], &page);
                    saveDebug("_loaded_%d.pnm", inputNr-inputCount+j, page);

                    if (outputPixFmt == -1 && page != NULL) {
                          outputPixFmt = page->format;
                    }

                    // pre-rotate
                    if (preRotate != 0) {
                        if (verbose>=VERBOSE_NORMAL) {
                            printf("pre-rotating %d degrees.\n", preRotate);
                        }
                        flipRotate(preRotate/90, &page);
                    }

                    // if sheet-size is not known yet (and not forced by --sheet-size), set now based on size of (first) input image
                    if ( w == -1 ) {
                        if ( sheetSize[WIDTH] != -1 ) {
                            w = sheetSize[WIDTH];
                        } else {
                            w = page->width * inputCount;
                        }
                    }
                    if ( h == -1 ) {
                        if ( sheetSize[HEIGHT] != -1 ) {
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
                    initImage(&sheet, w, h, AV_PIX_FMT_RGB24, true);

                }
                if (page != NULL) {
                    saveDebug("_page%d.pnm", inputNr-inputCount+j, page);
                    saveDebug("_before_center_page%d.pnm", inputNr-inputCount+j, sheet);

                    centerImage(page, (w * j / inputCount), 0, (w / inputCount), h, sheet);

                    saveDebug("_after_center_page%d.pnm", inputNr-inputCount+j, sheet);
                }
            }

            // the only case that buffer is not yet initialized is if all blank pages have been inserted
            if (sheet == NULL) {
                // last chance: try to get previous (unstretched/not zoomed) sheet size
                w = previousWidth;
                h = previousHeight;
                if (verbose >= VERBOSE_NORMAL) {
                    printf("need to guess sheet size from previous sheet: %dx%d\n", w, h);
                }
                if ((w == -1) || (h == -1)) {
                    errOutput("sheet size unknown, use at least one input file per sheet, or force using --sheet-size.");
                } else {
                    initImage(&sheet, w, h, AV_PIX_FMT_RGB24, true);
                }
            }

            previousWidth = w;
            previousHeight = h;

            // pre-mirroring
            if (preMirror != 0) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("pre-mirroring %s\n", getDirections(preMirror));
                }
                mirror(preMirror, sheet);
            }

            // pre-shifting
            if ((preShift[WIDTH] != 0) || ((preShift[HEIGHT] != 0))) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("pre-shifting [%d,%d]\n", preShift[WIDTH], preShift[HEIGHT]);
                }
                shift(preShift[WIDTH], preShift[HEIGHT], &sheet);
            }

            // pre-masking
            if (preMaskCount > 0) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("pre-masking\n ");
                }
                applyMasks(preMask, preMaskCount, sheet);
            }


            // --------------------------------------------------------------
            // --- verbose parameter output,                              ---
            // --------------------------------------------------------------

            // parameters and size are known now

            if (verbose >= VERBOSE_MORE) {
                switch(layout) {
                case LAYOUT_SINGLE:
                    printf("layout: single\n");
                    break;
                case LAYOUT_DOUBLE:
                    printf("layout: double\n");
                    break;
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
                        printf("[%d,%d,%d,%d] ",preWipe[i][LEFT],preWipe[i][TOP],preWipe[i][RIGHT],preWipe[i][BOTTOM]);
                    }
                    printf("\n");
                }
                if (preBorder[LEFT]!=0 || preBorder[TOP]!=0 || preBorder[RIGHT]!=0 || preBorder[BOTTOM]!=0) {
                    printf("pre-border: [%d,%d,%d,%d]\n", preBorder[LEFT], preBorder[TOP], preBorder[RIGHT], preBorder[BOTTOM]);
                }
                if (preMaskCount > 0) {
                    printf("pre-masking: ");
                    for (int i = 0; i < preMaskCount; i++) {
                        printf("[%d,%d,%d,%d] ",preMask[i][LEFT],preMask[i][TOP],preMask[i][RIGHT],preMask[i][BOTTOM]);
                    }
                    printf("\n");
                }
                if ((stretchSize[WIDTH] != -1) || (stretchSize[HEIGHT] != -1)) {
                    printf("stretch to: %dx%d\n", stretchSize[WIDTH], stretchSize[HEIGHT]);
                }
                if ((postStretchSize[WIDTH] != -1) || (postStretchSize[HEIGHT] != -1)) {
                    printf("post-stretch to: %dx%d\n", postStretchSize[WIDTH], postStretchSize[HEIGHT]);
                }
                if (zoomFactor != 1.0) {
                    printf("zoom: %f\n", zoomFactor);
                }
                if (postZoomFactor != 1.0) {
                    printf("post-zoom: %f\n", postZoomFactor);
                }
                if (noBlackfilterMultiIndex.count != -1) {
                    printf("blackfilter-scan-direction: %s\n", getDirections(blackfilterScanDirections));
                    printf("blackfilter-scan-size: [%d,%d]\n", blackfilterScanSize[0], blackfilterScanSize[1]);
                    printf("blackfilter-scan-depth: [%d,%d]\n", blackfilterScanDepth[0], blackfilterScanDepth[1]);
                    printf("blackfilter-scan-step: [%d,%d]\n", blackfilterScanStep[0], blackfilterScanStep[1]);
                    printf("blackfilter-scan-threshold: %f\n", blackfilterScanThreshold);
                    if (blackfilterExcludeCount > 0) {
                        printf("blackfilter-scan-exclude: ");
                        for (int i = 0; i < blackfilterExcludeCount; i++) {
                            printf("[%d,%d,%d,%d] ",blackfilterExclude[i][LEFT],blackfilterExclude[i][TOP],blackfilterExclude[i][RIGHT],blackfilterExclude[i][BOTTOM]);
                        }
                        printf("\n");
                    }
                    printf("blackfilter-intensity: %d\n", blackfilterIntensity);
                    if (noBlackfilterMultiIndex.count > 0) {
                        printf("blackfilter DISABLED for sheets: ");
                        printMultiIndex(noBlackfilterMultiIndex);
                    }
                } else {
                    printf("blackfilter DISABLED for all sheets.\n");
                }
                if (noNoisefilterMultiIndex.count != -1) {
                    printf("noisefilter-intensity: %d\n", noisefilterIntensity);
                    if (noNoisefilterMultiIndex.count > 0) {
                        printf("noisefilter DISABLED for sheets: ");
                        printMultiIndex(noNoisefilterMultiIndex);
                    }
                } else {
                    printf("noisefilter DISABLED for all sheets.\n");
                }
                if (noBlurfilterMultiIndex.count != -1) {
                    printf("blurfilter-size: [%d,%d]\n", blurfilterScanSize[0], blurfilterScanSize[1]);
                    printf("blurfilter-step: [%d,%d]\n", blurfilterScanStep[0], blurfilterScanStep[1]);
                    printf("blurfilter-intensity: %f\n", blurfilterIntensity);
                    if (noBlurfilterMultiIndex.count > 0) {
                        printf("blurfilter DISABLED for sheets: ");
                        printMultiIndex(noBlurfilterMultiIndex);
                    }
                } else {
                    printf("blurfilter DISABLED for all sheets.\n");
                }
                if (noGrayfilterMultiIndex.count != -1) {
                    printf("grayfilter-size: [%d,%d]\n", grayfilterScanSize[0], grayfilterScanSize[1]);
                    printf("grayfilter-step: [%d,%d]\n", grayfilterScanStep[0], grayfilterScanStep[1]);
                    printf("grayfilter-threshold: %f\n", grayfilterThreshold);
                    if (noGrayfilterMultiIndex.count > 0) {
                        printf("grayfilter DISABLED for sheets: ");
                        printMultiIndex(noGrayfilterMultiIndex);
                    }
                } else {
                    printf("grayfilter DISABLED for all sheets.\n");
                }
                if (noMaskScanMultiIndex.count != -1) {
                    printf("mask points: ");
                    for (int i = 0; i < pointCount; i++) {
                        printf("(%d,%d) ",point[i][X],point[i][Y]);
                    }
                    printf("\n");
                    printf("mask-scan-direction: %s\n", getDirections(maskScanDirections));
                    printf("mask-scan-size: [%d,%d]\n", maskScanSize[0], maskScanSize[1]);
                    printf("mask-scan-depth: [%d,%d]\n", maskScanDepth[0], maskScanDepth[1]);
                    printf("mask-scan-step: [%d,%d]\n", maskScanStep[0], maskScanStep[1]);
                    printf("mask-scan-threshold: [%f,%f]\n", maskScanThreshold[0], maskScanThreshold[1]);
                    printf("mask-scan-minimum: [%d,%d]\n", maskScanMinimum[0], maskScanMinimum[1]);
                    printf("mask-scan-maximum: [%d,%d]\n", maskScanMaximum[0], maskScanMaximum[1]);
                    printf("mask-color: %d\n", maskColor);
                    if (noMaskScanMultiIndex.count > 0) {
                        printf("mask-scan DISABLED for sheets: ");
                        printMultiIndex(noMaskScanMultiIndex);
                    }
                } else {
                    printf("mask-scan DISABLED for all sheets.\n");
                }
                if (noDeskewMultiIndex.count != -1) {
                    printf("deskew-scan-direction: ");
                    printEdges(deskewScanEdges);
                    printf("deskew-scan-size: %d\n", deskewScanSize);
                    printf("deskew-scan-depth: %f\n", deskewScanDepth);
                    printf("deskew-scan-range: %f\n", deskewScanRange);
                    printf("deskew-scan-step: %f\n", deskewScanStep);
                    printf("deskew-scan-deviation: %f\n", deskewScanDeviation);
                    if (noDeskewMultiIndex.count > 0) {
                        printf("deskew-scan DISABLED for sheets: ");
                        printMultiIndex(noDeskewMultiIndex);
                    }
                } else {
                    printf("deskew-scan DISABLED for all sheets.\n");
                }
                if (noWipeMultiIndex.count != -1) {
                    if (wipeCount > 0) {
                        printf("wipe areas: ");
                        for (int i = 0; i < wipeCount; i++) {
                            printf("[%d,%d,%d,%d] ", wipe[i][LEFT], wipe[i][TOP], wipe[i][RIGHT], wipe[i][BOTTOM]);
                        }
                        printf("\n");
                    }
                } else {
                    printf("wipe DISABLED for all sheets.\n");
                }
                if (middleWipe[0] > 0 || middleWipe[1] > 0) {
                    printf("middle-wipe (l,r): %d,%d\n", middleWipe[0], middleWipe[1]);
                }
                if (noBorderMultiIndex.count != -1) {
                    if (border[LEFT]!=0 || border[TOP]!=0 || border[RIGHT]!=0 || border[BOTTOM]!=0) {
                        printf("explicit border: [%d,%d,%d,%d]\n", border[LEFT], border[TOP], border[RIGHT], border[BOTTOM]);
                    }
                } else {
                    printf("border DISABLED for all sheets.\n");
                }
                if (noBorderScanMultiIndex.count != -1) {
                    printf("border-scan-direction: %s\n", getDirections(borderScanDirections));
                    printf("border-scan-size: [%d,%d]\n", borderScanSize[0], borderScanSize[1]);
                    printf("border-scan-step: [%d,%d]\n", borderScanStep[0], borderScanStep[1]);
                    printf("border-scan-threshold: [%d,%d]\n", borderScanThreshold[0], borderScanThreshold[1]);
                    if (noBorderScanMultiIndex.count > 0) {
                        printf("border-scan DISABLED for sheets: ");
                        printMultiIndex(noBorderScanMultiIndex);
                    }
                    printf("border-align: ");
                    printEdges(borderAlign);
                    printf("border-margin: [%d,%d]\n", borderAlignMargin[0], borderAlignMargin[1]);
                } else {
                    printf("border-scan DISABLED for all sheets.\n");
                }
                if (postWipeCount > 0) {
                    printf("post-wipe: ");
                    for (int i = 0; i < postWipeCount; i++) {
                        printf("[%d,%d,%d,%d] ",postWipe[i][LEFT],postWipe[i][TOP],postWipe[i][RIGHT],postWipe[i][BOTTOM]);
                    }
                    printf("\n");
                }
                if (postBorder[LEFT]!=0 || postBorder[TOP]!=0 || postBorder[RIGHT]!=0 || postBorder[BOTTOM]!=0) {
                    printf("post-border: [%d,%d,%d,%d]\n", postBorder[LEFT], postBorder[TOP], postBorder[RIGHT], postBorder[BOTTOM]);
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
                //if (ignoreMultiIndex.count > 0) {
                //    printf("EXCLUDE sheets: ");
                //    printMultiIndex(ignoreMultiIndex);
                //}
                printf("white-threshold: %f\n", whiteThreshold);
                printf("black-threshold: %f\n", blackThreshold);
                printf("sheet-background: %s %6x\n", ((sheetBackground == BLACK24) ? "black" : "white"), sheetBackground );
                printf("dpi: %d\n", dpi);
                printf("input-files per sheet: %d\n", inputCount);
                printf("output-files per sheet: %d\n", outputCount);
                if ((sheetSize[WIDTH] != -1) || (sheetSize[HEIGHT] != -1)) {
                    printf("sheet size forced to: %d x %d pixels\n", sheetSize[WIDTH], sheetSize[HEIGHT]);
                }
                printf("input-file-sequence:  %s\n", implode(s1, (const char **)inputFileNames, inputCount));
                printf("output-file-sequence: %s\n", implode(s1, (const char **)outputFileNames, outputCount));
                if (overwrite) {
                    printf("OVERWRITING EXISTING FILES\n");
                }
                printf("\n");
            }
            if (verbose >= VERBOSE_NORMAL) {
                printf("input-file%s for sheet %d: %s\n", pluralS(inputCount), nr, implode(s1, (const char **)inputFileNames, inputCount));
                printf("output-file%s for sheet %d: %s\n", pluralS(outputCount), nr, implode(s1, (const char **)outputFileNames, outputCount));
                printf("sheet size: %dx%d\n", sheet->width, sheet->height);
                printf("...\n");
            }


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
            stretch(w, h, &sheet);
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
                resize(w, h, &sheet);
                saveDebug("_after-resize%d.pnm", nr, sheet);
            }


            // handle sheet layout

            // LAYOUT_SINGLE
            if (layout == LAYOUT_SINGLE) {
                // set middle of sheet as single starting point for mask detection
                if (pointCount == 0) { // no manual settings, use auto-values
                    point[pointCount][X] = sheet->width / 2;
                    point[pointCount][Y] = sheet->height / 2;
                    pointCount++;
                }
                if (maskScanMaximum[WIDTH] == -1) {
                    maskScanMaximum[WIDTH] = sheet->width;
                }
                if (maskScanMaximum[HEIGHT] == -1) {
                    maskScanMaximum[HEIGHT] = sheet->height;
                }
                // avoid inner half of the sheet to be blackfilter-detectable
                if (blackfilterExcludeCount == 0) { // no manual settings, use auto-values
                    blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet->width / 4;
                    blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
                    blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet->width / 2 + sheet->width / 4;
                    blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet->height / 2 + sheet->height / 4;
                    blackfilterExcludeCount++;
                }
                // set single outside border to start scanning for final border-scan
                if (outsideBorderscanMaskCount == 0) { // no manual settings, use auto-values
                    outsideBorderscanMaskCount = 1;
                    outsideBorderscanMask[0][LEFT] = 0;
                    outsideBorderscanMask[0][RIGHT] = sheet->width - 1;
                    outsideBorderscanMask[0][TOP] = 0;
                    outsideBorderscanMask[0][BOTTOM] = sheet->height - 1;
                }

                // LAYOUT_DOUBLE
            } else if (layout == LAYOUT_DOUBLE) {
                // set two middle of left/right side of sheet as starting points for mask detection
                if (pointCount == 0) { // no manual settings, use auto-values
                    point[pointCount][X] = sheet->width / 4;
                    point[pointCount][Y] = sheet->height / 2;
                    pointCount++;
                    point[pointCount][X] = sheet->width - sheet->width / 4;
                    point[pointCount][Y] = sheet->height / 2;
                    pointCount++;
                }
                if (maskScanMaximum[WIDTH] == -1) {
                    maskScanMaximum[WIDTH] = sheet->width / 2;
                }
                if (maskScanMaximum[HEIGHT] == -1) {
                    maskScanMaximum[HEIGHT] = sheet->height;
                }
                if (middleWipe[0] > 0 || middleWipe[1] > 0) { // left, right
                    wipe[wipeCount][LEFT] = sheet->width / 2 - middleWipe[0];
                    wipe[wipeCount][TOP] = 0;
                    wipe[wipeCount][RIGHT] =  sheet->width / 2 + middleWipe[1];
                    wipe[wipeCount][BOTTOM] = sheet->height - 1;
                    wipeCount++;
                }
                // avoid inner half of each page to be blackfilter-detectable
                if (blackfilterExcludeCount == 0) { // no manual settings, use auto-values
                    blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet->width / 8;
                    blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
                    blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet->width / 4 + sheet->width / 8;
                    blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet->height / 2 + sheet->height / 4;
                    blackfilterExcludeCount++;
                    blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet->width / 2 + sheet->width / 8;
                    blackfilterExclude[blackfilterExcludeCount][TOP] = sheet->height / 4;
                    blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet->width / 2 + sheet->width / 4 + sheet->width / 8;
                    blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet->height / 2 + sheet->height / 4;
                    blackfilterExcludeCount++;
                }
                // set two outside borders to start scanning for final border-scan
                if (outsideBorderscanMaskCount == 0) { // no manual settings, use auto-values
                    outsideBorderscanMaskCount = 2;
                    outsideBorderscanMask[0][LEFT] = 0;
                    outsideBorderscanMask[0][RIGHT] = sheet->width / 2;
                    outsideBorderscanMask[0][TOP] = 0;
                    outsideBorderscanMask[0][BOTTOM] = sheet->height - 1;
                    outsideBorderscanMask[1][LEFT] = sheet->width / 2;
                    outsideBorderscanMask[1][RIGHT] = sheet->width - 1;
                    outsideBorderscanMask[1][TOP] = 0;
                    outsideBorderscanMask[1][BOTTOM] = sheet->height - 1;
                }
            }
            // if maskScanMaximum still unset (no --layout specified), set to full sheet size now
            if (maskScanMinimum[WIDTH] == -1) {
                maskScanMaximum[WIDTH] = sheet->width;
            }
            if (maskScanMinimum[HEIGHT] == -1) {
                maskScanMaximum[HEIGHT] = sheet->height;
            }


            // pre-wipe
            if (!isExcluded(nr, noWipeMultiIndex, ignoreMultiIndex)) {
                applyWipes(preWipe, preWipeCount, sheet);
            }

            // pre-border
            if (!isExcluded(nr, noBorderMultiIndex, ignoreMultiIndex)) {
                applyBorder(preBorder, sheet);
            }

            // black area filter
            if (!isExcluded(nr, noBlackfilterMultiIndex, ignoreMultiIndex)) {
                saveDebug("_before-blackfilter%d.pnm", nr, sheet);
                blackfilter(sheet);
                saveDebug("_after-blackfilter%d.pnm", nr, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ blackfilter DISABLED for sheet %d\n", nr);
                }
            }

            // noise filter
            if (!isExcluded(nr, noNoisefilterMultiIndex, ignoreMultiIndex)) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("noise-filter ...");
                }
                saveDebug("_before-noisefilter%d.pnm", nr, sheet);
                int filterResult = noisefilter(sheet);
                saveDebug("_after-noisefilter%d.pnm", nr, sheet);
                if (verbose >= VERBOSE_NORMAL) {
                    printf(" deleted %d clusters.\n", filterResult);
                }
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ noisefilter DISABLED for sheet %d\n", nr);
                }
            }

            // blur filter
            if (!isExcluded(nr, noBlurfilterMultiIndex, ignoreMultiIndex)) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("blur-filter...");
                }
                saveDebug("_before-blurfilter%d.pnm", nr, sheet);
                int filterResult = blurfilter(sheet);
                saveDebug("_after-blurfilter%d.pnm", nr, sheet);
                if (verbose >= VERBOSE_NORMAL) {
                    printf(" deleted %d pixels.\n", filterResult);
                }
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ blurfilter DISABLED for sheet %d\n", nr);
                }
            }

            // mask-detection
            if (!isExcluded(nr, noMaskScanMultiIndex, ignoreMultiIndex)) {
                detectMasks(sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ mask-scan DISABLED for sheet %d\n", nr);
                }
            }

            // permamently apply masks
            if (maskCount > 0) {
                saveDebug("_before-masking%d.pnm", nr, sheet);
                applyMasks(mask, maskCount, sheet);
                saveDebug("_after-masking%d.pnm", nr, sheet);
            }

            // gray filter
            if (!isExcluded(nr, noGrayfilterMultiIndex, ignoreMultiIndex)) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("gray-filter...");
                }
                saveDebug("_before-grayfilter%d.pnm", nr, sheet);
                int filterResult = grayfilter(sheet);
                saveDebug("_after-grayfilter%d.pnm", nr, sheet);
                if (verbose >= VERBOSE_NORMAL) {
                    printf(" deleted %d pixels.\n", filterResult);
                }
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ grayfilter DISABLED for sheet %d\n", nr);
                }
            }

            // rotation-detection
            if ((!isExcluded(nr, noDeskewMultiIndex, ignoreMultiIndex))) {
                saveDebug("_before-deskew%d.pnm", nr, sheet);

                // detect masks again, we may get more precise results now after first masking and grayfilter
                if (!isExcluded(nr, noMaskScanMultiIndex, ignoreMultiIndex)) {
                    detectMasks(sheet);
                } else {
                    if (verbose >= VERBOSE_MORE) {
                        printf("(mask-scan before deskewing disabled)\n");
                    }
                }

                // auto-deskew each mask
                for (int i = 0; i < maskCount; i++) {
                    saveDebug("_before-deskew-detect%d.pnm", nr*maskCount+i, sheet);
                    float rotation = detectRotation(sheet, mask[i]);
                    saveDebug("_after-deskew-detect%d.pnm", nr*maskCount+i, sheet);

                    if (verbose >= VERBOSE_NORMAL) {
                        printf("rotate (%d,%d): %f\n", point[i][X], point[i][Y], rotation);
                    }

                    if (rotation != 0.0) {
                        AVFrame *rect;
                        AVFrame *rectTarget;
                        initImage(&rect, (mask[i][RIGHT]-mask[i][LEFT]+1), (mask[i][BOTTOM]-mask[i][TOP]+1), sheet->format, false);
                        initImage(&rectTarget, rect->width, rect->height, sheet->format, true);

                        // copy area to rotate into rSource
                        copyImageArea(mask[i][LEFT], mask[i][TOP], rect->width, rect->height, sheet, 0, 0, rect);

                        // rotate
                        rotate(-rotation, rect, rectTarget);

                        // copy result back into whole image
                        copyImageArea(0, 0, rectTarget->width, rectTarget->height, rectTarget, mask[i][LEFT], mask[i][TOP], sheet);

                        av_frame_free(&rect);
                        av_frame_free(&rectTarget);
                    }
                }

                saveDebug("_after-deskew%d.pnm", nr, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ deskewing DISABLED for sheet %d\n", nr);
                }
            }

            // auto-center masks on either single-page or double-page layout
            if ( !isExcluded(nr, noMaskCenterMultiIndex, ignoreMultiIndex) ) { // (maskCount==pointCount to make sure all masks had correctly been detected)
                // perform auto-masking again to get more precise masks after rotation
                if (!isExcluded(nr, noMaskScanMultiIndex, ignoreMultiIndex)) {
                    detectMasks(sheet);
                } else {
                    if (verbose >= VERBOSE_MORE) {
                        printf("(mask-scan before centering disabled)\n");
                    }
                }

                saveDebug("_before-centering%d.pnm", nr, sheet);
                // center masks on the sheet, according to their page position
                for (int i = 0; i < maskCount; i++) {
                    centerMask(sheet, point[i], mask[i]);
                }
                saveDebug("_after-centering%d.pnm", nr, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ auto-centering DISABLED for sheet %d\n", nr);
                }
            }

            // explicit wipe
            if (!isExcluded(nr, noWipeMultiIndex, ignoreMultiIndex)) {
                applyWipes(wipe, wipeCount, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ wipe DISABLED for sheet %d\n", nr);
                }
            }

            // explicit border
            if (!isExcluded(nr, noBorderMultiIndex, ignoreMultiIndex)) {
                applyBorder(border, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ border DISABLED for sheet %d\n", nr);
                }
            }

            // border-detection
            if (!isExcluded(nr, noBorderScanMultiIndex, ignoreMultiIndex)) {
                int autoborder[MAX_MASKS][EDGES_COUNT];
                int autoborderMask[MAX_MASKS][EDGES_COUNT];
                saveDebug("_before-border%d.pnm", nr, sheet);
                for (int i = 0; i < outsideBorderscanMaskCount; i++) {
                    detectBorder(autoborder[i], outsideBorderscanMask[i], sheet);
                    borderToMask(autoborder[i], autoborderMask[i], sheet);
                }
                applyMasks(autoborderMask, outsideBorderscanMaskCount, sheet);
                for (int i = 0; i < outsideBorderscanMaskCount; i++) {
                    // border-centering
                    if (!isExcluded(nr, noBorderAlignMultiIndex, ignoreMultiIndex)) {
                        alignMask(autoborderMask[i], outsideBorderscanMask[i], sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ border-centering DISABLED for sheet %d\n", nr);
                        }
                    }
                }
                saveDebug("_after-border%d.pnm", nr, sheet);
            } else {
                if (verbose >= VERBOSE_MORE) {
                    printf("+ border-scan DISABLED for sheet %d\n", nr);
                }
            }

            // post-wipe
            if (!isExcluded(nr, noWipeMultiIndex, ignoreMultiIndex)) {
                applyWipes(postWipe, postWipeCount, sheet);
            }

            // post-border
            if (!isExcluded(nr, noBorderMultiIndex, ignoreMultiIndex)) {
                applyBorder(postBorder, sheet);
            }

            // post-mirroring
            if (postMirror != 0) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("post-mirroring %s\n", getDirections(postMirror));
                }
                mirror(postMirror, sheet);
            }

            // post-shifting
            if ((postShift[WIDTH] != 0) || ((postShift[HEIGHT] != 0))) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("post-shifting [%d,%d]\n", postShift[WIDTH], postShift[HEIGHT]);
                }
                shift(postShift[WIDTH], postShift[HEIGHT], &sheet);
            }

            // post-rotating
            if (postRotate != 0) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("post-rotating %d degrees.\n", postRotate);
                }
                flipRotate(postRotate/90, &sheet);
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

            stretch(w, h, &sheet);

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
                resize(w, h, &sheet);
            }

            // --- write output file ---

            // write split pages output

            if (writeoutput == true) {
                if (verbose >= VERBOSE_NORMAL) {
                    printf("writing output.\n");
                }
                // write files
                saveDebug("_before-save%d.pnm", nr, sheet);

                if ( outputPixFmt == -1 ) {
                    outputPixFmt = sheet->format;
                }

                for (int j = 0; j < outputCount; j++) {
                    // get pagebuffer
                    initImage(&page, sheet->width / outputCount, sheet->height, sheet->format, false);
                    copyImageArea(page->width * j, 0, page->width, page->height, sheet, 0, 0, page);

                    if (verbose >= VERBOSE_MORE) {
                        printf("saving file %s.\n", outputFileNames[j]);
                    }

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
        if ( optind >= argc && !inputWildcard )
            break;
        else if ( inputWildcard && outputWildcard )
            optind -= 2;
    }

    return 0;
}
