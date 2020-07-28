/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
 /*
** $Log: fdvt_hal.cpp $
 *
*/
#define LOG_TAG "mHalFDVT"

#define FDVT_DDP_SUPPORT

#include <utils/Errors.h>
#include <utils/Mutex.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <faces.h>

#include "fdvt_hal.h"
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>

#include "camera_custom_fd.h"

#include <mtkcam/def/PriorityDefs.h>
#include <sys/prctl.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FD_HAL);

using namespace android;


//****************************//
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//

#define ENABLE_FD_DEBUG_LOG (0)
#define USE_SW_FD_TO_DEBUG (0)
#if (MTKCAM_FDFT_USE_HW == '1') && (USE_SW_FD_TO_DEBUG == 0)
#define USE_HW_FD (1)

#define USE_HW_FD (0)
#endif

#if (MTKCAM_FDFT_SUB_VERSION == '1')
#define HW_FD_SUBVERSION (1)
#elif (MTKCAM_FDFT_SUB_VERSION == '2')
#define HW_FD_SUBVERSION (2)
#else
#define HW_FD_SUBVERSION (0)
#endif

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1
#define MHAL_UNINIT_ERROR 2
#define MHAL_REINIT_ERROR 3

#undef MAX_FACE_NUM
#define MAX_FACE_NUM 15

// FD algo parameter predefine
#define MHAL_FDVT_FDBUF_W (400)
#define FD_DRIVER_MAX_WIDTH (640)
#define FD_DRIVER_MAX_HEIGHT (640)


#define DLFD_SMOOTH_PARAM (2)
#define DLFD_SMOOTH_PARAM_BASE (2816)

//static MUINT32 __unused image_width_array[FD_SCALES]  = {400, 320, 258, 214, 180, 150, 126, 104, 88, 74, 62, 52, 42, 34};
//static MUINT32 __unused image_height_array[FD_SCALES] = {300, 240, 194, 162, 136, 114, 96, 78, 66, 56, 48, 40, 32, 26};

static Mutex&       gLock = *new Mutex();
static Mutex&       gInitLock = *new Mutex();
static int         gGenderDebug = 0;
//****************************//

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/*******************************************************************************
* static function
********************************************************************************/
// Create RGB565 QVGA for Tracking, use another thread
void* halFDVT::onAttThreadLoop(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    while(1) {
        ::sem_wait(&(_this->mSemAttExec));
        if(_this->mAttStop) {
            break;
        }
        _this->doHWAttribute();
    }
    return NULL;
}

/*******************************************************************************
* callback
********************************************************************************/
void FDVTCallback(EGNParams<FDVTConfig>& rParams)
{
    halFDVT *pFDHal = (halFDVT *)rParams.mpCookie;
    pFDHal->handleFDCallback(rParams);
    return;
}

/*******************************************************************************
* Public API
********************************************************************************/
halFDBase*
halFDVT::
getInstance()
{
    Mutex::Autolock _l(gLock);
    halFDBase* pHalFD;

    pHalFD = new halFDVT();

    return  pHalFD;

}

void
halFDVT::
destroyInstance()
{
    Mutex::Autolock _l(gLock);
    delete this;

}

halFDVT::halFDVT()
{
    mpMTKFDVTObj = NULL;

    mFDW = 0;
    mFDH = 0;
    mInited = 0;
    mDoDumpImage = 0;
    mFDRefresh = 3;
    mpAttrWorkBuf = NULL;
    mAllocator = NULL;
    mAttStop = 0;

    mpMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_HW);
}

halFDVT::~halFDVT()
{
    mFDW = 0;
    mFDH = 0;

    /*{
        mAttStop = 1;
        ::sem_post(&mSemAttExec);
        pthread_join(mAttThread, NULL);
        sem_destroy(&mSemAttExec);
    }*/

    if (mpMTKFDVTObj) {
        mpMTKFDVTObj->destroyInstance();
    }
    MY_LOGD("[Destroy] mpMTKFDVTObj->destroyInstance");

    mpMTKFDVTObj = NULL;
}

