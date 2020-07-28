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
//
#include <cutils/atomic.h>
#include <cutils/log.h>
#include <cutils/properties.h>
//
//#include <gui/SensorManager.h>
#include <utils/Looper.h>
//#include <gui/Sensor.h>
//
using namespace android;
//
#include <Log.h>
#include <common.h>
#include <v1/camutils/CamMisc.h>
//
#include <utils/SensorListener.h>
#include <SensorListenerImp.h>
//-----------------------------------------------------------------------------
//android::Mutex android::SensorManager::sLock;
//std::map<String16, SensorManager*> android::SensorManager::sPackageInstances;

#define PHASEOUT 1

bool
setThreadPriority(
    int policy,
    int priority)
{
    return true;
}
//-----------------------------------------------------------------------------
bool
getThreadPriority(
    int& policy,
    int& priority)
{
    return true;
}
//-----------------------------------------------------------------------------
int SensorListenerImp_callback(
    int     fd,
    int     events,
    void*   pData)
{
    return 1;
}
//-----------------------------------------------------------------------------
SensorListenerImp::
SensorListenerImp()
{
}
//----------------------------------------------------------------------------
SensorListenerImp::
~SensorListenerImp()
{
    FUNCTION_NAME;
}
//-----------------------------------------------------------------------------
SensorListener*
SensorListener::
createInstance(void)
{
	return NULL;
}
//----------------------------------------------------------------------------
MVOID
SensorListenerImp::
destroyInstance(void)
{
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
setListener(Listener func)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
enableSensor(
    SensorTypeEnum  sensorType,
    MUINT32         periodInMs)
{
	return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
disableSensor(SensorTypeEnum sensorType)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
init(void)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
uninit(void)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
getEvents(void)
{
    return MTRUE;
}
//----------------------------------------------------------------------------
MBOOL
SensorListenerImp::
isSupport(void)
{
	return MTRUE;
}


