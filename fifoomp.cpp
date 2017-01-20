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

#include "fifoomp.h"
#include <omp.h>
#include <thread>

OrderedMat::OrderedMat(): _frame(cv::Mat()), _pos(0) {}

long OrderedMat::operator()() const{
    return _pos;
}

bool operator < (const OrderedMat & a, const OrderedMat & b){
    return a() < b();
}

bool operator > (const OrderedMat & a, const OrderedMat & b){
    return ! (a < b);
}

fifoOmp::fifoOmp(cv::VideoCapture & v)
{
    _id = 0;
    _lock = new omp_lock_t;
    omp_init_lock(_lock);
    _vc = v;
    buffer_capacity = 32;
}

fifoOmp::fifoOmp(cv::VideoWriter & v)
{
    _id = 0;
    _lock = new omp_lock_t;
    omp_init_lock(_lock);
    _vw = v;
    buffer_capacity = 32;
}

//just a wrapper with verifications
fifoOmp &fifoOmp::operator >>(OrderedMat &im)
{
    omp_set_lock(_lock);
    _vc >> im._frame;
    im._pos = _id ++;
    omp_unset_lock(_lock);
    return *this;
}

fifoOmp &fifoOmp::operator <<(const OrderedMat &im)
{
    omp_set_lock(_lock);
    fifo.push(im);
    while(this->is_full()){
        while(fifo.top()() == _id){
            _vw << fifo.top()._frame;
            fifo.pop();
            ++_id;
        }
    }
    omp_unset_lock(_lock);

    return *this;
}

bool fifoOmp::is_full()
{
    return fifo.size() >= static_cast<unsigned int>(this->buffer_capacity);
}