MINT32
halFDVT::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH,
    MUINT8 *WorkingBuffer,
    MUINT32 WorkingBufferSize,
    MBOOL   isNormalWorld,
    MUINT8  Current_mode,  //0:FD, 1:SD, 2:vFB 3:CFB 4:VSDOF
    MINT32 FldNum
)
{
    Mutex::Autolock _l(gInitLock);
    MINT32 err = MHAL_NO_ERROR;
    MTKFDFTInitInfo FDFTInitInfo;
    FD_Customize_PARA FDCustomData;

    if(mInited) {
        MY_LOGW("Warning!!! FDVT HAL OBJ is already inited!!!!");
        MY_LOGW("Old Width/Height : %d/%d, Parameter Width/Height : %d/%d", mFDW, mFDH, fdW, fdH);
        return MHAL_REINIT_ERROR;
    }
    {
        char cLogLevel[PROPERTY_VALUE_MAX];
        ::property_get("vendor.debug.camera.fd.dumpimage", cLogLevel, "0");
        mDoDumpImage = atoi(cLogLevel);
    }
    // Start initial FD
    mCurrentMode = Current_mode;

    MY_LOGD("[mHalFDInit] Current_mode:%d, SrcW:%d, SrcH:%d, ",Current_mode, fdW, fdH);


    get_fd_CustomizeData(&FDCustomData);

    // Set FD/FT buffer resolution
    mFDW = fdW;
    mFDH = fdH;
    mFTWidth  = fdW;
    mFTHeight = fdH;

    mFDhw_W = FDFTInitInfo.FDBufWidth = MHAL_FDVT_FDBUF_W;
    mFDhw_H = FDFTInitInfo.FDBufHeight = MHAL_FDVT_FDBUF_W * mFDH / mFDW;
    FDFTInitInfo.FDTBufWidth = mFTWidth;
    FDFTInitInfo.FDTBufHeight =  mFTHeight;
    FDFTInitInfo.FDSrcWidth = mFDW;
    FDFTInitInfo.FDSrcHeight = mFDH;

    // Set FD/FT initial parameters
    mFDFilterRatio = FDCustomData.FDSizeRatio;
    FDFTInitInfo.WorkingBufAddr = WorkingBuffer;
    FDFTInitInfo.WorkingBufSize = WorkingBufferSize;
    FDFTInitInfo.FDThreadNum = FDCustomData.FDThreadNum;
    #if (USE_SW_FD_TO_DEBUG)
    FDFTInitInfo.FDThreshold = 256;
    #else
    FDFTInitInfo.FDThreshold = 272;//FDCustomData.FDThreshold;
    #endif
    FDFTInitInfo.MajorFaceDecision = FDCustomData.MajorFaceDecision;
    FDFTInitInfo.OTRatio = FDCustomData.OTRatio;
    FDFTInitInfo.SmoothLevelUI = 8;//FDCustomData.SmoothLevel;
    FDFTInitInfo.MomentumUI = DLFD_SMOOTH_PARAM_BASE + DLFD_SMOOTH_PARAM;// ALGO tuning
    FDFTInitInfo.Momentum = 2819;
    FDFTInitInfo.SmoothLevel = 8;
    FDFTInitInfo.MaxTrackCount = FDCustomData.MaxTrackCount;
    if(mCurrentMode == HAL_FD_MODE_VFB)
        FDFTInitInfo.FDSkipStep = 1;   //FB mode
    else
        FDFTInitInfo.FDSkipStep = FDCustomData.FDSkipStep;

    FDFTInitInfo.FDRectify = FDCustomData.FDRectify;

    FDFTInitInfo.OTFlow = FDCustomData.OTFlow;
    if(mCurrentMode == HAL_FD_MODE_VFB) {
        FDFTInitInfo.OTFlow = 1;
        if(FDFTInitInfo.OTFlow==0)
            FDFTInitInfo.FDRefresh = 90;   //FB mode
        else
            FDFTInitInfo.FDRefresh = FDCustomData.FDRefresh;   //FB mode
    }
    else{
        FDFTInitInfo.FDRefresh = FDCustomData.FDRefresh;
    }
    mFDRefresh = FDFTInitInfo.FDRefresh;
    FDFTInitInfo.FDImageArrayNum = 0;
    FDFTInitInfo.FDImageWidthArray = NULL;
    FDFTInitInfo.FDImageHeightArray = NULL;
    FDFTInitInfo.FDCurrent_mode = mCurrentMode;
    FDFTInitInfo.FDModel = FDCustomData.FDModel;
    FDFTInitInfo.FDMinFaceLevel = 0;
    FDFTInitInfo.FDMaxFaceLevel = 13;
    FDFTInitInfo.FDImgFmtCH1 = FACEDETECT_IMG_Y_SINGLE;
    FDFTInitInfo.FDImgFmtCH2 = FACEDETECT_IMG_RGB565;
    FDFTInitInfo.SDImgFmtCH1 = FACEDETECT_IMG_Y_SCALES;
    FDFTInitInfo.SDImgFmtCH2 = FACEDETECT_IMG_Y_SINGLE;
    FDFTInitInfo.SDThreshold = FDCustomData.SDThreshold;
    FDFTInitInfo.SDMainFaceMust = FDCustomData.SDMainFaceMust;
    FDFTInitInfo.GSensor = FDCustomData.GSensor;
    FDFTInitInfo.GenScaleImageBySw = 1;
    FDFTInitInfo.ParallelRGB565Conversion = false;
    FDFTInitInfo.LockOtBufferFunc = nullptr;
    FDFTInitInfo.UnlockOtBufferFunc = nullptr;
    FDFTInitInfo.lockAgent = nullptr;
    FDFTInitInfo.DisLimit = 0;
    FDFTInitInfo.DecreaseStep = 0;
    FDFTInitInfo.OTBndOverlap = 8;
    FDFTInitInfo.OTds = 2;
    FDFTInitInfo.OTtype = 1;
    {
        FDFTInitInfo.DelayThreshold = 289;
        FDFTInitInfo.DelayCount = 2; // 2 is default value
        FDFTInitInfo.DisLimit = 1; // 2 is default value
        FDFTInitInfo.DecreaseStep = 48; // 2 is default value
    }
    // secure
    misSecureFD = FDFTInitInfo.isSecureFD = !isNormalWorld;
    mFLDNum = FDFTInitInfo.LandmarkEnableCnt = misSecureFD ? 0 : ::property_get_int32("vendor.debug.camera.fld.count", FldNum);
    FDFTInitInfo.SilentModeFDSkipNum = ::property_get_int32("vendor.debug.camera.fdskipnum", 0);
    FDFTInitInfo.FLDAttribConfig = 0; // enable gender detection
    mGenderNum = FDFTInitInfo.GenderEnableCnt = misSecureFD ? 0 : ::property_get_int32("vendor.debug.camera.gender.count", 5);
    mPoseNum = FDFTInitInfo.PoseEnableCnt = misSecureFD ? 0 : ::property_get_int32("vendor.debug.camera.pose.count", 5);
    //FDFTInitInfo.gender_status_mutexAddr = &mattribureMutex;
    FDFTInitInfo.FDVersion = MTKCAM_HWFD_MAIN_VERSION + HW_FD_SUBVERSION;
    #if (USE_HW_FD)&&(HW_FD_SUBVERSION == 2)
    FDFTInitInfo.FDMINSZ = 0;
    #endif
    mSkipPartialFD = ::property_get_int32("vendor.debug.camera.fd.skipdlfd", 0);
    mSkipAllFD     = ::property_get_int32("vendor.debug.camera.fd.skipallfd", 0);
    gGenderDebug   = ::property_get_int32("vendor.debug.camera.fd.genderdebug", 0);
    if (mSkipPartialFD == 0 || mSkipAllFD == 0)
    {
        mSkipPartialFD = 1;
        mSkipAllFD = 1;
    }
    // allocate attribute buffer
    if (FDFTInitInfo.GenderEnableCnt || FDFTInitInfo.PoseEnableCnt)
    {
        mworkbufList = (unsigned char**)malloc(sizeof(unsigned char*) * MAX_CROP_NUM);
        mAllocator = IImageBufferAllocator::getInstance();
        IImageBufferAllocator::ImgParam imgParam(MAX_CROP_NUM * sizeof(unsigned char) * MAX_CROP_W * MAX_CROP_W * 2, 16);
        mpAttrWorkBuf = mAllocator->alloc("FDAttributeBuf", imgParam);
        if ( mpAttrWorkBuf.get() == 0 )
        {
            MY_LOGE("NULL Buffer\n");
            return MFALSE;
        }
        if ( !mpAttrWorkBuf->lockBuf( "FDAttributeBuf", (eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_SW_MASK)) )
        {
            MY_LOGE("lock Buffer failed\n");
            return MFALSE;
        }
        MY_LOGD("allocator buffer : %" PRIXPTR "", mpAttrWorkBuf->getBufVA(0));
        for(int i = 0; i < MAX_CROP_NUM; i++)
        {
            mworkbufList[i] = (unsigned char*)(mpAttrWorkBuf->getBufVA(0) + (i * sizeof(unsigned char) * MAX_CROP_W * MAX_CROP_W * 2));
            mworkbufListPA[i] = (mpAttrWorkBuf->getBufPA(0) + (i * sizeof(unsigned char) * MAX_CROP_W * MAX_CROP_W * 2));
            MY_LOGD("mworkbufList[%d] : %p", i, mworkbufList[i]);
        }
    }
    mAttrBusy = false;
    mAttStop = 0;
    sem_init(&mSemAttExec, 0, 0);
    pthread_create(&mAttThread, NULL, onAttThreadLoop, this);
    pthread_setname_np(mAttThread, "FDAttThread");
    // init driver
    mpFDStream = NSCam::NSIoPipe::NSEgn::IEgnStream<FDVTConfig>::createInstance(LOG_TAG);
    //mpFDStream->initFD(FD_DRIVER_MAX_WIDTH, FD_DRIVER_MAX_HEIGHT);
    //mpFDStream->init();
    INITParams<FDVTConfig> InitParams;
    InitParams.srcImg_maxWidth = fdW;
    InitParams.srcImg_maxHeight = fdW * 1.5;
    InitParams.feature_threshold = (-64);
    InitParams.isFdvtSecure = misSecureFD;
    mpFDStream->init(InitParams);
    // dump initial info
    dumpFDParam(FDFTInitInfo);
    // Set initial info to FD algo
    mpMTKFDVTObj->FDVTInit(&FDFTInitInfo);

    MY_LOGD("[%s] End", __FUNCTION__);
    mFrameCount = 0;
    mDetectedFaceNum = 0;

    mInited = 1;
    return err;
}

