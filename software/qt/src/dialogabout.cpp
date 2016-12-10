//==========================================================================
// Dialog - About
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
#include "dialogabout.h"

//==========================================================================
// Constructor
//==========================================================================
CDialogAbout::CDialogAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDialogAbout)
{
    // Load UI
    ui->setupUi (this);

    // Turn off close button
    Qt::WindowFlags flags = windowFlags ();
    flags &= (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setWindowFlags (flags);

    // Set Icons
    ui->imageProgramIcon->setScene (new QGraphicsScene (this));
    ui->imageProgramIcon->setBackgroundBrush (palette().background());
    ui->imageProgramIcon->scene()->addPixmap (QPixmap ("://asset/program.png"));


    ui->imageLogo1->setScene (new QGraphicsScene (this));
    ui->imageLogo1->setBackgroundBrush (palette().background());
    ui->imageLogo1->scene()->addPixmap (QPixmap (":/asset/logo_ekz.png"));

    ui->imageLogo2->setScene (new QGraphicsScene (this));
    ui->imageLogo2->setBackgroundBrush (palette().background());
    ui->imageLogo2->scene()->addPixmap (QPixmap (":/asset/logo_dograt.png"));

    // Set to fixed size
    adjustSize();
    setFixedSize (width (), height ());

}

//==========================================================================
// Destroyer
//==========================================================================
CDialogAbout::~CDialogAbout()
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
// Action Slot - on buttonClose clicked
//==========================================================================
void CDialogAbout::on_buttonClose_clicked()
{
    close();
}
