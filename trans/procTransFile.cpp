#include "procTransFile.h"

int CProcTransFile::handleLine(const char *pszLine)
{
  printf ("func:%s, line:%s\n", __FUNCTION__, pszLine);

  return 0;
}
