# SPDX-FileCopyrightText: 2020 The unpaper authors
#
# SPDX-License-Identifier: MIT

project('unpaper', 'c', version : '7.0.0', default_options : ['c_std=c11'])

if not meson.version().version_compare('>0.57')
    error('Meson 0.57 or later required')
endif

add_global_arguments('-D_POSIX_C_SOURCE=200809L', language : 'c')

cc = meson.get_compiler('c')

if not cc.has_function('strdup', prefix : '#include <string.h>', args : ['-D_POSIX_C_SOURCE=200809L'])
    error('A C library compatible with POSIX.1-2008 is required.')
endif

if cc.has_argument('-Werror=int-conversion')
    add_project_arguments('-Werror=int-conversion', language : 'c')
endif

unpaper_deps = [
    dependency('libavformat'), dependency('libavcodec'), dependency('libavutil'),
    cc.find_library('m', required : false)
]

conf_data = configuration_data()
conf_data.set('version', meson.project_version())
configure_file(input: 'version.h.in', output: 'version.h', configuration: conf_data)

unpaper = executable(
    'unpaper',
    'file.c', 'imageprocess.c', 'parse.c', 'tools.c', 'unpaper.c',
    dependencies : unpaper_deps,
    install : true,
)

sphinx = find_program('sphinx-build', required: true, version: '>= 3.4')

custom_target(
    'man',
    command: [sphinx, '-b', 'man', join_paths(meson.source_root(), 'doc'), '@OUTDIR@'],
    input: ['doc/conf.py', 'doc/index.rst', 'doc/unpaper.1.rst'],
    output: 'unpaper.1',
    install: true,
    install_dir: join_paths(get_option('prefix'), get_option('mandir'), 'man1'),
)

pymod = import('python')
python = pymod.find_installation(required: false, modules: ['pytest', 'PIL'], disabler: true)

if not python.found()
    warning('pytest or Pillow not found, tests will not be performed.')
endif

test(
    'pytest suite',
    python,
    args: [
        '-m', 'pytest', meson.project_source_root() + '/tests/unpaper_tests.py'
    ],
    env: [
        'TEST_IMGSRC_DIR=' + meson.project_source_root() + '/tests/source_images/',
        'TEST_GOLDEN_DIR=' + meson.project_source_root() + '/tests/golden_images/',
        'TEST_UNPAPER_BINARY=' + unpaper.full_path(),
    ],
    timeout : -1,
)
