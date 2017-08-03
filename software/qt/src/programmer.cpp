//==========================================================================
// Programmer
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
#include <QApplication>
#include <QFileInfo>
#include <QFile>
#include <QElapsedTimer>

#ifdef Q_OS_WIN
    #include <windows.h> // for Sleep
#else
    #include <unistd.h>
#endif

#include "err_chk.h"
#include "packer.h"
#include "programmer.h"

//==========================================================================
// Defines
//==========================================================================
// Maximum length of message on link layer
#define LEN_MESSAGE_MAX             1280

// ms
#define TIMEOUT_WAIT_RESPONSE_MIN   50

// Special char
#define CHAR_SOH                    0x01

// Values
#define VALUE_CRC8_IV               0x55

// RxState values
enum
{
    S_RX_IDLE,
    S_RX_CRC8,
    S_RX_LEN_MSB,
    S_RX_LEN_LSB,
    S_RX_DATA,
    S_RX_ERROR
};

// Command List (Common)
#define CMD_GET_INFO            0x00
#define CMD_ECHO                0xff

// Command List (BaseSystem)
#define CMD_CLR_UPROG           0x81
#define CMD_LOAD_UPROG          0x82
#define CMD_CHK_UPROG           0x83
#define CMD_START_UPROG         0x84

// Command List (User Program)
#define CMD_EXIT                0x01
#define CMD_CHIP_INFO           0x02
#define CMD_CHIP_ERASE          0x03
#define CMD_READ_DATA           0x04
#define CMD_WRITE_DATA          0x05
#define CMD_READ_REG            0x06
#define CMD_WRITE_REG           0x07
#define CMD_BLANK_CHECK         0x08
#define CMD_ENTER_4BYTE_ADDR    0x09
#define CMD_EXIT_4BYTE_ADDR     0x0A

//
#define VALUE_CRC32_IV      0x5555aaaa

//==========================================================================
// Constants
//==========================================================================

//==========================================================================
// Constructor
//==========================================================================
CProgrammer::CProgrammer (QObject *aParent) :
    QObject (aParent)
{
    memset (baseSystemName, 0, sizeof (baseSystemName));
    baseSystemVersion = 0;

    memset (userProgramName, 0, sizeof (userProgramName));
    userProgramVersion = 0;

    memset (chipId , 0xff, sizeof (chipId));

    errorMessage.clear ();
}

//==========================================================================
// Destroyer
//==========================================================================
CProgrammer::~CProgrammer()
{
    serialPort.close ();
}

//==========================================================================
// Open Port
//==========================================================================
int CProgrammer::openPort (QString aPortName)
{
    serialPort.setPortName (aPortName);
    if (!serialPort.open (QIODevice::ReadWrite))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    if (!serialPort.setBaudRate (QSerialPort::Baud115200))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    if (!serialPort.setDataBits (QSerialPort::Data8))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    if (!serialPort.setParity (QSerialPort::NoParity))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    if (!serialPort.setStopBits (QSerialPort::OneStop))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    if (!serialPort.setFlowControl(QSerialPort::NoFlowControl))
    {
        errorMessage = QString ("%1 ERROR: %1").arg(aPortName).arg(serialPort.errorString ());
        return -1;
    }

    serialPort.flush();

    return 0;
}

//==========================================================================
// Close Port
//==========================================================================
void CProgrammer::closePort (void)
{
    memset (baseSystemName, 0, sizeof (baseSystemName));
    baseSystemVersion = 0;

    serialPort.close ();
}

