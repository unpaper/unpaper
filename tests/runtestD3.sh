#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --stretch 20cm,10cm tests/imgsrc003.ppm tests/resultsD3.ppm >> ${output_file}

cat ${output_file}

echo "Done: [D3] Stretch to sheet size."
