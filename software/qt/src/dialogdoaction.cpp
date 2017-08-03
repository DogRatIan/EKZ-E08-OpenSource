//==========================================================================
// Dialog - Do Actions
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
#include <QTime>
#include <QDebug>

#include "dialogdoaction.h"

//==========================================================================
// Constructor
//==========================================================================
CDialogDoAction::CDialogDoAction (QWidget *aParent, int aAction) :
    QDialog (aParent),
    portName ("COM1"),
    flashInfo (NULL),
    ui(new Ui::CDialogDoAction),
    worker (aParent),
    action (aAction)
{
    QString str;

    // Load UI
    ui->setupUi (this);

    // Turn off close button
    Qt::WindowFlags flags = windowFlags ();
    flags &= (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setWindowFlags (flags);

    // Setup title and button
    switch (action)
    {
        case DOACTIONDIALOG_READ:
            setWindowTitle ("Read");
            setWindowIcon(QIcon (":/asset/read.png"));
            ui->buttonOk->setText ("Read");
            ui->buttonOk->setIcon(QIcon (":/asset/read.png"));
            ui->checkBox->setVisible (false);
            break;

        case DOACTIONDIALOG_WRITE:
            setWindowTitle ("Write");
            setWindowIcon(QIcon (":/asset/write.png"));
            ui->buttonOk->setText ("Write");
            ui->buttonOk->setIcon(QIcon (":/asset/write.png"));
            ui->checkBox->setText("Erase First");
            ui->checkBox->setVisible (true);
            break;

        case DOACTIONDIALOG_VERIFY:
            setWindowTitle ("Verify");
            setWindowIcon(QIcon (":/asset/verify.png"));
            ui->buttonOk->setText ("Verify");
            ui->buttonOk->setIcon(QIcon (":/asset/verify.png"));
            ui->checkBox->setVisible (false);
            break;

        case DOACTIONDIALOG_ERASE:
            setWindowTitle ("Erase");
            setWindowIcon(QIcon (":/asset/erase.png"));
            ui->buttonOk->setText ("Erase");
            ui->buttonOk->setIcon(QIcon (":/asset/erase.png"));
            ui->checkBox->setVisible (false);
            break;

        case DOACTIONDIALOG_DETECT:
            setWindowTitle ("Detect");
            setWindowIcon(QIcon (":/asset/detect.png"));
            ui->buttonOk->setText ("Detect");
            ui->buttonOk->setIcon(QIcon (":/asset/detect.png"));
            ui->checkBox->setVisible (false);
            break;

        default:
            ui->buttonOk->setVisible (false);
            break;
    }

    // Set to fixed size
    adjustSize();
    setFixedSize (width (), height ());

    // Setup worker
    connect (&worker, &CWorker::finished, this, &CDialogDoAction::handleWorkerFinished);
    connect (&worker, &CWorker::logMessage, this, &CDialogDoAction::handleWorkerLogMessage);
    connect (&worker, &CWorker::updateProgress, this, &CDialogDoAction::handleUpdateProgress);

    //
    if (action == DOACTIONDIALOG_DETECT)
    {
        str.sprintf ("%d devices found at table.", GetFlashTable()->deviceTable.size ());
        ui->listLog->addItem (str);
        ui->listLog->setCurrentRow(ui->listLog->count() - 1);
    }
}

//==========================================================================
// Destroyer
//==========================================================================
CDialogDoAction::~CDialogDoAction ()
{
    worker.requestAbort ();
    worker.wait ();

    disconnect (&worker, &CWorker::finished, this, &CDialogDoAction::handleWorkerFinished);
    disconnect (&worker, &CWorker::logMessage, this, &CDialogDoAction::handleWorkerLogMessage);
    disconnect (&worker, &CWorker::updateProgress, this, &CDialogDoAction::handleUpdateProgress);

    delete ui;
}

//==========================================================================
// Load data into buffer
//==========================================================================
void CDialogDoAction::loadDataBuffer (const unsigned char *aStr, unsigned long aSize)
{
    worker.loadDataBuffer (aStr, aSize);
}


//==========================================================================
// Action Slot - handle CWorker finished
//==========================================================================
void CDialogDoAction::handleWorkerFinished (void)
{
    setControls (false);

    if (worker.actionSuccess)
    {
        ui->buttonOk->setText ("OK");
        ui->buttonOk->setIcon(QIcon (":/asset/yes.png"));

        if ((action == DOACTIONDIALOG_ERASE)
                || (action == DOACTIONDIALOG_VERIFY)
                || (action == DOACTIONDIALOG_WRITE))
        {
            ui->buttonCancel->setEnabled (false);
        }
    }
    else
    {
        ui->buttonOk->setEnabled (false);
    }
}

//==========================================================================
// Action Slot - handle CWorker logMessage
//==========================================================================
void CDialogDoAction::handleWorkerLogMessage (QString aStr)
{
    ui->listLog->addItem (QString("[%1] %2").arg(QTime::currentTime().toString ("HH:mm:ss")).arg(aStr));
    ui->listLog->setCurrentRow(ui->listLog->count() - 1);
}

//==========================================================================
// Action Slot - handle Update Progress
//==========================================================================
void CDialogDoAction::handleUpdateProgress (int aValue)
{
    ui->progressBar->setValue (aValue);
}


//==========================================================================
// Action Slot - handle buttonCancel clicked
//==========================================================================
void CDialogDoAction::on_buttonCancel_clicked()
{
    close ();
}

//==========================================================================
// Action Slot - handle buttonOk clicked
//==========================================================================
void CDialogDoAction::on_buttonOk_clicked()
{
    // Exit if worker already success
    if (worker.actionSuccess)
    {
        //
        if (action == DOACTIONDIALOG_READ)
        {
            emit dataReady (worker.dataBuffer, worker.dataBufferSize);
        }

        if (action == DOACTIONDIALOG_DETECT)
        {
            emit resultDeviceInfo (worker.getFlashInfo ());
        }

        //
        close ();
        return;
    }

    // check worker running
    if (worker.isRunning())
        return;

    //
    setControls (true);

    //
    worker.setPortName (portName);

    switch (action)
    {
        case DOACTIONDIALOG_READ:
            worker.setFlashInfo (flashInfo);
            worker.setAction (WORKERACTION_READ);
            worker.start ();
            break;


        case DOACTIONDIALOG_WRITE:
            worker.setFlashInfo (flashInfo);
            worker.setAction (WORKERACTION_WRITE, ui->checkBox->isChecked());
            worker.start ();
            break;

        case DOACTIONDIALOG_VERIFY:
            worker.setFlashInfo (flashInfo);
            worker.setAction (WORKERACTION_VERIFY);
            worker.start ();
            break;

        case DOACTIONDIALOG_ERASE:
            worker.setFlashInfo (flashInfo);
            worker.setAction (WORKERACTION_ERASE);
            worker.start ();
            break;

        case DOACTIONDIALOG_DETECT:
            worker.setFlashInfo (&KGenericSpiFlash64KiB);
            worker.setAction (WORKERACTION_DETECT);
            worker.start ();
            break;

        default:
            ui->listLog->addItem (QString("ERROR. Invalid action %1").arg(action));
            ui->listLog->setCurrentRow(ui->listLog->count() - 1);
            setControls (false);
            ui->buttonOk->setEnabled (false);
            break;
    }

}

//==========================================================================
// Enable/Disable controls on running
//==========================================================================
void CDialogDoAction::setControls (bool aRunning)
{
    ui->buttonOk->setEnabled (!aRunning);
    ui->buttonCancel->setEnabled (!aRunning);
    ui->checkBox->setEnabled (!aRunning);

    if (aRunning)
        setCursor (Qt::WaitCursor);
    else
        setCursor (Qt::ArrowCursor);
}
