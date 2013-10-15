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
#ifndef DIALOGPROGINFO_H
#define DIALOGPROGINFO_H
//==========================================================================
//==========================================================================
#include <QDialog>
#include "worker.h"

//==========================================================================
//==========================================================================
namespace Ui {
class CDialogProgInfo;
}

class CDialogProgInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CDialogProgInfo(QWidget *parent = 0, const struct TProgrammerInfo *aInfo = NULL);
    ~CDialogProgInfo();

private slots:
    void on_buttonOk_clicked();

private:
    Ui::CDialogProgInfo *ui;
};

//==========================================================================
//==========================================================================
#endif // DIALOGPROGINFO_H
