#define LOG_TAG "HdrEffectHal_test"
//#define LOG_NDEBUG 0

#include <gtest/gtest.h>

#include <unistd.h>
#include <utils/String8.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <utils/Vector.h>
#include <cutils/log.h>

#include <mtkcam/common.h>
#include <mtkcam/utils/Format.h>
//#include <mtkcam/drv/imem_drv.h>    // link libcamdrv
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/utils/ImageBufferHeap.h>
//#include "../HdrEffectHal.h"

#include <mtkcam/feature/hdr/1.0/IHdrProc.h>

// camera3test_fixtures.h, hardware.h,  camera3.h add for start camera3 preview
#include "camera3test_fixtures.h"
#include <hardware/hardware.h>
#include <hardware/camera3.h>

#define RUN_IN_CAM3_LOAD 1

using namespace android;
using namespace camera3;
using namespace tests;


extern int testVar1;
extern int testVar2;

namespace NSCam {

class HdrEffectTest : public ::testing::Test {
protected:

    HdrEffectTest() {}

    virtual void SetUp() {
        const ::testing::TestInfo* const testInfo =
            ::testing::UnitTest::GetInstance()->current_test_info();
        ALOGD("Begin test: %s.%s", testInfo->test_case_name(), testInfo->name());

        //@todo implement this
        mpHdrProc = IHdrProc::createInstance();
        mpCamera3Device = new android::camera3::tests::Camera3Device;

        //mListener = new EffectListener();
    	//1. init IMem
    	mpIMemDrv = IMemDrv::createInstance();
    	if(!mpIMemDrv) {
    		ALOGE("SetUp can't createInstance IMemDrv");
    	}
    	mpIMemDrv->init();	//check this, see fd

        mpIImageBufAllocator =  IImageBufferAllocator::getInstance();
        if (mpIImageBufAllocator == NULL)
        {
            ALOGE("mpIImageBufAllocator is NULL \n");
        }
        mHDRProcDone = MFALSE;
    }

    virtual void TearDown() {
        const ::testing::TestInfo* const testInfo =
            ::testing::UnitTest::GetInstance()->current_test_info();
        ALOGD("End test:   %s.%s", testInfo->test_case_name(), testInfo->name());

        //delete mpEffectHal;
        //delete mpEffectHalClient;
    }
    MBOOL allocBuffer(IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt);
    void deallocBuffer(IImageBuffer* pBuf);