//==========================================================================
// Load Image file
//==========================================================================
int CProgrammer::loadEkzImage (QString aImagePath)
{
    CErrorCheck *err_chk = GetErrorCheck ();
    QFileInfo file_info;
    QFile file;
    unsigned char header[16];
    unsigned long crc32;
    unsigned short data_crc;
    unsigned long data_size;
    unsigned long file_size;
    int ret;

    // Check file exist
    file_info.setFile (aImagePath);
    if (!file_info.exists())
    {
        errorMessage = QString ("Fail to load image file. %1 not found.").arg (aImagePath);
        return -1;
    }

    // Clear user program area
    UserProgramSize = 0;
    UserProgramCrc = 0xffff;
    memset (UserProgramData, 0xff, sizeof (UserProgramData));

    //
    file.setFileName (aImagePath);
    if (!file.open (QIODevice::ReadOnly))
    {
        errorMessage = QString ("Fail to load image file. %1.").arg (file.errorString());
        return -1;
    }

    ret = -1;
    do
    {
        // Read Header
        if (file.read ((char *)header, sizeof (header)) != sizeof (header))
        {
            errorMessage = "Fail to load image file. Invalid header size.";
            break;
        }

        if (memcmp (&header[0], "EKZ", 3))
        {
            errorMessage = "Fail to load image file. Invalid header signature.";
            break;
        }

        data_size = UnpackU32 (&header[4]);
        data_crc = UnpackU32 (&header[8]);
        crc32 = UnpackU32 (&header[12]);

        qDebug () << QString ().sprintf ("Data Size=%lu", data_size);
        qDebug () << QString ().sprintf (" Data CRC=0x%04X", data_crc);
        qDebug () << QString ().sprintf ("    CRC32=0x%08lX", crc32);

        file_size = file.read ((char *)UserProgramData, sizeof (UserProgramData));
        qDebug () << QString ().sprintf ("Data Size=%lu", file_size);

        if (crc32 != err_chk->Crc32 (VALUE_CRC32_IV, UserProgramData, file_size))
        {
            errorMessage = "Fail to load image file. Invalid header CRC.";
            break;
        }
        if (data_size > file_size)
        {
            errorMessage = "Fail to load image file. Invalid data size.";
            break;
        }
        if (data_size & 0x1ff)
        {
            errorMessage = "Fail to load image file. Invalid data size.";
            break;
        }

        //
        UserProgramSize = data_size;
        UserProgramCrc = data_crc;

        qDebug () << QString ().sprintf ("[DEBUG] User Program %lu bytes.", UserProgramSize);
        qDebug () << QString ().sprintf ("[DEBUG] User Program CRC=0x%04X bytes.", UserProgramCrc);
        ret = 1;

    } while (0);
    file.close ();

    return ret;
}


//==========================================================================
// Exit User Programmer
//==========================================================================
void CProgrammer::exitUserProgram (bool aWait)
{
    int tx_len;
    unsigned char *ptr;

    // Check serial port opened
    if (!serialPort.isOpen())
    {
        errorMessage = "Port not open.";
        return;
    }

    // Exit All User program
    qDebug () << QString ("[DEBUG] Exit User Program...");
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_EXIT);
    tx_len = ptr - TxBuf;
    sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));

    //
    if (aWait)
    {
#ifdef Q_OS_WIN
    Sleep(500);
#else
    usleep (500000);
#endif
    }

    // Clear user program info
    memset (userProgramName, 0, sizeof (userProgramName));
    userProgramVersion = 0;

    //
    serialPort.flush();
}

//==========================================================================
// Check BaseSystem
//==========================================================================
int CProgrammer::checkBaseSystem (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    //
    valid = false;

    // Check serial port opened
    if (!serialPort.isOpen())
    {
        errorMessage = "Port not open.";
        return -1;
    }

    // Get Info
    qDebug () << QString ("[DEBUG] Checking BaseSystem...");
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_GET_INFO);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 35)
    {
        errorMessage = QString ("Fail to get BaseSystem info. Invalid Response.");
        qDebug () << QString ().sprintf ("[DEBUG]    CMD %02X: Invalid Response.", CMD_GET_INFO);
        return -1;
    }

    UnpackString (baseSystemName, &RxBuf[1], sizeof (baseSystemName));
    baseSystemVersion = UnpackU16 (&RxBuf[17]);
    memcpy (chipId, &RxBuf[19], sizeof (chipId));

    qDebug () << QString ().sprintf ("[DEBUG]    Info: %s, V%04X", baseSystemName, baseSystemVersion);

    qDebug () << QString ().sprintf ("[DEBUG]      ID: %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X",
                                     chipId[0], chipId[1], chipId[2], chipId[3],
                                     chipId[4], chipId[5], chipId[6], chipId[7],
                                     chipId[8], chipId[9], chipId[10], chipId[11],
                                     chipId[12], chipId[13], chipId[14], chipId[15]);

    if (qstrcmp (baseSystemName, "EKZ-E08"))
    {
        errorMessage = QString ("Invalid programmer.");
        memset (baseSystemName, 0, sizeof baseSystemName);
        baseSystemVersion = 0;
        return -1;
    }

    // Name match, mark programm valid
    valid = true;
    return 0;
}

