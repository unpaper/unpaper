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

/* --- image processing --------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
//#include <time.h>

#include "unpaper.h"
#include "tools.h"
#include "parse.h" //for maksOverlapAny


/****************************************************************************
 * image processing functions                                               *
 ****************************************************************************/


/* --- deskewing ---------------------------------------------------------- */

/**
 * Returns the maximum peak value that occurs when shifting a rotated virtual line above the image,
 * starting from one edge of an area and moving towards the middle point of the area.
 * The peak value is calulated by the absolute difference in the average blackness of pixels that occurs between two single shifting steps.
 *
 * @param m ascending slope of the virtually shifted (m=tan(angle)). Mind that this is negative for negative radians.
 */
static int detectEdgeRotationPeak(double m, int deskewScanSize, float deskewScanDepth, int shiftX, int shiftY, int left, int top, int right, int bottom, struct IMAGE* image) {
    int width;
    int height;
    int mid;
    int half;
    int sideOffset;
    int outerOffset;
    double X; // unrounded coordinates
    double Y;
    double stepX;
    double stepY;
    int x[MAX_ROTATION_SCAN_SIZE];
    int y[MAX_ROTATION_SCAN_SIZE];
    int xx;
    int yy;
    int lineStep;
    int dep;
    int pixel;
    int blackness;
    int lastBlackness;
    int diff;
    int maxDiff;
    int maxBlacknessAbs;
    int maxDepth;
    int accumulatedBlackness;
        
    width = right-left+1;
    height = bottom-top+1;    
    maxBlacknessAbs = (int) 255 * deskewScanSize * deskewScanDepth;
    
    if (shiftY==0) { // horizontal detection
        if (deskewScanSize == -1) {
            deskewScanSize = height;
        }
        limit(&deskewScanSize, MAX_ROTATION_SCAN_SIZE);
        limit(&deskewScanSize, height);

        maxDepth = width/2;
        half = deskewScanSize/2;
        outerOffset = (int)(abs(m) * half);
        mid = height/2;
        sideOffset = shiftX > 0 ? left-outerOffset : right+outerOffset;
        X = sideOffset + half * m;
        Y = top + mid - half;
        stepX = -m;
        stepY = 1.0;
    } else { // vertical detection
        if (deskewScanSize == -1) {
            deskewScanSize = width;
        }
        limit(&deskewScanSize, MAX_ROTATION_SCAN_SIZE);
        limit(&deskewScanSize, width);
        maxDepth = height/2;
        half = deskewScanSize/2;
        outerOffset = (int)(abs(m) * half);
        mid = width/2;
        sideOffset = shiftY > 0 ? top-outerOffset : bottom+outerOffset;
        X = left + mid - half;
        Y = sideOffset - (half * m);
        stepX = 1.0;
        stepY = -m; // (line goes upwards for negative degrees)
    }
    
    // fill buffer with coordinates for rotated line in first unshifted position
    for (lineStep = 0; lineStep < deskewScanSize; lineStep++) {
        x[lineStep] = (int)X;
        y[lineStep] = (int)Y;
        X += stepX;
        Y += stepY;
    }
    
    // now scan for edge, modify coordinates in buffer to shift line into search direction (towards the middle point of the area)
    // stop either when detectMaxDepth steps are shifted, or when diff falls back to less than detectThreshold*maxDiff
    lastBlackness = 0;
    diff = 0;
    maxDiff = 0;
    accumulatedBlackness = 0;
    for (dep = 0; (accumulatedBlackness < maxBlacknessAbs) && (dep < maxDepth) ; dep++) {
        // calculate blackness of virtual line
        blackness = 0;
        for (lineStep = 0; lineStep < deskewScanSize; lineStep++) {
            xx = x[lineStep];
            x[lineStep] += shiftX;
            yy = y[lineStep];
            y[lineStep] += shiftY;
            if ((xx >= left) && (xx <= right) && (yy >= top) && (yy <= bottom)) {
                pixel = getPixelDarknessInverse(xx, yy, image);
                blackness += (255 - pixel);
            }
        }
        diff = blackness - lastBlackness;
        lastBlackness = blackness;
        if (diff >= maxDiff) {
            maxDiff = diff;
        }
        accumulatedBlackness += blackness;
    }
    if (dep < maxDepth) { // has not terminated only because middle was reached
        return maxDiff;
    } else {
        return 0;
    }
}


/**
 * Detects rotation at one edge of the area specified by left, top, right, bottom.
 * Which of the four edges to take depends on whether shiftX or shiftY is non-zero,
 * and what sign this shifting value has.
 */
static double detectEdgeRotation(float deskewScanRange, float deskewScanStep, int deskewScanSize, float deskewScanDepth, int shiftX, int shiftY, int left, int top, int right, int bottom, struct IMAGE* image) {
    // either shiftX or shiftY is 0, the other value is -i|+i
    // depending on shiftX/shiftY the start edge for shifting is determined
    double rangeRad;
    double stepRad;
    double rotation;
    int peak;
    int maxPeak;
    double detectedRotation;
    double m;

    rangeRad = degreesToRadians((double)deskewScanRange);
    stepRad = degreesToRadians((double)deskewScanStep);
    detectedRotation = 0.0;
    maxPeak = 0;    
    // iteratively increase test angle,  alterating between +/- sign while increasing absolute value
    for (rotation = 0.0; rotation <= rangeRad; rotation = (rotation>=0.0) ? -(rotation + stepRad) : -rotation ) {    
        m = tan(rotation);
        peak = detectEdgeRotationPeak(m, deskewScanSize, deskewScanDepth, shiftX, shiftY, left, top, right, bottom, image);
        if (peak > maxPeak) {
            detectedRotation = rotation;
            maxPeak = peak;
        }
    }
    return radiansToDegrees(detectedRotation);
}


/**
 * Detect rotation of a whole area. 
 * Angles between -deskewScanRange and +deskewScanRange are scanned, at either the
 * horizontal or vertical edges of the area specified by left, top, right, bottom.
 */
