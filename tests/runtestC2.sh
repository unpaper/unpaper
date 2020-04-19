#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[C2] Explicit shifting."

set -e
set -x

rm -f tests/resultsC2.pbm
./unpaper -v -n --sheet-size a4 --pre-shift -5cm,9cm ${srcdir:-.}/tests/imgsrc002.png tests/resultsC2.pbm

[ -f tests/resultsC2.pbm ]

./compare-image ${srcdir:-.}/tests/goldenC2.pbm tests/resultsC2.pbm
