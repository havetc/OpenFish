#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <QThread>
#include <QProgressBar>
#include <QMessageBox>
#include <iostream>
#include "openwarp.h"

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

class RenderThread;

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

//protected:
//    virtual void wheelEvent(QWheelEvent* event);

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


/**
 * @brief The RenderThread class
 * utilisee pour faire fonctionner la conversion sur un thread different, afin de ne pas bloquer
 * l'interface et d'avoir une barre de chargement
 */
class RenderThread : public QThread {

    Q_OBJECT
public:
    RenderThread(bool withsound, int hauteur, float zoom, int fov, QString vid, cv::VideoCapture & inputVideo,
                 cv::Size output, cv::Size S /*input*/, const QString & NAME, const QDir& path, QProgressBar * barre, MainWindow * win)
        : QThread::QThread(){
        this->hauteur = hauteur;
        this->zoom = zoom;
        this->fov = fov;
        this->vid = vid;
        this->output =output;
        this->S = S;
        this->NAME = NAME;
        std::cout << "Output File: " << NAME.toLocal8Bit().constData() << std::endl;
        this->path = path;
        this->inputvideo  = inputVideo;
        this->barre = barre;
        this->withsound = withsound;

        QObject::connect(this, SIGNAL(update(int)),barre,SLOT(setValue(int)));
        QObject::connect(this, SIGNAL(end(bool)), win, SLOT(endRender(bool)));
        QObject::connect(this, SIGNAL(sound_error()), win, SLOT(errorPopUp()));
    }

    virtual void run() {

        using namespace std;
        using namespace cv;

        //char wndname[] = "Open Warp";
        //barre->setVisible(true);
        //barre->setValue(0);
        Mat src, res, mapx, mapy;
        /*
        //lancement du brouillon
        inputVideo.set(CAP_PROP_POS_FRAMES, 10);
        inputVideo >> src;              // read
        draft(src, output, &hauteur, &zoom);
        inputVideo.set(CAP_PROP_POS_FRAMES, 0);
    */
        create_map(mapx, mapy, S, output, hauteur , zoom, fov);

        cout << "map created" << endl;

        //barre->setMaximum(fcount);

        VideoWriter outputVideo;                                        // Open the output
        int ex;//fourcc
        ex = CV_FOURCC('X','V','I','D');
        outputVideo.open(path.absoluteFilePath( QString("temp.avi") ).toLocal8Bit().constData() , ex, inputvideo.get(CV_CAP_PROP_FPS), output , true);

        if (!outputVideo.isOpened())
        {
            cerr << "Could not open the temp output video for write: " << vid.toLocal8Bit().constData() << endl;
            QMessageBox::critical(NULL, QString("Erreur"), QString::fromLocal8Bit("Le fichier n'a pas pu être écrit"));
            end(true);
        }

        for (long i = 0; true; i++) //Show the image captured in the window and repeat
        {

            inputvideo >> src;              // read

            if (src.empty()) {
//                outputVideo.release();
                break;
            }
            else {

                remap(src, res, mapx, mapy, INTER_LINEAR);// , BORDER_WRAP);
                //res = src;

                if (i % 4 == 0) {
                    //imshow(wndname, res);
                    //waitKey(1);
                    //barre->setValue(i);
                    update(i);
                }
                outputVideo << res;
            }
        }



        QString temp = path.absoluteFilePath("temp.avi");

        if(withsound){

            int returned = 0;
            QString ffmpegCMD = QString("avconv -y -i \""+path.absoluteFilePath("temp.avi") +
                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\" " );

            QString logString = QString("echo \"avconv -y -i \""+path.absoluteFilePath("temp.avi") +
                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\"\" > logcmd.txt" );

            std::cout << ffmpegCMD.toLocal8Bit().constData() << std::endl;
            //only windows, system should work fine for linux
            returned =  system(logString.toLocal8Bit().constData());
            returned =  system(ffmpegCMD.toLocal8Bit().constData());

            if(returned != 0) {
                sound_error();
                //render done but ffmpeg bug, without sound
                cout << QFile::rename( temp, NAME) << endl;
                end(false);
            } else {
                cout << "temporaire a supprimer:" << temp.toStdString() << endl;
                QFile::remove(temp);
                //    MessageBox(NULL, TEXT("Conversion terminée"), __TEXT("Info"), MB_OK);
                cout << "Finished writing" << endl;
                //render done, with sound
                end(true);
            }
            //
        } else {
            cout << "temporaire a supprimer:" << temp.toStdString() << endl;
            cout << QFile::rename( temp, NAME) << endl;
            //    MessageBox(NULL, TEXT("Conversion terminée"), __TEXT("Info"), MB_OK);
            cout << "Finished writing" << endl;
            //render done voluntarly without sound
            end(false);
        }
    }


private:
    int hauteur;
    int fov;
    bool withsound;
    float zoom;
    QProgressBar * barre;
    //absolute path of the input video
    QString vid;
    cv::VideoCapture inputvideo;
    cv::Size output;
    cv::Size S /*input*/;

    //absolute path of the output video
    QString NAME;

    //path of the directory which contains input and output video
    QDir path;

signals:
    void update(int);
    void end(bool);
    void sound_error(void);

};


#endif // MAINWINDOW_H
