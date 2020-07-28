/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#include "MTKAutorama.h"
#include "MTKMotion.h"
#include <cutils/properties.h>
#include "plugin/PipelinePluginType.h"
#include "mtk/mtk_platform_metadata_tag.h"
#include "mtk/mtk_feature_type.h"
#include "math.h"
#include <utils/metadata/IMetadata.h>
#include <utils/imgbuf/IGrallocImageBufferHeap.h>
#include <utils/imgbuf/IImageBuffer.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
#include "BufferUtils.h"
#include "LogUtils.h"
#define LOG_TAG "AutoramaPlugin"

using NSCam::NSPipelinePlugin::Interceptor;
using NSCam::NSPipelinePlugin::PipelinePlugin;
using NSCam::NSPipelinePlugin::PluginRegister;
using NSCam::NSPipelinePlugin::Yuv;
using NSCam::NSPipelinePlugin::YuvPlugin;

using namespace NSCam::NSPipelinePlugin;
using NSCam::MSize;
using NSCam::MERROR;

#define DUMP_BUFFER_AUTORAMA "debug.filter.dumpautorama.enabled"
static int gDumpBufferAutoramaEnabled = ::property_get_int32(DUMP_BUFFER_AUTORAMA, 0);

#define AUTORAMA_MAX_WIDTH                    (30720)                      // max output image width
#define MOTION_MAX_IN_WIDTH                   (320)                       // max input image width
#define MOTION_MAX_IN_HEIGHT                  (240)                       // max input image height
#define MOTION_PIPE_WORKING_BUFFER_SIZE       (MOTION_MAX_IN_WIDTH * MOTION_MAX_IN_HEIGHT * 3)
#define OVERLAP_RATIO                         (32)

using NSCam::EImageFormat;
using NSCam::eImgFmt_NV21;
using NSCam::eImgFmt_NV12;
using NSCam::eImgFmt_YV12;
using NSCam::eImgFmt_UNKNOWN;

class AutoramaPlugin : public YuvPlugin::IProvider
{
public:
    typedef YuvPlugin::Property Property;
    typedef YuvPlugin::Selection Selection;
    typedef YuvPlugin::Request::Ptr RequestPtr;
    typedef YuvPlugin::RequestCallback::Ptr RequestCallbackPtr;
    typedef YuvPlugin::ConfigParam ConfigParam;

private:
    struct AutoramaResultInfo{
        uintptr_t ImgBufferAddr;    // output image address, currently, only output YUV420
        MUINT16  ImgWidth;         // output image width
        MUINT16  ImgHeight;        // output image height
    };

    struct AutoramaMotionResult
    {
        /* motion tracking results */
        MINT16      MV_X;                   // horizontal accumulated motion
        MINT16      MV_Y;                   // vertical accumulated motion
        MBOOL       ReadyToShot;            // ready to shot flag (0/1 = not ready/ready)
        MINT32      RetCode;                // returned code of state machine
        MUINT8      ErrPattern;             // returned error/warning bit pattern
                                            // bit 0: low trust value (set if warning)
                                            // bit 1: large motion vector (set if warning)
                                            // bit 2: vertical shake (set if warning)
        MTKAUTORAMA_DIRECTION_ENUM Direction;   // panorama direction enum
    };

    static MVOID* PanoramathreadFunc(void *);
    pthread_t  PanoramaFuncThread;
    sem_t      PanoramaSemThread;
    sem_t      PanoramamergeDone;
    sem_t      PanoramaStitchCalc;
    MBOOL      mCancel = MFALSE;
    MBOOL      mInited = MFALSE;

    IImageBuffer*  mpFrameBuffer = nullptr;
    IImageBuffer*  mpMotionBuffer = nullptr;
    IImageBuffer*  mpPanoramaWorkingBuf = nullptr;

    int               mPanoramaFrameWidth = 0;
    int               mPanoramaFrameHeight = 0;
    int               mPanoramaFrameSize = 0;
    MTKAutorama* m_pMTKAutoramaObj = nullptr;
    MTKMotion*  m_pMTKMotionObj = nullptr;
    //MINT32      mAEValue;

    int32_t   mPanoramaFrameIdx = 0;
    int32_t   mPanoramaaddImgIdx = 0;
    int32_t   mStitchDir = MTKAUTORAMA_DIR_NO;
    AutoramaResultInfo *mpAutoramaResult;
    int mMaxShotNum = 9;
    int testcount = 0;

    MINT32 mCommand = AUTORAMA_UNKNOWN;

    //bool  mInplace = false;
    int    mOpenID1 = 0;
    int    mOpenID2 = 0;
    //bool  mUseSize = false;
    MSize mSize;

