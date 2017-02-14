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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_getsize.h"
#include "openwarp.h"
#include "asmOpenCV.h"
#include <QMessageBox>
#if QT_DEPRECATED_SINCE(5, 0)
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include <QGraphicsPixmapItem>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <QThread>
#include <QWheelEvent>
#include <QTranslator>


GetSize::GetSize(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GetSize)
{
    ui->setupUi(this);
}

GetSize::~GetSize()
{
    delete ui;
}

MainWindow::MainWindow(QWidget *parent, int argc, char **argv) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), GS_wnd(new GetSize(this))
{

    ui->setupUi(this);
    this->argc = argc;
    this->argv = argv;
    this->scn = new QGraphicsScene();
    this->item = new QGraphicsPixmapItem();
    this->scn->addItem(item);
    this->ui->graphicsView->setScene(scn);
    this->thr = NULL;
    this->ui->progressBar->setVisible(false);
    this->type = INPUT_SIZE;

    QObject::connect(scn, SIGNAL(selectionChanged()), ui->graphicsView, SLOT(update()) );
    QObject::connect(ui->actionAction_1, SIGNAL(triggered(bool)),this,SLOT(selectFile()) );
    QObject::connect(ui->pushButton, SIGNAL(clicked(bool)),this,SLOT(startRender()) );
    QObject::connect(ui->verticalSliderHaut,SIGNAL(valueChanged(int)),this,SLOT(update()) );
    QObject::connect(ui->verticalSliderFov,SIGNAL(valueChanged(int)),this,SLOT(update()) );
    QObject::connect(ui->verticalSliderZoom,SIGNAL(valueChanged(int)),this,SLOT(update()) );
    QObject::connect(ui->actionR_solution, SIGNAL(triggered(bool)), this->GS_wnd, SLOT(show()));
    QObject::connect(this->GS_wnd, SIGNAL(accepted()), this, SLOT(selectRes()));
    QObject::connect(ui->horizontalSliderTime, SIGNAL(valueChanged(int)), this, SLOT(setTime(int)));
    QObject::connect(ui->framesSlider, SIGNAL(valueChanged(int)), this, SLOT(update()) );
    QObject::connect(ui->offsetSlider, SIGNAL(valueChanged(int)), this, SLOT(update()) );

    QObject::connect(ui->setEn, SIGNAL(triggered(bool)), this, SLOT(setEN()));
    QObject::connect(ui->setFR, SIGNAL(triggered(bool)), this, SLOT(setFR()));
    QObject::connect(ui->setRu, SIGNAL(triggered(bool)), this, SLOT(setRU()));

    QString defaultlocale = QLocale::system().name().section('_', 0, 0);
    this->resetTranslator(defaultlocale);
}

MainWindow::~MainWindow()
{
    if (thr != NULL)
        delete thr;

    delete GS_wnd;
    delete item;
    delete scn;
    delete ui;
}