double detectRotation(int deskewScanEdges, int deskewScanRange, float deskewScanStep, int deskewScanSize, float deskewScanDepth, float deskewScanDeviation, int left, int top, int right, int bottom, struct IMAGE* image) {
    double rotation[4];
    int count;
    double total;
    double average;
    double deviation;
    int i;
    
    count = 0;
    
    if ((deskewScanEdges & 1<<LEFT) != 0) {
        // left
        rotation[count] = detectEdgeRotation(deskewScanRange, deskewScanStep, deskewScanSize, deskewScanDepth, 1, 0, left, top, right, bottom, image);
        if (verbose >= VERBOSE_NORMAL) {
            printf("detected rotation left: [%d,%d,%d,%d]: %f\n", left,top,right,bottom, rotation[count]);
        }
        count++;
    }
    if ((deskewScanEdges & 1<<TOP) != 0) {
        // top
        rotation[count] = - detectEdgeRotation(deskewScanRange, deskewScanStep, deskewScanSize, deskewScanDepth, 0, 1, left, top, right, bottom, image);
        if (verbose >= VERBOSE_NORMAL) {
            printf("detected rotation top: [%d,%d,%d,%d]: %f\n", left,top,right,bottom, rotation[count]);
        }
        count++;
    }
    if ((deskewScanEdges & 1<<RIGHT) != 0) {
        // right
        rotation[count] = detectEdgeRotation(deskewScanRange, deskewScanStep, deskewScanSize, deskewScanDepth, -1, 0, left, top, right, bottom, image);
        if (verbose >= VERBOSE_NORMAL) {
            printf("detected rotation right: [%d,%d,%d,%d]: %f\n", left,top,right,bottom, rotation[count]);
        }
        count++;
    }
    if ((deskewScanEdges & 1<<BOTTOM) != 0) {
        // bottom
        rotation[count] = - detectEdgeRotation(deskewScanRange, deskewScanStep, deskewScanSize, deskewScanDepth, 0, -1, left, top, right, bottom, image);
        if (verbose >= VERBOSE_NORMAL) {
            printf("detected rotation bottom: [%d,%d,%d,%d]: %f\n", left,top,right,bottom, rotation[count]);
        }
        count++;
    }
    
    total = 0.0;
    for (i = 0; i < count; i++) {
        total += rotation[i];
    }
    average = total / count;
    total = 0.0;
    for (i = 0; i < count; i++) {
        total += sqr(rotation[i]-average);
    }
    deviation = sqrt(total);
    if (verbose >= VERBOSE_NORMAL) {
        printf("rotation average: %f  deviation: %f  rotation-scan-deviation (maximum): %f  [%d,%d,%d,%d]\n", average, deviation, deskewScanDeviation, left,top,right,bottom);
    }
    if (deviation <= deskewScanDeviation) {
        return average;
    } else {
        if (verbose >= VERBOSE_NONE) {
            printf("out of deviation range - NO ROTATING\n");
        }
        return 0.0;
    }
}

/**
 * Rotates a whole image buffer by the specified radians, around its middle-point.
 * Usually, the buffer should have been converted to a qpixels-representation before, to increase quality.
 * (To rotate parts of an image, extract the part with copyBuffer, rotate, and re-paste with copyBuffer.)
 */
//void rotate(double radians, struct IMAGE* source, struct IMAGE* target, double* trigonometryCache, int trigonometryCacheBaseSize) {
void rotate(double radians, struct IMAGE* source, struct IMAGE* target) {
    const int w = source->width;
    const int h = source->height;
    const int halfX = (w-1)/2;
    const int halfY = (h-1)/2;
    const int midX = w/2;
    const int midY = h/2;
    const int midMax = max(midX,midY);
    
    // create 2D rotation matrix
    const float sinval = sinf(radians);
    const float cosval = cosf(radians);
    const float m11 = cosval;
    const float m12 = sinval;
    const float m21 = -sinval;
    const float m22 = cosval;

    // step through all pixels of the target image, 
    // symmetrically in all four quadrants to reduce trigonometric calculations
    int dX;
    int dY;

    for (dY = 0; dY <= midMax; dY++) {

        for (dX = 0; dX <= midMax; dX++) {
            // matrix multiplication to get rotated pixel pos (as in quadrant I)
            const int diffX = dX * m11 + dY * m21;
            const int diffY = dX * m12 + dY * m22;

            int x;
            int y;

            // quadrant I
            x = midX + dX;
            y = midY - dY;
            if ((x < w) && (y >= 0)) {
                const int oldX = midX + diffX;
                const int oldY = midY - diffY;
                const int pixel = getPixel(oldX, oldY, source);
                setPixel(pixel, x, y, target);
            }
            
            // quadrant II
            x = halfX - dY;
            y = midY - dX;
            if ((x >=0) && (y >= 0)) {
                const int oldX = halfX - diffY;
                const int oldY = midY - diffX;
                const int pixel = getPixel(oldX, oldY, source);
                setPixel(pixel, x, y, target);
            }
            
            // quadrant III
            x = halfX - dX;
            y = halfY + dY;
            if ((x >=0) && (y < h)) {
                const int oldX = halfX - diffX;
                const int oldY = halfY + diffY;
                const int pixel = getPixel(oldX, oldY, source);
                setPixel(pixel, x, y, target);
            }
            
            // quadrant IV
            x = midX + dY;
            y = halfY + dX;
            if ((x < w) && (y < h)) {
                const int oldX = midX + diffY;
                const int oldY = halfY + diffX;
                const int pixel = getPixel(oldX, oldY, source);
                setPixel(pixel, x, y, target);
            }
        }
    }
}


/**
 * Converts an image buffer to a qpixel-representation, i.e. enlarge the whole
 * whole image both horizontally and vertically by factor 2 (leading to a
 * factor 4 increase in total).
 * qpixelBuf must have been allocated before with 4-times amount of memory as
 * buf.
 */
void convertToQPixels(struct IMAGE* image, struct IMAGE* qpixelImage) {
    int x;
    int y;
    
    for (y = 0; y < image->height; y++) {
        const int yy = y*2;
        for (x = 0; x < image->width; x++) {
            const int xx = x*2;

            const int pixel = getPixel(x, y, image);

            setPixel(pixel, xx, yy, qpixelImage);
            setPixel(pixel, xx+1, yy, qpixelImage);
            setPixel(pixel, xx, yy+1, qpixelImage);
            setPixel(pixel, xx+1, yy+1, qpixelImage);
        }
    }
}


/**
 * Converts an image buffer back from a qpixel-representation to normal, i.e.
 * shrinks the whole image both horizontally and vertically by factor 2
 * (leading to a factor 4 decrease in total).
 * buf must have been allocated before with 1/4-times amount of memory as
 * qpixelBuf.
 */
