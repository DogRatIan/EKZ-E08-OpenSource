//==========================================================================
// Main Window
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
#include <QtXml>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "mainwindow.h"
#include "dialogabout.h"
#include "dialogconfigcomm.h"
#include "dialogselectdevice.h"
#include "dialogdoaction.h"
#include "dialogproginfo.h"
#include "flashtable.h"
#include "ui_mainwindow.h"

//==========================================================================
// Defines
//==========================================================================
#define VALUE_TABLE_COL_WIDTH   28
#define VALUE_TABLE_ROW_HEIGHT  22
#define VALUE_TABLE_ADDR_WIDTH  70

#define CONFIG_FILENAME "ekz-e08.conf"

//==========================================================================
// Constructor
//==========================================================================
CMainWindow::CMainWindow (QWidget *aParent) :
    QMainWindow (aParent),
    ui(new Ui::CMainWindow),
    ModelDataBuffer (this, GetFlashTable()->getMaximumSize()),
    currentFlash (NULL)
{
    int i;
    int w;
    int h;

    // Load UI
    ui->setupUi (this);

    // Setup Menu Action
    connect (ui->ActionExit, SIGNAL(triggered()), this, SLOT(close()));

    // Set Table col/row size
    ui->tableDataBuffer->setModel (&ModelDataBuffer);
    for (i = 0; i < ModelDataBuffer.columnCount (); i ++)
        ui->tableDataBuffer->setColumnWidth (i, VALUE_TABLE_COL_WIDTH);
    for (i = 0; i < ModelDataBuffer.rowCount (); i ++)
        ui->tableDataBuffer->setRowHeight (i, VALUE_TABLE_ROW_HEIGHT);

    // Set Header size
    ui->tableDataBuffer->horizontalHeader()->setSectionResizeMode (QHeaderView::Fixed);
    ui->tableDataBuffer->verticalHeader()->setSectionResizeMode (QHeaderView::Fixed);
    ui->tableDataBuffer->verticalHeader()->setFixedWidth (VALUE_TABLE_ADDR_WIDTH);

    // Set table overall size
    w = ui->tableDataBuffer->verticalHeader()->width() + ModelDataBuffer.columnCount () * VALUE_TABLE_COL_WIDTH + 5;
    h = ui->tableDataBuffer->horizontalHeader()->height() + ModelDataBuffer.rowCount () * VALUE_TABLE_ROW_HEIGHT + 5;

    ui->tableDataBuffer->setFixedSize (w, h);
    ui->scrollDataBuffer->setMinimum (0);
    ui->scrollDataBuffer->setMaximum (0);
    ui->scrollDataBuffer->setPageStep (16);

    // Set Icons
    ui->imageProgrammer->setScene (new QGraphicsScene (this));
    ui->imageProgrammer->setBackgroundBrush (palette().background());
    ui->imageProgrammer->scene()->addPixmap (QPixmap (":/asset/zif16.png"));

    ui->imageLogo1->setScene (new QGraphicsScene (this));
    ui->imageLogo1->setBackgroundBrush (palette().background());
    ui->imageLogo1->scene()->addPixmap (QPixmap (":/asset/logo_ekz.png"));

    ui->imageLogo2->setScene (new QGraphicsScene (this));
    ui->imageLogo2->setBackgroundBrush (palette().background());
    ui->imageLogo2->scene()->addPixmap (QPixmap (":/asset/logo_dograt.png"));

    //
//    ui->labelProgrammerSn->setText ("-\n-");

    QFont font = ui->labelHashResult->font();
//    if (QApplication::platformName() == "windows")
//        font.setPointSize (ui->labelHashResult->fontInfo().pointSize() - 1);
//    else if (QApplication::platformName() == "cocoa")
//        font.setPointSize (ui->labelHashResult->fontInfo().pointSize() - 2);
//    else if (QApplication::platformName() == "xcb")
        font.setPointSize (ui->labelHashResult->fontInfo().pointSize() - 1);

    ui->labelHashResult->setFont(font);

    // Set whole windows to fixed size
    adjustSize();
    setFixedSize (width (), height ());

    // Clear data buffer
    clearBuffer ();

    //
    loadConfig ();

}

