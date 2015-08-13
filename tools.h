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


/* --- tool functions for image handling ---------------------------------- */

void initImage(AVFrame **image, int width, int height, int pixel_format, bool fill);

static inline void replaceImage(AVFrame **image, AVFrame **newimage) {
    av_frame_free(image);
    *image = *newimage;
}

bool setPixel(int pixel, const int x, const int y, AVFrame *image);

int getPixel(int x, int y, AVFrame *image);

uint8_t getPixelDarknessInverse(int x, int y, AVFrame *image);

int clearRect(const int left, const int top, const int right, const int bottom, AVFrame *image, const int blackwhite);

void copyImageArea(const int x, const int y, const int width, const int height, AVFrame *source, const int toX, const int toY, AVFrame *target);

void centerImage(AVFrame *source, int toX, int toY, int ww, int hh, AVFrame *target);

uint8_t inverseBrightnessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image);

uint8_t inverseLightnessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image);

uint8_t darknessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image);

int countPixelsRect(int left, int top, int right, int bottom, int minColor, int maxBrightness, bool clear, AVFrame *image);

int countPixelNeighbors(int x, int y, int intensity, int whiteMin, AVFrame *image);

void clearPixelNeighbors(int x, int y, int whiteMin, AVFrame *image);

void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity, AVFrame *image);
