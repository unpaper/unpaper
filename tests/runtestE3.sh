#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[E3] Splitting 2-page layout into separate output pages (with explicit input and output)."

set -e
set -x

for i in 1 2; do
    rm -f tests/resultsE3-0$i.pbm
done
./unpaper -v --layout double --output-pages 2 ${srcdir:-.}/tests/imgsrcE001.png tests/resultsE3-01.pbm tests/resultsE3-02.pbm

for i in 1 2; do
    [ -f tests/resultsE3-0$i.pbm ]

    ./compare-image ${srcdir:-.}/tests/goldenE3-0$i.pbm tests/resultsE3-0$i.pbm
done
