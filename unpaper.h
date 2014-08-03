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

/* --- preprocessor macros ------------------------------------------------ */
              
#define max(a, b) ( (a >= b) ? (a) : (b) )
#define pluralS(i) ( (i > 1) ? "s" : "" )
#define pixelValue(r, g, b) ( (r)<<16 | (g)<<8 | (b) )
#define pixelGrayscaleValue(g) ( (g)<<16 | (g)<<8 | (g) )
#define pixelGrayscale(r, g, b) ( ( ( r == g ) && ( r == b ) ) ? r : ( ( r + g + b ) / 3 ) ) // average (optimized for already gray values)
#define pixelLightness(r, g, b) ( r < g ? ( r < b ? r : b ) : ( g < b ? g : b ) ) // minimum
#define pixelDarknessInverse(r, g, b) ( r > g ? ( r > b ? r : b ) : ( g > b ? g : b ) ) // maximum
#define red(pixel) ( (pixel >> 16) & 0xff )
#define green(pixel) ( (pixel >> 8) & 0xff )
#define blue(pixel) ( pixel & 0xff )


/* --- preprocessor constants ---------------------------------------------- */
              
#define MAX_MULTI_INDEX 10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_ROTATION_SCAN_SIZE 10000 // maximum pixel count of virtual line to detect rotation with
#define MAX_MASKS 100
#define MAX_POINTS 100
#define MAX_FILES 100
#define MAX_PAGES 2
#define WHITE 255
#define GRAY 127
#define BLACK 0
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
	BRIGHT,
	DARK,
	SHADINGS_COUNT
} SHADINGS;

typedef enum {
	RED,
	GREEN,
	BLUE,
	COLORCOMPONENTS_COUNT
} COLORCOMPONENTS;

typedef enum {
	PBM,
	PGM,
	PPM,
	FILETYPES_COUNT
} FILETYPES;

typedef enum {
	INTERP_NN,
	INTERP_LINEAR,
	INTERP_CUBIC,
	INTERP_FUNCTIONS_COUNT
} INTERP_FUNCTIONS;


/* --- struct ------------------------------------------------------------- */

struct IMAGE {
    uint8_t *buffer;
    int width;
    int height;
    int stride;
    int background;
    bool color;
};

void errOutput(const char *fmt, ...)
    __attribute__((format(printf, 1, 2)))
    __attribute__((noreturn));

/* --- global variable ---------------------------------------------------- */

extern VERBOSE_LEVEL verbose;
extern INTERP_FUNCTIONS interpolateType;
