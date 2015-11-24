#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
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
    void endRender();
    void startRender();

//protected:
//    virtual void wheelEvent(QWheelEvent* event);

private:
    Ui::MainWindow *ui;

    GetSize* GS_wnd;
    int argc;
    char** argv;
    std::string vid;
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
    RenderThread(int hauteur, float zoom, std::string vid, cv::VideoCapture & inputVideo,
                 cv::Size output, cv::Size S /*input*/, const std::string & NAME, const std::string path, QProgressBar * barre, MainWindow * win)
        : QThread::QThread(){
        this->hauteur = hauteur;
        this->zoom = zoom;
        this->vid = vid;
        this->output =output;
        this->S = S;
        this->NAME = NAME;
        this->path = path;
        this->inputvideo  = inputVideo;
        this->barre = barre;

        QObject::connect(this, SIGNAL(update(int)),barre,SLOT(setValue(int)));
        QObject::connect(this, SIGNAL(end()), win, SLOT(endRender()));
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
        create_map(mapx, mapy, S, output, hauteur , zoom);

        cout << "map created" << endl;

        //barre->setMaximum(fcount);

        VideoWriter outputVideo;                                        // Open the output
        int ex;//fourcc
        outputVideo.open(path+"\\temp.avi", ex = -1, inputvideo.get(CV_CAP_PROP_FPS), output , true);

        if (!outputVideo.isOpened())
        {
            cout << "Could not open the output video for write: " << vid << endl;
            QMessageBox::critical(NULL, QString("Erreur"), QString("Le fichier n'a pas pu être écrit"));
            return;// -1;
        }

        for (long i = 0; true; i++) //Show the image captured in the window and repeat
        {

            inputvideo >> src;              // read

            if (src.empty()) {
                outputVideo.release();
                break;
            }
            else {

                remap(src, res, mapx, mapy, INTER_LINEAR);// , BORDER_WRAP);
                //res = src;

                if (i % 10 == 0) {
                    //imshow(wndname, res);
                    //waitKey(1);
                    //barre->setValue(i);
                    update(i);
                }
                outputVideo << res;
            }
        }

        // comme tous les chemins de fichiers cree dans ce projet, il ne sont pas compatible UNIX mais que windows
        //il faut faire des appels a des librairie Qt pour gerant des chemins ou utiliser des #ifdef pour avoir une version multiplateforme
        string ffmpegCMD("\"\""+path+"\\ffmpeg\" -y -i \""+path+"\\temp.avi\" -i \""+vid+"\" -map 0:v -map 1:a -c copy -shortest \""+NAME+"\"");
        cout << ffmpegCMD << endl;
        cout << system(ffmpegCMD.c_str());
        string temp(path + "\\temp.avi");
        cout << "temporaire a supprimer:" << temp << endl;
        QFile::remove(QString::fromStdString(temp));
    //    MessageBox(NULL, TEXT("Conversion terminée"), __TEXT("Info"), MB_OK);
        cout << "Finished writing" << endl;
        end();
    }

private:
    int hauteur;
    float zoom;
    QProgressBar * barre;
    std::string vid;
    cv::VideoCapture inputvideo;
    cv::Size output;
    cv::Size S /*input*/;
    std::string NAME;
    std::string path;

signals:
    void update(int);
    void end();
};


#endif // MAINWINDOW_H
