#include <stdio.h>
#include <glob.h>
#include <unistd.h>
#include <map>
#include <string>
#include "file.h"
#include "notiferLine.h"
using namespace std;

typedef int (*pFunc_t)(void*);

int processDir(const char *pszDir, const char *pszPatter, int nHandleMaxFile, pFunc_t pFunc)
{
  // check params
  if (NULL == pszDir || NULL == pszPatter || NULL == pFunc ||access (pszDir, R_OK) != 0) {
    return -1;
  }
  
  int nMaxFiles = nHandleMaxFile;
  int nDirStatus = 0;
  glob_t files;
  char szSelectedPattern[256] = {0};
  
  snprintf (szSelectedPattern, sizeof (szSelectedPattern), "%s/%s", pszDir, pszPatter);
  nDirStatus = glob (szSelectedPattern, 0, NULL, &files);
  switch (nDirStatus) {
    case 0:
      //printf ("files:%ld  nMaxFiles=%d\n",files.gl_pathc, 256);
      for (int i = 0; i < files.gl_pathc && (i < nMaxFiles||nMaxFiles==0); i++) {
        char szFileName[256] = {0};
        sprintf (szFileName, "%s", files.gl_pathv[i]);
        pFunc(szFileName);
      }
      break;    
    case GLOB_NOMATCH:
      printf ("No file under %s", pszDir);
      break;
    default:
      break;
  }
  return 0;
}

int handleFile(const char *pszFile)
{
  CNotiferLine notiferLine;
  CFile file;
  printf ("handle file:%s\n", pszFile);
  int nRtn = file.open (pszFile);
  if (0 == nRtn) {
    file.addNotifer (&notiferLine);
    file.getLine ();
  }
  file.close ();
  return 0;
}

int main(int argc, char *argv[])
{
  //map<string, string> mapBuf;

  processDir ("decoded", "*.txt", 256, (pFunc_t)handleFile);

#if 0
  for (int i = 0; i < 1000000; i++) {
    char szBuf1[256] = {0};
    char szBuf2[256] = {0};
    sprintf (szBuf1, "helloworld_key_%d", i);
    sprintf (szBuf2, "content_%d", i);
    mapBuf.insert(make_pair (szBuf1, szBuf2));
  }
  
  mapBuf["helloworld_key_83"] = "nihao";
  map<string, string>::iterator itMap;
  for (itMap=mapBuf.begin(); itMap!=mapBuf.end(); itMap++) {
    printf ("%s\t->%s\n", itMap->first.c_str (), itMap->second.c_str ());
  }
#endif //0
  return 0;
}
