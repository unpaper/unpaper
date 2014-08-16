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

/* --- global declarations ------------------------------------------------ */


#if HAVE_STDBOOL_H
# include <stdbool.h>
#else
typedef enum {
    FALSE,
    TRUE
} BOOLEAN;

# define bool BOOLEAN
# define false FALSE
# define true TRUE
#endif

#include <libavutil/frame.h>
#include <math.h>

/* --- preprocessor macros ------------------------------------------------ */

#define pluralS(i) ( (i > 1) ? "s" : "" )

/* --- preprocessor constants ---------------------------------------------- */

#define MAX_MULTI_INDEX 10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_ROTATION_SCAN_SIZE 10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_MASKS 100
#define MAX_POINTS 100
#define MAX_FILES 100
#define MAX_PAGES 2
#define WHITE 0xFF
#define GRAY 0x1F
#define BLACK 0x00
#define WHITE24 0xFFFFFF
#define GRAY24 0x1F1F1F
#define BLACK24 0x000000
#define BLANK_TEXT "<blank>"


/* --- typedefs ----------------------------------------------------------- */

typedef enum {
    VERBOSE_QUIET = -1,
    VERBOSE_NONE = 0,
    VERBOSE_NORMAL = 1,
    VERBOSE_MORE = 2,
    VERBOSE_DEBUG = 3,
    VERBOSE_DEBUG_SAVE = 4
} VERBOSE_LEVEL;

typedef enum {
    X,
    Y,
    COORDINATES_COUNT
} COORDINATES;

typedef enum {
    WIDTH,
    HEIGHT,
    DIMENSIONS_COUNT
} DIMENSIONS;

typedef enum {
    HORIZONTAL,
    VERTICAL,
    DIRECTIONS_COUNT
} DIRECTIONS;

typedef enum {
    LEFT,
    TOP,
    RIGHT,
    BOTTOM,
    EDGES_COUNT
} EDGES;

typedef enum {
    LAYOUT_NONE,
    LAYOUT_SINGLE,
    LAYOUT_DOUBLE,
    LAYOUTS_COUNT
} LAYOUTS;

typedef enum {
    INTERP_NN,
    INTERP_LINEAR,
    INTERP_CUBIC,
    INTERP_FUNCTIONS_COUNT
} INTERP_FUNCTIONS;


void errOutput(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)))
    __attribute__((noreturn));

/* --- global variable ---------------------------------------------------- */

extern VERBOSE_LEVEL verbose;
extern INTERP_FUNCTIONS interpolateType;

extern unsigned int absBlackThreshold;
extern unsigned int absWhiteThreshold;
extern unsigned int absBlackfilterScanThreshold;
extern unsigned int absGrayfilterThreshold;
extern double deskewScanRangeRad;
extern double deskewScanStepRad;
extern double deskewScanDeviationRad;

