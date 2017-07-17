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
#ifndef _INC_PROGRAMMER_H
#define _INC_PROGRAMMER_H

//==========================================================================
//==========================================================================
#include <QObject>
#include <QString>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

//==========================================================================
// Defines
//==========================================================================
#define MAX_USER_PROGRAM_SIZE 0x2000

//==========================================================================
//==========================================================================
class CProgrammer : public QObject
{
    Q_OBJECT

public:
    explicit CProgrammer(QObject * aParent = 0);
    ~CProgrammer();

    int testPort (QString aPortName);

    int openPort (QString aPortName);
    void closePort (void);
    int loadEkzImage (QString aImagePath);
    void exitUserProgram (bool aWait = false);
    int checkBaseSystem (void);
    int loadUserProgram (void);
    int checkUserProgram (void);

    unsigned long getFlashInfo (void);
    int readFlash (unsigned char *aDest, unsigned long aSize);
    int verifyFlash (const unsigned char *aSrc, unsigned long aSize);
    int writeFlash (const unsigned char *aSrc, unsigned long aSize);
    int eraseFlash (unsigned long aEraseTime);
    int blankCheck (unsigned long aBlankSize = 0);

    char baseSystemName[16];
    unsigned short baseSystemVersion;

    char userProgramName[16];
    unsigned short userProgramVersion;

    unsigned char chipId[16];

    bool valid;

    QString errorMessage;

signals:
    void updateProgress (int aValue);

private:
    QSerialPort serialPort;

    unsigned char TxBuf[1280];
    unsigned char RxBuf[1280];
    unsigned long timeoutWaitResp;

    unsigned char  UserProgramData[MAX_USER_PROGRAM_SIZE];
    unsigned short UserProgramCrc;
    unsigned long  UserProgramSize;


    int sendCommand (const unsigned char *aCmd, int aCmdLen, unsigned char *aResp, int aRespLen, unsigned long aTimeout = 100);
    int waitRx (unsigned char *aResp, int aRespLen);


};

//==========================================================================
//==========================================================================
#endif
