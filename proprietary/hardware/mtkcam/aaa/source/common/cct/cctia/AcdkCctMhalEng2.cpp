#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <utils/Errors.h>
#include <utils/threads.h>
#include <utils/String8.h>
#include <utils/KeyedVector.h>
#include <cutils/properties.h>
#include <semaphore.h>

	//#include <mtkcam/v1/config/PriorityDefs.h>
	//./include/mtkcam/middleware/v1/PriorityDefs.h
	//./include/mtkcam/def/PriorityDefs.h
#include <mtkcam/middleware/v1/PriorityDefs.h>

//#include <cutils/pmem.h>

	//#include "mtkcam/acdk/AcdkTypes.h"
//#include "mtkcam/main/acdk/AcdkTypes.h"
#include "AcdkErrCode.h"
#include "AcdkLog.h"
	//#include "mtkcam/acdk/AcdkCommon.h"
//#include "mtkcam/main/acdk/AcdkCommon.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
	//#include "AcdkBase.h"
#include "AcdkCctBase.h"
	//#include "AcdkMhalBase.h"
#include "AcdkCctMhalBase.h"
#include "AcdkUtility.h"
using namespace NSACDK;
	//using namespace NSAcdkMhal;
using namespace NSAcdkCctMhal;
using namespace android;

//#include <binder/IMemory.h>
//#include <binder/MemoryBase.h>
//#include <binder/MemoryHeapBase.h>
#include <utils/RefBase.h>
#include <system/camera.h>
#include <hardware/camera.h>
#include <dlfcn.h>
#include <mtkcam/utils/fwk/MtkCamera.h>

	//#include <mtkcam/utils/common.h> //MBOOL
#include <utils/String8.h>
#include <device/depend/DefaultCameraDevice1.h>
#include "AcdkCctMhalEng2.h"
sp<NSCam::DefaultCameraDevice1> mCameraDevice;

static void* gCamLib = NULL;
#if (PLATFORM_SDK_VERSION >= 21)
#define CAM_PATH "hw/android.hardware.camera.provider@2.4-impl-mediatek.so"
#else
    #ifdef __LP64__
        //#pragma message("acdk 64 bits")
        #define CAM_PATH "/vendor/lib64/hw/android.hardware.camera.provider@2.4-impl-mediatek.so"
    #else
        //#pragma message("acdk 32 bits")
        #define CAM_PATH "/vendor/lib/hw/android.hardware.camera.provider@2.4-impl-mediatek.so"
    #endif
#endif

void create_CameDev (string instanceName, int instanceID)
{
    if ( ! gCamLib )
    {
        gCamLib = ::dlopen(CAM_PATH, RTLD_NOW);
        if  ( ! gCamLib )
        {
            char const *err_str = ::dlerror();
            ACDK_LOGE("dlopen: %s error=%s", CAM_PATH, (err_str ? err_str : "unknown"));
        }
    }

    typedef NSCam::ICameraDeviceManager * (*pfnEntry_T)();
    pfnEntry_T pfnEntry = (pfnEntry_T)::dlsym(gCamLib, "getCameraDeviceManager");
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        ACDK_LOGE("dlsym: %s error=%s getModuleLib:%p", "getCameraDeviceManager", (err_str ? err_str : "unknown"), gCamLib);
        return;
    }

    NSCam::ICameraDeviceManager *tmp_dev_mgr = pfnEntry();
    String8 rDevName = String8("MtkEng");

    mCameraDevice= new NSCam::DefaultCameraDevice1(
                                     tmp_dev_mgr,
                                     instanceName,
                                     instanceID,
                                     rDevName);
}
void close_CameDev ()
{
    //mCameraDevice->onCloseLocked();
    mCameraDevice->close();

    mCameraDevice = NULL;

    if (gCamLib){
        ::dlclose(gCamLib);
        gCamLib = NULL;
    }
}
//void open_CameDev (ICameraDeviceCallback *cam_dev_cb)
void open_CameDev (const sp<ICameraDeviceCallback>& cam_dev_cb, camera_notify_callback notify_cb, camera_data_callback data_cb, camera_data_timestamp_callback data_cb_timestamp, camera_request_memory get_memory)
{
    mCameraDevice->open(cam_dev_cb, notify_cb, data_cb, data_cb_timestamp, get_memory);
}

void setParameters_CameDev (string param)
{
    mCameraDevice->setParameters(param);
}
//using getParameters_cb = std::function<void(const ::android::hardware::hidl_string& parms)>;
string getParamString;
void __getParamCallback (const ::android::hardware::hidl_string& parms)
{
    getParamString = (string)parms;
}
NSCam::CameraDevice1Base::getParameters_cb getParamCallback = __getParamCallback;
void getParameters_CameDev (string &param)
{
    mCameraDevice->getParameters(getParamCallback);
    param = getParamString;
}
void enableMsgType_CameDev (uint32_t msgType)
{
    mCameraDevice->enableMsgType(msgType);
}

void startPreview_CameDev ()
{
    mCameraDevice->startPreview();
}
void stopPreview_CameDev ()
{
    mCameraDevice->stopPreview();
}
void takePicture_CameDev ()
{
    mCameraDevice->takePicture();
}








