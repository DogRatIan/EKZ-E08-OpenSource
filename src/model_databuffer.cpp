//==========================================================================
// Model - DataBuffer
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
#include <QMessageBox>
#include <QCryptographicHash>

#include "err_chk.h"
#include "model_databuffer.h"

//==========================================================================
// Defines
//==========================================================================

//==========================================================================
// Constructor
//==========================================================================
CModelDataBuffer::CModelDataBuffer (QObject *aParent, unsigned long aMaxDataSize) :
    QAbstractTableModel (aParent),
    dataBuffer (NULL),
    dataBufferSize (0),
    viewOffset (0),
    maxDataSize (aMaxDataSize)
{

}

//==========================================================================
// Destroyer
//==========================================================================
CModelDataBuffer::~CModelDataBuffer()
{
    if (dataBuffer != NULL)
        delete [] dataBuffer;
    dataBuffer = NULL;
    dataBufferSize = 0;
}


//==========================================================================
// Get ROW count
//==========================================================================
int CModelDataBuffer::rowCount (const QModelIndex &aParent) const
{
    return 16;
}

//==========================================================================
// Get COLUMN count
//==========================================================================
int CModelDataBuffer::columnCount (const QModelIndex &aParent) const
{
    return 16;
}


//==========================================================================
// Access of data
//==========================================================================
QVariant CModelDataBuffer::data (const QModelIndex &aIndex, int aRole) const
{
    unsigned long offset;

    switch (aRole)
    {
        case Qt::DisplayRole:
            offset = aIndex.row () * 16 + aIndex.column () + viewOffset;
            if (offset >= dataBufferSize)
                return QString ("");
            else
            {
                return QString ().sprintf ("%02X", dataBuffer[offset]);
            }
            break;


        case Qt::TextAlignmentRole:
            return Qt::AlignHCenter + Qt::AlignVCenter;
    }


    return QVariant ();
}

//==========================================================================
// Header
//==========================================================================
QVariant CModelDataBuffer::headerData (int aSection, Qt::Orientation aOrientation, int aRole) const
{
    if (aRole == Qt::DisplayRole)
    {
        if (aOrientation == Qt::Horizontal)
        {
            return QString ().sprintf ("%02X", aSection);
        }
        else if (aOrientation == Qt::Vertical)
        {
            return QString ().sprintf ("%07lX", aSection * 16 + viewOffset);
        }
    }

    return QVariant();
}

//==========================================================================
// Load Data file
//==========================================================================
QString CModelDataBuffer::loadDataFile (QString aFile)
{
    QFile file (aFile);

    // Check file exist
    if (!file.exists())
        return (tr ("File no found.\n%1").arg (aFile));

    // Clear old data buffer
    if (dataBuffer != NULL)
        delete [] dataBuffer;
    dataBuffer = NULL;
    dataBufferSize = 0;
    viewOffset = 0;

    // Check file size
    qDebug () << QString ().sprintf ("Data file size=%lld", file.size());
    if (file.size () > maxDataSize)
        return (tr ("File too large.\nMaximum size is %1").arg (maxDataSize));

    // Allocate data buffer
    dataBufferSize = file.size ();
    dataBuffer = new unsigned char [dataBufferSize];
    if (dataBuffer == NULL)
        return (tr ("Data buffer allication error."));

    // Open and read file into Buffer
    if (!file.open (QIODevice::ReadOnly))
        return file.errorString();

    if (file.read ((char *)dataBuffer, dataBufferSize) != dataBufferSize)
    {
        file.close ();
        return (tr ("Read file error."));
    }

    file.close ();

    // Emit data changed
    QModelIndex top_left = createIndex (0,0);
    QModelIndex bottom_right = createIndex (15,15);
    emit dataChanged(top_left, bottom_right);

    //
    updateHash ();
    return "";
}

//==========================================================================
// Save Data file
//==========================================================================
QString CModelDataBuffer::saveDataFile (QString aFile)
{
    QFile file (aFile);

    // Check data buffer
    if (dataBuffer == NULL)
        return (tr ("No data, save abort."));

    // Open and read file into Buffer
    if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
        return file.errorString();

    if (file.write ((char *)dataBuffer, dataBufferSize) != dataBufferSize)
    {
        file.close ();
        return (tr ("Write file error."));
    }

    file.close ();

    //
    return "";
}


