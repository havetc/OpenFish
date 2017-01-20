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

#ifndef OPENWARP
#define OPENWARP


//#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/core/cuda.hpp>
#include <QFileDialog>
#include <QProgressBar>

using namespace cv;

Point3f polar_to_sphere(float angle, float mag, float r = 1);

//on cherche a obtenir le rayon pour avoir la distance à l'écran
float get_r(float theta, float phi, Point3f & contact);

/**
 * function that create the map used to remap each frame
 * frameSize: output size (?)
 */
void create_map(Mat & map_x, Mat & map_y, CvSize frameSize, CvSize output, float AngleHauteur = 45, float zoom = 1, int fovChange=100);

void on_trackbar( int, void * change);

void on_trackbar2(int, void * value);

QPixmap draft(Mat image, Size & output, int hauteur, int zoom , int fovChange);

VideoCapture getInputVideo(QString vid);

//DEPRECATED
//int startConv(int hauteur, float zoom, std::string vid, VideoCapture & inputVideo,
//              Size output, Size S /*input*/, const std::string & NAME, const std::string path);

#endif // OPENWARP

