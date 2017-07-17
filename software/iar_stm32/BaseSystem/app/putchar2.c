#include "uart1.h"

//==========================================================================
// putchar() for printf()
//==========================================================================
int putchar2 (int aCh)
{
    unsigned char buf[1];
    int i;

    buf[0] = aCh;
    for (i = 10000; i; i --)
    {
        if (Uart1Write (buf, 1) == 1)
            break;
    }

    return 0;
}


