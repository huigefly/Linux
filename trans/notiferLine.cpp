#include "notiferLine.h"

CNotiferLine::CNotiferLine()
{

}

CNotiferLine::~CNotiferLine()
{

}

int CNotiferLine::notify(void *pParam)
{
  printf ("notifer line :%s\n", (char *)pParam);

  return 0;
}
	