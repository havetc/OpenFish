#ifndef OPENWARP
#define OPENWARP


#include <opencv2/opencv_modules.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <QFileDialog>

using namespace cv;

Point3f polar_to_sphere(float angle, float mag, float r = 1);

//on cherche a obtenir le rayon pour avoir la distance à l'écran
float get_r(float theta, float phi, Point3f & contact);

/*function that create the map used to remap each frame
 * frameSize: output size (?)
 */
void create_map(Mat & map_x, Mat & map_y, CvSize frameSize, CvSize output, float AngleHauteur = 45, float zoom = 1);

void on_trackbar( int, void * change);

void on_trackbar2(int, void * value);

void draft(Mat image, Size & output, int * hauteur, float * zoom );


int start(int argc, char **argv);


#endif // OPENWARP