void convertFromQPixels(struct IMAGE* qpixelImage, struct IMAGE* image) {
    int x;
    int y;
    
    for (y = 0; y < image->height; y++) {
        const int yy = y*2;
        for (x = 0; x < image->width; x++) {
            const int xx = x*2;

            const int a = getPixel(xx, yy, qpixelImage);
            const int b = getPixel(xx+1, yy, qpixelImage);
            const int c = getPixel(xx, yy+1, qpixelImage);
            const int d = getPixel(xx+1, yy+1, qpixelImage);

            const int r = (red(a) + red(b) + red(c) + red(d)) / 4;
            const int g = (green(a) + green(b) + green(c) + green(d)) / 4;
            const int bl = (blue(a) + blue(b) + blue(c) + blue(d)) / 4;

            const int pixel = pixelValue(r, g, bl);

            setPixel(pixel, x, y, image);
        }
    }
}


/* --- stretching / resizing / shifting ------------------------------------ */

/**
 * Stretches the image so that the resulting image has a new size.
 *
 * @param w the new width to stretch to
 * @param h the new height to stretch to
 */
void stretch(int w, int h, struct IMAGE* image) {
    struct IMAGE newimage;
    int x;
    int y;
    int matrixX;
    int matrixY;
    int matrixWidth;
    int matrixHeight;
    int blockWidth;
    int blockHeight;
    int blockWidthRest;
    int blockHeightRest;
    int fillIndexWidth;
    int fillIndexHeight;
    int fill;
    int xx;
    int yy;
    int sum;
    int sumR;
    int sumG;
    int sumB;
    int sumCount;
    int pixel;

    if (verbose >= VERBOSE_MORE) {
        printf("stretching %dx%d -> %dx%d\n", image->width, image->height, w, h);
    }

    // allocate new buffer's memory
    initImage(&newimage, w, h, image->bitdepth, image->color, WHITE);
    
    blockWidth = image->width / w; // (0 if enlarging, i.e. w > image->width)
    blockHeight = image->height / h;

    if (w <= image->width) {
        blockWidthRest = (image->width) % w;
    } else { // modulo-operator doesn't work as expected: (3680 % 7360)==3680 ! (not 7360 as expected)
             // shouldn't always be a % b = b if a < b ?
        blockWidthRest = w;
    }

    if (h <= image->height) {
        blockHeightRest = (image->height) % h;
    } else {
        blockHeightRest = h;
    }

    // for each new pixel, get a matrix of pixels from which the new pixel should be derived
    // (when enlarging, this matrix is always of size 1x1)
    matrixY = 0;
    fillIndexHeight = 0;
    for (y = 0; y < h; y++) {
        fillIndexWidth = 0;
        matrixX = 0;
        if ( ( (y * blockHeightRest) / h ) == fillIndexHeight ) { // next fill index?
            // (If our optimizer is cool, the above "* blockHeightRest / h" will disappear
            // when images are enlarged, because in that case blockHeightRest = h has been set before,
            // thus we're in a Kripke-branch where blockHeightRest and h are the same variable.
            // No idea if gcc's optimizer does this...) (See again below.)
            fillIndexHeight++;
            fill = 1;
        } else {
            fill = 0;
        }
        matrixHeight = blockHeight + fill;
        for (x = 0; x < w; x++) {
            if ( ( (x * blockWidthRest) / w ) == fillIndexWidth ) { // next fill index?
                fillIndexWidth++;
                fill = 1;
            } else {
                fill = 0;
            }
            matrixWidth = blockWidth + fill;
            // if enlarging, map corrdinates directly
            if (blockWidth == 0) { // enlarging
                matrixX = (x * image->width) / w;
            }
            if (blockHeight == 0) { // enlarging
                matrixY = (y * image->height) / h;
            }
            
            // calculate average pixel value in source matrix
            if ((matrixWidth == 1) && (matrixHeight == 1)) { // optimization: quick version
                pixel = getPixel(matrixX, matrixY, image);
            } else {
                sumCount = 0;
                if (!image->color) {
                    sum = 0;
                    for (yy = 0; yy < matrixHeight; yy++) {
                        for (xx = 0; xx < matrixWidth; xx++) {
                            sum += getPixelGrayscale(matrixX + xx, matrixY + yy, image);
                            sumCount++;
                        }
                    }
                    sum = sum / sumCount;
                    pixel = pixelGrayscaleValue(sum);
                } else { // color
                    sumR = 0;
                    sumG = 0;
                    sumB = 0;
                    for (yy = 0; yy < matrixHeight; yy++) {
                        for (xx = 0; xx < matrixWidth; xx++) {
                            pixel = getPixel(matrixX + xx, matrixY + yy, image);
                            sumR += (pixel >> 16) & 0xff;
                            sumG += (pixel >> 8) & 0xff;
                            sumB += pixel & 0xff;
                            //sumR += getPixelComponent(matrixX + xx, matrixY + yy, RED, image);
                            //sumG += getPixelComponent(matrixX + xx, matrixY + yy, GREEN, image);
                            //sumB += getPixelComponent(matrixX + xx, matrixY + yy, BLUE, image);
                            sumCount++;
                        }
                    }
                    pixel = pixelValue( sumR/sumCount, sumG/sumCount, sumB/sumCount );
                }
            }
            setPixel(pixel, x, y, &newimage);
            
            // pixel may have resulted in a gray value, which will be converted to 1-bit
            // when the file gets saved, if .pbm format requested. black-threshold will apply.
            
            if (blockWidth > 0) { // shrinking
                matrixX += matrixWidth;
            }
        }
        if (blockHeight > 0) { // shrinking
            matrixY += matrixHeight;
        }
    }
    replaceImage(image, &newimage);
}

/**
 * Resizes the image so that the resulting sheet has a new size and the image
 * content is zoomed to fit best into the sheet, while keeping it's aspect ration.
 *
 * @param w the new width to resize to
 * @param h the new height to resize to
 */
void resize(int w, int h, struct IMAGE* image) {
    struct IMAGE newimage;
    int ww;
    int hh;
    float wRat;
    float hRat;
    
    if (verbose >= VERBOSE_NORMAL) {
        printf("resizing %dx%d -> %dx%d\n", image->width, image->height, w, h);
    }

    wRat = (float)w / image->width;
    hRat = (float)h / image->height;
    if (wRat < hRat) { // horizontally more shrinking/less enlarging is needed: fill width fully, adjust height
        ww = w;
        hh = image->height * w / image->width;
    } else if (hRat < wRat) {
        ww = image->width * h / image->height;
        hh = h;
    } else { // wRat == hRat
        ww = w;
        hh = h;
    }
    stretch(ww, hh, image);
    initImage(&newimage, w, h, image->bitdepth, image->color, image->background);
    centerImage(image, 0, 0, w, h, &newimage);
    replaceImage(image, &newimage);
}


/**
 * Shifts the image.
 *
 * @param shiftX horizontal shifting
 * @param shiftY vertical shifting
 */
