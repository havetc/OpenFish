#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "openwarp.h"
#include "asmOpenCV.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QGraphicsPixmapItem>

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

    QObject::connect(ui->actionAction_1, SIGNAL(triggered(bool)),this,SLOT(selectFile()) );
    QObject::connect(ui->verticalSliderHaut,SIGNAL(valueChanged(int)),this,SLOT(update()) );
    QObject::connect(ui->verticalSliderZoom,SIGNAL(valueChanged(int)),this,SLOT(update()) );

}

MainWindow::~MainWindow()
{
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
            ui->graphicsView->fitInView(item,Qt::KeepAspectRatioByExpanding);
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
    ui->graphicsView->fitInView(item,Qt::KeepAspectRatioByExpanding);
    ui->graphicsView->update();
    ui->graphicsView->show();
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
