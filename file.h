/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */


/* --- tool function for file handling ------------------------------------ */

bool loadImage(FILE *f, struct IMAGE* image, int* type);

bool saveImage(FILE *outputFile, struct IMAGE* image, int type, float blackThreshold);

void saveDebug(char* filename, struct IMAGE* image);
