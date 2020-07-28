#ifndef __DP_CONDITION_ANDROID_H__
#define __DP_CONDITION_ANDROID_H__

// Use Android system condtion
#include "Condition.h"

using namespace android;

class DpCondition: public Condition
{
public:
    DpCondition()
    {
    }

    ~DpCondition()
    {
    }
};

#endif  // __DP_CONDITION_ANDROID_H__
