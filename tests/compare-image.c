// Copyright © 2014 Diego Elio Pettenò
// SPDX-FileCopyrightText: 2005 The unpaper authors
//
// SPDX-License-Identifier: GPL-2.0-only

#include <stdio.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

AVFrame *fetch_frame_from_file(const char *filename) {
  int ret, got_frame = 0;
  AVFormatContext *s = NULL;
  AVCodecContext *avctx = avcodec_alloc_context3(NULL);
  AVCodec *codec;
  AVPacket pkt;
  AVFrame *frame = av_frame_alloc();
  char errbuff[1024];

  if (!avctx) {
    fprintf(stderr, "cannot allocate a new context\n");
    return NULL;
  }

  ret = avformat_open_input(&s, filename, NULL, NULL);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    fprintf(stderr, "unable to open file %s: %s\n", filename, errbuff);
    return NULL;
  }

  avformat_find_stream_info(s, NULL);

  if (s->nb_streams < 1) {
    fprintf(stderr, "unable to open file %s: missing streams\n", filename);
    return NULL;
  }

  if (s->streams[0]->codec->codec_type != AVMEDIA_TYPE_VIDEO) {
    fprintf(stderr, "unable to open file %s: wrong stream\n", filename);
    return NULL;
  }

  ret = avcodec_copy_context(avctx, s->streams[0]->codec);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    fprintf(stderr, "cannot set the new context for %s: %s\n", filename,
            errbuff);
    return NULL;
  }

  codec = avcodec_find_decoder(avctx->codec_id);
  if (!codec) {
    fprintf(stderr, "unable to open file %s: unsupported format\n", filename);
    return NULL;
  }

  ret = avcodec_open2(avctx, codec, NULL);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    fprintf(stderr, "unable to open file %s: %s\n", filename, errbuff);
    return NULL;
  }

  ret = av_read_frame(s, &pkt);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    fprintf(stderr, "unable to open file %s: %s\n", filename, errbuff);
    return NULL;
  }

  if (pkt.stream_index != 0) {
    fprintf(stderr, "unable to open file %s: invalid stream.\n", filename);
    return NULL;
  }

  ret = avcodec_decode_video2(avctx, frame, &got_frame, &pkt);
  if (ret < 0) {
    av_strerror(ret, errbuff, sizeof(errbuff));
    fprintf(stderr, "unable to open file %s: %s\n", filename, errbuff);
    return NULL;
  }

  return frame;
}

int main(int argc, char *argv[]) {
  AVFrame *golden, *result;
  float allowed_difference = 0.1;

  if (argc == 4) {
    allowed_difference = strtof(argv[3], NULL);
  } else if (argc != 3) {
    fprintf(stderr, "Usage: %s golden-file result-file [allowed-difference]\n",
            argv[0]);
    return 1;
  }

  avcodec_register_all();
  av_register_all();

  golden = fetch_frame_from_file(argv[1]);
  result = fetch_frame_from_file(argv[2]);

  if (!golden || !result)
    return 1;

  if (golden->height != result->height || golden->width != result->width) {
    fprintf(stderr, "image sizes don't match: %d×%d vs %d×%d\n", golden->height,
            golden->width, result->height, result->width);
    return 1;
  }

  if (golden->format != result->format) {
    fprintf(stderr, "image format does not match.\n");
    return 1;
  }

  const size_t image_size = golden->height * golden->width;
  size_t diffbytes = 0;

  for (int y = 0; y < golden->height; y++) {
    for (int x = 0; x < golden->width; x++) {
      size_t coord = golden->linesize[0] * y + x;
      diffbytes += !!(golden->data[0][coord] ^ result->data[0][coord]);
    }
  }

  const float percent_diff = (diffbytes / (float)image_size) * 100.0;

  fprintf(stderr, "%zd bytes differ over %zd — %.2f%% (limit: %.2f%%)\n",
          diffbytes, image_size, percent_diff, allowed_difference);

  if (percent_diff > allowed_difference)
    return 1;

  return 0;
}
