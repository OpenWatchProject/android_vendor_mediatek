#ifndef __PQ_MUTEX_H__
#define __PQ_MUTEX_H__

// Use Android system mutex
#include "Mutex.h"

using namespace android;

class PQMutex: public Mutex
{
public:
    PQMutex()
    {
    }

    ~PQMutex()
    {
    }
};
#endif  // __PQ_MUTEX_H__