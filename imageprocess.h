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

double detectRotation(int deskewScanEdges, double deskewScanRange, double deskewScanStep, int deskewScanSize, float deskewScanDepth, float deskewScanDeviation, int left, int top, int right, int bottom, struct IMAGE* image);

void rotate(const double radians, struct IMAGE* source, struct IMAGE* target);


/* --- stretching / resizing / shifting ------------------------------------ */


void stretch(int w, int h, struct IMAGE* image);

void resize(int w, int h, struct IMAGE* image);

void shift(int shiftX, int shiftY, struct IMAGE* image);


/* --- mask-detection ----------------------------------------------------- */


int detectMasks(int mask[MAX_MASKS][EDGES_COUNT], bool maskValid[MAX_MASKS], int point[MAX_POINTS][COORDINATES_COUNT], int pointCount, int maskScanDirections, int maskScanSize[DIRECTIONS_COUNT], int maskScanDepth[DIRECTIONS_COUNT], int maskScanStep[DIRECTIONS_COUNT], float maskScanThreshold[DIRECTIONS_COUNT], int maskScanMinimum[DIMENSIONS_COUNT], int maskScanMaximum[DIMENSIONS_COUNT],  struct IMAGE* image);

void applyMasks(int mask[MAX_MASKS][EDGES_COUNT], const int maskCount, const int maskColor, struct IMAGE* image);


/* --- wiping ------------------------------------------------------------- */


void applyWipes(int area[MAX_MASKS][EDGES_COUNT], int areaCount, int wipeColor, struct IMAGE* image);


/* --- mirroring ---------------------------------------------------------- */


void mirror(int directions, struct IMAGE* image);


/* --- flip-rotating ------------------------------------------------------ */


void flipRotate(int direction, struct IMAGE* image);


/* --- blackfilter -------------------------------------------------------- */


void blackfilter(int blackfilterScanDirections, int blackfilterScanSize[DIRECTIONS_COUNT], int blackfilterScanDepth[DIRECTIONS_COUNT], int blackfilterScanStep[DIRECTIONS_COUNT], float blackfilterScanThreshold, int blackfilterExclude[MAX_MASKS][EDGES_COUNT], int blackfilterExcludeCount, int blackfilterIntensity, float blackThreshold, struct IMAGE* image);


/* --- noisefilter -------------------------------------------------------- */


int noisefilter(int intensity, float whiteThreshold, struct IMAGE* image);


/* --- blurfilter --------------------------------------------------------- */

int blurfilter(int blurfilterScanSize[DIRECTIONS_COUNT], int blurfilterScanStep[DIRECTIONS_COUNT], float blurfilterIntensity, float whiteThreshold, struct IMAGE* image);


/* --- grayfilter --------------------------------------------------------- */


int grayfilter(int grayfilterScanSize[DIRECTIONS_COUNT], int grayfilterScanStep[DIRECTIONS_COUNT], float grayfilterThreshold, float blackThreshold, struct IMAGE* image);


/* --- border-detection --------------------------------------------------- */


void centerMask(int centerX, int centerY, int left, int top, int right, int bottom, struct IMAGE* image);

void alignMask(int mask[EDGES_COUNT], int outside[EDGES_COUNT], int direction, int margin[DIRECTIONS_COUNT], struct IMAGE* image);

void detectBorder(int border[EDGES_COUNT], int borderScanDirections, int borderScanSize[DIRECTIONS_COUNT], int borderScanStep[DIRECTIONS_COUNT], int borderScanThreshold[DIRECTIONS_COUNT], float blackThreshold, int outsideMask[EDGES_COUNT], struct IMAGE* image);

void borderToMask(int border[EDGES_COUNT], int mask[EDGES_COUNT], struct IMAGE* image);

void applyBorder(int border[EDGES_COUNT], int borderColor, struct IMAGE* image);
