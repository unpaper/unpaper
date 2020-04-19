#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[E2] Splitting 2-page layout into separate output pages (with output wildcard only)."

set -e
set -x

for i in 1 2; do
    rm -f tests/resultsE2-0$i.pbm
done
./unpaper -v --layout double --output-pages 2 ${srcdir:-.}/tests/imgsrcE001.png tests/resultsE2-%02d.pbm

for i in 1 2; do
    [ -f tests/resultsE2-0$i.pbm ]

    ./compare-image ${srcdir:-.}/tests/goldenE2-0$i.pbm tests/resultsE2-0$i.pbm
done
