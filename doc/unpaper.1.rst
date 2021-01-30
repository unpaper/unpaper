.. SPDX-FileCopyrightText: 2005 The unpaper Authors
..
.. SPDX-License-Identifier: GPL-2.0-only

:orphan:

unpaper
=======

Synopsis
--------

**unpaper** [*options*] (*input patterns* *output patterns* | *input files* *output files*)

Overview
--------

unpaper is a post-processing tool for scanned sheets of paper,
especially for book pages that have been scanned from previously created
photocopies. The main purpose is to make scanned book pages better
readable on screen after conversion to PDF. Additionally, unpaper might
be useful to enhance the quality of scanned pages before performing
optical character recognition (OCR).

unpaper tries to clean scanned images by removing dark edges that
appeared through scanning or copying on areas outside the actual page
content (e.g. dark areas between the left-hand-side and the
right-hand-side of a double- sided book-page scan). The program also
tries to detect misaligned centering and rotation of pages and will
automatically straighten each page by rotating it to the correct angle.
This process is called "deskewing". Note that the automatic processing
will sometimes fail. It is always a good idea to manually control the
results of unpaper and adjust the parameter settings according to the
requirements of the input. Each processing step can also be disabled
individually for each sheet.

Input and output files can be in either ``.pbm``, ``.pgm`` or ``.ppm``
format, thus generally in ``.pnm`` format, as also used by the Linux
scanning tools ``scanimage`` and ``scanadf``. Conversion to PDF can e.g.
be achieved with the Linux tools ``pgm2tiff``, ``tiffcp`` and
``tiff2pdf``.

Input and Output files
----------------------

Input and output files need to be designed either by using patterns or
an ordered list of input and output files; if patterns are used, such as
``%04d``, then they are substituted for the input and output sheet
number before opening the file for input or output.

If you're not using patterns, then the program expects one or two input
files depending on what is passed as ``--input-pages`` and one or two
output files depending on what is passed as ``--output-pages``, in
order.

Missing output file names are fatal and will stop processing; missing
initial input file names are fatal, and so is any missing input file if
a range of sheets is defined through ``--sheet`` or ``--end-sheet``.

``unpaper`` accepts files in PNM format, which means they might be in
``.pbm``, ``.pgm``, ``.ppm`` or ``.pnm`` format, which is what is
produced by Linux command line scanning tools such as ``scanimage`` and
``scanadf``.

Options
-------

.. program:: unpaper

.. option:: -l { single | double | none } ; --layout { single | double | none }

   Set default layout options for a sheet:

   ``single``
      One page per sheet.

   ``double``
      Two pages per sheet, landscape orientation (one page on the left
      half, one page on the right half).

   ``none``
      No auto-layout, mask-scan-points may individually be specified.

   Using ``single`` or ``double`` automatically sets corresponding
   ``--mask-scan-points``. The default is ``single``.

.. option:: -start sheet ; --start-sheet start-sheet

   Number of first sheet to process in multi-sheet mode. (default: 1)

.. option:: -end sheet ; --end-sheet sheet

   Number of last sheet to process in multi-sheet mode. -1 indicates
   processing until no more input file with the corresponding page
   number is available (default: -1)

.. option:: -# sheet-range ; --sheet sheet-range

   Optionally specifies which sheets to process in the range between
   start-sheet and end-sheet.

.. option:: -x sheet-range ; --exclude sheet-range

   Excludes sheets from processing in the range between start-sheet and
   end-sheet.

.. option:: --pre-rotate { -90 | 90 }

   Rotates the whole image clockwise (``90``) or anti-clockwise
   (``-90``) before any other processing.

.. option:: --post-rotate { -90 | 90 }

   Rotates the whole image clockwise (``90``) or anti-clockwise
   (``-90``) after any other processing.

.. option:: -M { v | h | v,h } ; --pre-mirror { v | h | v,h }

   Mirror the image, after possible pre-rotation. Either ``v`` (for
   vertical mirroring), ``h`` (for horizontal mirroring) or ``v,h`` (for
   both) can be specified.

.. option:: --post-mirror { v | h | v,h }

   Mirror the image, after any other processing except possible
   post-rotation. Either ``v`` (for vertical mirroring), ``h`` (for
   horizontal mirroring) or ``v,h`` (for both) can be specified.

.. option:: --pre-shift h , v

   Shift the image before further processing. Values for h (horizontal
   shift) and v (vertical shift) can either be positive or negative.

.. option:: --post-shift h , v

   Shift the image after other processing. Values for h (horizontal
   shift) and v (vertical shift) can either be positive or negative.

