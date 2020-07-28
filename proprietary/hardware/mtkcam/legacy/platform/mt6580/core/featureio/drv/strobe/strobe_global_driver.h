#pragma once

#ifdef WIN32
#include "win_test.h"
#include "win_test_cpp.h"
#else
#include <utils/threads.h>
using namespace android;
#endif

class StrobeGlobalDriver
{
    private:
        int mStrobeHandle;
        int mUsers;
        mutable Mutex mLock;

        StrobeGlobalDriver();
        int openkd_nolock();
        int closekd_nolock();
        int sendCommand_nolock(int cmd, int typeId, int ctId, int arg);
        int sendCommandRet_nolock(int cmd, int typeId, int ctId, int *arg);

    public:
        static StrobeGlobalDriver *getInstance();
        int openkd();
        int closekd();
        int sendCommand(int cmd, int typeId, int ctId, int arg);
        int sendCommandRet(int cmd, int typeId, int ctId, int *arg);
        /* TODO init and uninit is not symmetric in this class */
        int init(int typeId, int ctId);
        int uninit(int typeId, int ctId);
};
