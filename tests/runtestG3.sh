#!/bin/sh

# SPDX-FileCopyrightText: 2005 The unpaper authors
#
# SPDX-License-Identifier: GPL-2.0-only

echo "[G3] No processing, without overwrite on an existing file."

set -e
set -x

rm -f tests/resultsG3.pbm
touch tests/resultsG3.pbm

./unpaper -v --no-processing 1 ${srcdir:-.}/tests/imgsrc001.png tests/resultsG3.pbm

[ -s tests/resultsG3.pbm ]

./compare-image ${srcdir:-.}/tests/goldenG3.pbm tests/resultsG3.pbm
