/*
 * This file is part of Unpaper.
 *
 * Copyright © 2013 Michael McMaster <michael@codesrc.com>
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

#include "unpaper.h"
#include "interpolate.h"
#include "tools.h"

#include <math.h>
#include <stdlib.h>

static int nearest(float x, float y, struct IMAGE* source);
static int bilinearInterpolate(float x, float y, struct IMAGE* source);
static int bicubicInterpolate(float x, float y, struct IMAGE* source);
static void convertImageToFloat(struct IMAGE* source);

static int (*interpolateFunction)(float, float, struct IMAGE*) =
	bicubicInterpolate;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Public Methods
/////////////////////////////////////////////////////////////////////////////

void setInterpolateMethod(INTERPOLATE_METHOD method)
{
	switch (method)
	{
		case INTERPOLATE_NN:
			interpolateFunction = nearest;
			break;
		case INTERPOLATE_LINEAR:
			interpolateFunction = bilinearInterpolate;
			break;
		case INTERPOLATE_CUBIC:
			interpolateFunction = bicubicInterpolate;
			break;
		default:
			break; // ignore.
	};
}

void interpolateInit(struct IMAGE* source)
{
	convertImageToFloat(source);
}

int interpolate(float x, float y, struct IMAGE* source)
{
	return interpolateFunction(x, y, source);
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Private Methods
/////////////////////////////////////////////////////////////////////////////

/**
 * Convert the image from a packed 8-bit RGB format to a floating point unpacked
 * format to avoid bit manipulation overheads during interpolation.
 */
static void convertImageToFloat(struct IMAGE* source)
{
	size_t bufSize = source->width * source->height * 4 * sizeof(float);
	source->bufferFloat = realloc(source->bufferFloat, bufSize);

#pragma omp parallel for
	for (int y = 0; y < source->height; ++y)
	{
		for (int x = 0; x < source->width; ++x)
		{
			int pixel = getPixel(x, y, source);
			source->bufferFloat[y * source->width * 4 + x * 4] = red(pixel);
			source->bufferFloat[y * source->width * 4 + x * 4 + 1] = green(pixel);
			source->bufferFloat[y * source->width * 4 + x * 4 + 2] = blue(pixel);
			source->bufferFloat[y * source->width * 4 + x * 4 + 3] = 0;
		}
	}
}


/**
 * Nearest-neighbour interpolation.
 */
static int nearest(float x, float y, struct IMAGE* source)
{
	// Round to nearest location.
	int x1 = (int) roundf(x);
	int y1 = (int) roundf(y);
	return getPixel(x1, y1, source);
}

#ifdef HAVE_V4SF
// Make use of the GCC vectorization features. This allows SIMD compilation for
// much faster code.
typedef float v4sf __attribute__ ((vector_size(16)));
static v4sf white = {255,255,255,255};

static v4sf getFloatPixel(int x, int y, struct IMAGE* source)
{
	if (x < source->width && x >= 0 && y < source->height && y >= 0)
	{
		return *((v4sf*) (&source->bufferFloat[y * source->width * 4 + x * 4]));
	}
	else
	{
		return white;
	}
}

static int cvtFloatPixelToInt(v4sf val)
{
	return pixelValue((int) val[0], (int) val[1], (int) val[2]);
}
#endif



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Cubic methods
/////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_V4SF
/**
 * 1-D cubic interpolation. Clamps the return value between 0 and 255 to
 * support 8-bit colour images.
*/
static v4sf cubic(float x, v4sf a, v4sf b, v4sf c, v4sf d)
{
	v4sf result = b + 0.5f * x * (c - a + x * (2.0f * a - 5.0f * b + 4.0f * c - d + x * (3.0f * (b - c) + d - a)));
	for (int i = 0; i < 3; ++i)
	{
		if (result[i] > 255) result[i] = 255;
		else if (result[i] < 0) result[i] = 0;
	}
	return result;
}

