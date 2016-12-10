//==========================================================================
// Dialog - Select Device
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
#include <list>

#include "dialogselectdevice.h"

//==========================================================================
// Constructor
//==========================================================================
CDialogSelectDevice::CDialogSelectDevice (QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDialogSelectDevice),
    selectedFlash (NULL)
{
    int i;
    CFlashTable *flash_table = GetFlashTable ();
    std::set<std::string>::iterator it;

    // Load UI
    ui->setupUi (this);

    // Turn off close button
    Qt::WindowFlags flags = windowFlags ();
    flags &= (~(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint));
    setWindowFlags (flags);

    // Setup combo Brand List
    ui->comboBrand->clear ();
    comboBrandReady = false;

    i = 0;
    for (it = flash_table->serialFlashBrand.begin(); it != flash_table->serialFlashBrand.end(); ++it)
    {
        const char *ptr = it->c_str ();
        ui->comboBrand->insertItem (i, QString(ptr));
        i ++;
    }
    ui->comboBrand->setCurrentIndex (-1);
    comboBrandReady = true;
    selectedFlash = NULL;
    updateFlashInfo ();

    // Set to fixed size
    adjustSize();
    setFixedSize (width (), height ());
}

//==========================================================================
// Destroyer
//==========================================================================
CDialogSelectDevice::~CDialogSelectDevice()
{
    delete ui;
}

//==========================================================================
// Set Flash Info
//==========================================================================
void CDialogSelectDevice::setFlashInfo (const struct TFlashInfo *aInfo)
{
    // No Info
    if (aInfo == NULL)
        return;

    // Set brand combo
    if (strlen (aInfo->brand))
    {
        ui->comboBrand->setCurrentIndex (ui->comboBrand->findText (QString (aInfo->brand), Qt::MatchStartsWith));
    }

    // Set device combo
    if (strlen (aInfo->partNumber))
    {
        ui->comboDevice->setCurrentIndex (ui->comboDevice->findText (QString (aInfo->partNumber), Qt::MatchStartsWith));
    }
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
// Update Flash Info
//==========================================================================
void CDialogSelectDevice::updateFlashInfo (void)
{
    if (selectedFlash == NULL)
    {
        ui->labelDeviceId->setText ("-");
        ui->labelDeviceSize->setText ("-");
    }
    else
    {
        QString str;

        str.sprintf ("%02lX %02lX %02lX", ((selectedFlash->id >> 16) & 0xff), ((selectedFlash->id >> 8) & 0xff), ((selectedFlash->id >> 0) & 0xff));
        ui->labelDeviceId->setText (str);

        str.sprintf ("%d KiB", selectedFlash->totalSizeKiB);
        ui->labelDeviceSize->setText (str);
    }
}

//==========================================================================
// Action Slot - on buttonOk clicked
//==========================================================================
void CDialogSelectDevice::on_buttonOk_clicked()
{
    emit resultDeviceInfo (selectedFlash);
    close();
}

//==========================================================================
// Action Slot - on buttonCancel clicked
//==========================================================================
void CDialogSelectDevice::on_buttonCancel_clicked()
{
    close();
}

//==========================================================================
// Action Slot - on comboBrand currentIndexChanged
//==========================================================================
void CDialogSelectDevice::on_comboBrand_currentIndexChanged(const QString &aStr)
{
    int i;
    CFlashTable *flash_table = GetFlashTable ();
    std::set<std::string>::iterator it;

    // ComboBox is under construction
    if (!comboBrandReady)
        return;

    qDebug () << QString ().sprintf ("[DEBUG] %s selected", aStr.toUtf8().data());

    // Clear device list combo
    ui->comboDevice->clear ();
    ui->comboDevice->setCurrentIndex (-1);
    comboDeviceReady = false;
    selectedFlash = NULL;
    updateFlashInfo ();

    // No Brand selected
    if (aStr.length() == 0)
        return;

    // Ask CFlashTable to update device list
    flash_table->generateSerialFlashList (aStr.toUtf8().data());

    // Setup Device List Combo
    i = 0;
    for (it = flash_table->serialFlashList.begin(); it != flash_table->serialFlashList.end(); ++it)
    {
        const char *ptr = it->c_str ();
        ui->comboDevice->insertItem (i, QString(ptr));
        i ++;
    }
    ui->comboDevice->setCurrentIndex (-1);
    comboDeviceReady = true;
}

//==========================================================================
// Action Slot - on comboDevice currentIndexChanged
//==========================================================================
void CDialogSelectDevice::on_comboDevice_currentIndexChanged(const QString &aStr)
{
    CFlashTable *flash_table = GetFlashTable ();
    QString selected_brand = ui->comboBrand->currentText();
    char brand[64];
    char device[64];

    // ComboBox is under construction
    if (!comboDeviceReady)
        return;

    // Clear last selection
    selectedFlash = NULL;
    updateFlashInfo ();

    // No Device select
    if ((aStr.length() == 0) || (selected_brand.length() == 0))
        return;

    // Convert and save selection to buffer
    qstrncpy (brand, selected_brand.toUtf8().data(), sizeof (brand));
    qstrncpy (device, aStr.toUtf8().data(), sizeof (device));

    qDebug () << QString ().sprintf ("[DEBUG] %s %s selected", brand, device);

    selectedFlash = flash_table->getSerialFlash (brand, device);

    updateFlashInfo ();
}
