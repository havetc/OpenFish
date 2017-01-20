/**************************************************************************
**   corentin, 11/01/2017 2017

**This file is part of OpenFish.
**
**OpenFish is free software: you can redistribute it and/or modify
**it under the terms of the GNU General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**OpenFish is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU General Public License
**along with OpenFish.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "renderThread.hpp"
#include <iostream>
//#include "openwarp.h"

namespace Ui {
class MainWindow;
class GetSize;
}

/**
 * @brief The OUTPUT_TYPE enum
 * choice of output configuration possible
 */
enum OUTPUT_TYPE
{
    INPUT_SIZE = 1,
    INPUT_HEIGHT = 2,
    INPUT_WIDTH = 3,
    CUSTOM = 4
};

/**
 * @brief The GetSize class
 * It contains all the stuff related to the contextual window
 * asking for the size
 */
class GetSize : public QDialog
{
    Q_OBJECT

public:
    explicit GetSize(QWidget *parent = 0);
    ~GetSize();

    //ui should better be private, but its simpler to manage with public
    Ui::GetSize *ui;
};

/**
 * @brief The MainWindow class
 * It contains all the main windows stuff
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0, int argc = 0, char** argv = NULL);
    ~MainWindow();

public slots:

    void selectFile();
    void selectRes();
    void update();
    void setTime(int time);
    void endRender(bool withsound);
    void startRender();
    void errorPopUp();

private:
    Ui::MainWindow *ui;

    GetSize* GS_wnd;
    int argc;
    char** argv;

    ///complete absolute path of the input video
    QString vid;

    cv::Mat src;
    cv::VideoCapture inputvideo;
    cv::Size resSortie;
    QGraphicsScene *scn;
    QGraphicsPixmapItem *item;
    RenderThread * thr;
    OUTPUT_TYPE type;

};




#endif // MAINWINDOW_H
