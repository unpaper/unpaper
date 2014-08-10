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

/* --- preprocessor macros ------------------------------------------------ */

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

#define pluralS(i) ( (i > 1) ? "s" : "" )
#define red(pixel) ( (pixel >> 16) & 0xff )
#define green(pixel) ( (pixel >> 8) & 0xff )
#define blue(pixel) ( pixel & 0xff )

static inline int pixelValue(uint8_t r, uint8_t g, uint8_t b) {
    return (r)<<16 | (g)<<8 | (b);
}

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


/* --- struct ------------------------------------------------------------- */

struct IMAGE {
    int background;
    AVFrame *frame;
};

void errOutput(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)))
    __attribute__((noreturn));

/* --- global variable ---------------------------------------------------- */

extern VERBOSE_LEVEL verbose;
extern INTERP_FUNCTIONS interpolateType;
