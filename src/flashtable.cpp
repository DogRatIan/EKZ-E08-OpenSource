//==========================================================================
// Flash Table
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
#include <QDebug>
#include <string.h>

#include "flashtable.h"

//==========================================================================
//==========================================================================
#define IMAGEFILE_GENERAL_SFLASH     "spi-generic.ekz"
#define IMAGEFILE_SST_SFLASH         "spi-sst.ekz"

//==========================================================================
// Singleton CFlashTable
//==========================================================================
static CFlashTable gFlashTable;

// Get pointer
CFlashTable *GetFlashTable (void)
{
    return &gFlashTable;
}


//==========================================================================
// Constants
//==========================================================================
const struct TFlashInfo KGenericSpiFlash64KiB = {"Generic", 0x000000, "GENERIC", 4096, 16, 256, 64, 3.3, IMAGEFILE_GENERAL_SFLASH};

static const struct TFlashInfo KSerialFlashTable[] = {
    {"MXIC",        0xC22010,   "MX25L512",             4096, 16,   256, 64,   3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22011,   "MX25L1005",            4096, 32,   256, 128,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22012,   "MX25L2005",            4096, 64,   256, 256,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22013,   "MX25L4005",            4096, 128,  256, 512,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22014,   "MX25L8005",            4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22015,   "MX25L1605",            4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22016,   "MX25L3205",            4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22017,   "MX25L6405",            4096, 2048, 256, 8192, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3011,   "W25X10",               4096, 32,   256, 128,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3012,   "W25X20",               4096, 64,   256, 256,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3013,   "W25X40",               4096, 128,  256, 512,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3014,   "W25X80",               4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3015,   "W25X16",               4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF4015,   "W25Q16",               4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3016,   "W25X32",               4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3017,   "W25X64",               4096, 2048, 256, 8192, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4300,   "AT25DF021",            4096, 64,   256, 256,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4401,   "AT25DF041A",           4096, 128,  256, 512,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4502,   "AT25DF081",            4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4602,   "AT25DF161",            4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4700,   "AT(25/26)DF321",       4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4701,   "AT25DF321A",           4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4800,   "AT25DF641",            4096, 2048, 256, 8192, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4501,   "AT26DF081A",           4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4600,   "AT26DF161",            4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4601,   "AT26DF161A",           4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010212,   "S25FL004A/040A",       4096, 128,  256, 512,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010213,   "S25FL008A",            4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010214,   "S25FL016A",            4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010215,   "S25FL032A",            4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010216,   "S25FL064A",            4096, 2048, 256, 8192, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C3014,   "EN25D80",              4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C3114,   "EN25F80",              4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C5114,   "EN25T80",              4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373010,   "A25L512",              4096, 16,   256, 64,   3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373011,   "A25L010",              4096, 32,   256, 128,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373012,   "A25L020",              4096, 64,   256, 256,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373013,   "A25L040",              4096, 128,  256, 512,  3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373014,   "A25L080",              4096, 256,  256, 1024, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373015,   "A25L016",              4096, 512,  256, 2048, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373016,   "A25L032",              4096, 1024, 256, 4096, 3.3, IMAGEFILE_GENERAL_SFLASH},
    {"SST",         0xBF48FF,   "SST25VF512/A",         4096, 16,   256, 64,   3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF49FF,   "SST25(LF/VF)010/A",    4096, 32,   256, 128,  3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF43FF,   "SST25(LF/VF)020/A",    4096, 64,   256, 256,  3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258C,   "SST25(PF/VF)020B",     4096, 64,   256, 256,  3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF44FF,   "SST25(LF/VF)040/A",    4096, 128,  256, 512,  3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258D,   "SST25(PF/VF)040B",     4096, 128,  256, 512,  3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258E,   "SST25(PF/VF)080B",     4096, 256,  256, 1024, 3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF2541,   "SST25VF016B",          4096, 512,  256, 2048, 3.3, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF254A,   "SST25VF032B",          4096, 1024, 256, 4096, 3.3, IMAGEFILE_SST_SFLASH},
    {NULL,          0,          NULL,                   0,    0,    0,   0,    0,   0}};

//==========================================================================
// Constructor
//==========================================================================
CFlashTable::CFlashTable()
{
    const struct TFlashInfo *info;
    std::set<std::string>::iterator it;

    // Make sure the set is empty
    serialFlashBrand.clear ();
    serialFlashList.clear ();

    // Create Serial Flash Brand List
    info = &KSerialFlashTable[0];
    while (info->id)
    {
        it = serialFlashBrand.find (info->brand);
        if (it == serialFlashBrand.end ())
        {
            serialFlashBrand.insert (info->brand);
        }
        info ++;
    }

    if (DEBUG)
    {
        for (it = serialFlashBrand.begin(); it != serialFlashBrand.end(); ++it)
        {
            qDebug () << QString ().sprintf ("[DEBUG] %s", it->c_str ());
        }
    }
}

//==========================================================================
// Destroyer
//==========================================================================
CFlashTable::~CFlashTable()
{
    serialFlashBrand.clear ();
}

//==========================================================================
// Generate Serial Flash Device List
//==========================================================================
void CFlashTable::generateSerialFlashList (const char *aBrand)
{
    const struct TFlashInfo *info;
    std::set<std::string>::iterator it;

    // Clear old list
    serialFlashList.clear();

    // Create Serial Flash Brand List
    info = &KSerialFlashTable[0];
    while (info->id)
    {
        if (strcmp (info->brand, aBrand) == 0)
        {
            it = serialFlashList.find (info->partNumber);
            if (it == serialFlashList.end ())
            {
                serialFlashList.insert (info->partNumber);
            }
        }
        info ++;
    }

    if (DEBUG)
    {
        for (it = serialFlashList.begin(); it != serialFlashList.end(); ++it)
        {
            qDebug () << QString ().sprintf ("[DEBUG] %s ", it->c_str ());
        }
    }
}

//==========================================================================
// Get Serial Flash Info (by Brand and Part Number)
//==========================================================================
const struct TFlashInfo *CFlashTable::getSerialFlash (const char *aBrand, const char *aPartNumber)
{
    const struct TFlashInfo *info;

    // Create Serial Flash Brand List
    info = &KSerialFlashTable[0];
    while (info->id)
    {
        if ((qstrcmp (info->brand, aBrand) == 0) && (qstrcmp (info->partNumber, aPartNumber) == 0))
        {
            return info;
        }
        info ++;
    }

    return NULL;
}

//==========================================================================
// Get Serial Flash Info (by ID)
//==========================================================================
const struct TFlashInfo *CFlashTable::getSerialFlash (unsigned long aId)
{
    const struct TFlashInfo *info;

    // Create Serial Flash Brand List
    info = &KSerialFlashTable[0];
    while (info->id)
    {
        if (info->id == aId)
        {
            return info;
        }
        info ++;
    }

    return NULL;
}


//==========================================================================
// Get maximum FLASH size
//==========================================================================
unsigned long CFlashTable::getMaximumSize (void)
{
    int max_size_kib;
    const struct TFlashInfo *info;

    max_size_kib = 0;
    info = &KSerialFlashTable[0];
    while (info->id)
    {
        if (info->totalSizeKiB > max_size_kib)
            max_size_kib = info->totalSizeKiB;
        info ++;
    }

    return ((unsigned long)max_size_kib * 1024);
}
