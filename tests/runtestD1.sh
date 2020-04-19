#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[D1] Crop to sheet size."

set -e
set -x

rm -f tests/resultsD1.ppm
./unpaper -v -n --sheet-size 20cm,10cm ${srcdir:-.}/tests/imgsrc003.png tests/resultsD1.ppm

[ -f tests/resultsD1.ppm ]

./compare-image ${srcdir:-.}/tests/goldenD1.ppm tests/resultsD1.ppm
