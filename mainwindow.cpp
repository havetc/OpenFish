#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_getsize.h"
#include "openwarp.h"
#include "asmOpenCV.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <QThread>
#include <QWheelEvent>


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
// implementation d'un zoom molette, peu convaincant
/*
void MainWindow::wheelEvent(QWheelEvent *event){

        ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        // Scale the view / do the zoom
        double scaleFactor = 1.15;
        if(event->delta() > 0) {
            // Zoom in
            ui->graphicsView-> scale(scaleFactor, scaleFactor);

        } else {
            // Zooming out
             ui->graphicsView->scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }


        //ui->graphicsView->setTransform(QTransform(h11, h12, h21, h22, 0, 0));
}
*/

void MainWindow::selectFile()
{
    QString tempfilename = QFileDialog::getOpenFileName(this,QString("Sélectionner la vidéo à traiter"),QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));


    QFile file(tempfilename);
    if (!file.exists()){
        QMessageBox::warning(this, QString("Attention"), QString("Le fichier: "+this->vid+" n'exite pas"));
    } else {
        this->inputvideo = getInputVideo(tempfilename);
        if(this->inputvideo.isOpened()) {
            //we change the attribute of the class only if the new filename is valid, otherwise we keep the old one
            this->vid = tempfilename;
            QMessageBox::information(this, QString("Vidéo Ouverte"), this->vid);
            ui->verticalSliderHaut->setEnabled(true);
            ui->verticalSliderZoom->setEnabled(true);
            ui->verticalSliderFov->setEnabled(true);
            ui->pushButton->setEnabled(true);
            this->inputvideo.set(CAP_PROP_POS_FRAMES, 10);
            inputvideo >> src;              // read
            //defini size
            selectRes();
            QPixmap dem = draft(src, this->resSortie, 45, 100, 100);
            inputvideo.set(CAP_PROP_POS_FRAMES, 0);
            item->setPixmap(dem);
            //ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
            ui->graphicsView->update();
            this->resize(resSortie.width + 300, resSortie.height + 100);
            this->move(0,0);
            //this->adjustSize();
            //ui->graphicsView->show();
        } else {
            QMessageBox::critical(this, QString("Attention"), QString("Le fichier: "+tempfilename+" n'est pas un fichier vidéo valide"));
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
        QMessageBox::critical(this,QString("error"), QString("output resolution non determined"));
    }

    if (inputvideo.isOpened()) {
        update();
    }
}

void MainWindow::update()
{
    QPixmap dem = draft(src, resSortie, this->ui->verticalSliderHaut->value(), this->ui->verticalSliderZoom->value(), this->ui->verticalSliderFov->value());
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

void MainWindow::endRender(bool withsound)
{
    this->ui->progressBar->setVisible(false);

    this->ui->menuTest->setEnabled(true);
    this->ui->menuOptions->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->inputvideo.set(CAP_PROP_POS_FRAMES, 0);
    if(withsound){
        QMessageBox::information(this, QString("Info"), QString("Conversion terminée"));
    } else {
        QMessageBox::information(this, QString("Info"), QString("Conversion terminée, sans son"));
    }

}

void MainWindow::startRender()
{
    this->ui->menuTest->setEnabled(false);
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
                           this->vid, this->inputvideo, this->resSortie, S, NAME, path, ui->progressBar, this);

    thr->start();

}

QPixmap draft(Mat image, Size & output, int hauteur, int zoom, int fovChange ) {
    //zoom en pourcentage
    Mat mx, my, mapx, mapy, res;
    Size s = image.size();

    create_map(mapx, mapy, s, output / 10, hauteur, zoom/100.0, fovChange);
    resize(mapx, mx, output);
    resize(mapy, my, output);
    remap(image, res, mx, my, INTER_LINEAR);// , BORDER_WRAP);
    //imshow("Open Warp", res);
    return ASM::cvMatToQPixmap(res);

}