//==========================================================================
// Destroyer
//==========================================================================
CMainWindow::~CMainWindow()
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
// Save Config
//==========================================================================
void CMainWindow::saveConfig (void)
{
    QFileInfo config_file (QApplication::applicationDirPath().append(QDir::separator()).append(CONFIG_FILENAME));

    // Form the DOM
    QDomDocument doc;
    QDomElement root = doc.createElement ("ekz-e08");
    QDomElement settings = doc.createElement ("settings");

    settings.setAttribute ("Port", lastPortName);
    settings.setAttribute ("LastPath", lastDataPath);

    if (currentFlash != NULL)
    {
        settings.setAttribute ("Brand", currentFlash->brand);
        settings.setAttribute ("Device", currentFlash->partNumber);
    }

    root.appendChild (settings);
    doc.appendChild (root);

    // Save to file
    QFile file (config_file.absoluteFilePath());
    if (file.open (QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QString str = doc.toString ();
        file.write (str.toUtf8());
        file.close ();
    }
}

//==========================================================================
// Load Config
//==========================================================================
void CMainWindow::loadConfig (void)
{
    CFlashTable *flash_table = GetFlashTable();

    // Load Config file
    QFileInfo config_file (QApplication::applicationDirPath().append(QDir::separator()).append(CONFIG_FILENAME));

    if (config_file.exists())
    {
        qDebug() << "Load config file: " << config_file.absoluteFilePath();

        QDomDocument doc;
        QFile file (config_file.absoluteFilePath());
        if (file.open(QIODevice::ReadOnly))
        {
            doc.setContent (&file);
            file.close();

            QDomElement root = doc.firstChildElement ("ekz-e08");
            QDomElement settings = root.firstChildElement ("settings");
            if (!settings.isNull())
            {
                lastPortName = settings.attribute("Port");

                currentFlash = flash_table->getSerialFlash (settings.attribute("Brand").toUtf8().data(),
                                                                      settings.attribute("Device").toUtf8().data());

                if (currentFlash != NULL)
                {
                    QString str;

                    ui->labelDeviceBrand->setText (currentFlash->brand);
                    ui->labelDeviceName->setText (currentFlash->partNumber);


                    str.sprintf ("%02lX %02lX %02lX", ((currentFlash->id >> 16) & 0xff), ((currentFlash->id >> 8) & 0xff), ((currentFlash->id >> 0) & 0xff));
                    ui->labelDeviceId->setText (str);

                    str.sprintf ("%d KiB", currentFlash->totalSizeKiB);
                    ui->labelDeviceSize->setText (str);
                }

                lastDataPath = settings.attribute("LastPath");
            }
        }
    }
}

//==========================================================================
// Open File
//==========================================================================
void CMainWindow::openFile (void)
{
    QString str;
    QString filename;
    QFileInfo file_info;
    unsigned long size;
    QString default_path;

    // Set Dialog default path
    if (lastDataPath.length() == 0)
        default_path = QCoreApplication::applicationDirPath();
    else
        default_path = lastDataPath;

    // Prompt user to select binary file
    const QFileDialog::Options options = 0;
    QString selectedFilter;
    filename = QFileDialog::getOpenFileName (this, tr("Open File"),
                                default_path,
                                tr("Binary Files (*.bin);;All Files (*)"),
                                &selectedFilter,
                                options);
    if (filename == NULL)
        return;
    file_info.setFile (filename);

    // Clear last file info on UI
    ui->labelDataFile->setText ("-");
    ui->labelDataSize->setText ("-");

    // Load file at Model
    str = ModelDataBuffer.loadDataFile (filename);
    if (str.length())
    {
        QMessageBox::warning (this, tr ("ERROR"), str, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }
    size = ModelDataBuffer.getBufferSize ();

    //
    ui->labelDataFile->setText (file_info.fileName());
    ui->labelDataSize->setText (QString ().sprintf ("%ld (0x000000- 0x%06lX)", size, size - 1));
    ui->tableDataBuffer->clearSelection();
    ui->labelHashResult->setText (QString("   MD5: %1\n  SHA1: %2\nSHA256: %3\nCRC32b: %4")
                                  .arg(ModelDataBuffer.hashResultMd5)
                                  .arg(ModelDataBuffer.hashResultSha1)
                                  .arg(ModelDataBuffer.hashResultSha256)
                                  .arg(ModelDataBuffer.hashResultCrc32b));

    // Setup Scrollbar
    ui->scrollDataBuffer->setValue (0);
    ui->scrollDataBuffer->setMinimum (0);
    ui->scrollDataBuffer->setMaximum (size / 16);
    ui->scrollDataBuffer->setPageStep (16);

    //
    lastDataPath = file_info.absoluteFilePath ();
    saveConfig ();
}

//==========================================================================
// Save File
//==========================================================================
void CMainWindow::saveFile (void)
{
    QString str;
    QString filename;
    QFileInfo file_info;
    QString default_path;

    // Set Dialog default path
    if (lastDataPath.length() == 0)
        default_path = QCoreApplication::applicationDirPath();
    else
        default_path = lastDataPath;

    // Check Data buffer size
    if (ModelDataBuffer.getBufferSize () == 0)
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle ("ERROR");
        msgbox.setText ("Data buffer is empty.");
        msgbox.setIcon (QMessageBox::Critical);
        msgbox.setStandardButtons (QMessageBox::Ok);
        msgbox.exec ();
        return;
    }

    // Prompt user to select binary file
    const QFileDialog::Options options = 0;
    QString selectedFilter;
    filename = QFileDialog::getSaveFileName (this, tr("Save File"),
                                default_path,
                                tr("Binary Files (*.bin);;All Files (*)"),
                                &selectedFilter,
                                options);
    if (filename == NULL)
        return;
    file_info.setFile (filename);


    // Save file at Model
    str = ModelDataBuffer.saveDataFile (filename);
    if (str.length())
    {
        QMessageBox::warning (this, tr ("ERROR"), str, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    //
    ui->labelDataFile->setText (file_info.fileName());
    ui->tableDataBuffer->clearSelection();

    //
    lastDataPath = file_info.absoluteFilePath ();
    saveConfig ();
}

//==========================================================================
// Clear Buffer
//==========================================================================
void CMainWindow::clearBuffer (void)
{
    ModelDataBuffer.clearBuffer ();
    ui->labelDataFile->clear ();
    ui->labelDataSize->clear ();
    ui->scrollDataBuffer->setValue (0);
    ui->scrollDataBuffer->setMinimum (0);
    ui->scrollDataBuffer->setMaximum (0);

    //
    ui->labelHashResult->setText (QString("   MD5: \n  SHA1: \nSHA256: \nCRC32b: "));

}

//==========================================================================
// Clear Buffer
//==========================================================================
void CMainWindow::openDoAction (int aAction, const unsigned char *aPtr, unsigned long aSize)
{
    CDialogDoAction dialog (this, aAction);
    QString err = "";

    do
    {
        if (portName.length() == 0)
        {
            err = "No communication port selected.";
            break;
        }

        if (aAction == DOACTIONDIALOG_DETECT)
            break;

        if (currentFlash == NULL)
        {
            err = "No device selected.";
            break;
        }
        if (currentFlash->id == 0)
        {
            err = "No device selected.";
            break;
        }

        if (aSize)
        {
            if (aSize > ((unsigned long)currentFlash->totalSizeKiB * 1024))
            {
                err = "Buffer size larger than FLASH size.";
                break;
            }
        }

    } while (0);

    if (err.length())
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle ("ERROR");
        msgbox.setText (err);
        msgbox.setIcon (QMessageBox::Critical);
        msgbox.setStandardButtons (QMessageBox::Ok);
        msgbox.exec ();
    }
    else
    {
        dialog.portName = portName;
        dialog.flashInfo = currentFlash;

        if ((aPtr != NULL) && (aSize ))
        {
            dialog.loadDataBuffer (aPtr, aSize);
        }

        connect (&dialog, &CDialogDoAction::dataReady, this, &CMainWindow::handleDataReady);
        connect (&dialog, &CDialogDoAction::resultDeviceInfo, this, &CMainWindow::handleSelectDeviceResult);
        dialog.exec ();
        disconnect (&dialog, &CDialogDoAction::dataReady, this, &CMainWindow::handleDataReady);
        disconnect (&dialog, &CDialogDoAction::resultDeviceInfo, this, &CMainWindow::handleSelectDeviceResult);
    }
}

//==========================================================================
// Action Slot - on scrollDataBuffer changed
//==========================================================================
void CMainWindow::on_scrollDataBuffer_valueChanged(int aValue)
{
    ModelDataBuffer.scroll (aValue * 16);
}

//==========================================================================
// Action Slot - on buttonClearBuffer clicked
//==========================================================================
void CMainWindow::on_buttonClearBuffer_clicked()
{
    clearBuffer ();
}

//==========================================================================
// Action Slot - on buttonConfigComm clicked
//==========================================================================
void CMainWindow::on_buttonConfigComm_clicked()
{
    CDialogConfigComm dialog (this);

    dialog.setPortName (lastPortName);

    connect (&dialog, &CDialogConfigComm::resultReady, this, &CMainWindow::handleConfigCommResult);
    connect (&dialog, &CDialogConfigComm::resultProgrammerInfo, this, &CMainWindow::handleProgrammerInfo);
    dialog.exec ();
    disconnect (&dialog, &CDialogConfigComm::resultReady, this, &CMainWindow::handleConfigCommResult);
    disconnect (&dialog, &CDialogConfigComm::resultProgrammerInfo, this, &CMainWindow::handleProgrammerInfo);
}

//==========================================================================
// Action Slot - on buttonSelectDevice clicked
//==========================================================================
void CMainWindow::on_buttonSelectDevice_clicked()
{
    CDialogSelectDevice dialog (this);

    dialog.setFlashInfo (currentFlash);

    connect (&dialog, &CDialogSelectDevice::resultDeviceInfo, this, &CMainWindow::handleSelectDeviceResult);
    dialog.exec ();
    disconnect (&dialog, &CDialogSelectDevice::resultDeviceInfo, this, &CMainWindow::handleSelectDeviceResult);
}

//==========================================================================
// Action Slot - on buttonProgrammerInfo clicked
//==========================================================================
void CMainWindow::on_buttonProgrammerInfo_clicked()
{
    CDialogProgInfo dialog (this, &programmerInfo);

    dialog.exec ();
}


//==========================================================================
// Action Slot - on buttonRead clicked
//==========================================================================
void CMainWindow::on_buttonRead_clicked()
{
    openDoAction (DOACTIONDIALOG_READ);
}

//==========================================================================
// Action Slot - on buttonWrite clicked
//==========================================================================
void CMainWindow::on_buttonWrite_clicked()
{
    // Check Data buffer size
    if (ModelDataBuffer.getBufferSize () == 0)
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle ("ERROR");
        msgbox.setText ("Data buffer is empty.");
        msgbox.setIcon (QMessageBox::Critical);
        msgbox.setStandardButtons (QMessageBox::Ok);
        msgbox.exec ();
        return;
    }

    openDoAction (DOACTIONDIALOG_WRITE, ModelDataBuffer.getBufferPointer(), ModelDataBuffer.getBufferSize());
}

//==========================================================================
// Action Slot - on buttonVerify clicked
//==========================================================================
void CMainWindow::on_buttonVerify_clicked()
{
    // Check Data buffer size
    if (ModelDataBuffer.getBufferSize () == 0)
    {
        QMessageBox msgbox;

        msgbox.setWindowTitle ("ERROR");
        msgbox.setText ("Data buffer is empty.");
        msgbox.setIcon (QMessageBox::Critical);
        msgbox.setStandardButtons (QMessageBox::Ok);
        msgbox.exec ();
        return;
    }

    openDoAction (DOACTIONDIALOG_VERIFY, ModelDataBuffer.getBufferPointer(), ModelDataBuffer.getBufferSize());
}

//==========================================================================
// Action Slot - on buttonErase clicked
//==========================================================================
void CMainWindow::on_buttonErase_clicked()
{
    openDoAction (DOACTIONDIALOG_ERASE);
}

//==========================================================================
// Action Slot - on buttonDetect clicked
//==========================================================================
void CMainWindow::on_buttonDetect_clicked()
{
    openDoAction (DOACTIONDIALOG_DETECT);
}

//==========================================================================
// Action Slot - on ActionAbout triggered
//==========================================================================
void CMainWindow::on_ActionAbout_triggered()
{
    CDialogAbout dialog (this);

    dialog.exec ();
}

//==========================================================================
// Action Slot - handle CDialogConfigComm result
//==========================================================================
void CMainWindow::handleConfigCommResult(QString aPortName)
{
    ui->labelCommPort->setText (aPortName);
    portName = aPortName;
    lastPortName = portName;

    //
    saveConfig();
}

//==========================================================================
// Action Slot - handle CDialogConfigComm ProgrammerInfo
//==========================================================================
void CMainWindow::handleProgrammerInfo (struct TProgrammerInfo aInfo)
{
    memcpy (&programmerInfo, &aInfo, sizeof (programmerInfo));

//    QString str_version;
//    QString str_sn;
//    QString str;
//    int i;

//    str_version.sprintf ("%d.%02d", (aInfo.version >> 8) & 0xff, (aInfo.version >> 0) & 0xff);

//    for (i = 0; i < (int) sizeof (aInfo.chipId); i ++)
//    {
//        if ((i) && ((i & 0x7) == 0))
//            str.sprintf ("\n%02X", aInfo.chipId[i]);
//        else
//            str.sprintf ("%02X", aInfo.chipId[i]);
//        str_sn.append (str);
//    }

    ui->labelProgrammerModel->setText (aInfo.name);

    if (qstrlen (aInfo.name) > 0)
        ui->buttonProgrammerInfo->setEnabled (true);
    else
        ui->buttonProgrammerInfo->setEnabled (false);
//    ui->labelProgrammerFirmware->setText (str_version);
//    ui->labelProgrammerSn->setText (str_sn);
}

//==========================================================================
// Action Slot - handle CDialogSelectDevice result
//==========================================================================
void CMainWindow::handleSelectDeviceResult (const struct TFlashInfo *aResult)
{
    currentFlash = aResult;

    if (currentFlash == NULL)
    {
        ui->labelDeviceBrand->setText ("-");
        ui->labelDeviceId->setText ("-");
        ui->labelDeviceName->setText ("-");
        ui->labelDeviceSize->setText ("-");
    }
    else
    {
        QString str;

        ui->labelDeviceBrand->setText (currentFlash->brand);
        ui->labelDeviceName->setText (currentFlash->partNumber);


        str.sprintf ("%02lX %02lX %02lX", ((currentFlash->id >> 16) & 0xff), ((currentFlash->id >> 8) & 0xff), ((currentFlash->id >> 0) & 0xff));
        ui->labelDeviceId->setText (str);

        str.sprintf ("%d KiB", currentFlash->totalSizeKiB);
        ui->labelDeviceSize->setText (str);

    }

    //
    saveConfig();
}

//==========================================================================
// Action Slot - handle Data Ready
//==========================================================================
void CMainWindow::handleDataReady (const unsigned char *aData, unsigned long aDataSize)
{
    QString str;
    unsigned long size;

    qDebug () << QString ().sprintf ("Data Size=%ld", aDataSize);

    //
    if (aData == NULL)
        return;

    // Clear last file info on UI
    ui->labelDataFile->setText ("-");
    ui->labelDataSize->setText ("-");

    // Load file at Model
    if ((str = ModelDataBuffer.loadData (aData, aDataSize)) != NULL)
    {
        QMessageBox::warning (this, tr ("ERROR"), str, QMessageBox::Ok, QMessageBox::NoButton);
        return;
    }

    size = ModelDataBuffer.getBufferSize ();

    //
    ui->labelDataSize->setText (QString ().sprintf ("%ld (0x000000- 0x%06lX)", size, size - 1));
    ui->tableDataBuffer->clearSelection();
    ui->labelHashResult->setText (QString("   MD5: %1\n  SHA1: %2\nSHA256: %3\nCRC32b: %4")
                                  .arg(ModelDataBuffer.hashResultMd5)
                                  .arg(ModelDataBuffer.hashResultSha1)
                                  .arg(ModelDataBuffer.hashResultSha256)
                                  .arg(ModelDataBuffer.hashResultCrc32b));

    // Setup Scrollbar
    ui->scrollDataBuffer->setValue (0);
    ui->scrollDataBuffer->setMinimum (0);
    ui->scrollDataBuffer->setMaximum (size / 16);
    ui->scrollDataBuffer->setPageStep (16);

}


//==========================================================================
// Action Slot - on buttonOpenFile clicked
//==========================================================================
void CMainWindow::on_buttonOpenFile_clicked()
{
    openFile ();
}

//==========================================================================
// Action Slot - on buttonSaveBuffer clicked
//==========================================================================
void CMainWindow::on_buttonSaveBuffer_clicked()
{
    saveFile ();
}

//==========================================================================
// Menu - File/Open
//==========================================================================
void CMainWindow::on_ActionOpen_triggered()
{
    openFile ();
}


//==========================================================================
// Menu - File/Save As
//==========================================================================
void CMainWindow::on_actionSave_as_triggered()
{
    saveFile ();
}

