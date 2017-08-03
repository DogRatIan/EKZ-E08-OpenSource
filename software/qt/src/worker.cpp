//==========================================================================
// Worker
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
#include <QFile>

#include "worker.h"

//==========================================================================
// Defines
//==========================================================================

//==========================================================================
// Variables
//==========================================================================

//==========================================================================
// Constructor
//==========================================================================
CWorker::CWorker (QObject * aParent) :
    QThread (aParent),
    actionSuccess (false),
    dataBuffer (NULL),
    dataBufferSize (0),
    abort (false),
    currentPortName ("COM1"),
    currentAction (WORKERACTION_NONE),
    currentFlashInfo (NULL)
{
}

//==========================================================================
// Destroyer
//==========================================================================
CWorker::~CWorker ()
{
    // Release memory
    if (dataBuffer != NULL)
        delete [] dataBuffer;
    dataBuffer = NULL;
    dataBufferSize = 0;
}

//==========================================================================
// Main operation
//==========================================================================
void CWorker::run()
{
    CProgrammer programmer;

    abort = false;
    actionSuccess = false;

    connect (&programmer, &CProgrammer::updateProgress, this, &CWorker::handleUpdateProgress);

    emit updateProgress (0);
    try
    {
        switch (currentAction)
        {
            case WORKERACTION_CHECK_PROGRAMMER:
                doActionCheckProgrammer (&programmer);
                emit updateProgress (100);
                break;

            case WORKERACTION_READ:
                doActionRead (&programmer);
                emit updateProgress (100);
                break;

            case WORKERACTION_WRITE:
                doActionWrite (&programmer);
                emit updateProgress (100);
                break;

            case WORKERACTION_VERIFY:
                doActionVerify (&programmer);
                emit updateProgress (100);
                break;

            case WORKERACTION_ERASE:
                doActionErase (&programmer);
                emit updateProgress (100);
                break;

            case WORKERACTION_DETECT:
                doActionDetect (&programmer);
                emit updateProgress (100);
                break;

            default:
                throw QString ("CWorker: No action set.");
        }
    }
    catch (const char *aMsg)
    {
        emit logMessage (QString (aMsg));
    }
    catch (QString aMsg)
    {
        emit logMessage (aMsg);
    }

    // Exit User Program and Close port
    programmer.exitUserProgram ();
    programmer.closePort();

    //
    disconnect (&programmer, &CProgrammer::updateProgress, this, &CWorker::handleUpdateProgress);
}

//==========================================================================
// Action Slot - handle Update Progress
//==========================================================================
void CWorker::handleUpdateProgress (int aValue)
{
    emit updateProgress (aValue);
}

//==========================================================================
// Request Abort
//==========================================================================
void CWorker::requestAbort (void)
{
    abort = true;
}

//==========================================================================
// Set Port Name
//==========================================================================
void CWorker::setPortName (QString aPortName)
{
    if (isRunning())
        return;

    currentPortName = aPortName;
    actionSuccess = false;
}

//==========================================================================
// Set Action
//==========================================================================
void CWorker::setAction (int aAction, bool aCheckBoxValue)
{
    if (isRunning())
        return;

    currentAction = aAction;
    checkBoxValue = aCheckBoxValue;
    actionSuccess = false;
}

//==========================================================================
// Set FLASH Info
//==========================================================================
void CWorker::setFlashInfo (const struct TFlashInfo *aFlashInfo)
{
    if (isRunning())
        return;

    currentFlashInfo = aFlashInfo;
    actionSuccess = false;
}


//==========================================================================
// Get Port Name
//==========================================================================
QString CWorker::getPortName (void)
{
    return currentPortName;
}

//==========================================================================
// Get Flash Info
//==========================================================================
const struct TFlashInfo *CWorker::getFlashInfo (void)
{
    return currentFlashInfo;
}