.. option:: --pre-wipe left, top, right, bottom

   Manually wipe out an area before further processing. Any pixel in a
   wiped area will be set to white. Multiple areas to be wiped may be
   specified by multiple occurrences of this options.

.. option:: --post-wipe left, top, right, bottom

   Manually wipe out an area after processing. Any pixel in a wiped area
   will be set to white. Multiple areas to be wiped may be specified by
   multiple occurrences of this options.

.. option:: --pre-border left, top, right, bottom

   Clear the border-area of the sheet before further processing. Any
   pixel in the border area will be set to white.

.. option:: --post-border left, top, right, bottom

   Clear the border-area of the sheet after other processing. Any pixel
   in the border area will be set to white.

.. option:: --pre-mask x1, y1, x2, y2

   Specify masks to apply before any other processing. Any pixel outside
   a mask will be set to white, unless another mask includes this pixel.

   Only pixels inside a mask will remain. Multiple masks may be
   specified. No deskewing will be applied to the masks specified by
   ``--pre-mask``.

.. option:: -s { width, height | size-name } ; --size { width, height | size-name }

   Change the sheet size before other processing is applied. Content on
   the sheet gets zoomed to fit to the appropriate size, but the aspect
   ratio is preserved. Instead, if the sheet's aspect ratio changes, the
   zoomed content gets centered on the sheet.

   Possible values for size-name are: ``a5``, ``a4``, ``a3``,
   ``letter``, ``legal``. All size names can also be applied in rotated
   landscape orientation, use ``a4-landscape``, ``letter-landscape``
   etc.

.. option:: --post-size { width, height | size-name }

   Change the sheet size preserving the content's aspect ratio after
   other processing steps are applied.

.. option:: --stretch { width, height | size-name }

   Change the sheet size before other processing is applied. Content on
   the sheet gets stretched to the specified size, possibly changing the
   aspect ratio.

.. option:: --post-stretch { width, height | size-name }

   Change the sheet size after other processing is applied. Content on
   the sheet gets stretched to the specified size, possibly changing the
   aspect ratio.

.. option:: -z factor ; --zoom factor

   Change the sheet size according to the given factor before other
   processing is done.

.. option:: --post-zoom factor

   Change the sheet size according to the given factor after processing
   is done.

.. option:: -bn { v | h | v, h } ; --blackfilter-scan-direction { v | h | v, h }

   Directions in which to search for solidly black areas. Either ``v``
   (for vertical searching), ``h`` (for horizontal searching) or ``v,h``
   (for both) can be specified. The blackfilter works by moving a
   virtual bar across each page. The darkness inside the virtual bar is
   determined and if it exceeds ``blackfilter-scan-threshold`` black
   pixels in the area are filled. During filling the blackness of each
   pixel is determined by ``black-threshold``. The bar is then moved by
   ``blackfilter-scan-step`` in the scanning direction. Once a page
   border is encountered the bar is moved down (horizontal scan) or
   right (vertical scan) by its ``blackfilter-scan-size``.

.. option:: -bs { size | h-size, v-size } ; --blackfilter-scan-size { size | h-size, v-size }

   Size of virtual bar in direction of scanning (meaning width for
   horizontal pass, height for vertical pass) used for black area
   detection. Two values may be specified to individually set the size
   for the horizontal scanning-pass and the vertical pass. (default:
   ``20,20``)

.. option:: -bd { depth | h-depth, v-depth } ; --blackfilter-scan-depth { depth | h-depth, v-depth }

   Depth of virtual bar in non-scanning direction (meaning height for
   horizontal pass, width for vertical pass) used for black area
   detection. Two values may be specified to individually set the depth
   for the horizontal scanning-pass and the vertical pass. (default:
   ``500,500``)

.. option:: -bp { step | h-step, v-step } ; --blackfilter-scan-step { step | h-step, v-step }

   Steps to move virtual bar for black area detection. Two values may be
   specified to individually set the step for the horizontal
   scanning-pass and the vertical pass. (default: ``5,5``)

.. option:: -bt threshold ; --blackfilter-scan-threshold threshold

   Ratio of dark pixels above which a black area gets detected.
   (default: ``0.95``).

.. option:: -bx left, top, right, bottom ; --blackfilter-scan-exclude left, top, right, bottom

   Area on which the blackfilter should not operate. This can be useful
   to prevent the blackfilter from working on inner page content. May be
   specified multiple times to set more than one area.

.. option:: -bi intensity ; --blackfilter-intensity intensity

   Intensity with which to delete black areas. This deletes pixels
   around the virtual scan bar. Larger values will leave less
   noise-pixels around former black areas, but may delete page content.
   (default: ``20``)

