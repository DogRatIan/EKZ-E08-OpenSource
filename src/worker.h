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
#ifndef _INC_WORKER_H
#define _INC_WORKER_H

//==========================================================================
//==========================================================================
#include <QThread>
#include "flashtable.h"
#include "programmer.h"

//==========================================================================
// Defines
//==========================================================================
struct TProgrammerInfo
{
    char name[16];
    unsigned short version;
    unsigned char chipId[16];
};

enum
{
    WORKERACTION_NONE = 0,
    WORKERACTION_CHECK_PROGRAMMER,
    WORKERACTION_READ,
    WORKERACTION_WRITE,
    WORKERACTION_VERIFY,
    WORKERACTION_ERASE,
    WORKERACTION_DETECT
};

//==========================================================================
// Class define
//==========================================================================
class CWorker : public QThread
{
    Q_OBJECT

public:
    CWorker (QObject * aParent = 0);
    ~CWorker ();

    void run() Q_DECL_OVERRIDE;

    void requestAbort (void);
    void setPortName (QString aPortName);
    void setAction (int aAction);
    void setFlashInfo (const struct TFlashInfo *aFlashInfo);
    void loadDataBuffer (const unsigned char *aSrc, unsigned long aSize);

    QString getPortName (void);
    const struct TFlashInfo *getFlashInfo (void);

    struct TProgrammerInfo programmerInfo;

    bool actionSuccess;
    unsigned char *dataBuffer;
    unsigned long dataBufferSize;

signals:
    void logMessage (const QString &s);
    void updateProgress (int aValue);

public slots:
    void handleUpdateProgress (int aValue);

private:
    bool abort;

    QString currentPortName;
    int currentAction;
    const struct TFlashInfo *currentFlashInfo;

    void setupProgrammer (CProgrammer *aProgrammer);
    void doActionCheckProgrammer (CProgrammer *aProgrammer);
    void doActionRead (CProgrammer *aProgrammer);
    void doActionWrite (CProgrammer *aProgrammer);
    void doActionVerify (CProgrammer *aProgrammer);
    void doActionErase (CProgrammer *aProgrammer);
    void doActionDetect (CProgrammer *aProgrammer);
};

//==========================================================================
//==========================================================================
#endif

