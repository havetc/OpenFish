#ifndef RENDERTHREAD_HPP
#define RENDERTHREAD_HPP

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <QThread>
#include <QProgressBar>
#include <QMainWindow>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QDir>
#include <iostream>
#include <fifoomp.h>
#include "openwarp.h"
#include <omp.h>


class MainWindow;

/**
 * @brief The RenderThread class
 * utilisee pour faire fonctionner la conversion sur un thread different, afin de ne pas bloquer
 * l'interface et d'avoir une barre de chargement
 */
class RenderThread : public QThread {

    Q_OBJECT
public:
    RenderThread(bool withsound, int hauteur, float zoom, int fov, QString vid, cv::VideoCapture & inputVideo,
                 cv::Size output, cv::Size S /*input*/, const QString & NAME, const QDir& path)
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
        this->withsound = withsound;

//        QObject::connect(this, SIGNAL(update(int)),barre,SLOT(setValue(int)));
//        QObject::connect(this, SIGNAL(end(bool)), win, SLOT(endRender(bool)));
//        QObject::connect(this, SIGNAL(sound_error()), win, SLOT(errorPopUp()));
    }

    virtual void run() {

        using namespace std;
        using namespace cv;

        Mat mapx, mapy;
        create_map(mapx, mapy, S, output, hauteur , zoom, fov);

        cout << "map created" << endl;

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

        fifoOmp inputsource(inputvideo);
        long number_of_frame = inputvideo.get(CV_CAP_PROP_FRAME_COUNT);
        fifoOmp outputsource(outputVideo);
#pragma omp parallel for shared(inputsource, outputsource)
        for (long i = 0; i < number_of_frame; i++) //Show the image captured in the window and repeat
        {
            OrderedMat src, res;

            inputsource >> src;              // read

            remap(src._frame, res._frame, mapx, mapy, INTER_LINEAR, BORDER_CONSTANT);

            if (src._pos % 32 == 0) {
                update(src._pos);
            }
            res._pos = src._pos;
            outputsource << res;

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


#endif // RENDERTHREAD_HPP