    template <typename T> inline bool updateEntry(IMetadata* metadata, const MUINT32 tag, const T& val)
    {
        if (metadata == NULL)
        {
            MY_LOGD("updateEntry pMetadata is NULL");
            return false;
        }

        IMetadata::IEntry entry(tag);
        entry.push_back(val, Type2Type<T>());
        metadata->update(tag, entry);
        return true;
    }

    template <class T> inline bool tryGetMetadata( IMetadata const *pMetadata, MUINT32 tag, T& rVal )
    {
        if(pMetadata == nullptr) return MFALSE;

        IMetadata::IEntry entry = pMetadata->entryFor(tag);
        if(!entry.isEmpty())
        {
            rVal = entry.itemAt(0,Type2Type<T>());
            return true;
        }
        else
        {
            MY_LOGW("No metadata %d ", tag);
        }
        return false;
    }

public:
    AutoramaPlugin();
    ~AutoramaPlugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    void config(const ConfigParam &param);
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);
private:
    void createAutoramaObject();
    MINT32 initAutorama(int autoramaFrameWidth,int autoramaFrameHeight, int shotNum);
    MINT32 uninitAutorama();
    bool stopAutorama(int cancel);
    MINT32 addImage();
    MINT32 CreateMotionSrc(MVOID * srcbufadr, int ImgWidth, int ImgHeight, MVOID * dstbufadr);
    MINT32 autoramaProcess(IImageBuffer * inBuffer, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot);
    MINT32 mHalCamFeatureMerge();
    MINT32 autoramaGetWokSize(int SrcWidth, int SrcHeight, int ShotNum, int &WorkingSize);
    MINT32 autoramaDoMotion(MUINT32* ImgSrc,MUINT32* MotionResult,MBOOL& isShot);
    MINT32 autoramaCalcStitch(void* SrcImg,MINT32 gEv,MTKAUTORAMA_DIRECTION_ENUM DIRECTION);
    MINT32 autoramaDoStitch();
    MINT32 autoramaGetResult(AutoramaResultInfo* ResultInfo);
};

AutoramaPlugin*   autoramaPluginObj;
sem_t      PanoramaAddImgDone;

AutoramaPlugin::AutoramaPlugin()
{
    FUNCTION_IN;
    autoramaPluginObj = this;
    mpAutoramaResult = new AutoramaResultInfo();
    PanoramaFuncThread = (pthread_t) NULL;
    PanoramaSemThread.count = 0;
    PanoramamergeDone.count = 0;
    PanoramaStitchCalc.count = 0;
    sem_init(&autoramaPluginObj->PanoramaSemThread, 0, 0);
    sem_init(&autoramaPluginObj->PanoramamergeDone, 0, 0);
    sem_init(&autoramaPluginObj->PanoramaStitchCalc, 0, 0);
    FUNCTION_OUT;
}

AutoramaPlugin::~AutoramaPlugin()
{
    FUNCTION_IN;
    mpAutoramaResult = nullptr;
    FUNCTION_OUT;
}

void AutoramaPlugin::set(MINT32 openID1, MINT32 openID2)
{
    FUNCTION_IN;
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
    FUNCTION_OUT;
}

const AutoramaPlugin::Property& AutoramaPlugin::property()
{
    FUNCTION_IN;
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK AUTORAMA";
        prop.mFeatures = MTK_FEATURE_AUTORAMA;
        inited = true;
    }
    FUNCTION_OUT;
    return prop;
}

MERROR AutoramaPlugin::negotiate(Selection &sel)
{
    FUNCTION_IN;
    MERROR ret = OK;

    //mIBufferFull: preview frame.
    sel.mIBufferFull.setRequired(MTRUE);
    sel.mIBufferFull
        .addAcceptedFormat(eImgFmt_YV12)
        .addAcceptedSize(MFALSE);

    sel.mOBufferFull.setRequired(MTRUE);
    sel.mOBufferFull
        .addAcceptedFormat(eImgFmt_YV12)
        .addAcceptedSize(eImgSize_Full);

    sel.mIMetadataApp.setRequired(MTRUE);
    sel.mIMetadataHal.setRequired(MFALSE);
    sel.mOMetadataApp.setRequired(MTRUE);
    sel.mOMetadataHal.setRequired(MFALSE);

    FUNCTION_OUT;
    return ret;
}

void AutoramaPlugin::init()
{
    FUNCTION_IN;
    //create algorithm object
    createAutoramaObject();
    FUNCTION_OUT;
}

void AutoramaPlugin::uninit()
{
    FUNCTION_IN;
    uninitAutorama();
    FUNCTION_OUT;
}

void AutoramaPlugin::config(const ConfigParam &param)
{
    FUNCTION_IN;
    FUNCTION_OUT;
}

void AutoramaPlugin::abort(std::vector<RequestPtr> &pRequests)
{
    FUNCTION_IN;
    (void)pRequests;
    FUNCTION_OUT;
};

