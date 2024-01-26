// Copyright © 2005-2007 Jens Gulden
// Copyright © 2011-2011 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

/* --- tool functions for file handling ------------------------------------ */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>

#include "imageprocess/blit.h"
#include "unpaper.h"

/**
 * Loads image data from a file in pnm format.
 *
 * @param f file to load
 * @param image structure to hold loaded image
 * @param type returns the type of the loaded image
 */
void loadImage(const char *filename, AVFrame **image) {
  int ret;
  AVFormatContext *s = NULL;
  AVCodecContext *avctx = NULL;
  const AVCodec *codec;
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

  codec = avcodec_find_decoder(s->streams[0]->codecpar->codec_id);
  if (!codec)
    errOutput("unable to open file %s: unsupported format", filename);

  avctx = avcodec_alloc_context3(codec);
  if (!avctx)
    errOutput("cannot allocate decoder context for %s", filename);

  ret = avcodec_parameters_to_context(avctx, s->streams[0]->codecpar);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof errbuff);
    errOutput("unable to copy parameters to context: %s", errbuff);
  }

  ret = avcodec_open2(avctx, codec, NULL);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof errbuff);
    errOutput("unable to open file %s: %s", filename, errbuff);
  }

  ret = av_read_frame(s, &pkt);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof errbuff);
    errOutput("unable to open file %s: %s", filename, errbuff);
  }

  if (pkt.stream_index != 0)
    errOutput("unable to open file %s: invalid stream.", filename);

  ret = avcodec_send_packet(avctx, &pkt);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof errbuff);
    errOutput("cannot send packet to decoder: %s", errbuff);
  }

  ret = avcodec_receive_frame(avctx, frame);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof errbuff);
    errOutput("error while receiving frame from decoder: %s", errbuff);
  }

  switch (frame->format) {
  case AV_PIX_FMT_Y400A: // 8-bit grayscale PNG
  case AV_PIX_FMT_GRAY8:
  case AV_PIX_FMT_RGB24:
  case AV_PIX_FMT_MONOBLACK:
  case AV_PIX_FMT_MONOWHITE:
    *image = av_frame_clone(frame);
    break;

  case AV_PIX_FMT_PAL8: {
    Rectangle area = full_image(frame);

    *image = create_image(size_of_rectangle(area), AV_PIX_FMT_RGB24, false,
                          sheetBackgroundPixel, absBlackThreshold);

    const uint32_t *palette = (const uint32_t *)frame->data[1];
    scan_rectangle(area) {
      const uint8_t palette_index = frame->data[0][frame->linesize[0] * y + x];
      set_pixel(*image, (Point){x, y},
                pixelValueToPixel(palette[palette_index]), absBlackThreshold);
    }
  } break;

  default:
    errOutput("unable to open file %s: unsupported pixel format", filename);
  }

  avcodec_free_context(&avctx);
  avformat_close_input(&s);
}

/**
 * Saves image data to a file in pgm or pbm format.
 *
 * @param filename file name to save image to
 * @param image image to save
 * @param type filetype of the image to save
 * @return true on success, false on failure
 */
void saveImage(char *filename, AVFrame *input, int outputPixFmt) {
  enum AVCodecID output_codec = -1;
  const AVCodec *codec;
  AVFormatContext *out_ctx;
  AVCodecContext *codec_ctx;
  AVStream *video_st;
  AVFrame *output = input;
  AVPacket *pkt = NULL;
  int ret;
  char errbuff[1024];

  if (avformat_alloc_output_context2(&out_ctx, NULL, "image2", filename) < 0 ||
      out_ctx == NULL) {
    errOutput("unable to allocate output context.");
  }

  if ((ret = av_opt_set(out_ctx->priv_data, "update", "true", 0)) < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to configure update option: %s", errbuff);
  }

  switch (outputPixFmt) {
  case AV_PIX_FMT_RGB24:
    output_codec = AV_CODEC_ID_PPM;
    break;
  case AV_PIX_FMT_Y400A:
  case AV_PIX_FMT_GRAY8:
    outputPixFmt = AV_PIX_FMT_GRAY8;
    output_codec = AV_CODEC_ID_PGM;
    break;
  case AV_PIX_FMT_MONOBLACK:
  case AV_PIX_FMT_MONOWHITE:
    outputPixFmt = AV_PIX_FMT_MONOWHITE;
    output_codec = AV_CODEC_ID_PBM;
    break;
  default:
    output_codec = -1;
    break;
  }

  if (input->format != outputPixFmt) {
    output =
        create_image((RectangleSize){input->width, input->height}, outputPixFmt,
                     false, sheetBackgroundPixel, absBlackThreshold);
    copy_rectangle(input, output, full_image(input), POINT_ORIGIN,
                   absBlackThreshold);
  }

  codec = avcodec_find_encoder(output_codec);
  if (!codec) {
    errOutput("output codec not found");
  }

  video_st = avformat_new_stream(out_ctx, codec);
  if (!video_st) {
    errOutput("could not alloc output stream");
  }

  codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    errOutput("could not alloc codec context");
  }

  codec_ctx->width = output->width;
  codec_ctx->height = output->height;
  codec_ctx->pix_fmt = output->format;
  video_st->codecpar->width = output->width;
  video_st->codecpar->height = output->height;
  video_st->codecpar->format = output->format;
  video_st->time_base.den = codec_ctx->time_base.den = 1;
  video_st->time_base.num = codec_ctx->time_base.num = 1;

  ret = avcodec_open2(codec_ctx, codec, NULL);

  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to open codec: %s", errbuff);
  }

  if (verbose >= VERBOSE_MORE)
    av_dump_format(out_ctx, 0, filename, 1);

  if ((ret = avio_open(&out_ctx->pb, filename, AVIO_FLAG_WRITE)) < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("cannot alloc I/O context for %s: %s", filename, errbuff);
  }

  if ((ret = avformat_write_header(out_ctx, NULL)) < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("error writing header to '%s': %s", filename, errbuff);
  }

  pkt = av_packet_alloc();
  if (!pkt) {
    errOutput("unable to allocate output packet");
  }

  ret = avcodec_send_frame(codec_ctx, output);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to send frame to encoder: %s", errbuff);
  }

  ret = avcodec_receive_packet(codec_ctx, pkt);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    errOutput("unable to receive packet from encoder: %s", errbuff);
  }

  av_write_frame(out_ctx, pkt);

  av_write_trailer(out_ctx);

  av_packet_free(&pkt);
  avcodec_free_context(&codec_ctx);
  avformat_free_context(out_ctx);

  if (output != input)
    av_frame_free(&output);
}

/**
 * Saves the image if full debugging mode is enabled.
 */
void saveDebug(char *filenameTemplate, int index, AVFrame *image) {
  if (verbose >= VERBOSE_DEBUG_SAVE) {
    char debugFilename[100];
    sprintf(debugFilename, filenameTemplate, index);
    saveImage(debugFilename, image, image->format);
  }
}
