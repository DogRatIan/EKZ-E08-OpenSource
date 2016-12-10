//==========================================================================
// Buffer pack/unpack routines
//==========================================================================
//  Copyright (c) 2012-Today DogRatIan.  All rights reserved.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//==========================================================================
// Naming conventions
// ~~~~~~~~~~~~~~~~~~
//                Class : Leading C
//               Struct : Leading T
//             Constant : Leading K
//      Global Variable : Leading g
//    Function argument : Leading a
//       Local Variable : All lower case
//==========================================================================
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
    aDest[0] = (unsigned char)(aValue >> 24);
    aDest[1] = (unsigned char)(aValue >> 16);
    aDest[2] = (unsigned char)(aValue >> 8);
    aDest[3] = (unsigned char)(aValue >> 0);

    return 4;
}

int PackU16 (unsigned char *aDest, unsigned short aValue)
{
    aDest[0] = (unsigned char)(aValue >> 8);
    aDest[1] = (unsigned char)(aValue >> 0);

    return 2;
}

int PackU8 (unsigned char *aDest, unsigned char aValue)
{
    *aDest = aValue;

    return 1;
}


int PackString (unsigned char *aDest, const char *aSrc, int aLen, char aFill)
{
    int len;

    len = strlen (aSrc);
    if (len > aLen)
        len = aLen;
    memset (aDest, aFill, aLen);
    memcpy (aDest, aSrc, len);

    return aLen;
}

//==========================================================================
// Buffer Unpack utils
//==========================================================================
float UnpackFloat (const unsigned char *aSrc)
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

unsigned long UnpackU32 (const unsigned char *aSrc)
{
    unsigned long ret;

    ret = *aSrc;  ret <<= 8; aSrc ++;
    ret |= *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc;

    return ret;
}

unsigned short UnpackU16 (const unsigned char *aSrc)
{
    unsigned short ret;

    ret = *aSrc; ret <<= 8; aSrc ++;
    ret |= *aSrc;

    return ret;
}

unsigned char UnpackU8 (const unsigned char *aSrc)
{
    return *aSrc;
}

void UnpackString (char *aDest, const unsigned char *aSrc, int aLen)
{
    memcpy (aDest, aSrc, aLen);
    aDest[aLen - 1] = 0;
}
