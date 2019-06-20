#include <stdio.h>
#include <arpa/inet.h>

int main()
{
    short value = 0x1234;
    short obj = htons (value);
    printf ("value:%p,obj:%d\n", value, obj);

    if (0x12 == (*(char *)&obj)) {
        printf ("is big endian\n");
    } else if (0x34 == (*(char *)&obj)) {
	printf ("is big endian\n");
    }

    return 0;
}
