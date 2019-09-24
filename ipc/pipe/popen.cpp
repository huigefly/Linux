#include <stdio.h>
#include <stdlib.h>

int main()
{
    int i=0;
    // type : w
    // FILE *fp = popen ("cat", "w");
    // fprintf (fp, "shenmedongxi dosjangojdongxi!");

    // type : r
    FILE *fp = popen ("ls -l", "r");
    char buf[1024];
    while (fgets (buf, 1024, fp)){
        printf ("%s", buf);
        i++;
    }
    
    printf ("\n----number:%d\n", i);

    int n = pclose (fp);
    printf ("n:%d\n", n);
}