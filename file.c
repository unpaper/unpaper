/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */

/* --- tool functions for file handling ------------------------------------ */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "unpaper.h"
#include "tools.h" //only needed for getPixelGrayscale



/**
 * Tests if a file exists.
 */
bool fileExists(char* filename) {
    FILE *f;
    f = fopen(filename,"r");
    if (f == NULL) {
        return false;
    } else {
        fclose(f);
        return true;
    }
}


/**
 * Loads image data from a file in pnm format.
 *
 * @param filename name of file to load
 * @param image structure to hold loaded image
 * @param type returns the type of the loaded image
 * @return true on success, false on failure
 */
bool loadImage(char* filename, struct IMAGE* image, int* type) {
    FILE *f;
    int bytesPerLine;
    char magic[10];
    char word[255];
    char c;
    int maxColorIndex;
    int inputSize;
    int read;
    uint8_t* buffer2;
    int lineOffsetInput;
    int lineOffsetOutput;
    int x;
    int y;
    int bb;
    int off;
    int bits;
    int bit;
    int pixel;
    int size;
    int pos;
    uint8_t* p;
    uint8_t r, g, b;

    if (verbose>=VERBOSE_MORE) {
        printf("loading file %s.\n", filename);
    }

    // open input file
    f = fopen(filename, "rb");
    if (f == NULL) {
        printf("*** error: Unable to open file %s.\n", filename);
        return false;
    }

    // read magic number
    fread(magic, 1, 2, f);
    magic[2] = 0; // terminate
    if (strcmp(magic, "P4")==0) {
        *type = PBM;
        image->bitdepth = 1;
        image->color = false;
    } else if (strcmp(magic, "P5")==0) {
        *type = PGM;
        image->bitdepth = 8;
        image->color = false;
    } else if (strcmp(magic, "P6")==0) {
        *type = PPM;
        image->bitdepth = 8;
        image->color = true;
    } else {
        printf("*** error: input file format using magic '%s' is unknown.\n", magic);
        return false;
    }

    // get image info: width, height, optionally depth
    fgetc(f); // skip \n after magic number
    fscanf(f, "%s", word);
    while (word[0]=='#') { // skip comment lines
        do {
            fscanf(f, "%c", &c);
        } while ((feof(f)==0)&&(c!='\n'));
        fscanf(f, "%s", word);
    }
    // now reached width/height pair as decimal ascii
    sscanf(word, "%d", &image->width);
    fscanf(f, "%d", &image->height);
    fgetc(f); // skip \n after width/height pair
    if (*type == PBM) {
        bytesPerLine = (image->width + 7) / 8;
    } else { // PGM or PPM
        fscanf(f, "%s", word);
        while (word[0]=='#') { // skip comment lines
            do {
                fscanf(f, "%c", &c);
            } while ((feof(f) == 0) && (c != '\n'));
            fscanf(f, "%s", word);
        }
        // read max color value
        sscanf(word, "%d", &maxColorIndex);
        fgetc(f); // skip \n after max color index
        if (maxColorIndex > 255) {
            printf("*** error: grayscale / color-component bit depths above 8 are not supported.\n");
            return false;
        }
        bytesPerLine = image->width;
        if (*type == PPM) {
            bytesPerLine *= 3; // 3 color-components per pixel
        }
    }

    // read binary image data
    inputSize = bytesPerLine * image->height;

    image->buffer = (uint8_t*)malloc(inputSize);
    read = fread(image->buffer, 1, inputSize, f);
    if (read != inputSize) {
        printf("*** error: Only %d out of %d could be read.\n", read, inputSize);
        return false;
    }
    
    if (*type == PBM) { // internally convert b&w to 8-bit for processing
        buffer2 = (uint8_t*)malloc(image->width * image->height);
        lineOffsetInput = 0;
        lineOffsetOutput = 0;
        for (y = 0; y < image->height; y++) {
            for (x = 0; x < image->width; x++) {
                bb = x >> 3;  // x / 8;
                off = x & 7; // x % 8;
                bit = 128>>off;
                bits = image->buffer[lineOffsetInput + bb];
                bits &= bit;
                if (bits == 0) { // 0: white pixel
                    pixel = 0xff;
                } else {
                    pixel = 0x00;
                }
                buffer2[lineOffsetOutput+x] = pixel; // set as whole byte
            }
            lineOffsetInput += bytesPerLine;
            lineOffsetOutput += image->width;
        }
        free(image->buffer);
        image->buffer = buffer2;
    }
    fclose(f);

    if (*type == PPM) {
        // init cached values for grayscale, lightness and darknessInverse
        size = image->width * image->height;
        image->bufferGrayscale = (uint8_t*)malloc(size);
        image->bufferLightness = (uint8_t*)malloc(size);
        image->bufferDarknessInverse = (uint8_t*)malloc(size);
        p = image->buffer;
        for (pos = 0; pos < size; pos++) {
            r = *p;
            p++;
            g = *p;
            p++;
            b = *p;
            p++;            
            image->bufferGrayscale[pos] = pixelGrayscale(r, g, b);
            image->bufferLightness[pos] = pixelLightness(r, g, b);
            image->bufferDarknessInverse[pos] = pixelDarknessInverse(r, g, b);
        }
    } else {
        image->bufferGrayscale = image->buffer;
        image->bufferLightness = image->buffer;
        image->bufferDarknessInverse = image->buffer;
    }
    
    return true;
}


