#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite -n --input-pages 2 --input-file-sequence tests/imgsrc003.ppm tests/imgsrc005.pbm --output-file-sequence tests/resultsB2.ppm >> ${output_file}

cat ${output_file}

echo "Done: [B2] Combined Color/Black+White, No Processing."