MERROR AutoramaPlugin::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    FUNCTION_IN;
    (void)pCallback;
    MERROR ret = OK;
    MBOOL isShot;
    int32_t mvX = 0;
    int32_t mvY = 0;
    int32_t dir = (int32_t) (pRequest->mIBufferFull->acquire()->getTimestamp() / 1000000);
    IImageBuffer *in = NULL, *out = NULL;
    MY_LOGD("testcount=%d", testcount);

    IMetadata* pImetadata = pRequest->mIMetadataApp->acquire();
    IMetadata* pOmetadata = pRequest->mOMetadataApp->acquire();

    if(pRequest->mIBufferFull == NULL) {
        MY_LOGW("pRequest->mIBufferFull==NULL");
    } else {
        in = pRequest->mIBufferFull->acquire();
    }
    if(pRequest->mOBufferFull == NULL) {
        MY_LOGW("pRequest->mOBufferFull==NULL");
    }  else {
        out = pRequest->mOBufferFull->acquire();
    }

    MINT32 handle_state = mCommand;
    if (pImetadata == nullptr) {
        MY_LOGW("mIBufferFull->acquire()==NULL");
    } else {
        MY_LOGD("pImetadata->count() = %d", pImetadata->count());
        MINT32 cmd_state = AUTORAMA_UNKNOWN;
        if (pImetadata->count() > 0) {
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_AUTORAMA_CMD,
                    cmd_state);
            if (mCommand == AUTORAMA_QUEUE
                    && !(cmd_state == AUTORAMA_QUEUE
                            || cmd_state == AUTORAMA_CANCEL)) {
                MY_LOGD(
                        "MTK_POSTALGO_AUTORAMA_CMD wait for queue and ignore cmd %d",
                        cmd_state);
                handle_state = AUTORAMA_UNKNOWN;
            } else if (mCommand == AUTORAMA_UNKNOWN && cmd_state == AUTORAMA_CANCEL) {
                MY_LOGD(
                        "cancel autorama wait for AUTORAMA_UNKNOWN and ignore cmd %d",
                        cmd_state);
                handle_state = AUTORAMA_UNKNOWN;
            } else {
                handle_state = mCommand = cmd_state;
            }
        } else {
            handle_state = mCommand = AUTORAMA_UNKNOWN;
        }
    }
    MY_LOGD("MTK_POSTALGO_AUTORAMA_CMD value handle=%d, cmd = %d", handle_state,
            mCommand);

    // recevie APP stop cmd from metadata: 1: merged or not merged.
    if (handle_state == AUTORAMA_CANCEL) {
        stopAutorama(mCommand);
        updateEntry<MINT32>(pOmetadata, MTK_POSTALGO_AUTORAMA_CMD, AUTORAMA_CANCEL);
        mCommand = AUTORAMA_UNKNOWN;
        uninitAutorama();
    } else if (handle_state == AUTORAMA_MERGE) {
        stopAutorama(mCommand);
        updateEntry<MINT32>(pOmetadata, MTK_POSTALGO_AUTORAMA_CMD, AUTORAMA_MERGE);
        IMetadata::IEntry entry(MTK_POSTALGO_AUTORAMA_BUFFER_SIZE);
        entry.push_back(mpAutoramaResult->ImgWidth, Type2Type<MINT32>());
        entry.push_back(mpAutoramaResult->ImgHeight, Type2Type<MINT32>());
        pOmetadata->update(MTK_POSTALGO_AUTORAMA_BUFFER_SIZE, entry);
        mCommand = AUTORAMA_QUEUE;
    } else if (handle_state == AUTORAMA_QUEUE) {
        updateEntry<MINT32>(pOmetadata, MTK_POSTALGO_AUTORAMA_CMD, AUTORAMA_COMPLETE);
        MINT32 jpegRotation = 0;
        int buffer_length = mpAutoramaResult->ImgWidth * mpAutoramaResult->ImgHeight * 1.5;
        IImageBuffer* mpOutBuffer = BufferUtils::acquireWorkingBuffer(
                MSize(mpAutoramaResult->ImgWidth,mpAutoramaResult->ImgHeight),eImgFmt_YV12);
        memcpy((void *)(mpOutBuffer->getBufVA(0)),
                (void *)(mpPanoramaWorkingBuf->getBufVA(0)),
                buffer_length);
        IMetadata *pImetadata = pRequest->mIMetadataApp->acquire();
        if (pImetadata != nullptr && pImetadata->count() > 0) {
            tryGetMetadata<MINT32>(pImetadata, MTK_POSTALGO_JPEG_ORIENTATION,
                    jpegRotation);
        }
        MY_LOGD("MTK_POSTALGO_JPEG_ORIENTATION value %d", jpegRotation);
        BufferUtils::mdpResizeAndConvert(mpOutBuffer, out, jpegRotation);
        if (gDumpBufferAutoramaEnabled != 0) {
            char bufferName[128];
            sprintf(bufferName, "autorama_out_%d", mPanoramaaddImgIdx);
            MY_LOGD("bufferName=%s", bufferName);
            BufferUtils::dumpBuffer(out, bufferName);
        }
        mCommand = AUTORAMA_UNKNOWN;
        mpOutBuffer = nullptr;
        uninitAutorama();
    } else if (handle_state == AUTORAMA_START) {
        if (pRequest->mIBufferFull != NULL && pRequest->mOBufferFull != NULL) {
            MSize inSize, outSize;
            if (in == nullptr) {
                MY_LOGW("mIBufferFull->acquire()==NULL");
            } else {
                if (!mInited) {
                    inSize = in->getImgSize();
                    mPanoramaFrameWidth = inSize.w;
                    mPanoramaFrameHeight = inSize.h;
                    mPanoramaFrameSize = (mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2);
                    MY_LOGD("mPanoramaFrameWidth =%d, mPanoramaFrameHeight =%d, mPanoramaFrameSize =%d",
                                mPanoramaFrameWidth, mPanoramaFrameHeight, mPanoramaFrameSize);
                    //allocate buffer and Initial algorithm
                    initAutorama(mPanoramaFrameWidth, mPanoramaFrameHeight, mMaxShotNum);
                }
                //do motion.
                MRESULT Retcode = autoramaProcess(in, mvX, mvY, dir, isShot);
                if (S_AUTORAMA_OK != Retcode) {
                    MY_LOGD("autoramaProcess fail Retcode = %d", Retcode);
                }
                MRect region(MPoint(mvX,mvY),MPoint(dir,(int)isShot));
                updateEntry<MRect>(pOmetadata, MTK_POSTALGO_AUTORAMA_MOTION_DATA, region);
                updateEntry<MINT32>(pOmetadata, MTK_POSTALGO_AUTORAMA_CMD, AUTORAMA_START);
            }
        }
    }

    if (pCallback != nullptr) {
        MY_LOGD("callback request");
        pCallback->onCompleted(pRequest, 0);
    }
    testcount ++;
    MY_LOGD("exit AutoRama_PLUGIN ret=%d", ret);
    FUNCTION_OUT;
    return ret;
}

