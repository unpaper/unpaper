# SPDX-FileCopyrightText: 2020 The unpaper authors
#
# SPDX-License-Identifier: MIT

project('unpaper', 'c', default_options : ['c_std=c11'])

cc = meson.get_compiler('c')

unpaper_deps = [
    dependency('libavformat'), dependency('libavcodec'), dependency('libavutil'),
    cc.find_library('m', required : false)
]

executable(
    'unpaper',
    'file.c', 'imageprocess.c', 'parse.c', 'tools.c', 'unpaper.c',
    dependencies : unpaper_deps,
    install : true,
)
