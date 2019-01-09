#include "file.h"
#include <string.h>
#include <unistd.h>

CFile::CFile()
{
  this->m_fp = NULL;
  memset (this->m_szBuf, 0, BUF_LINE_MAX);
}

CFile::~CFile()
{
  this->close ();
  if (this->m_fp) {
    this->m_fp = NULL;
  }
}

int CFile::open(const char *pszName)
{
  if (NULL == pszName || access (pszName, R_OK) != 0) {
    return -1;
  }
  
  this->m_fp = fopen (pszName, "r");
  if (this->m_fp == NULL) {
    return -2;
  }

  return 0;
}

int CFile::getLine(callBack_handleLine_t pFunc)
{
  while (fgets (this->m_szBuf, BUF_LINE_MAX, this->m_fp)) {
    pFunc (this->m_szBuf);
    memset (this->m_szBuf, 0, BUF_LINE_MAX);
  }
  
  return 0;
}

int CFile::close()
{
  if (this->m_fp) {
    fclose (this->m_fp);
  }
  printf ("func:%s\n", __FUNCTION__);
  return 0;
}