void MainWindow::selectFile()
{
#if QT_DEPRECATED_SINCE(5, 0)
    QString tempfilename = QFileDialog::getOpenFileName(this,tr("Sélectionner la vidéo à traiter"),QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
#else
    QString tempfilename = QFileDialog::getOpenFileName(this,tr("Sélectionner la vidéo à traiter"),QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
#endif

    QFile file(tempfilename);
    if (!file.exists()){
        QMessageBox::warning(this, tr("Attention"), tr("Le fichier: ")+ this->vid + tr(" n'exite pas"));
    } else {
        this->inputvideo = getInputVideo(tempfilename);
        if(this->inputvideo.isOpened()) {
            //we change the attribute of the class only if the new filename is valid, otherwise we keep the old one
            this->vid = tempfilename;
            QMessageBox::information(this, tr("Vidéo Ouverte"), this->vid);
            ui->verticalSliderHaut->setEnabled(true);
            ui->verticalSliderZoom->setEnabled(true);
            ui->verticalSliderFov->setEnabled(true);
            ui->horizontalSliderTime->setEnabled(true);
            ui->pushButton->setEnabled(true);
            this->inputvideo.set(CV_CAP_PROP_POS_FRAMES, 10);
            inputvideo >> src;              // read
            //defini size
            selectRes();
            QPixmap dem = draft(src, this->resSortie, 45, 100, 100, 2, 0);
            inputvideo.set(CV_CAP_PROP_POS_FRAMES, 0);
            item->setPixmap(dem);
            //ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
            ui->graphicsView->update();
            this->resize(resSortie.width + 300, resSortie.height + 100);
            this->move(0,0);
            //this->adjustSize();
            //ui->graphicsView->show();
        } else {
            QMessageBox::critical(this, tr("Attention"), tr("Le fichier: ")+tempfilename+tr(" n'est pas un fichier vidéo valide") );
            //if the new file was invalid, we reload the old one
            this->inputvideo = getInputVideo(this->vid);
        }
    }
}

void MainWindow::selectRes()
{
    if(this->GS_wnd->ui->radioButtonEntree->isChecked()) {
        type = INPUT_SIZE;
        if (inputvideo.isOpened()) {
            resSortie = src.size();
        }
    } else if (this->GS_wnd->ui->radioButtonHeigth->isChecked()) {
        type = INPUT_HEIGHT;
        if (inputvideo.isOpened()) {
            resSortie = Size(src.size().height, src.size().height);
        }
    } else if (this->GS_wnd->ui->radioButtonWidth->isChecked()) {
        type = INPUT_WIDTH;
        if (inputvideo.isOpened()) {
            resSortie = Size(src.size().width, src.size().width);
        }
    } else if (this->GS_wnd->ui->radioButtonCustom->isChecked()) {
        type = CUSTOM;
        int height = this->GS_wnd->ui->spinBoxHeigth->value();
        int width = this->GS_wnd->ui->spinBoxWidth->value();
        this->resSortie = cv::Size(width, height);
    } else {
        QMessageBox::critical(this,tr("error"), tr("output resolution non determined"));
    }

    if (inputvideo.isOpened()) {
        update();
    }
}

void MainWindow::update()
{
    QPixmap dem = draft(src, resSortie, this->ui->verticalSliderHaut->value(),
                        this->ui->verticalSliderZoom->value(), this->ui->verticalSliderFov->value(),
                        this->ui->framesSlider->value(), this->ui->offsetSlider->value());
    this->scn->clear();
    this->item = new QGraphicsPixmapItem();
    this->item->setPixmap(dem);
    this->scn->addItem(this->item);
    //ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
    ui->graphicsView->setScene(NULL);
    ui->graphicsView->setScene(this->scn);
    //ui->graphicsView->adjustSize();
    //ui->graphicsView->repaint();
    ui->graphicsView->show();
}

void MainWindow::setTime(int time)
{
    try{
        double nb = inputvideo.get(CV_CAP_PROP_FRAME_COUNT);
        inputvideo.set(CV_CAP_PROP_POS_FRAMES, int(time*nb/(1000)));
        inputvideo >> src; // read a frame
        if (inputvideo.isOpened()) {
            update();
        }
    }catch(Exception e){
        //if there is no video, do nothing
    }

}

void MainWindow::endRender(bool withsound)
{
    this->ui->progressBar->setVisible(false);
    this->ui->horizontalSliderTime->setEnabled(true);
    this->ui->menuTest->setEnabled(true);
    this->ui->menuOptions->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->inputvideo.set(CV_CAP_PROP_POS_FRAMES, 0);
    if(withsound){
        QMessageBox::information(this, tr("Info"), tr("Conversion terminée"));
    } else {
        QMessageBox::information(this, tr("Info"), tr("Conversion terminée, sans son"));
    }
    delete this->thr;

}

void MainWindow::startRender()
{
    this->ui->menuTest->setEnabled(false);
    this->ui->horizontalSliderTime->setEnabled(false);
    this->ui->menuOptions->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    //keep the program directory inside "prog"
    QDir path (QCoreApplication::applicationDirPath());
    std::cout << "Path: " << path.absolutePath().toStdString() << std::endl;
    char shaut[100];
    char szoom[100];
    char sfov[100];
    sprintf(shaut, "%d", this->ui->verticalSliderHaut->value());
    sprintf(szoom, "%d", this->ui->verticalSliderZoom->value());
    sprintf(sfov, "%d", this->ui->verticalSliderFov->value());
    Size S = Size((int)this->inputvideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
        (int)inputvideo.get(CV_CAP_PROP_FRAME_HEIGHT));
    std::wstring::size_type pAt = vid.toStdString().find_last_of('.');                  // Find extension point

    // Form the new name with container
    const QString NAME = QString::fromStdString(vid.toStdString().substr(0, pAt).append("H"+std::string(shaut)+"Z"+std::string(szoom)+"F"+std::string(sfov)+".avi") );

    this->ui->progressBar->setVisible(true);

    int fcount = this->inputvideo.get(CV_CAP_PROP_FRAME_COUNT);

    this->ui->progressBar->setValue(0);
    this->ui->progressBar->setMaximum(fcount);

    thr = new RenderThread(this->ui->actionAvec_audio->isChecked(),
                this->ui->verticalSliderHaut->value(),this->ui->verticalSliderZoom->value() / 100.0,this->ui->verticalSliderFov->value(),
                           this->ui->framesSlider->value(), this->ui->offsetSlider->value(),
                           this->vid, this->inputvideo, this->resSortie, S, NAME, path);

    QObject::connect(this->thr, SIGNAL(update(int)),this->ui->progressBar,SLOT(setValue(int)));
    QObject::connect(this->thr, SIGNAL(end(bool)), this, SLOT(endRender(bool)));
    QObject::connect(this->thr, SIGNAL(sound_error()), this, SLOT(errorPopUp()));

    thr->start();

}

void MainWindow::errorPopUp()
{
    QMessageBox::warning(NULL,tr("Problème son"),tr("Erreur de ffmpeg, le fichier est convertit mais sans son"));
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange)
    {
        std::cout << "langage change" << std::endl;
        ui->retranslateUi(this);
        this->GS_wnd->ui->retranslateUi(this->GS_wnd);
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::resetTranslator(QString locale){
    static QTranslator translator;
    QCoreApplication::removeTranslator(&translator);
    translator.load(QString("OpenFish_") + locale, QString("lang"));
    QCoreApplication::installTranslator(&translator);
}

void MainWindow::setFR()
{
    this->resetTranslator("fr");
}

void MainWindow::setEN()
{
    this->resetTranslator("en");
}

void MainWindow::setRU()
{
    this->resetTranslator("ru");
}

QPixmap draft(Mat image, Size & output, int hauteur, int zoom, int fovChange, int frames, int offset ) {
    //zoom en pourcentage
    Mat mx, my, mapx, mapy, res;
    Size s = image.size();
    Size outputsmall = output;
    outputsmall.height /= 10;
    outputsmall.width /= 10;

    create_map(mapx, mapy, s, outputsmall, hauteur, zoom/100.0, fovChange, frames, offset);
    resize(mapx, mx, output);
    resize(mapy, my, output);
    remap(image, res, mx, my, INTER_LINEAR);// , BORDER_WRAP);
    //imshow("Open Warp", res);
    return ASM::cvMatToQPixmap(res);

}
