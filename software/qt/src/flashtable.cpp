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
#include <QFileInfo>
#include <QApplication>
#include <QDir>
#include <string.h>

#include "json/json.h"
#include "flashtable.h"

//==========================================================================
//==========================================================================
#define IMAGEFILE_GENERAL_SFLASH    "spi-generic.ekz"
#define IMAGEFILE_SST_SFLASH        "spi-sst.ekz"

#define FLASHTABLE_FILENAME         "ekz-flashtable.json"


#define DEFAULT_ERASE_TIME      60

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
const struct TFlashInfo KGenericSpiFlash64KiB = {"Generic", 0x000000, "GENERIC", 64, DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH};

//   brand          id          partNumber              totalSizeKiB
//                                                             chipEraseTime
static const struct TFlashInfo KDefaultFlashTable[] = {
    {"MXIC",        0xC22010,   "MX25L512",             64,    DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22011,   "MX25L1005",            128,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22012,   "MX25L2005",            256,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22013,   "MX25L4005",            512,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22014,   "MX25L8005",            1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22015,   "MX25L1605",            2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22016,   "MX25L3205",            4096,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22017,   "MX25L6405",            8192,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"MXIC",        0xC22019,   "MX25L25645",           32768, 150,                IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3011,   "W25X10",               128,   6,                  IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3012,   "W25X20",               256,   6,                  IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3013,   "W25X40",               512,   10,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3014,   "W25X80",               1024,  20,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3015,   "W25X16",               2048,  40,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF4015,   "W25Q16",               2048,  40,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3016,   "W25X32",               4096,  80,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF3017,   "W25X64",               8192,  80,                 IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF4018,   "W25Q128",              16384, 200,                IMAGEFILE_GENERAL_SFLASH},
    {"WINBOND",     0xEF4019,   "W25Q256",              32768, 400,                IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4300,   "AT25DF021",            256,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4401,   "AT25DF041A",           512,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4502,   "AT25DF081",            1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4602,   "AT25DF161",            2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4700,   "AT(25/26)DF321",       4096,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4701,   "AT25DF321A",           4096,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4800,   "AT25DF641",            8192,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4501,   "AT26DF081A",           1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4600,   "AT26DF161",            2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"ATMEL",       0x1F4601,   "AT26DF161A",           2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010212,   "S25FL004A/040A",       512,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010213,   "S25FL008A",            1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010214,   "S25FL016A",            2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010215,   "S25FL032A",            4096,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SPANSION",    0x010216,   "S25FL064A",            8192,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C3014,   "EN25D80",              1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C3114,   "EN25F80",              1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"EON",         0x1C5114,   "EN25T80",              1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373010,   "A25L512",              64,    DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373011,   "A25L010",              128,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373012,   "A25L020",              256,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373013,   "A25L040",              512,   DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373014,   "A25L080",              1024,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373015,   "A25L016",              2048,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"AMIC",        0x373016,   "A25L032",              4096,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"SST",         0xBF48FF,   "SST25VF512/A",         64,    DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF49FF,   "SST25(LF/VF)010/A",    128,   DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF43FF,   "SST25(LF/VF)020/A",    256,   DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258C,   "SST25(PF/VF)020B",     256,   DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF44FF,   "SST25(LF/VF)040/A",    512,   DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258D,   "SST25(PF/VF)040B",     512,   DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF258E,   "SST25(PF/VF)080B",     1024,  DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF2541,   "SST25VF016B",          2048,  DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"SST",         0xBF254A,   "SST25VF032B",          4096,  DEFAULT_ERASE_TIME, IMAGEFILE_SST_SFLASH},
    {"China",       0xC86017,   "25L064",               8192,  DEFAULT_ERASE_TIME, IMAGEFILE_GENERAL_SFLASH},
    {"",            0,          "",                     0,     0,   ""}};

//==========================================================================
// Constructor
//==========================================================================
CFlashTable::CFlashTable()
{
    deviceTable.clear();
}

//==========================================================================
// Destroyer
//==========================================================================
CFlashTable::~CFlashTable()
{
    serialFlashBrand.clear ();
    deviceTable.clear();
}

//==========================================================================
// Generate Serial Flash Brand List
//==========================================================================
void CFlashTable::generateBrandList (void)
{
//    const struct TFlashInfo *info;
    std::set<std::string>::iterator it_brand;

    // Make sure the set is empty
    serialFlashBrand.clear ();
    serialFlashList.clear ();

    // Create Serial Flash Brand List
    for (std::list<struct TFlashInfo>::iterator info = deviceTable.begin(); info != deviceTable.end(); ++info)
    {
        it_brand = serialFlashBrand.find (info->brand);
        if (it_brand == serialFlashBrand.end ())
        {
            serialFlashBrand.insert (info->brand);
        }
    }

//    info = &KSerialFlashTable[0];
//    while (info->id)
//    {
//        it = serialFlashBrand.find (info->brand);
//        if (it == serialFlashBrand.end ())
//        {
//            serialFlashBrand.insert (info->brand);
//        }
//        info ++;
//    }
//    if (DEBUG)
//    {
//        for (it = serialFlashBrand.begin(); it != serialFlashBrand.end(); ++it)
//        {
//            qDebug () << QString ().sprintf ("[DEBUG] %s", it->c_str ());
//        }
//    }
}

//==========================================================================
// Generate Serial Flash Device List
//==========================================================================
void CFlashTable::generateSerialFlashList (const char *aBrand)
{
//    const struct TFlashInfo *info;
    std::set<std::string>::iterator it;

    // Clear old list
    serialFlashList.clear();

    // Create Serial Flash Brand List
//    info = &KSerialFlashTable[0];
//    while (info->id)
    for (std::list<struct TFlashInfo>::iterator info = deviceTable.begin(); info != deviceTable.end(); ++info)
    {
        if (strcmp (info->brand, aBrand) == 0)
        {
            it = serialFlashList.find (info->partNumber);
            if (it == serialFlashList.end ())
            {
                serialFlashList.insert (info->partNumber);
            }
        }
//        info ++;
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
int CFlashTable::getSerialFlash (struct TFlashInfo *aInfo, const char *aBrand, const char *aPartNumber)
{
//    const struct TFlashInfo *info;

    // Create Serial Flash Brand List
//    info = &KSerialFlashTable[0];
//    while (info->id)
    for (std::list<struct TFlashInfo>::iterator info = deviceTable.begin(); info != deviceTable.end(); ++info)
    {
        if ((qstrcmp (info->brand, aBrand) == 0) && (qstrcmp (info->partNumber, aPartNumber) == 0))
        {
            qstrncpy (aInfo->brand, info->brand, sizeof (aInfo->brand));
            qstrncpy (aInfo->partNumber, info->partNumber, sizeof (aInfo->partNumber));
            qstrncpy (aInfo->imageFile, info->imageFile, sizeof (aInfo->imageFile));
            aInfo->id = info->id;
            aInfo->totalSizeKiB = info->totalSizeKiB;
            aInfo->chipEraseTime = info->chipEraseTime;
            return 0;
        }
//        info ++;
    }

    return -1;
}

//==========================================================================
// Get Serial Flash Info (by ID)
//==========================================================================
int CFlashTable::getSerialFlash (struct TFlashInfo *aInfo, unsigned long aId)
{
//    const struct TFlashInfo *info;

    // Create Serial Flash Brand List
//    info = &KSerialFlashTable[0];
//    while (info->id)
    for (std::list<struct TFlashInfo>::iterator info = deviceTable.begin(); info != deviceTable.end(); ++info)
    {
        if (info->id == aId)
        {
            qstrncpy (aInfo->brand, info->brand, sizeof (aInfo->brand));
            qstrncpy (aInfo->partNumber, info->partNumber, sizeof (aInfo->partNumber));
            qstrncpy (aInfo->imageFile, info->imageFile, sizeof (aInfo->imageFile));
            aInfo->id = info->id;
            aInfo->totalSizeKiB = info->totalSizeKiB;
            aInfo->chipEraseTime = info->chipEraseTime;
            return 0;
        }
//        info ++;
    }

    return -1;
}


//==========================================================================
// Get maximum FLASH size
//==========================================================================
unsigned long CFlashTable::getMaximumSize (void)
{
    int max_size_kib;
//    const struct TFlashInfo *info;

    max_size_kib = 0;
//    info = &KSerialFlashTable[0];
//    while (info->id)
    for (std::list<struct TFlashInfo>::iterator info = deviceTable.begin(); info != deviceTable.end(); ++info)
    {
        if (info->totalSizeKiB > max_size_kib)
            max_size_kib = info->totalSizeKiB;
        info ++;
    }

    return ((unsigned long)max_size_kib * 1024);
}

//==========================================================================
// Load Flash Table from file
//==========================================================================
const char *CFlashTable::loadFromFile (void)
{
    QFileInfo flash_table_file (QApplication::applicationDirPath().append(QDir::separator()).append(FLASHTABLE_FILENAME));
    deviceTable.clear ();

    if (flash_table_file.size() > 0)
    {
        int line_count;
        QString line;
        int idx;
        Json::Value j_root;
        Json::Value j_item;
        Json::Reader j_reader;
        std::string j_str;
        char buf[64];
        bool ok;
        QString str;

        qDebug () << FLASHTABLE_FILENAME << " found.";
        qDebug () << "size=" << flash_table_file.size();

        j_str = "";
        QFile file (flash_table_file.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            line_count = 0;
            QTextStream in_stream (&file);
            line = in_stream.readLine();
            while (!line.isNull())
            {
                j_str.append (line.toUtf8().data());
                line_count ++;
                line = in_stream.readLine();
            }
            file.close();
            qDebug () << "number of line=" << line_count;
            qDebug () << "j_str len=" << j_str.length();

            if (j_reader.parse (j_str, j_root))
            {
                struct TFlashInfo info;

                idx = 0;
                while (1)
                {
                    j_item = j_root[idx];
                    if (j_item.isNull())
                        break;
                    idx ++;

                    memset (&info, 0, sizeof (info));
                    qstrncpy (info.brand, j_item.get ("brand", "").asCString(), sizeof (info.brand));
                    qstrncpy (info.partNumber, j_item.get ("partNumber", "").asCString(), sizeof (info.brand));
                    qstrncpy (info.imageFile, j_item.get ("imageFile", "").asCString(), sizeof (info.brand));
                    info.chipEraseTime = j_item.get ("chipEraseTime", DEFAULT_ERASE_TIME).asInt();
                    info.totalSizeKiB = j_item.get ("totalSizeKiB", 64).asInt();
                    qstrncpy (buf, j_item.get ("id", "").asCString(), sizeof (buf));

                    if (memcmp (buf, "0x", 2) == 0)
                    {
                        str = &buf[2];
                        info.id = str.toInt(&ok, 16);
                        if (!ok)
                            info.id = 0;
                    }

                    deviceTable.push_back (info);
                }
                qDebug () << idx << " device loaded.";
                return NULL;
            }
            else
            {
                return "Invalid FLASH table contents.";
            }
        }
        else
        {
            return "Unable to load " FLASHTABLE_FILENAME;
        }


        return (FLASHTABLE_FILENAME " found.");
    }
    else
    {
        qDebug () << FLASHTABLE_FILENAME << " not found.";
        return saveDefaultTable ();
    }
}


//==========================================================================
// Save Default FLASH Table to file
//==========================================================================
const char *CFlashTable::saveDefaultTable (void)
{
    QFileInfo flash_table_file (QApplication::applicationDirPath().append(QDir::separator()).append(FLASHTABLE_FILENAME));

    const struct TFlashInfo *curr_flash;
    struct TFlashInfo info;
    Json::Value j_root;
    Json::StyledWriter j_writer;
    std::string j_str;
    QString str;

    int idx;

    idx = 0;
    curr_flash = KDefaultFlashTable;
    while (curr_flash->id > 0)
    {
        j_root[idx]["brand"] = curr_flash->brand;
        str.sprintf ("0x%06lX", curr_flash->id);
        j_root[idx]["id"] = str.toUtf8().data();
        j_root[idx]["partNumber"] = curr_flash->partNumber;
        j_root[idx]["totalSizeKiB"] = curr_flash->totalSizeKiB;
        j_root[idx]["chipEraseTime"] = curr_flash->chipEraseTime;
        j_root[idx]["imageFile"] = curr_flash->imageFile;

        memcpy (&info, curr_flash, sizeof (info));
        deviceTable.push_back (info);

        idx ++;
        curr_flash ++;
    }

    j_str = j_writer.write(j_root);

    QFile file (flash_table_file.absoluteFilePath());
    if (file.open (QIODevice::WriteOnly | QIODevice::Truncate))
    {
        file.write (j_str.c_str());
        file.close ();

        qDebug () << FLASHTABLE_FILENAME << " created.";

        return (FLASHTABLE_FILENAME " created.");
    }
    else
        return ("Unable to create " FLASHTABLE_FILENAME ".");
}
