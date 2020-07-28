#ifndef __DP_SYNC_ANDROID_H__
#define __DP_SYNC_ANDROID_H__

#include "DpDataType.h"

#define DP_SYNC_TIMEOUT 3000
#define DP_SYNC_INC_STEP 1

class DpSync
{
public:
    DpSync();
    ~DpSync();

    void createFence(int32_t &fd, uint32_t val);

    void wait(int32_t fd, int32_t timeout = DP_SYNC_TIMEOUT);

    void wakeup(uint32_t inc = DP_SYNC_INC_STEP);

private:
    bool        createTimeline(void);

    int32_t     m_timelineFd;
    char        m_syncName[26];
};

#endif  // __DP_SYNC_ANDROID_H__
