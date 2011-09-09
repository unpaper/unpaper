/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */

/* --- The main program  -------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "unpaper.h"
#include "imageprocess.h"
#include "tools.h"
#include "file.h"
#include "parse.h"
 
#define WELCOME                                                         \
    "unpaper " VERSION " - written by Jens Gulden 2005-2007.\n"         \
    "Licensed under the GNU General Public License, this comes with no warranty.\n"
              
#define USAGE                                                           \
    WELCOME "\n"                                                        \
    "Usage: unpaper [options] <input-file(s)> <output-file(s)>\n\n"     \
    "Filenames may contain a formatting placeholder starting with '%%' to insert a\n" \
    "page counter for multi-page processing. E.g.: 'scan%%03d.pbm' to process files\n" \
    "scan001.pbm, scan002.pbm, scan003.pbm etc.\n\n"                    \
    "See 'man unpaper' for options details"

//-vvv --debug                        Undocumented.
//-vvvv --debug-save                  Undocumented.


#define HELP "Run 'man unpaper' for usage information.\n"


/* --- constants ---------------------------------------------------------- */

// file type names (see typedef FILETYPES)
static const char FILETYPE_NAMES[FILETYPES_COUNT][4] = {
    "pbm",
    "pgm",
    "ppm"
};



/****************************************************************************
 * MAIN()                                                                   *
 ****************************************************************************/

/**
 * The main program.
 */
