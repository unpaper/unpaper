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

/****************************************************************************
 * image processing functions                                               *
 ****************************************************************************/


/* --- deskewing ---------------------------------------------------------- */

double detectRotation(struct IMAGE* image, int mask[EDGES_COUNT]);

void rotate(const float radians, struct IMAGE* source, struct IMAGE* target);


/* --- stretching / resizing / shifting ------------------------------------ */


void stretch(int w, int h, struct IMAGE* image);

void resize(int w, int h, struct IMAGE* image);

void shift(int shiftX, int shiftY, struct IMAGE* image);


/* --- mask-detection ----------------------------------------------------- */


void detectMasks(struct IMAGE* image);

void applyMasks(int mask[MAX_MASKS][EDGES_COUNT], const int maskCount, const int maskColor, struct IMAGE* image);


/* --- wiping ------------------------------------------------------------- */


void applyWipes(int area[MAX_MASKS][EDGES_COUNT], int areaCount, struct IMAGE* image);


/* --- mirroring ---------------------------------------------------------- */


void mirror(int directions, struct IMAGE* image);


/* --- flip-rotating ------------------------------------------------------ */


void flipRotate(int direction, struct IMAGE* image);


/* --- blackfilter -------------------------------------------------------- */


void blackfilter(struct IMAGE* image);


/* --- noisefilter -------------------------------------------------------- */


int noisefilter(struct IMAGE* image);


/* --- blurfilter --------------------------------------------------------- */

int blurfilter(struct IMAGE* image);


/* --- grayfilter --------------------------------------------------------- */


int grayfilter(struct IMAGE* image);


/* --- border-detection --------------------------------------------------- */


void centerMask(struct IMAGE* image, int center[COORDINATES_COUNT], int mask[EDGES_COUNT]);

void alignMask(int mask[EDGES_COUNT], int outside[EDGES_COUNT], int direction, int margin[DIRECTIONS_COUNT], struct IMAGE* image);

void detectBorder(int border[EDGES_COUNT], int outsideMask[EDGES_COUNT], struct IMAGE* image);

void borderToMask(int border[EDGES_COUNT], int mask[EDGES_COUNT], struct IMAGE* image);

void applyBorder(int border[EDGES_COUNT], int borderColor, struct IMAGE* image);
