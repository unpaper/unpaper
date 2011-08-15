#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence tests/imgsrc004.pgm tests/imgsrc005.pbm --output-file-sequence tests/resultsB3.ppm >> ${output_file}

cat ${output_file}

echo "Done: [B3] Combined Gray/Black+White, No Processing."