//==========================================================================
// Load Data into buffer
//==========================================================================
void CWorker::loadDataBuffer (const unsigned char *aSrc, unsigned long aSize)
{
    if (isRunning())
        return;

    // Null pointer or zero size
    if ((aSrc == NULL) || (aSize == 0))
        return;

    // Allocate data buffer
    if (dataBufferSize != aSize)
    {
        dataBufferSize = aSize;
        if (dataBuffer != NULL)
            delete [] dataBuffer;
        dataBuffer = NULL;
        if (dataBufferSize)
            dataBuffer = new unsigned char [dataBufferSize];
    }

    // Load data
    if (dataBuffer != NULL)
        memcpy (dataBuffer, aSrc, dataBufferSize);
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
// Setup Programmer
//==========================================================================
void CWorker::setupProgrammer (CProgrammer *aProgrammer)
{
    // Form Image file path
    QString image_path;
    image_path.sprintf (":/asset/%s", currentFlashInfo->imageFile);

    emit logMessage (QString ().sprintf("Using image file '%s'.", currentFlashInfo->imageFile));

    // Load Image file
    if (aProgrammer->loadEkzImage (image_path) < 0)
        throw (aProgrammer->errorMessage);

    // Open Port
    aProgrammer->closePort ();

    // Test programmer
    emit logMessage (QString ().sprintf("Communication port is %s.", currentPortName.toUtf8().data()));
    if (aProgrammer->openPort (currentPortName) < 0)
        throw (aProgrammer->errorMessage);

    if (aProgrammer->checkBaseSystem () < 0)
    {
        aProgrammer->exitUserProgram (true);
        if (aProgrammer->checkBaseSystem () < 0)
            throw (aProgrammer->errorMessage);
    }
    emit logMessage (QString ().sprintf("Programmer found, %s V%04X",
                                        aProgrammer->baseSystemName, aProgrammer->baseSystemVersion));

    // Load User program
    if (aProgrammer->loadUserProgram() < 0)
        throw (aProgrammer->errorMessage);

    // Power up waiting
    msleep (100);

    // Check user program
    if (aProgrammer->checkUserProgram() < 0)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf("Image file load success, %s V%04X",
                                        aProgrammer->userProgramName, aProgrammer->userProgramVersion));

}

//==========================================================================
// Do Action - ACTION_CHECK_PROGRAMMER
//==========================================================================
void CWorker::doActionCheckProgrammer (CProgrammer *aProgrammer)
{
    qDebug () << QString ("[DEBUG] doActionCheckProgrammer");

    // Clear Info
    memset (&programmerInfo, 0, sizeof (programmerInfo));

    // Close port
    aProgrammer->closePort ();

    // Test programmer
    if (aProgrammer->openPort (currentPortName) < 0)
        throw (aProgrammer->errorMessage);

    if (aProgrammer->checkBaseSystem () < 0)
    {
        aProgrammer->exitUserProgram (true);
        if (aProgrammer->checkBaseSystem () < 0)
            throw (aProgrammer->errorMessage);
    }

    // Save info
    if (aProgrammer->valid)
    {
        qstrncpy (programmerInfo.name, aProgrammer->baseSystemName, sizeof (programmerInfo.name));
        programmerInfo.version = aProgrammer->baseSystemVersion;
        memcpy (programmerInfo.chipId, aProgrammer->chipId, sizeof (programmerInfo.chipId));
        actionSuccess = true;
    }

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();
}

