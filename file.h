/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */


/* --- tool function for file handling ------------------------------------ */

bool fileExists(char* filename);

bool loadImage(char* filename, struct IMAGE* image, int* type);

bool saveImage(char* filename, struct IMAGE* image, int type, bool overwrite, float blackThreshold);

void saveDebug(char* filename, struct IMAGE* image);
