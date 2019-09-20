#include<stdio.h>
#define f(a,b)  a>b?a:b 
 
int f(int, float)
{
	return 0;
}
int main()
{
	printf("%d\n",f(1,2));
	return 0;
}