/**
 * 2-D bicubic interpolation
*/
static int bicubicInterpolate(float x, float y, struct IMAGE* source)
{
	int fx = (int) x;
	int fy = (int) y;

	v4sf v[4];
	for (int i = -1; i < 3; ++i)
	{
		v[i+1] =
			cubic(
				x - fx,
				getFloatPixel(fx - 1, fy + i, source),
				getFloatPixel(fx, fy + i, source),
				getFloatPixel(fx + 1, fy + i, source),
				getFloatPixel(fx + 2, fy + i, source));
	}
	return cvtFloatPixelToInt(cubic(y - fy, v[0], v[1], v[2], v[3]));
}
#else

/**
 * 1-D cubic interpolation. Clamps the return value between 0 and 255 to
 * support 8-bit colour images.
*/
static int cubic(float x, int a, int b, int c, int d)
{
	int result = b + 0.5f * x * (c - a + x * (2.0f * a - 5.0f * b + 4.0f * c - d + x * (3.0f * (b - c) + d - a)));
	if (result > 255) result = 255;
	if (result < 0) result = 0;
	return result;
}

/**
 * 1-D cubic interpolation
 * This function expects (and returns) colour pixel values.
*/
static int cubicPixel(float x, int a, int b, int c, int d)
{
	int red = cubic(x, red(a), red(b), red(c), red(d));
	int green = cubic(x, green(a), green(b), green(c), green(d));
	int blue = cubic(x, blue(a), blue(b), blue(c), blue(d));
	return pixelValue(red, green, blue);
}

/**
 * 2-D bicubic interpolation
*/
static int bicubicInterpolate(float x, float y, struct IMAGE* source)
{
	int fx = (int) x;
	int fy = (int) y;

	int v[4];
	for (int i = -1; i < 3; ++i) {
		v[i+1] = cubicPixel(
				x - fx,
				getPixel(fx - 1, fy + i, source),
				getPixel(fx, fy + i, source),
				getPixel(fx + 1, fy + i, source),
				getPixel(fx + 2, fy + i, source));
	}
	return cvtFloatPixelToInt(cubic(y - fy, v[0], v[1], v[2], v[3]));
}
#endif

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Bilinear methods
/////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_V4SF
/**
 * 1-D linear interpolation.
*/
static v4sf linear(float x, v4sf a, v4sf b)
{
	return (1.0f - x) * a + x * b;
}

/**
 * 2-D bilinear interpolation
*/
static int bilinearInterpolate(float x, float y, struct IMAGE* source)
{
	int x1 = (int) x;
	int x2 = (int) ceilf(x);
	int y1 = (int) y;
	int y2 = (int) ceilf(y);

	// Check edge conditions to avoid divide-by-zero
	if (x2 > source->width || y2 > source->height)
		return getPixel(x, y, source);
	else if (x2 == x1 && y2 == y1)
		return getPixel(x, y, source);
	else if (x2 == x1)
	{
		v4sf p1 = getFloatPixel(x1, y1, source);
		v4sf p2 = getFloatPixel(x1, y2, source);
		return cvtFloatPixelToInt(linear(y - y1, p1, p2));
	} else if (y2 == y1)
	{
		v4sf p1 = getFloatPixel(x1, y1, source);
		v4sf p2 = getFloatPixel(x2, y1, source);
		return cvtFloatPixelToInt(linear(x - x1, p1, p2));
	}

	v4sf pixel1 = getFloatPixel(x1, y1, source);
	v4sf pixel2 = getFloatPixel(x2, y1, source);
	v4sf pixel3 = getFloatPixel(x1, y2, source);
	v4sf pixel4 = getFloatPixel(x2, y2, source);

	v4sf val1 = linear(x - x1, pixel1, pixel2);
	v4sf val2 = linear(x - x1, pixel3, pixel4);
	return cvtFloatPixelToInt(linear(y - y1, val1, val2));
}

#else
/**
 * 1-D linear interpolation.
*/
static int linear(float x, int a, int b)
{
	return (1.0f - x) * a + x * b;
}

/**
 * 1-D linear interpolation
 * This function expects (and returns) colour pixel values.
*/
static int linearPixel(float x, int a, int b)
{
	int red = linear(x, red(a), red(b));
	int green = linear(x, green(a), green(b));
	int blue = linear(x, blue(a), blue(b));
	return pixelValue(red, green, blue);
}

#endif


