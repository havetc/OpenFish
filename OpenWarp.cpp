#ifndef WARP
#define WARP

#include "openwarp.h"
#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <QFileDialog>

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

//on cherche a obtenir le rayon pour avoir la distance à l'écran
float get_r(float theta, float phi, Point3f & contact) {
    //coordonnee du point sur le dome
    Point3f pdome = polar_to_sphere(phi, theta);

    float angle = acos(pdome.dot(contact));
    return 1.0 / cos(angle);
}

/*function that create the map used to remap each frame
 * frameSize: output size (?)
 */
void create_map(Mat & map_x, Mat & map_y, CvSize frameSize, CvSize output, float AngleHauteur , float zoom ) {

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

            Vec<float, 1> x((M_PI / 2) * p.x / (output.height / 2));
            Vec<float, 1> y((M_PI / 2) * p.y / (output.height / 2));

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


int start(int argc, char **argv)
{
    //FreeConsole();
    //String vid = String("C:\\Users\\Corentin\\Videos\\Mon film.mp4");

    char wndname[] = "Open Warp";
    imshow(wndname, Mat(500,800,CV_16U));

    QString qwvid = QFileDialog::getOpenFileName();
    //GetOpenFileName(&ofn);

    // Now simpley display the file name 
    //MessageBox(NULL, ofn.lpstrFile, __TEXT("File Name"), MB_OK);
    wstring wvid = qwvid.toStdWString();


    wcout << wvid << endl;
    //cout << getBuildInformation() << endl;
    //cuda::printShortCudaDeviceInfo(cuda::getDevice());
    //setup converter
  
    Mat map_x, map_y;

    string vid = string(wvid.begin(), wvid.end());

    VideoCapture inputVideo(vid);              // Open input
    if (!inputVideo.isOpened())
    {
        cout << "Could not open the input video: " << vid << endl;
        return -1;
    }

    string::size_type pAt = vid.find_last_of('.');                  // Find extension point

    int ex = static_cast<int>(inputVideo.get(CV_CAP_PROP_FOURCC));     // Get Codec Type- Int form


    char EXT[] = { (char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0 };

    Size S = Size((int)inputVideo.get(CV_CAP_PROP_FRAME_WIDTH),    // Acquire input size
        (int)inputVideo.get(CV_CAP_PROP_FRAME_HEIGHT));


    Size output(1000, 1000);


    cout << "Input frame resolution: Width=" << S.width << "  Height=" << S.height
        << " of nr#: " << inputVideo.get(CV_CAP_PROP_FRAME_COUNT) << endl;
    cout << "Input codec type: " << EXT << endl;

    Mat src, res, mapx, mapy;
    int hauteur = 45;
    float zoom = 1;

    //lancement du brouillon
    inputVideo.set(CAP_PROP_POS_FRAMES, 10);
    inputVideo >> src;              // read
    draft(src, output, &hauteur, &zoom);
    inputVideo.set(CAP_PROP_POS_FRAMES, 0);

    create_map(mapx, mapy, S, output, hauteur , zoom);
    cout << "map created" << endl;

    string prog = string(argv[0]);
    const string path = prog.substr(0, prog.find_last_of('\\'));
    cout << "Path: " << path << endl;
    char shaut[100];
    char szoom[100];
    sprintf(shaut, "%d", hauteur);
    sprintf(szoom, "%d", zoom);

    const string NAME = vid.substr(0, pAt)+"H"+string(shaut)+"Z"+string(szoom)+".avi";   // Form the new name with container
    VideoWriter outputVideo;                                        // Open the output
    outputVideo.open(path+"\\temp.avi", ex = -1, inputVideo.get(CV_CAP_PROP_FPS), output , true);

    if (!outputVideo.isOpened())
    {
        cout << "Could not open the output video for write: " << vid << endl;
        return -1;
    }

    for (long i = 0; true; i++) //Show the image captured in the window and repeat
    {

        inputVideo >> src;              // read

        if (src.empty()) {
            outputVideo.release();
            break;
        }
        else {

            remap(src, res, mapx, mapy, INTER_LINEAR);// , BORDER_WRAP);
            //res = src;

            if (i % 1 == 0) {
                imshow(wndname, res);
                waitKey(1);
            }
            outputVideo << res;
        }
    }
    
    string ffmpegCMD("\"\""+path+"\\ffmpeg\" -y -i \""+path+"\\temp.avi\" -i \""+vid+"\" -map 0:v -map 1:a -c copy -shortest \""+NAME+"\"");
    cout << ffmpegCMD << endl;
    cout << system(ffmpegCMD.c_str());
    string temp(path + "\\temp.avi");
    cout << "temporaire a supprimer:" << temp << endl;
    remove(temp.c_str());
//    MessageBox(NULL, TEXT("Conversion terminée"), __TEXT("Info"), MB_OK);
    cout << "Finished writing" << endl;

    //waitKey(0);
    return 0;
}

#endif