/**
 * Saves image data to a file in pgm or pbm format.
 *
 * @param filename name of file to save
 * @param image image to save
 * @param type filetype of the image to save
 * @param overwrite allow overwriting existing files
 * @param blackThreshold threshold for grayscale-to-black&white conversion
 * @return true on success, false on failure
 */
bool saveImage(char* filename, struct IMAGE* image, int type, bool overwrite, float blackThreshold) {
    uint8_t* buf;
    int bytesPerLine;
    int inputSize;
    int outputSize;
    int lineOffsetOutput;
    int offsetInput;
    int offsetOutput;
    int x;
    int y;
    int pixel;
    int b;
    int off;
    uint8_t bit;
    uint8_t val;
    const char* outputMagic;
    FILE* outputFile;
    int blackThresholdAbs;
    bool result;

    if (verbose>=VERBOSE_MORE) {
        printf("saving file %s.\n", filename);
    }

    result = true;
    if (type == PBM) { // convert to pbm
        blackThresholdAbs = WHITE * (1.0 - blackThreshold);
        bytesPerLine = (image->width + 7) >> 3; // / 8;
        outputSize = bytesPerLine * image->height;
        buf = (uint8_t*)malloc(outputSize);
        memset(buf, 0, outputSize);
        lineOffsetOutput = 0;
        for (y = 0; y < image->height; y++) {
            for (x = 0; x < image->width; x++) {
                pixel = getPixelGrayscale(x, y, image);
                b = x >> 3; // / 8;
                off = x & 7; // % 8;
                bit = 128>>off;
                val = buf[lineOffsetOutput + b];
                if (pixel < blackThresholdAbs) { // dark
                    val |= bit; // set bit to one: black
                } else { // bright
                    val &= (~bit); // set bit to zero: white
                }
                buf[lineOffsetOutput+b] = val;
            }
            lineOffsetOutput += bytesPerLine;
        }
    } else if (type == PPM) { // maybe convert to color
        outputSize = image->width * image->height * 3;
        if (image->color) { // color already
            buf = image->buffer;
        } else { // convert to color
            buf = (uint8_t*)malloc(outputSize);
            inputSize = image->width * image->height;
            offsetOutput = 0;
            for (offsetInput = 0; offsetInput < inputSize; offsetInput++) {
                pixel = image->buffer[offsetInput];
                buf[offsetOutput++] = pixel;
                buf[offsetOutput++] = pixel;
                buf[offsetOutput++] = pixel;
            }
        }
    } else { // PGM
        outputSize = image->width * image->height;
        buf = image->buffer;
    }
    
    switch (type) {
        case PBM:
            outputMagic = "P4";
            break;
        case PPM:
            outputMagic = "P6";
            break;
        default: // PGM
            outputMagic = "P5";
            break;
    }

    // write to file
    if ( overwrite || ( ! fileExists( filename ) ) ) {
        outputFile = fopen(filename, "wb");
        if (outputFile != 0) {
            fprintf(outputFile, "%s\n", outputMagic);
            fprintf(outputFile, "# generated by unpaper\n");
            fprintf(outputFile, "%u %u\n", image->width, image->height);
            if ((type == PGM)||(type == PPM)) {
                fprintf(outputFile, "255\n"); // maximum color index per color-component
            }
            fwrite(buf, 1, outputSize, outputFile);
            fclose(outputFile);
        } else {
            printf("*** error: Cannot open output file '%s'.\n", filename);
            result = false;
        }
    } else {
        printf("file %s already exists (use --overwrite to replace).\n", filename);
        result = false;
    }
    if (buf != image->buffer) {
        free(buf);
    }
    return result;
}    


/**
 * Saves the image if full debugging mode is enabled.
 */
void saveDebug(char* filename, struct IMAGE* image) {
    int type;
    
    if (verbose >= VERBOSE_DEBUG_SAVE) {
        if (image->color) {
            type = PPM;
        } else if (image->bitdepth == 1) {
            type = PBM;
        } else {
            type = PGM;
        }
        saveImage(filename, image, type, true, 0.5); // 0.5 is a dummy, not used because PGM depth
    }
}
