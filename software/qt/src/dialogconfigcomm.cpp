//==========================================================================
// Dialog - Config Communication
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
#include <QMessageBox>
#include <QtSerialPort/QSerialPortInfo>

#include "dialogconfigcomm.h"

//==========================================================================
// Constructor
//==========================================================================
CDialogConfigComm::CDialogConfigComm (QWidget *aParent) :
    QDialog (aParent),
    ui(new Ui::CDialogConfigComm),
    worker (aParent)
{
    // Load UI
    ui->setupUi (this);

    // Turn off close button
    Qt::WindowFlags flags = windowFlags ();
    flags &= (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setWindowFlags (flags);

    // Add serial port name to Combobox
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QString str;

        // Filter out ttyACM on linux
        if (QApplication::platformName() == "xcb")
        {
            if (!info.portName().startsWith("ttyACM"))
                continue;
        }

        // Form port name
        str = info.portName();
        if (info.description().length() > 0)
        {
            str.append (": ");
            str.append (info.description());
        }

        if (info.manufacturer().length() > 0)
        {
            if (info.manufacturer().startsWith("("))
                str.append (info.manufacturer());
            else
            {
                str.append (" (");
                str.append (info.manufacturer());
                str.append (")");
            }
        }

        ui->comboPort->addItem (str);
    }
    ui->comboPort->setCurrentIndex (-1);

    // Set to fixed size
    adjustSize();
    setFixedSize (width (), height ());

    // Setup worker
    connect (&worker, &CWorker::finished, this, &CDialogConfigComm::handleWorkerFinished);

}

//==========================================================================
// Destroyer
//==========================================================================
CDialogConfigComm::~CDialogConfigComm ()
{
    worker.requestAbort ();
    worker.wait ();

    delete ui;
}

//==========================================================================
// Set Port Name
//==========================================================================
void CDialogConfigComm::setPortName (QString aPortName)
{
    if (aPortName.length() > 0)
        ui->comboPort->setCurrentIndex (ui->comboPort->findText (aPortName,Qt::MatchStartsWith));

}

//==========================================================================
// Action Slot - on buttonOk clicked
//==========================================================================
void CDialogConfigComm::on_buttonOk_clicked()
{
    // Check no selection
    if (ui->comboPort->currentIndex() < 0)
    {
        close ();
        return;
    }

    // Get port name
    QString selected_port = ui->comboPort->currentText ();
    int i = selected_port.indexOf(":");
    if (i > 0)
        selected_port.remove(i, selected_port.length() - i);

    // check worker running
    if (worker.isRunning())
        return;

    //
    setControls (true);

    // Start worker
    worker.setPortName (selected_port);
    worker.setAction (WORKERACTION_CHECK_PROGRAMMER);
    worker.start ();

}

//==========================================================================
// Action Slot - on buttonCancel clicked
//==========================================================================
void CDialogConfigComm::on_buttonCancel_clicked()
{
    close ();
}

//==========================================================================
// Action Slot - handle CWorker finished
//==========================================================================
void CDialogConfigComm::handleWorkerFinished (void)
{
    setControls (false);

    if (strlen (worker.programmerInfo.name) > 0)
    {
        emit resultReady (worker.getPortName());
        emit resultProgrammerInfo (worker.programmerInfo);

        close ();
    }
    else
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle ("ERROR");
        msgbox.setText ("EKZ-E08 Programmer not found.");
        msgbox.setIcon (QMessageBox::Critical);
        msgbox.setStandardButtons (QMessageBox::Ok);
        msgbox.exec ();

//        emit resultReady (QString (""));
        close ();
    }
}

//==========================================================================
// Enable/Disable controls on running
//==========================================================================
void CDialogConfigComm::setControls (bool aRunning)
{
    ui->buttonOk->setEnabled (!aRunning);
    ui->buttonCancel->setEnabled (!aRunning);

    if (aRunning)
        setCursor (Qt::WaitCursor);
    else
        setCursor (Qt::ArrowCursor);
}

