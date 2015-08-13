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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <libavutil/avutil.h>
#include <libavutil/pixfmt.h>

#include "unpaper.h"
#include "tools.h"

/****************************************************************************
 * tool functions                                                           *
 *   - arithmetic tool functions                                            *
 *   - tool functions for image handling                                    *
 ****************************************************************************/

static inline uint8_t pixelGrayscale(uint8_t r, uint8_t g, uint8_t b) {
    return (r + g + b) / 3;
}

/* --- tool functions for image handling ---------------------------------- */

static void getPixelComponents(AVFrame *image, int x, int y, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t defval) {
    uint8_t *pix;

    if ( (x < 0) || (x >= image->width) || (y < 0) || (y >= image->height) ) {
        *r = *g = *b = defval;
        return;
    }

    switch(image->format) {
    case AV_PIX_FMT_GRAY8:
        pix = image->data[0] + (y * image->linesize[0] + x);
        *r = *g = *b = *pix;
        break;
    case AV_PIX_FMT_Y400A:
        pix = image->data[0] + (y * image->linesize[0] + x *2);
        *r = *g = *b = *pix;
        break;
    case AV_PIX_FMT_RGB24:
        pix = image->data[0] + (y * image->linesize[0] + x * 3);
        *r = pix[0];
        *g = pix[1];
        *b = pix[2];
        break;
    case AV_PIX_FMT_MONOWHITE:
        pix = image->data[0] + (y * image->linesize[0] + x / 8);
        if ( *pix & (128 >> (x % 8)) )
            *r = *g = *b = BLACK;
        else
            *r = *g = *b = WHITE;
        break;
    case AV_PIX_FMT_MONOBLACK:
        pix = image->data[0] + (y * image->linesize[0] + x / 8);
        if ( *pix & (128 >> (x % 8)) )
            *r = *g = *b = WHITE;
        else
            *r = *g = *b = BLACK;
        break;
    default:
        errOutput("unknown pixel format.");
    }
}

/**
 * Allocates a memory block for storing image data and fills the IMAGE-struct
 * with the specified values.
 */
void initImage(AVFrame **image, int width, int height, int pixel_format, bool fill) {
    int ret;

    (*image) = av_frame_alloc();
    (*image)->width = width;
    (*image)->height = height;
    (*image)->format = pixel_format;

    ret = av_frame_get_buffer(*image, 8);
    if (ret < 0) {
        char errbuff[1024];
        av_strerror(ret, errbuff, sizeof(errbuff));
        errOutput("unable to allocate buffer: %s", errbuff);
    }

    if ( fill ) {
        for (int y = 0; y < (*image)->height; y++) {
            for (int x = 0; x < (*image)->width; x++) {
                setPixel(sheetBackground, x, y, *image);
            }
        }
    }
}

/**
 * Sets the color/grayscale value of a single pixel.
 *
 * @return true if the pixel has been changed, false if the original color was the one to set
 */
bool setPixel(int pixel, int x, int y, AVFrame *image) {
    uint8_t *pix;

    if ( (x < 0) || (x >= image->width) || (y < 0) || (y >= image->height) ) {
        return false; //nop
    }

    uint8_t r = (pixel >> 16) & 0xff;
    uint8_t g = (pixel >> 8) & 0xff;
    uint8_t b = pixel & 0xff;

    uint8_t pixelbw = pixelGrayscale(r, g, b) < absBlackThreshold ? BLACK : WHITE;

    switch(image->format) {
    case AV_PIX_FMT_GRAY8:
        pix = image->data[0] + (y * image->linesize[0] + x);
        *pix = pixelGrayscale(r, g, b);
        break;
    case AV_PIX_FMT_Y400A:
        pix = image->data[0] + (y * image->linesize[0] + x * 2);
        pix[0] = pixelGrayscale(r, g, b);
        pix[1] = 0xFF; // no alpha.
        break;
    case AV_PIX_FMT_RGB24:
        pix = image->data[0] + (y * image->linesize[0] + x * 3);
        pix[0] = r;
        pix[1] = g;
        pix[2] = b;
        break;
    case AV_PIX_FMT_MONOWHITE:
        pixelbw = ~pixelbw; // reverse compared to following case
    case AV_PIX_FMT_MONOBLACK:
        pix = image->data[0] + (y * image->linesize[0] + x / 8);
        if ( pixelbw == WHITE ) {
            *pix = *pix | (128 >> (x % 8));
        } else if ( pixelbw == BLACK ) {
            *pix = *pix & ~(128 >> (x % 8));
        }
        break;
    default:
        errOutput("unknown pixel format.");
    }
    return true;
}