.. option:: -ni intensity ; -noisefilter-intensity intensity

   Intensity with which to delete individual pixels or tiny clusters of
   pixels. Any cluster which only contains intensity dark pixels
   together will be deleted. (default: ``4``)

.. option:: -ls { size | h-size, v-size } ; --blurfilter-size { size | h-size, v-size }

   Size of blurfilter area to search for "lonely" clusters of pixels.
   (default: ``100,100``)

.. option:: -lp { step | h-step, v-step } ; --blurfilter-step { step | h-step, v-step }

   Size of "blurring" steps in each direction. (default: ``50,50``)

.. option:: -li ratio ; --blurfilter-intensity ratio

   Relative intensity with which to delete tiny clusters of pixels. Any
   blurred area which contains at most the ratio of dark pixels will be
   cleared. (default: ``0.01``)

.. option:: -gs { size | h-size, v-size } ; --grayfilter-size { size | h-size, v-size }

   Size of grayfilter mask to search for "gray-only" areas of pixels.
   (default: ``50,50``)

.. option:: -gp { step | h-step, v-step } ; --grayfilter-step { step | h-step, v-step }

   Size of steps moving the grayfilter mask in each direction. (default:
   ``20,20``)

.. option:: -gt ratio ; --grayfilter-threshold ratio

   Relative intensity of grayness which is accepted before clearing the
   grayfilter mask in cases where no black pixel is found in the mask.
   (default: ``0.5``)

.. option:: -p x, y; --mask-scan-point x, y

   Manually set starting point for mask-detection. Multiple
   ``--mask-scan-point`` options may be specified to detect multiple
   masks.

.. option:: -m x1, y1, x2, y2; --mask x1, y1, x2, y2

   Manually add a mask, in addition to masks automatically detected
   around the ``--mask-scan-point`` coordinates (unless
   ``--no-mask-scan`` is specified).

   Any pixel outside a mask will be set to white, unless another mask
   covers this pixel.

.. option:: -mn { v \| h \| v,h }; --mask-scan-direction { v \| h \| v,h }

   Directions in which to search for mask borders, starting from
   --mask-scan-point coordinates. Either ``v`` (for vertical mirroring),
   ``h`` (for horizontal mirroring) or ``v,h`` (for both) can be
   specified. (default: ``h``, as ``v`` may cut text- paragraphs on
   single-page sheets)

.. option:: -ms { size \| h-size, v-size }; --mask-scan-size { size \| h-size, v-size }

   Width of the virtual bar used for mask detection. Two values may be
   specified to individually set horizontal and vertical size. (default:
   ``50,50``)

.. option:: -md { depth \| h-depth, v-depth }; --mask-scan-depth { depth \| h-depth, v-depth }

   Height of the virtual bar used for mask detection. (default:
   ``-1,-1``, using the total width or height of the sheet)

.. option:: -mp { step \| h-step, v-step }; --mask-scan-step { step \| h-step, v-step }

   Steps to move the virtual bar for mask detection. (default: ``5,5``)

.. option:: -mt { threshold \| h-threshold, v-threshold }; --mask-scan-threshold { threshold \| h-threshold, v-threshold }

   Ratio of dark pixels below which an edge gets detected, relative to
   maximum blackness when counting from the start coordinate heading
   towards one edge. (default: ``0.1``)

.. option:: -mm w, h; --mask-scan-minimum w, h

   Minimum allowed size of an auto-detected mask. Masks detected below
   this size will be ignored and set to the size specified by
   mask-scan-maximum. (default: ``100,100``)

.. option:: -mM w, h; --mask-scan-maximum w, h

   Maximum allowed size of an auto-detected mask. Masks detected above
   this size will be shrunk to the maximum value, each direction
   individually. (default: sheet size, or page size derived from
   ``--layout`` option)

.. option:: -mc color; --mask-color color

   Color value with which to wipe out pixels not covered by any mask.
   Maybe useful for testing in order to visualize the effect of masking.
   (Note that an RGB-value is expected: R*65536 + G*256 + B.)

.. option:: -dn { left \| top \| right \| bottom },...; --deskew-scan-direction { left \| top \| right \| bottom },...

   Edges from which to scan for rotation. Each edge of a mask can be
   used to detect the mask's rotation. If multiple edges are specified,
   the average value will be used, unless the statistical deviation
   exceeds ``--deskew-scan-deviation``. Use ``left`` for scanning from
   the left edge, ``top`` for scanning from the top edge, ``right`` for
   scanning from the right edge, ``bottom`` for scanning from the
   bottom. Multiple directions can be separated by commas. (default:
   ``left,right``)

