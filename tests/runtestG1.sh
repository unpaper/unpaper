#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[G1] No processing, with overwrite on non-existing file."

set -e
set -x

rm -f tests/resultsG1.pbm

./unpaper --overwrite -v --no-processing 1 ${srcdir:-.}/tests/imgsrc001.png tests/resultsG1.pbm

[ -f tests/resultsG1.pbm ]

./compare-image ${srcdir:-.}/tests/goldenG1.pbm tests/resultsG1.pbm
