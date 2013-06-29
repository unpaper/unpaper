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
#include <math.h>

#include "unpaper.h"

/****************************************************************************
 * tool functions                                                           *
 *   - arithmetic tool functions                                            *
 *   - tool functions for image handling                                    *
 ****************************************************************************/


/* --- arithmetic tool functions ------------------------------------------ */

/**
 * Returns the quadratic square of a number.
 */
double sqr(double d) {
    return d*d;
}


/**
 * Converts degrees to radians.
 */
double degreesToRadians(double d) {
    return d * M_PI / 180.0;
}


/**
 * Converts radians to degrees.
 */
double radiansToDegrees(double r) {
    return r * 180.0 / M_PI;
}


/**
 * Limits an integer value to a maximum.
 */
void limit(int* i, int max) {
    if (*i > max) {
        *i = max;
    }
}


/* --- tool functions for image handling ---------------------------------- */

/**
 * Allocates a memory block for storing image data and fills the IMAGE-struct
 * with the specified values.
 */
void initImage(struct IMAGE* image, int width, int height, int bitdepth, bool color, int background) {
    int size;
    
    size = width * height;
    if ( color ) {
        image->bufferGrayscale = (uint8_t*)malloc(size);
        image->bufferLightness = (uint8_t*)malloc(size);
        image->bufferDarknessInverse = (uint8_t*)malloc(size);
        memset(image->bufferGrayscale, background, size);
        memset(image->bufferLightness, background, size);
        memset(image->bufferDarknessInverse, background, size);
        size *= 3;
    }
    image->buffer = (uint8_t*)malloc(size);
    memset(image->buffer, background, size);
    if ( ! color ) {
        image->bufferGrayscale = image->buffer;
        image->bufferLightness = image->buffer;
        image->bufferDarknessInverse = image->buffer;
    }
    image->width = width;
    image->height = height;
    image->bitdepth = bitdepth;
    image->color = color;
    image->background = background;
}


/**
 * Frees an image.
 */
void freeImage(struct IMAGE* image) {    
    free(image->buffer);
    if (image->color) {
        free(image->bufferGrayscale);
        free(image->bufferLightness);
        free(image->bufferDarknessInverse);
    }
}


/**
 * Replaces one image with another.
 */
void replaceImage(struct IMAGE* image, struct IMAGE* newimage) {    
    freeImage(image);
    // pass-back new image
    *image = *newimage; // copy whole struct
}


/**
 * Sets the color/grayscale value of a single pixel.
 *
 * @return true if the pixel has been changed, false if the original color was the one to set
 */ 
bool setPixel(int pixel, int x, int y, struct IMAGE* image) {
    uint8_t* p;
    int w, h;
    int pos;
    bool result;
    uint8_t r, g, b;
    
    w = image->width;
    h = image->height;
    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return false; //nop
    } else {
        pos = (y * w) + x;
        r = (pixel >> 16) & 0xff;
        g = (pixel >> 8) & 0xff;
        b = pixel & 0xff;
        if ( ! image->color ) {
            p = &image->buffer[pos];
            if ((r == g) && (r == b)) { // optimization (avoid division by 3)
                pixel = r;
            } else {
                pixel = pixelGrayscale(r, g, b); // convert to gray (will already be in most cases, but we can't be sure)
            }
            if (*p != (uint8_t)pixel) {
                *p = (uint8_t)pixel;
                return true;
            } else {
                return false;
            }
        } else { // color
            result = false;
            p = &image->buffer[pos*3];
            if (*p != r) {
                *p = r;
                result = true;
            }
            p++;
            if (*p != g) {
                *p = g;
                result = true;
            }
            p++;
            if (*p != b) {
                *p = b;
                result = true;
            }
            if ( result ) { // modified: update cached grayscale, lightness and brightnessInverse values
                image->bufferGrayscale[pos] = pixelGrayscale(r, g, b);
                image->bufferLightness[pos] = pixelLightness(r, g, b);
                image->bufferDarknessInverse[pos] = pixelDarknessInverse(r, g, b);
            }
            return result;
        }
    }
}


/**
 * Returns the color or grayscale value of a single pixel.
 * Always returns a color-compatible value (which may be interpreted as 8-bit grayscale)
 *
 * @return color or grayscale-value of the requested pixel, or WHITE if the coordinates are outside the image
 */ 
