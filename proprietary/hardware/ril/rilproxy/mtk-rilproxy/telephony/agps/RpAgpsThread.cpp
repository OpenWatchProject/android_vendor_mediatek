/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBasics.h"
#include "RfxObject.h"
#include "RpAgpsThread.h"
#include "RpAgpsdAdapter.h"
#include "RpAgpsLog.h"

/*****************************************************************************
 * Class RfxMainThread
 *****************************************************************************/
RpAgpsThread::RpAgpsThread() : m_looper(NULL) {
    AGPS_LOGD("RpAgpsThread Construct 0x%x", this);
}


RpAgpsThread::~RpAgpsThread() {
    AGPS_LOGD("RpAgpsThread Destructor 0x%x", this);
}


sp<RpAgpsThread> RpAgpsThread::create() {
    AGPS_LOGD("RpAgpsThread::create");
    RpAgpsThread *t = new RpAgpsThread();
    t->run("RpAgpsThread");
    return t;
}


status_t RpAgpsThread::readyToRun() {
    AGPS_LOGD("RpAgpsThread::readyToRun");
    m_looper = Looper::prepare(0);
    RpAgpsdAdapter *adapter = RFX_OBJ_GET_INSTANCE(RpAgpsdAdapter);
    adapter->registerFd(m_looper);
    return android::NO_ERROR;
}


bool RpAgpsThread::threadLoop() {
    m_looper->pollAll(-1);
    return true;
}


sp<Looper> RpAgpsThread::getLooper() {
    return m_looper;
}