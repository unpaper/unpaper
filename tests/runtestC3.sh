#!/bin/sh

echo "[C3] Explicit -1 size shifting."

. tests/prologue.sh

$UNPAPER -v --overwrite -n --sheet-size a4 --pre-shift -1cm tests/imgsrc002.pbm tests/resultsC3.pbm
