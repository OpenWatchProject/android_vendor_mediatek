
#include <utils/Log.h>


#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/SurfaceControl.h>
#include <ui/DisplayInfo.h>
#include <gui/Surface.h>


#include "Source.h"
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cutils/properties.h>

#include "NdkCameraError.h"
#include "NdkCameraManager.h"
#include "NdkCameraDevice.h"
#include "NdkCameraCaptureSession.h"
#include "NdkCameraMetadataTags.h"
#include "NdkCameraMetadata.h"
#include "NdkCaptureRequest.h"
#include "NdkImage.h"
#undef LOG_TAG
#define LOG_TAG "[VT][Source][test]"

namespace android
{
typedef struct CameraInfo {
    int32_t  max_width;
    int32_t  max_height;
    int32_t  max_fps;
    int32_t  min_fps;
    int32_t  sensor_degree;
    int32_t  facing;
} CameraInfo_t;

status_t getCameraCount(int32_t *CameraNumbers)
{
    ALOGD("%s", __FUNCTION__);
    status_t  pass = UNKNOWN_ERROR;
    ACameraManager* mgr = nullptr ;
    ACameraIdList *cameraIdList = nullptr;

    int numCameras = 0;
    mgr = ACameraManager_create();
    camera_status_t ret = ACameraManager_getCameraIdList(mgr, &cameraIdList);

    if(ret != ACAMERA_OK || cameraIdList == nullptr) {
        ALOGE("Get camera id list failed: ret %d, cameraIdList %p",ret, cameraIdList);
        goto cleanup;
    }

    *CameraNumbers = cameraIdList->numCameras;
    ALOGD("Number of cameras: %d", *CameraNumbers);

cleanup:

    if(mgr) {
        ACameraManager_delete(mgr);
        mgr = nullptr;
    }

    if(cameraIdList) {
        ACameraManager_deleteCameraIdList(cameraIdList);
        cameraIdList = nullptr;
    }

    return OK;
}
status_t getCameraInfo(int32_t *CameraNumbers,  CameraInfo_t* CameraInfo)
{

    ALOGD("%s", __FUNCTION__);
    status_t  pass = UNKNOWN_ERROR;
    ACameraManager* mgr = nullptr ;
    ACameraIdList *cameraIdList = nullptr;
    ACameraMetadata* chars = nullptr;
    int numCameras = 0;
    mgr = ACameraManager_create();
    camera_status_t ret = ACameraManager_getCameraIdList(mgr, &cameraIdList);

    if(ret != ACAMERA_OK || cameraIdList == nullptr) {
        ALOGE("Get camera id list failed: ret %d, cameraIdList %p",ret, cameraIdList);
        goto cleanup;
    }

    *CameraNumbers = cameraIdList->numCameras;
    ALOGD("Number of cameras: %d", *CameraNumbers);

    for(int i = 0; i < cameraIdList->numCameras; i++) {
        ALOGD("Camera ID: %s", cameraIdList->cameraIds[i]);
    }

    numCameras = cameraIdList->numCameras;

    for(int i = 0; i < numCameras; i++) {
        ret = ACameraManager_getCameraCharacteristics(
                  mgr, cameraIdList->cameraIds[i], &chars);

        if(ret != ACAMERA_OK) {
            ALOGE("Get camera characteristics failed: ret %d", ret);
            goto cleanup;
        }

        int32_t numTags = 0;
        const uint32_t* tags = nullptr;
        ret = ACameraMetadata_getAllTags(chars, &numTags, &tags);

        if(ret != ACAMERA_OK) {
            ALOGE("Get camera characteristics tags failed: ret %d", ret);
            goto cleanup;
        }

        for(int tid = 0; tid < numTags; tid++) {
            uint32_t tagId = tags[tid];
            ALOGV("%s capture request contains key %u", __FUNCTION__, tagId);
            uint32_t sectionId = tagId >> 16;
            ALOGV("tagId %u, sectionId %u", tagId, sectionId);

            if(sectionId >= ACAMERA_SECTION_COUNT && sectionId < ACAMERA_VENDOR) {
                ALOGE("Unknown tagId %u, sectionId %u", tagId, sectionId);
                goto cleanup;
            }
        }

        ACameraMetadata_const_entry entry;

        ret = ACameraMetadata_getConstEntry(chars, ACAMERA_LENS_FACING, &entry);

        if(ret != ACAMERA_OK) {
            ALOGE("Get const available capabilities key failed. ret %d", ret);
            goto cleanup;
        }

        ALOGD("ACAMERA_LENS_FACING key: tag: %x, count %d,type %d, data %p",
              entry.tag, entry.count, entry.type, entry.data.u8);

        if(entry.tag != ACAMERA_LENS_FACING ||
                entry.count == 0 || entry.type != ACAMERA_TYPE_BYTE || entry.data.u8 == nullptr) {

            goto cleanup;
        }

        CameraInfo[i].facing = * (entry.data.u8);
        ALOGE("CameraInfo[%d].facing %d", i,* (entry.data.u8));

        ret = ACameraMetadata_getConstEntry(chars, ACAMERA_SENSOR_ORIENTATION, &entry);

        if(ret != ACAMERA_OK) {
            ALOGE("Get  ORIENTATION key failed. ret %d", ret);
            goto cleanup;
        }

        ALOGD("SENSOR_ORIENTATION key: tag: %x , count %u ,type %d, data %p",
              entry.tag, entry.count,entry.type, entry.data.i32);

        if(entry.tag != ACAMERA_SENSOR_ORIENTATION ||
                entry.count == 0 || entry.type != ACAMERA_TYPE_INT32|| entry.data.i32 == nullptr) {

            goto cleanup;
        }

        CameraInfo[i].sensor_degree= * (entry.data.i32);
        ALOGE("CameraInfo[%d].sensor_degree %d", i,* (entry.data.i32));

        ret = ACameraMetadata_getConstEntry(chars, ACAMERA_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES, &entry);


        if(ret != ACAMERA_OK) {
            ALOGE("Get FPS_RANGE key failed. ret %d", ret);
            goto cleanup;
        }

        ALOGD("TARGET_FPS_RANGES key: tag: %x , count %u ,type %d , data %p ",
              entry.tag, entry.count, entry.type, entry.data.i32);

        if(entry.tag != ACAMERA_CONTROL_AE_AVAILABLE_TARGET_FPS_RANGES ||

                entry.count == 0 || entry.type != ACAMERA_TYPE_INT32|| entry.data.i32 == nullptr) {
            goto cleanup;
        }

        const int TARGET_FPS_RANGES_SIZE = 2;

        const int TARGET_FPS_OFFSET = 1;

        int32_t minFps = entry.data.i32[0];

        int32_t maxFps = entry.data.i32[TARGET_FPS_OFFSET];

        for(uint32_t count = 0; count < entry.count; count+=TARGET_FPS_RANGES_SIZE) {
            ALOGD("count %d fps MIN %d ,MAX %d\n", count,entry.data.i32[count],entry.data.i32[count+TARGET_FPS_OFFSET]);

            if(minFps > entry.data.i32[count]) minFps = entry.data.i32[count];

            if(maxFps < entry.data.i32[count + TARGET_FPS_OFFSET]) maxFps = entry.data.i32[count + TARGET_FPS_OFFSET];
        }

        CameraInfo[i].min_fps = minFps;
        CameraInfo[i].max_fps = maxFps;

        ALOGD(" CameraInfo[%d].min_fps %d ,CameraInfo.max_fps %d\n", i,CameraInfo[i].min_fps,CameraInfo[i].max_fps);

        ret = ACameraMetadata_getConstEntry(chars, ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS, &entry);

        if(ret != ACAMERA_OK) {
            ALOGE("Get const available capabilities key failed. ret %d", ret);
            goto cleanup;
        }

        ALOGD("STREAM_CONFIGURATIONS key: tag: %x, count %u ,type %d , data %p ",
              entry.tag, entry.count,entry.type, entry.data.i32);

        if(entry.tag != ACAMERA_SCALER_AVAILABLE_STREAM_CONFIGURATIONS ||
                entry.count == 0 ||   entry.count %4  != 0
                || entry.type != ACAMERA_TYPE_INT32|| entry.data.i32 == nullptr) {

            goto cleanup;
        }

        const int STREAM_CONFIGURATION_SIZE = 4;

        const int STREAM_FORMAT_OFFSET = 0;

        const int STREAM_WIDTH_OFFSET = 1;

        const int STREAM_HEIGHT_OFFSET = 2;

        const int STREAM_IS_INPUT_OFFSET = 3;

        uint32_t YUV_count_index = 0;

        int32_t maxW =  0;

        int32_t maxH  =  0;

        for(uint32_t count =0; count < entry.count; count += STREAM_CONFIGURATION_SIZE) {
            int32_t format = entry.data.i32[count + STREAM_FORMAT_OFFSET];
            int32_t width = entry.data.i32[count + STREAM_WIDTH_OFFSET];
            int32_t height = entry.data.i32[count + STREAM_HEIGHT_OFFSET];
            int32_t isInput = entry.data.i32[count + STREAM_IS_INPUT_OFFSET];

            ALOGV("format %x width %d ,height %d,isInput%d\n", format,width,height,isInput);

            if(format == AIMAGE_FORMAT_YUV_420_888) {
                YUV_count_index = count;
                ALOGD("YUV_420_888  format %x width %d ,height %d,isInput%d\n", format,width,height,isInput);

                if(maxW < width)   maxW = width   ;

                if(maxH  < height)   maxH  = height ;
            }
        }

        CameraInfo[i].max_width = maxW;
        CameraInfo[i].max_height = maxH;
        ALOGD(" CameraInfo[%d].max_width %d ,max_height %d\n", i,CameraInfo[i].max_width,CameraInfo[i].max_height);

        ACameraMetadata_free(chars);
        chars = nullptr;

    }

    pass = OK;

    for(int32_t i = 0; i < *CameraNumbers ; i++) {
        ALOGD("CameraInfo[%d]  facing %d sensor_degree %d min_fps %d max_fps %d maxW %d ,maxH %d\n",
              i,
              CameraInfo[i].facing,CameraInfo[i].sensor_degree,
              CameraInfo[i].min_fps,CameraInfo[i].max_fps,
              CameraInfo[i].max_width,CameraInfo[i].max_height);
    }

cleanup:

    if(chars) {
        ACameraMetadata_free(chars);
        chars = nullptr;
    }

    if(mgr) {
        ACameraManager_delete(mgr);
        mgr = nullptr;
    }

    if(cameraIdList) {
        ACameraManager_deleteCameraIdList(cameraIdList);
        cameraIdList = nullptr;
    }

    ALOGI("%s %s", __FUNCTION__, pass == OK ? "pass" : "fail");

    return pass;

}
class CameraDeviceListener
{
public:
    static void onDisconnected(void* obj, ACameraDevice* device) {
        ALOGV("Camera %s is disconnected! obj %p", ACameraDevice_getId(device),obj);

        return;
    }

