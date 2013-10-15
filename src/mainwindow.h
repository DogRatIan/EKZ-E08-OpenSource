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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

//==========================================================================
//==========================================================================
#include <QMainWindow>

#include "flashtable.h"
#include "model_databuffer.h"
#include "worker.h"

//==========================================================================
//==========================================================================
namespace Ui {
class CMainWindow;
}

class CMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit CMainWindow (QWidget *aParent = 0);
    ~CMainWindow ();

    QString portName;
    QString lastPortName;
    
public slots:
    void handleConfigCommResult (QString aPortName);
    void handleProgrammerInfo (struct TProgrammerInfo aInfo);
    void handleSelectDeviceResult (const struct TFlashInfo *aResult);
    void handleDataReady (const unsigned char *aData, unsigned long aDataSize);

private slots:
    void on_buttonOpenFile_clicked();

    void on_scrollDataBuffer_valueChanged(int value);

    void on_buttonClearBuffer_clicked();

    void on_ActionOpen_triggered();

    void on_buttonConfigComm_clicked();

    void on_ActionAbout_triggered();

    void on_buttonSelectDevice_clicked();

    void on_buttonRead_clicked();

    void on_buttonWrite_clicked();

    void on_buttonVerify_clicked();

    void on_buttonErase_clicked();

    void on_buttonDetect_clicked();

    void on_buttonSaveBuffer_clicked();

    void on_actionSave_as_triggered();

    void on_buttonProgrammerInfo_clicked();

private:
    Ui::CMainWindow *ui;

    CModelDataBuffer ModelDataBuffer;
    const struct TFlashInfo *currentFlash;
    struct TProgrammerInfo programmerInfo;

    QString lastDataPath;

    void saveConfig (void);
    void loadConfig (void);
    void openFile (void);
    void saveFile (void);
    void clearBuffer (void);
    void openDoAction (int aAction, const unsigned char *aPtr = NULL, unsigned long aSize = 0);

};

//==========================================================================
//==========================================================================
#endif // MAINWINDOW_H