void shift(int shiftX, int shiftY, struct IMAGE* image) {
    struct IMAGE newimage;
    int x;
    int y;
    int pixel;

    // allocate new buffer's memory
    initImage(&newimage, image->width, image->height, image->bitdepth, image->color, image->background);
    
    for (y = 0; y < image->height; y++) {
        for (x = 0; x < image->width; x++) {
            pixel = getPixel(x, y, image);
            setPixel(pixel, x + shiftX, y + shiftY, &newimage);
        }
    }
    replaceImage(image, &newimage);
}


/* --- mask-detection ----------------------------------------------------- */

/**
 * Finds one edge of non-black pixels headig from one starting point towards edge direction.
 *
 * @return number of shift-steps until blank edge found
 */
static int detectEdge(int startX, int startY, int shiftX, int shiftY, int maskScanSize, int maskScanDepth, float maskScanThreshold, struct IMAGE* image) {
    // either shiftX or shiftY is 0, the other value is -i|+i
    int left;
    int top;
    int right;
    int bottom;
    
    const int half = maskScanSize / 2;
    int total = 0;
    int count = 0;

    if (shiftY==0) { // vertical border is to be detected, horizontal shifting of scan-bar
        if (maskScanDepth == -1) {
            maskScanDepth = image->height;
        }
        const int halfDepth = maskScanDepth / 2;
        left = startX - half;
        top = startY - halfDepth;
        right = startX + half;
        bottom = startY + halfDepth;
    } else { // horizontal border is to be detected, vertical shifting of scan-bar
        if (maskScanDepth == -1) {
            maskScanDepth = image->width;
        }
        const int halfDepth = maskScanDepth / 2;
        left = startX - halfDepth;
        top = startY - half;
        right = startX + halfDepth;
        bottom = startY + half;
    }
    
    while (true) { // !
        const int blackness = 255 - brightnessRect(left, top, right, bottom, image);
        total += blackness;
        count++;
        // is blackness below threshold*average?
        if ((blackness < ((maskScanThreshold*total)/count))||(blackness==0)) { // this will surely become true when pos reaches the outside of the actual image area and blacknessRect() will deliver 0 because all pixels outside are considered white
            return count; // ! return here, return absolute value of shifting difference
        }
        left += shiftX;
        right += shiftX;
        top += shiftY;
        bottom += shiftY;
    }
}


/**
 * Detects a mask of white borders around a starting point.
 * The result is returned via call-by-reference parameters left, top, right, bottom.
 *
 * @return the detected mask in left, top, right, bottom; or -1, -1, -1, -1 if no mask could be detected
 */
static bool detectMask(int startX, int startY, int maskScanDirections, int maskScanSize[DIRECTIONS_COUNT], int maskScanDepth[DIRECTIONS_COUNT], int maskScanStep[DIRECTIONS_COUNT], float maskScanThreshold[DIRECTIONS_COUNT], int maskScanMinimum[DIMENSIONS_COUNT], int maskScanMaximum[DIMENSIONS_COUNT], int* left, int* top, int* right, int* bottom, struct IMAGE* image) {
    int width;
    int height;
    int half[DIRECTIONS_COUNT];
    bool success;
    
    half[HORIZONTAL] = maskScanSize[HORIZONTAL] / 2;
    half[VERTICAL] = maskScanSize[VERTICAL] / 2;
    if ((maskScanDirections & 1<<HORIZONTAL) != 0) {
        *left = startX - maskScanStep[HORIZONTAL] * detectEdge(startX, startY, -maskScanStep[HORIZONTAL], 0, maskScanSize[HORIZONTAL], maskScanDepth[HORIZONTAL], maskScanThreshold[HORIZONTAL], image) - half[HORIZONTAL];
        *right = startX + maskScanStep[HORIZONTAL] * detectEdge(startX, startY, maskScanStep[HORIZONTAL], 0, maskScanSize[HORIZONTAL], maskScanDepth[HORIZONTAL], maskScanThreshold[HORIZONTAL], image) + half[HORIZONTAL];
    } else { // full range of sheet
        *left = 0;
        *right = image->width - 1;
    }
    if ((maskScanDirections & 1<<VERTICAL) != 0) {
        *top = startY - maskScanStep[VERTICAL] * detectEdge(startX, startY, 0, -maskScanStep[VERTICAL], maskScanSize[VERTICAL], maskScanDepth[VERTICAL], maskScanThreshold[VERTICAL], image) - half[VERTICAL];
        *bottom = startY + maskScanStep[VERTICAL] * detectEdge(startX, startY, 0, maskScanStep[VERTICAL], maskScanSize[VERTICAL], maskScanDepth[VERTICAL], maskScanThreshold[VERTICAL], image) + half[VERTICAL];
    } else { // full range of sheet
        *top = 0;
        *bottom = image->height - 1;
    }
    
    // if below minimum or above maximum, set to maximum
    width = *right - *left;
    height = *bottom - *top;
    success = true;
    if ( ((maskScanMinimum[WIDTH] != -1) && (width < maskScanMinimum[WIDTH])) || ((maskScanMaximum[WIDTH] != -1) && (width > maskScanMaximum[WIDTH])) ) {
        width = maskScanMaximum[WIDTH] / 2;
        *left = startX - width;
        *right = startX + width;
        success = false;;
    }
    if ( ((maskScanMinimum[HEIGHT] != -1) && (height < maskScanMinimum[HEIGHT])) || ((maskScanMaximum[HEIGHT] != -1) && (height > maskScanMaximum[HEIGHT])) ) {
        height = maskScanMaximum[HEIGHT] / 2;
        *top = startY - height;
        *bottom = startY + height;
        success = false;
    }
    return success;
}


/**
 * Detects masks around the points specified in point[].
 *
 * @param mask point to array into which detected masks will be stored
 * @return number of masks stored in mask[][]
 */
