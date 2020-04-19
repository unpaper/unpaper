#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[F2] Merging 2-page layout into single output page (with output wildcard only)."

set -e
set -x

rm -f tests/resultsF21.pbm
./unpaper -v --layout double --input-pages 2 ${srcdir:-.}/tests/imgsrcE001.png ${srcdir:-.}/tests/imgsrcE002.png tests/resultsF2%d.pbm

[ -f tests/resultsF21.pbm ]

./compare-image ${srcdir:-.}/tests/goldenF21.pbm tests/resultsF21.pbm