//==========================================================================
// Load User Program
//==========================================================================
int CProgrammer::loadUserProgram (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;
    unsigned long offset;

    // Check serial port opened
    if (!serialPort.isOpen())
    {
        errorMessage = "Port not open.";
        return -1;
    }

    // Clear User Program Area
    qDebug () << QString ("[DEBUG] Clear User Program Area...");
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_CLR_UPROG);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to clear user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[0] != CMD_CLR_UPROG)
    {
        errorMessage = QString ("Fail to clear user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to clear user program.");
        return -1;
    }
    qDebug () << QString ("[DEBUG]     Done.");

    // Load User Program Area
    qDebug () << QString ("[DEBUG] Load User Program...");
    for (offset = 0; offset < UserProgramSize; offset += 512)
    {
        memset (RxBuf, 0, sizeof (RxBuf));
        ptr = TxBuf;
        ptr += PackU8 (ptr, CMD_LOAD_UPROG);
        ptr += PackU32 (ptr, offset);
        memcpy (ptr, &UserProgramData[offset], 512);
        ptr += 512;
        tx_len = ptr - TxBuf;
        rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
        if (rx_len < 2)
        {
            errorMessage = QString ("Fail to load user program. Invalid Response.");
            return -1;
        }
        if (RxBuf[0] != CMD_LOAD_UPROG)
        {
            errorMessage = QString ("Fail to load user program. Invalid Response.");
            return -1;
        }
        if (RxBuf[1] != 1)
        {
            errorMessage = QString ("Unable to load user program.");
            return -1;
        }
    }
    qDebug () << QString ("[DEBUG]     Done.");

    // Check CRC
    qDebug () << QString ("[DEBUG] Check User Program CRC...");
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_CHK_UPROG);
    ptr += PackU16 (ptr, UserProgramCrc);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to verify user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[0] != CMD_CHK_UPROG)
    {
        errorMessage = QString ("Fail to verify user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Invalid user program loaded.");
        return -1;
    }
    qDebug () << QString ("[DEBUG]     Done.");

    // Start User Program
    qDebug () << QString ("[DEBUG] Start User Program...");
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_START_UPROG);
    ptr += PackU16 (ptr, UserProgramCrc);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to start user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[0] != CMD_START_UPROG)
    {
        errorMessage = QString ("Fail to start user program. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to start user program.");
        return -1;
    }
    qDebug () << QString ("[DEBUG]     Done.");

    //
    return 0;
}

//==========================================================================
// Check User Program
//==========================================================================
int CProgrammer::checkUserProgram (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    // Check serial port opened
    if (!serialPort.isOpen())
    {
        errorMessage = "Port not open.";
        return -1;
    }

    //
    qDebug () << QString ("[DEBUG] Checking User Program...");

    // Get Info
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_GET_INFO);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 35)
    {
        errorMessage = QString ("Fail to get user program info. Invalid Response.");
        return -1;
    }

    UnpackString (userProgramName, &RxBuf[1], sizeof (userProgramName));
    userProgramVersion = UnpackU16 (&RxBuf[17]);

    qDebug () << QString ().sprintf ("[DEBUG]     Info: %s, V%04X", userProgramName, userProgramVersion);
    if (strcmp (userProgramName, "EKZ-E08") == 0)
    {
        errorMessage = QString ("Invalid User Program.");
        memset (userProgramName, 0, sizeof (userProgramName));
        userProgramVersion = 0;
        return -1;
    }

    return 0;
}