int detectMasks(int mask[MAX_MASKS][EDGES_COUNT], bool maskValid[MAX_MASKS], int point[MAX_POINTS][COORDINATES_COUNT], int pointCount, int maskScanDirections, int maskScanSize[DIRECTIONS_COUNT], int maskScanDepth[DIRECTIONS_COUNT], int maskScanStep[DIRECTIONS_COUNT], float maskScanThreshold[DIRECTIONS_COUNT], int maskScanMinimum[DIMENSIONS_COUNT], int maskScanMaximum[DIMENSIONS_COUNT],  struct IMAGE* image) {
    int left;
    int top;
    int right;
    int bottom;
    int i;
    int maskCount;
    
    maskCount = 0;
    if (maskScanDirections != 0) {
         for (i = 0; i < pointCount; i++) {
             maskValid[i] = detectMask(point[i][X], point[i][Y], maskScanDirections, maskScanSize, maskScanDepth, maskScanStep, maskScanThreshold, maskScanMinimum, maskScanMaximum, &left, &top, &right, &bottom, image);
             if (!(left==-1 || top==-1 || right==-1 || bottom==-1)) {
                 mask[maskCount][LEFT] = left;
                 mask[maskCount][TOP] = top;
                 mask[maskCount][RIGHT] = right;
                 mask[maskCount][BOTTOM] = bottom;
                 maskCount++;
                 if (verbose>=VERBOSE_NORMAL) {
                     printf("auto-masking (%d,%d): %d,%d,%d,%d", point[i][X], point[i][Y], left, top, right, bottom);
                     if (maskValid[i] == false) { // (mask had been auto-set to full page size)
                         printf(" (invalid detection, using full page size)");
                     }
                     printf("\n");
                 }
             } else {
                 if (verbose>=VERBOSE_NORMAL) {
                     printf("auto-masking (%d,%d): NO MASK FOUND\n", point[i][X], point[i][Y]);
                 }
             }
             //if (maskValid[i] == false) { // (mask had been auto-set to full page size)
             //    if (verbose>=VERBOSE_NORMAL) {
             //        printf("auto-masking (%d,%d): NO MASK DETECTED\n", point[i][X], point[i][Y]);
             //    }
             //}
         }
    }
    return maskCount;
}


/**
 * Permanently applies image masks. Each pixel which is not covered by at least
 * one mask is set to maskColor.
 */
void applyMasks(int mask[MAX_MASKS][EDGES_COUNT], int maskCount, int maskColor, struct IMAGE* image) {
    int x;
    int y;
    int i;
    
    if (maskCount<=0) {
        return;
    }
    for (y=0; y < image->height; y++) {
        for (x=0; x < image->width; x++) {
            // in any mask?
            bool m = false;
            for (i=0; ((m==false) && (i<maskCount)); i++) {
                const int left = mask[i][LEFT];
                const int top = mask[i][TOP];
                const int right = mask[i][RIGHT];
                const int bottom = mask[i][BOTTOM];
                if (y>=top && y<=bottom && x>=left && x<=right) {
                    m = true;
                }
            }
            if (m == false) {
                setPixel(maskColor, x, y, image); // delete: set to white
            }
        }
    }
}


/* --- wiping ------------------------------------------------------------- */

/**
 * Permanently wipes out areas of an images. Each pixel covered by a wipe-area
 * is set to wipeColor.
 */
void applyWipes(int area[MAX_MASKS][EDGES_COUNT], int areaCount, int wipeColor, struct IMAGE* image) {
    int x;
    int y;
    int i;

    for (i = 0; i < areaCount; i++) {
        int count = 0;
        for (y = area[i][TOP]; y <= area[i][BOTTOM]; y++) {
            for (x = area[i][LEFT]; x <= area[i][RIGHT]; x++) {
                if ( setPixel(wipeColor, x, y, image) ) {
                    count++;
                }
            }
        }
        if (verbose >= VERBOSE_MORE) {
            printf("wipe [%d,%d,%d,%d]: %d pixels\n", area[i][LEFT], area[i][TOP], area[i][RIGHT], area[i][BOTTOM], count);
        }
    }
}


/* --- mirroring ---------------------------------------------------------- */

/**
 * Mirrors an image either horizontally, vertically, or both.
 */
void mirror(int directions, struct IMAGE* image) {
    int x;
    int y;

    const bool horizontal = !!((directions & 1<<HORIZONTAL) != 0);
    const bool vertical = !!((directions & 1<<VERTICAL) != 0);
    int untilX = ((horizontal==true)&&(vertical==false)) ? ((image->width - 1) >> 1) : (image->width - 1);  // w>>1 == (int)(w-0.5)/2
    int untilY = (vertical==true) ? ((image->height - 1) >> 1) : image->height - 1;

    for (y = 0; y <= untilY; y++) {
        const int yy = (vertical==true) ? (image->height - y - 1) : y;
        if ((vertical==true) && (horizontal==true) && (y == yy)) { // last middle line in odd-lined image mirrored both h and v
            untilX = ((image->width - 1) >> 1);
        }
        for (x = 0; x <= untilX; x++) {
            const int xx = (horizontal==true) ? (image->width - x - 1) : x;
            const int pixel1 = getPixel(x, y, image);
            const int pixel2 = getPixel(xx, yy, image);
            setPixel(pixel2, x, y, image);
            setPixel(pixel1, xx, yy, image);
        }
    }
}


/* --- flip-rotating ------------------------------------------------------ */

/**
 * Rotates an image clockwise or anti-clockwise in 90-degrees.
 *
 * @param direction either -1 (rotate anti-clockwise) or 1 (rotate clockwise)
 */
void flipRotate(int direction, struct IMAGE* image) {
    struct IMAGE newimage;
    int x;
    int y;
    
    initImage(&newimage, image->height, image->width, image->bitdepth, image->color, WHITE); // exchanged width and height
    for (y = 0; y < image->height; y++) {
        const int xx = ((direction > 0) ? image->height - 1 : 0) - y * direction;
        for (x = 0; x < image->width; x++) {
            const int yy = ((direction < 0) ? image->width - 1 : 0) + x * direction;
            const int pixel = getPixel(x, y, image);
            setPixel(pixel, xx, yy, &newimage);
        }
    }
    replaceImage(image, &newimage);
}


/* --- blackfilter -------------------------------------------------------- */

/**
 * Filters out solidly black areas scanning to one direction.
 *
 * @param stepX is 0 if stepY!=0
 * @param stepY is 0 if stepX!=0
 * @see blackfilter()
 */
