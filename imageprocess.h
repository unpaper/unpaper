/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */

/****************************************************************************
 * image processing functions                                               *
 ****************************************************************************/


/* --- deskewing ---------------------------------------------------------- */

double detectRotation(int deskewScanEdges, int deskewScanRange, float deskewScanStep, int deskewScanSize, float deskewScanDepth, float deskewScanDeviation, int left, int top, int right, int bottom, struct IMAGE* image);

void rotate(double radians, struct IMAGE* source, struct IMAGE* target);

void convertToQPixels(struct IMAGE* image, struct IMAGE* qpixelImage);

void convertFromQPixels(struct IMAGE* qpixelImage, struct IMAGE* image);


/* --- stretching / resizing / shifting ------------------------------------ */


void stretch(int w, int h, struct IMAGE* image);

void resize(int w, int h, struct IMAGE* image);

void shift(int shiftX, int shiftY, struct IMAGE* image);


/* --- mask-detection ----------------------------------------------------- */


int detectMasks(int mask[MAX_MASKS][EDGES_COUNT], bool maskValid[MAX_MASKS], int point[MAX_POINTS][COORDINATES_COUNT], int pointCount, int maskScanDirections, int maskScanSize[DIRECTIONS_COUNT], int maskScanDepth[DIRECTIONS_COUNT], int maskScanStep[DIRECTIONS_COUNT], float maskScanThreshold[DIRECTIONS_COUNT], int maskScanMinimum[DIMENSIONS_COUNT], int maskScanMaximum[DIMENSIONS_COUNT],  struct IMAGE* image);

void applyMasks(int mask[MAX_MASKS][EDGES_COUNT], int maskCount, int maskColor, struct IMAGE* image);


/* --- wiping ------------------------------------------------------------- */


void applyWipes(int area[MAX_MASKS][EDGES_COUNT], int areaCount, int wipeColor, struct IMAGE* image);


/* --- mirroring ---------------------------------------------------------- */


void mirror(int directions, struct IMAGE* image);


/* --- flip-rotating ------------------------------------------------------ */


void flipRotate(int direction, struct IMAGE* image);


/* --- blackfilter -------------------------------------------------------- */


void blackfilter(int blackfilterScanDirections, int blackfilterScanSize[DIRECTIONS_COUNT], int blackfilterScanDepth[DIRECTIONS_COUNT], int blackfilterScanStep[DIRECTIONS_COUNT], float blackfilterScanThreshold, int blackfilterExclude[MAX_MASKS][EDGES_COUNT], int blackfilterExcludeCount, int blackfilterIntensity, float blackThreshold, struct IMAGE* image);


/* --- noisefilter -------------------------------------------------------- */


int noisefilter(int intensity, float whiteThreshold, struct IMAGE* image);


/* --- blurfilter --------------------------------------------------------- */

int blurfilter(int blurfilterScanSize[DIRECTIONS_COUNT], int blurfilterScanStep[DIRECTIONS_COUNT], float blurfilterIntensity, float whiteThreshold, struct IMAGE* image);


/* --- grayfilter --------------------------------------------------------- */


int grayfilter(int grayfilterScanSize[DIRECTIONS_COUNT], int grayfilterScanStep[DIRECTIONS_COUNT], float grayfilterThreshold, float blackThreshold, struct IMAGE* image);


/* --- border-detection --------------------------------------------------- */


void centerMask(int centerX, int centerY, int left, int top, int right, int bottom, struct IMAGE* image);

void alignMask(int mask[EDGES_COUNT], int outside[EDGES_COUNT], int direction, int margin[DIRECTIONS_COUNT], struct IMAGE* image);

void detectBorder(int border[EDGES_COUNT], int borderScanDirections, int borderScanSize[DIRECTIONS_COUNT], int borderScanStep[DIRECTIONS_COUNT], int borderScanThreshold[DIRECTIONS_COUNT], float blackThreshold, int outsideMask[EDGES_COUNT], struct IMAGE* image);

void borderToMask(int border[EDGES_COUNT], int mask[EDGES_COUNT], struct IMAGE* image);

void applyBorder(int border[EDGES_COUNT], int borderColor, struct IMAGE* image);