//==========================================================================
// Get Flash ID
//==========================================================================
unsigned long CProgrammer::getFlashInfo (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;
    unsigned long chip_id;

    // Get Info
    chip_id = 0;
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_CHIP_INFO);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 6)
    {
        errorMessage = QString ("Fail to get FLASH info. Invalid Response.");
        return 0xffffffff;
    }
    if (RxBuf[0] != CMD_CHIP_INFO)
    {
        errorMessage = QString ("Fail to get FLASH info. Invalid Response.");
        return 0xffffffff;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to get FLASH info.");
        return 0xffffffff;
    }

    //
    errorMessage = QString ().sprintf ("[DEBUG]     SPI FLASH ID: %02X %02X %02d\n", RxBuf[2], RxBuf[3], RxBuf[4]);
    chip_id = (((unsigned long)RxBuf[2] << 16) | ((unsigned long)RxBuf[3] << 8) | ((unsigned long)RxBuf[4] << 0));

    return chip_id;
}

//==========================================================================
// Read Status Register
//==========================================================================
int CProgrammer::readStatus (void)
{
    QString str;
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_READ_REG);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    str.sprintf ("CMD_READ_REG, RxLen=%d", rx_len);
    qDebug () << str;
    str.sprintf (" %02X %02X %02X", RxBuf[0], RxBuf[1], RxBuf[2]);
    qDebug () << str;

    if (rx_len < 3)
    {
        errorMessage = QString ("Fail to write status register. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to write status register.");
        return -1;
    }

    return RxBuf[2];
}

//==========================================================================
// Write Status Register
//==========================================================================
int CProgrammer::writeStatus (unsigned char aValue)
{
    QString str;
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_WRITE_REG);
    ptr += PackU8 (ptr, aValue);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    str.sprintf ("CMD_WRITE_REG, RxLen=%d", rx_len);
    qDebug () << str;
    str.sprintf (" %02X %02X %02X", RxBuf[0], RxBuf[1], RxBuf[2]);
    qDebug () << str;

    if (rx_len < 3)
    {
        errorMessage = QString ("Fail to write status register. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to write status register.");
        return -1;
    }

    return 0;
}