//==========================================================================
// Do Action - ACTION_READ
//==========================================================================
void CWorker::doActionRead (CProgrammer *aProgrammer)
{
    unsigned long flash_size;
    unsigned long flash_id;


    qDebug () << QString ("[DEBUG] doActionRead");

    // Check Flash Info valid
    if (currentFlashInfo == NULL)
    {
        emit logMessage ("No device selected.");
        return;
    }

    // Allocate data buffer
    flash_size = (unsigned long)currentFlashInfo->totalSizeKiB * 1024;
    if (dataBufferSize != flash_size)
    {
        dataBufferSize = flash_size;
        if (dataBuffer != NULL)
            delete [] dataBuffer;
        dataBuffer = NULL;
        if (dataBufferSize)
            dataBuffer = new unsigned char [dataBufferSize];
        memset (dataBuffer, 0, dataBufferSize);
    }

    // Show action to log
    emit logMessage (QString ().sprintf ("Reading %s %s.", currentFlashInfo->brand, currentFlashInfo->partNumber));

    // Setup Programmer
    setupProgrammer (aProgrammer);

    // Read Device ID
    flash_id = aProgrammer->getFlashInfo ();
    if (flash_id == 0xffffffff)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Device found. ID: %06lX.", flash_id));

    if (currentFlashInfo->id != flash_id)
    {
        throw (QString ("Device ID not match."));
    }

    // Enter 4-Byte Address Mode if > 16MiB
    if (currentFlashInfo->totalSizeKiB > 16384)
    {
        if (aProgrammer->enter4ByteAddrMode() < 0)
        {
            emit logMessage ("Fail to enter 4-Byte Address Mode.");
            throw (aProgrammer->errorMessage);
        }
        emit logMessage ("4-Byte Address Mode Entered.");
    }

    // Read data
    emit logMessage (QString ().sprintf ("Reading %d KiB...", currentFlashInfo->totalSizeKiB));
    if (aProgrammer->readFlash(dataBuffer, dataBufferSize) < 0)
        throw (aProgrammer->errorMessage);

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();

    //
    emit logMessage (QString ("Done."));

    // Mark success
    actionSuccess = true;
}

//==========================================================================
// Do Action - ACTION_WRITE
//==========================================================================
void CWorker::doActionWrite (CProgrammer *aProgrammer)
{
    unsigned long flash_id;

    qDebug () << QString ("[DEBUG] doActionWrite");

    // Check Flash Info valid
    if (currentFlashInfo == NULL)
    {
        emit logMessage ("No device selected.");
        return;
    }

    // Show action to log
    emit logMessage (QString ().sprintf ("Writing %s %s.", currentFlashInfo->brand, currentFlashInfo->partNumber));

    // Setup Programmer
    setupProgrammer (aProgrammer);

    // Read Device ID
    flash_id = aProgrammer->getFlashInfo ();
    if (flash_id == 0xffffffff)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Device found. ID: %06lX.", flash_id));

    if (currentFlashInfo->id != flash_id)
    {
        throw (QString ("Device ID not match."));
    }

    // Enter 4-Byte Address Mode if > 16MiB
    if (currentFlashInfo->totalSizeKiB > 16384)
    {
        if (aProgrammer->enter4ByteAddrMode() < 0)
        {
            emit logMessage ("Fail to enter 4-Byte Address Mode.");
            throw (aProgrammer->errorMessage);
        }
        emit logMessage ("4-Byte Address Mode Entered.");
    }

    if (checkBoxValue)
    {
        QString str;

        // Erase Device
        str.sprintf ("Erase started. Max time is %lds.", currentFlashInfo->chipEraseTime);
        emit logMessage (str);
        if (aProgrammer->eraseFlash (currentFlashInfo->chipEraseTime) < 0)
            throw (aProgrammer->errorMessage);

        // Blank Check
        emit logMessage (QString ("Blank Check."));
        if (aProgrammer->blankCheck (currentFlashInfo->totalSizeKiB * 1024) < 0)
            throw (aProgrammer->errorMessage);
    }

    // Write data
    emit logMessage (QString ().sprintf ("Writing %ld bytes...", dataBufferSize));
    if (aProgrammer->writeFlash(dataBuffer, dataBufferSize) < 0)
        throw (aProgrammer->errorMessage);

    // Verify data
    emit logMessage (QString ().sprintf ("Verify %ld bytes...", dataBufferSize));
    if (aProgrammer->verifyFlash (dataBuffer, dataBufferSize) < 0)
        throw (aProgrammer->errorMessage);

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();

    //
    emit logMessage (QString ("Done."));

    // Mark success
    actionSuccess = true;
}

