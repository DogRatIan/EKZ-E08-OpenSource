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


#ifndef MODEL_DATABUFFER_H
#define MODEL_DATABUFFER_H

#include <QAbstractTableModel>

class CModelDataBuffer : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit CModelDataBuffer (QObject *aParent, unsigned long aMaxDataSize);
    ~CModelDataBuffer ();
    
    int rowCount (const QModelIndex &aParent = QModelIndex()) const ;
    int columnCount (const QModelIndex &aParent = QModelIndex()) const;
    QVariant data (const QModelIndex &aIndex, int aRole = Qt::DisplayRole) const;
    QVariant headerData (int aSection, Qt::Orientation aOrientation, int aRole) const;

    QString loadDataFile (QString aFile);
    QString loadData (const unsigned char *aSrc, unsigned long aSize);
    QString saveDataFile (QString aFile);
    unsigned long getBufferSize (void);
    const unsigned char *getBufferPointer (void);
    void scroll (unsigned long aPosition);
    void clearBuffer (void);

    QString hashResultMd5;
    QString hashResultSha1;
    QString hashResultSha256;
    QString hashResultCrc32b;

private:
    unsigned char *dataBuffer;
    unsigned long dataBufferSize;
    unsigned long viewOffset;
    unsigned long maxDataSize;

    void updateHash (void);

};

#endif // MODEL_DATABUFFER_H
