//==========================================================================
//
//==========================================================================
//  Copyright (c) 2013-Today DogRatIan.  All rights reserved.
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
#ifndef FLASHTABLE_H
#define FLASHTABLE_H

//==========================================================================
//==========================================================================
#include <string>
#include <set>

//==========================================================================
// Defines
//==========================================================================
struct TFlashInfo
{
    const char *brand;
    unsigned long id;
    const char *partNumber;
    int sectorSize;
    int numOfSector;
    int pageSize;
    int totalSizeKiB;
    double voltage;
    const char *imageFile;
};

//==========================================================================
//==========================================================================
class CFlashTable
{
public:
    CFlashTable();
    ~CFlashTable();

    void generateSerialFlashList (const char *aBrand);
    const struct TFlashInfo *getSerialFlash (const char *aBrand, const char *aPartNumber);
    const struct TFlashInfo *getSerialFlash (unsigned long aId);
    unsigned long getMaximumSize (void);

    std::set<std::string> serialFlashBrand;
    std::set<std::string> serialFlashList;


private:
};

//==========================================================================
// Use this to get the singleton CFlashTable
//==========================================================================
CFlashTable *GetFlashTable (void);


//==========================================================================
// Constants
//==========================================================================
extern const struct TFlashInfo KGenericSpiFlash64KiB;

//==========================================================================
//==========================================================================
#endif // FLASHTABLE_H