//==========================================================================
// Read Chip
//==========================================================================
int CProgrammer::readFlash (unsigned char *aDest, unsigned long aSize)
{
    unsigned long addr;
    unsigned long l;
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    // Read Data
    qDebug () << QString ("[DEBUG] Read data.");
    for (addr = 0; addr < aSize; addr += 1024)
    {
        memset (RxBuf, 0, sizeof (RxBuf));
        ptr = TxBuf;
        ptr += PackU8 (ptr, CMD_READ_DATA);
        ptr += PackU32 (ptr, addr);
        tx_len = ptr - TxBuf;
        rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
        if (rx_len < (1024 + 4))
        {
            errorMessage = QString ().sprintf ("Fail to read FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[0] != CMD_READ_DATA)
        {
            errorMessage = QString ().sprintf ("Fail to read FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[1] != 1)
        {
            errorMessage = QString ().sprintf ("Unable to read FLASH. Address 0x%08lX.", addr);
            return -1;
        }

        //
        l = aSize - addr;
        if (l > 1024)
            l = 1024;
        memcpy (&aDest[addr], &RxBuf[2], l);

        if ((addr & 0xffff) == 0)
        {
            emit updateProgress (int((float)addr / aSize * 100 + 0.5));
        }
    }

    qDebug () << QString ("[DEBUG]     Done.");

    return 0;
}

//==========================================================================
// Verify Chip
//==========================================================================
int CProgrammer::verifyFlash (const unsigned char *aSrc, unsigned long aSize)
{
    unsigned long addr;
    unsigned long l;
    int rx_len;
    int tx_len;
    unsigned char *ptr;
    unsigned long i;

    // Read Data
    qDebug () << QString ("[DEBUG] Verify data.");
    for (addr = 0; addr < aSize; addr += 1024)
    {
        memset (RxBuf, 0, sizeof (RxBuf));
        ptr = TxBuf;
        ptr += PackU8 (ptr, CMD_READ_DATA);
        ptr += PackU32 (ptr, addr);
        tx_len = ptr - TxBuf;
        rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
        if (rx_len < (1024 + 4))
        {
            errorMessage = QString ().sprintf ("Fail to read FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[0] != CMD_READ_DATA)
        {
            errorMessage = QString ().sprintf ("Fail to read FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[1] != 1)
        {
            errorMessage = QString ().sprintf ("Unable to read FLASH. Address 0x%08lX.", addr);
            return -1;
        }

        //
        l = aSize - addr;
        if (l > 1024)
            l = 1024;

        if (memcmp (&aSrc[addr], &RxBuf[2], l))
        {
            for (i = 0; i < l; i ++)
            {
                if (aSrc[addr + i] != RxBuf[2 + i])
                    break;
            }
            errorMessage = QString ().sprintf ("Verify error at 0x%06lX.", addr + i);
            return -1;
        }

        if ((addr & 0xffff) == 0)
        {
            emit updateProgress (int((float)addr / aSize * 100 + 0.5));
        }
    }

    qDebug () << QString ("[DEBUG]     Done.");

    return 0;
}

//==========================================================================
// Write Chip
//==========================================================================
int CProgrammer::writeFlash (const unsigned char *aSrc, unsigned long aSize)
{
    unsigned long addr;
    unsigned long write_len;
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    // Read Data
    qDebug () << QString().sprintf ("[DEBUG] Write data %ld bytes.", aSize);
    for (addr = 0; addr < aSize; addr += 1024)
    {
        // Calculate len of this time
        write_len = aSize - addr;
        if (write_len > 1024)
            write_len = 1024;

        //
        memset (RxBuf, 0, sizeof (RxBuf));
        ptr = TxBuf;
        ptr += PackU8 (ptr, CMD_WRITE_DATA);
        ptr += PackU32 (ptr, addr);

        memcpy (ptr, &aSrc[addr], write_len);
        ptr += write_len;

        if (write_len < 1024)
        {
            memset (ptr, 0xff, 1024 - write_len);
            ptr += (1024 - write_len);
        }

        tx_len = ptr - TxBuf;
        rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf), 10000);
        if (rx_len < 2)
        {
            errorMessage = QString ().sprintf ("Fail to write FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[0] != CMD_WRITE_DATA)
        {
            errorMessage = QString ().sprintf ("Fail to write FLASH. Invalid Response at 0x%08lX.", addr);
            return -1;
        }
        if (RxBuf[1] != 1)
        {
            errorMessage = QString ().sprintf ("Unable to write FLASH. Address 0x%08lX.", addr);
            return -1;
        }

        if ((addr & 0xffff) == 0)
        {
            emit updateProgress (int((float)addr / aSize * 100 + 0.5));
        }
    }

    qDebug () << QString ("[DEBUG]     Done.");

    return 0;
}

//==========================================================================
// Erase Chip
//==========================================================================
int CProgrammer::eraseFlash (unsigned long aEraseTime)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;
    unsigned long timeout;

    // Chip Erase
    qDebug () << QString ("[DEBUG] Chip Erase. (It may take several minutes.)");
    qDebug () << QString ("[DEBUG] Erase Time=") << aEraseTime << "s";
    timeout = (aEraseTime + 10) * 1000;
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_CHIP_ERASE);
    ptr += PackU32 (ptr, timeout);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf), timeout);
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to erase FLASH. Invalid Response.");
        return -1;
    }
    if (RxBuf[0] != CMD_CHIP_ERASE)
    {
        errorMessage = QString ("Fail to erase FLASH. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to erase FLASH.");
        return -1;
    }
    qDebug () << QString().sprintf ("[DEBUG]     Erase done. %02X %02X", RxBuf[2], RxBuf[3]);

    return 0;
}

//==========================================================================
// Blank Check
//==========================================================================
int CProgrammer::blankCheck (unsigned long aBlankSize)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    if (aBlankSize == 0)
        return 0;

    // Blank Check
    qDebug () << QString().sprintf ("[DEBUG] Blank Check. Size=%ld", aBlankSize);
    memset (RxBuf, 0, sizeof (RxBuf));
    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_BLANK_CHECK);
    ptr += PackU32 (ptr, 0);
    ptr += PackU32 (ptr, aBlankSize);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf), 250000);
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to blank check FLASH. Invalid Response.");
        return -1;
    }
    if (RxBuf[0] != CMD_BLANK_CHECK)
    {
        errorMessage = QString ("Fail to blank check FLASH. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("FLASH in not blank.");
        return -1;
    }
    qDebug () << QString ("[DEBUG]     Blank Check Done.");

    return 0;
}

