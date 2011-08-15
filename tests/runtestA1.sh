#!/bin/sh

. tests/prologue.sh

$UNPAPER --version > ${output_file}
$UNPAPER -v --overwrite tests/imgsrc001.pbm tests/resultsA1.pbm >> ${output_file}

cat ${output_file}

echo "Done: [A1] Single-Page Template Layout, Black+White, Full Processing."
