#ifndef FIFOOMP_H
#define FIFOOMP_H
#include <omp.h>
#include <cv.hpp>
#include <queue>
#include <opencv2/highgui/highgui.hpp>

class OrderedMat
{
public:
    OrderedMat();
    OrderedMat(cv::Mat & Mat, long pos);
    long operator()() const;
public:
    cv::Mat _frame;
    long _pos;
};

bool operator < (const OrderedMat & a, const OrderedMat & b);
bool operator > (const OrderedMat & a, const OrderedMat & b);

class fifoOmp
{
public:
    fifoOmp(cv::VideoCapture & v);
    fifoOmp(cv::VideoWriter & v);
    ~fifoOmp() {delete _lock;}

    fifoOmp & operator>>(OrderedMat & im);
    fifoOmp & operator<<(const OrderedMat & im);
    bool is_full();


private:
    std::priority_queue<OrderedMat,std::deque<OrderedMat>,std::greater<OrderedMat> > fifo;
    omp_lock_t * _lock;
    int buffer_capacity;
    long _id;
    cv::VideoCapture _vc;
    cv::VideoWriter _vw;

};

#endif // FIFOOMP_H