/**
 * Returns the color or grayscale value of a single pixel.
 * Always returns a color-compatible value (which may be interpreted as 8-bit grayscale)
 *
 * @return color or grayscale-value of the requested pixel, or WHITE if the coordinates are outside the image
 */
int getPixel(int x, int y, AVFrame *image) {
    uint8_t r, g, b;
    getPixelComponents(image, x, y, &r, &g, &b, WHITE);
    return pixelValue(r, g, b);
}

/**
 * Returns the grayscale (=brightness) value of a single pixel.
 *
 * @return grayscale-value of the requested pixel, or WHITE if the coordinates are outside the image
 */
static uint8_t getPixelGrayscale(int x, int y, AVFrame *image) {
    uint8_t r, g, b;
    getPixelComponents(image, x, y, &r, &g, &b, WHITE);
    return pixelGrayscale(r, g, b);
}


/**
 * Returns the 'lightness' value of a single pixel. For color images, this
 * value denotes the minimum brightness of a single color-component in the
 * total color, which means that any color is considered 'dark' which has
 * either the red, the green or the blue component (or, of course, several
 * of them) set to a high value. In some way, this is a measure how close a
 * color is to white.
 * For grayscale images, this value is equal to the pixel brightness.
 *
 * @return lightness-value (the higher, the lighter) of the requested pixel, or WHITE if the coordinates are outside the image
 */
static uint8_t getPixelLightness(int x, int y, AVFrame *image) {
    uint8_t r, g, b;
    getPixelComponents(image, x, y, &r, &g, &b, WHITE);
    return min3(r, g, b);
}


/**
 * Returns the 'inverse-darkness' value of a single pixel. For color images, this
 * value denotes the maximum brightness of a single color-component in the
 * total color, which means that any color is considered 'light' which has
 * either the red, the green or the blue component (or, of course, several
 * of them) set to a high value. In some way, this is a measure how far away a
 * color is to black.
 * For grayscale images, this value is equal to the pixel brightness.
 *
 * @return inverse-darkness-value (the LOWER, the darker) of the requested pixel, or WHITE if the coordinates are outside the image
 */
uint8_t getPixelDarknessInverse(int x, int y, AVFrame *image) {
    uint8_t r, g, b;
    getPixelComponents(image, x, y, &r, &g, &b, WHITE);
    return max3(r, g, b);
}

/**
 * Sets the color/grayscale value of a single pixel to white.
 *
 * @return true if the pixel has been changed, false if the original color was the one to set
 */
static bool clearPixel(int x, int y, AVFrame *image) {
    return setPixel(WHITE24, x, y, image);
}


/**
 * Clears a rectangular area of pixels with either black or white.
 * @return The number of pixels actually changed from black (dark) to white.
 */
int clearRect(const int left, const int top, const int right, const int bottom, AVFrame *image, const int blackwhite) {
    int count = 0;

    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            if (setPixel(blackwhite, x, y, image)) {
                count++;
            }
        }
    }
    return count;
}


/**
 * Copies one area of an image into another.
 */
void copyImageArea(const int x, const int y, const int width, const int height, AVFrame *source, const int toX, const int toY, AVFrame *target) {

    // naive but generic implementation
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int pixel = getPixel(x+col, y+row, source);
            setPixel(pixel, toX+col, toY+row, target);
        }
    }
}

/**
 * Centers one area of an image inside an area of another image.
 * If the source area is smaller than the target area, is is equally
 * surrounded by a white border, if it is bigger, it gets equally cropped
 * at the edges.
 */
static void centerImageArea(int x, int y, int w, int h, AVFrame *source, int toX, int toY, int ww, int hh, AVFrame *target) {
    if ((w < ww) || (h < hh)) { // white rest-border will remain, so clear first
        clearRect(toX, toY, toX + ww - 1, toY + hh - 1, target, sheetBackground);
    }
    if (w < ww) {
        toX += (ww - w) / 2;
    }
    if (h < hh) {
        toY += (hh - h) / 2;
    }
    if (w > ww) {
        x += (w - ww) / 2;
        w = ww;
    }
    if (h > hh) {
        y += (h - hh) / 2;
        h = hh;
    }
    copyImageArea(x, y, w, h, source, toX, toY, target);
}