void AutoramaPlugin::createAutoramaObject()
{
    FUNCTION_IN;
    if (mpAutoramaResult == nullptr) {
        mpAutoramaResult = new AutoramaResultInfo();
    }
    if (autoramaPluginObj->m_pMTKAutoramaObj)
    {
        MY_LOGI("[createAutoramaObject] m_pMTKAutoramaObj Init has been called \n");
    }
    else
    {
        autoramaPluginObj->m_pMTKAutoramaObj = MTKAutorama::createInstance(DRV_AUTORAMA_OBJ_SW);
    }

    if (!autoramaPluginObj->m_pMTKAutoramaObj)
    {
        MY_LOGI("[createAutoramaObject] m_pMTKAutoramaObj Init failed\n");
    }

    if (m_pMTKMotionObj)
    {
        MY_LOGI("[createAutoramaObject] m_pMTKMotionObj Init has been called \n");
    }
    else
    {
        m_pMTKMotionObj = MTKMotion::createInstance(DRV_MOTION_OBJ_PANO);
    }

    if (!m_pMTKMotionObj)
    {
        MY_LOGI("[createAutoramaObject] m_pMTKMotionObj Init failed \n");
    }
    FUNCTION_OUT;
}

MINT32 AutoramaPlugin::initAutorama(int autoramaFrameWidth,int autoramaFrameHeight, int maxShotNum)
{
    FUNCTION_IN;
    MTKAutoramaEnvInfo AutoramaEnvInfo;
    MTKMotionEnvInfo MyMotionEnvInfo;
    MTKMotionTuningPara MyMotionTuningPara;
    MRESULT Retcode = S_AUTORAMA_OK;
    MINT32 initBufSize = 0;

    int iFOV_horizontal = 50;
    int iFOV_vertical = 50;

    // (1) Create frame buffer buffer: (preview size) * (max capture num)
    if (autoramaPluginObj->mpFrameBuffer == nullptr) {
        autoramaPluginObj->mpFrameBuffer = BufferUtils::acquireWorkingBuffer(MSize(autoramaFrameWidth, autoramaFrameHeight * maxShotNum),eImgFmt_YV12);
    }

    // (2) Create motion buffer: MOTION_PIPE_WORKING_BUFFER_SIZE.
    if (mpMotionBuffer == nullptr) {
        mpMotionBuffer = BufferUtils::acquireWorkingBuffer(MSize(MOTION_MAX_IN_WIDTH, MOTION_MAX_IN_HEIGHT * 3),eImgFmt_RGB565);
    }

    // (3) create Create working buffer buffer: the initBufSize algo return.
    createAutoramaObject();
    autoramaGetWokSize(autoramaFrameWidth, autoramaFrameHeight, maxShotNum, initBufSize);
    MY_LOGD("autorama working buffer size %d",initBufSize);
    if (mpPanoramaWorkingBuf == nullptr) {
        mpPanoramaWorkingBuf = BufferUtils::acquireWorkingBuffer(MSize(autoramaFrameWidth * maxShotNum, autoramaFrameHeight * 2),eImgFmt_YV12);
    }

    MUINT32 focalLengthInPixel = autoramaFrameWidth / (2.0 * tan(iFOV_horizontal/2.0/180.0*M_PI));

    MY_LOGI("[initAutorama] viewnalge (h,v)=(%d,%d) focalLengthInPixel=%d", iFOV_horizontal, iFOV_vertical,focalLengthInPixel);

    /*  Pano Driver Init  */
    /*  Set Pano Driver Environment Parameters */
    AutoramaEnvInfo.SrcImgWidth = autoramaFrameWidth;
    AutoramaEnvInfo.SrcImgHeight = autoramaFrameHeight;
    AutoramaEnvInfo.MaxPanoImgWidth = AUTORAMA_MAX_WIDTH;
    AutoramaEnvInfo.WorkingBufAddr = reinterpret_cast<MUINT8*>(mpPanoramaWorkingBuf->getBufVA(0));
    AutoramaEnvInfo.WorkingBufSize = initBufSize;
    AutoramaEnvInfo.MaxSnapshotNumber = maxShotNum;
    AutoramaEnvInfo.FixAE = 0;
    AutoramaEnvInfo.FocalLength = 750;//focalLengthInPixel;
    AutoramaEnvInfo.GPUWarp = 0;
    AutoramaEnvInfo.SrcImgFormat = MTKAUTORAMA_IMAGE_YV12;
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaInit(&AutoramaEnvInfo, 0);

    MyMotionEnvInfo.WorkingBuffAddr = reinterpret_cast<MUINT8*>(mpMotionBuffer->getBufVA(0));
    MyMotionEnvInfo.pTuningPara = &MyMotionTuningPara;
    MyMotionEnvInfo.SrcImgWidth = MOTION_MAX_IN_WIDTH;
    MyMotionEnvInfo.SrcImgHeight = MOTION_MAX_IN_HEIGHT;
    MyMotionEnvInfo.WorkingBuffSize = MOTION_PIPE_WORKING_BUFFER_SIZE;
    MyMotionEnvInfo.pTuningPara->OverlapRatio = OVERLAP_RATIO;
    m_pMTKMotionObj->MotionInit(&MyMotionEnvInfo, NULL);

    // (5) reset member parameter
    mPanoramaaddImgIdx = 0;
    mPanoramaFrameIdx = 0;
    mStitchDir = MTKAUTORAMA_DIR_NO;
    mInited = MTRUE;

    // (6) thread create
    sem_init(&autoramaPluginObj->PanoramaSemThread, 0, 0);
    sem_init(&autoramaPluginObj->PanoramamergeDone, 0, 0);
    sem_init(&autoramaPluginObj->PanoramaStitchCalc, 0, 0);
    sem_init(&PanoramaAddImgDone, 0, 0);
    MY_LOGD("pthread_create PanoramaFuncThread ");
    pthread_create(&PanoramaFuncThread, NULL, PanoramathreadFunc, this);
    FUNCTION_OUT;
    return Retcode;
}