int main(int argc, char* argv[]) {

    // --- parameter variables ---
    int layout;
    int startSheet;
    int endSheet;
    int startInput;
    int startOutput;
    int inputCount;
    int outputCount;
    char* inputFileSequence[MAX_FILES];
    int inputFileSequenceCount;
    char* outputFileSequence[MAX_FILES];
    int outputFileSequenceCount;
    int sheetSize[DIMENSIONS_COUNT];
    int sheetBackground;
    int preRotate;
    int postRotate;
    int preMirror;
    int postMirror;
    int preShift[DIRECTIONS_COUNT];
    int postShift[DIRECTIONS_COUNT];
    int size[DIRECTIONS_COUNT];
    int postSize[DIRECTIONS_COUNT];
    int stretchSize[DIRECTIONS_COUNT];
    int postStretchSize[DIRECTIONS_COUNT];
    float zoomFactor;
    float postZoomFactor;
    int pointCount;
    int point[MAX_POINTS][COORDINATES_COUNT];
    int maskCount;
    int mask[MAX_MASKS][EDGES_COUNT];
    int wipeCount;
    int wipe[MAX_MASKS][EDGES_COUNT];
    int middleWipe[2];
    int preWipeCount;
    int preWipe[MAX_MASKS][EDGES_COUNT];
    int postWipeCount;
    int postWipe[MAX_MASKS][EDGES_COUNT];
    int preBorder[EDGES_COUNT];
    int postBorder[EDGES_COUNT];
    int border[EDGES_COUNT];
    bool maskValid[MAX_MASKS];
    int preMaskCount;
    int preMask[MAX_MASKS][EDGES_COUNT];
    int blackfilterScanDirections;
    int blackfilterScanSize[DIRECTIONS_COUNT];
    int blackfilterScanDepth[DIRECTIONS_COUNT];
    int blackfilterScanStep[DIRECTIONS_COUNT];
    float blackfilterScanThreshold;    
    int blackfilterExcludeCount;
    int blackfilterExclude[MAX_MASKS][EDGES_COUNT];
    int blackfilterIntensity;
    int noisefilterIntensity;
    int blurfilterScanSize[DIRECTIONS_COUNT];
    int blurfilterScanStep[DIRECTIONS_COUNT];
    float blurfilterIntensity;
    int grayfilterScanSize[DIRECTIONS_COUNT];
    int grayfilterScanStep[DIRECTIONS_COUNT];
    float grayfilterThreshold;
    int maskScanDirections;
    int maskScanSize[DIRECTIONS_COUNT];
    int maskScanDepth[DIRECTIONS_COUNT];
    int maskScanStep[DIRECTIONS_COUNT];
    float maskScanThreshold[DIRECTIONS_COUNT];
    int maskScanMinimum[DIMENSIONS_COUNT];
    int maskScanMaximum[DIMENSIONS_COUNT];
    int maskColor;
    int deskewScanEdges;
    int deskewScanSize;
    float deskewScanDepth;
    float deskewScanRange;
    float deskewScanStep;
    float deskewScanDeviation;
    int borderScanDirections;
    int borderScanSize[DIRECTIONS_COUNT];
    int borderScanStep[DIRECTIONS_COUNT];
    int borderScanThreshold[DIRECTIONS_COUNT];
    int borderAlign;
    int borderAlignMargin[DIRECTIONS_COUNT];
    int outsideBorderscanMask[MAX_PAGES][EDGES_COUNT]; // set by --layout
    int outsideBorderscanMaskCount;
    float whiteThreshold;
    float blackThreshold;
    bool writeoutput;
    bool qpixels;
    bool multisheets;
    const char* outputTypeName; 
    int noBlackfilterMultiIndex[MAX_MULTI_INDEX];
    int noBlackfilterMultiIndexCount;
    int noNoisefilterMultiIndex[MAX_MULTI_INDEX];
    int noNoisefilterMultiIndexCount;
    int noBlurfilterMultiIndex[MAX_MULTI_INDEX];
    int noBlurfilterMultiIndexCount;
    int noGrayfilterMultiIndex[MAX_MULTI_INDEX];
    int noGrayfilterMultiIndexCount;
    int noMaskScanMultiIndex[MAX_MULTI_INDEX];
    int noMaskScanMultiIndexCount;
    int noMaskCenterMultiIndex[MAX_MULTI_INDEX];
    int noMaskCenterMultiIndexCount;
    int noDeskewMultiIndex[MAX_MULTI_INDEX];
    int noDeskewMultiIndexCount;
    int noWipeMultiIndex[MAX_MULTI_INDEX];
    int noWipeMultiIndexCount;
    int noBorderMultiIndex[MAX_MULTI_INDEX];
    int noBorderMultiIndexCount;
    int noBorderScanMultiIndex[MAX_MULTI_INDEX];
    int noBorderScanMultiIndexCount;
    int noBorderAlignMultiIndex[MAX_MULTI_INDEX];
    int noBorderAlignMultiIndexCount;
    int sheetMultiIndex[MAX_MULTI_INDEX];
    int sheetMultiIndexCount;    
    int excludeMultiIndex[MAX_MULTI_INDEX];
    int excludeMultiIndexCount;
    int ignoreMultiIndex[MAX_MULTI_INDEX];
    int ignoreMultiIndexCount;    
    int autoborder[MAX_MASKS][EDGES_COUNT];
    int autoborderMask[MAX_MASKS][EDGES_COUNT];
    int insertBlank[MAX_MULTI_INDEX];
    int insertBlankCount;    
    int replaceBlank[MAX_MULTI_INDEX];
    int replaceBlankCount;    
    bool overwrite;
    bool showTime;
    int dpi;
    
    // --- local variables ---
    int x;
    int y;
    int w;
    int h;
    int left;
    int top;
    int right;
    int bottom;
    int i;
    int j;
    int previousWidth;
    int previousHeight;
    int previousBitdepth;
    bool previousColor;
    int inputFileSequencePos; // index 'looping' through input-file-seq (without additionally inserted blank images)
    int outputFileSequencePos; // index 'looping' through output-file-seq
    int inputFileSequencePosTotal; // index 'looping' through input-file-seq (with additional blank images)
    char inputFilenamesResolvedBuffer[MAX_PAGES][255]; // count: inputCount
    char outputFilenamesResolvedBuffer[MAX_PAGES][255]; // count: outputCount;
    char* inputFilenamesResolved[MAX_PAGES];
    char* outputFilenamesResolved[MAX_PAGES];
    char s1[1023]; // buffers for result of implode()
    char s2[1023];
    char debugFilename[100];
    struct IMAGE sheet;
    struct IMAGE sheetBackup;
    struct IMAGE originalSheet;
    struct IMAGE qpixelSheet;
    struct IMAGE page;
    const char* layoutStr;
    const char* inputTypeName; 
    const char* inputTypeNames[MAX_PAGES];
    int inputType;
    int filterResult;
    double rotation;
    int q;
    struct IMAGE rect;
    struct IMAGE rectTarget;
    int outputType;
    int outputDepth;
    int bd;
    bool col;
    bool success;
    bool done;
    bool anyWildcards;
    bool allInputFilesMissing;
    int nr;
    int inputNr;
    int outputNr;
    bool first;
    clock_t startTime;
    clock_t endTime;
    clock_t time;
    unsigned long int totalTime;
    int totalCount;
    bool ins;
    bool repl;
    int blankCount;
    int exitCode;

    sheet.buffer = NULL;
    page.buffer = NULL;
    exitCode = 0; // error code to return
    bd = 1; // default bitdepth if not resolvable (i.e. usually empty input, so bd=1 is good choice)
    col = false; // default no color if not resolvable
    
    // explicitly un-initialize variables that are sometimes not used to avoid compiler warnings
    qpixelSheet.buffer = NULL; // used optionally, deactivated by --no-qpixels
    startTime = 0;             // used optionally in debug mode -vv or with --time
    endTime = 0;               // used optionally in debug mode -vv or with --time
    inputNr = -1;              // will be initialized in first run of main-loop
    outputNr = -1;             // will be initialized in first run of main-loop


    // -----------------------------------------------------------------------    
    // --- process all sheets                                              ---
    // -----------------------------------------------------------------------    
    
    // count from start sheet to end sheet
    startSheet = 1; // defaults, may be changed in first run of for-loop
    endSheet = -1;
    startInput = -1;
    startOutput = -1;
    totalTime = 0;
    totalCount = 0;
    inputFileSequencePos = 0;
    outputFileSequencePos = 0;
    inputFileSequencePosTotal = 0;
    previousWidth = previousHeight = previousBitdepth = -1;
    previousColor = false;
    first = true;
    
    for (nr = startSheet; (endSheet == -1) || (nr <= endSheet); nr++) {

        // --- default values ---
        w = h = -1;
        layout = LAYOUT_SINGLE;
        layoutStr = "single";
        preRotate = 0;
        postRotate = 0;
        preMirror = 0;
        postMirror = 0;
        preShift[WIDTH] = preShift[HEIGHT] = 0;
        postShift[WIDTH] = postShift[HEIGHT] = 0;
        size[WIDTH] = size[HEIGHT] = -1;
        postSize[WIDTH] = postSize[HEIGHT] = -1;
        stretchSize[WIDTH] = stretchSize[HEIGHT] = -1;
        postStretchSize[WIDTH] = postStretchSize[HEIGHT] = -1;
        zoomFactor = 1.0;
        postZoomFactor = 1.0;
        outputTypeName = NULL; // default derived from input
        outputDepth = -1; // default derived from input
        pointCount = 0;
        maskCount = 0;
        preMaskCount = 0;
        wipeCount = 0;
        preWipeCount = 0;
        postWipeCount = 0;
        middleWipe[0] = middleWipe[1] = 0; // left/right
        border[LEFT] = border[TOP] = border[RIGHT] = border[BOTTOM] = 0;
        preBorder[LEFT] = preBorder[TOP] = preBorder[RIGHT] = preBorder[BOTTOM] = 0;
        postBorder[LEFT] = postBorder[TOP] = postBorder[RIGHT] = postBorder[BOTTOM] = 0;
        blackfilterScanDirections = (1<<HORIZONTAL) | (1<<VERTICAL);
        blackfilterScanSize[HORIZONTAL] = blackfilterScanSize[VERTICAL] = 20;
        blackfilterScanDepth[HORIZONTAL] = blackfilterScanDepth[VERTICAL] = 500;
        blackfilterScanStep[HORIZONTAL] = blackfilterScanStep[VERTICAL] = 5;
        blackfilterScanThreshold = 0.95;
        blackfilterExcludeCount = 0;
        blackfilterIntensity = 20;
        noisefilterIntensity = 4;
        blurfilterScanSize[HORIZONTAL] = blurfilterScanSize[VERTICAL] = 100;
        blurfilterScanStep[HORIZONTAL] = blurfilterScanStep[VERTICAL] = 50;
        blurfilterIntensity = 0.01;
        grayfilterScanSize[HORIZONTAL] = grayfilterScanSize[VERTICAL] = 50;
        grayfilterScanStep[HORIZONTAL] = grayfilterScanStep[VERTICAL] = 20;
        grayfilterThreshold = 0.5;
        maskScanDirections = (1<<HORIZONTAL);
        maskScanSize[HORIZONTAL] = maskScanSize[VERTICAL] = 50;
        maskScanDepth[HORIZONTAL] = maskScanDepth[VERTICAL] = -1;
        maskScanStep[HORIZONTAL] = maskScanStep[VERTICAL] = 5;
        maskScanThreshold[HORIZONTAL] = maskScanThreshold[VERTICAL] = 0.1;
        maskScanMinimum[WIDTH] = maskScanMinimum[HEIGHT] = 100;
        maskScanMaximum[WIDTH] = maskScanMaximum[HEIGHT] = -1; // set default later
        maskColor = pixelValue(WHITE, WHITE, WHITE);
        deskewScanEdges = (1<<LEFT) | (1<<RIGHT);
        deskewScanSize = 1500;
        deskewScanDepth = 0.5;
        deskewScanRange = 5.0;
        deskewScanStep = 0.1;
        deskewScanDeviation = 1.0;
        borderScanDirections = (1<<VERTICAL);
        borderScanSize[HORIZONTAL] = borderScanSize[VERTICAL] = 5;
        borderScanStep[HORIZONTAL] = borderScanStep[VERTICAL] = 5;
        borderScanThreshold[HORIZONTAL] = borderScanThreshold[VERTICAL] = 5;
        borderAlign = 0; // center
        borderAlignMargin[HORIZONTAL] = borderAlignMargin[VERTICAL] = 0; // center
        outsideBorderscanMaskCount = 0;
        whiteThreshold = 0.9;
        blackThreshold = 0.33;
        sheetSize[WIDTH] = sheetSize[HEIGHT] = -1;
        sheetBackground = WHITE;
        writeoutput = true;
        qpixels = true;
        multisheets = true;
        inputCount = 1;
        outputCount = 1;
        inputFileSequenceCount = 0;
        outputFileSequenceCount = 0;
        verbose = VERBOSE_NONE;
        noBlackfilterMultiIndexCount = 0; // 0: allow all, -1: disable all, n: individual entries
        noNoisefilterMultiIndexCount = 0;
        noBlurfilterMultiIndexCount = 0;
        noGrayfilterMultiIndexCount = 0;
        noMaskScanMultiIndexCount = 0;
        noMaskCenterMultiIndexCount = 0;
        noDeskewMultiIndexCount = 0;
        noWipeMultiIndexCount = 0;
        noBorderMultiIndexCount = 0;
        noBorderScanMultiIndexCount = 0;
        noBorderAlignMultiIndexCount = 0;
        sheetMultiIndexCount = -1; // default: process all between start-sheet and end-sheet
        excludeMultiIndexCount = 0;
        ignoreMultiIndexCount = 0;
        insertBlankCount = 0;
        replaceBlankCount = 0;
        overwrite = false;
        showTime = false;
        dpi = 300;


        // -------------------------------------------------------------------
        // --- parse parameters                                            ---
        // -------------------------------------------------------------------
        
        i = 1;
        while ((argc==0) || ((i < argc) && (argv[i][0]=='-'))) {

            // --help
            if (argc==0 || strcmp(argv[i], "--help")==0 || strcmp(argv[i], "-h")==0 || strcmp(argv[i], "-?")==0 || strcmp(argv[i], "/?")==0 || strcmp(argv[i], "?")==0) {
                puts(USAGE);
                return 0;

            // --version -V
            } else if (strcmp(argv[i], "-V")==0 || strcmp(argv[i], "--version")==0) {
                puts(VERSION);
                return 0;

            // --layout  -l
            } else if (strcmp(argv[i], "-l")==0 || strcmp(argv[i], "--layout")==0) {
                i++;
                //noMaskCenterMultiIndexCount = 0; // enable mask centering
                if (strcmp(argv[i], "single")==0) {
                    layout = LAYOUT_SINGLE;
                } else if (strcmp(argv[i], "double")==0) {
                    layout = LAYOUT_DOUBLE;
                } else if (strcmp(argv[i], "none")==0) {
                    layout = LAYOUT_NONE;
                } else {
                    printf("*** error: Unknown layout mode '%s'.", argv[i]);
                    exitCode = 1;
                }

            // --sheet -#
            } else if ((strcmp(argv[i], "-#")==0)||(strcmp(argv[i], "--sheet")==0)) {
                parseMultiIndex(&i, argv, sheetMultiIndex, &sheetMultiIndexCount);
                if (sheetMultiIndexCount > 0) {
                    if (startSheet > sheetMultiIndex[0]) { // allow 0 as start sheet, might be overwritten by --start-sheet again
                        startSheet = sheetMultiIndex[0];
                    }
                }

            // --start-sheet
            } else if ((strcmp(argv[i], "-start")==0)||(strcmp(argv[i], "--start-sheet")==0)) {
                sscanf(argv[++i],"%d", &startSheet);
                if (nr < startSheet) {
                    nr = startSheet;
                }

            // --end-sheet
            } else if ((strcmp(argv[i], "-end")==0)||(strcmp(argv[i], "--end-sheet")==0)) {
                sscanf(argv[++i],"%d", &endSheet);

            // --start-input
            } else if ((strcmp(argv[i], "-si")==0)||(strcmp(argv[i], "--start-input")==0)) {
                sscanf(argv[++i],"%d", &startInput);

            // --start-output
            } else if ((strcmp(argv[i], "-so")==0)||(strcmp(argv[i], "--start-output")==0)) {
                sscanf(argv[++i],"%d", &startOutput);

            // --sheet-size
            } else if ((strcmp(argv[i], "-S")==0)||(strcmp(argv[i], "--sheet-size")==0)) {
                parseSize(argv[++i], sheetSize, dpi, &exitCode);

            // --sheet-background
            } else if (strcmp(argv[i], "--sheet-background")==0) {
                sheetBackground = parseColor(argv[++i], &exitCode);

            // --exclude  -x
            } else if (strcmp(argv[i], "-x")==0 || strcmp(argv[i], "--exclude")==0) {
                parseMultiIndex(&i, argv, excludeMultiIndex, &excludeMultiIndexCount);
                if (excludeMultiIndexCount == -1) {
                    excludeMultiIndexCount = 0; // 'exclude all' makes no sence
                }

            // --no-processing  -n
            } else if (strcmp(argv[i], "-n")==0 || strcmp(argv[i], "--no-processing")==0) {
                parseMultiIndex(&i, argv, ignoreMultiIndex, &ignoreMultiIndexCount);



            // --pre-rotate
            } else if (strcmp(argv[i], "--pre-rotate")==0) {
                sscanf(argv[++i],"%d", &preRotate);
                if ((preRotate != 0) && (abs(preRotate) != 90)) {
                    printf("Cannot set --pre-rotate value other than -90 or 90, ignoring.\n");
                    preRotate = 0;
                }

            // --post-rotate
            } else if (strcmp(argv[i], "--post-rotate")==0) {
                sscanf(argv[++i],"%d", &postRotate);
                if ((postRotate != 0) && (abs(postRotate) != 90)) {
                    printf("Cannot set --post-rotate value other than -90 or 90, ignoring.\n");
                    postRotate = 0;
                }

            // --pre-mirror  -M
            } else if (strcmp(argv[i], "-M")==0 || strcmp(argv[i], "--pre-mirror")==0) {
                preMirror = parseDirections(argv[++i], &exitCode); // s = "v", "v,h", "vertical,horizontal", ...

            // --post-mirror
            } else if (strcmp(argv[i], "--post-mirror")==0) {
                postMirror = parseDirections(argv[++i], &exitCode);


            // --pre-shift
            } else if (strcmp(argv[i], "--pre-shift")==0) {
                parseSize(argv[++i], preShift, dpi, &exitCode);

            // --post-shift
            } else if (strcmp(argv[i], "--post-shift")==0) {
                parseSize(argv[++i], postShift, dpi, &exitCode);


            // --pre-mask
            } else if ( strcmp(argv[i], "--pre-mask")==0 && (preMaskCount<MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                preMask[preMaskCount][LEFT] = left;
                preMask[preMaskCount][TOP] = top;
                preMask[preMaskCount][RIGHT] = right;
                preMask[preMaskCount][BOTTOM] = bottom;
                preMaskCount++;


            // -s --size
            } else if ((strcmp(argv[i], "-s")==0)||(strcmp(argv[i], "--size")==0)) {
                parseSize(argv[++i], size, dpi, &exitCode);

            // --post-size
            } else if (strcmp(argv[i], "--post-size")==0) {
                parseSize(argv[++i], postSize, dpi, &exitCode);

            // --stretch
            } else if (strcmp(argv[i], "--stretch")==0) {
                parseSize(argv[++i], stretchSize, dpi, &exitCode);

            // --post-stretch
            } else if (strcmp(argv[i], "--post-stretch")==0) {
                parseSize(argv[++i], postStretchSize, dpi, &exitCode);

            // -z --zoom
            } else if (strcmp(argv[i], "--zoom")==0) {
                sscanf(argv[++i],"%f", &zoomFactor);

            // --post-zoom
            } else if (strcmp(argv[i], "--post-zoom")==0) {
                sscanf(argv[++i],"%f", &postZoomFactor);


            // --mask-scan-point  -p
            } else if ((strcmp(argv[i], "-p")==0 || strcmp(argv[i], "--mask-scan-point")==0) && (pointCount < MAX_POINTS)) {
                x = -1;
                y = -1;
                sscanf(argv[++i],"%d,%d", &x, &y);
                point[pointCount][X] = x;
                point[pointCount][Y] = y;
                pointCount++;


            // --mask  -m    
            } else if ((strcmp(argv[i], "-m")==0 || strcmp(argv[i], "--mask")==0) && (maskCount<MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                mask[maskCount][LEFT] = left;
                mask[maskCount][TOP] = top;
                mask[maskCount][RIGHT] = right;
                mask[maskCount][BOTTOM] = bottom;
                maskValid[maskCount] = true;
                maskCount++;


            // --wipe  -W    
            } else if ((strcmp(argv[i], "-W")==0 || strcmp(argv[i], "--wipe")==0) && (wipeCount < MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                wipe[wipeCount][LEFT] = left;
                wipe[wipeCount][TOP] = top;
                wipe[wipeCount][RIGHT] = right;
                wipe[wipeCount][BOTTOM] = bottom;
                wipeCount++;

            // ---pre-wipe
            } else if ((strcmp(argv[i], "--pre-wipe")==0) && (preWipeCount < MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                preWipe[preWipeCount][LEFT] = left;
                preWipe[preWipeCount][TOP] = top;
                preWipe[preWipeCount][RIGHT] = right;
                preWipe[preWipeCount][BOTTOM] = bottom;
                preWipeCount++;

            // ---post-wipe
            } else if ((strcmp(argv[i], "--post-wipe")==0) && (postWipeCount < MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                postWipe[postWipeCount][LEFT] = left;
                postWipe[postWipeCount][TOP] = top;
                postWipe[postWipeCount][RIGHT] = right;
                postWipe[postWipeCount][BOTTOM] = bottom;
                postWipeCount++;

            // --middle-wipe -mw
            } else if (strcmp(argv[i], "-mw")==0 || strcmp(argv[i], "--middle-wipe")==0) {
                parseInts(argv[++i], middleWipe);


            // --border  -B
            } else if ((strcmp(argv[i], "-B")==0 || strcmp(argv[i], "--border")==0)) {
                sscanf(argv[++i],"%d,%d,%d,%d", &border[LEFT], &border[TOP], &border[RIGHT], &border[BOTTOM]);

            // --pre-border
            } else if (strcmp(argv[i], "--pre-border")==0) {
                sscanf(argv[++i],"%d,%d,%d,%d", &preBorder[LEFT], &preBorder[TOP], &preBorder[RIGHT], &preBorder[BOTTOM]);

            // --post-border
            } else if (strcmp(argv[i], "--post-border")==0) {
                sscanf(argv[++i],"%d,%d,%d,%d", &postBorder[LEFT], &postBorder[TOP], &postBorder[RIGHT], &postBorder[BOTTOM]);


            // --no-blackfilter
            } else if (strcmp(argv[i], "--no-blackfilter")==0) {
                parseMultiIndex(&i, argv, noBlackfilterMultiIndex, &noBlackfilterMultiIndexCount);

            // --blackfilter-scan-direction  -bn
            } else if (strcmp(argv[i], "-bn")==0 || strcmp(argv[i], "--blackfilter-scan-direction")==0) {
                blackfilterScanDirections = parseDirections(argv[++i], &exitCode);

            // --blackfilter-scan-size  -bs
            } else if (strcmp(argv[i], "-bs")==0 || strcmp(argv[i], "--blackfilter-scan-size")==0) {
                parseInts(argv[++i], blackfilterScanSize);

            // --blackfilter-scan-depth  -bd
            } else if (strcmp(argv[i], "-bd")==0 || strcmp(argv[i], "--blackfilter-scan-depth")==0) {
                parseInts(argv[++i], blackfilterScanDepth);

            // --blackfilter-scan-step  -bp
            } else if (strcmp(argv[i], "-bp")==0 || strcmp(argv[i], "--blackfilter-scan-step")==0) {
                parseInts(argv[++i], blackfilterScanStep);

            // --blackfilter-scan-threshold  -bt   
            } else if (strcmp(argv[i], "-bt")==0 || strcmp(argv[i], "--blackfilter-scan-threshold")==0) {
                sscanf(argv[++i], "%f", &blackfilterScanThreshold);

            // --blackfilter-scan-exclude  -bx
            } else if ((strcmp(argv[i], "-bx")==0 || strcmp(argv[i], "--blackfilter-scan-exclude")==0) && (blackfilterExcludeCount < MAX_MASKS)) {
                left = -1;
                top = -1;
                right = -1;
                bottom = -1;
                sscanf(argv[++i],"%d,%d,%d,%d", &left, &top, &right, &bottom); // x1, y1, x2, y2
                blackfilterExclude[blackfilterExcludeCount][LEFT] = left;
                blackfilterExclude[blackfilterExcludeCount][TOP] = top;
                blackfilterExclude[blackfilterExcludeCount][RIGHT] = right;
                blackfilterExclude[blackfilterExcludeCount][BOTTOM] = bottom;
                blackfilterExcludeCount++;

            // --blackfilter-intensity  -bi
            } else if (strcmp(argv[i], "-bi")==0 || strcmp(argv[i], "--blackfilter-intensity")==0) {
                sscanf(argv[++i], "%d", &blackfilterIntensity);


            // --no-noisefilter
            } else if (strcmp(argv[i], "--no-noisefilter")==0) {
                parseMultiIndex(&i, argv, noNoisefilterMultiIndex, &noNoisefilterMultiIndexCount);

            // --noisefilter-intensity  -ni 
            } else if (strcmp(argv[i], "-ni")==0 || strcmp(argv[i], "--noisefilter-intensity")==0) {
                sscanf(argv[++i], "%d", &noisefilterIntensity);


            // --no-blurfilter
            } else if (strcmp(argv[i], "--no-blurfilter")==0) {
                parseMultiIndex(&i, argv, noBlurfilterMultiIndex, &noBlurfilterMultiIndexCount);

            // --blurfilter-size  -ls
            } else if (strcmp(argv[i], "-ls")==0 || strcmp(argv[i], "--blurfilter-size")==0) {
                parseInts(argv[++i], blurfilterScanSize);

            // --blurfilter-step  -lp
            } else if (strcmp(argv[i], "-lp")==0 || strcmp(argv[i], "--blurfilter-step")==0) {
                parseInts(argv[++i], blurfilterScanStep);

            // --blurfilter-intensity  -li 
            } else if (strcmp(argv[i], "-li")==0 || strcmp(argv[i], "--blurfilter-intensity")==0) {
                sscanf(argv[++i], "%f", &blurfilterIntensity);


            // --no-grayfilter
            } else if (strcmp(argv[i], "--no-grayfilter")==0) {
                parseMultiIndex(&i, argv, noGrayfilterMultiIndex, &noGrayfilterMultiIndexCount);

            // --grayfilter-size  -gs
            } else if (strcmp(argv[i], "-gs")==0 || strcmp(argv[i], "--grayfilter-size")==0) {
                parseInts(argv[++i], grayfilterScanSize);

            // --grayfilter-step  -gp
            } else if (strcmp(argv[i], "-gp")==0 || strcmp(argv[i], "--grayfilter-step")==0) {
                parseInts(argv[++i], grayfilterScanStep);

            // --grayfilter-threshold  -gt 
            } else if (strcmp(argv[i], "-gt")==0 || strcmp(argv[i], "--grayfilter-threshold")==0) {
                sscanf(argv[++i], "%f", &grayfilterThreshold);


            // --no-mask-scan
            } else if (strcmp(argv[i], "--no-mask-scan")==0) {
                parseMultiIndex(&i, argv, noMaskScanMultiIndex, &noMaskScanMultiIndexCount);

            // --mask-scan-direction  -mn
            } else if (strcmp(argv[i], "-mn")==0 || strcmp(argv[i], "--mask-scan-direction")==0) {
                maskScanDirections = parseDirections(argv[++i], &exitCode);

            // --mask-scan-size  -ms
            } else if (strcmp(argv[i], "-ms")==0 || strcmp(argv[i], "--mask-scan-size")==0) {
                parseInts(argv[++i], maskScanSize);

            // --mask-scan-depth  -md
            } else if (strcmp(argv[i], "-md")==0 || strcmp(argv[i], "--mask-scan-depth")==0) {
                parseInts(argv[++i], maskScanDepth);

            // --mask-scan-step  -mp
            } else if (strcmp(argv[i], "-mp")==0 || strcmp(argv[i], "--mask-scan-step")==0) {
                parseInts(argv[++i], maskScanStep);

            // --mask-scan-threshold  -mt   
            } else if (strcmp(argv[i], "-mt")==0 || strcmp(argv[i], "--mask-scan-threshold")==0) {
                parseFloats(argv[++i], maskScanThreshold);

            // --mask-scan-minimum  -mm
            } else if (strcmp(argv[i], "-mm")==0 || strcmp(argv[i], "--mask-scan-minimum")==0) {
                sscanf(argv[++i],"%d,%d", &maskScanMinimum[WIDTH], &maskScanMinimum[HEIGHT]);

            // --mask-scan-maximum  -mM
            } else if (strcmp(argv[i], "-mM")==0 || strcmp(argv[i], "--mask-scan-maximum")==0) {
                sscanf(argv[++i],"%d,%d", &maskScanMaximum[WIDTH], &maskScanMaximum[HEIGHT]);

            // --mask-color
            } else if (strcmp(argv[i], "-mc")==0 || strcmp(argv[i], "--mask-color")==0) {
                sscanf(argv[++i],"%d", &maskColor);


            // --no-mask-center
            } else if (strcmp(argv[i], "--no-mask-center")==0) {
                parseMultiIndex(&i, argv, noMaskCenterMultiIndex, &noMaskCenterMultiIndexCount);


            // --no-deskew
            } else if (strcmp(argv[i], "--no-deskew")==0) {
                parseMultiIndex(&i, argv, noDeskewMultiIndex, &noDeskewMultiIndexCount);

            // --deskew-scan-direction  -dn
            } else if (strcmp(argv[i], "-dn")==0 || strcmp(argv[i], "--deskew-scan-direction")==0) {
                deskewScanEdges = parseEdges(argv[++i], &exitCode);

            // --deskew-scan-size  -ds
            } else if (strcmp(argv[i], "-ds")==0 || strcmp(argv[i], "--deskew-scan-size")==0) {
                sscanf(argv[++i],"%d", &deskewScanSize);

            // --deskew-scan-depth  -dd
            } else if (strcmp(argv[i], "-dd")==0 || strcmp(argv[i], "--deskew-scan-depth")==0) {
                sscanf(argv[++i],"%f", &deskewScanDepth);

            // --deskew-scan-range  -dr
            } else if (strcmp(argv[i], "-dr")==0 || strcmp(argv[i], "--deskew-scan-range")==0) {
                sscanf(argv[++i],"%f", &deskewScanRange);

            // --deskew-scan-step  -dp
            } else if (strcmp(argv[i], "-dp")==0 || strcmp(argv[i], "--deskew-scan-step")==0) {
                sscanf(argv[++i],"%f", &deskewScanStep);

            // --deskew-scan-deviation  -dv
            } else if (strcmp(argv[i], "-dv")==0 || strcmp(argv[i], "--deskew-scan-deviation")==0) {
                sscanf(argv[++i],"%f", &deskewScanDeviation);

            // --no-border-scan
            } else if (strcmp(argv[i], "--no-border-scan")==0) {
                parseMultiIndex(&i, argv, noBorderScanMultiIndex, &noBorderScanMultiIndexCount);

            // --border-scan-direction  -Bn
            } else if (strcmp(argv[i], "-Bn")==0 || strcmp(argv[i], "--border-scan-direction")==0) {
                borderScanDirections = parseDirections(argv[++i], &exitCode);

            // --border-scan-size  -Bs
            } else if (strcmp(argv[i], "-Bs")==0 || strcmp(argv[i], "--border-scan-size")==0) {
                parseInts(argv[++i], borderScanSize);

            // --border-scan-step  -Bp
            } else if (strcmp(argv[i], "-Bp")==0 || strcmp(argv[i], "--border-scan-step")==0) {
                parseInts(argv[++i], borderScanStep);

            // --border-scan-threshold  -Bt   
            } else if (strcmp(argv[i], "-Bt")==0 || strcmp(argv[i], "--border-scan-threshold")==0) {
                parseInts(argv[++i], borderScanThreshold);


            // --border-align  -Ba
            } else if (strcmp(argv[i], "-Ba")==0 || strcmp(argv[i], "--border-align")==0) {
                borderAlign = parseEdges(argv[++i], &exitCode);

            // --border-margin  -Bm
            } else if (strcmp(argv[i], "-Bm")==0 || strcmp(argv[i], "--border-margin")==0) {
                parseSize(argv[++i], borderAlignMargin, dpi, &exitCode);

            // --no-border-align
            } else if (strcmp(argv[i], "--no-border-align")==0) {
                parseMultiIndex(&i, argv, noBorderAlignMultiIndex, &noBorderAlignMultiIndexCount);

            // --no-wipe
            } else if (strcmp(argv[i], "--no-wipe")==0) {
                parseMultiIndex(&i, argv, noWipeMultiIndex, &noWipeMultiIndexCount);

            // --no-border
            } else if (strcmp(argv[i], "--no-border")==0) {
                parseMultiIndex(&i, argv, noBorderMultiIndex, &noBorderMultiIndexCount);


            // --white-treshold
            } else if (strcmp(argv[i], "-w")==0 || strcmp(argv[i], "--white-threshold")==0) {
                sscanf(argv[++i],"%f", &whiteThreshold);
                
            // --black-treshold
            } else if (strcmp(argv[i], "-b")==0 || strcmp(argv[i], "--black-threshold")==0) {
                sscanf(argv[++i],"%f", &blackThreshold);


            // --input-pages
            } else if (strcmp(argv[i], "-ip")==0 || strcmp(argv[i], "--input-pages")==0) {
                sscanf(argv[++i],"%d", &inputCount);
                if ( ! (inputCount >= 1 && inputCount <= 2 ) ) {
                    printf("Cannot set --input-pages value other than 1 or 2, ignoring.\n");
                    inputCount = 1;
                }

            // --output-pages
            } else if (strcmp(argv[i], "-op")==0 || strcmp(argv[i], "--output-pages")==0) {
                sscanf(argv[++i],"%d", &outputCount);
                if ( ! (outputCount >= 1 && outputCount <= 2 ) ) {
                    printf("Cannot set --output-pages value other than 1 or 2, ignoring.\n");
                    outputCount = 1;
                }


            // --input-file-sequence
            } else if (strcmp(argv[i], "-if")==0 || strcmp(argv[i], "--input-file-sequence")==0) {
                inputFileSequenceCount = 0;
                i++;
                done = false;
                while ( (i < argc) && (!done) ) {
                    inputFileSequence[inputFileSequenceCount] = argv[i];
                    if (inputFileSequence[inputFileSequenceCount][0] == '-') { // is next option
                        done = true;
                        i--;
                    } else { // continue collecting filenames
                        i++;
                        inputFileSequenceCount++;
                    }
                }

            // --output-file-sequence
            } else if (strcmp(argv[i], "-of")==0 || strcmp(argv[i], "--output-file-sequence")==0) {
                outputFileSequenceCount = 0;
                i++;
                done = false;
                while ( (i < argc) && (!done) ) {
                    outputFileSequence[outputFileSequenceCount] = argv[i];
                    if (outputFileSequence[outputFileSequenceCount][0] == '-') { // is next option
                        done = true;
                        i--;
                    } else { // continue collecting filenames
                        i++;
                        outputFileSequenceCount++;
                    }
                }

            // --insert-blank
            } else if (strcmp(argv[i], "--insert-blank")==0) {
                parseMultiIndex(&i, argv, insertBlank, &insertBlankCount);

            // --replace-blank
            } else if (strcmp(argv[i], "--replace-blank")==0) {
                parseMultiIndex(&i, argv, replaceBlank, &replaceBlankCount);


            // --test-only  -T
            } else if (strcmp(argv[i], "-T")==0 || strcmp(argv[i], "--test-only")==0) {
                writeoutput = false;

            // --no-qpixels
            } else if (strcmp(argv[i], "--no-qpixels")==0) {
                qpixels = false;

            // --no-multi-pages
            } else if (strcmp(argv[i], "--no-multi-pages")==0) {
                multisheets = false;

            // --dpi
            } else if (strcmp(argv[i], "--dpi")==0) {
                sscanf(argv[++i],"%d", &dpi);

            // --type  -t
            } else if (strcmp(argv[i], "-t")==0 || strcmp(argv[i], "--type")==0) { 
                outputTypeName = argv[++i];

            // --depth  -d
            } else if (strcmp(argv[i], "-d")==0 || strcmp(argv[i], "--depth")==0) { 
                sscanf(argv[++i], "%d", &outputDepth);

            // --quiet  -q
            } else if (strcmp(argv[i], "-q")==0  || strcmp(argv[i], "--quiet")==0) {
                verbose = VERBOSE_QUIET;

            // --overwrite
            } else if (strcmp(argv[i], "--overwrite")==0) {
                overwrite = true;

            // --time
            } else if (strcmp(argv[i], "--time")==0) {
                showTime = true;

            // --verbose  -v
            } else if (strcmp(argv[i], "-v")==0  || strcmp(argv[i], "--verbose")==0) {
                verbose = VERBOSE_NORMAL;

            // -vv
            } else if (strcmp(argv[i], "-vv")==0) {
                verbose = VERBOSE_MORE;

            // --debug -vvv (undocumented)
            } else if (strcmp(argv[i], "-vvv")==0 || strcmp(argv[i], "--debug")==0) {
                verbose = VERBOSE_DEBUG;

            // --debug-save -vvvv (undocumented)
            } else if (strcmp(argv[i], "-vvvv")==0 || strcmp(argv[i], "--debug-save")==0) {
                verbose = VERBOSE_DEBUG_SAVE;

            // unkown parameter            
            } else {
                printf("*** error: Unknown parameter '%s'.\n", argv[i]);
                exitCode = 1;
            }
            
            if (exitCode != 0) {
                printf("Try 'unpaper --help' for options.\n");
                return exitCode;
            }
            i++;
        }

        
        // -------------------------------------------------------------------
        // --- begin processing                                            ---
        // -------------------------------------------------------------------

        if (first) {
            if (startSheet < nr) { // startSheet==0
                nr = startSheet;
            }
            first = false;
        }

        if ( nr == startSheet ) {
            if ( verbose >= VERBOSE_NORMAL ) {
                printf(WELCOME); // welcome message
            }
            
            if (startInput == -1) {
                startInput = (startSheet - 1) * inputCount + 1;
            }
            if (startOutput == -1) {
                startOutput = (startSheet - 1) * outputCount + 1;
            }
            
            inputNr = startInput;
            outputNr = startOutput;    
        }
        
        showTime |= (verbose >= VERBOSE_DEBUG); // always show processing time in verbose-debug mode
        
        // get filenames
        if (inputFileSequenceCount == 0) { // not yet set via option --input-file-sequence
            if (i < argc) {
                inputFileSequence[0] = argv[i++];
                inputFileSequenceCount = 1;
            } else {
                printf("*** error: Missing input filename.\n" HELP);
                return 1;
            }
        }
        if (outputFileSequenceCount == 0) { // not yet set via option --output-file-sequence
            if (i < argc) {
                outputFileSequence[0] = argv[i++];
                outputFileSequenceCount = 1;
            } else {
                printf("*** error: Missing output filename.\n" HELP);
                return 1;
            }
        }                

        // resolve filenames for current sheet
        anyWildcards = false;
        allInputFilesMissing = true;
        blankCount = 0;
        for (j = 0; j < inputCount; j++) {
            if ( (!anyWildcards) && (strchr(inputFileSequence[inputFileSequencePos], '%') != 0) ) {
                anyWildcards = true;
            }
            ins = isInMultiIndex(inputFileSequencePosTotal+1, insertBlank, insertBlankCount);
            repl = isInMultiIndex(inputFileSequencePosTotal+1, replaceBlank, replaceBlankCount);
            if (!(ins || repl)) {
                sprintf(inputFilenamesResolvedBuffer[j], inputFileSequence[inputFileSequencePos++], inputNr);
                inputFilenamesResolved[j] = inputFilenamesResolvedBuffer[j];
                if ( allInputFilesMissing && ( fileExists(inputFilenamesResolved[j]) ) ) {
                    allInputFilesMissing = false;
                }
            } else { // use blank input
                inputFilenamesResolved[j] = NULL;
                blankCount++;
                //allInputFilesMissing = false;
                if (repl) { // but skip input file sequence pos if replace-mode
                    inputFileSequencePos++;
                }
            }
            if ( inputFileSequencePos >= inputFileSequenceCount ) { // next 'loop' in input-file-seq
                inputFileSequencePos = 0;
                inputNr++;
            }
            inputFileSequencePosTotal++;
        }
        if (blankCount == inputCount) {
            allInputFilesMissing = false;
        }
        
        // multi-(input-)sheets?
        if ( multisheets && anyWildcards ) { // might already have been disabled by option (multisheets==false)
            //nop, multisheets remains true
        } else {
            multisheets = false;
            endSheet = startSheet;
        }

        for (j = 0; j < outputCount; j++) {
            if ( (!anyWildcards) && (strchr(outputFileSequence[outputFileSequencePos], '%') != 0) ) {
                anyWildcards = true;
            }
            sprintf(outputFilenamesResolvedBuffer[j], outputFileSequence[outputFileSequencePos++], outputNr);
            outputFilenamesResolved[j] = outputFilenamesResolvedBuffer[j];
            if ( outputFileSequencePos >= outputFileSequenceCount ) { // next 'loop' in output-file-seq
                outputFileSequencePos = 0;
                outputNr++;
            }
        }

        // test if (at least one) input file exists
        if ( multisheets && ( allInputFilesMissing ) ) {
            if (nr == startSheet) { // only an error if first file not found, otherwise regular end of multisheet processing
                printf("*** error: Input file(s) %s not found.\n", implode(s1, (const char **)inputFilenamesResolved, inputCount));
            }
            endSheet = nr - 1; // exit for-loop

        } else { // at least one input page file exists


            // ---------------------------------------------------------------
            // --- process single sheet                                    ---
            // ---------------------------------------------------------------

            if (isInMultiIndex(nr, sheetMultiIndex, sheetMultiIndexCount) && (!isInMultiIndex(nr, excludeMultiIndex, excludeMultiIndexCount))) {

                if (verbose >= VERBOSE_NORMAL) {
                    printf("\n-------------------------------------------------------------------------------\n");
                }
                if (verbose > VERBOSE_QUIET) {
                    if (multisheets) {
                        printf("Processing sheet #%d: %s -> %s\n", nr, implode(s1, (const char **)inputFilenamesResolved, inputCount), implode(s2, (const char **)outputFilenamesResolved, outputCount));
                    } else {
                        printf("Processing sheet: %s -> %s\n", implode(s1, (const char **)inputFilenamesResolved, inputCount), implode(s2, (const char **)outputFilenamesResolved, outputCount));
                    }
                }

                // load input image(s)
                success = true;
                for ( j = 0; (success) && (j < inputCount); j++) {
                
                    if ( (inputFilenamesResolved[j] == NULL) || fileExists(inputFilenamesResolved[j]) ) {

                        if (inputFilenamesResolved[j] != NULL) { // may be null if --insert-blank or --replace-blank
                        
                            success = loadImage(inputFilenamesResolved[j], &page, &inputType);
                            inputTypeName = FILETYPE_NAMES[inputType];
                            inputTypeNames[j] = inputTypeName;
                            sprintf(debugFilename, "_loaded_%d.pnm", inputNr-inputCount+j);
                            saveDebug(debugFilename, &page);

                            if (!success) {
                                printf("*** error: Cannot load image %s.\n", inputFilenamesResolved[j]);
                                exitCode = 2;
                            } else {
                                // pre-rotate
                                if (preRotate != 0) {
                                    if (verbose>=VERBOSE_NORMAL) {
                                        printf("pre-rotating %d degrees.\n", preRotate);
                                    }
                                    if (preRotate == 90) {
                                        flipRotate(1, &page);
                                    } else if (preRotate == -90) {
                                        flipRotate(-1, &page);
                                    }
                                }

                                // if sheet-size is not known yet (and not forced by --sheet-size), set now based on size of (first) input image
                                if ( w == -1 ) {
                                    if ( sheetSize[WIDTH] != -1 ) {
                                        w = sheetSize[WIDTH];
                                    } else {
                                        w = page.width * inputCount;
                                    }
                                }
                                if ( h == -1 ) {
                                    if ( sheetSize[HEIGHT] != -1 ) {
                                        h = sheetSize[HEIGHT];
                                    } else {
                                        h = page.height;
                                    }
                                }
                            }
                        } else { // inputFilenamesResolved[j] == NULL
                            page.buffer = NULL;
                            inputTypeNames[j] = "<none>";
                        }
                                                
                        // place image into sheet buffer
                        if ( (inputCount == 1) && (page.buffer != NULL) && (page.width == w) && (page.height == h) ) { // quick case: single input file == whole sheet
                            sheet.buffer = page.buffer;
                            sheet.bufferGrayscale = page.bufferGrayscale;
                            sheet.bufferLightness = page.bufferLightness;
                            sheet.bufferDarknessInverse = page.bufferDarknessInverse;
                            sheet.width = page.width;
                            sheet.height = page.height;
                            sheet.bitdepth = page.bitdepth;
                            sheet.color = page.color;
                            sheet.background = sheetBackground;
                        } else { // generic case: place image onto sheet by copying
                            // allocate sheet-buffer if not done yet
                            if ((sheet.buffer == NULL) && (w != -1) && (h != -1)) {
                                if ((page.buffer != NULL) && (page.bitdepth != 0)) {
                                    bd = page.bitdepth;
                                    col = page.color;
                                } else {
                                    if (outputDepth != -1) { // set via --depth
                                        bd = outputDepth; 
                                    } else {
                                        // bd remains default
                                    }
                                }
                                initImage(&sheet, w, h, bd, col, sheetBackground);
                                
                            } else if ((page.buffer != NULL) && ((page.bitdepth > sheet.bitdepth) || ( (!sheet.color) && page.color ))) { // make sure current sheet buffer has enough bitdepth and color-mode
                                sheetBackup = sheet;
                                // re-allocate sheet
                                bd = page.bitdepth;
                                col = page.color;
                                initImage(&sheet, w, h, bd, col, sheetBackground);
                                // copy old one
                                copyImage(&sheetBackup, 0, 0, &sheet);
                                freeImage(&sheetBackup);
                            }
                            if (page.buffer != NULL) {
                                if (verbose >= VERBOSE_DEBUG_SAVE) {
                                    sprintf(debugFilename, "_page%d.pnm", inputNr-inputCount+j);
                                    saveDebug(debugFilename, &page);
                                    sprintf(debugFilename, "_before_center_page%d.pnm", inputNr-inputCount+j);
                                    saveDebug(debugFilename, &sheet);
                                }
                                
                                centerImage(&page, (w * j / inputCount), 0, (w / inputCount), h, &sheet);
                                
                                if (verbose >= VERBOSE_DEBUG_SAVE) {
                                    sprintf(debugFilename, "_after_center_page%d.pnm", inputNr-inputCount+j);
                                    saveDebug(debugFilename, &sheet);
                                }
                                freeImage(&page);
                            }
                        }
                        
                    } else {
                        // allow missing page file (existance of at least one has been made sure before)
                    }
                    
                }
                
                // the only case that buffer is not yet initialized is if all blank pages have been inserted
                if (sheet.buffer == NULL) {
                    // last chance: try to get previous (unstretched/not zoomed) sheet size
                    w = previousWidth;
                    h = previousHeight;
                    bd = previousBitdepth;
                    col = previousColor;
                    if (verbose >= VERBOSE_NORMAL) {
                        printf("need to guess sheet size from previous sheet: %dx%d\n", w, h);
                    }
                    if ((w == -1) || (h == -1)) {
                        printf("*** error: sheet size unknown, use at least one input file per sheet, or force using --sheet-size.\n");
                        return 2;
                    } else {
                        initImage(&sheet, w, h, bd, col, sheetBackground);
                    }
                }

                if (success) { // sheet loaded successfully, size is known

                    previousWidth = w;
                    previousHeight = h;
                    previousBitdepth = bd;
                    previousColor = col;
                    // handle file types
                    if (outputTypeName == NULL) { // auto-set output type according to sheet format, if not explicitly set by user
                        if (sheet.color) {
                            outputType = PPM;
                        } else {
                            if (sheet.bitdepth == 1) {
                                outputType = PBM;
                            } else {
                                outputType = PGM;
                            }
                        }
                        outputTypeName = FILETYPE_NAMES[outputType];
                    } else { // parse user-setting
                        outputType = -1;
                        for (i = 0; (outputType == -1) && (i < FILETYPES_COUNT); i++) {
                            if (strcmp(outputTypeName, FILETYPE_NAMES[i])==0) {
                                outputType = i;
                            }
                        }
                        if (outputType == -1) {
                            printf("*** error: output file format '%s' is not known.\n", outputTypeName);
                            return 2;
                        }
                    }
                    if (outputDepth == -1) { // set output depth to be as input depth, if not explicitly set by user
                        outputDepth = sheet.bitdepth;
                    }

                    if (showTime) {
                        startTime = clock();
                    }

                    // pre-mirroring
                    if (preMirror != 0) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("pre-mirroring ");
                            printDirections(preMirror);
                        }
                        mirror(preMirror, &sheet);
                    }

                    // pre-shifting
                    if ((preShift[WIDTH] != 0) || ((preShift[HEIGHT] != 0))) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("pre-shifting [%d,%d]\n", preShift[WIDTH], preShift[HEIGHT]);
                        }
                        shift(preShift[WIDTH], preShift[HEIGHT], &sheet);
                    }

                    // pre-masking
                    if (preMaskCount > 0) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("pre-masking\n ");
                        }
                        applyMasks(preMask, preMaskCount, maskColor, &sheet);
                    }


                    // --------------------------------------------------------------
                    // --- verbose parameter output,                              ---
                    // --------------------------------------------------------------
                    
                    // parameters and size are known now
                    
                    if (verbose >= VERBOSE_MORE) {
                        if (layout != LAYOUT_NONE) {
                            if (layout == LAYOUT_SINGLE) {
                                layoutStr = "single";
                            } else if (layout == LAYOUT_DOUBLE) {
                                layoutStr = "double";
                            }
                            printf("layout: %s\n", layoutStr);
                        }

                        if (preRotate != 0) {
                            printf("pre-rotate: %d\n", preRotate);
                        }
                        if (preMirror != 0) {
                            printf("pre-mirror: ");
                            printDirections(preMirror);
                        }
                        if ((preShift[WIDTH] != 0) || ((preShift[HEIGHT] != 0))) {
                            printf("pre-shift: ");
                            printInts(preShift);
                        }
                        if (preWipeCount > 0) {
                            printf("pre-wipe: ");
                            for (i = 0; i < preWipeCount; i++) {
                                printf("[%d,%d,%d,%d] ",preWipe[i][LEFT],preWipe[i][TOP],preWipe[i][RIGHT],preWipe[i][BOTTOM]);
                            }
                            printf("\n");
                        }
                        if (preBorder[LEFT]!=0 || preBorder[TOP]!=0 || preBorder[RIGHT]!=0 || preBorder[BOTTOM]!=0) {
                            printf("pre-border: [%d,%d,%d,%d]\n", preBorder[LEFT], preBorder[TOP], preBorder[RIGHT], preBorder[BOTTOM]);
                        }
                        if (preMaskCount > 0) {
                            printf("pre-masking: ");
                            for (i = 0; i < preMaskCount; i++) {
                                printf("[%d,%d,%d,%d] ",preMask[i][LEFT],preMask[i][TOP],preMask[i][RIGHT],preMask[i][BOTTOM]);
                            }
                            printf("\n");
                        }
                        if ((stretchSize[WIDTH] != -1) || (stretchSize[HEIGHT] != -1)) {
                            printf("stretch to: %dx%d\n", stretchSize[WIDTH], stretchSize[HEIGHT]);
                        }
                        if ((postStretchSize[WIDTH] != -1) || (postStretchSize[HEIGHT] != -1)) {
                            printf("post-stretch to: %dx%d\n", postStretchSize[WIDTH], postStretchSize[HEIGHT]);
                        }
                        if (zoomFactor != 1.0) {
                            printf("zoom: %f\n", zoomFactor);
                        }
                        if (postZoomFactor != 1.0) {
                            printf("post-zoom: %f\n", postZoomFactor);
                        }
                        if (noBlackfilterMultiIndexCount != -1) {
                            printf("blackfilter-scan-direction: ");
                            printDirections(blackfilterScanDirections);
                            printf("blackfilter-scan-size: ");
                            printInts(blackfilterScanSize);
                            printf("blackfilter-scan-depth: ");
                            printInts(blackfilterScanDepth);
                            printf("blackfilter-scan-step: ");
                            printInts(blackfilterScanStep);
                            printf("blackfilter-scan-threshold: %f\n", blackfilterScanThreshold);
                            if (blackfilterExcludeCount > 0) {
                                printf("blackfilter-scan-exclude: ");
                                for (i = 0; i < blackfilterExcludeCount; i++) {
                                    printf("[%d,%d,%d,%d] ",blackfilterExclude[i][LEFT],blackfilterExclude[i][TOP],blackfilterExclude[i][RIGHT],blackfilterExclude[i][BOTTOM]);
                                }
                                printf("\n");
                            }
                            printf("blackfilter-intensity: %d\n", blackfilterIntensity);
                            if (noBlackfilterMultiIndexCount > 0) {
                                printf("blackfilter DISABLED for sheets: ");
                                printMultiIndex(noBlackfilterMultiIndex, noBlackfilterMultiIndexCount);
                            }
                        } else {
                            printf("blackfilter DISABLED for all sheets.\n");
                        }
                        if (noNoisefilterMultiIndexCount != -1) {
                            printf("noisefilter-intensity: %d\n", noisefilterIntensity);
                            if (noNoisefilterMultiIndexCount > 0) {
                                printf("noisefilter DISABLED for sheets: ");
                                printMultiIndex(noNoisefilterMultiIndex, noNoisefilterMultiIndexCount);
                            }
                        } else {
                            printf("noisefilter DISABLED for all sheets.\n");
                        }
                        if (noBlurfilterMultiIndexCount != -1) {
                            printf("blurfilter-size: ");
                            printInts(blurfilterScanSize);
                            printf("blurfilter-step: ");
                            printInts(blurfilterScanStep);
                            printf("blurfilter-intensity: %f\n", blurfilterIntensity);
                            if (noBlurfilterMultiIndexCount > 0) {
                                printf("blurfilter DISABLED for sheets: ");
                                printMultiIndex(noBlurfilterMultiIndex, noBlurfilterMultiIndexCount);
                            }
                        } else {
                            printf("blurfilter DISABLED for all sheets.\n");
                        }
                        if (noGrayfilterMultiIndexCount != -1) {
                            printf("grayfilter-size: ");
                            printInts(grayfilterScanSize);
                            printf("grayfilter-step: ");
                            printInts(grayfilterScanStep);
                            printf("grayfilter-threshold: %f\n", grayfilterThreshold);
                            if (noGrayfilterMultiIndexCount > 0) {
                                printf("grayfilter DISABLED for sheets: ");
                                printMultiIndex(noGrayfilterMultiIndex, noGrayfilterMultiIndexCount);
                            }
                        } else {
                            printf("grayfilter DISABLED for all sheets.\n");
                        }
                        if (noMaskScanMultiIndexCount != -1) {
                            printf("mask points: ");
                            for (i = 0; i < pointCount; i++) {
                                printf("(%d,%d) ",point[i][X],point[i][Y]);
                            }
                            printf("\n");
                            printf("mask-scan-direction: ");
                            printDirections(maskScanDirections);
                            printf("mask-scan-size: ");
                            printInts(maskScanSize);
                            printf("mask-scan-depth: ");
                            printInts(maskScanDepth);
                            printf("mask-scan-step: ");
                            printInts(maskScanStep);
                            printf("mask-scan-threshold: ");//%f\n", maskScanThreshold);
                            printFloats(maskScanThreshold);
                            printf("mask-scan-minimum: [%d,%d]\n", maskScanMinimum[WIDTH], maskScanMinimum[HEIGHT]);
                            printf("mask-scan-maximum: [%d,%d]\n", maskScanMaximum[WIDTH], maskScanMaximum[HEIGHT]);
                            printf("mask-color: %d\n", maskColor);
                            if (noMaskScanMultiIndexCount > 0) {
                                printf("mask-scan DISABLED for sheets: ");
                                printMultiIndex(noMaskScanMultiIndex, noMaskScanMultiIndexCount);
                            }
                        } else {
                            printf("mask-scan DISABLED for all sheets.\n");
                        }
                        if (noDeskewMultiIndexCount != -1) {
                            printf("deskew-scan-direction: ");
                            printEdges(deskewScanEdges);
                            printf("deskew-scan-size: %d\n", deskewScanSize);
                            printf("deskew-scan-depth: %f\n", deskewScanDepth);
                            printf("deskew-scan-range: %f\n", deskewScanRange);
                            printf("deskew-scan-step: %f\n", deskewScanStep);
                            printf("deskew-scan-deviation: %f\n", deskewScanDeviation);
                            if (qpixels==false) {
                                printf("qpixel-coding DISABLED.\n");
                            }
                            if (noDeskewMultiIndexCount > 0) {
                                printf("deskew-scan DISABLED for sheets: ");
                                printMultiIndex(noDeskewMultiIndex, noDeskewMultiIndexCount);
                            }
                        } else {
                            printf("deskew-scan DISABLED for all sheets.\n");
                        }
                        if (noWipeMultiIndexCount != -1) {
                            if (wipeCount > 0) {
                                printf("wipe areas: ");
                                for (i = 0; i < wipeCount; i++) {
                                    printf("[%d,%d,%d,%d] ", wipe[i][LEFT], wipe[i][TOP], wipe[i][RIGHT], wipe[i][BOTTOM]);
                                }
                                printf("\n");
                            }
                        } else {
                            printf("wipe DISABLED for all sheets.\n");
                        }
                        if (middleWipe[0] > 0 || middleWipe[1] > 0) {
                            printf("middle-wipe (l,r): %d,%d\n", middleWipe[0], middleWipe[1]);
                        }
                        if (noBorderMultiIndexCount != -1) {
                            if (border[LEFT]!=0 || border[TOP]!=0 || border[RIGHT]!=0 || border[BOTTOM]!=0) {
                                printf("explicit border: [%d,%d,%d,%d]\n", border[LEFT], border[TOP], border[RIGHT], border[BOTTOM]);
                            }
                        } else {
                            printf("border DISABLED for all sheets.\n");
                        }
                        if (noBorderScanMultiIndexCount != -1) {
                            printf("border-scan-direction: ");
                            printDirections(borderScanDirections);
                            printf("border-scan-size: ");
                            printInts(borderScanSize);
                            printf("border-scan-step: ");
                            printInts(borderScanStep);
                            printf("border-scan-threshold: ");//%f\n", maskScanThreshold);
                            printInts(borderScanThreshold);
                            if (noBorderScanMultiIndexCount > 0) {
                                printf("border-scan DISABLED for sheets: ");
                                printMultiIndex(noBorderScanMultiIndex, noBorderScanMultiIndexCount);
                            }
                            printf("border-align: ");
                            printEdges(borderAlign);
                            printf("border-margin: ");
                            printInts(borderAlignMargin);
                        } else {
                            printf("border-scan DISABLED for all sheets.\n");
                        }
                        if (postWipeCount > 0) {
                            printf("post-wipe: ");
                            for (i = 0; i < postWipeCount; i++) {
                                printf("[%d,%d,%d,%d] ",postWipe[i][LEFT],postWipe[i][TOP],postWipe[i][RIGHT],postWipe[i][BOTTOM]);
                            }
                            printf("\n");
                        }
                        if (postBorder[LEFT]!=0 || postBorder[TOP]!=0 || postBorder[RIGHT]!=0 || postBorder[BOTTOM]!=0) {
                            printf("post-border: [%d,%d,%d,%d]\n", postBorder[LEFT], postBorder[TOP], postBorder[RIGHT], postBorder[BOTTOM]);
                        }
                        if (postMirror != 0) {
                            printf("post-mirror: ");
                            printDirections(postMirror);
                        }
                        if ((postShift[WIDTH] != 0) || ((postShift[HEIGHT] != 0))) {
                            printf("post-shift: ");
                            printInts(postShift);
                        }
                        if (postRotate != 0) {
                            printf("post-rotate: %d\n", postRotate);
                        }
                        //if (ignoreMultiIndexCount > 0) {
                        //    printf("EXCLUDE sheets: ");
                        //    printMultiIndex(ignoreMultiIndex, ignoreMultiIndexCount);
                        //}
                        printf("white-threshold: %f\n", whiteThreshold);
                        printf("black-threshold: %f\n", blackThreshold);
                        printf("sheet-background: %s\n", ((sheetBackground == BLACK) ? "black" : "white") );
                        printf("dpi: %d\n", dpi);
                        printf("input-files per sheet: %d\n", inputCount);
                        printf("output-files per sheet: %d\n", outputCount);
                        if ((sheetSize[WIDTH] != -1) || (sheetSize[HEIGHT] != -1)) {
                            printf("sheet size forced to: %d x %d pixels\n", sheetSize[WIDTH], sheetSize[HEIGHT]);
                        }
                        printf("input-file-sequence:  %s\n", implode(s1, (const char **)inputFileSequence, inputFileSequenceCount));
                        printf("output-file-sequence: %s\n", implode(s1, (const char **)outputFileSequence, outputFileSequenceCount));
                        if (overwrite) {
                            printf("OVERWRITING EXISTING FILES\n");
                        }
                        printf("\n");
                    }
                    if (verbose >= VERBOSE_NORMAL) {
                        printf("input-file%s for sheet %d: %s (type%s %s)\n", pluralS(inputCount), nr, implode(s1, (const char **)inputFilenamesResolved, inputCount), pluralS(inputCount), implode(s2, inputTypeNames, inputCount));
                        printf("output-file%s for sheet %d: %s (type %s)\n", pluralS(outputCount), nr, implode(s1, (const char **)outputFilenamesResolved, outputCount), outputTypeName);
                        printf("sheet size: %dx%d\n", sheet.width, sheet.height);
                        printf("...\n");
                    }


                    // -------------------------------------------------------
                    // --- process image data                              ---
                    // -------------------------------------------------------

                    // stretch
                    if ((stretchSize[WIDTH] != -1) || (stretchSize[HEIGHT] != -1)) {
                        if (stretchSize[WIDTH] != -1) {
                            w = stretchSize[WIDTH];
                        } else {
                            w = sheet.width;
                        }
                        if (stretchSize[HEIGHT] != -1) {
                            h = stretchSize[HEIGHT];
                        } else {
                            h = sheet.height;
                        }
                        saveDebug("./_before-stretch.pnm", &sheet);
                        stretch(w, h, &sheet);
                        saveDebug("./_after-stretch.pnm", &sheet);
                    } 
                    
                    // zoom
                    if (zoomFactor != 1.0) {
                        w = sheet.width * zoomFactor;
                        h = sheet.height * zoomFactor;
                        stretch(w, h, &sheet);
                    }

                    // size
                    if ((size[WIDTH] != -1) || (size[HEIGHT] != -1)) {
                        if (size[WIDTH] != -1) {
                            w = size[WIDTH];
                        } else {
                            w = sheet.width;
                        }
                        if (size[HEIGHT] != -1) {
                            h = size[HEIGHT];
                        } else {
                            h = sheet.height;
                        }
                        saveDebug("./_before-resize.pnm", &sheet);
                        resize(w, h, &sheet);
                        saveDebug("./_after-resize.pnm", &sheet);
                    } 
                    
                    
                    // handle sheet layout
                    
                    // LAYOUT_SINGLE
                    if (layout == LAYOUT_SINGLE) {
                        // set middle of sheet as single starting point for mask detection
                        if (pointCount == 0) { // no manual settings, use auto-values
                            point[pointCount][X] = sheet.width / 2;
                            point[pointCount][Y] = sheet.height / 2;
                            pointCount++;
                        }
                        if (maskScanMaximum[WIDTH] == -1) {
                            maskScanMaximum[WIDTH] = sheet.width;
                        }
                        if (maskScanMaximum[HEIGHT] == -1) {
                            maskScanMaximum[HEIGHT] = sheet.height;
                        }
                        // avoid inner half of the sheet to be blackfilter-detectable
                        if (blackfilterExcludeCount == 0) { // no manual settings, use auto-values
                            blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet.width / 4;
                            blackfilterExclude[blackfilterExcludeCount][TOP] = sheet.height / 4;
                            blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet.width / 2 + sheet.width / 4;
                            blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet.height / 2 + sheet.height / 4;
                            blackfilterExcludeCount++;
                        }
                        // set single outside border to start scanning for final border-scan
                        if (outsideBorderscanMaskCount == 0) { // no manual settings, use auto-values
                            outsideBorderscanMaskCount = 1;
                            outsideBorderscanMask[0][LEFT] = 0;
                            outsideBorderscanMask[0][RIGHT] = sheet.width - 1;
                            outsideBorderscanMask[0][TOP] = 0;
                            outsideBorderscanMask[0][BOTTOM] = sheet.height - 1;
                        }
                        
                    // LAYOUT_DOUBLE
                    } else if (layout == LAYOUT_DOUBLE) {
                        // set two middle of left/right side of sheet as starting points for mask detection
                        if (pointCount == 0) { // no manual settings, use auto-values
                            point[pointCount][X] = sheet.width / 4;
                            point[pointCount][Y] = sheet.height / 2;
                            pointCount++;
                            point[pointCount][X] = sheet.width - sheet.width / 4;
                            point[pointCount][Y] = sheet.height / 2;
                            pointCount++;
                        }
                        if (maskScanMaximum[WIDTH] == -1) {
                            maskScanMaximum[WIDTH] = sheet.width / 2;
                        }
                        if (maskScanMaximum[HEIGHT] == -1) {
                            maskScanMaximum[HEIGHT] = sheet.height;
                        }
                        if (middleWipe[0] > 0 || middleWipe[1] > 0) { // left, right
                            wipe[wipeCount][LEFT] = sheet.width / 2 - middleWipe[0];
                            wipe[wipeCount][TOP] = 0;
                            wipe[wipeCount][RIGHT] =  sheet.width / 2 + middleWipe[1];
                            wipe[wipeCount][BOTTOM] = sheet.height - 1;
                            wipeCount++;
                        }
                        // avoid inner half of each page to be blackfilter-detectable
                        if (blackfilterExcludeCount == 0) { // no manual settings, use auto-values
                            blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet.width / 8;
                            blackfilterExclude[blackfilterExcludeCount][TOP] = sheet.height / 4;
                            blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet.width / 4 + sheet.width / 8;
                            blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet.height / 2 + sheet.height / 4;
                            blackfilterExcludeCount++;
                            blackfilterExclude[blackfilterExcludeCount][LEFT] = sheet.width / 2 + sheet.width / 8;
                            blackfilterExclude[blackfilterExcludeCount][TOP] = sheet.height / 4;
                            blackfilterExclude[blackfilterExcludeCount][RIGHT] = sheet.width / 2 + sheet.width / 4 + sheet.width / 8;
                            blackfilterExclude[blackfilterExcludeCount][BOTTOM] = sheet.height / 2 + sheet.height / 4;
                            blackfilterExcludeCount++;
                        }
                        // set two outside borders to start scanning for final border-scan
                        if (outsideBorderscanMaskCount == 0) { // no manual settings, use auto-values
                            outsideBorderscanMaskCount = 2;
                            outsideBorderscanMask[0][LEFT] = 0;
                            outsideBorderscanMask[0][RIGHT] = sheet.width / 2;
                            outsideBorderscanMask[0][TOP] = 0;
                            outsideBorderscanMask[0][BOTTOM] = sheet.height - 1;
                            outsideBorderscanMask[1][LEFT] = sheet.width / 2;
                            outsideBorderscanMask[1][RIGHT] = sheet.width - 1;
                            outsideBorderscanMask[1][TOP] = 0;
                            outsideBorderscanMask[1][BOTTOM] = sheet.height - 1;
                        }
                    }
                    // if maskScanMaximum still unset (no --layout specified), set to full sheet size now
                    if (maskScanMinimum[WIDTH] == -1) {
                        maskScanMaximum[WIDTH] = sheet.width;
                    }
                    if (maskScanMinimum[HEIGHT] == -1) {
                        maskScanMaximum[HEIGHT] = sheet.height;
                    }

                    
                    // pre-wipe
                    if (!isExcluded(nr, noWipeMultiIndex, noWipeMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyWipes(preWipe, preWipeCount, maskColor, &sheet);
                    }

                    // pre-border
                    if (!isExcluded(nr, noBorderMultiIndex, noBorderMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyBorder(preBorder, maskColor, &sheet);
                    }

                    // black area filter
                    if (!isExcluded(nr, noBlackfilterMultiIndex, noBlackfilterMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        saveDebug("./_before-blackfilter.pnm", &sheet);
                        blackfilter(blackfilterScanDirections, blackfilterScanSize, blackfilterScanDepth, blackfilterScanStep, blackfilterScanThreshold, blackfilterExclude, blackfilterExcludeCount, blackfilterIntensity, blackThreshold, &sheet);
                        saveDebug("./_after-blackfilter.pnm", &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ blackfilter DISABLED for sheet %d\n", nr);
                        }
                    }

                    // noise filter
                    if (!isExcluded(nr, noNoisefilterMultiIndex, noNoisefilterMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("noise-filter ...");
                        }
                        saveDebug("./_before-noisefilter.pnm", &sheet);
                        filterResult = noisefilter(noisefilterIntensity, whiteThreshold, &sheet);
                        saveDebug("./_after-noisefilter.pnm", &sheet);
                        if (verbose >= VERBOSE_NORMAL) {
                            printf(" deleted %d clusters.\n", filterResult);
                        }
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ noisefilter DISABLED for sheet %d\n", nr);
                        }
                    }

                    // blur filter
                    if (!isExcluded(nr, noBlurfilterMultiIndex, noBlurfilterMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("blur-filter...");
                        }
                        saveDebug("./_before-blurfilter.pnm", &sheet);
                        filterResult = blurfilter(blurfilterScanSize, blurfilterScanStep, blurfilterIntensity, whiteThreshold, &sheet);
                        saveDebug("./_after-blurfilter.pnm", &sheet);
                        if (verbose >= VERBOSE_NORMAL) {
                            printf(" deleted %d pixels.\n", filterResult);
                        }
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ blurfilter DISABLED for sheet %d\n", nr);
                        }
                    }

                    // mask-detection
                    if (!isExcluded(nr, noMaskScanMultiIndex, noMaskScanMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        maskCount = detectMasks(mask, maskValid, point, pointCount, maskScanDirections, maskScanSize, maskScanDepth, maskScanStep, maskScanThreshold, maskScanMinimum, maskScanMaximum, &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ mask-scan DISABLED for sheet %d\n", nr);
                        }
                    }

                    // permamently apply masks
                    if (maskCount > 0) {
                        saveDebug("./_before-masking.pnm", &sheet);
                        applyMasks(mask, maskCount, maskColor, &sheet);
                        saveDebug("./_after-masking.pnm", &sheet);
                    }

                    // gray filter
                    if (!isExcluded(nr, noGrayfilterMultiIndex, noGrayfilterMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("gray-filter...");
                        }
                        saveDebug("./_before-grayfilter.pnm", &sheet);
                        filterResult = grayfilter(grayfilterScanSize, grayfilterScanStep, grayfilterThreshold, blackThreshold, &sheet);
                        saveDebug("./_after-grayfilter.pnm", &sheet);
                        if (verbose >= VERBOSE_NORMAL) {
                            printf(" deleted %d pixels.\n", filterResult);
                        }
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ grayfilter DISABLED for sheet %d\n", nr);
                        }
                    }

                    // rotation-detection
                    if ((!isExcluded(nr, noDeskewMultiIndex, noDeskewMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount))) {
                        saveDebug("./_before-deskew.pnm", &sheet);
                        originalSheet = sheet; // copy struct entries ('clone')
                        // convert to qpixels
                        if (qpixels==true) {
                            if (verbose>=VERBOSE_NORMAL) {
                                printf("converting to qpixels.\n");
                            }
                            initImage(&qpixelSheet, sheet.width * 2, sheet.height * 2, sheet.bitdepth, sheet.color, sheetBackground);
                            convertToQPixels(&sheet, &qpixelSheet);
                            sheet = qpixelSheet;
                            q = 2; // qpixel-factor for coordinates in both directions
                        } else {
                            q = 1;
                        }

                        // detect masks again, we may get more precise results now after first masking and grayfilter
                        if (!isExcluded(nr, noMaskScanMultiIndex, noMaskScanMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                            maskCount = detectMasks(mask, maskValid, point, pointCount, maskScanDirections, maskScanSize, maskScanDepth, maskScanStep, maskScanThreshold, maskScanMinimum, maskScanMaximum, &originalSheet);
                        } else {
                            if (verbose >= VERBOSE_MORE) {
                                printf("(mask-scan before deskewing disabled)\n");
                            }
                        }

                        // auto-deskew each mask
                        for (i = 0; i < maskCount; i++) {

                            // if ( maskValid[i] == true ) { // point may have been invalidated if mask has not been auto-detected

                                // for rotation detection, original buffer is used (not qpixels)
                                saveDebug("./_before-deskew-detect.pnm", &originalSheet);
                                rotation = - detectRotation(deskewScanEdges, deskewScanRange, deskewScanStep, deskewScanSize, deskewScanDepth, deskewScanDeviation, mask[i][LEFT], mask[i][TOP], mask[i][RIGHT], mask[i][BOTTOM], &originalSheet);
                                saveDebug("./_after-deskew-detect.pnm", &originalSheet);

                                if (rotation != 0.0) {
                                    if (verbose>=VERBOSE_NORMAL) {
                                        printf("rotate (%d,%d): %f\n", point[i][X], point[i][Y], rotation);
                                    }
                                    initImage(&rect, (mask[i][RIGHT]-mask[i][LEFT]+1)*q, (mask[i][BOTTOM]-mask[i][TOP]+1)*q, sheet.bitdepth, sheet.color, sheetBackground);
                                    initImage(&rectTarget, rect.width, rect.height, sheet.bitdepth, sheet.color, sheetBackground);

                                    // copy area to rotate into rSource
                                    copyImageArea(mask[i][LEFT]*q, mask[i][TOP]*q, rect.width, rect.height, &sheet, 0, 0, &rect);

                                    // rotate
                                    rotate(degreesToRadians(rotation), &rect, &rectTarget);

                                    // copy result back into whole image
                                    copyImageArea(0, 0, rectTarget.width, rectTarget.height, &rectTarget, mask[i][LEFT]*q, mask[i][TOP]*q, &sheet);

                                    freeImage(&rect);
                                    freeImage(&rectTarget);
                                } else {
                                    if (verbose >= VERBOSE_NORMAL) {
                                        printf("rotate (%d,%d): -\n", point[i][X], point[i][Y]);
                                    }
                                }

                            // }
                        } 

                        // convert back from qpixels
                        if (qpixels == true) {
                            if (verbose >= VERBOSE_NORMAL) {
                                printf("converting back from qpixels.\n");
                            }
                            convertFromQPixels(&qpixelSheet, &originalSheet);
                            freeImage(&qpixelSheet);
                            sheet = originalSheet;
                        }
                        saveDebug("./_after-deskew.pnm", &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ deskewing DISABLED for sheet %d\n", nr);
                        }
                    }

                    // auto-center masks on either single-page or double-page layout
                    if ( (!isExcluded(nr, noMaskCenterMultiIndex, noMaskCenterMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) && (layout != LAYOUT_NONE) && (maskCount == pointCount) ) { // (maskCount==pointCount to make sure all masks had correctly been detected)
                        // perform auto-masking again to get more precise masks after rotation                    
                        if (!isExcluded(nr, noMaskScanMultiIndex, noMaskScanMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                            maskCount = detectMasks(mask, maskValid, point, pointCount, maskScanDirections, maskScanSize, maskScanDepth, maskScanStep, maskScanThreshold, maskScanMinimum, maskScanMaximum, &sheet);
                        } else {
                            if (verbose >= VERBOSE_MORE) {
                                printf("(mask-scan before centering disabled)\n");
                            }
                        }

                        saveDebug("./_before-centering.pnm", &sheet);
                        // center masks on the sheet, according to their page position
                        for (i = 0; i < maskCount; i++) {
                            centerMask(point[i][X], point[i][Y], mask[i][LEFT], mask[i][TOP], mask[i][RIGHT], mask[i][BOTTOM], &sheet);
                        }
                        saveDebug("./_after-centering.pnm", &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ auto-centering DISABLED for sheet %d\n", nr);
                        }
                    }

                    // explicit wipe
                    if (!isExcluded(nr, noWipeMultiIndex, noWipeMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyWipes(wipe, wipeCount, maskColor, &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ wipe DISABLED for sheet %d\n", nr);
                        }
                    }

                    // explicit border
                    if (!isExcluded(nr, noBorderMultiIndex, noBorderMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyBorder(border, maskColor, &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ border DISABLED for sheet %d\n", nr);
                        }
                    }

                    // border-detection
                    if (!isExcluded(nr, noBorderScanMultiIndex, noBorderScanMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        saveDebug("./_before-border.pnm", &sheet);
                        for (i = 0; i < outsideBorderscanMaskCount; i++) {
                            detectBorder(autoborder[i], borderScanDirections, borderScanSize, borderScanStep, borderScanThreshold, blackThreshold, outsideBorderscanMask[i], &sheet);
                            borderToMask(autoborder[i], autoborderMask[i], &sheet);
                        }
                        applyMasks(autoborderMask, outsideBorderscanMaskCount, maskColor, &sheet);
                        for (i = 0; i < outsideBorderscanMaskCount; i++) {
                            // border-centering
                            if (!isExcluded(nr, noBorderAlignMultiIndex, noBorderAlignMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                                alignMask(autoborderMask[i], outsideBorderscanMask[i], borderAlign, borderAlignMargin, &sheet);
                            } else {
                                if (verbose >= VERBOSE_MORE) {
                                    printf("+ border-centering DISABLED for sheet %d\n", nr);
                                }
                            }
                        }
                        saveDebug("./_after-border.pnm", &sheet);
                    } else {
                        if (verbose >= VERBOSE_MORE) {
                            printf("+ border-scan DISABLED for sheet %d\n", nr);
                        }
                    }

                    // post-wipe
                    if (!isExcluded(nr, noWipeMultiIndex, noWipeMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyWipes(postWipe, postWipeCount, maskColor, &sheet);
                    }

                    // post-border
                    if (!isExcluded(nr, noBorderMultiIndex, noBorderMultiIndexCount, ignoreMultiIndex, ignoreMultiIndexCount)) {
                        applyBorder(postBorder, maskColor, &sheet);
                    }

                    // post-mirroring
                    if (postMirror != 0) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("post-mirroring ");
                            printDirections(postMirror);
                        }
                        mirror(postMirror, &sheet);
                    }

                    // post-shifting
                    if ((postShift[WIDTH] != 0) || ((postShift[HEIGHT] != 0))) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("post-shifting [%d,%d]\n", postShift[WIDTH], postShift[HEIGHT]);
                        }
                        shift(postShift[WIDTH], postShift[HEIGHT], &sheet);
                    }

                    // post-rotating
                    if (postRotate != 0) {
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("post-rotating %d degrees.\n", postRotate);
                        }
                        if (postRotate == 90) {
                            flipRotate(1, &sheet);
                        } else if (postRotate == -90) {
                            flipRotate(-1, &sheet);
                        }
                    }

                    // post-stretch
                    if ((postStretchSize[WIDTH] != -1) || (postStretchSize[HEIGHT] != -1)) {
                        if (postStretchSize[WIDTH] != -1) {
                            w = postStretchSize[WIDTH];
                        } else {
                            w = sheet.width;
                        }
                        if (postStretchSize[HEIGHT] != -1) {
                            h = postStretchSize[HEIGHT];
                        } else {
                            h = sheet.height;
                        }
                        stretch(w, h, &sheet);
                    } 
                    
                    // post-zoom
                    if (postZoomFactor != 1.0) {
                        w = sheet.width * postZoomFactor;
                        h = sheet.height * postZoomFactor;
                        stretch(w, h, &sheet);
                    }

                    // post-size
                    if ((postSize[WIDTH] != -1) || (postSize[HEIGHT] != -1)) {
                        if (postSize[WIDTH] != -1) {
                            w = postSize[WIDTH];
                        } else {
                            w = sheet.width;
                        }
                        if (postSize[HEIGHT] != -1) {
                            h = postSize[HEIGHT];
                        } else {
                            h = sheet.height;
                        }
                        resize(w, h, &sheet);
                    } 
                    
                    if (showTime) {
                        endTime = clock();
                    }

                    // --- write output file ---

                    // write split pages output

                    if (writeoutput == true) {    
                        if (verbose >= VERBOSE_NORMAL) {
                            printf("writing output.\n");
                        }
                        // write files
                        saveDebug("./_before-save.pnm", &sheet);
                        success = true;
                        page.width = sheet.width / outputCount;
                        page.height = sheet.height;
                        page.bitdepth = sheet.bitdepth;
                        page.color = sheet.color;
                        for ( j = 0; success && (j < outputCount); j++) {
                            // get pagebuffer
                            if ( outputCount == 1 ) {
                                page.buffer = sheet.buffer;
                                page.bufferGrayscale = sheet.bufferGrayscale;
                                page.bufferLightness = sheet.bufferLightness;
                                page.bufferDarknessInverse = sheet.bufferDarknessInverse;
                            } else { // generic case: copy page-part of sheet into own buffer
                                if (page.color) {
                                    page.buffer = (uint8_t*)malloc( page.width * page.height * 3 );
                                    page.bufferGrayscale = (uint8_t*)malloc( page.width * page.height );
                                    page.bufferLightness = (uint8_t*)malloc( page.width * page.height );
                                    page.bufferDarknessInverse = (uint8_t*)malloc( page.width * page.height );
                                } else {
                                    page.buffer = (uint8_t*)malloc( page.width * page.height );
                                    page.bufferGrayscale = page.buffer;
                                    page.bufferLightness = page.buffer;
                                    page.bufferDarknessInverse = page.buffer;
                                }
                                copyImageArea(page.width * j, 0, page.width, page.height, &sheet, 0, 0, &page);
                            }
                            
                            success = saveImage(outputFilenamesResolved[j], &page, outputType, overwrite, blackThreshold);
                            
                            if ( outputCount > 1 ) {
                                freeImage(&page);
                            }
                            if (success == false) {
                                printf("*** error: Could not save image data to file %s.\n", outputFilenamesResolved[j]);
                                exitCode = 2;
                            }
                        }
                    }

                    freeImage(&sheet);
                    sheet.buffer = NULL;

                    if (showTime) {
                        if (startTime > endTime) { // clock overflow
                            endTime -= startTime; // "re-underflow" value again
                            startTime = 0;
                        }
                        time = endTime - startTime;
                        totalTime += time;
                        totalCount++;
                        printf("- processing time:  %f s\n", (float)time/CLOCKS_PER_SEC);
                    }
                }
            }
        }
    }
    if ( showTime && (totalCount > 1) ) {
       printf("- total processing time of all %d sheets:  %f s  (average:  %f s)\n", totalCount, (double)totalTime/CLOCKS_PER_SEC, (double)totalTime/totalCount/CLOCKS_PER_SEC);
    }
    return exitCode;
}