extern int layout;
extern int startSheet;
extern int endSheet;
extern int startInput;
extern int startOutput;
extern int inputCount;
extern int outputCount;
extern int sheetSize[DIMENSIONS_COUNT];
extern int sheetBackground;
extern int preRotate;
extern int postRotate;
extern int preMirror;
extern int postMirror;
extern int preShift[DIRECTIONS_COUNT];
extern int postShift[DIRECTIONS_COUNT];
extern int size[DIRECTIONS_COUNT];
extern int postSize[DIRECTIONS_COUNT];
extern int stretchSize[DIRECTIONS_COUNT];
extern int postStretchSize[DIRECTIONS_COUNT];
extern float zoomFactor;
extern float postZoomFactor;
extern int pointCount;
extern int point[MAX_POINTS][COORDINATES_COUNT];
extern int maskCount;
extern int mask[MAX_MASKS][EDGES_COUNT];
extern int wipeCount;
extern int wipe[MAX_MASKS][EDGES_COUNT];
extern int middleWipe[2];
extern int preWipeCount;
extern int preWipe[MAX_MASKS][EDGES_COUNT];
extern int postWipeCount;
extern int postWipe[MAX_MASKS][EDGES_COUNT];
extern int preBorder[EDGES_COUNT];
extern int postBorder[EDGES_COUNT];
extern int border[EDGES_COUNT];
extern bool maskValid[MAX_MASKS];
extern int preMaskCount;
extern int preMask[MAX_MASKS][EDGES_COUNT];
extern int blackfilterScanDirections;
extern int blackfilterScanSize[DIRECTIONS_COUNT];
extern int blackfilterScanDepth[DIRECTIONS_COUNT];
extern int blackfilterScanStep[DIRECTIONS_COUNT];
extern float blackfilterScanThreshold;
extern int blackfilterExcludeCount;
extern int blackfilterExclude[MAX_MASKS][EDGES_COUNT];
extern int blackfilterIntensity;
extern int noisefilterIntensity;
extern int blurfilterScanSize[DIRECTIONS_COUNT];
extern int blurfilterScanStep[DIRECTIONS_COUNT];
extern float blurfilterIntensity;
extern int grayfilterScanSize[DIRECTIONS_COUNT];
extern int grayfilterScanStep[DIRECTIONS_COUNT];
extern float grayfilterThreshold;
extern int maskScanDirections;
extern int maskScanSize[DIRECTIONS_COUNT];
extern int maskScanDepth[DIRECTIONS_COUNT];
extern int maskScanStep[DIRECTIONS_COUNT];
extern float maskScanThreshold[DIRECTIONS_COUNT];
extern int maskScanMinimum[DIMENSIONS_COUNT];
extern int maskScanMaximum[DIMENSIONS_COUNT]; // set default later
extern int maskColor;
extern int deskewScanEdges;
extern int deskewScanSize;
extern float deskewScanDepth;
extern float deskewScanRange;
extern float deskewScanStep;
extern float deskewScanDeviation;
extern int borderScanDirections;
extern int borderScanSize[DIRECTIONS_COUNT];
extern int borderScanStep[DIRECTIONS_COUNT];
extern int borderScanThreshold[DIRECTIONS_COUNT];
extern int borderAlign; // center
extern int borderAlignMargin[DIRECTIONS_COUNT]; // center
extern int outsideBorderscanMask[MAX_PAGES][EDGES_COUNT]; // set by --layout
extern int outsideBorderscanMaskCount;
extern float whiteThreshold;
extern float blackThreshold;
extern bool writeoutput;
extern bool multisheets;

extern struct MultiIndex noBlackfilterMultiIndex;
extern struct MultiIndex noNoisefilterMultiIndex;
extern struct MultiIndex noBlurfilterMultiIndex;
extern struct MultiIndex noGrayfilterMultiIndex;
extern struct MultiIndex noMaskScanMultiIndex;
extern struct MultiIndex noMaskCenterMultiIndex;
extern struct MultiIndex noDeskewMultiIndex;
extern struct MultiIndex noWipeMultiIndex;
extern struct MultiIndex noBorderMultiIndex;
extern struct MultiIndex noBorderScanMultiIndex;
extern struct MultiIndex noBorderAlignMultiIndex;
extern struct MultiIndex sheetMultiIndex;
extern struct MultiIndex excludeMultiIndex;
extern struct MultiIndex ignoreMultiIndex;
extern struct MultiIndex insertBlank;
extern struct MultiIndex replaceBlank;

extern int autoborder[MAX_MASKS][EDGES_COUNT];
extern int autoborderMask[MAX_MASKS][EDGES_COUNT];
extern bool overwrite;
extern int dpi;

/* --- tool function for file handling ------------------------------------ */

void loadImage(const char *filename, AVFrame **image);

void saveImage(char *filename, AVFrame *image, int outputPixFmt);

void saveDebug(char *filenameTemplate, int index, AVFrame *image)
    __attribute__((format(printf, 1, 0)));

/* --- arithmetic tool functions ------------------------------------------ */

static inline double degreesToRadians(double d) {
    return d * M_PI / 180.0;
}

static inline void limit(int* i, int max) {
    if (*i > max) {
        *i = max;
    }
}

#define max(a, b)                               \
    ({ __typeof__ (a) _a = (a);                 \
        __typeof__ (b) _b = (b);                \
        _a > _b ? _a : _b; })

#define max3(a, b, c)                                                   \
    ({ __typeof__ (a) _a = (a);                                         \
        __typeof__ (b) _b = (b);                                        \
        __typeof__ (c) _c = (c);                                        \
        ( _a > _b ? ( _a > _c ? _a : _c ) : ( _b > _c ? _b : _c ) ); })

#define min3(a, b, c)                                                   \
    ({ __typeof__ (a) _a = (a);                                         \
        __typeof__ (b) _b = (b);                                        \
        __typeof__ (c) _c = (c);                                        \
        ( _a < _b ? ( _a < _c ? _a : _c ) : ( _b < _c ? _b : _c ) ); })

#define red(pixel) ( (pixel >> 16) & 0xff )
#define green(pixel) ( (pixel >> 8) & 0xff )
#define blue(pixel) ( pixel & 0xff )

static inline int pixelValue(uint8_t r, uint8_t g, uint8_t b) {
    return (r)<<16 | (g)<<8 | (b);
}