.. option:: -ds pixels; --deskew-scan-size pixels

   Size of virtual line for rotation detection. (default: ``1500``)

.. option:: -dd ratio; --deskew-scan-depth ratio

   Amount of dark pixels to accumulate until scanning is stopped,
   relative to scan-bar size. (default: ``0.5``)

.. option:: -dr degrees; --deskew-scan-range degrees

   Range in which to search for rotation, from -degrees to +degrees
   rotation. (default: ``5.0``)

.. option:: -dp degrees; --deskew-scan-step degrees

   Steps between single rotation-angle detections. Lower numbers lead to
   better results but slow down processing. (default: ``0.1``)

.. option:: -dv deviation; --deskew-scan-deviation deviation

   Maximum statistical deviation allowed among the results from detected
   edges. No rotation if exceeded. (default: ``1.0``)

.. option:: -W left, top, right, bottom; --wipe left, top, right, bottom

   Manually wipe out an area. Any pixel in a wiped area will be set to
   white. Multiple ``--wipe`` areas may be specified. This is applied
   after deskewing and before automatic border-scan.

.. option:: -mw { size \| left, right }; --middle-wipe { size \| left, right }

   If ``--layout`` is set to ``double``, this may specify the size of a
   middle area to wipe out between the two pages on the sheet. This may
   be useful if the blackfilter fails to remove some black areas (e.g.
   resulting from photo-copying in the middle between two pages).

.. option:: -B left, top, right, bottom; --border left, top, right, bottom

   Manually add a border. Any pixel in the border area will be set to
   white. This is applied after deskewing and before automatic
   border-scan.

.. option:: -Bn { v \| h \| v,h }; --border-scan-direction { v \| h \| v,h }

   Directions in which to search for outer border. Either ``v`` (for
   vertical mirroring), ``h`` (for horizontal mirroring) or ``v,h`` (for
   both) can be specified. (default: ``v``)

.. option:: -Bs { size \| h-size, v-size }; --border-scan-size { size \| h-size, v-size }

   Width of virtual bar used for border detection. Two values may be
   specified to individually set horizontal and vertical size. (default:
   ``5,5``)

.. option:: -Bp { step \| h-step, v-step }; --border-scan-step { step \| h-step, v-step }

   Steps to move virtual bar for border detection. (default: ``5,5``)

.. option:: -Bt threshold; --border-scan-threshold threshold

   Absolute number of dark pixels covered by the border-scan mask above
   which a border is detected. (default: ``5``)

.. option:: -Ba { left \| top \| right \| bottom }; --border-align { left \| top \| right \| bottom }

   Direction where to shift the detected border-area. Use
   ``--border-margin`` to specify horizontal and vertical distances to
   be kept from the sheet-edge. (default: ``none``)

.. option:: -Bm vertical, horizontal; --border-margin vertical, horizontal

   Distance to keep from the sheet edge when aligning a border area. May
   use measurement suffices such as cm, in.

.. option:: -w threshold; --white-threshold threshold

   Brightness ratio above which a pixel is considered white. (default:
   ``0.9``)

.. option:: -b threshold; --black-threshold threshold

   Brightness ratio below which a pixel is considered black (non-gray).
   This is used by the gray-filter and the blackfilter. This value is
   also used when converting a grayscale image to black-and-white mode
   (default: ``0.33``)

.. option:: -ip { 1 \| 2 }; --input-pages { 1 \| 2 }

   If ``2`` is specified, read two input images instead of one and
   internally combine them to a doubled-layout sheet before further
   processing. Before internally combining, ``--pre-rotation`` is
   optionally applied individually to both input images as the very
   first processing steps.

.. option:: -op { 1 \| 2 }; --output-pages { 1 \| 2 }

   If ``2`` is specified, write two output images instead of one, as a
   result of splitting a doubled-layout sheet after processing. After
   splitting the sheet, ``--post-rotation`` is optionally applied
   individually to both output images as the very last processing step.

.. option:: -S { width, height \| size-name }; --sheet-size { width, height \| size-name }

   Force a fix sheet size. Usually, the sheet size is determined by the
   input image size (if ``input-pages=1``), or by the double size of the
   first page in a two-page input set (if ``input-pages=2``). If the
   input image is smaller than the size specified here, it will appear
   centered and surrounded with a white border on the sheet. If the
   input image is bigger, it will be centered and the edges will be
   cropped. This option may also be helpful to get regular sized output
   images if the input image sizes differ. Standard size-names like
   ``a4-landscape``, ``letter``, etc. may be used (see ``--size``).
   (default: as in input file)

