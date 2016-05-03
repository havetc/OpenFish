#ifndef WARP
#define WARP

#include "openwarp.h"
//#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/cuda.hpp>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>

#define _USE_MATH_DEFINES // for C++
#include <math.h>
#include <stdio.h>
#include <iostream>

using namespace cv;
using namespace std;

Point3f polar_to_sphere(float angle, float mag, float r)
{
    float st = sin(mag);
    return Point3f(r * st * cos(angle), r * st * sin(angle), r * cos(mag));
}

//on cherche a obtenir le rayon pour avoir la distance a l'ecran
float get_r(float theta, float phi, Point3f & contact) {
    //coordonnee du point sur le dome
    Point3f pdome = polar_to_sphere(phi, theta);

    float angle = acos(pdome.dot(contact));
    return 1.0 / cos(angle);
}

/*function that create the map used to remap each frame
 * frameSize: output size (?)
 */
void create_map(Mat & map_x, Mat & map_y, CvSize frameSize, CvSize output, float AngleHauteur , float zoom, int fovChange ) {


    map_x.create(output, CV_32FC1);
    map_y.create(output, CV_32FC1);

    int height = frameSize.height;
    int width = frameSize.width;

    Point2f center(output.width / 2.0, output.height / 2.0);

    //angle lié à la hauteur, en radian
    float hauteur = (M_PI / 180) * AngleHauteur;
    float invzoom = 1/zoom;
    float thetac = M_PI / 2.0 - hauteur;

    //definition des coordonnée de contact dome/plan
    float zco = cos(thetac);
    float yco = 0;
    float xco = sin(thetac);

    cout << "contact: x: " << xco << " y:" << yco << " z:" << zco << endl;

    Point3f contact(xco, yco, zco);

    Matx33f rot3d(cos(hauteur), 0, sin(hauteur),
        0, 1, 0,
        -sin(hauteur), 0, cos(hauteur));
    cout << "contact coord: " << contact << endl;
    cout << "rotated contact: " << rot3d * contact << endl;

    for (int j = 0; j < map_x.rows; j++)
    {
        for (int i = map_x.cols / 2; i < map_x.cols; i++) {
            Point2f p(i, j);
            p -= center;
            Vec<float, 1> resMag;
            Vec<float, 1> resAng;

            Vec<float, 1> x((M_PI / 2.0) * p.x * (fovChange / 100.0) / (output.height / 2.0));
            Vec<float, 1> y((M_PI / 2.0) * p.y * (fovChange / 100.0) / (output.height / 2.0));

            cartToPolar(x, y, resMag, resAng);
            /*if (resMag[0] > 1) {
                cout << x << y << endl;
            }*/
            float r = get_r(resMag[0], resAng[0], contact);

            Point3f plan = polar_to_sphere(resAng[0], resMag[0], r);

            Point3f rotate = rot3d * plan;

            //right side
            map_y.at<float>(j, i) = (-rotate.z) * invzoom * height + height / 2;
            map_x.at<float>(j, i) = - rotate.y * invzoom * height + width / 2;

            //left side
            map_y.at<float>(map_x.rows - j - 1, map_x.cols - i) = (-rotate.z) * invzoom * height + height / 2;
            map_x.at<float>(map_x.rows - j - 1, map_x.cols - i) =  - rotate.y * invzoom * height + width / 2;
        }
    }
    convertMaps(map_x, map_y, map_x, map_y, CV_16SC2);
}

void on_trackbar( int, void * change) {
    *(bool*)change = true;
}

void on_trackbar2(int, void * value) {
    *(int*)value = *(int*)value - *(int*)value % 16;
}


