#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence tests/imgsrc003.ppm tests/imgsrc004.pgm --output-file-sequence tests/resultsB1.ppm >> ${output_file}

cat ${output_file}

echo "Done: [B1] Combined Color/Gray, No Processing."
