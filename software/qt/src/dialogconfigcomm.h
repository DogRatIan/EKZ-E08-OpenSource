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
#ifndef DIALOGCONFIGCOMM_H
#define DIALOGCONFIGCOMM_H

#include <QDialog>
#include "ui_dialogconfigcomm.h"
#include "worker.h"

//==========================================================================
//==========================================================================
namespace Ui {
class CDialogConfigComm;
}


class CDialogConfigComm : public QDialog
{
    Q_OBJECT
public:
    explicit CDialogConfigComm(QWidget *aParent = 0);
    ~CDialogConfigComm();

    void setPortName (QString aPortName);

signals:
    void resultReady (QString aStr);
    void resultProgrammerInfo (struct TProgrammerInfo aInfo);

public slots:
    void handleWorkerFinished (void);
    
private slots:
    void on_buttonOk_clicked();

    void on_buttonCancel_clicked();

private:
    Ui::CDialogConfigComm *ui;

    CWorker worker;

    void setControls (bool aRunning);
};

//==========================================================================
//==========================================================================
#endif // DIALOGCONFIGCOMM_H
