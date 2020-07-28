#define LOG_TAG "MtkCam/SensorListener"
//
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <sys/mman.h>
//
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <utils/Looper.h>
//
#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>
//
#include <string>
//
using namespace android;
using std::string;

//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/Trace.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/sys/SensorListener.h>
//-----------------------------------------------------------------------------
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }
//
#define FUNCTION_NAME               MY_LOGD("")
#define FUNCTION_IN                 MY_LOGD("+")
#define FUNCTION_OUT                MY_LOGD("-")
//-----------------------------------------------------------------------------
class SensorListenerImpThread : public Thread
{
    public:
        SensorListenerImpThread(Looper*) : Thread(false)
        {
        }
        //
        ~SensorListenerImpThread()
        {
        }
        //
        status_t readyToRun()
        {
            return NO_ERROR;
        }
        //
        virtual bool threadLoop()
        {
            return true;
        }
        // force looper wake up
        void wake() {
        }
};
//-----------------------------------------------------------------------------
class SensorListenerImp : public SensorListener
{
    public:
        SensorListenerImp(const char* Caller="Unknown");
        ~SensorListenerImp();
        //
        virtual void    destroyInstance(void);
        virtual MBOOL   setListener(Listener func);
        virtual MBOOL   enableSensor(
                            SensorTypeEnum  sensorType,
                            MUINT32         periodInMs);
        virtual MBOOL   disableSensor(SensorTypeEnum sensorType);
        //
        virtual MBOOL   init(void);
        virtual MBOOL   uninit(void);
        virtual MBOOL   getEvents(void);
        virtual MBOOL   isSupport(void);
};
//-----------------------------------------------------------------------------
int SensorListenerImp_callback(
    int     /*fd*/,
    int     /*events*/,
    void*   )
{
    return 1;
}
//-----------------------------------------------------------------------------
SensorListenerImp::
SensorListenerImp(const char*)
{
}
//----------------------------------------------------------------------------
SensorListenerImp::
~SensorListenerImp()
{
}
//-----------------------------------------------------------------------------
SensorListener*
SensorListener::
createInstance(const char* )
{
    return NULL;
}


//----------------------------------------------------------------------------
MVOID
SensorListenerImp::
destroyInstance(void)
{
    delete this;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
setListener(Listener)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
enableSensor(
    SensorTypeEnum  /*sensorType*/,
    MUINT32         /*periodInMs*/)
{
    return 0;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
disableSensor(SensorTypeEnum /*sensorType*/)
{
    return 0;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
init(void)
{
    return 0;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
uninit(void)
{
    return 0;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
getEvents(void)
{
    return 0;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
isSupport(void)
{
    return MFALSE;
}

