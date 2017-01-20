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
