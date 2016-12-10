//==========================================================================
//
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
#ifndef _INC_PACKER_H
#define _INC_PACKER_H

//==========================================================================
//==========================================================================
int PackFloat (unsigned char *aDest, float aValue);
int PackU32 (unsigned char *aDest, unsigned long aValue);
int PackU16 (unsigned char *aDest, unsigned short aValue);
int PackU8 (unsigned char *aDest, unsigned char aValue);
int PackString (unsigned char *aDest, const char *aSrc, int aLen, char aFill = 0);

//==========================================================================
// Buffer Unpack utils
//==========================================================================
float UnpackFloat (const unsigned char *aSrc);
unsigned long UnpackU32 (const unsigned char *aSrc);
unsigned short UnpackU16 (const unsigned char *aSrc);
unsigned char UnpackU8 (const unsigned char *aSrc);
void UnpackString (char *aDest, const unsigned char *aSrc, int aLen);

//==========================================================================
//==========================================================================
#endif