MINT32
halFDVT::halFDGetVersion(
)
{
    return HAL_FD_VER_HW50;
}

MINT32
halFDVT::halFDDo(
struct FD_Frame_Parameters &Param
)
{
    Mutex::Autolock _l(gInitLock);
    FdOptions FDOps;
    MBOOL SDEnable;
    MINT32 StartPos = 0;
    fd_cal_struct *pfd_cal_data;


    SDEnable = Param.SDEnable && (mCurrentMode == HAL_FD_MODE_SD);

    if(!mInited) {
        return MHAL_UNINIT_ERROR;
    }

    if (((mFrameCount % mSkipAllFD) != 0))
    {
        MY_LOGD("skip this frame : mFrameCount:%d, mSkipAllFD:%d", mFrameCount, mSkipAllFD);
        mFrameCount++;
        return MHAL_NO_ERROR;
    }

    FACEDETECT_GSENSOR_DIRECTION direction;
    if( Param.Rotation_Info == 0 )
        direction = FACEDETECT_GSENSOR_DIRECTION_0;
    else if( Param.Rotation_Info == 90 )
        direction = FACEDETECT_GSENSOR_DIRECTION_270;
    else if( Param.Rotation_Info == 270 )
        direction = FACEDETECT_GSENSOR_DIRECTION_90;
    else if( Param.Rotation_Info == 180 )
        direction = FACEDETECT_GSENSOR_DIRECTION_180;
    else
        direction = FACEDETECT_GSENSOR_DIRECTION_NO_SENSOR;

    // Set FD operation
    FDOps.fd_state = FDVT_GFD_MODE;
    FDOps.direction = direction;
    FDOps.fd_scale_start_position = StartPos;
    FDOps.gfd_fast_mode = 0;
    FDOps.ae_stable = Param.AEStable;
    FDOps.ForceFDMode = FDVT_GFD_MODE;
    if (Param.pImageBufferPhyP2 != NULL) {
        FDOps.inputPlaneCount = 3;
    } else if (Param.pImageBufferPhyP1 != NULL) {
        FDOps.inputPlaneCount = 2;
    } else {
        FDOps.inputPlaneCount = 1;
    }
    FDOps.ImageBufferPhyPlane1 = Param.pImageBufferPhyP0;
    FDOps.ImageBufferPhyPlane2 = Param.pImageBufferPhyP1;
    FDOps.ImageBufferPhyPlane3 = Param.pImageBufferPhyP2;
    FDOps.ImageBufferRGB565 = Param.pRGB565Image;
    FDOps.ImageBufferSrcVirtual = (MUINT8*)Param.pImageBufferVirtual;
    FDOps.startW = 0;
    FDOps.startH = 0;
    FDOps.model_version = 0;
    FDOps.curr_gtype = Param.gammaType;
    FDOps.LV = Param.LvValue;
    {
        FDVT_OPERATION_MODE_ENUM mode;
        mpMTKFDVTObj->FDVTGetMode(&mode);

        mpMTKFDVTObj->FDVTMain(&FDOps);
        if (FDOps.doPhase2)
        {
            pfd_cal_data = mpMTKFDVTObj->FDGetCalData();//Get From Algo

            for(int i = 0; i < MAX_FACE_SEL_NUM; i++) {
                pfd_cal_data->display_flag[i] = (kal_bool)0;
            }
            if ((mFrameCount % mSkipPartialFD) == 0)
            {
                MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "FDVTMain out1");
                doHWFaceDetection(pfd_cal_data);
            }
            else
            {
            }

            MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "FDVTMain out2");

            mpMTKFDVTObj->FDVTMainFastPhase(mGammaCtrl);

            if ( (FDOps.doGender || FDOps.doPose) && !misSecureFD ) // run attribute
            {
                if (gGenderDebug || mCurrentMode == HAL_FD_MODE_MANUAL)
                {
                    mpMTKFDVTObj->FDVTMainCropPhase(mtodoList, mbufStatus, mworkbufList, mpatchSize, mAIEAttrTask);
                    mAttDir = mFDDir;
                    doHWAttribute();
                    mpMTKFDVTObj->FDVTMainJoinPhase(mbufStatus[AIE_ATTR_TYPE_GENDER], mgender_FM, AIE_ATTR_TYPE_GENDER);
                    mpMTKFDVTObj->FDVTMainJoinPhase(mbufStatus[AIE_ATTR_TYPE_POSE], mpose_FM, AIE_ATTR_TYPE_POSE);
                }
                else
                {
                    Mutex::Autolock _l(mattribureMutex);
                    if ( !mAttrBusy )
                    {
                        mAttrBusy = true;
                        mpMTKFDVTObj->FDVTMainJoinPhase(mbufStatus[AIE_ATTR_TYPE_GENDER], mgender_FM, AIE_ATTR_TYPE_GENDER);
                        mpMTKFDVTObj->FDVTMainJoinPhase(mbufStatus[AIE_ATTR_TYPE_POSE], mpose_FM, AIE_ATTR_TYPE_POSE);
                        mpMTKFDVTObj->FDVTMainCropPhase(mtodoList, mbufStatus, mworkbufList, mpatchSize, mAIEAttrTask);
                        mAttDir = mFDDir;
                        ::sem_post(&mSemAttExec);
                    }
                }
            }
            mpMTKFDVTObj->FDVTMainPostPhase();
            mpMTKFDVTObj->FDVTMainJoinPhase(mbufStatus[AIE_ATTR_TYPE_POSE], mpose_FM, -1);
        }
        MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "FDVTMain out3");

    }

    mFrameCount++;

    return MHAL_NO_ERROR;
}

