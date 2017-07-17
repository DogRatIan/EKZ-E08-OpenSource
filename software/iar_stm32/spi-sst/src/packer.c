#include <string.h>
#include <stdio.h>

//==========================================================================
// Buffer packing utils
//==========================================================================
int PackFloat (unsigned char *aDest, float aValue)
{
    unsigned char *ptr = (unsigned char *)&aValue;

    aDest[3] = ptr[0];
    aDest[2] = ptr[1];
    aDest[1] = ptr[2];
    aDest[0] = ptr[3];

    return 4;
}

int PackU32 (unsigned char *aDest, unsigned long aValue)
{
    aDest[0] = (aValue >> 24);
    aDest[1] = (aValue >> 16);
    aDest[2] = (aValue >> 8);
    aDest[3] = (aValue >> 0);

    return 4;
}

int PackU16 (unsigned char *aDest, unsigned short aValue)
{
    aDest[0] = (aValue >> 8);
    aDest[1] = (aValue >> 0);

    return 2;
}

int PackU8 (unsigned char *aDest, unsigned char aValue)
{
    *aDest = aValue;

    return 1;
}


int PackString (unsigned char *aDest, const char *aSrc, int aLen)
{
    int len;

    len = strlen (aSrc);
    if (len > aLen)
        len = aLen;
    memset (aDest, 0, aLen);
    memcpy (aDest, aSrc, len);

    return aLen;
}

//==========================================================================
// Buffer Unpack utils
//==========================================================================
float UnpackFloat (unsigned char *aSrc)
{
    float f;
    unsigned char *ptr;

    ptr = (unsigned char*)&f;

    ptr[0] = aSrc[3];
    ptr[1] = aSrc[2];
    ptr[2] = aSrc[1];
    ptr[3] = aSrc[0];

    return f;
}

unsigned long UnpackU32 (unsigned char *aSrc)
{
    unsigned long ret;

    ret = *aSrc;  ret <<= 8; aSrc ++;
    ret |= *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc;

    return ret;
}

unsigned short UnpackU16 (unsigned char *aSrc)
{
    unsigned short ret;

    ret = *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc;

    return ret;
}

unsigned char UnpackU8 (unsigned char *aSrc)
{
    return *aSrc;
}

void UnpackString (char *aDest, unsigned char *aSrc, int aLen)
{
    memcpy (aDest, aSrc, aLen);
    aDest[aLen - 1] = 0;
}