/**
 * Centers a whole image inside an area of another image.
 */
void centerImage(AVFrame *source, int toX, int toY, int ww, int hh, AVFrame *target) {
    centerImageArea(0, 0, source->width, source->height, source, toX, toY, ww, hh, target);
}


/**
 * Returns the average brightness of a rectagular area.
 */
uint8_t inverseBrightnessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image) {
    unsigned int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            total += getPixelGrayscale(x, y, image);
        }
    }
    return WHITE - (total / count);
}


/**
 * Returns the inverseaverage lightness of a rectagular area.
 */
uint8_t inverseLightnessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image) {
    unsigned int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            total += getPixelLightness(x, y, image);
        }
    }
    return WHITE - (total / count);
}


/**
 * Returns the average darkness of a rectagular area.
 */
uint8_t darknessRect(const int x1, const int y1, const int x2, const int y2, AVFrame *image) {
    unsigned int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            total += getPixelDarknessInverse(x, y, image);
        }
    }
    return WHITE - (total / count);
}


/**
 * Counts the number of pixels in a rectangular area whose grayscale
 * values ranges between minColor and maxBrightness. Optionally, the area can get
 * cleared with white color while counting.
 */
int countPixelsRect(int left, int top, int right, int bottom, int minColor, int maxBrightness, bool clear, AVFrame *image) {
    int count = 0;

    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            const int pixel = getPixelGrayscale(x, y, image);
            if ((pixel>=minColor) && (pixel <= maxBrightness)) {
                if (clear==true) {
                    clearPixel(x, y, image);
                }
                count++;
            }
        }
    }
    return count;
}


/**
 * Counts the number of dark pixels around the pixel at (x,y), who have a
 * square-metric distance of 'level' to the (x,y) (thus, testing the values
 * of 9 pixels if level==1, 16 pixels if level==2 and so on).
 * Optionally, the pixels can get cleared after counting.
 */
static int countPixelNeighborsLevel(int x, int y, bool clear, int level, int whiteMin, AVFrame *image) {
    int count = 0;

    // upper and lower rows
    for (int xx = x - level; xx <= x + level; xx++) {
        // upper row
        uint8_t pixel = getPixelLightness(xx, y - level, image);
        if (pixel < whiteMin) { // non-light pixel found
            if (clear == true) {
                clearPixel(xx, y - level, image);
            }
            count++;
        }
        // lower row
        pixel = getPixelLightness(xx, y + level, image);
        if (pixel < whiteMin) {
            if (clear == true) {
                clearPixel(xx, y + level, image);
            }
            count++;
        }
    }
    // middle rows
    for (int yy = y-(level-1); yy <= y+(level-1); yy++) {
        // first col
        uint8_t pixel = getPixelLightness(x - level, yy, image);
        if (pixel < whiteMin) {
            if (clear == true) {
                clearPixel(x - level, yy, image);
            }
            count++;
        }
        // last col
        pixel = getPixelLightness(x + level, yy, image);
        if (pixel < whiteMin) {
            if (clear == true) {
                clearPixel(x + level, yy, image);
            }
            count++;
        }
    }
    /* old version, not optimized:
    for (yy = y-level; yy <= y+level; yy++) {
        for (xx = x-level; xx <= x+level; xx++) {
            if (abs(xx-x)==level || abs(yy-y)==level) {
                pixel = getPixelLightness(xx, yy, image);
                if (pixel < whiteMin) {
                    if (clear==true) {
                        clearPixel(xx, yy, image);
                    }
                    count++;
                }
            }
        }
    }*/
    return count;
}


/**
 * Count all dark pixels in the distance 0..intensity that are directly
 * reachable from the dark pixel at (x,y), without having to cross bright
 * pixels.
 */
int countPixelNeighbors(int x, int y, int intensity, int whiteMin, AVFrame *image) {
    int count = 1; // assume self as set
    int lCount = -1;

    // can finish when one level is completely zero
    for (int level = 1; (lCount != 0) && (level <= intensity); level++) {
        lCount = countPixelNeighborsLevel(x, y, false, level, whiteMin, image);
        count += lCount;
    }
    return count;
}


