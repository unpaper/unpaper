unpaper
=======

Originally written by Jens Gulden — see AUTHORS for more information.
Licensed under GNU GPL v2 — see COPYING for more information.

Overview
--------

`unpaper` is a post-processing tool for scanned sheets of paper,
especially for book pages that have been scanned from previously
created photocopies.  The main purpose is to make scanned book pages
better readable on screen after conversion to PDF. Additionally,
`unpaper` might be useful to enhance the quality of scanned pages
before performing optical character recognition (OCR).

`unpaper` tries to clean scanned images by removing dark edges that
appeared through scanning or copying on areas outside the actual page
content (e.g.  dark areas between the left-hand-side and the
right-hand-side of a double- sided book-page scan).

The program also tries to detect misaligned centering and rotation of
pages and will automatically straighten each page by rotating it to
the correct angle. This process is called "deskewing".

Note that the automatic processing will sometimes fail. It is always a
good idea to manually control the results of unpaper and adjust the
parameter settings according to the requirements of the input. Each
processing step can also be disabled individually for each sheet.

Input files can be in many format, depending on the support coming
from [libav](http://libav.org). PNM formats (as used by scanning tools
such as `scanimage` and `scanadf`) are fully supported. Basic support
for some variants of TIFF and PNG files is also present.

Output files are generated in PNM rawbits format. Conversion to PDF
can be achieved with Linux tools such as `pgm2tiff`, `tiffcp` and
`tiff2pdf`.
