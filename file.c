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

/* --- tool functions for file handling ------------------------------------ */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

#include "unpaper.h"
#include "tools.h" //only needed for getPixelGrayscale

/**
 * Loads image data from a file in pnm format.
 *
 * @param f file to load
 * @param image structure to hold loaded image
 * @param type returns the type of the loaded image
 */
void loadImage(const char *filename, struct IMAGE* image) {
    uint8_t *src, *dst;
    uint8_t r, g, b;
    int x, y, ret, got_frame = 0, size, pos;
    AVFormatContext *s = NULL;
    AVCodecContext *avctx = NULL;
    AVCodec *codec;
    AVPacket pkt;
    AVFrame *frame = av_frame_alloc();
    char errbuff[1024];

    ret = avformat_open_input(&s, filename, NULL, NULL);
    if (ret < 0) {
	av_strerror(ret, errbuff, sizeof(errbuff));
	errOutput("unable to open file %s: %s", filename, errbuff);
    }

    avformat_find_stream_info(s, NULL);

    av_dump_format(s, 0, filename, 0);

    if (s->nb_streams < 1)
	errOutput("unable to open file %s: missing streams", filename);

    if (s->streams[0]->codec->codec_type != AVMEDIA_TYPE_VIDEO)
	errOutput("unable to open file %s: wrong stream", filename);

    avctx = s->streams[0]->codec;

    codec = avcodec_find_decoder(avctx->codec_id);
    if (!codec)
	errOutput("unable to open file %s: unsupported format", filename);

    ret = avcodec_open2(avctx, codec, NULL);
    if (ret < 0) {
	av_strerror(ret, errbuff, sizeof(errbuff));
	errOutput("unable to open file %s: %s", filename, errbuff);
    }

    ret = av_read_frame(s, &pkt);
    if (ret < 0) {
	av_strerror(ret, errbuff, sizeof(errbuff));
	errOutput("unable to open file %s: %s", filename, errbuff);
    }

    if (pkt.stream_index != 0)
	errOutput("unable to open file %s: invalid stream.", filename);

    ret = avcodec_decode_video2(s->streams[0]->codec, frame, &got_frame, &pkt);
    if (ret < 0) {
	av_strerror(ret, errbuff, sizeof(errbuff));
	errOutput("unable to open file %s: %s", filename, errbuff);
    }

    image->width = frame->width;
    image->height = frame->height;

    switch(frame->format) {
    case AV_PIX_FMT_MONOWHITE:
	image->bitdepth = 1;
	image->color = false;
	image->buffer = malloc(frame->width * frame->height);
	src = frame->data[0];
	dst = image->buffer;
	for (y = 0; y < frame->height; y++) {
	    for (x = 0; x < frame->width; x++) {
		int bb = x >> 3; // x / 8;
                int off = x & 7; // x % 8;
                int bit = 128 >> off;
                int bits = src[bb] & bit;

		// 0: white pixel
		dst[x] = (bits == 0 ? 0xff : 0x00);
	    }
	    src += frame->linesize[0];
	    dst += frame->width;
	}
	image->bufferGrayscale = image->buffer;
	image->bufferLightness = image->buffer;
	image->bufferDarknessInverse = image->buffer;
	break;

    case AV_PIX_FMT_GRAY8:
	image->bitdepth = 8;
	image->color = false;
	image->buffer = malloc(frame->width * frame->height);
	src = frame->data[0];
	dst = image->buffer;
	for (y = 0; y < frame->height; y++) {
	    memcpy(dst, src, frame->width);
	    src += frame->linesize[0];
	    dst += frame->width;
	}
	image->bufferGrayscale = image->buffer;
	image->bufferLightness = image->buffer;
	image->bufferDarknessInverse = image->buffer;
	break;

    case AV_PIX_FMT_RGB24:
	image->bitdepth = 8;
	image->color = true;
	image->buffer = malloc(frame->width * frame->height * 3);
	src = frame->data[0];
	dst = image->buffer;
	for (y = 0; y < frame->height; y++) {
	    memcpy(dst, src, frame->width * 3);
	    src += frame->linesize[0];
	    dst += frame->width * 3;
	}
	size = frame->width * frame->height;
	image->bufferGrayscale = malloc(size);
	image->bufferLightness = malloc(size);
	image->bufferDarknessInverse = malloc(size);
        src = image->buffer;
        for (pos = 0; pos < size; pos++) {
            r = *src;
            src++;
            g = *src;
            src++;
            b = *src;
            src++;            
            image->bufferGrayscale[pos] = pixelGrayscale(r, g, b);
            image->bufferLightness[pos] = pixelLightness(r, g, b);
            image->bufferDarknessInverse[pos] = pixelDarknessInverse(r, g, b);
        }
	break;

    default:
	errOutput("unable to open file %s: unsupported pixel format", filename);
    }
}


/**
 * Saves image data to a file in pgm or pbm format.
 *
 * @param outputFile file to save image to
 * @param image image to save
 * @param type filetype of the image to save
 * @param blackThreshold threshold for grayscale-to-black&white conversion
 * @return true on success, false on failure
 */
void saveImage(FILE *outputFile, struct IMAGE* image, int type, float blackThreshold) {
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
    int blackThresholdAbs;

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
    fprintf(outputFile, "%s\n", outputMagic);
    fprintf(outputFile, "# generated by unpaper\n");
    fprintf(outputFile, "%u %u\n", image->width, image->height);
    if ((type == PGM)||(type == PPM)) {
      fprintf(outputFile, "255\n"); // maximum color index per color-component
    }
    fwrite(buf, 1, outputSize, outputFile);

    if (buf != image->buffer) {
        free(buf);
    }
}    


/**
 * Saves the image if full debugging mode is enabled.
 */
void saveDebug(char* filename, struct IMAGE* image) {
    int type;
    
    if (verbose >= VERBOSE_DEBUG_SAVE) {
        FILE *f = fopen(filename, "w");

        if (image->color) {
            type = PPM;
        } else if (image->bitdepth == 1) {
            type = PBM;
        } else {
            type = PGM;
        }
        saveImage(f, image, type, 0.5); // 0.5 is a dummy, not used because PGM depth

        fclose(f);
    }
}
