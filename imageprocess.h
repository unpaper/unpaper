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

double detectRotation(AVFrame *image, int mask[EDGES_COUNT]);

void rotate(const float radians, AVFrame *source, AVFrame *target);


/* --- stretching / resizing / shifting ------------------------------------ */


void stretch(int w, int h, AVFrame **image);

void resize(int w, int h, AVFrame **image);

void shift(int shiftX, int shiftY, AVFrame **image);


/* --- mask-detection ----------------------------------------------------- */


void detectMasks(AVFrame *image);

void applyMasks(int mask[MAX_MASKS][EDGES_COUNT], const int maskCount, AVFrame *image);


/* --- wiping ------------------------------------------------------------- */


void applyWipes(int area[MAX_MASKS][EDGES_COUNT], int areaCount, AVFrame *image);


/* --- mirroring ---------------------------------------------------------- */


void mirror(int directions, AVFrame *image);


/* --- flip-rotating ------------------------------------------------------ */


void flipRotate(int direction, AVFrame **image);


/* --- blackfilter -------------------------------------------------------- */


void blackfilter(AVFrame *image);


/* --- noisefilter -------------------------------------------------------- */


int noisefilter(AVFrame *image);


/* --- blurfilter --------------------------------------------------------- */

int blurfilter(AVFrame *image);


/* --- grayfilter --------------------------------------------------------- */


int grayfilter(AVFrame *image);


/* --- border-detection --------------------------------------------------- */


void centerMask(AVFrame *image, int center[COORDINATES_COUNT], int mask[EDGES_COUNT]);

void alignMask(int mask[EDGES_COUNT], int outside[EDGES_COUNT], AVFrame *image);

void detectBorder(int border[EDGES_COUNT], int outsideMask[EDGES_COUNT], AVFrame *image);

void borderToMask(int border[EDGES_COUNT], int mask[EDGES_COUNT], AVFrame *image);

void applyBorder(int border[EDGES_COUNT], AVFrame *image);