int getPixel(int x, int y, struct IMAGE* image) {
    const int w = image->width;
    const int h = image->height;
    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return pixelValue(WHITE, WHITE, WHITE);
    } else {
        int pos = (y * w) + x;
        if ( ! image->color ) {
            const int pix = (uint8_t)image->buffer[pos];
            return pixelValue(pix, pix, pix);
        } else { // color
            pos *= 3;
            const uint8_t r = (uint8_t)image->buffer[pos++];
            const uint8_t g = (uint8_t)image->buffer[pos++];
            const uint8_t b = (uint8_t)image->buffer[pos];
            return pixelValue(r, g, b);
        }
    }
}


/**
 * Returns a color component of a single pixel (0-255).
 *
 * @param colorComponent either RED, GREEN or BLUE
 * @return color or grayscale-value of the requested pixel, or WHITE if the coordinates are outside the image
 */ 
 /* (currently not used)
int getPixelComponent(int x, int y, int colorComponent, struct IMAGE* image) {
    int w, h;
    int pos;

    w = image->width;
    h = image->height;
    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return WHITE;
    } else {
        pos = (y * w) + x;
        if ( ! image->color ) {
            return (uint8_t)image->buffer[pos];
        } else { // color
            return (uint8_t)image->buffer[(pos * 3) + colorComponent];
        }
    }
}
*/

/**
 * Returns the grayscale (=brightness) value of a single pixel.
 *
 * @return grayscale-value of the requested pixel, or WHITE if the coordinates are outside the image
 */ 
int getPixelGrayscale(int x, int y, struct IMAGE* image) {
    const int w = image->width;
    const int h = image->height;
    const int pos = (y * w) + x;

    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return WHITE;
    } else {
        return image->bufferGrayscale[pos];
    }
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
static int getPixelLightness(int x, int y, struct IMAGE* image) {
    const int w = image->width;
    const int h = image->height;
    const int pos = (y * w) + x;

    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return WHITE;
    } else {
        return image->bufferLightness[pos];
    }
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
int getPixelDarknessInverse(int x, int y, struct IMAGE* image) {
    const int w = image->width;
    const int h = image->height;
    const int pos = (y * w) + x;

    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return WHITE;
    } else {
        return image->bufferDarknessInverse[pos];
    }
}


/**
 * Sets the color/grayscale value of a single pixel to either black or white.
 *
 * @return true if the pixel has been changed, false if the original color was the one to set
 */ 
static bool setPixelBW(int x, int y, struct IMAGE* image, int blackwhite) {
    const int w = image->width;
    const int h = image->height;
    const int pos = (y * w) + x;

    if ( (x < 0) || (x >= w) || (y < 0) || (y >= h) ) {
        return false; //nop
    } else {
        if ( ! image->color ) {
            uint8_t *p = &image->buffer[pos];
            if (*p != blackwhite) {
                *p = blackwhite;
                return true;
            } else {
                return false;
            }
        } else { // color
            uint8_t *p = &image->buffer[pos * 3];
            bool result = false;

            if (*p != blackwhite) {
                *p = blackwhite;
                result = true;
            }
            p++;
            if (*p != blackwhite) {
                *p = blackwhite;
                result = true;
            }
            p++;
            if (*p != blackwhite) {
                *p = blackwhite;
                result = true;
            }
            return result;
        }
    }
}


/**
 * Sets the color/grayscale value of a single pixel to white.
 *
 * @return true if the pixel has been changed, false if the original color was the one to set
 */ 
static bool clearPixel(int x, int y, struct IMAGE* image) {
    return setPixelBW(x, y, image, WHITE);
}


/**
 * Clears a rectangular area of pixels with either black or white.
 * @return The number of pixels actually changed from black (dark) to white.
 */
int clearRect(const int left, const int top, const int right, const int bottom, struct IMAGE* image, const int blackwhite) {
    int count = 0;

    for (int y = top; y <= bottom; y++) {
        for (int x = left; x <= right; x++) {
            if (setPixelBW(x, y, image, blackwhite)) {
                count++;
            }
        }
    }
    return count;
}


/**
 * Copies one area of an image into another.
 */
void copyImageArea(const int x, const int y, const int width, const int height, struct IMAGE* source, const int toX, const int toY, struct IMAGE* target) {

    // naive but generic implementation
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            const int pixel = getPixel(x+col, y+row, source);
            setPixel(pixel, toX+col, toY+row, target);
        }
    }
}


/**
 * Copies a whole image into another.
 */
void copyImage(struct IMAGE* source, int toX, int toY, struct IMAGE* target) {
    copyImageArea(0, 0, source->width, source->height, source, toX, toY, target);
}


