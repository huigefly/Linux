#include <stdio.h>
#include <string.h>

int main()
{
  FILE *fp = fopen ("./data_file", "r");

  if (NULL == fp) {
    return -1;
  }

  char szBuf[256] = {0};
  while (fgets (szBuf, 255, fp)) {
	printf ("buf:%s\n", szBuf);
  }

  fclose (fp);



  return 0;
}
