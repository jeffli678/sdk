/**
 * @file posix/wait.cpp
 * @brief POSIX event/timeout handling
 *
 * (c) 2013-2014 by Mega Limited, Wellsford, New Zealand
 *
 * This file is part of the MEGA SDK - Client Access Engine.
 *
 * Applications using the MEGA API must present a valid application key
 * and comply with the the rules set forth in the Terms of Service.
 *
 * The MEGA SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * @copyright Simplified (2-clause) BSD License.
 *
 * You should have received a copy of the license along with this
 * program.
 */

#include "mega.h"
#include "mega/thread/cppthread.h"

namespace mega {

//thread
CppThread::CppThread()
{
	thread = NULL;
}

void CppThread::start(void *(*start_routine)(void*), void *parameter)
{
	thread = new std::thread(start_routine, parameter);
}

void CppThread::join()
{
	thread->join();
}

CppThread::~CppThread()
{
	delete thread;
}

//mutex
CppMutex::CppMutex()
{
	mutex = NULL;
	rmutex = NULL;
}

void CppMutex::init(bool recursive = true)
{
	if (mutex || rmutex) return;

    if(recursive)
		rmutex = new std::recursive_mutex;
    else
		mutex = new std::mutex;
}

void CppMutex::lock()
{
	mutex ? mutex->lock() : rmutex->lock();
}

void CppMutex::unlock()
{
	mutex ? mutex->unlock() : rmutex->unlock();
}

CppMutex::~CppMutex()
{
	delete mutex;
	delete rmutex;
}


//semaphore
CppSemaphore::CppSemaphore()
{
    count = 0;
}
void CppSemaphore::release()
{
    std::unique_lock<std::mutex> lock(mtx);
    count++;
    cv.notify_one();
}

void CppSemaphore::wait()
{
    std::unique_lock<std::mutex> lock(mtx);

    while(count == 0){
        cv.wait(lock); // I believe this one releases the lock while waiting
    }
    count--;
}


int CppSemaphore::timedwait(int milliseconds)
{
    std::unique_lock<std::mutex> lock(mtx);
    int toret = -2;
    while(count == 0){
    std::cv_status status = cv.wait_for(lock,std::chrono::milliseconds(milliseconds));
        if (status == std::cv_status::timeout)
        {
            toret = -1;
            break; //we won't wait any longer
        }
        if(status == std::cv_status::no_timeout)
        {
            toret = 0;
        }
    }

    count--; //TODO: check consistency with the other implementations: will the sempahore be decreased in case of timeout?
    return toret;
}

CppSemaphore::~CppSemaphore()
{
}

} // namespace