    static void onError(void* obj, ACameraDevice* device, int errorCode) {
        ALOGV("Camera %s receive error %d! obj %p", ACameraDevice_getId(device), errorCode,obj);

        return;
    }

private:

};
class CaptureSessionListener
{

public:
    static void onClosed(void* obj, ACameraCaptureSession *session) {
        // TODO: might want an API to query cameraId even session is closed?
        ALOGV("Session %p is closed! obj %p", session,obj);
        return;
    }

    static void onReady(void* /*obj*/, ACameraCaptureSession * /*session*/) {

    }

    static void onActive(void* /*obj*/, ACameraCaptureSession * /*session*/) {

    }

private:

};


struct testMA : public AHandler {
    enum {
        kWhatSourceNotify,
        kWhatSourceNotify1,
    };
    testMA() {
        ALOGD("new testMA");
    }
    void setSource(sp<Source> source) {
        mSource = source;
    }

    void setSource1(sp<Source> source) {
        mSource1 = source;
    }

protected:
    virtual ~testMA() {}
    void onMessageReceived(const sp<AMessage> &msg) {
#if 1
        ALOGV("testMA msg->what() %d ",msg->what());

        //CHECK(msg->what() == kWhatSourceNotify);
        if(msg->what() == kWhatSourceNotify) {
            int32_t what;
            CHECK(msg->findInt32("what",&what));

            if(what == Source::kWhatAccessUnit) {
                sp<ABuffer> buffer;
                msg->findBuffer("accessUnit", &buffer);
                char bufs[255];
                sprintf(bufs, "/sdcard/source.bin");
                FILE *fps = fopen(bufs, "ab");

                if(fps) {
                    fwrite((void *)(buffer->data()), 1, buffer->size(), fps);
                    fclose(fps);
                }

                printf(" get au size %zu\n",buffer->size());

            } else if(what == Source::kWhatError) {
                int32_t errorType;
                msg->findInt32("err", &errorType);
            }

        } else if(msg->what() == kWhatSourceNotify1) {
            int32_t what;
            CHECK(msg->findInt32("what",&what));

            if(what == Source::kWhatAccessUnit) {
                sp<ABuffer> buffer;
                msg->findBuffer("accessUnit", &buffer);
                char bufs[255];
                sprintf(bufs, "/sdcard/source1.bin");
                FILE *fps = fopen(bufs, "ab");

                if(fps) {
                    fwrite((void *)(buffer->data()), 1, buffer->size(), fps);
                    fclose(fps);
                }

                printf(" get au size %zu",buffer->size());
            } else if(what == Source::kWhatError) {
                int32_t errorType;
                msg->findInt32("err", &errorType);
            }
        }



#endif
    }
private:
    wp<Source> mSource;
    wp<Source> mSource1;