// internal function
bool
halFDVT::handleFDCallback(
    EGNParams<FDVTConfig>& rParams
)
{
    Mutex::Autolock _l(mDriverLock);
    if (rParams.mEGNConfigVec.size() > 0)
    {
        if (rParams.mEGNConfigVec[0].FD_MODE == NSCam::NSIoPipe::FDMODE)
        {
            mFdvtDrvResult = rParams.mEGNConfigVec[0];
            mFDCond.signal();
        }
        else
        {
            mAttDrvResult = rParams.mEGNConfigVec[0];
            mAttrCond.signal();
        }
    }

    return true;
}

bool
halFDVT::doHWAttribute(
)
{
    EGNParams<FDVTConfig> FdvtParams;
    FDVTConfig FdvtConfig;
    int err = OK;

    mpAttrWorkBuf->syncCache();
    for (int attr = 0; attr < MAX_AIE_ATTR_TYPE; attr++)
    {
        for (int task = 0; task < mAIEAttrTask[attr]; task++)
        {
            int psz = mpatchSize[mtodoList[attr][task]];
            if (psz == 0)
            {
                break;
            }
            {
                Mutex::Autolock __l(mSyncLock);
                Mutex::Autolock _l(mDriverLock);
                FdvtParams.mpEngineID = NSCam::NSIoPipe::eFDVT;
                FdvtParams.mpfnCallback = FDVTCallback;
                FdvtParams.mpCookie = this;

                FdvtConfig.FD_MODE = (attr == AIE_ATTR_TYPE_GENDER) ? NSCam::NSIoPipe::ATTRIBUTEMODE : NSCam::NSIoPipe::POSEMODE;
                FdvtConfig.SRC_IMG_WIDTH = psz;
                FdvtConfig.SRC_IMG_HEIGHT = psz;
                FdvtConfig.SRC_IMG_FMT = NSCam::NSIoPipe::FMT_YUYV;
                FdvtConfig.source_img_address = (MUINT64*)mworkbufListPA[mtodoList[attr][task]];
                FdvtConfig.source_img_address_UV = nullptr;
                FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_0;

                FdvtParams.mEGNConfigVec.push_back(FdvtConfig);
                mpFDStream->EGNenque(FdvtParams);

                err = mAttrCond.waitRelative(mDriverLock, 2000000000); // wait 2 sec if time out
                FdvtParams.mEGNConfigVec.clear();

                if  ( OK != err ) {
                    // error handling
                    MY_LOGE("FD HW driver run attribute timeout...please check...");
                    return false;
                }
            }
            {
                Mutex::Autolock _l(mattribureMutex);
                if (attr == AIE_ATTR_TYPE_GENDER)
                {
                    memcpy(mgender_FM[mtodoList[attr][task]], &(mAttDrvResult.ATTRIBUTEOUTPUT), sizeof(struct NSCam::NSIoPipe::ATTRIBUTE_RESULT));
                }
                else
                {
                    memcpy(mpose_FM[mtodoList[attr][task]], &(mAttDrvResult.POSEOUTPUT), sizeof(struct NSCam::NSIoPipe::POSE_RESULT));
                }
                mbufStatus[attr][mtodoList[attr][task]] = 2;
            }
        }
    }
    mAttrBusy = false;

    return true;
}

