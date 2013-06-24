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

#ifndef INTERPOLATE_H
#define INTERPOLATE_H

typedef enum {
	INTERPOLATE_NN,
	INTERPOLATE_LINEAR,
	INTERPOLATE_CUBIC,
	INTERPOLATE_FUNCTIONS_COUNT
} INTERPOLATE_METHOD;

/** Set the method to use when interpolating.

    If not set, the default is INTERPOLATE_CUBIC
*/
void setInterpolateMethod(INTERPOLATE_METHOD method);

/** Prepare the source image for interpolation processing.

    This function creates a copy of the image data using a
    floating-point representation which speeds up the
    interpolation processing.

    interpolateInit must be called before calling interpolate(), and
    whenever the source image is modified.
*/
void interpolateInit(struct IMAGE* source);

/** Interpolate a colour at the given location.

    It is legal to call interpolate while x and y are
    located outside the image boundaries. In such cases
    a white colour is returned.
    
    @returns an RGB colour value represented as packed 8-bit unsigned
    values.

    @pre interpolateInit has been called.
*/
int interpolate(float x, float y, struct IMAGE* source);


#endif
