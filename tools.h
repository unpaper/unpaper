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

/* --- arithmetic tool functions ------------------------------------------ */


double sqr(double d);

double degreesToRadians(double d);

double radiansToDegrees(double r);

void limit(int* i, int max);


/* --- tool functions for image handling ---------------------------------- */


void initImage(struct IMAGE* image, int width, int height, int bitdepth, bool color, int background);

void freeImage(struct IMAGE* image);

void replaceImage(struct IMAGE* image, struct IMAGE* newimage);

bool setPixel(int pixel, int x, int y, struct IMAGE* image);

int getPixel(int x, int y, struct IMAGE* image);

int getPixelGrayscale(int x, int y, struct IMAGE* image);

int getPixelDarknessInverse(int x, int y, struct IMAGE* image);

int clearRect(int left, int top, int right, int bottom, struct IMAGE* image, int blackwhite);

void copyImageArea(int x, int y, int width, int height, struct IMAGE* source, int toX, int toY, struct IMAGE* target);

void copyImage(struct IMAGE* source, int toX, int toY, struct IMAGE* target);

void centerImage(struct IMAGE* source, int toX, int toY, int ww, int hh, struct IMAGE* target);

int brightnessRect(int x1, int y1, int x2, int y2, struct IMAGE* image);

int lightnessRect(int x1, int y1, int x2, int y2, struct IMAGE* image);

int darknessInverseRect(int x1, int y1, int x2, int y2, struct IMAGE* image);

int countPixelsRect(int left, int top, int right, int bottom, int minColor, int maxBrightness, bool clear, struct IMAGE* image);

int countPixelNeighbors(int x, int y, int intensity, int whiteMin, struct IMAGE* image);

void clearPixelNeighbors(int x, int y, int whiteMin, struct IMAGE* image);

void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity, struct IMAGE* image);