bool
halFDVT::doHWFaceDetection(
    void *pCal_Data
)
{
    Mutex::Autolock __l(mSyncLock);
    Mutex::Autolock _l(mDriverLock);
    fd_cal_struct *pfd_cal_data = (fd_cal_struct *)pCal_Data;
    EGNParams<FDVTConfig> FdvtParams;
    FDVTConfig FdvtConfig;
    int err = OK;

    FdvtParams.mpEngineID = NSCam::NSIoPipe::eFDVT;
    FdvtParams.mpfnCallback = FDVTCallback;
    FdvtParams.mpCookie = this;

    FdvtConfig.FD_MODE = NSCam::NSIoPipe::FDMODE;
    FdvtConfig.SRC_IMG_WIDTH = mFDW;
    FdvtConfig.SRC_IMG_HEIGHT = mFDH;
    FdvtConfig.SRC_IMG_FMT = NSCam::NSIoPipe::FMT_YUYV;
    FdvtConfig.source_img_address = (MUINT64*)pfd_cal_data->srcbuffer_phyical_addr_plane1;
    FdvtConfig.source_img_address_UV = nullptr;
    switch(pfd_cal_data->current_feature_index)
    {
        case 1:
        case 2:
        case 3:
            mFDDir = FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_0;
            break;
        case 4:
        case 6:
        case 8:
            mFDDir = FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_90;
            break;
        case 5:
        case 7:
        case 9:
            mFDDir = FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_270;
            break;
        case 10:
        case 11:
        case 12:
            mFDDir = FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_180;
            break;
        default:
            MY_LOGW("error fd direction : %d", pfd_cal_data->current_feature_index);
            mFDDir = FdvtConfig.INPUT_ROTATE_DEGREE = NSCam::NSIoPipe::DEGREE_0;
            break;
    }
    FdvtConfig.FDOUTPUT.PYRAMID0_RESULT.fd_partial_result = 0;
    FdvtConfig.FDOUTPUT.PYRAMID1_RESULT.fd_partial_result = 0;
    FdvtConfig.FDOUTPUT.PYRAMID2_RESULT.fd_partial_result = 0;

    FdvtParams.mEGNConfigVec.push_back(FdvtConfig);

    mpFDStream->EGNenque(FdvtParams);

    err = mFDCond.waitRelative(mDriverLock, 2000000000); // wait 2 sec if time out
    if  ( OK != err ) {
        // error handling
        MY_LOGE("FD HW driver timeout...please check...");
        return false;
    }
    float widthRatio = (float)mFDhw_W / (float)mFDW;
    float heightRatio = (float)mFDhw_H / (float)mFDH;

    auto handleFDResult = [&] (NSCam::NSIoPipe::FDRESULT &DrvResult, int Idx) -> int
    {
        MUINT32 i = 0, j = 0;
        MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "debug : Idx=%d, result=%d", Idx, DrvResult.fd_partial_result);
        for(i = Idx; i < (DrvResult.fd_partial_result + Idx); i++, j++)
        {
            #define CLIP(_x, min, max) \
            _x < min ? min : _x > max ? max : _x

            pfd_cal_data->face_candi_pos_x0[i] = CLIP((int)(DrvResult.anchor_x0[j] * widthRatio), 0, mFDhw_W - 1);
            pfd_cal_data->face_candi_pos_y0[i] = CLIP((int)(DrvResult.anchor_y0[j] * heightRatio), 0, mFDhw_H - 1);
            pfd_cal_data->face_candi_pos_x1[i] = CLIP((int)(DrvResult.anchor_x1[j] * widthRatio), 0, mFDhw_W - 1);
            pfd_cal_data->face_candi_pos_y1[i] = CLIP((int)(DrvResult.anchor_y1[j] * heightRatio), 0, mFDhw_H - 1);

            //
            pfd_cal_data->face_reliabiliy_value[i] = DrvResult.anchor_score[j] + 3600;
            pfd_cal_data->display_flag[i] = (kal_bool)1;
            pfd_cal_data->face_feature_set_index[i] = pfd_cal_data->current_feature_index;
            pfd_cal_data->rip_dir[i] = pfd_cal_data->current_feature_index;
            pfd_cal_data->rop_dir[i] = 0;
            pfd_cal_data->result_type[i] = GFD_RST_TYPE;

        }
        return i;
    };
    int FaceNum = 0;
    FaceNum = handleFDResult(mFdvtDrvResult.FDOUTPUT.PYRAMID0_RESULT, FaceNum);
    FaceNum = handleFDResult(mFdvtDrvResult.FDOUTPUT.PYRAMID1_RESULT, FaceNum);
    FaceNum = handleFDResult(mFdvtDrvResult.FDOUTPUT.PYRAMID2_RESULT, FaceNum);

    if (CC_UNLIKELY(FaceNum != mFdvtDrvResult.FDOUTPUT.FD_TOTAL_NUM))
    {
        MY_LOGW("please check face pyramid total num(%d) != FD_TOTAL_NUM(%d)", FaceNum, mFdvtDrvResult.FDOUTPUT.FD_TOTAL_NUM);
        MY_LOGW("Pyramid0 : %d, Pyramid1 : %d, Pyramid2 : %d", mFdvtDrvResult.FDOUTPUT.PYRAMID0_RESULT.fd_partial_result
                                                             , mFdvtDrvResult.FDOUTPUT.PYRAMID1_RESULT.fd_partial_result
                                                             , mFdvtDrvResult.FDOUTPUT.PYRAMID2_RESULT.fd_partial_result);
    }


    return true;
}
//