MINT32 AutoramaPlugin::uninitAutorama()
{
    FUNCTION_IN;

    autoramaPluginObj->mpFrameBuffer = nullptr;
    mpMotionBuffer = nullptr;
    mpPanoramaWorkingBuf = nullptr;
    mpAutoramaResult = nullptr;
// TODO: need reback from AE limiter mode.
/**
    if (mpHal3A)
    {
        mpHal3A->send3ACtrl(E3ACtrl_SetAELimiterMode, false, 0);
        mpHal3A->detachCb(IHal3ACb::eID_NOTIFY_AE_RT_PARAMS, this);
        mpHal3A->destroyInstance("MTKPanorama");
        mpHal3A = NULL;
    }
**/
    if (m_pMTKMotionObj) {
        m_pMTKMotionObj->MotionExit();
        m_pMTKMotionObj->destroyInstance();
    }
    m_pMTKMotionObj = NULL;

    if (autoramaPluginObj->m_pMTKAutoramaObj) {
        autoramaPluginObj->m_pMTKAutoramaObj->AutoramaExit();
        autoramaPluginObj->m_pMTKAutoramaObj->destroyInstance();
    }
    autoramaPluginObj->m_pMTKAutoramaObj = NULL;
    mInited = MFALSE;
    autoramaPluginObj->mCancel = MFALSE;
    FUNCTION_OUT;
    return S_AUTORAMA_OK;
}