/**
 * Clears all dark pixels that are directly reachable from the dark pixel at
 * (x,y). This should be called only if it has previously been detected that
 * the amount of pixels to clear will be reasonable small.
 */
void clearPixelNeighbors(int x, int y, int whiteMin, AVFrame *image) {
    int lCount = -1;

    clearPixel(x, y, image);

    // lCount will become 0, otherwise countPixelNeighbors() would previously have delivered a bigger value (and this here would not have been called)
    for (int level = 1; lCount != 0; level++) {
        lCount = countPixelNeighborsLevel(x, y, true, level, whiteMin, image);
    }
}

/**
 * Solidly fills a line of pixels heading towards a specified direction
 * until color-changes in the pixels to overwrite exceed the 'intensity'
 * parameter.
 *
 * @param stepX either -1 or 1, if stepY is 0, else 0
 * @param stepY either -1 or 1, if stepX is 0, else 0
 */
static int fillLine(int x, int y, int stepX, int stepY, int color, uint8_t maskMin, uint8_t maskMax, int intensity, AVFrame *image) {
    int distance = 0;
    int intensityCount = 1; // first pixel must match, otherwise directly exit
    const int w = image->width;
    const int h = image->height;

    while (true) {
        x += stepX;
        y += stepY;
        uint8_t pixel = getPixelGrayscale(x, y, image);
        if ((pixel>=maskMin) && (pixel<=maskMax)) {
            intensityCount = intensity; // reset counter
        } else {
            intensityCount--; // allow maximum of 'intensity' pixels to be bright, until stop
        }
        if ((intensityCount > 0) && (x>=0) && (x<w) && (y>=0) && (y<h)) {
            setPixel(color, x, y, image);
            distance++;
        } else {
            return distance; // exit here
        }
    }
}


/**
 * Start flood-filling around the edges of a line which has previously been
 * filled using fillLine(). Here, the flood-fill algorithm performs its
 * indirect recursion.
 *
 * @param stepX either -1 or 1, if stepY is 0, else 0
 * @param stepY either -1 or 1, if stepX is 0, else 0
 * @see fillLine()
 */
static void floodFillAroundLine(int x, int y, int stepX, int stepY, int distance, int color, int maskMin, int maskMax, int intensity, AVFrame *image) {
    for (int d = 0; d < distance; d++) {
        if (stepX != 0) {
            x += stepX;
            floodFill(x, y + 1, color, maskMin, maskMax, intensity, image); // indirect recursion
            floodFill(x, y - 1, color, maskMin, maskMax, intensity, image); // indirect recursion
        } else { // stepY != 0
            y += stepY;
            floodFill(x + 1, y, color, maskMin, maskMax, intensity, image); // indirect recursion
            floodFill(x - 1, y, color, maskMin, maskMax, intensity, image); // indirect recursion
        }
    }
}


/**
 * Flood-fill an area of pixels. (Naive implementation, optimizable.)
 *
 * @see earlier header-declaration to enable indirect recursive calls
 */
void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity, AVFrame *image) {
    // is current pixel to be filled?
    const int pixel = getPixelGrayscale(x, y, image);
    if ((pixel>=maskMin) && (pixel<=maskMax)) {
        // first, fill a 'cross' (both vertical, horizontal line)
        setPixel(color, x, y, image);
        const int left = fillLine(x, y, -1, 0, color, maskMin, maskMax, intensity, image);
        const int top = fillLine(x, y, 0, -1, color, maskMin, maskMax, intensity, image);
        const int right = fillLine(x, y, 1, 0, color, maskMin, maskMax, intensity, image);
        const int bottom = fillLine(x, y, 0, 1, color, maskMin, maskMax, intensity, image);
        // now recurse on each neighborhood-pixel of the cross (most recursions will immediately return)
        floodFillAroundLine(x, y, -1, 0, left, color, maskMin, maskMax, intensity, image);
        floodFillAroundLine(x, y, 0, -1, top, color, maskMin, maskMax, intensity, image);
        floodFillAroundLine(x, y, 1, 0, right, color, maskMin, maskMax, intensity, image);
        floodFillAroundLine(x, y, 0, 1, bottom, color, maskMin, maskMax, intensity, image);
    }
}