MINT32
halFDVT::halFDUninit(
)
{
    //MHAL_LOG("[halFDUninit] IN \n");
    Mutex::Autolock _l(gInitLock);

    if(!mInited) {
        MY_LOGW("FD HAL Object is already uninited...");
        return MHAL_NO_ERROR;
    }

    mAttStop = 1;
    ::sem_post(&mSemAttExec);
    pthread_join(mAttThread, NULL);
    sem_destroy(&mSemAttExec);

    mpMTKFDVTObj->FDVTReset();

    mpFDStream->uninit();
    mpFDStream->destroyInstance(LOG_TAG);

    if(mpAttrWorkBuf != NULL && mAllocator != NULL) {
        mpAttrWorkBuf->unlockBuf("FDAttributeBuf");
        mAllocator->free(mpAttrWorkBuf.get());
        mpAttrWorkBuf = NULL;
    }

    if (mworkbufList) {
        free(mworkbufList);
        mworkbufList = nullptr;
    }

    mInited = 0;

    return MHAL_NO_ERROR;
}

MINT32
halFDVT::halGetGammaSetting(MINT32* &gammaCtrl)
{
    gammaCtrl = mGammaCtrl;
    return 0;
}

MINT32
halFDVT::halFDGetFaceInfo(
    MtkCameraFaceMetadata *fd_info_result
)
{
    MUINT8 i;
    MY_LOGD("[GetFaceInfo] NUM_Face:%d,", mFdResult_Num);

    if( (mFdResult_Num < 0) || (mFdResult_Num > 15) )
         mFdResult_Num = 0;

    fd_info_result->number_of_faces =  mFdResult_Num;

    for(i=0;i<mFdResult_Num;i++)
    {
        fd_info_result->faces[i].rect[0] = mFdResult[i].rect[0];
        fd_info_result->faces[i].rect[1] = mFdResult[i].rect[1];
        fd_info_result->faces[i].rect[2] = mFdResult[i].rect[2];
        fd_info_result->faces[i].rect[3] = mFdResult[i].rect[3];
        fd_info_result->faces[i].score = mFdResult[i].score;
        fd_info_result->posInfo[i].rop_dir = mFdResult[i].rop_dir;
        fd_info_result->posInfo[i].rip_dir  = mFdResult[i].rip_dir;
    }

    return MHAL_NO_ERROR;
}