bool AutoramaPlugin::stopAutorama(int cancel)
{
    FUNCTION_IN;
    MY_LOGI("do merge %d mPanoramaaddImgIdx %d PanoramaNum %d", cancel,mPanoramaaddImgIdx,mMaxShotNum);
    autoramaPluginObj->mCancel = MTRUE;
    int err;
    sem_post(&autoramaPluginObj->PanoramaSemThread);
    pthread_join(PanoramaFuncThread, NULL);

    if(autoramaPluginObj->m_pMTKAutoramaObj)
    {
        if ((cancel == 1) || (mPanoramaaddImgIdx == mMaxShotNum))
        {
            // Do merge
            MY_LOGI("STOP_AUTORAMA: Merge Accidently ");
            sem_wait(&autoramaPluginObj->PanoramaStitchCalc);
            err = mHalCamFeatureMerge();
            sem_post(&autoramaPluginObj->PanoramamergeDone);
            if (err != S_AUTORAMA_OK)
            {
                MY_LOGE("  mHalCamFeatureMerge fail");
                return false;
            }
        }
        else
        {
            MY_LOGI("STOP_AUTORAMA: Cancel");
        }
    }else{
       MY_LOGE("AUTORAMA fail: m_pMTKAutoramaObj is NULL");
    }
    FUNCTION_OUT;
    return  true;
}

MINT32 AutoramaPlugin::addImage()
{
    FUNCTION_IN;
    MINT32 Retcode = S_AUTORAMA_OK;
    MY_LOGD("mPanoramaaddImgIdx %d PanoramaNum %d", mPanoramaaddImgIdx, mMaxShotNum);
    if (mPanoramaaddImgIdx >= mMaxShotNum){
        return Retcode;
    }
    if (autoramaPluginObj->mCancel)
    {
        MY_LOGI("exit mCancel %d", mCancel);
        return Retcode;
    }
    Retcode = autoramaCalcStitch((void*)(autoramaPluginObj->mpFrameBuffer->getBufVA(0) +
                            (mPanoramaFrameSize * mPanoramaaddImgIdx)),
                            mPanoramaaddImgIdx,
                            (MTKAUTORAMA_DIRECTION_ENUM)mStitchDir);

    if ( Retcode != S_AUTORAMA_OK) {
         MY_LOGE("addImage(): ret %d", Retcode);
         return Retcode;
     }
    sem_post(&autoramaPluginObj->PanoramaStitchCalc);
    mPanoramaaddImgIdx++;
    FUNCTION_OUT;
    return Retcode;
}

