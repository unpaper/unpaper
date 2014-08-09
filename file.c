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
    int x, y, ret, got_frame = 0;
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

    if (verbose >= VERBOSE_MORE)
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

    switch(frame->format) {
    case AV_PIX_FMT_MONOBLACK:
	image->frame = av_frame_alloc();
	image->frame->width = frame->width;
	image->frame->height = frame->height;
	image->frame->format = AV_PIX_FMT_MONOWHITE;

	ret = av_frame_get_buffer(image->frame, 8);
	if (ret < 0) {
	    av_strerror(ret, errbuff, sizeof(errbuff));
	    errOutput("unable to open file %s: %s", filename, errbuff);
	}

	src = frame->data[0];
	dst = image->frame->data[0];
	for (y = 0; y < frame->height; y++) {
	    for (x = 0; x < frame->linesize[0]; x++) {
                dst[x] = ~src[x];
	    }
	    src += frame->linesize[0];
	    dst += image->frame->linesize[0];
	}
	break;

    case AV_PIX_FMT_Y400A: // 8-bit grayscale PNG
    case AV_PIX_FMT_GRAY8:
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_MONOWHITE:
	image->frame = frame;
	break;

    default:
	errOutput("unable to open file %s: unsupported pixel format", filename);
    }
}


/**
 * Saves image data to a file in pgm or pbm format.
 *
 * @param filename file name to save image to
 * @param image image to save
 * @param type filetype of the image to save
 * @param blackThreshold threshold for grayscale-to-black&white conversion
 * @return true on success, false on failure
 */
void saveImage(char *filename, struct IMAGE* image, int outputPixFmt, float blackThreshold) {
    AVOutputFormat *fmt = NULL;
    enum AVCodecID output_codec = -1;
    AVCodec *codec;
    AVFormatContext *out_ctx;
    AVCodecContext *codec_ctx;
    AVStream *video_st;
    int ret;
    char errbuff[1024];

    fmt = av_guess_format("image2", NULL, NULL);

    if ( !fmt ) {
        errOutput("could not find suitable output format.");
    }

    out_ctx = avformat_alloc_context();
    if ( !out_ctx ) {
        errOutput("unable to allocate output context.");
    }

    out_ctx->oformat = fmt;
    snprintf(out_ctx->filename, sizeof(out_ctx->filename), "%s", filename);

    switch (outputPixFmt) {
    case AV_PIX_FMT_RGB24:
        output_codec = AV_CODEC_ID_PPM;
        break;
    case AV_PIX_FMT_Y400A:
    case AV_PIX_FMT_GRAY8:
        outputPixFmt = AV_PIX_FMT_GRAY8;
        output_codec = AV_CODEC_ID_PGM;
        break;
    case AV_PIX_FMT_MONOWHITE:
        output_codec = AV_CODEC_ID_PBM;
        break;
    }

    if ( image->frame->format != outputPixFmt ) {
        struct IMAGE output;
        initImage(&output, image->frame->width, image->frame->height,
                  outputPixFmt, image->background);
        if ( outputPixFmt == AV_PIX_FMT_MONOWHITE ) {
            const int blackThresholdAbs = WHITE * (1.0 - blackThreshold);
            int x, y;
            for (y = 0; y < image->frame->height; y++) {
                for (x = 0; x < image->frame->width; x++) {
                    const int pixel = getPixelGrayscale(x, y, image);
                    setPixel((pixel < blackThresholdAbs ? BLACK24 : WHITE24), x, y, &output);
                }
            }
        } else {
            copyImageArea(0, 0, image->frame->width, image->frame->height,
                          image, 0, 0, &output);
        }
        replaceImage(image, &output);
    }

    codec = avcodec_find_encoder(output_codec);
    if ( !codec ) {
        errOutput("output codec not found");
    }

    video_st = avformat_new_stream(out_ctx, codec);
    if ( !video_st ) {
        errOutput("could not alloc output stream");
    }

    codec_ctx = video_st->codec;
    codec_ctx->width = image->frame->width;
    codec_ctx->height = image->frame->height;
    codec_ctx->pix_fmt = image->frame->format;
    codec_ctx->time_base.den = 1;
    codec_ctx->time_base.num = 1;

    ret = avcodec_open2(codec_ctx, NULL, NULL);

    if (ret < 0) {
        av_strerror(ret, errbuff, sizeof(errbuff));
        errOutput("unable to open codec: %s", errbuff);
    }

    if (verbose >= VERBOSE_MORE)
	av_dump_format(out_ctx, 0, filename, 1);

    if (avio_open(&out_ctx->pb, filename, AVIO_FLAG_WRITE) < 0) {
        errOutput("could not open '%s'", filename);
    }

    avformat_write_header(out_ctx, NULL);

    AVPacket pkt;
    int got_packet;
    av_init_packet(&pkt);

    /* encode the image */
    ret = avcodec_encode_video2(video_st->codec, &pkt, image->frame, &got_packet);

    if (ret < 0) {
        av_strerror(ret, errbuff, sizeof(errbuff));
        errOutput("unable to write file %s: %s", filename, errbuff);
    }
    av_write_frame(out_ctx, &pkt);

    av_write_trailer(out_ctx);
    avcodec_close(codec_ctx);

    av_free(codec_ctx);
    av_free(video_st);

    avio_close(out_ctx->pb);
    av_free(out_ctx);
}

/**
 * Saves the image if full debugging mode is enabled.
 */
void saveDebug(char* filename, struct IMAGE* image) {
    //saveImage(filename, image);
}