MINT32
halFDVT::halFDGetFaceResult(
    MtkCameraFaceMetadata *fd_result,
    MINT32 ResultMode
)
{
    result pbuf[MAX_FACE_NUM];
    MUINT8 i;
    MINT8 DrawMode = 0;
    MINT32 PrevFaceNum = mDetectedFaceNum;

    if (ResultMode == 1)
    {
        // for UI
        mpMTKFDVTObj->FDVTGetICSResult((MUINT8 *) fd_result, (MUINT8 *) pbuf, mFDhw_W, mFDhw_H, 0, 0, 0, 5);
    }
    else
    {
        // for 3A
        mpMTKFDVTObj->FDVTGetICSResult((MUINT8 *) fd_result, (MUINT8 *) pbuf, mFDhw_W, mFDhw_H, 0, 0, 0, 1);
    }

    MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "index[0] result : (%d, %d), (%d, %d)", pbuf[0].x0, pbuf[0].y0, pbuf[0].x1, pbuf[0].y1);
    MY_LOGD_IF(ENABLE_FD_DEBUG_LOG, "index[0] ICSresult : (%d, %d), (%d, %d)", fd_result->faces[0].rect[0], fd_result->faces[0].rect[1], fd_result->faces[0].rect[2], fd_result->faces[0].rect[3]);

    mDetectedFaceNum = mFdResult_Num = fd_result->number_of_faces;

    fd_result->CNNFaces.PortEnable = 0;
    fd_result->CNNFaces.IsTrueFace = 0;
    //
    fd_result->landmarkNum = mFLDNum;
    fd_result->genderNum = mGenderNum;
    fd_result->poseNum = mPoseNum;

    fd_result->gmIndex = mGammaCtrl[0];
    fd_result->gmData = &mGammaCtrl[1];
    fd_result->gmSize = 192;

    //Facial Size Filter
    for(i=0;i < mFdResult_Num;i++)
    {
        int diff = fd_result->faces[i].rect[3] - fd_result->faces[i].rect[1];
        float ratio = (float)diff / 2000.0;
        if(ratio < mFDFilterRatio) {
            int j;
            for(j = i; j < (mFdResult_Num - 1); j++) {
                fd_result->faces[j].rect[0] = fd_result->faces[j+1].rect[0];
                fd_result->faces[j].rect[1] = fd_result->faces[j+1].rect[1];
                fd_result->faces[j].rect[2] = fd_result->faces[j+1].rect[2];
                fd_result->faces[j].rect[3] = fd_result->faces[j+1].rect[3];
                fd_result->faces[j].score = fd_result->faces[j+1].score;
                fd_result->faces[j].id = fd_result->faces[j+1].id;
                fd_result->posInfo[j].rop_dir = fd_result->posInfo[j+1].rop_dir;
                fd_result->posInfo[j].rip_dir = fd_result->posInfo[j+1].rip_dir;
                fd_result->faces_type[j] = fd_result->faces_type[j+1];
            }
            fd_result->faces[j].rect[0] = 0;
            fd_result->faces[j].rect[1] = 0;
            fd_result->faces[j].rect[2] = 0;
            fd_result->faces[j].rect[3] = 0;
            fd_result->faces[j].score = 0;
            fd_result->posInfo[j].rop_dir = 0;
            fd_result->posInfo[j].rip_dir = 0;
            fd_result->number_of_faces--;
            mFdResult_Num--;
            i--;
        }
    }

    for(i=0;i<mFdResult_Num;i++)
    {
          mFdResult[i].rect[0] = fd_result->faces[i].rect[0];
          mFdResult[i].rect[1] = fd_result->faces[i].rect[1];
          mFdResult[i].rect[2] = fd_result->faces[i].rect[2];
          mFdResult[i].rect[3] = fd_result->faces[i].rect[3];
          mFdResult[i].score = fd_result->faces[i].score;
          mFdResult[i].rop_dir = fd_result->posInfo[i].rop_dir;
          mFdResult[i].rip_dir  = fd_result->posInfo[i].rip_dir;
    }
    for(i=mFdResult_Num;i<MAX_FACE_NUM;i++)
    {
          mFdResult[i].rect[0] = 0;
          mFdResult[i].rect[1] = 0;
          mFdResult[i].rect[2] = 0;
          mFdResult[i].rect[3] = 0;
          mFdResult[i].score = 0;
          mFdResult[i].rop_dir = 0;
          mFdResult[i].rip_dir  = 0;
    }

    return mFdResult_Num;
}