MINT32 AutoramaPlugin::autoramaProcess(IImageBuffer * inBuffer, int32_t& mvX, int32_t& mvY, int32_t& dir, MBOOL& isShot)
{
    FUNCTION_IN;
    MINT32 err = S_AUTORAMA_OK;
    isShot = false;

    AutoramaMotionResult PanoramaResult;
    memset((void*)&PanoramaResult,0,sizeof(AutoramaMotionResult));
    //use MDP to resize buffer for mpMotionBuffer
    BufferUtils::mdpResizeBuffer(inBuffer, MOTION_MAX_IN_WIDTH, MOTION_MAX_IN_HEIGHT, mpMotionBuffer);
    err = autoramaDoMotion((MUINT32 *)mpMotionBuffer->getBufVA(0), (MUINT32 *) &PanoramaResult, isShot);

    if ( err != S_AUTORAMA_OK ) {
        MY_LOGE("autoramaDoMotion fail");
        return err;
    }
    MY_LOGD("isShot: %d",isShot);

    mStitchDir = (MTKAUTORAMA_DIRECTION_ENUM)PanoramaResult.Direction;
    if (mPanoramaFrameIdx == 0)
    {
       isShot = true;
       PanoramaResult.MV_X = 0;
       PanoramaResult.MV_Y = 0;
       mStitchDir = MTKAUTORAMA_DIR_NO;
       //mpMhalCam->mHalCam3ACtrl(1, 0, 1);
       //mpHal3AObj->setAutoExposureLock(0);
    }

    MY_LOGD("PanoramaResult.MV X: %d Y: %d dir %d isShot %d ", PanoramaResult.MV_X, PanoramaResult.MV_Y, mStitchDir, isShot);
    if (isShot)
    {
        if (mPanoramaFrameIdx < mMaxShotNum)
        {
            MY_LOGD("mPanoramaFrameIdx %d mMaxShotNum %d", mPanoramaFrameIdx,mMaxShotNum);

            MUINT8 *pbufIn = (MUINT8 *) inBuffer->getBufVA(0);
            MUINT8 *pbufOut = (MUINT8 *) (autoramaPluginObj->mpFrameBuffer->getBufVA(0) + (mPanoramaFrameSize * mPanoramaFrameIdx));
            memcpy(pbufOut, pbufIn, (mPanoramaFrameWidth * mPanoramaFrameHeight * 3 / 2) );
            mPanoramaFrameIdx++;
            sem_post(&autoramaPluginObj->PanoramaSemThread);
        }
        else
        {
            MY_LOGD("mPanoramaFrameIdx %d", mPanoramaFrameIdx);
            mPanoramaFrameIdx++;
        }
    }
    mvX = PanoramaResult.MV_X;
    mvY = PanoramaResult.MV_Y;
    dir = mStitchDir;
    FUNCTION_OUT;

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID* AutoramaPlugin::PanoramathreadFunc(void *)
{
    FUNCTION_IN;

    ::prctl(PR_SET_NAME,"PanoTHREAD", 0, 0, 0);

    // (1) set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
#if 1
    // loop for thread until access uninit state
    MY_LOGD("autoramaPluginObj->mCancel: %d", autoramaPluginObj->mCancel);
    while(!autoramaPluginObj->mCancel)
    {
        int SemValue;
        sem_getvalue(&autoramaPluginObj->PanoramaSemThread, &SemValue);
        MY_LOGD("wait PanoramaSemThread, Semaphone value: %d", SemValue);
        sem_wait(&autoramaPluginObj->PanoramaSemThread);
        MY_LOGD("get PanoramaSemThread Semaphone");
        MINT32 err = autoramaPluginObj->addImage();
        if (err != S_AUTORAMA_OK) {
             MY_LOGD("addImage fail");
        }
        MY_LOGD("after do merge");
    }
    sem_post(&PanoramaAddImgDone);
#endif
    FUNCTION_OUT;
    return NULL;
}

/*******************************************************************************
* when all image was added done, stitch all the image into one merged picture, and return the image info.
********************************************************************************/
MINT32 AutoramaPlugin::mHalCamFeatureMerge()
{
    FUNCTION_IN;

    MINT32 err = S_AUTORAMA_OK;
    sem_wait(&PanoramaAddImgDone);
    MY_LOGD("autoramaDoStitch");
    err = autoramaDoStitch();
    if ( err != S_AUTORAMA_OK) {
        return err;
    }

    MY_LOGD("autoramaGetResult");
    err = autoramaGetResult(mpAutoramaResult);
    if ( err != S_AUTORAMA_OK) {
        return err;
    }
    MY_LOGI(" ImgWidth %d ImgHeight %d",
        mpAutoramaResult->ImgWidth, mpAutoramaResult->ImgHeight);
    FUNCTION_OUT;
    return err;
}

MINT32
AutoramaPlugin::autoramaGetWokSize(int SrcWidth, int SrcHeight, int MaxShotNum, int &WorkingSize)
{
    FUNCTION_IN;
    MTKAutoramaGetEnvInfo AutoramaGetEnvInfo;
    MINT32 Retcode = S_AUTORAMA_OK;

    AutoramaGetEnvInfo.ImgWidth          = SrcWidth;
    AutoramaGetEnvInfo.ImgHeight         = SrcHeight;
    AutoramaGetEnvInfo.MaxSnapshotNumber = MaxShotNum;
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_GET_ENV_INFO, 0, &AutoramaGetEnvInfo);
    if(Retcode!=S_AUTORAMA_OK)
    {
        MY_LOGD("[autoramaGetWokSize] MTKAUTORAMA_FEATURE_GET_RESULT Fail\n");
    }

    WorkingSize = AutoramaGetEnvInfo.WorkingBuffSize;
    FUNCTION_OUT;
    return Retcode;
}

/*******************************************************************************
* Motion algo do accordign the frame buffer when phone moving.
********************************************************************************/
MINT32 AutoramaPlugin::autoramaDoMotion(MUINT32* ImgSrc,MUINT32* MotionResult,MBOOL &isShot)
{
    FUNCTION_IN;
    MINT32 err = S_AUTORAMA_OK;
    MTKMotionProcInfo MotionInfo;

    if (!m_pMTKMotionObj) {
        err = E_AUTORAMA_ERR;
        MY_LOGE("Err, Init has been called \n");

        return err;
    }
    MotionInfo.ImgAddr = (void *)ImgSrc;
    MY_LOGD("ImgAddr 0x%p\n",(void*)MotionInfo.ImgAddr);
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_SET_PROC_INFO, &MotionInfo, NULL);
    m_pMTKMotionObj->MotionMain();
    m_pMTKMotionObj->MotionFeatureCtrl(MTKMOTION_FEATURE_GET_RESULT, NULL, MotionResult);

    isShot = ((AutoramaMotionResult*)MotionResult)->ReadyToShot > 0 ? true : false;
    MY_LOGD("shot: %d",isShot);
