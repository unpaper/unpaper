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

sphinx = find_program('sphinx-build', required: true)

custom_target(
    'man',
    command: [sphinx, '-b', 'man', join_paths(meson.source_root(), 'doc'), '@OUTDIR@'],
    input: ['doc/conf.py', 'doc/index.rst', 'doc/unpaper.1.rst'],
    output: 'unpaper.1',
    install: true,
    install_dir: join_paths(get_option('prefix'), get_option('mandir'), 'man1'),
)
