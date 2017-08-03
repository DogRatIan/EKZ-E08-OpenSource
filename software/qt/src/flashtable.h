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
    char brand[64];
    unsigned long id;
    char partNumber[64];
    int totalSizeKiB;
    int chipEraseTime;
    char imageFile[128];
};

//==========================================================================
//==========================================================================
class CFlashTable
{
public:
    CFlashTable();
    ~CFlashTable();

    void generateBrandList (void);
    void generateSerialFlashList (const char *aBrand);
    int getSerialFlash(struct TFlashInfo *aInfo, const char *aBrand, const char *aPartNumber);
    int getSerialFlash (struct TFlashInfo *aInfo, unsigned long aId);
    unsigned long getMaximumSize (void);

    const char *loadFromFile(void);
    const char *saveDefaultTable(void);

    std::set<std::string> serialFlashBrand;
    std::set<std::string> serialFlashList;

    std::list <struct TFlashInfo> deviceTable;

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