static void blackfilterScan(int stepX, int stepY, int size, int dep, float threshold, int exclude[MAX_MASKS][EDGES_COUNT], int excludeCount, int intensity, float blackThreshold, struct IMAGE* image) {
    int left;
    int top;
    int right;
    int bottom;
    int blackness;
    int thresholdBlack;
    int x;
    int y;
    int shiftX;
    int shiftY;
    int l, t, r, b;
    int diffX;
    int diffY;
    int mask[EDGES_COUNT];
    bool alreadyExcludedMessage;

    thresholdBlack = (int)(WHITE * (1.0-blackThreshold));
    if (stepX != 0) { // horizontal scanning
        left = 0;
        top = 0;
        right = size -1;
        bottom = dep - 1;
        shiftX = 0;
        shiftY = dep;
    } else { // vertical scanning
        left = 0;
        top = 0;
        right = dep -1;
        bottom = size - 1;
        shiftX = dep;
        shiftY = 0;
    }
    while ((left < image->width) && (top < image->height)) { // individual scanning "stripes" over the whole sheet
        l = left;
        t = top;
        r = right;
        b = bottom;
        // make sure last stripe does not reach outside sheet, shift back inside (next +=shift will exit while-loop)
        if (r >= image->width || b >= image->height) {
            diffX = r-image->width+1;
            diffY = b-image->height+1;
            l -= diffX;
            t -= diffY;
            r -= diffX;
            b -= diffY;
        }
        alreadyExcludedMessage = false;
        while ((l < image->width) && (t < image->height)) { // single scanning "stripe"
            blackness = 255 - darknessInverseRect(l, t, r, b, image);
            if (blackness >= 255*threshold) { // found a solidly black area
                mask[LEFT] = l;
                mask[TOP] = t;
                mask[RIGHT] = r;
                mask[BOTTOM] = b;
                if (! masksOverlapAny(mask, exclude, excludeCount) ) {
                    if (verbose >= VERBOSE_NORMAL) {
                        printf("black-area flood-fill: [%d,%d,%d,%d]\n", l, t, r, b);
                        alreadyExcludedMessage = false;
                    }
                    // start flood-fill in this area (on each pixel to make sure we get everything, in most cases first flood-fill from first pixel will delete all other black pixels in the area already)
                    for (y = t; y <= b; y++) {
                        for (x = l; x <= r; x++) {
                            floodFill(x, y, pixelValue(WHITE, WHITE, WHITE), 0, thresholdBlack, intensity, image);
                        }
                    }
                } else {
                    if ((verbose >= VERBOSE_NORMAL) && (!alreadyExcludedMessage)) {
                        printf("black-area EXCLUDED: [%d,%d,%d,%d]\n", l, t, r, b);
                        alreadyExcludedMessage = true; // do this only once per scan-stripe, otherwise too many mesages
                    }
                }
            }
            l += stepX;
            t += stepY;
            r += stepX;
            b += stepY;
        }
        left += shiftX;
        top += shiftY;
        right += shiftX;
        bottom += shiftY;
    }
}


/**
 * Filters out solidly black areas, as appearing on bad photocopies.
 * A virtual bar of width 'size' and height 'depth' is horizontally moved 
 * above the middle of the sheet (or the full sheet, if depth ==-1).
 */
void blackfilter(int blackfilterScanDirections, int blackfilterScanSize[DIRECTIONS_COUNT], int blackfilterScanDepth[DIRECTIONS_COUNT], int blackfilterScanStep[DIRECTIONS_COUNT], float blackfilterScanThreshold, int blackfilterExclude[MAX_MASKS][EDGES_COUNT], int blackfilterExcludeCount, int blackfilterIntensity, float blackThreshold, struct IMAGE* image) {
    if ((blackfilterScanDirections & 1<<HORIZONTAL) != 0) { // left-to-right scan
        blackfilterScan(blackfilterScanStep[HORIZONTAL], 0, blackfilterScanSize[HORIZONTAL], blackfilterScanDepth[HORIZONTAL], blackfilterScanThreshold, blackfilterExclude, blackfilterExcludeCount, blackfilterIntensity, blackThreshold, image);
    }
    if ((blackfilterScanDirections & 1<<VERTICAL) != 0) { // top-to-bottom scan
        blackfilterScan(0, blackfilterScanStep[VERTICAL], blackfilterScanSize[VERTICAL], blackfilterScanDepth[VERTICAL], blackfilterScanThreshold, blackfilterExclude, blackfilterExcludeCount, blackfilterIntensity, blackThreshold, image);
    }
}


/* --- noisefilter -------------------------------------------------------- */

/**
 * Applies a simple noise filter to the image.
 *
 * @param intensity maximum cluster size to delete
 */
int noisefilter(int intensity, float whiteThreshold, struct IMAGE* image) {
    int x;
    int y;
    int whiteMin;
    int count;
    int pixel;
    int neighbors;
    
    whiteMin = (int)(WHITE * whiteThreshold);
    count = 0;
    for (y = 0; y < image->height; y++) {
        for (x = 0; x < image->width; x++) {
            pixel = getPixelDarknessInverse(x, y, image);
            if (pixel < whiteMin) { // one dark pixel found
                neighbors = countPixelNeighbors(x, y, intensity, whiteMin, image); // get number of non-light pixels in neighborhood
                if (neighbors <= intensity) { // ...not more than 'intensity'?
                    clearPixelNeighbors(x, y, whiteMin, image); // delete area
                    count++;
                }
            }
        }
    }
    return count;
}


/* --- blurfilter --------------------------------------------------------- */

/**
 * Removes noise using a kind of blurfilter, as alternative to the noise
 * filter. This algoithm counts pixels while 'shaking' the area to detect,
 * and clears the area if the amount of white pixels exceeds whiteTreshold.
 */