.. option:: --sheet-background { black \| white }

   Sets a color with which the sheet is filled before any image is
   loaded and placed onto it. This can be useful when the sheet size and
   the image size differ.

.. option:: --no-blackfilter sheet-range

   Disables black area scan. Individual sheet indices can be specified.

.. option:: --no-noisefilter sheet-range

   Disables the noisefilter. Individual sheet indices can be specified.

.. option:: --no-blurfilter sheet-range

   Disables the blurfilter. Individual sheet indices can be specified.

.. option:: --no-grayfilter sheet-range

   Disables the grayfilter. Individual sheet indices can be specified.

.. option:: --no-mask-scan sheet-range

   Disables mask-detection. Masks explicitly set by ``--mask`` will
   still have effect. Individual sheet indices can be specified.

.. option:: --no-mask-center sheet-range

   Disables auto-centering of each mask. Auto-centering is performed by
   default if the ``--layout`` option has been set. Individual sheet
   indices can be specified.

.. option:: --no-deskew sheet-range

   Disables deskewing. Individual sheet indices can be specified.

.. option:: --no-wipe sheet-range

   Disables explicit wipe-areas. This means the effect of parameter
   ``--wipe`` can be disabled individually per sheet.

.. option:: --no-border sheet-range

   Disables explicitly set borders. This means the effect of parameter
   ``--border`` can be disabled individually per sheet.

.. option:: --no-border-scan sheet-range

   Disables border-scanning from the edges of the sheet. Individual
   sheet indices can be specified.

.. option:: --no-border-align sheet-range

   Disables aligning of the area detected by border-scanning (see
   ``--border-align``). Individual sheet indices can be specified.

.. option:: -n sheet-range; --no-processing sheet-range

   Do not perform any processing on a sheet except pre/post rotating and
   mirroring, and file-depth conversions on saving. This option has the
   same effect as setting all ``--no-xxx`` options together. Individual
   sheet indices can be specified.

.. option:: --interpolate { nearest \| linear \| cubic }

   Set the interpolation function used for deskewing and stretching. The
   ``cubic`` option provides the best image quality, while ``nearest``
   is the fastest. (default: ``cubic``)

.. option:: --no-multi-pages

   Disable multi-page processing even if the input filename contains a
   ``%`` (usually indicating the start of a placeholder for the page
   counter).

.. option:: --dpi dpi

   Dots per inch used for conversion of measured size values, like e.g.
   ``21cm,27.9cm``. Mind that this parameter should occur before
   specifying any size value with measurement suffix. (default: ``300``)

.. option:: -t { pbm \| pgm \| ppm }; --type { pbm \| pgm> \| ppm }

   Output file type (and bit depth). If not specified, the one with the
   same, or closest, pixel format as the original input files will be
   used.

   ``pbm``
      Portable Bit Map, monochrome raw image.

   ``pgm``
      Portable Grayscale Map, 8-bit per pixel grayscale raw image.

   ``ppm``
      Portable Pixel Map, 24-bit per pixel RGB raw image.

.. option:: -T ; --test-only

   Do not write any output. May be useful in combination with
   ``--verbose`` to get information about the input.

.. option:: -si nr; --start-input nr

   Set the first page number to substitute for '%d' in input filenames.
   Every time the input file sequence is repeated, this number gets
   increased by 1. (default: (startsheet-1)*inputpages+1)

.. option:: -so nr; --start-output nr

   Set the first page number to substitute for '%d' in output filenames.
   Every time the output file sequence is repeated, this number gets
   increased by 1. (default: (startsheet-1)*outputpages+1)

.. option:: --insert-blank nr [,nr...]

   Use blank input instead of an input file from the input file sequence
   at the specified index-positions. The input file sequence will be
   interrupted temporarily and will continue with the next input file
   afterwards. This can be useful to insert blank content into a
   sequence of input images.

.. option:: --replace-blank nr [,nr...]

   Like ``--insert-blank``, but the input images at the specified index
   positions get replaced with blank content and thus will be ignored.

.. option:: --overwrite

   Allow overwriting existing files. Otherwise the program terminates
   with an error if an output file to be written already exists.

.. option:: -q ; --quiet

   Quiet mode, no output at all.

.. option:: -v ; --verbose

   Verbose output, more info messages.

.. option:: -vv

   Even more verbose output, show parameter settings before processing.

.. option:: -V ; --version

   Output version and build information.
