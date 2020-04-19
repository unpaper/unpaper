#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[A1] Single-Page Template Layout, Black+White, Full Processing."

set -e
set -x

rm -f tests/resultsA1.pbm
./unpaper -v ${srcdir:-.}/tests/imgsrc001.png tests/resultsA1.pbm

[ -f tests/resultsA1.pbm ]

./compare-image ${srcdir:-.}/tests/goldenA1.pbm tests/resultsA1.pbm
