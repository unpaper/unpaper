#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --size 20cm,10cm tests/imgsrc003.ppm tests/resultsD2.ppm >> ${output_file}

cat ${output_file}

echo "Done: [D2] Fit to sheet size."
