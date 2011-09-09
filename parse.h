/* ---------------------------------------------------------------------------
unpaper - written by Jens Gulden 2005-2007                                  */

/* ------------------------------------------------------------------------ */



/* --- tool functions for parameter parsing and verbose output ------------ */


int parseDirections(char* s, int* exitCode);

void printDirections(int d);

int parseEdges(char* s, int* exitCode);

void printEdges(int d);

void parseInts(char* s, int i[2]);

void parseSize(char* s, int i[2], int dpi, int* exitCode);

int parseColor(char* s, int* exitCode);

void printInts(int i[2]);

void parseFloats(char* s, float f[2]);

void printFloats(float f[2]);

char* implode(char* buf, const char* s[], int cnt);

void parseMultiIndex(int* i, char* argv[], int multiIndex[], int* multiIndexCount);

bool isInMultiIndex(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount);

bool isExcluded(int index, int multiIndex[MAX_MULTI_INDEX], int multiIndexCount, int excludeIndex[MAX_MULTI_INDEX], int excludeIndexCount);

void printMultiIndex(int multiIndex[MAX_MULTI_INDEX], int multiIndexCount);

bool masksOverlapAny(int m[EDGES_COUNT], int masks[MAX_MASKS][EDGES_COUNT], int masksCount);