/* transfert dans mainWindow
void draft(Mat image, Size & output, int * hauteur, float * zoom ) {
    //zoom en pourcentage
    int izoom = 100;
    bool change = true;
    createTrackbar("hauteur ", "Open Warp", hauteur, 90, on_trackbar, &change);
    createTrackbar("zoom", "Open Warp", &izoom, 500, on_trackbar, &change);
//    createTrackbar("Largeur sortie ", "Open Warp", &output.width, 4000, on_trackbar2, &output.width);
//    createTrackbar("Hauteur sortie", "Open Warp", &output.height, 4000, on_trackbar2, &output.height);
    Mat mx, my, mapx, mapy, res;
    Size s = image.size();
    while (waitKey(10) == -1) {
        //on retrace que si l'on a changé un paramètre
        if (change) {
            *zoom = izoom / 100.0;
            create_map(mapx, mapy, s, output / 10, *hauteur, *zoom);
            resize(mapx, mx, output);
            resize(mapy, my, output);
            remap(image, res, mx, my, INTER_LINEAR);// , BORDER_WRAP);
            imshow("Open Warp", res);
        }
        change = false;
    }
    //MessageBeep(MB_OK);
    //waitKey(0);
}
*/



VideoCapture getInputVideo(QString vid)
{
    VideoCapture inputVideo(vid.toLocal8Bit().constData());// Open input
    if (!inputVideo.isOpened())
    {
        cout << "Could not open the input video: " << vid.toLocal8Bit().constData() << endl;
        return inputVideo;
    }


    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form


    char EXT[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };

    Size S = Size((int)inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
        (int)inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));


    //Size output(1000, 1000);


    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
        << " of nr#: " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;

    return inputVideo;
}

/**
  *
  *  DEPRECATED, Now running inside the RenderThread
  *
  */

//int startConv(int hauteur, float zoom, std::string vid, VideoCapture & inputVideo, Size output, Size S /*input*/,
//              const std::string & NAME, const std::string path)
//{

//    std::cout << "test startConv" << std::endl;


//    //barre->setVisible(true);
//    //barre->setValue(0);
//    Mat src, res, mapx, mapy;
///*
//    //lancement du brouillon
//    inputVideo.set(CAP_PROP_POS_FRAMES, 10);
//    inputVideo >> src;              // read
//    draft(src, output, &hauteur, &zoom);
//    inputVideo.set(CAP_PROP_POS_FRAMES, 0);
//*/
//    create_map(mapx, mapy, S, output, hauteur , zoom);

//    cout << "map created" << endl;

//    //barre->setMaximum(fcount);

//    VideoWriter outputVideo;                                        // Open the output
//    int ex;//fourcc
//    outputVideo.open(path+"\\temp.avi", ex = -1, inputVideo.get(CV_CAP_PROP_FPS), output , true);

//    if (!outputVideo.isOpened())
//    {
//        cout << "Could not open the output video for write: " << vid << endl;
//        return -1;
//    }

//    for (long i = 0; true; i++) //Show the image captured in the window and repeat
//    {

//        inputVideo >> src;              // read

//        if (src.empty()) {
//            outputVideo.release();
//            break;
//        }
//        else {

//            remap(src, res, mapx, mapy, INTER_LINEAR);// , BORDER_WRAP);
//            //res = src;

//            if (i % 10 == 0) {
//                //imshow(wndname, res);
//                //waitKey(1);
//                //barre->setValue(i);
//                //RenderThread::update(i);
//            }
//            outputVideo << res;
//        }
//    }

//    string ffmpegCMD("\"\""+path+"/ffmpeg\" -y -i \""+path+"/temp.avi\" -i \""+vid+"\" -map 0:v -map 1:a -c copy -shortest \""+NAME+"\" > log.txt \"");
//    cout << ffmpegCMD << endl;
//    cout << "TEST TEST TEST TEST" << endl;
//    cerr << "TEST2 TEST2 TES T  2 TEST2" << endl;
//    cout << system(ffmpegCMD.c_str()) << endl;
//    string temp(path + "/temp.avi");
//    cout << "temporaire a supprimer:" << temp << endl;
////    remove(temp.c_str());
////    MessageBox(NULL, TEXT("Conversion terminée"), __TEXT("Info"), MB_OK);
//    cout << "Finished writing" << endl;
//    //barre->setVisible(false);
//    //waitKey(0);
//    return 0;
//}

#endif