//==========================================================================
// Enter 4 byte address mode
//==========================================================================
int CProgrammer::enter4ByteAddrMode (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_ENTER_4BYTE_ADDR);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to enter 4-byte address mode. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to enter 4-byte address mode.");
        return -1;
    }

    return 0;
}

//==========================================================================
// Exit 4 byte address mode
//==========================================================================
int CProgrammer::exit4ByteAddrMode (void)
{
    int rx_len;
    int tx_len;
    unsigned char *ptr;

    ptr = TxBuf;
    ptr += PackU8 (ptr, CMD_EXIT_4BYTE_ADDR);
    tx_len = ptr - TxBuf;
    rx_len = sendCommand (TxBuf, tx_len, RxBuf, sizeof (RxBuf));
    if (rx_len < 2)
    {
        errorMessage = QString ("Fail to exit 4-byte address mode. Invalid Response.");
        return -1;
    }
    if (RxBuf[1] != 1)
    {
        errorMessage = QString ("Unable to exit 4-byte address mode.");
        return -1;
    }

    return 0;
}

//==========================================================================
//==========================================================================
// Public members
// ^^^^^^^^^^^^^^^
//==========================================================================
//==========================================================================
// Private members
// vvvvvvvvvvvvvv
//==========================================================================
//==========================================================================


//==========================================================================
// Send command
//==========================================================================
int CProgrammer::sendCommand (const unsigned char *aCmd, int aCmdLen, unsigned char *aResp, int aRespLen, unsigned long aTimeout)
{
    CErrorCheck *err_chk = GetErrorCheck ();
    unsigned char txbuf[LEN_MESSAGE_MAX];
    unsigned short crc8;
    int txlen;

    // Check port opened
    if (!serialPort.isOpen())
    {
        qDebug () << QString ("[DEBUG] Open Port Fail");
        return -1;
    }

    // Limit the length
    if (aCmdLen > (LEN_MESSAGE_MAX - 4))
    {
        errorMessage = QString ("CProgrammer ERROR: Command too large.");
        return -1;
    }

    // Set Timeout
    if (aTimeout < TIMEOUT_WAIT_RESPONSE_MIN)
        timeoutWaitResp = TIMEOUT_WAIT_RESPONSE_MIN;
    else
        timeoutWaitResp = aTimeout;


    // Form Tx buf
    txbuf[0] = CHAR_SOH;
    crc8 = VALUE_CRC8_IV;
    crc8 = err_chk->Crc8 (crc8, (aCmdLen >> 8));
    crc8 = err_chk->Crc8 (crc8, (aCmdLen >> 0));
    crc8 = err_chk->Crc8 (crc8, aCmd, aCmdLen);
    txbuf[1] = crc8;
    txbuf[2] = (aCmdLen >> 8);
    txbuf[3] = (aCmdLen >> 0);
    memcpy (&txbuf[4], aCmd, aCmdLen);
    txlen = aCmdLen + 4;

    // Send out
//    qDebug () << QString ().sprintf ("[DEBUG] TXed %d bytes", txlen);
//    {
//        QString str_debug;
//        unsigned char *ptr_debug = txbuf;
//        for (int i_debug = 0; i_debug < txlen; i_debug ++, ptr_debug++)
//            str_debug.append (QString ().sprintf(" %02X", *ptr_debug));
//        qDebug () << str_debug;
//    }
    serialPort.write ((const char*)txbuf, txlen);

    // Wait response
    return waitRx (aResp, aRespLen);
}

