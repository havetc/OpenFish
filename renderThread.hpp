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
    RenderThread(bool withsound, int hauteur, float zoom, int fov, int frames, int offs,
                 QString vid, cv::VideoCapture & inputVideo,
                 cv::Size output, cv::Size S, const QString & NAME, const QDir& path)
        : QThread::QThread(){
        this->hauteur = hauteur;
        this->zoom = zoom;
        this->fov = fov;
        this->frames = frames;
        this->offs = offs;
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
        create_map(mapx, mapy, S, output, hauteur , zoom, fov, frames, offs);

        cout << "map created" << endl;

        VideoWriter outputVideo;                                        // Open the output
        int ex;//fourcc
        ex = CV_FOURCC('X','V','I','D');
        outputVideo.open(path.absoluteFilePath( QString("temp.avi") ).toLocal8Bit().constData() , ex, inputvideo.get(CV_CAP_PROP_FPS), output , true);

        if (!outputVideo.isOpened())
        {
            cerr << "Could not open the temp output video for write: " << qPrintable(vid) << endl;
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
            if(src._frame.cols != 0){
                remap(src._frame, res._frame, mapx, mapy, INTER_LINEAR, BORDER_CONSTANT);

                if (src._pos % 32 == 0) {
                    update(src._pos);
                }
                res._pos = src._pos;
                outputsource << res;
            }
        }

        QString temp = path.absoluteFilePath("temp.avi");

        if(withsound){

            int returned = 0;
#if defined _WIN32 || defined _WIN64
            QString ffmpegCMD = QString("ffmpeg.exe -y -i \""+path.absoluteFilePath("temp.avi") +
                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\" ");
//            QString ffmpegCMD = QString("DIR" );

//            QString logString = QString("echo \"ffmpeg.exe -y -i \""+path.absoluteFilePath("temp.avi") +
//                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\"\" > logcmd.txt" );
//            returned =  _wsystem(logString.toStdWString().data());
            returned =  _wsystem(ffmpegCMD.toStdWString().data());
#elif defined __linux__
            QString ffmpegCMD = QString("avconv -y -i \""+path.absoluteFilePath("temp.avi") +
                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\" " );

            QString logString = QString("echo \"avconv -y -i \""+path.absoluteFilePath("temp.avi") +
                                        "\" -i \""+vid+"\" -map 0:v -map 1:a -c:a copy -c:v copy -shortest \""+NAME+"\"\" > logcmd.txt" );
            returned =  system(qPrintable(logString));
            returned =  system(qPrintable(ffmpegCMD));
#endif
            std::cout << qPrintable(ffmpegCMD) << std::endl;
            outputVideo.release();
            if(returned != 0) {
                sound_error();
                //render done but ffmpeg bug, without sound
                cout << QFile::rename( temp, NAME) << endl;
                end(false);
            } else {
                cout << "temporaire a supprimer:" << qPrintable(temp) << endl;
                if (QFile::remove(temp)){
                    cout << "Finished writing" << endl;
                } else {
                    cout << "error " << qPrintable(temp) << " couldn't be suppressed" << endl;
                }
                //render done, with sound
                end(true);
            }
            //
        } else {
            cout << "temporaire a supprimer:" << qPrintable(temp) << endl;
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
    int frames;
    int offs;
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
