#include "file.h"
#include <string.h>
#include <unistd.h>

CFile::CFile()
{
  this->m_fp = NULL;
  memset (this->m_szBuf, 0, BUF_LINE_MAX);
  this->m_vecNotifer.clear ();
}

CFile::~CFile()
{
  this->close ();
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

int CFile::getLine()
{
  while (fgets (this->m_szBuf, BUF_LINE_MAX, this->m_fp)) {
    std::vector<CINotifer*>::iterator it;
    int nSize = this->m_vecNotifer.size ();
    //for (it=this->m_vecNotifer.begin (); it!=this->m_vecNotifer.end (); it++) {
    for (int i = 0; i < nSize; i++) {
      //it->notify (this->m_szBuf);
      this->m_vecNotifer[i]->notify (this->m_szBuf);
    }
    memset (this->m_szBuf, 0, BUF_LINE_MAX);
  }
  return 0;
}

int CFile::close()
{
  if (this->m_fp) {
    fclose (this->m_fp);
    this->m_fp = NULL;
  }
  return 0;
}

int CFile::addNotifer(CINotifer* pNotifer)
{
  if (pNotifer) {
    this->m_vecNotifer.push_back (pNotifer);
  }
  return 0;
}
