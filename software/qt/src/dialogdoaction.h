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
#ifndef DIALOGDOACTION_H
#define DIALOGDOACTION_H

#include <QDialog>
#include "ui_dialogdoaction.h"
#include "worker.h"
#include "flashtable.h"

//==========================================================================
// Defines
//==========================================================================
enum
{
    DOACTIONDIALOG_NONE = 0,
    DOACTIONDIALOG_READ,
    DOACTIONDIALOG_WRITE,
    DOACTIONDIALOG_VERIFY,
    DOACTIONDIALOG_ERASE,
    DOACTIONDIALOG_DETECT
};

//==========================================================================
//==========================================================================
namespace Ui {
class CDialogDoAction;
}


class CDialogDoAction : public QDialog
{
    Q_OBJECT
public:
    explicit CDialogDoAction(QWidget *aParent = 0, int aAction = DOACTIONDIALOG_NONE);
    ~CDialogDoAction();

    void loadDataBuffer (const unsigned char *aStr, unsigned long aSize);

    QString portName;
    const struct TFlashInfo *flashInfo;

signals:
//    void resultReady (QString aStr);
    void dataReady (const unsigned char *aData, unsigned long aDataSize);
    void resultDeviceInfo (const struct TFlashInfo *aInfo);

public slots:
    void handleWorkerFinished (void);
    void handleWorkerLogMessage (QString aStr);
    void handleUpdateProgress (int aValue);

private slots:

    void on_buttonCancel_clicked();

    void on_buttonOk_clicked();

private:
    Ui::CDialogDoAction *ui;

    CWorker worker;
    int action;

    void setControls (bool aRunning);
};

//==========================================================================
//==========================================================================
#endif // DIALOGCONFIGCOMM_H