/**
 * Centers one area of an image inside an area of another image.
 * If the source area is smaller than the target area, is is equally
 * surrounded by a white border, if it is bigger, it gets equally cropped
 * at the edges.
 */
static void centerImageArea(int x, int y, int w, int h, struct IMAGE* source, int toX, int toY, int ww, int hh, struct IMAGE* target) {
    if ((w < ww) || (h < hh)) { // white rest-border will remain, so clear first
        clearRect(toX, toY, toX + ww - 1, toY + hh - 1, target, target->background);
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
void centerImage(struct IMAGE* source, int toX, int toY, int ww, int hh, struct IMAGE* target) {
    centerImageArea(0, 0, source->width, source->height, source, toX, toY, ww, hh, target);
}


/**
 * Returns the average brightness of a rectagular area.
 */
int brightnessRect(const int x1, const int y1, const int x2, const int y2, struct IMAGE* image) {
    int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            total += getPixelGrayscale(x, y, image);
        }
    }
    return total / count;
}


/**
 * Returns the average lightness of a rectagular area.
 */
int lightnessRect(const int x1, const int y1, const int x2, const int y2, struct IMAGE* image) {
    int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            const int pixel = getPixelLightness(x, y, image);
            total += pixel;
        }
    }
    return total / count;
}


/**
 * Returns the average darkness of a rectagular area.
 */
int darknessInverseRect(const int x1, const int y1, const int x2, const int y2, struct IMAGE* image) {
    int total = 0;
    const int count = (x2-x1+1)*(y2-y1+1);

    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {
            total += getPixelDarknessInverse(x, y, image);
        }
    }
    return total / count;
}


/**
 * Counts the number of pixels in a rectangular area whose grayscale
 * values ranges between minColor and maxBrightness. Optionally, the area can get
 * cleared with white color while counting.
 */
int countPixelsRect(int left, int top, int right, int bottom, int minColor, int maxBrightness, bool clear, struct IMAGE* image) {
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
static int countPixelNeighborsLevel(int x, int y, bool clear, int level, int whiteMin, struct IMAGE* image) {
    int xx;
    int yy;
    int count = 0;
    int pixel;
    
    // upper and lower rows
    for (xx = x - level; xx <= x + level; xx++) {
        // upper row
        pixel = getPixelLightness(xx, y - level, image);
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
    for (yy = y-(level-1); yy <= y+(level-1); yy++) {
        // first col
        pixel = getPixelLightness(x - level, yy, image);
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
int countPixelNeighbors(int x, int y, int intensity, int whiteMin, struct IMAGE* image) {
    int level;
    int count = 1; // assume self as set
    int lCount = -1;
    
    for (level = 1; (lCount != 0) && (level <= intensity); level++) { // can finish when one level is completely zero
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
void clearPixelNeighbors(int x, int y, int whiteMin, struct IMAGE* image) {
    int level;
    int lCount = -1;

    clearPixel(x, y, image);    

    for (level = 1; lCount != 0; level++) { // lCount will become 0, otherwise countPixelNeighbors() would previously have delivered a bigger value (and this here would not have been called)
        lCount = countPixelNeighborsLevel(x, y, true, level, whiteMin, image);
    }
}


/**
 * Flood-fill an area of pixels.
 * (Declaration of header for indirect recursive calls.)
 */
void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity, struct IMAGE* image);


/**
 * Solidly fills a line of pixels heading towards a specified direction
 * until color-changes in the pixels to overwrite exceed the 'intensity'
 * parameter.
 *
 * @param stepX either -1 or 1, if stepY is 0, else 0
 * @param stepY either -1 or 1, if stepX is 0, else 0
 */
static int fillLine(int x, int y, int stepX, int stepY, int color, int maskMin, int maskMax, int intensity, struct IMAGE* image) {
    int distance = 0;
    int intensityCount = 1; // first pixel must match, otherwise directly exit
    const int w = image->width;
    const int h = image->height;

    while (true) {
        int pixel;

        x += stepX;
        y += stepY;
        pixel = getPixelGrayscale(x, y, image);
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
static void floodFillAroundLine(int x, int y, int stepX, int stepY, int distance, int color, int maskMin, int maskMax, int intensity, struct IMAGE* image) {
    int d;
    
    for (d = 0; d < distance; d++) {
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
void floodFill(int x, int y, int color, int maskMin, int maskMax, int intensity, struct IMAGE* image) {
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
