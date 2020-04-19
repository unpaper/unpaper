#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[C3] Explicit -1 size shifting."

set -e
set -x

rm -f tests/resultsC3.pbm
./unpaper -v -n --sheet-size a4 --pre-shift -1cm ${srcdir:-.}/tests/imgsrc002.png tests/resultsC3.pbm

[ -f tests/resultsC3.pbm ]

./compare-image ${srcdir:-.}/tests/goldenC3.pbm tests/resultsC3.pbm
