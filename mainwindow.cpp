#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "openwarp.h"
#include "asmOpenCV.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QGraphicsPixmapItem>
#include <iostream>
#include <QThread>


MainWindow::MainWindow(QWidget *parent, int argc, char **argv) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
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

    QObject::connect(ui->actionAction_1, SIGNAL(triggered(bool)),this,SLOT(selectFile()) );
    QObject::connect(ui->pushButton, SIGNAL(clicked(bool)),this,SLOT(startRender()) );
    QObject::connect(ui->verticalSliderHaut,SIGNAL(valueChanged(int)),this,SLOT(update()) );
    QObject::connect(ui->verticalSliderZoom,SIGNAL(valueChanged(int)),this,SLOT(update()) );

}

MainWindow::~MainWindow()
{
    if (thr != NULL)
        delete thr;

    delete item;
    delete scn;
    delete ui;
}

void MainWindow::selectFile()
{
    QString qwvid = QFileDialog::getOpenFileName(this,QString("Sélectionner la vidéo à traiter"),QStandardPaths::writableLocation(QStandardPaths::MoviesLocation));
    //GetOpenFileName(&ofn);

    // Now simpley display the file name
    //MessageBox(NULL, ofn.lpstrFile, __TEXT("File Name"), MB_OK);
    this->vid = qwvid.toStdString();
    //start(argc, argv);
    QFile file(qwvid);
    if (!file.exists()){
        QMessageBox::warning(this, QString("Attention"), QString("Le fichier: "+qwvid+" n'exite pas"));
    } else {
        this->inputvideo = getInputVideo(vid);
        if(this->inputvideo.isOpened()) {
            QMessageBox::information(this, QString("Vidéo Ouverte"), QString::fromStdString(vid));
            ui->verticalSliderHaut->setEnabled(true);
            ui->verticalSliderZoom->setEnabled(true);
            ui->pushButton->setEnabled(true);
            this->inputvideo.set(CAP_PROP_POS_FRAMES, 10);
            inputvideo >> src;              // read
            //TODO: definir size
            Size output(1000, 1000);
            QPixmap dem = draft(src, output, 45, 100);
            inputvideo.set(CAP_PROP_POS_FRAMES, 0);
            item->setPixmap(dem);
            ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
            ui->graphicsView->update();
            //ui->graphicsView->show();
        } else {
            QMessageBox::critical(this, QString("Attention"), QString("Le fichier: "+qwvid+" n'est pas un fichier vidéo valide"));
        }
    }
}

void MainWindow::update()
{
    Size output(1000, 1000);
    QPixmap dem = draft(src, output, this->ui->verticalSliderHaut->value(), this->ui->verticalSliderZoom->value());
    item->setPixmap(dem);
    ui->graphicsView->fitInView(item,Qt::KeepAspectRatio);
    ui->graphicsView->update();
    ui->graphicsView->show();
}

void MainWindow::endRender()
{
    this->ui->progressBar->setVisible(false);

    this->ui->menuTest->setEnabled(true);
    this->ui->menuOptions->setEnabled(true);
    this->ui->pushButton->setEnabled(true);
    this->inputvideo.set(CAP_PROP_POS_FRAMES, 0);
    QMessageBox::information(this, QString("Info"), QString("Conversion terminée"));

}

void MainWindow::startRender()
{
    this->ui->menuTest->setEnabled(false);
    this->ui->menuOptions->setEnabled(false);
    this->ui->pushButton->setEnabled(false);
    std::string prog = QCoreApplication::applicationDirPath().toStdString();
    const std::string path = prog.substr(0, prog.find_last_of('\\'));
    std::cout << "Path: " << path << std::endl;
    char shaut[100];
    char szoom[100];
    sprintf(shaut, "%d", this->ui->verticalSliderHaut->value());
    sprintf(szoom, "%d", this->ui->verticalSliderZoom->value());
    Size S = Size((int)this->inputvideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
        (int)inputvideo.get(CV_CAP_PROP_FRAME_HEIGHT));
    std::string::size_type pAt = vid.find_last_of('.');                  // Find extension point

    const std::string NAME = vid.substr(0, pAt)+"H"+std::string(shaut)+"Z"+std::string(szoom)+".avi";   // Form the new name with container

    this->ui->progressBar->setVisible(true);

    int fcount = this->inputvideo.get(CV_CAP_PROP_FRAME_COUNT);

    this->ui->progressBar->setValue(0);
    this->ui->progressBar->setMaximum(fcount);

    thr = new RenderThread(this->ui->verticalSliderHaut->value(),this->ui->verticalSliderZoom->value() / 100.0,
                           this->vid, this->inputvideo, Size(1000,1000), S, NAME, path, ui->progressBar, this);

    thr->start();

}

QPixmap draft(Mat image, Size & output, int hauteur, int zoom ) {
    //zoom en pourcentage
    Mat mx, my, mapx, mapy, res;
    Size s = image.size();

    create_map(mapx, mapy, s, output / 10, hauteur, zoom/100.0);
    resize(mapx, mx, output);
    resize(mapy, my, output);
    remap(image, res, mx, my, INTER_LINEAR);// , BORDER_WRAP);
    //imshow("Open Warp", res);
    return ASM::cvMatToQPixmap(res);

}