/**
    if(shot && mpHal3A)
        mpHal3A->send3ACtrl(E3ACtrl_SetAELimiterMode, true, 0);
**/
    FUNCTION_OUT;
    return err;
}

/*******************************************************************************
* Used in addimage.
********************************************************************************/
MINT32 AutoramaPlugin::autoramaCalcStitch(void* SrcImg,MINT32 gEv,MTKAUTORAMA_DIRECTION_ENUM DIRECTION)
{
    FUNCTION_IN;
    MINT32 Retcode = S_AUTORAMA_OK;
    MTKAutoramaProcInfo AutoramaProcInfo;
// TODO: need set AE limiter mode.
#if 0
    //For first capture
    if(gEv==0 && mpHal3A)
        mpHal3A->send3ACtrl(E3ACtrl_SetAELimiterMode, true, 0);

    MY_LOGI("[autoramaCalcStitch] Panorama EV %d",mAEValue);
    gImgEV[gEv] = mAEValue;
#endif
    AutoramaProcInfo.AutoramaCtrlEnum = MTKAUTORAMA_CTRL_ADD_IMAGE;
    AutoramaProcInfo.SrcImgAddr = SrcImg;
    AutoramaProcInfo.EV = 0;//gImgEV[gEv];
    AutoramaProcInfo.StitchDirection = DIRECTION;
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_SET_PROC_INFO, &AutoramaProcInfo, 0);
    if(Retcode!=S_AUTORAMA_OK)
    {
          MY_LOGE("MTKAUTORAMA_FEATURE_SET_PROC_INFO Fail \n");
          return Retcode;
    }
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaMain();
    if(Retcode!=S_AUTORAMA_OK)
    {
          MY_LOGE("AutoramaMain Fail\n");
    }
    FUNCTION_OUT;
    return Retcode;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 AutoramaPlugin::autoramaDoStitch()
{
    FUNCTION_IN;
    MINT32 Retcode = S_AUTORAMA_OK;
    MTKAutoramaProcInfo AutoramaProcInfo;

    AutoramaProcInfo.AutoramaCtrlEnum = MTKAUTORAMA_CTRL_STITCH;
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_SET_PROC_INFO, &AutoramaProcInfo, 0);
    if(Retcode!=S_AUTORAMA_OK)
    {
        MY_LOGD("[autoramaDoStitch] MTKAUTORAMA_FEATURE_SET_PROC_INFO Fail \n");
        return Retcode;
    }
     /* Stitching the images stored in Pano Driver */
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaMain();
    if(Retcode!=S_AUTORAMA_OK)
    {
        MY_LOGD("[autoramaDoStitch] AutoramaMain Fail\n");
    }
    FUNCTION_OUT;
    return Retcode;
}


/*******************************************************************************
* after merged done, algo return the image info of this merged pictures.
********************************************************************************/
MINT32 AutoramaPlugin::autoramaGetResult(AutoramaResultInfo* ResultInfo)
{
    FUNCTION_IN;
    MINT32 Retcode = S_AUTORAMA_OK;
    MTKAutoramaResultInfo AutoramaResultInfo;
    Retcode = autoramaPluginObj->m_pMTKAutoramaObj->AutoramaFeatureCtrl(MTKAUTORAMA_FEATURE_GET_RESULT, 0, &AutoramaResultInfo);
    if(Retcode!=S_AUTORAMA_OK)
    {
        MY_LOGD("[autoramaGetResult] MTKAUTORAMA_FEATURE_GET_RESULT Fail\n");
    }
    MY_LOGD("[autoramaGetResult] ImgWidth %d ImgHeight %d ImgBufferAddr 0x%p\n",AutoramaResultInfo.ImgWidth,AutoramaResultInfo.ImgHeight,(void*)AutoramaResultInfo.ImgBufferAddr);
    ResultInfo->ImgWidth=AutoramaResultInfo.ImgWidth;
    ResultInfo->ImgHeight=AutoramaResultInfo.ImgHeight;
    ResultInfo->ImgBufferAddr=reinterpret_cast<uintptr_t>(AutoramaResultInfo.ImgBufferAddr);
    FUNCTION_OUT;
    return Retcode;
}

//REGISTER_PLUGIN_PROVIDER_DYNAMIC(Yuv, AutoramaPlugin, MTK_FEATURE_AUTORAMA);
REGISTER_PLUGIN_PROVIDER(Yuv, AutoramaPlugin);
