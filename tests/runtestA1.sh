#!/bin/sh

echo "[A1] Single-Page Template Layout, Black+White, Full Processing."

. tests/prologue.sh

$UNPAPER -v --overwrite tests/imgsrc001.pbm tests/resultsA1.pbm
