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
#ifndef _INC_ERR_CHK_H
#define _INC_ERR_CHK_H

//==========================================================================
//==========================================================================

//==========================================================================
//==========================================================================
class CErrorCheck
{
  public:
    CErrorCheck (void);
    ~CErrorCheck (void);

    unsigned char Crc8 (unsigned char aCrc, unsigned char aData);
    unsigned char Crc8 (unsigned char aCrc, const unsigned char *aData, int aLen);
    unsigned char Crc8 (unsigned char aCrc, const char *aData);

    unsigned short Crc16 (unsigned short aCrc, unsigned char aData);
    unsigned short Crc16 (unsigned short aCrc, const unsigned char *aData, int aLen);
    unsigned short Crc16 (unsigned short aCrc, const char *aData);

    unsigned short Crc16Ccitt (unsigned short aCrc, unsigned char aData);
    unsigned short Crc16Ccitt (unsigned short aCrc, const unsigned char *aData, int aLen);
    unsigned short Crc16Ccitt (unsigned short aCrc, const char *aData);

    unsigned long Crc32 (unsigned long aCrc, unsigned char aData);
    unsigned long Crc32 (unsigned long aCrc, const unsigned char *aData, int aLen);
    unsigned long Crc32 (unsigned long aCrc, const char *aData);

};

//==========================================================================
// Use this to get the singleton CErrorCheck
//==========================================================================
CErrorCheck *GetErrorCheck (void);

//==========================================================================
//==========================================================================
#endif