//==========================================================================
// Wait RX
//==========================================================================
int CProgrammer::waitRx (unsigned char *aResp, int aRespLen)
{
    QElapsedTimer timer;
    CErrorCheck *err_chk = GetErrorCheck ();
    int rx_state;
    int rx_len;
    int rxing_idx;
    unsigned char rx_crc;
    unsigned char chk_crc;
    int buf_len;
    unsigned char buf[64];
    unsigned char ch;
    unsigned char rx_buf[LEN_MESSAGE_MAX];

    //
    rx_state = S_RX_IDLE;
    rxing_idx = 0;
    rx_len = 0;
    rx_crc = 0;
    timer.start();
    while (timer.elapsed() < timeoutWaitResp)
    {
        serialPort.waitForReadyRead (0);
        buf_len = serialPort.read((char *)buf, sizeof (buf));

        if (buf_len < 0)
        {
            errorMessage = QString ("%1 ERROR: %1").arg(serialPort.portName()).arg(serialPort.errorString ());
            return -1;
        }

        // No data
        if (buf_len == 0)
            continue;

//        qDebug () << QString ().sprintf ("[DEBUG] %d bytes read.", buf_len);

        // Processing data
        for (int i = 0; i < buf_len ; i ++)
        {
            ch = buf[i];

            switch (rx_state)
            {
                case S_RX_IDLE:
                    if (ch == CHAR_SOH)
                    {
                        rx_state = S_RX_CRC8;
                        rx_len = 0;
                        rxing_idx = 0;
                    }
                    break;

                case S_RX_CRC8:
                    rx_crc = ch;
                    rx_state = S_RX_LEN_MSB;
                    break;

                case S_RX_LEN_MSB:
                    rx_len = ch << 8;
                    rx_state = S_RX_LEN_LSB;
                    break;

                case S_RX_LEN_LSB:
                    rx_len |= ch;
                    rx_state = S_RX_DATA;
                    if (rx_len == 0)
                    {
                        errorMessage = QString ("CProgrammer ERROR: Contentless package.");
                        qDebug () << QString ("[DEBUG] Contentless package.");
                        return -1;
                    }
                    break;

                case S_RX_DATA:
                    if (rxing_idx >= (int)sizeof (rx_buf))
                    {
                        errorMessage = QString ("CProgrammer ERROR: Rx overflow.");
                        qDebug () << QString ("[DEBUG] Rx overflow.");
                        rx_state = S_RX_ERROR;
                        break;
                    }
                    rx_buf[rxing_idx] = ch;
                    rxing_idx ++;
                    if (rxing_idx == rx_len)
                    {
                        chk_crc = VALUE_CRC8_IV;
                        chk_crc = err_chk->Crc8 (chk_crc, (rx_len >> 8));
                        chk_crc = err_chk->Crc8 (chk_crc, (rx_len >> 0));
                        chk_crc = err_chk->Crc8 (chk_crc, rx_buf, rx_len);
                        if (rx_crc != chk_crc)
                        {
                            errorMessage = QString ("CProgrammer ERROR: Rx CRC Error.");
                            qDebug () << QString ().sprintf ("[DEBUG] CRC Error, Rxed=%02X, Cal=%02X", rx_crc, chk_crc);
                            return -1;
                        }

                        if (rx_len > aRespLen)
                        {
                            errorMessage = QString ("CProgrammer ERROR: Rx package too large.");
                            qDebug () << QString ("[DEBUG] Rx package too large.");
                            return -1;
                        }
                        else
                        {
                            memcpy (aResp, rx_buf, rx_len);
//                            qDebug () << QString ().sprintf ("[DEBUG] Rxed %d bytes", rx_len);
//                            {
//                                QString str_debug;
//                                unsigned char *ptr_debug = rx_buf;
//                                for (int i_debug = 0; i_debug < rx_len; i_debug ++, ptr_debug++)
//                                    str_debug.append (QString ().sprintf(" %02X", *ptr_debug));
//                                qDebug () << str_debug;
//                            }
                            return rx_len;
                        }
                    }
                    break;
            }
        }
        timer.restart();
    }

    errorMessage = QString ("CProgrammer ERROR: Rx timeout.");
    qDebug () << QString ("[DEBUG] Rx timeout.");
    return -1;
}