    DISALLOW_EVIL_CONSTRUCTORS(testMA);
};


}  // namespace android

int main(int argc, char **argv)
{
    using namespace android;

    ProcessState::self()->startThreadPool();
	status_t err;
	sp<SurfaceComposerClient> composerClient = new SurfaceComposerClient;
	err = composerClient->initCheck();
	if (err != NO_ERROR) {
		fprintf(stderr, "SurfaceComposerClient::initCheck error: %#x\n", err);
		return -1;
	}


    sp<IBinder> display = SurfaceComposerClient::getInternalDisplayToken();
    DisplayInfo info;
    SurfaceComposerClient::getDisplayInfo(display, &info);

    ssize_t displayWidth = 480;//info.w;
    ssize_t displayHeight = 640;//info.h;


    ALOGD("display is %zu x %zu %d %p \n", displayWidth, displayHeight,argc,argv);


	sp<SurfaceControl> sc = composerClient->createSurface(
			String8("Preview Surface"), displayWidth, displayHeight,
			PIXEL_FORMAT_RGBX_8888, ISurfaceComposerClient::eOpaque);
	if (sc == NULL || !sc->isValid()) {
		fprintf(stderr, "Failed to create SurfaceControl\n");
		return 0;
	}
	
	SurfaceComposerClient::Transaction{}
			.setLayer(sc, 0x7FFFFFFF)	  // always on top
			.show(sc)
			.apply();
	
	sp<Surface> surface = sc->getSurface();
	CHECK(surface != NULL);


    int32_t mCameraID = 1;

    int32_t CameraNumbers;

    getCameraCount(&CameraNumbers);
    CameraInfo_t CamInfo[CameraNumbers];

    getCameraInfo(&CameraNumbers,  CamInfo);

    ACameraManager* mgr = nullptr ;
    ACameraIdList *cameraIdList = nullptr;
    ACameraDevice * mpCameraDevice= nullptr;
    int numCameras = 0;
    mgr = ACameraManager_create();
    camera_status_t ret = ACameraManager_getCameraIdList(mgr, &cameraIdList);

    if(ret != ACAMERA_OK || cameraIdList == nullptr) {
        ALOGE("Get camera id list failed: ret %d, cameraIdList %p",ret, cameraIdList);
        //goto cleanup;
    }

    ALOGD("Camera numCameras: %d", cameraIdList->numCameras);

    for(int i = 0; i < cameraIdList->numCameras; i++) {
        ALOGD("Camera ID: %s", cameraIdList->cameraIds[i]);
    }

    CameraDeviceListener mDeviceListener ;
    ACameraDevice_StateCallbacks mDeviceCb {
        &mDeviceListener,
        CameraDeviceListener::onDisconnected,
        CameraDeviceListener::onError
    };

    ret =ACameraManager_openCamera(mgr,cameraIdList->cameraIds[mCameraID],&mDeviceCb,&mpCameraDevice);

    if(ret != ACAMERA_OK || mpCameraDevice == nullptr) {
        ALOGE("openCamera failed: ret %d ",ret);
        // goto cleanup;
    }

    ALOGD("openCamera  : ret %d ",ret);

#if 0

    ACaptureSessionOutputContainer* mOutputs = nullptr;
    ACaptureSessionOutput* mPreviewOutput = nullptr;
    ACameraCaptureSession* mSession = nullptr;
    ACaptureRequest* mPreviewRequest = nullptr;

    ret = ACaptureSessionOutputContainer_create(&mOutputs);

    if(ret != ACAMERA_OK) {
        ALOGE("Create capture session output container failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureSessionOutputContainer_create  : ret %d ",ret);

    ANativeWindow *nativeWindow = (ANativeWindow *)(surface.get());
    ret = ACaptureSessionOutput_create(nativeWindow,&mPreviewOutput);
    ALOGD("ACaptureSessionOutput_create  : ret %d ",ret);

    ret = ACaptureSessionOutputContainer_add(mOutputs, mPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("Sesssion preview output add failed! ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureSessionOutputContainer_add  : ret %d ",ret);
    CaptureSessionListener mSessionListener;
    ACameraCaptureSession_stateCallbacks mSessionCb {
        &mSessionListener,
        CaptureSessionListener::onClosed,
        CaptureSessionListener::onReady,
        CaptureSessionListener::onActive
    };

    ret = ACameraDevice_createCaptureSession(
              mpCameraDevice, mOutputs, &mSessionCb, &mSession);
    ALOGD("ACameraDevice_createCaptureSession  : ret %d ",ret);

    //ACaptureRequest* mPreviewRequest = nullptr;
    ret =ACameraDevice_createCaptureRequest(
             mpCameraDevice, TEMPLATE_PREVIEW, &mPreviewRequest);
    ALOGD("ACameraDevice_createCaptureRequest  : ret %d ",ret);

    ACameraOutputTarget* mReqPreviewOutput = nullptr;
    ret = ACameraOutputTarget_create(nativeWindow, &mReqPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("  create request preview output target failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACameraOutputTarget_create  : ret %d ",ret);
    ret = ACaptureRequest_addTarget(mPreviewRequest, mReqPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE(" add preview request output failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureRequest_addTarget  : ret %d ",ret);
    int previewSeqId;
    ret =ACameraCaptureSession_setRepeatingRequest(
             mSession, nullptr, 1, &mPreviewRequest, &previewSeqId);
    ALOGD("ACameraCaptureSession_setRepeatingRequest  : ret %d ",ret);
    usleep(5000000);


    //Seems 1st preview starts process end here
#endif

// TODO::
    printf("source test start <====== \r\n ");

    sp<testMA> mMA= new  testMA() ;
    sp<android::ALooper> testLooper = new android::ALooper;
    testLooper->setName("MA_looper");
    testLooper->start(
        false /* runOnCallingThread */,
        false /* canCallJava */,
        PRIORITY_AUDIO);

    testLooper->registerHandler(mMA);
    sp<AMessage> notify = new AMessage(testMA::kWhatSourceNotify, mMA);

    sp<Source> mSource = new Source(0,0,1);
    mSource->SetNotify(notify);
    mMA->setSource(mSource);

    bool testH264= true;
    char value[PROPERTY_VALUE_MAX];
	property_set("vendor.source.test","1");
    if(property_get("vendor.source.test.h264", value, NULL)) {
        testH264 =!strcmp("1", value) || !strcasecmp("true", value);
    }

    video_enc_fmt fmt;
    fmt.bitrate = 960000;
    fmt.framerate =30 ;
    testH264?fmt.mimetype = "video/avc":fmt.mimetype = "video/hevc";
    fmt.width  = 480;
    fmt.height = 640 ;
    fmt.IFrameIntervalSec = 4;
    fmt.profile = testH264 ? 66:1;
    fmt.level = 30;


    printf("vt0 -sensor 0 Start \n");
    mSource->setSourceConfig(&fmt);    //INIT

    sp<IGraphicBufferProducer> outBufferProducer;
    status_t retStatus = OK;
    mSource->setCurrentCameraInfo(CamInfo[mCameraID].facing,CamInfo[mCameraID].sensor_degree,3);    ///CamInfo[mCameraID].facing,CamInfo[mCameraID].sensor_degree);


    retStatus = mSource->getBufferQueueProducer(&outBufferProducer);

    ALOGD("producer %p", &outBufferProducer);

    sp<Surface> surfaceRecord = new Surface(outBufferProducer, true /* controlledByApp */);
    ANativeWindow *nativeWindowRecord = (ANativeWindow *)(surfaceRecord.get());

    ALOGD("nativeWindowRecord %p", nativeWindowRecord);

    ACaptureRequest* mRecordRequest  ;
    ACameraOutputTarget* mRecordOutput  ;
    ACaptureSessionOutputContainer* mRecordOutputs = nullptr;
    ACameraCaptureSession* mRecordSession = nullptr;
    ACaptureSessionOutput* mRecordCapSessionOutput = nullptr;

    mSource->Start();


    ret = ACaptureSessionOutputContainer_create(&mRecordOutputs);

    if(ret != ACAMERA_OK) {
        ALOGE("Create mRecordOutput session output container failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureSessionOutputContainer_create mRecordOutput : ret %d ",ret);



    ret = ACaptureSessionOutput_create(nativeWindowRecord,&mRecordCapSessionOutput);
    ALOGD("ACaptureSessionOutput_create  : ret %d ",ret);


#if 1 //add preview session

    ACaptureSessionOutput* mPreviewOutput = nullptr;


    ANativeWindow *nativeWindow = (ANativeWindow *)(surface.get());
    ret = ACaptureSessionOutput_create(nativeWindow,&mPreviewOutput);
    ALOGD("ACaptureSessionOutput_create mPreviewOutput : ret %d ",ret);

    ret = ACaptureSessionOutputContainer_add(mRecordOutputs, mPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("Sesssion preview output add failed! ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureSessionOutputContainer_add mPreviewOutput : ret %d ",ret);

#endif



    ret = ACaptureSessionOutputContainer_add(mRecordOutputs, mRecordCapSessionOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("Sesssion preview output add failed! ret %d", ret);
        return ret;
    }



    ALOGD("ACaptureSessionOutputContainer_add  : ret %d ",ret);
    CaptureSessionListener mRecordSessionListener;
    ACameraCaptureSession_stateCallbacks mSessionCbRecord {
        &mRecordSessionListener,
        CaptureSessionListener::onClosed,
        CaptureSessionListener::onReady,
        CaptureSessionListener::onActive
    };

    ret = ACameraDevice_createCaptureSession(
              mpCameraDevice, mRecordOutputs, &mSessionCbRecord, &mRecordSession);
    ALOGD("ACameraDevice_createCaptureSession  : ret %d ",ret);



    ret =ACameraDevice_createCaptureRequest(mpCameraDevice, TEMPLATE_RECORD, &mRecordRequest);
    ALOGD("ACameraDevice_createCaptureRequest  Record : ret %d ",ret);



#if 1 //add preview target

    ACameraOutputTarget* mReqPreviewOutput = nullptr;
    ret = ACameraOutputTarget_create(nativeWindow, &mReqPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("  create request preview output target failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACameraOutputTarget_create mReqPreviewOutput : ret %d ",ret);
    ret = ACaptureRequest_addTarget(mRecordRequest, mReqPreviewOutput);

    if(ret != ACAMERA_OK) {
        ALOGE(" add preview request output failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureRequest_addTarget mReqPreviewOutput : ret %d ",ret);
#endif

    ret = ACameraOutputTarget_create(nativeWindowRecord, &mRecordOutput);

    if(ret != ACAMERA_OK) {
        ALOGE("  create request Record output target failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACameraOutputTarget_create  Record: ret %d ",ret);
    ret = ACaptureRequest_addTarget(mRecordRequest, mRecordOutput);

    if(ret != ACAMERA_OK) {
        ALOGE(" add Record request output failed. ret %d", ret);
        return ret;
    }

    ALOGD("ACaptureRequest_addTarget Record  : ret %d ",ret);
    int recordSeqId = 0;
    ret =ACameraCaptureSession_setRepeatingRequest(mRecordSession, nullptr, 1, &mRecordRequest, &recordSeqId);
    ALOGD("ACameraCaptureSession_setRepeatingRequest Record: ret %d, recordId %d ",ret, recordSeqId);
    /*
    printf("sleep 5S \r\n");
    usleep(5000000);
    mSource->Stop();
    printf("stop and sleep 5s \r\n");
    usleep(5000000);
    mSource->Start();
    printf("start and sleep 5s \r\n");
    usleep(5000000);

    mSource->Pause();
    printf("Pause and sleep 5s \r\n");
    usleep(5000000);

    mSource->Resume();
    printf("Resume and sleep 5s \r\n");
    usleep(5000000);

    printf("update WH to 320 240  sleep 5s \r\n");


    fmt.width  = 240;
    fmt.height = 320 ;
    mSource->setSourceConfig (&fmt);//INIT
    usleep(5000000);

        printf("update WH to 640 480  sleep 5s \r\n");


    fmt.width  = 480;
    fmt.height = 640 ;
    mSource->setSourceConfig (&fmt);//INIT
    usleep(5000000);

    printf("stop sensor now,sleep 5s======>\r\n");
    ACameraCaptureSession_stopRepeating(mRecordSession);

    usleep(5000000);

    printf("start sensor again,sleep 5s======>\r\n");
    ret =ACameraCaptureSession_setRepeatingRequest(mRecordSession, nullptr, 1, &mRecordRequest, &recordSeqId);
    printf("ACameraCaptureSession_setRepeatingRequest Record: ret %d, recordId %d \r\n",ret, recordSeqId );

    usleep(5000000);

    printf("stop sensor again,sleep 5s======>\r\n");
    ACameraCaptureSession_stopRepeating(mRecordSession);
    usleep(5000000);

    printf("stop source now======>\r\n");
    ret =ACameraCaptureSession_setRepeatingRequest(mRecordSession, nullptr, 1, &mRecordRequest, &recordSeqId);
    //usleep(2000000);
    */

    usleep(2000000);

	fmt.bitrate = 800000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);
	
    fmt.bitrate = 600000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

    fmt.bitrate = 400000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

    fmt.bitrate = 200000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 100000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 50000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 100000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 200000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 400000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 600000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

	fmt.bitrate = 800000;
    mSource->setSourceConfig(&fmt);    //INIT
    usleep(1000000);

    mSource->Stop();

    printf("font test test end ======>\r\n");

    return 0;




}