//==========================================================================
// Load data from array
//==========================================================================
QString CModelDataBuffer::loadData (const unsigned char *aSrc, unsigned long aSize)
{
    // Clear old data buffer
    if (dataBuffer != NULL)
        delete [] dataBuffer;
    dataBuffer = NULL;
    dataBufferSize = 0;
    viewOffset = 0;

    if (aSize > maxDataSize)
        return (tr ("Array too large.\nMaximum size is %1").arg (maxDataSize));

    // Allocate data buffer
    dataBufferSize = aSize;
    dataBuffer = new unsigned char [dataBufferSize];
    if (dataBuffer == NULL)
        return (tr ("Data buffer allication error."));

    // Copy data
    memcpy (dataBuffer, aSrc, dataBufferSize);

    // Emit data changed
    QModelIndex top_left = createIndex (0,0);
    QModelIndex bottom_right = createIndex (15,15);
    emit dataChanged(top_left, bottom_right);

    //
    updateHash ();
    return "";
}


//==========================================================================
// Get size of data buffer
//==========================================================================
unsigned long CModelDataBuffer::getBufferSize (void)
{
    return dataBufferSize;
}

//==========================================================================
// Get data buffer pointer
//==========================================================================
const unsigned char *CModelDataBuffer::getBufferPointer (void)
{
    return dataBuffer;
}


//==========================================================================
// Scroll data
//==========================================================================
void CModelDataBuffer::scroll (unsigned long aPosition)
{
    unsigned long max_offset;
    unsigned long new_offset;

    if (dataBufferSize > 256)
    {
        max_offset = dataBufferSize - 256;
        if (max_offset & 0x0f)
        {
            max_offset &= (~0x0f);
            max_offset += 0x10;
        }
    }
    else
        max_offset = 0;

    if (aPosition > max_offset)
        new_offset = max_offset;
    else
        new_offset = aPosition;

    if (new_offset == viewOffset)
        return;

    viewOffset = new_offset;

    // Emit data changed
    QModelIndex top_left = createIndex (0,0);
    QModelIndex bottom_right = createIndex (15,15);
    emit dataChanged(top_left, bottom_right);
    emit headerDataChanged (Qt::Vertical, 0, 15);
}

//==========================================================================
// Clear Buffer
//==========================================================================
void CModelDataBuffer::clearBuffer (void)
{
    if (dataBuffer != NULL)
        delete [] dataBuffer;
    dataBuffer = NULL;
    dataBufferSize = 0;
    viewOffset = 0;
    QModelIndex top_left = createIndex (0,0);
    QModelIndex bottom_right = createIndex (15,15);
    emit dataChanged(top_left, bottom_right);
    emit headerDataChanged (Qt::Vertical, 0, 15);

    updateHash ();
}


//==========================================================================
//==========================================================================
// ^^^^^^^^^^^^^^^
// Public members
//==========================================================================
//==========================================================================
// Private members
// vvvvvvvvvvvvvv
//==========================================================================
//==========================================================================


//==========================================================================
// Calculate hash result of data
//==========================================================================
void CModelDataBuffer::updateHash (void)
{
    // No data
    if (dataBuffer == NULL)
    {
        hashResultMd5 = "";
        hashResultSha1 = "";
        hashResultSha256 = "";
        hashResultCrc32b = "";
        return;
    }

    //
    QCryptographicHash hash_md5 (QCryptographicHash::Md5);

    hash_md5.addData((const char *)dataBuffer, dataBufferSize);
    hashResultMd5 = QString(hash_md5.result().toHex());

    qDebug () << hashResultMd5;

    //
    QCryptographicHash hash_sha1 (QCryptographicHash::Sha1);

    hash_sha1.addData((const char *)dataBuffer, dataBufferSize);
    hashResultSha1 = QString(hash_sha1.result().toHex());

    qDebug () << hashResultSha1;

    //
    QCryptographicHash hash_sha256 (QCryptographicHash::Sha256);

    hash_sha256.addData((const char *)dataBuffer, dataBufferSize);
    hashResultSha256 = QString(hash_sha256.result().toHex());

    qDebug () << hashResultSha256;

    //
    CErrorCheck *err_chk = GetErrorCheck();

    hashResultCrc32b = QString().sprintf ("%08lx", err_chk->Crc32 (0xffffffff, dataBuffer, dataBufferSize) ^ 0xffffffff);

    qDebug () << hashResultCrc32b;

}