    // prevent to use loadFromFile (Fatal,lockBufLocked)
    // one lockBuf already in allocBuffer
    uint32_t loadFileToBuf(char const*const fname, MUINT8*  buf, MUINT32 size);
    static MBOOL HDRProcCompleteCallback(MVOID* user,MBOOL ret);

protected:
    android::camera3::tests::Camera3Device * mpCamera3Device;
    //sp<EffectListener> mListener;
    IHdrProc             *mpHdrProc;
	IMemDrv             *mpIMemDrv;
    IImageBufferAllocator* mpIImageBufAllocator;
    typedef struct
    {
        IImageBuffer* pImgBuf;
        IMEM_BUF_INFO memBuf;
    } ImageBufferMap;
    std::vector<ImageBufferMap> mvImgBufMap;
    MBOOL           mHDRProcDone;
};



/******************************************************************************
*
*******************************************************************************/
MBOOL
HdrEffectTest::
allocBuffer(IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt)
{
    MBOOL ret = MTRUE;

    IImageBuffer* pBuf = NULL;

    if( fmt != eImgFmt_JPEG )
    {
        /* To avoid non-continuous multi-plane memory, allocate ION memory and map it to ImageBuffer */
        MUINT32 plane = NSCam::Utils::Format::queryPlaneCount(fmt);
        ImageBufferMap bufMap;

        bufMap.memBuf.size = 0;
        for (MUINT32 i=0; i<plane; i++) {
            bufMap.memBuf.size += ((NSCam::Utils::Format::queryPlaneWidthInPixels(fmt,i, w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(fmt,i) + 7) / 8) * NSCam::Utils::Format::queryPlaneHeightInPixels(fmt, i, h);
        }

        if(mpIMemDrv == NULL) {
            ALOGE("null mpIMemDrv");
            return MFALSE;
        }
        if (mpIMemDrv->allocVirtBuf(&bufMap.memBuf)) {
            ALOGE("g_pIMemDrv->allocVirtBuf() error \n");
            return MFALSE;
        }
        if (mpIMemDrv->mapPhyAddr(&bufMap.memBuf)) {
            ALOGE("mpIMemDrv->mapPhyAddr() error \n");
            return MFALSE;
        }
        ALOGD("allocBuffer at PA(%p) VA(%p) Size(0x%x)"
                , (void*)bufMap.memBuf.phyAddr
                , (void*)bufMap.memBuf.virtAddr
                , bufMap.memBuf.size
                );

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

        MUINT32 strideInBytes[3] = {0};
        for (MUINT32 i = 0; i < plane; i++) {
            strideInBytes[i] = (NSCam::Utils::Format::queryPlaneWidthInPixels(fmt,i, w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(fmt, i) + 7) / 8;
        }
        IImageBufferAllocator::ImgParam imgParam(fmt
                                                , MSize(w,h)
                                                , strideInBytes
                                                , bufBoundaryInBytes
                                                , plane
                                                );

        PortBufInfo_v1 portBufInfo = PortBufInfo_v1(bufMap.memBuf.memID
                                                    , bufMap.memBuf.virtAddr
                                                    , bufMap.memBuf.useNoncache
                                                    , bufMap.memBuf.bufSecu
                                                    , bufMap.memBuf.bufCohe
                                                    );

        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(LOG_TAG
                                                            , imgParam
                                                            , portBufInfo
                                                            );
        if(pHeap == 0) {
            ALOGE("pHeap is NULL");
            return MFALSE;
        }

        //
        pBuf = pHeap->createImageBuffer();
        pBuf->incStrong(pBuf);

        bufMap.pImgBuf = pBuf;
        mvImgBufMap.push_back(bufMap);
    }
    else
    {
        MINT32 bufBoundaryInBytes = 0;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(w,h),
                w * h * 6 / 5,  //FIXME
                bufBoundaryInBytes
                );

        if(mpIImageBufAllocator == NULL) {
            ALOGE("null mpIImageBufAllocator");
            return MFALSE;
        }
        pBuf = mpIImageBufAllocator->alloc_ion(LOG_TAG, imgParam);
    }
    if (!pBuf || !pBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ) )
    {
        ALOGE("Null allocated or lock Buffer failed\n");
        ret = MFALSE;

    }
    else
    {
        // flush
        pBuf->syncCache(eCACHECTRL_INVALID);  //hw->cpu
		ALOGD("allocBuffer addr(%p), width(%d), height(%d), format(0x%x)", pBuf, w, h, fmt);
        *ppBuf = pBuf;
    }
    // TODO remove this unlockbuf, if there is no loadFromFile() call behind
    //pBuf->unlockBuf(LOG_TAG);
lbExit:
    return ret;
}


/*******************************************************************************
*
*******************************************************************************/
void
HdrEffectTest::
deallocBuffer(IImageBuffer* pBuf)
{

    if(!pBuf) {
        ALOGD("free a null buffer");
        return;
    }

    pBuf->unlockBuf(LOG_TAG);
    switch (pBuf->getImgFormat())
    {
        case eImgFmt_JPEG:
            mpIImageBufAllocator->free(pBuf);
            break;
        //case eImgFmt_I422:
        //case eImgFmt_I420:
        default:
            pBuf->decStrong(pBuf);
            for (std::vector<ImageBufferMap>::iterator it = mvImgBufMap.begin();
                    it != mvImgBufMap.end();
                    it++)
            {
                if (it->pImgBuf == pBuf)
                {
                    mpIMemDrv->unmapPhyAddr(&it->memBuf);
                    if (mpIMemDrv->freeVirtBuf(&it->memBuf))
                    {
                        ALOGE("m_pIMemDrv->freeVirtBuf() error");
                    }
                    else
                    {
                        mvImgBufMap.erase(it);
                    }
                    break;
                }
            }
    }

    pBuf = NULL;

}


#if 1
uint32_t
HdrEffectTest::
loadFileToBuf(char const*const fname, MUINT8*  buf, MUINT32 size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    ALOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        ALOGE("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    ALOGD("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            ALOGE("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        if (nr == 0) {
            ALOGE("can't read from file [%s]", fname);
            break;
        }
        readCnt += nr;
        cnt++;
    }
    ALOGD("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}
#endif


/*******************************************************************************
*
*******************************************************************************/
MBOOL
HdrEffectTest::
HDRProcCompleteCallback(MVOID* user,MBOOL ret){

    HdrEffectTest * self = reinterpret_cast <HdrEffectTest*>(user);
    if (NULL == self ){
        return MFALSE;
    }
    ALOGD("HDRProcCompleteCallback ret:%d",ret);
    self->mHDRProcDone = ret;
    return MTRUE;
}

TEST_F(HdrEffectTest, HDRApplied) {

    ALOGD("HDRApplied +");

    char szSrcFileName[6][100];
    char szResultFileName[100];
    MUINT32 empty = 0;

//  camera 3 preview start
#if RUN_IN_CAM3_LOAD
    if(mpCamera3Device == NULL){
        ALOGE("mpCamera3Device is NULL \n");
    } else {
        ALOGD("mpCamera3Device SetUp");
        mpCamera3Device->SetUp();

        ALOGD("mpCamera3Device getNumOfCams:%d",mpCamera3Device->getNumOfCams());
        int id = 0;
        for (id = 0; id < mpCamera3Device->getNumOfCams(); id++) {
            if (!mpCamera3Device->isHal3Supported(id))
                break;
        }
        ALOGD("mpCamera3Device id:%d",id);
            mpCamera3Device->openCamera(0);
            //Camera init with callback
        ALOGD("mpCamera3Device init");
            mpCamera3Device->init();

        for (int i = CAMERA3_TEMPLATE_PREVIEW; i < CAMERA3_TEMPLATE_COUNT; i++) {
            const camera_metadata_t *request = /*mpCamera3Device->getCam3Device()*/mpCamera3Device->mDevice->ops->construct_default_request_settings(mpCamera3Device->mDevice, i);
            EXPECT_TRUE(request != NULL);
            EXPECT_LT((size_t)0, get_camera_metadata_entry_count(request));
            EXPECT_LT((size_t)0, get_camera_metadata_data_count(request));

            ALOGD("Template type %d:",i);
            dump_indented_camera_metadata(request, 0, 2, 4);
        }

    }
#endif
// camera 3 preview end

    ASSERT_NE((void*)NULL, mpHdrProc);
    ASSERT_EQ(MTRUE, mpHdrProc->init());

    mpHdrProc->setCompleteCallback(HDRProcCompleteCallback,this);
    // AE info should set before prepare, or we don't know how many HDR frame
    // will be processed.
    mpHdrProc->setParam(HDRProcParam_Set_AOEMode,0,empty);
    mpHdrProc->setParam(HDRProcParam_Set_MaxSensorAnalogGain,8192,empty);
    mpHdrProc->setParam(HDRProcParam_Set_MaxAEExpTimeInUS,500000,empty);
    mpHdrProc->setParam(HDRProcParam_Set_MinAEExpTimeInUS,500,empty);
    mpHdrProc->setParam(HDRProcParam_Set_ShutterLineTime,13139,empty);
    mpHdrProc->setParam(HDRProcParam_Set_MaxAESensorGain,65536,empty);
    mpHdrProc->setParam(HDRProcParam_Set_MinAESensorGain,1195,empty);
    mpHdrProc->setParam(HDRProcParam_Set_ExpTimeInUS0EV,20004,empty);
    mpHdrProc->setParam(HDRProcParam_Set_SensorGain0EV,1708,empty);
    mpHdrProc->setParam(HDRProcParam_Set_FlareOffset0EV,0,empty);
    mpHdrProc->setParam(HDRProcParam_Set_GainBase0EV,0,empty);
    mpHdrProc->setParam(HDRProcParam_Set_LE_LowAvg,0,empty);
    mpHdrProc->setParam(HDRProcParam_Set_SEDeltaEVx100,0,empty);
    MUINT32 u4Histogram[128] ={ 1620, 1000, 288, 246, 226, 157, 131, 179,
                                64, 72, 94, 106, 82, 109, 98, 91,
                                101, 99, 144, 118, 76, 49, 32, 26,
                                24, 31, 23, 22, 34, 42, 60, 66,
                                70, 87, 89, 78, 81, 95, 91, 94,
                                134, 134, 142, 144, 142, 121, 89, 67,
                                72, 55, 92, 135, 72, 21, 24, 19,
                                13, 18, 13, 7, 9, 8, 16, 18,
                                19, 10, 7, 8, 10, 9, 21, 22,
                                27, 24, 22, 14, 18, 17, 17, 14,
                                21, 19, 26, 16, 14, 7, 6, 5,
                                5, 4, 5, 4, 2, 3, 0, 2,
                                1, 1, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                0, 0, 0, 0, 0, 0, 0, 0,
                                };
    mpHdrProc->setParam(HDRProcParam_Set_Histogram,(uintptr_t)u4Histogram,empty);

    MUINT32 HDRFrameNum = 0;
    android::Vector < MUINT32 > vu4Eposuretime;
    android::Vector < MUINT32 > vu4SensorGain;
    android::Vector < MUINT32 > vu4FlareOffset;
    vu4Eposuretime.clear();
    vu4SensorGain.clear();
    vu4FlareOffset.clear();

    mpHdrProc->getHDRCapInfo(HDRFrameNum, vu4Eposuretime,vu4SensorGain,vu4FlareOffset);
    ALOGD("HDRFrameNum: %d",HDRFrameNum);


    MUINT32 pic_w = 2560;
    MUINT32 pic_h = 1440;
    String8 pic_format = String8("jpeg");
    MUINT32 th_w = 160;
    MUINT32 th_h = 128;
    MUINT32 th_q = 90;
    MUINT32 jpeg_q = 90;
    MUINT32 rot = 0;
    MRect   cropRegion(pic_w, pic_h);
    MUINT32 zoom = 100;
    //String8 sensor_capture_width = String8("4192");
    //String8 sensor_capture_height = String8("3104");


    mpHdrProc->setJpegParam( pic_w
                            ,pic_h
                            ,th_w
                            ,th_h
                            ,jpeg_q);

    mpHdrProc->setShotParam( pic_w
                            ,pic_h
                            ,rot
                            ,cropRegion
                            ,zoom);


    mpHdrProc->setParam(HDRProcParam_Set_transform
                        ,rot
                        ,0);

    mpHdrProc->setParam(HDRProcParam_Set_sensor_size
                        ,0
                        ,0);

    mpHdrProc->setParam(HDRProcParam_Set_sensor_type
                        ,1/*NSCam::SENSOR_TYPE_RAW*/
                        ,0);

    // should input 3A information and get cap info before prepare.
    mpHdrProc->prepare();


    MUINT32 uSrcMainFormat = 0;
    MUINT32 uSrcMainWidth = 0;
    MUINT32 uSrcMainHeight = 0;

    MUINT32 uSrcSmallFormat = 0;
    MUINT32 uSrcSmallWidth = 0;
    MUINT32 uSrcSmallHeight = 0;

    mpHdrProc->getParam(HDRProcParam_Get_src_main_format,uSrcMainFormat,empty);
    mpHdrProc->getParam(HDRProcParam_Get_src_main_size,uSrcMainWidth,uSrcMainHeight);
    mpHdrProc->getParam(HDRProcParam_Get_src_small_format,uSrcSmallFormat,empty);
    mpHdrProc->getParam(HDRProcParam_Get_src_small_size,uSrcSmallWidth,uSrcSmallHeight);

    MUINT32 u4SurfaceIndex[6];
    EImageFormat InputImageFormat[6];
    MUINT32 InputImageWidth[6] ;
    MUINT32 InputImageHeight[6] ;

    for (MUINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        u4SurfaceIndex[i]     =  i;
        if((i%2) == 0){
            InputImageFormat[i] = (EImageFormat)uSrcMainFormat;
            InputImageWidth[i]  =  uSrcMainWidth;
            InputImageHeight[i] =  uSrcMainHeight;
        } else {
            InputImageFormat[i] = (EImageFormat)uSrcSmallFormat;
            InputImageWidth[i]  =  uSrcSmallWidth;
            InputImageHeight[i] =  uSrcSmallHeight;
        }
        ALOGD("Surface[%d] Format[%d] size (%dX%d)",u4SurfaceIndex[i],InputImageFormat[i],InputImageWidth[i],InputImageHeight[i]);
    }

    IImageBuffer* SrcImgBuffer[6];
    EImageFormat DstimageFormat = eImgFmt_YUY2;
    MUINT32 DstimageWidth = pic_w;
    MUINT32 DstimageHeight = pic_h;
    IImageBuffer* DstImgBuffer;

    EImageFormat Dstimage_TB_Format = eImgFmt_Y800;
    MUINT32 Dstimage_TB_Width = 160;
    MUINT32 Dstimage_TB_Height = 128;
    IImageBuffer* DstImg_TB_Buffer;

    ALOGD("allocate source buffer");

    for (MUINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        ALOGD("allocBuffer [%d] , InputImageWidth[%d], InputImageHeight[%d], InputImageFormat[%d]",i, InputImageWidth[i], InputImageHeight[i], InputImageFormat[i]);

        allocBuffer(&SrcImgBuffer[i], InputImageWidth[i], InputImageHeight[i], InputImageFormat[i]);
        ASSERT_NE((void*)NULL, SrcImgBuffer[i]);
        if((i%2) == 0){
            sprintf(szSrcFileName[i], "/data/input/mpSourceImgBuf[%d]_%dx%d.i420",i,InputImageWidth[i],InputImageHeight[i]);
        } else {
            sprintf(szSrcFileName[i], "/data/input/mpSourceImgBuf[%d]_%dx%d.y",i,InputImageWidth[i],InputImageHeight[i]);
        }

    }

    // fill src buffer
    for (MUINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        if((i%2) == 0){
            loadFileToBuf(szSrcFileName[i],(MUINT8*) SrcImgBuffer[i]->getBufVA(0),SrcImgBuffer[i]->getBufSizeInBytes(0)+SrcImgBuffer[i]->getBufSizeInBytes(1)+SrcImgBuffer[i]->getBufSizeInBytes(2));
        }else{
            loadFileToBuf(szSrcFileName[i],(MUINT8*) SrcImgBuffer[i]->getBufVA(0),SrcImgBuffer[i]->getBufSizeInBytes(0));
        }
    }

    ALOGD("allocate dst buffer");
    allocBuffer(&DstImgBuffer, DstimageWidth, DstimageHeight, DstimageFormat);

    ALOGD("allocate dst tb buffer");
    allocBuffer(&DstImg_TB_Buffer, Dstimage_TB_Width, Dstimage_TB_Height, Dstimage_TB_Format);

    //ASSERT_EQ(MTRUE, DstImgBuffer->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN ));
    ASSERT_NE((void*)NULL, DstImgBuffer);
    ASSERT_NE((void*)NULL, DstImg_TB_Buffer);
    //
    // TODO: should set output format in parameter



    ALOGD("mpHdrProc start");
    ASSERT_EQ(MTRUE, mpHdrProc->start());

    ALOGD("mpHdrProc addOutputFrame (VA)addr:%p", DstImgBuffer->getBufVA(0));
    ASSERT_EQ(MTRUE, mpHdrProc->addOutputFrame(0,DstImgBuffer));
    ALOGD("mpHdrProc addOutputFrame (VA)addr:%p", DstImg_TB_Buffer->getBufVA(0));
    ASSERT_EQ(MTRUE, mpHdrProc->addOutputFrame(1,DstImg_TB_Buffer));

#if 1  // TEST

    ALOGD("mpHdrProc addInputFrame");

    for (MUINT32 i = 0; i < HDRFrameNum*2; i++)
    {
        ASSERT_EQ(MTRUE, mpHdrProc->addInputFrame(i,SrcImgBuffer[i]));
    }
#endif


    while(MFALSE == mHDRProcDone){
        sleep(1);
        ALOGD("wait for HDR process done");
    }
    ALOGD("!!! HDR process done");

    ::sprintf(szResultFileName, "/sdcard/0000_10_DstImgBuffer_%dx%d.yuy2",DstimageWidth, DstimageHeight);
    DstImgBuffer->saveToFile(szResultFileName);
    ::sprintf(szResultFileName, "/sdcard/0000_10_DstImg_TB_Buffer_%dx%d.y", Dstimage_TB_Width, Dstimage_TB_Height);
    DstImg_TB_Buffer->saveToFile(szResultFileName);

    mpHdrProc->release();
    mpHdrProc->uninit();

#if 1 //TODO, should free
    ALOGD("deallocBuffer DstImgBuffer");
    for (MUINT32 i = 0; i < HDRFrameNum; i++)
    {
        deallocBuffer(SrcImgBuffer[i]);
    }
    ALOGD("deallocBuffer DstImgBuffer");
    deallocBuffer(DstImgBuffer);
    ALOGD("deallocBuffer DstImg_TB_Buffer");
    deallocBuffer(DstImg_TB_Buffer);
#endif

    //  camera 3 preview start

#if RUN_IN_CAM3_LOAD

    if(mpCamera3Device){
    ALOGD("closeCamera");
        mpCamera3Device->closeCamera();
    //ALOGD("TearDown");
        //mpCamera3Device->TearDown();
    }
#endif
    //  camera 3 preview end

    ALOGD("HDRApplied -");
}




} // namespace NSCam

#if 0   //@todo reopen this
#include <gtest/gtest.h>

//#define LOG_TAG "CameraFrameTest"
//#define LOG_NDEBUG 0
#include <utils/Log.h>

#include "hardware/hardware.h"
#include "hardware/camera2.h"

//#include <common/CameraDeviceBase.h>
#include <utils/StrongPointer.h>
#include <gui/CpuConsumer.h>
#include <gui/Surface.h>

#include <unistd.h>

#include "CameraStreamFixture.h"
#include "TestExtensions.h"

#define CAMERA_FRAME_TIMEOUT    1000000000 //nsecs (1 secs)
#define CAMERA_HEAP_COUNT       2 //HALBUG: 1 means registerBuffers fails
#define CAMERA_FRAME_DEBUGGING  0

using namespace android;
using namespace android::camera2;

namespace android {
namespace camera2 {
namespace tests {

static CameraStreamParams STREAM_PARAMETERS = {
    /*mFormat*/     CAMERA_STREAM_AUTO_CPU_FORMAT,
    /*mHeapCount*/  CAMERA_HEAP_COUNT
};

class CameraFrameTest
    : public ::testing::TestWithParam<int>,
      public CameraStreamFixture {

public:
    CameraFrameTest() : CameraStreamFixture(STREAM_PARAMETERS) {
        TEST_EXTENSION_FORKING_CONSTRUCTOR;

        if (!HasFatalFailure()) {
            CreateStream();
        }
    }

    ~CameraFrameTest() {
        TEST_EXTENSION_FORKING_DESTRUCTOR;

        if (mDevice.get()) {
            mDevice->waitUntilDrained();
        }
    }

    virtual void SetUp() {
        TEST_EXTENSION_FORKING_SET_UP;
    }
    virtual void TearDown() {
        TEST_EXTENSION_FORKING_TEAR_DOWN;
    }

protected:

};

TEST_P(CameraFrameTest, GetFrame) {

    TEST_EXTENSION_FORKING_INIT;

    /* Submit a PREVIEW type request, then wait until we get the frame back */
    CameraMetadata previewRequest;
    ASSERT_EQ(OK, mDevice->createDefaultRequest(CAMERA2_TEMPLATE_PREVIEW,
                                                &previewRequest));
    {
        Vector<int32_t> outputStreamIds;
        outputStreamIds.push(mStreamId);
        ASSERT_EQ(OK, previewRequest.update(ANDROID_REQUEST_OUTPUT_STREAMS,
                                            outputStreamIds));
        if (CAMERA_FRAME_DEBUGGING) {
            int frameCount = 0;
            ASSERT_EQ(OK, previewRequest.update(ANDROID_REQUEST_FRAME_COUNT,
                                                &frameCount, 1));
        }
    }

    if (CAMERA_FRAME_DEBUGGING) {
        previewRequest.dump(STDOUT_FILENO);
    }

    for (int i = 0; i < GetParam(); ++i) {
        ALOGD("Submitting capture request %d", i);
        CameraMetadata tmpRequest = previewRequest;
        ASSERT_EQ(OK, mDevice->capture(tmpRequest));
    }

    for (int i = 0; i < GetParam(); ++i) {
        ALOGD("Reading capture request %d", i);
        ASSERT_EQ(OK, mDevice->waitForNextFrame(CAMERA_FRAME_TIMEOUT));

        CaptureResult result;
        ASSERT_EQ(OK, mDevice->getNextResult(&result));

        // wait for buffer to be available
        ASSERT_EQ(OK, mFrameListener->waitForFrame(CAMERA_FRAME_TIMEOUT));
        ALOGD("We got the frame now");

        // mark buffer consumed so producer can re-dequeue it
        CpuConsumer::LockedBuffer imgBuffer;
        ASSERT_EQ(OK, mCpuConsumer->lockNextBuffer(&imgBuffer));
        ASSERT_EQ(OK, mCpuConsumer->unlockBuffer(imgBuffer));
    }

}

//FIXME: dont hardcode stream params, and also test multistream
INSTANTIATE_TEST_CASE_P(FrameParameterCombinations, CameraFrameTest,
    testing::Range(1, 10));


}
}
}
#endif
