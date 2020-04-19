#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[B1] Combined Color/Gray, No Processing."

set -e
set -x

rm -f tests/resultsB1.ppm
./unpaper -v -n --input-pages 2 ${srcdir:-.}/tests/imgsrc003.png ${srcdir:-.}/tests/imgsrc004.png tests/resultsB1.ppm

[ -f tests/resultsB1.ppm ]

./compare-image ${srcdir:-.}/tests/goldenB1.ppm tests/resultsB1.ppm
