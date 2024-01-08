#include <string.h>

#include "options.h"

void
optionsInit(Options *o)
{
  memset(o, 0, sizeof(Options));

  o->layout = LAYOUT_SINGLE;
}