int blurfilter(int blurfilterScanSize[DIRECTIONS_COUNT], int blurfilterScanStep[DIRECTIONS_COUNT], float blurfilterIntensity, float whiteThreshold, struct IMAGE* image) {
    int whiteMin;
    int left;
    int top;
    int right;
    int bottom;
    int count;
    int maxLeft;
    int maxTop;
    int blocksPerRow;
    int* prevCounts; // Number of dark pixels in previous row
    int* curCounts; // Number of dark pixels in current row
    int* nextCounts; // Number of dark pixels in next row
    int block; // Number of block in row; Counting begins with 1
    int max;
    int total; // Number of pixels in a block
    int result;
    
    result = 0;
    whiteMin = (int)(WHITE * whiteThreshold);
    left = 0;
    top = 0;
    right = blurfilterScanSize[HORIZONTAL] - 1;
    bottom = blurfilterScanSize[VERTICAL] - 1;
    maxLeft = image->width - blurfilterScanSize[HORIZONTAL];
    maxTop = image->height - blurfilterScanSize[VERTICAL];

    blocksPerRow = image->width / blurfilterScanSize[HORIZONTAL];
    // allocate one extra block left and right
    prevCounts = calloc(blocksPerRow + 2, sizeof(int));
    curCounts = calloc(blocksPerRow + 2, sizeof(int));
    nextCounts = calloc(blocksPerRow + 2, sizeof(int));

    total = blurfilterScanSize[HORIZONTAL] * blurfilterScanSize[VERTICAL];

    block = 1;
    for (left = 0; left <= maxLeft; left += blurfilterScanSize[HORIZONTAL]) {
	curCounts[block] = countPixelsRect(left, top, right, bottom, 0, whiteMin, false, image);
	block++;
	right += blurfilterScanSize[HORIZONTAL];
    }
    curCounts[0] = total;
    curCounts[blocksPerRow] = total;
    nextCounts[0] = total;
    nextCounts[blocksPerRow] = total;

    // Loop through all blocks. For a block calculate the number of dark pixels in this block, the number of dark pixels in the block in the top-left corner and similarly for the block in the top-right, bottom-left and bottom-right corner. Take the maximum of these values. Clear the block if this number is not large enough compared to the total number of pixels in a block.
    for (top = 0; top <= maxTop; top += blurfilterScanSize[HORIZONTAL]) {
	left = 0;
	right = blurfilterScanSize[HORIZONTAL] - 1;
	nextCounts[0] = countPixelsRect(left, top+blurfilterScanStep[VERTICAL], right, bottom+blurfilterScanSize[VERTICAL], 0, whiteMin, false, image);

	block = 1;
	for (left = 0; left <= maxLeft; left += blurfilterScanSize[HORIZONTAL]) {
	    // current block
	    count = curCounts[block];
	    max = count;
	    // top left
	    count = prevCounts[block-1];
	    if (count > max) {
		max = count;
	    }
	    // top right
	    count = prevCounts[block+1];
	    if (count > max) {
		max = count;
	    }
	    // bottom left
	    count = nextCounts[block-1];
	    if (count > max) {
		max = count;
	    }
	    // bottom right (has still to be calculated)
	    nextCounts[block+1] = countPixelsRect(left+blurfilterScanSize[HORIZONTAL], top+blurfilterScanStep[VERTICAL], right+blurfilterScanSize[HORIZONTAL], bottom+blurfilterScanSize[VERTICAL], 0, whiteMin, false, image);
	    if (count > max) {
		max = count;
	    }
	    if ((((float)max)/total) <= blurfilterIntensity) { // Not enough dark pixels
		clearRect(left, top, right, bottom, image, WHITE);
		result += curCounts[block];
		curCounts[block] = total; // Update information
	    }

	    right += blurfilterScanSize[HORIZONTAL];
	    block++;
	}

	bottom += blurfilterScanSize[VERTICAL];
	//Switch Buffers
	const int* tmpCounts;
	tmpCounts = prevCounts;
	prevCounts = curCounts;
	curCounts = nextCounts;
	nextCounts = (int*)tmpCounts;
    }
    free(prevCounts);
    free(curCounts);
    free(nextCounts);

    return result;
}


/* --- grayfilter --------------------------------------------------------- */

/**
 * Clears areas which do not contain any black pixels, but some "gray shade" only.
 * Two conditions have to apply before an area gets deleted: first, not a single black pixel may be contained,
 * second, a minimum threshold of blackness must not be exceeded.
 */
int grayfilter(int grayfilterScanSize[DIRECTIONS_COUNT], int grayfilterScanStep[DIRECTIONS_COUNT], float grayfilterThreshold, float blackThreshold, struct IMAGE* image) {
    int blackMax;
    int left;
    int top;
    int right;
    int bottom;
    int count;
    int lightness;
    int thresholdAbs;
    int result;
    
    result = 0;
    blackMax = (int)(WHITE * (1.0-blackThreshold));
    thresholdAbs = (int)(WHITE * grayfilterThreshold);
    left = 0;
    top = 0;
    right = grayfilterScanSize[HORIZONTAL] - 1;
    bottom = grayfilterScanSize[VERTICAL] - 1;
    
    while (true) { // !
        count = countPixelsRect(left, top, right, bottom, 0, blackMax, false, image);
        if (count == 0) {
            lightness = lightnessRect(left, top, right, bottom, image);
            if ((WHITE - lightness) < thresholdAbs) { // (lower threshold->more deletion)
                result += clearRect(left, top, right, bottom, image, WHITE);
            }
        }
        if (left < image->width) { // not yet at end of row
            left += grayfilterScanStep[HORIZONTAL];
            right += grayfilterScanStep[HORIZONTAL];
        } else { // end of row
            if (bottom >= image->height) { // has been last row
                return result; // exit here
            }
            // next row:
            left = 0;
            right = grayfilterScanSize[HORIZONTAL] - 1;
            top += grayfilterScanStep[VERTICAL];
            bottom += grayfilterScanStep[VERTICAL];
        }
    }
}


/* --- border-detection --------------------------------------------------- */

/**
 * Moves a rectangular area of pixels to be centered above the centerX, centerY coordinates.
 */
void centerMask(int centerX, int centerY, int left, int top, int right, int bottom, struct IMAGE* image) {
    struct IMAGE newimage;
    
    const int width = right - left + 1;
    const int height = bottom - top + 1;
    const int targetX = centerX - width/2;
    const int targetY = centerY - height/2;
    if ((targetX >= 0) && (targetY >= 0) && ((targetX+width) <= image->width) && ((targetY+height) <= image->height)) {
        if (verbose >= VERBOSE_NORMAL) {
            printf("centering mask [%d,%d,%d,%d] (%d,%d): %d, %d\n", left, top, right, bottom, centerX, centerY, targetX-left, targetY-top);
        }
        initImage(&newimage, width, height, image->bitdepth, image->color, image->background);
        copyImageArea(left, top, width, height, image, 0, 0, &newimage);
        clearRect(left, top, right, bottom, image, image->background);
        copyImageArea(0, 0, width, height, &newimage, targetX, targetY, image);
        freeImage(&newimage);
    } else {
        if (verbose >= VERBOSE_NORMAL) {
            printf("centering mask [%d,%d,%d,%d] (%d,%d): %d, %d - NO CENTERING (would shift area outside visible image)\n", left, top, right, bottom, centerX, centerY, targetX-left, targetY-top);
        }
    }
}


/**
 * Moves a rectangular area of pixels to be centered inside a specified area coordinates.
 */
