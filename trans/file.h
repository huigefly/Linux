#ifndef _FILE_H__201901091637
#define _FILE_H__201901091637

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "notifer.h"

#define BUF_LINE_MAX  4096
typedef int (*callBack_handleLine_t)(void *);

class CFile
{
public:
  int open(const char *pszName);
  int getLine();
  int close();
  int addNotifer(CINotifer* pNotifer);
  
  CFile();
  virtual ~CFile();
   
private:
  FILE *m_fp; 
  char m_szBuf[BUF_LINE_MAX];
  std::vector<CINotifer*> m_vecNotifer;
};

#endif // _FILE_H__201901091637
