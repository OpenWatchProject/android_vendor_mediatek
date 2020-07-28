#ifndef __DP_MUTEX_ANDROID_H__
#define __DP_MUTEX_ANDROID_H__

// Use Android system mutex
#include "Mutex.h"

using namespace android;

class DpMutex: public Mutex
{
public:
    DpMutex()
    {
    }

    ~DpMutex()
    {
    }
};

#endif  // __DP_MUTEX_ANDROID_H__