void alignMask(int mask[EDGES_COUNT], int outside[EDGES_COUNT], int direction, int margin[DIRECTIONS_COUNT], struct IMAGE* image) {
    struct IMAGE newimage;
    int targetX;
    int targetY;
    
    const int width = mask[RIGHT] - mask[LEFT] + 1;
    const int height = mask[BOTTOM] - mask[TOP] + 1;
    if (direction & 1<<LEFT) {
        targetX = outside[LEFT] + margin[HORIZONTAL];
    } else if (direction & 1<<RIGHT) {
        targetX = outside[RIGHT] - width - margin[HORIZONTAL];
    } else {
        targetX = (outside[LEFT] + outside[RIGHT] - width) / 2;
    }
    if (direction & 1<<TOP) {
        targetY = outside[TOP] + margin[VERTICAL];
    } else if (direction & 1<<BOTTOM) {
        targetY = outside[BOTTOM] - height - margin[VERTICAL];
    } else {
        targetY = (outside[TOP] + outside[BOTTOM] - height) / 2;
    }
    if (verbose >= VERBOSE_NORMAL) {
        printf("aligning mask [%d,%d,%d,%d] (%d,%d): %d, %d\n", mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM], targetX, targetY, targetX - mask[LEFT], targetY - mask[TOP]);
    }
    initImage(&newimage, width, height, image->bitdepth, image->color, image->background);
    copyImageArea(mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM], image, 0, 0, &newimage);
    clearRect(mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM], image, image->background);
    copyImageArea(0, 0, width, height, &newimage, targetX, targetY, image);
    freeImage(&newimage);
}


/**
 * Find the size of one border edge.
 *
 * @param x1..y2 area inside of which border is to be detected
 */
static int detectBorderEdge(int outsideMask[EDGES_COUNT], int stepX, int stepY, int size, int threshold, int maxBlack, struct IMAGE* image) {
    int left;
    int top;
    int right;
    int bottom;
    int max;
    int cnt;
    int result;
    
    if (stepY == 0) { // horizontal detection
        if (stepX > 0) {
            left = outsideMask[LEFT];
            top = outsideMask[TOP];
            right = outsideMask[LEFT] + size;
            bottom = outsideMask[BOTTOM];
        } else {
            left = outsideMask[RIGHT] - size;
            top = outsideMask[TOP];
            right = outsideMask[RIGHT];
            bottom = outsideMask[BOTTOM];
        }
        max = (outsideMask[RIGHT] - outsideMask[LEFT]);
    } else { // vertical detection
        if (stepY > 0) {
            left = outsideMask[LEFT];
            top = outsideMask[TOP];
            right = outsideMask[RIGHT];
            bottom = outsideMask[TOP] + size;
        } else {
            left = outsideMask[LEFT];
            top = outsideMask[BOTTOM] - size;
            right = outsideMask[RIGHT];
            bottom = outsideMask[BOTTOM];
        }
        max = (outsideMask[BOTTOM] - outsideMask[TOP]);
    }
    result = 0;
    while (result < max) {
        cnt = countPixelsRect(left, top, right, bottom, 0, maxBlack, false, image);
        if (cnt >= threshold) {
            return result; // border has been found: regular exit here
        }
        left += stepX;
        top += stepY;
        right += stepX;
        bottom += stepY;
        result += abs(stepX+stepY); // (either stepX or stepY is 0)
    }
    return 0; // no border found between 0..max
}


/**
 * Detects a border of completely non-black pixels around the area outsideBorder[LEFT],outsideBorder[TOP]-outsideBorder[RIGHT],outsideBorder[BOTTOM].
 */
void detectBorder(int border[EDGES_COUNT], int borderScanDirections, int borderScanSize[DIRECTIONS_COUNT], int borderScanStep[DIRECTIONS_COUNT], int borderScanThreshold[DIRECTIONS_COUNT], float blackThreshold, int outsideMask[EDGES_COUNT], struct IMAGE* image) {
    int blackThresholdAbs;
    
    border[LEFT] = outsideMask[LEFT];
    border[TOP] = outsideMask[TOP];
    border[RIGHT] = image->width - outsideMask[RIGHT];
    border[BOTTOM] = image->height - outsideMask[BOTTOM];
    
    blackThresholdAbs = (int)(WHITE * (1.0 - blackThreshold));
    if (borderScanDirections & 1<<HORIZONTAL) {
        border[LEFT] += detectBorderEdge(outsideMask, borderScanStep[HORIZONTAL], 0, borderScanSize[HORIZONTAL], borderScanThreshold[HORIZONTAL], blackThresholdAbs, image);
        border[RIGHT] += detectBorderEdge(outsideMask, -borderScanStep[HORIZONTAL], 0, borderScanSize[HORIZONTAL], borderScanThreshold[HORIZONTAL], blackThresholdAbs, image);
    }
    if (borderScanDirections & 1<<VERTICAL) {
        border[TOP] += detectBorderEdge(outsideMask, 0, borderScanStep[VERTICAL], borderScanSize[VERTICAL], borderScanThreshold[VERTICAL], blackThresholdAbs, image);
        border[BOTTOM] += detectBorderEdge(outsideMask, 0, -borderScanStep[VERTICAL], borderScanSize[VERTICAL], borderScanThreshold[VERTICAL], blackThresholdAbs, image);
    }
    if (verbose >= VERBOSE_NORMAL) {
        printf("border detected: (%d,%d,%d,%d) in [%d,%d,%d,%d]\n", border[LEFT], border[TOP], border[RIGHT], border[BOTTOM], outsideMask[LEFT], outsideMask[TOP], outsideMask[RIGHT], outsideMask[BOTTOM]);
    }
}


/**
 * Converts a border-tuple to a mask-tuple.
 */
void borderToMask(int border[EDGES_COUNT], int mask[EDGES_COUNT], struct IMAGE* image) {
    mask[LEFT] = border[LEFT];
    mask[TOP] = border[TOP];
    mask[RIGHT] = image->width - border[RIGHT] - 1;
    mask[BOTTOM] = image->height - border[BOTTOM] - 1;
    if (verbose >= VERBOSE_DEBUG) {
        printf("border [%d,%d,%d,%d] -> mask [%d,%d,%d,%d]\n", border[LEFT], border[TOP], border[RIGHT], border[BOTTOM], mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM]);
    }
}


/**
 * Applies a border to the whole image. All pixels in the border range at the
 * edges of the sheet will be cleared.
 */
void applyBorder(int border[EDGES_COUNT], int borderColor, struct IMAGE* image) {
    int mask[EDGES_COUNT];
    
    if (border[LEFT]!=0 || border[TOP]!=0 || border[RIGHT]!=0 || border[BOTTOM]!=0) {
        borderToMask(border, mask, image);
        if (verbose >= VERBOSE_NORMAL) {
            printf("applying border (%d,%d,%d,%d) [%d,%d,%d,%d]\n", border[LEFT], border[TOP], border[RIGHT], border[BOTTOM], mask[LEFT], mask[TOP], mask[RIGHT], mask[BOTTOM]);
        }
        applyMasks(&mask, 1, borderColor, image);
    }
}
