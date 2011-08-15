#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --sheet-size 20cm,10cm tests/imgsrc003.ppm tests/resultsD1.ppm >> ${output_file}

cat ${output_file}

echo "Done: [D1] Crop to sheet size."
