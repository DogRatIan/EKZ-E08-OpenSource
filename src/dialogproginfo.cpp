//==========================================================================
//
//==========================================================================
//  Copyright (c) 2012-Today DogRatIan.  All rights reserved.
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
#include "dialogproginfo.h"
#include "ui_dialogproginfo.h"

//==========================================================================
// Constructor
//==========================================================================
CDialogProgInfo::CDialogProgInfo(QWidget *parent, const TProgrammerInfo *aInfo) :
    QDialog(parent),
    ui(new Ui::CDialogProgInfo)
{
    // Load UI
    ui->setupUi (this);

    // Turn off close button
    Qt::WindowFlags flags = windowFlags ();
    flags &= (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setWindowFlags (flags);

    // Set info
    if (aInfo != NULL)
    {
        QString str_version;
        QString str_sn;
        QString str;
        int i;

        str_version.sprintf ("%d.%02d", (aInfo->version >> 8) & 0xff, (aInfo->version >> 0) & 0xff);

        for (i = 0; i < (int) sizeof (aInfo->chipId); i ++)
        {
            str.sprintf ("%02X", aInfo->chipId[i]);
            str_sn.append (str);

            ui->labelModel->setText (aInfo->name);
            ui->labelFirmware->setText (str_version);
            ui->labelSerialNumber->setText (str_sn);
        }
    }

    // Set to fixed size
    adjustSize();
    setFixedSize (width (), height ());
}

//==========================================================================
// Destroyer
//==========================================================================
CDialogProgInfo::~CDialogProgInfo()
{
    delete ui;
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
// Action Slot - on buttonOk clicked
//==========================================================================
void CDialogProgInfo::on_buttonOk_clicked()
{
    close ();
}
