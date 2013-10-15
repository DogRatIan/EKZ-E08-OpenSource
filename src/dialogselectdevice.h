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
#ifndef DIALOGSELECTDEVICE_H
#define DIALOGSELECTDEVICE_H

#include <QDialog>
#include "flashtable.h"
#include "ui_dialogselectdevice.h"


namespace Ui {
class CDialogSelectDevice;
}

class CDialogSelectDevice : public QDialog
{
    Q_OBJECT
public:
    explicit CDialogSelectDevice (QWidget *parent = 0);
    ~CDialogSelectDevice ();

    void setFlashInfo (const struct TFlashInfo *aInfo);

    
signals:
    void resultDeviceInfo (const struct TFlashInfo *aInfo);

public slots:
    
private slots:
    void on_buttonOk_clicked();

    void on_buttonCancel_clicked();

    void on_comboBrand_currentIndexChanged(const QString &aStr);

    void on_comboDevice_currentIndexChanged(const QString &aStr);

private:
    Ui::CDialogSelectDevice *ui;

    bool comboBrandReady;
    bool comboDeviceReady;

    const struct TFlashInfo *selectedFlash;

    void updateFlashInfo (void);
};

#endif // DIALOGSELECTDEVICE_H