//==========================================================================
// Do Action - ACTION_VERIFY
//==========================================================================
void CWorker::doActionVerify (CProgrammer *aProgrammer)
{
    unsigned long flash_id;

    qDebug () << QString ("[DEBUG] doActionVerify");

    // Check Flash Info valid
    if (currentFlashInfo == NULL)
    {
        emit logMessage ("No device selected.");
        return;
    }

    // Show action to log
    emit logMessage (QString ().sprintf ("Verify %s %s.", currentFlashInfo->brand, currentFlashInfo->partNumber));

    // Setup Programmer
    setupProgrammer (aProgrammer);

    // Read Device ID
    flash_id = aProgrammer->getFlashInfo ();
    if (flash_id == 0xffffffff)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Device found. ID: %06lX.", flash_id));

    if (currentFlashInfo->id != flash_id)
    {
        throw (QString ("Device ID not match."));
    }

    // Enter 4-Byte Address Mode if > 16MiB
    if (currentFlashInfo->totalSizeKiB > 16384)
    {
        if (aProgrammer->enter4ByteAddrMode() < 0)
        {
            emit logMessage ("Fail to enter 4-Byte Address Mode.");
            throw (aProgrammer->errorMessage);
        }
        emit logMessage ("4-Byte Address Mode Entered.");
    }

    // Verify data
    emit logMessage (QString ().sprintf ("Verify %ld bytes...", dataBufferSize));
    if (aProgrammer->verifyFlash (dataBuffer, dataBufferSize) < 0)
        throw (aProgrammer->errorMessage);

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();

    //
    emit logMessage (QString ("Done."));

    // Mark success
    actionSuccess = true;
}

//==========================================================================
// Do Action - ACTION_ERASE
//==========================================================================
void CWorker::doActionErase (CProgrammer *aProgrammer)
{
    QString str;
    unsigned long flash_id;

    qDebug () << QString ("[DEBUG] doActionErase");

    // Check Flash Info valid
    if (currentFlashInfo == NULL)
    {
        emit logMessage ("No device selected.");
        return;
    }

    // Show action to log
    emit logMessage (QString ().sprintf ("Erase %s %s.", currentFlashInfo->brand, currentFlashInfo->partNumber));

    // Setup Programmer
    setupProgrammer (aProgrammer);

    // Read Device ID
    flash_id = aProgrammer->getFlashInfo ();
    if (flash_id == 0xffffffff)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Device found. ID: %06lX.", flash_id));

    if (currentFlashInfo->id != flash_id)
    {
        throw (QString ("Device ID not match."));
    }

    // Erase Device
    str.sprintf ("Erase started. Max time is %lds.", currentFlashInfo->chipEraseTime);
    emit logMessage (str);
    if (aProgrammer->eraseFlash (currentFlashInfo->chipEraseTime) < 0)
        throw (aProgrammer->errorMessage);

    // Blank Check
    emit logMessage (QString ("Blank Check."));
    if (aProgrammer->blankCheck (currentFlashInfo->totalSizeKiB * 1024) < 0)
        throw (aProgrammer->errorMessage);

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();

    //
    emit logMessage (QString ("Done."));

    // Mark success
    actionSuccess = true;
}

//==========================================================================
// Do Action - ACTION_DETECT
//==========================================================================
void CWorker::doActionDetect (CProgrammer *aProgrammer)
{
    unsigned long flash_id;
    int status;

    qDebug () << QString ("[DEBUG] doActionDetect");

    // Show action to log
    emit logMessage (QString ("Detecting Device..."));

    // Setup Programmer
    setupProgrammer (aProgrammer);

    // Read Device ID
    flash_id = aProgrammer->getFlashInfo ();
    if (flash_id == 0xffffffff)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Device found. ID: %06lX.", flash_id));

    // Read status register
    status = aProgrammer->readStatus ();
    if (status < 0)
        throw (aProgrammer->errorMessage);

    emit logMessage (QString ().sprintf ("Status register: %02X.", status));


    currentFlashInfo = GetFlashTable()->getSerialFlash (flash_id);
    if (currentFlashInfo != NULL)
    {
        emit logMessage (QString ().sprintf ("Device Info: %s %s.", currentFlashInfo->brand, currentFlashInfo->partNumber));
    }

    // Exit User Program and Close port
    aProgrammer->exitUserProgram ();
    aProgrammer->closePort();

    //
    if (currentFlashInfo == NULL)
    {
        emit logMessage (QString ("No valid device found."));
    }
    else
    {
        emit logMessage (QString ("Done."));
        actionSuccess = true;
    }

}