MINT32
halFDVT::halFDYUYV2ExtractY(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT32 src_width,
MUINT32 src_height
)
{
    MY_LOGD("DO Extract Y In");
    int i;

    int length = src_width*src_height*2;

    for(i=length;i != 0;i-=2)
    {
      *dstAddr++ = *srcAddr;
      srcAddr+=2;
    }

    MY_LOGD("DO Extract Y Out");

    return MHAL_NO_ERROR;
}

void halFDVT::dumpFDImages(MUINT8* pFDImgBuffer, MUINT8* pFTImgBuffer)
{
    char szFileName[100]={'\0'};
    char szFileName1[100]={'\0'};
    FILE * pRawFp;
    FILE * pRawFp1;
    int i4WriteCnt;
    static int count = 0;

    if (mDoDumpImage > 1) {
        sprintf(szFileName1, "/sdcard/FDTest/src_%04dx%04d_%04d_YUV.raw", mFDW, mFDH, count);
        sprintf(szFileName, "/sdcard/FDTest/dst_%04dx%04d_%04d_RGB.raw", mFTWidth, mFTHeight, count);
        count++;
    } else {
        sprintf(szFileName1, "/sdcard/src_%04d_%04d_YUV.raw", mFDW, mFDH);
        sprintf(szFileName, "/sdcard/dst_%04d_%04d_RGB.raw", mFTWidth, mFTHeight);
    }
    pRawFp1 = fopen(szFileName1, "wb");
    if (NULL == pRawFp1 )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)pFDImgBuffer,1, (mFDW * mFDH * 2),pRawFp1);
    fflush(pRawFp1);
    fclose(pRawFp1);

    pRawFp = fopen(szFileName, "wb");
    if (NULL == pRawFp )
    {
        MY_LOGD("Can't open file to save RAW Image\n");
        while(1);
    }
    i4WriteCnt = fwrite((void *)pFTImgBuffer,1, (mFTWidth * mFTHeight * 2),pRawFp);
    fflush(pRawFp);
    fclose(pRawFp);
}

// Dump FD information
void halFDVT::dumpFDParam(MTKFDFTInitInfo &FDFTInitInfo)
{
    MY_LOGD("FDThreshold = %d", FDFTInitInfo.FDThreshold);
    MY_LOGD("MajorFaceDecision = %d", FDFTInitInfo.MajorFaceDecision);
    MY_LOGD("FDTBufW/H = %d/%d", FDFTInitInfo.FDTBufWidth, FDFTInitInfo.FDTBufHeight);

    MY_LOGD("GSensor = %d", FDFTInitInfo.GSensor);
    MY_LOGD("FDManualMode = %d", FDFTInitInfo.FDManualMode);
    #if (USE_HW_FD)&&(HW_FD_SUBVERSION == 2)
    MY_LOGD("FDMINSZ = %d", FDFTInitInfo.FDMINSZ);
    #endif
    #if MTK_ALGO_PSD_MODE
    MY_LOGD("FDVersion = %d, Version = %d", FDFTInitInfo.FDVersion, halFDGetVersion());
    #endif
    MY_LOGD("DL skip : %d, ALL Skip : %d", mSkipPartialFD, mSkipAllFD);
}
