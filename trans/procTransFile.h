#ifndef _PROCESS_FILE__H_201901091634
#define _PROCESS_FILE__H_201901091634

#include <stdio.h>
#include <stdlib.h>
#include "file.h"

class CProcTransFile : public CFile
{
public:
  int handleLine(const char *pszLine);
  
private:
  
};

#endif // _PROCESS_FILE__H_201901091634
 