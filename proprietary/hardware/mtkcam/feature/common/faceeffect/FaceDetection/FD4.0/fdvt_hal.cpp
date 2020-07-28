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
//#include "MediaHal.h"
//#include <mhal/inc/camera/faces.h>
#include <faces.h>
//#include "MediaLog.h"
//#include "MediaAssert.h"
#include "fdvt_hal.h"
#include <mtkcam/utils/std/Log.h>

#include "camera_custom_fd.h"

#ifdef FDVT_DDP_SUPPORT
#include <DpBlitStream.h>
#endif

#include <mtkcam/def/PriorityDefs.h>
#include <sys/prctl.h>


using namespace android;

#define DUMP_IMAGE (0)


//****************************//
//-------------------------------------------//
//  Global face detection related parameter  //
//-------------------------------------------//

#define USE_SW_FD_TO_DEBUG (0)
#if (MTKCAM_FDFT_USE_HW == '1') && (USE_SW_FD_TO_DEBUG == 0)
#define USE_HW_FD (1)
#else
#define USE_HW_FD (0)
#endif

#if (MTKCAM_FDFT_SUB_VERSION == '1')
#define HW_FD_SUBVERSION (1)
#elif (MTKCAM_FDFT_SUB_VERSION == '2')
#define HW_FD_SUBVERSION (2)
#elif (MTKCAM_FDFT_SUB_VERSION == '3')
#define HW_FD_SUBVERSION (3)
#else
#define HW_FD_SUBVERSION (0)
#endif

//#define MTK_ALGO_PSD_MODE (1)

#if MTK_ALGO_PSD_MODE && USE_HW_FD
#include <cam_fdvt.h>
#endif

#if (MTKCAM_HWFD_MAIN_VERSION == 35)
#define SINGLE_FACE_STABLE_ENABLE (1)
#endif
#if (MTKCAM_HWFD_MAIN_VERSION == 40)
#define SINGLE_FACE_STABLE_ENABLE (1)
#endif

#define MHAL_NO_ERROR 0
#define MHAL_INPUT_SIZE_ERROR 1
#define MHAL_UNINIT_ERROR 2
#define MHAL_REINIT_ERROR 3

#undef MAX_FACE_NUM
#define MAX_FACE_NUM 15

#define MHAL_FDVT_FTBUF_W (320)
#define MHAL_FDVT_FTBUF_H (240)

// v1 is for SD/FB default mode, v2 is for 320x240 manuel mode, v3 is for 400x300 manuel mode
static MUINT32 image_width_array_v1[FD_SCALES]  = {320, 256, 204, 160, 128, 102, 80, 64, 50, 40, 34, 0, 0, 0};
static MUINT32 image_height_array_v1[FD_SCALES] = {240, 192, 152, 120, 96, 76, 60, 48, 38, 30, 25, 0, 0, 0};
static MUINT32 image_width_array_v2[FD_SCALES]  = {320, 262, 210, 168, 134, 108, 86, 70, 56, 46, 38, 0, 0, 0};
static MUINT32 image_height_array_v2[FD_SCALES] = {240, 196, 157, 125, 100,  80, 64, 52, 41, 33, 27, 0, 0, 0};
static MUINT32 image_width_array_v3[FD_SCALES]  = {400, 328, 262, 210, 168, 134, 108, 86, 70, 56, 46, 38, 0, 0};
static MUINT32 image_height_array_v3[FD_SCALES] = {300, 245, 196, 157, 125, 100,  80, 64, 52, 41, 33, 27, 0, 0};
static MUINT32 image_width_array_v4[FD_SCALES]  = {400, 320, 258, 214, 180, 150, 126, 104, 88, 74, 62, 52, 42, 34};
static MUINT32 image_height_array_v4[FD_SCALES] = {300, 240, 194, 162, 136, 114, 96, 78, 66, 56, 48, 40, 32, 26};

static const MUINT32 gimage_input_width_vga = 640;
static const MUINT32 gimage_input_height_vga = 480;
static const MUINT32 gimage_input_height_buffer = 640;
static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img, MBOOL input_4_3, MBOOL input_16_9);
static void CreateScaleImages_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer,MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles);

static Mutex&       gLock = *new Mutex();
static Mutex&       gInitLock = *new Mutex();
//****************************//
#if USE_PORTRAIT
static sp<mtkdl::PortraitNet> gpPortrait;
static MINT32      gFitstPortInit;
static MINT32      gPortInited;
#endif

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
* Utility
********************************************************************************/
typedef struct
{
    MUINT8 *srcAddr;
    MUINT32 srcWidth;
    MUINT32 srcHeight;
    MUINT8 *dstAddr;
    MUINT32 dstWidth;
    MUINT32 dstHeight;
} PIPE_BILINEAR_Y_RESIZER_STRUCT, *P_PIPE_BILINEAR_Y_RESIZER_STRUCT;

#define PIPE_IUL_I_TO_X(i)              ((i) << 16)                       ///< Convert from integer to S15.16 fixed-point
#define PIPE_IUL_X_TO_I(x)              (((x) + (1 << 15)) >> 16)    ///< Convert from S15.16 fixed-point to integer (round)
#define PIPE_IUL_X_TO_I_CHOP(x)     ((x) >> 16)                       ///< Convert from S15.16 fixed-point to integer (chop)
#define PIPE_IUL_X_TO_I_CARRY(x)        (((x) + 0x0000FFFF) >> 16) ///< Convert from S15.16 fixed-point to integer (carry)
#define PIPE_IUL_X_FRACTION(x)      ((x) & 0x0000FFFF)

#define PIPE_LINEAR_INTERPOLATION(val1, val2, weighting2)   \
   PIPE_IUL_X_TO_I((val1) * (PIPE_IUL_I_TO_X(1) - (weighting2)) + (val2) * (weighting2))

static void PipeBilinearResizer(P_PIPE_BILINEAR_Y_RESIZER_STRUCT pUtlRisizerInfo)
{
    if(pUtlRisizerInfo->srcWidth == 0 || pUtlRisizerInfo->srcHeight==0)
        return;
    if(pUtlRisizerInfo->dstWidth == 0 || pUtlRisizerInfo->dstHeight==0)
        return;

    const MUINT32 srcPitch = pUtlRisizerInfo->srcWidth;
    const MUINT32 srcStepX = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcWidth) /pUtlRisizerInfo->dstWidth;
    const MUINT32 srcStepY = PIPE_IUL_I_TO_X(pUtlRisizerInfo->srcHeight) /pUtlRisizerInfo->dstHeight;
    const MUINT32 img_w = pUtlRisizerInfo->dstWidth;

    MUINT8 *const src_buffer = pUtlRisizerInfo->srcAddr;
    MUINT8 *dstAddr= pUtlRisizerInfo->dstAddr;
    MUINT32 srcCoordY = 0;
    MINT32 h = pUtlRisizerInfo->dstHeight;

    while (--h >= 0)
    {
        MINT32 w = img_w;
        MUINT32 srcCoordX = 0;

        MINT32 srcOffset_1;
        MINT32 srcOffset_2;
        MUINT8 *src_ptr_1;
        MUINT8 *src_ptr_2;

        MINT32 y_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordY);
        MINT32 y_chop  = PIPE_IUL_X_TO_I_CHOP(srcCoordY);

        if(y_carry < 0 || y_carry >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;
        if(y_chop < 0 || y_chop >= (MINT32)pUtlRisizerInfo->srcHeight)
            return;


        srcOffset_1 = y_chop * srcPitch;
        srcOffset_2 = y_carry * srcPitch;
        src_ptr_1 = src_buffer + srcOffset_1;
        src_ptr_2 = src_buffer + srcOffset_2;

        while (--w >= 0)
        {
            MUINT8 pixel_1, pixel_2;
            MINT32 y, y1;

            MINT32 x_carry = PIPE_IUL_X_TO_I_CARRY(srcCoordX);
            MINT32 x_chop = PIPE_IUL_X_TO_I_CHOP(srcCoordX);

            MINT32 weighting2;

            weighting2 = PIPE_IUL_X_FRACTION(srcCoordX);

            /// 1st horizontal interpolation.
            pixel_1 = *(src_ptr_1 + x_chop);
            pixel_2 = *(src_ptr_1 + x_carry);
            y = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

            /// 2nd horizontal interpolation.
            pixel_1 = *(src_ptr_2 + x_chop);
            pixel_2 = *(src_ptr_2 + x_carry);
            y1 = PIPE_LINEAR_INTERPOLATION(pixel_1, pixel_2, weighting2);

            /// Vertical interpolation.
            weighting2 = PIPE_IUL_X_FRACTION(srcCoordY);

            y = PIPE_LINEAR_INTERPOLATION(y, y1, weighting2);

            *dstAddr++ = (MUINT8)y;

            srcCoordX += srcStepX;
        }
        srcCoordY += srcStepY;
    }
}

static void Create640WidthImage_Y(MUINT8* src_img, MUINT32 src_w, MUINT32 src_h, MUINT8* dst_img)
{
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    UtlRisizerInfo.srcAddr = src_img;
    UtlRisizerInfo.srcWidth= src_w;
    UtlRisizerInfo.srcHeight= src_h;
    UtlRisizerInfo.dstAddr = dst_img;
    UtlRisizerInfo.dstWidth = gimage_input_width_vga;
    UtlRisizerInfo.dstHeight = gimage_input_width_vga*src_h/src_w;

    PipeBilinearResizer(&UtlRisizerInfo);
}

static void CreateScaleImages_Y(MUINT8* image_buffer_vga, MUINT8* image_buffer,MUINT32 w, MUINT32 h, MUINT32* ImageWidthArray, MUINT32* ImageHeightArray, MUINT32 sacles)
{
    MUINT32 current_scale;
    PIPE_BILINEAR_Y_RESIZER_STRUCT UtlRisizerInfo;
    MUINT8* dst_ptr;
    dst_ptr = image_buffer;

    for ( current_scale = 0 ; current_scale < sacles ; current_scale ++ )
    {
        UtlRisizerInfo.srcAddr = image_buffer_vga;
        UtlRisizerInfo.srcWidth= w;
        UtlRisizerInfo.srcHeight= h;
        UtlRisizerInfo.dstAddr = dst_ptr;
        UtlRisizerInfo.dstWidth = ImageWidthArray[current_scale];
        UtlRisizerInfo.dstHeight = ImageHeightArray[current_scale];
        if (UtlRisizerInfo.dstWidth == 0 || UtlRisizerInfo.dstHeight==0)
        {
            MY_LOGD("PipeBilinearResizer break %d", current_scale);
            break;
        }
        PipeBilinearResizer(&UtlRisizerInfo);
        dst_ptr+= ImageWidthArray[current_scale]*ImageHeightArray[current_scale];
    }
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

    mFTStop = 0;
    mFTBufReady = 1;
    mFDModelVer = 0;
    sem_init(&mSemFTExec, 0, 0);
    sem_init(&mSemFTLock, 0, 1);
    pthread_create(&mFTThread, NULL, onFTThreadLoop, this);

    #if (USE_HW_FD)
    mpMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_HW);
    #else
    mpMTKFDVTObj = MTKDetection::createInstance(DRV_FD_OBJ_FDFT_SW);
    #endif
}

halFDVT::~halFDVT()
{
    mFDW = 0;
    mFDH = 0;
    {
        mFTStop = 1;
        mFTBufReady = 0;
        ::sem_post(&mSemFTExec);
        pthread_join(mFTThread, NULL);
        sem_destroy(&mSemFTExec);
    }
    MY_LOGD("[Destroy] mSemFTExec");
    if (mpMTKFDVTObj) {
        mpMTKFDVTObj->destroyInstance();
    }
    MY_LOGD("[Destroy] mpMTKFDVTObj->destroyInstance");
    sem_destroy(&mSemFTLock);
    MY_LOGD("[Destroy] mSemFTLock");
    mpMTKFDVTObj = NULL;
}

MINT32 YUY2toRGB888(void *pSrcAddr, MINT32 width, MINT32 height, MINT32 target_width, MINT32 target_height, void *pTargetAddr, MINT32 dstStride = 0)
{
#if USE_DL_FD || USE_PORTRAIT
    MINT32 RetCode = S_CNN_OK;
    void *pSource = pSrcAddr;
    MINT32 img_width = width;
    MINT32 img_height = height;

    DP_STATUS_ENUM status;
    DpBlitStream  DpStream;
    void          *pSrcBuffer[3];
    void          *pDstBuffer[3];
    uint32_t      src_size[3];
    uint32_t      dst_size[3];

    // source address
    pSrcBuffer[0] = pSource;
    pSrcBuffer[1] = 0;
    pSrcBuffer[2] = 0;

    // source size
    src_size[0] = img_width*img_height*2;
    src_size[1] = 0;
    src_size[2] = 0;

    if (dstStride == 0)
    {
        MY_LOGD("Stride = 0, set stride = width\n");
        dstStride = target_width;
    }

    // source config
    status = DpStream.setSrcBuffer(pSrcBuffer, src_size, 1);
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> setSrcBuffer fail! %p\n", LOG_TAG, __FUNCTION__, pSource);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }
    status = DpStream.setRotate(0);
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> setRotate fail!\n", LOG_TAG, __FUNCTION__);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }
    status = DpStream.setSrcConfig(img_width, img_height, eYUYV);
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> setSrcConfig fail!\n", LOG_TAG, __FUNCTION__);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }

    // target address
    pDstBuffer[0] = pTargetAddr;
    pDstBuffer[1] = 0;
    pDstBuffer[2] = 0;

    // target size
    dst_size[0] = dstStride*target_height*3;
    dst_size[1] = 0;
    dst_size[2] = 0;

    // target config
    status = DpStream.setDstBuffer(pDstBuffer, dst_size, 1);
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> setDstBuffer fail!\n", LOG_TAG, __FUNCTION__);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }
    //status = DpStream.setDstConfig(target_width, target_height, eBGR888);
    status = DpStream.setDstConfig(target_width, target_height, dstStride*3, 0, eBGR888);
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> setDstConfig fail!\n", LOG_TAG, __FUNCTION__);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }

    // start processing
    status = DpStream.invalidate();
    if (status != DP_STATUS_RETURN_SUCCESS)
    {
        MY_LOGE("[%s] %s -> invalidate fail!\n", LOG_TAG, __FUNCTION__);
        RetCode = E_CNN_MDP_FAIL;
        return RetCode;
    }

    return RetCode;
#else
    return 0;
#endif
}

void* halFDVT::onInitThreadFunc(void* arg)
{
#if USE_PORTRAIT
    MY_LOGD("start portrait init thread");
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    mtkdl::PortraitNetFactory::E_MODEL_T eModelP =
        static_cast<mtkdl::PortraitNetFactory::E_MODEL_T>(::property_get_int32("vendor.debug.FDCNN.model", PORTRAIT_DEFAULT_MODEL));

    gpPortrait = mtkdl::PortraitNetFactory::get().create(eModelP, false);

    gPortInited = 1;
    MY_LOGD("end portrait init thread");
#endif
    return NULL;
}

MINT32
halFDVT::halFDInit(
    MUINT32 fdW,
    MUINT32 fdH,
    MUINT8 *WorkingBuffer,
    MUINT32 WorkingBufferSize,
    MBOOL   SWResizerEnable,
    MUINT8  Current_mode,  //0:FD, 1:SD, 2:vFB 3:CFB 4:VSDOF
    MINT32 FldNum
)
{
    Mutex::Autolock _l(gInitLock);
    MUINT32 i;
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
    #if (0 == SMILE_DETECT_SUPPORT)
    // If Smile detection is not support, change mode to FD mode
    if (mCurrentMode == HAL_FD_MODE_SD) {
        mCurrentMode = HAL_FD_MODE_FD;
    }
    #endif
    MY_LOGD("[mHalFDInit] Current_mode:%d, SrcW:%d, SrcH:%d, ",Current_mode, fdW, fdH);

    if ( Current_mode == HAL_FD_MODE_FD || Current_mode == HAL_FD_MODE_MANUAL ) {
        #if (MTKCAM_HWFD_MAIN_VERSION == 40)
        for(i=0;i<FD_SCALES;i++)
        {
            mimage_width_array[i] = image_width_array_v4[i];
            mimage_height_array[i] = image_height_array_v4[i];
        }
        mUserScaleNum = 14;
        #else
        for(i=0;i<FD_SCALES;i++)
        {
            mimage_width_array[i] = image_width_array_v3[i];
            mimage_height_array[i] = image_height_array_v3[i];
        }
        mUserScaleNum = 12;
        #endif
        if(Current_mode == HAL_FD_MODE_MANUAL) {
            mUseUserScale = 1;
        } else {
            mUseUserScale = 0;
        }
    } else {
        for(i=0;i<FD_SCALES;i++)
        {
            mimage_width_array[i] = image_width_array_v1[i];
            mimage_height_array[i] = image_height_array_v1[i];
        }
        mUserScaleNum = 11;
        mUseUserScale = 0;
    }

    get_fd_CustomizeData(&FDCustomData);

    // Set FD/FT buffer resolution
    #if (SINGLE_FACE_STABLE_ENABLE == 1)
    // force enable adaptive scale table
    mUseUserScale = 1;
    #else
    if (Current_mode != HAL_FD_MODE_MANUAL) {
        mUseUserScale = FDCustomData.UseCustomScale;

    }
    #endif
    mFDW = fdW;
    mFDH = fdH;
    if (halFDGetVersion() < HAL_FD_VER_HW42 && halFDGetVersion() != HAL_FD_VER_HW37 ) {
        mFTWidth  = MHAL_FDVT_FTBUF_W;
        mFTHeight = MHAL_FDVT_FTBUF_W * mFDH/mFDW;
    }
    else {
        mFTWidth  = fdW;
        mFTHeight = fdH;
    }
    for(int j=0;j<FD_SCALES;j++)
    {
        mimage_height_array[j] = mimage_width_array[j]*mFDH/mFDW;
        mUserScaleNum = j;
        if(mimage_height_array[j] <= 25 || mimage_width_array[j] <= 25) {
            break;
        }
        // HW limit
        if ((mFDW / mimage_width_array[j]) < 7)
        {
            mMustScaleIndex = j;
        }
    }
    MY_LOGD("mMustScaleIndex : %d", mMustScaleIndex);
    FDFTInitInfo.FDBufWidth = mimage_width_array[0];
    FDFTInitInfo.FDBufHeight = mimage_height_array[0];
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
    FDFTInitInfo.FDThreshold = FDCustomData.FDThreshold;
    #endif
    FDFTInitInfo.MajorFaceDecision = FDCustomData.MajorFaceDecision;
    FDFTInitInfo.OTRatio = FDCustomData.OTRatio;
    FDFTInitInfo.SmoothLevel = FDCustomData.SmoothLevel;
    FDFTInitInfo.Momentum = FDCustomData.Momentum;
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
    FDFTInitInfo.FDImageArrayNum = 14;
    FDFTInitInfo.FDImageWidthArray = mimage_width_array;
    FDFTInitInfo.FDImageHeightArray = mimage_height_array;
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
    FDFTInitInfo.ParallelRGB565Conversion = true;
    FDFTInitInfo.LockOtBufferFunc = LockFTBuffer;
    FDFTInitInfo.UnlockOtBufferFunc = UnlockFTBuffer;
    FDFTInitInfo.lockAgent = this;
    FDFTInitInfo.DisLimit = 0;
    FDFTInitInfo.DecreaseStep = 0;
    FDFTInitInfo.OTBndOverlap = 8;
    if (halFDGetVersion() < HAL_FD_VER_HW40)
    {
        FDFTInitInfo.DelayThreshold = 127; // 127 is default value for FD3.5
        FDFTInitInfo.DelayCount = 3; // 2 is default value
        FDFTInitInfo.DisLimit = 4;
        FDFTInitInfo.DecreaseStep = 384;
    }
    else
    {
        FDFTInitInfo.DelayThreshold = 75; // 83 is default value for FD4.0
        FDFTInitInfo.DelayCount = 2; // 2 is default value
    }
    FDFTInitInfo.FDManualMode = mUseUserScale;
    FDFTInitInfo.LandmarkEnableCnt = FldNum;
    #if MTK_ALGO_PSD_MODE
    FDFTInitInfo.FDVersion = MTKCAM_HWFD_MAIN_VERSION;
    #endif
    #if (USE_HW_FD)&&(HW_FD_SUBVERSION >= 2)
    FDFTInitInfo.FDMINSZ = 0;
    #endif

    FDFTInitInfo.FLDAttribConfig = 1;
    #if MTK_ALGO_PSD_MODE && USE_HW_FD
    #if (MTKCAM_HWFD_MAIN_VERSION == 40)
    FDVT_OpenDriverWithUserCount(FDFTInitInfo.FDModel - 1);
    mFDModelVer = FDVT_GetModelVersion();
    MY_LOGD("FD4.0 model ver : %d", mFDModelVer);
    FDFTInitInfo.ModelVersion = mFDModelVer;
    #else
    FDVT_OpenDriverWithUserCount(FDFTInitInfo.FDModel);
    #endif
    #endif
    // dump initial info
    dumpFDParam(FDFTInitInfo);
    // Set initial info to FD algo
    mpMTKFDVTObj->FDVTInit(&FDFTInitInfo);

    mEnableSWResizerFlag = SWResizerEnable;
    if(mEnableSWResizerFlag)
    {
        mImageScaleTotalSize = 0;
        for(i=0; i<FD_SCALES;i++)
        {
            mImageScaleTotalSize += mimage_width_array[i]*mimage_height_array[i];
        }
        mpImageScaleBuffer = new unsigned char[mImageScaleTotalSize];
    }

    mpImageVGABuffer = new unsigned char[gimage_input_width_vga*gimage_input_height_buffer];
    memset(mpImageVGABuffer, 0, gimage_input_width_vga*gimage_input_height_buffer);

    MY_LOGD("[%s] End", __FUNCTION__);
    mFrameCount = 0;
    mDetectedFaceNum = 0;
    #if USE_PORTRAIT
    MY_LOGD("use default model");
    if (gFitstPortInit == 0)
    {
        pthread_create(&mInitThread, NULL, onInitThreadFunc, this);
        gFitstPortInit = 1;
    }

    mNeedRunPort = 0;
    //for portrait
    mPortStop = 0;
    sem_init(&mSemPortExec, 0, 0);
    sem_init(&mSemPortLock, 0, 0);
    pthread_create(&mPortThread, NULL, onPortThreadLoop, this);
    //
    #endif
    mInited = 1;
    return err;
}

MINT32
halFDVT::halFDGetVersion(
)
{
    #if (USE_HW_FD)

    #if (MTKCAM_HWFD_MAIN_VERSION == 40)
    return HAL_FD_VER_HW40 + HW_FD_SUBVERSION;
    #endif // end MTKCAM_HWFD_MAIN_VERSION == 40

    #if (MTKCAM_HWFD_MAIN_VERSION == 35)
    #if (HW_FD_SUBVERSION == 1)
    return HAL_FD_VER_HW37;
    #else
    return HAL_FD_VER_HW36;
    #endif
    #endif

    #else // else USE_HW_FD
    return HAL_FD_VER_SW36;
    #endif // end USE_HW_FD
}

MINT32
halFDVT::halFDDo(
struct FD_Frame_Parameters &Param
)
{
    Mutex::Autolock _l(gInitLock);
    FdOptions FDOps;
    MUINT8* y_vga;
    MBOOL SDEnable;
    MINT32 StartPos = 0;
    MINT32 ScaleNum = mUserScaleNum;
    FDVT_OPERATION_MODE_ENUM Mode = FDVT_IDLE_MODE;
    #if USE_PORTRAIT
    MINT32 PortraitRunning = 0;
    #endif
    #if MTK_ALGO_PSD_MODE
    // FD3.5 & FD4.0
    fd_cal_struct *pfd_cal_data;
    // FD4.0
    //FdDrv_input_struct fdDrvInput;
    //FdDrv_output_struct fdDrvOutput;
    #endif

    SDEnable = Param.SDEnable && (mCurrentMode == HAL_FD_MODE_SD);
    //MY_LOGD("SD Status: SDEnable:%d, img_w:%d, img_h:%d, Src_Phy_Addr:%p,", mCurrentMode, mFTWidth, mFTHeight, Param.pImageBufferPhyP0);

    if(!mInited) {
        return MHAL_UNINIT_ERROR;
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

    FDOps.gfd_fast_mode = 0;
    #if (SINGLE_FACE_STABLE_ENABLE == 1)
    // Dynamic scaler
    if (mDetectedFaceNum != 0 && (mFrameCount % mFDRefresh) != 0) {
        MINT32 i;
        float WidthSize = (float)(2000.0 * mFDW) / (float)(mFDW - (Param.padding_w*2));
        float FaceRatio;
        MINT32 smallidx = 256, largeidx = -1;
        for(int j = 0; j < mDetectedFaceNum; j++)
        {
            FaceRatio = ((float)(mFaceResult[j].rect[2] - mFaceResult[j].rect[0])) / WidthSize;
            MY_LOGD("FGFD Ratio : %f, Normalized width : %f", FaceRatio, WidthSize);
            for (i = 0; i < mUserScaleNum; i++) {
                if(FaceRatio <= ((float)24/(float)mimage_width_array[i]))
                    break;
            }
            MY_LOGD("WillDBG stop loop : %d", i);
            if(i != 0) {
                float leftdiff, rightdiff;
                leftdiff = FaceRatio - ((float)24/(float)mimage_width_array[i - 1]);
                rightdiff = ((float)24/(float)mimage_width_array[i]) - FaceRatio;
                if(leftdiff < rightdiff) {
                    i--;
                }
            }
            if (i < smallidx)
            {
                smallidx = i;
            }
            if (i > largeidx)
            {
                largeidx = i;
            }
        }
        if(largeidx <= 1) {
            StartPos = 0;
            ScaleNum = 3 + largeidx;
        } else if (smallidx >= (mUserScaleNum - 2)) {
            ScaleNum = 3 + (mUserScaleNum - smallidx - 1);
            StartPos = mUserScaleNum - ScaleNum;
        } else {
            StartPos = (smallidx >= 2)? (smallidx - 2) : 0;
            ScaleNum = largeidx - smallidx + 5;
            if ((ScaleNum + StartPos) > mUserScaleNum)
            {
                ScaleNum = mUserScaleNum - StartPos;
            }
        }
        if (StartPos > mMustScaleIndex)
        {
            StartPos = mMustScaleIndex;
            ScaleNum = mUserScaleNum - StartPos;
        }
        FDOps.gfd_fast_mode = 1;
        MY_LOGD("WillDBG Start pos : %d, Scalenum : %d", StartPos, ScaleNum);
    }
    if (mDetectedFaceNum != 0) {
        Mode = FDVT_GFD_MODE;
    }
    mFrameCount++;
    #endif

    // Set FD operation
    FDOps.fd_state = FDVT_GFD_MODE;
    FDOps.direction = direction;
    FDOps.fd_scale_count = ScaleNum;
    FDOps.fd_scale_start_position = StartPos;
    FDOps.ae_stable = Param.AEStable;
    FDOps.ForceFDMode = Mode;
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
    FDOps.ImageScaleBuffer = mpImageScaleBuffer;
    FDOps.ImageBufferRGB565 = Param.pRGB565Image;
    FDOps.ImageBufferSrcVirtual = (MUINT8*)Param.pImageBufferVirtual;
    FDOps.startW = Param.padding_w;
    FDOps.startH = Param.padding_h;
    FDOps.model_version = mFDModelVer;

    if(mEnableSWResizerFlag)
    {
        FDVT_OPERATION_MODE_ENUM mode;
        mpMTKFDVTObj->FDVTGetMode(&mode);

        // Prepare buffer for FD algo
        #if (USE_HW_FD)
        if(SDEnable || halFDGetVersion() < HAL_FD_VER_HW40) {
            if(mFDW == 640) // VGA like size
            {
                y_vga = (MUINT8*)Param.pPureYImage;
                CreateScaleImages_Y( (MUINT8*)y_vga, mpImageScaleBuffer, mFDW, mFDH, mimage_width_array, mimage_height_array, 1);
            }
            else
            {
                MY_LOGD("Not VGA like size : %dx%d", mFDW, mFDH);
                Create640WidthImage_Y((MUINT8*)Param.pPureYImage, mFDW, mFDH, mpImageVGABuffer);
                y_vga = mpImageVGABuffer;
                CreateScaleImages_Y( (MUINT8*)Param.pPureYImage, mpImageScaleBuffer, mFDW, mFDH, mimage_width_array, mimage_height_array, 1);
            }
        }
        #else
        if(mFDW == 640) // VGA like size
        {
            y_vga = (MUINT8*)Param.pPureYImage;
            if( mode == FDVT_GFD_MODE || SDEnable)
            {
                CreateScaleImages_Y( (MUINT8*)y_vga, mpImageScaleBuffer, mFDW, mFDH, mimage_width_array, mimage_height_array, 1);
            }
        }
        else
        {
            if(SDEnable || mode == FDVT_GFD_MODE)
            {
                MY_LOGD("Not VGA like size : %dx%d", mFDW, mFDH);
                if(SDEnable) {
                    Create640WidthImage_Y((MUINT8*)Param.pPureYImage, mFDW, mFDH, mpImageVGABuffer);
                    y_vga = mpImageVGABuffer;
                } else {
                    //y_vga = ImageBuffer2;
                }
                CreateScaleImages_Y( (MUINT8*)Param.pPureYImage, mpImageScaleBuffer, mFDW, mFDH, mimage_width_array, mimage_height_array, 1);
            }
        }
        #endif // end of #if (USE_HW_FD)

        mpMTKFDVTObj->FDVTMain(&FDOps);
        #if MTK_ALGO_PSD_MODE
        if (FDOps.doPhase2)
        {
            pfd_cal_data = mpMTKFDVTObj->FDGetCalData();//Get From Algo

            //MY_LOGD("FDVTMain out1");

            //Add Driver Control
            //
            #if USE_PORTRAIT
            if (mNeedRunPort > 0 && gPortInited)
            //if (gPortInited)
            {
                MY_LOGD("USE Portrait");
                mPortraitPara.pCal_Data = pfd_cal_data;
                mPortraitPara.InWidth = mFDW;
                mPortraitPara.InHeight = mFDH;
                mPortraitPara.pFDBufVirtual = (void *)Param.pImageBufferVirtual;
                RunPortrait(this);
                PortraitRunning = 1;
            }
            MY_LOGD("USE HW FD 1");
            #endif
            doHWFaceDetection(pfd_cal_data);


            mpMTKFDVTObj->FDVTMainPhase2();//call To Algo
        }
        #endif
        // Wait FT Buffer done.
        LockFTBuffer(this);
        UnlockFTBuffer(this);

        result  DetectResult[MAX_FACE_NUM];
        mpMTKFDVTObj->FDVTGetResult( (MUINT8 *)&DetectResult, FACEDETECT_TRACKING_DISPLAY);

        if(SDEnable)
        {
            FDOps.fd_state = FDVT_SD_MODE;
            FDOps.ImageBufferRGB565 = y_vga;
            mpMTKFDVTObj->FDVTMain(&FDOps);
        }
    }
    else
    {
        FDOps.ImageScaleBuffer = Param.pScaleImages;
        mpMTKFDVTObj->FDVTMain(&FDOps);

        if(SDEnable)
        {
            FDOps.fd_state = FDVT_SD_MODE;
            FDOps.ImageBufferRGB565 = Param.pPureYImage;
            mpMTKFDVTObj->FDVTMain(&FDOps);
        }
    }
    #if USE_PORTRAIT
    if(PortraitRunning)
    {
        WaitPortrait(this);
    }
    #endif

    return MHAL_NO_ERROR;
}

// internal function
bool
halFDVT::doHWFaceDetection(
    void *pCal_Data
)
{
#if MTK_ALGO_PSD_MODE && USE_HW_FD
    fd_cal_struct *pfd_cal_data = (fd_cal_struct *)pCal_Data;
#if 1
    int i;
    FdDrv_input_struct mFDDrvInput;
    FdDrv_output_struct mFDDrvOutput;
    FdDrv_input_struct *FDDrvInput = &mFDDrvInput;
    FdDrv_output_struct *FDDrvOutput = &mFDDrvOutput;
    int width = pfd_cal_data->img_width_array[0];
    int start_pos = pfd_cal_data->fd_scale_start_position;
    FDDrvInput->fd_mode = 1;
    if (pfd_cal_data->inputPlaneCount == 1) {
        FDDrvInput->source_img_fmt = FMT_YUYV;
    } else if (pfd_cal_data->inputPlaneCount == 2) {
        FDDrvInput->source_img_fmt = FMT_YUV_2P;
    } else {
        MY_LOGD("Warning!!!! the plane count : %d is not supported", pfd_cal_data->inputPlaneCount);
        FDDrvInput->source_img_fmt = FMT_YUYV;
    }
    //FDDrvInput->fd_mode = 0; // 640x480
    //FDDrvInput->source_img_fmt = FMT_YUYV;
    FDDrvInput->scale_manual_mode = pfd_cal_data->fd_manual_mode;
    if (pfd_cal_data->fd_manual_mode) {
        FDDrvInput->source_img_width[0] = pfd_cal_data->fd_img_src_width;
        FDDrvInput->source_img_height[0] = pfd_cal_data->fd_img_src_height;
    } else {
        FDDrvInput->source_img_width[0] = 640;
        FDDrvInput->source_img_height[0] = 480;
    }
#if (MTKCAM_HWFD_MAIN_VERSION == 40)
    int j;
    for (j = 0; j < 18; j++)
    {
        FDDrvInput->dynamic_change_model[j] = 0;
    }
#endif
    FDDrvInput->scale_num_from_user = pfd_cal_data->fd_scale_count;
    memcpy(&(FDDrvInput->source_img_width[1]), &(pfd_cal_data->img_width_array[start_pos]), sizeof(pfd_cal_data->img_width_array[0])*pfd_cal_data->fd_scale_count);
    memcpy(&(FDDrvInput->source_img_height[1]), &(pfd_cal_data->img_height_array[start_pos]), sizeof(pfd_cal_data->img_height_array[0])*pfd_cal_data->fd_scale_count);
    FDDrvInput->feature_threshold = 0;
    #if (MTKCAM_HWFD_MAIN_VERSION == 40)
    if(pfd_cal_data->g_scale_frame_division[0]) {
        FDDrvInput->GFD_skip = 1;
    } else {
        FDDrvInput->GFD_skip = 0;
    }
    FDDrvInput->GFD_skip_V = 0;
    #else
    MY_LOGD("FD skip num : %d", pfd_cal_data->g_scale_frame_division[0]);
    FDDrvInput->GFD_skip = pfd_cal_data->g_scale_frame_division[0];
    #endif
    FDDrvInput->RIP_feature = pfd_cal_data->current_feature_index;

    FDDrvInput->scale_from_original = 0;
    FDDrvInput->source_img_address = (MUINT64*)pfd_cal_data->srcbuffer_phyical_addr_plane1;
    if (FDDrvInput->source_img_fmt == FMT_YUV_2P) {
        FDDrvInput->source_img_address_UV = (MUINT64*)pfd_cal_data->srcbuffer_phyical_addr_plane2;
    } else {
        FDDrvInput->source_img_address_UV = NULL;
    }
    for(i = 0; i < MAX_FACE_SEL_NUM; i++) {
        pfd_cal_data->display_flag[i] = (kal_bool)0;
    }

    FDVT_Enque(FDDrvInput);
    FDVT_Deque(FDDrvOutput);
    //MY_LOGD("new_face_number : %d", FDDrvOutput->new_face_number);

    for(i = 0; i < FDDrvOutput->new_face_number; i++) {
        pfd_cal_data->face_candi_pos_x0[i] = FDDrvOutput->face_candi_pos_x0[i] * width / 640;
        pfd_cal_data->face_candi_pos_y0[i] = FDDrvOutput->face_candi_pos_y0[i] * width / 640;
        pfd_cal_data->face_candi_pos_x1[i] = FDDrvOutput->face_candi_pos_x1[i] * width / 640;
        pfd_cal_data->face_candi_pos_y1[i] = FDDrvOutput->face_candi_pos_y1[i] * width / 640;
        pfd_cal_data->face_reliabiliy_value[i] = FDDrvOutput->face_reliabiliy_value[i];
        pfd_cal_data->display_flag[i] = (kal_bool)1;
        pfd_cal_data->face_feature_set_index[i] = FDDrvOutput->face_feature_set_index[i];
        pfd_cal_data->rip_dir[i] = FDDrvOutput->rip_dir[i];
        pfd_cal_data->rop_dir[i] = FDDrvOutput->rop_dir[i];
        pfd_cal_data->rop_dir[i] = FDDrvOutput->rop_dir[i];
        pfd_cal_data->result_type[i] = GFD_RST_TYPE;
        pfd_cal_data->face_candi_model[i] = 0;
    }
#if (MTKCAM_HWFD_MAIN_VERSION == 40)
    if (halFDGetVersion() >= HAL_FD_VER_HW43)
    {
        return true;
    }
    // work around, use other model to fix hw bug and limit total result couldn't larger than 1024
    if (pfd_cal_data->current_feature_index >= 4 && pfd_cal_data->current_feature_index <= 9)
    {
        int offset = FDDrvOutput->new_face_number;
        for (j = 0; j < 18; j++)
        {
            FDDrvInput->dynamic_change_model[j] = 1;
        }
        FDVT_Enque(FDDrvInput);
        FDVT_Deque(FDDrvOutput);
        if ( (FDDrvOutput->new_face_number + offset) > MAX_FACE_SEL_NUM )
        {
            FDDrvOutput->new_face_number = MAX_FACE_SEL_NUM - offset;
        }
        for(i = 0; i < FDDrvOutput->new_face_number; i++) {
            pfd_cal_data->face_candi_pos_x0[i + offset] = FDDrvOutput->face_candi_pos_x0[i] * width / 640;
            pfd_cal_data->face_candi_pos_y0[i + offset] = FDDrvOutput->face_candi_pos_y0[i] * width / 640;
            pfd_cal_data->face_candi_pos_x1[i + offset] = FDDrvOutput->face_candi_pos_x1[i] * width / 640;
            pfd_cal_data->face_candi_pos_y1[i + offset] = FDDrvOutput->face_candi_pos_y1[i] * width / 640;
            pfd_cal_data->face_reliabiliy_value[i + offset] = FDDrvOutput->face_reliabiliy_value[i];
            pfd_cal_data->display_flag[i + offset] = (kal_bool)1;
            pfd_cal_data->face_feature_set_index[i + offset] = FDDrvOutput->face_feature_set_index[i];
            pfd_cal_data->rip_dir[i + offset] = FDDrvOutput->rip_dir[i];
            pfd_cal_data->rop_dir[i + offset] = FDDrvOutput->rop_dir[i];
            pfd_cal_data->rop_dir[i + offset] = FDDrvOutput->rop_dir[i];
            pfd_cal_data->result_type[i + offset] = GFD_RST_TYPE;
            pfd_cal_data->face_candi_model[i + offset] = 1;
        }
    }
#endif
#endif
#endif
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

    #if MTK_ALGO_PSD_MODE && USE_HW_FD
    FDVT_CloseDriverWithUserCount();
    #endif

    mpMTKFDVTObj->FDVTReset();

    #if USE_PORTRAIT
    //for portrait
    mPortStop = 1;
    ::sem_post(&mSemPortExec);
    pthread_join(mPortThread, NULL);
    sem_destroy(&mSemPortExec);
    sem_destroy(&mSemPortLock);
    mNeedRunPort = 0;
    //
    #endif

    if (mEnableSWResizerFlag)
    {
         delete[]mpImageScaleBuffer;
    }
    delete[]mpImageVGABuffer;
    mInited = 0;

    return MHAL_NO_ERROR;
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
    MINT32 /*ResultMode*/
)
{

    MINT32 faceCnt = 0;
    result pbuf[MAX_FACE_NUM];
    MUINT8 i;
    MINT8 DrawMode = 0;
    MINT32 PrevFaceNum = mDetectedFaceNum;

    faceCnt = mpMTKFDVTObj->FDVTGetResult((MUINT8 *) pbuf, FACEDETECT_TRACKING_DISPLAY);

    MY_LOGD("[%s]first scale W(%d) H(%d)", __FUNCTION__, mimage_width_array[0], mimage_height_array[0]);
    mpMTKFDVTObj->FDVTGetICSResult((MUINT8 *) fd_result, (MUINT8 *) pbuf, mimage_width_array[0], mimage_height_array[0], 0, 0, 0, DrawMode);

    mDetectedFaceNum = mFdResult_Num = fd_result->number_of_faces;

    fd_result->CNNFaces.PortEnable = 0;
    fd_result->CNNFaces.IsTrueFace = 0;

    #if USE_PORTRAIT
    if (mNeedRunPort && mvCNNout.size() >= 2)
    {
        MY_LOGD("mvCNNout : %f, %f", mvCNNout[0], mvCNNout[1]);
        fd_result->CNNFaces.PortEnable = 1;
        fd_result->CNNFaces.CnnResult0 = mvCNNout[0];
        fd_result->CNNFaces.CnnResult1 = mvCNNout[1];
        fd_result->CNNFaces.IsTrueFace = (mvCNNout[1] > mvCNNout[0]);
    }
    else
    {
        fd_result->CNNFaces.PortEnable = 0;
        fd_result->CNNFaces.IsTrueFace = 0;
        fd_result->CNNFaces.CnnResult0 = 0.0;
        fd_result->CNNFaces.CnnResult1 = 0.0;
    }
    if((PrevFaceNum != 0 && mDetectedFaceNum == 0) || ((mNeedRunPort > 0 && mNeedRunPort < 50 && mDetectedFaceNum == 0) && fd_result->CNNFaces.IsTrueFace))
    {
        if (gPortInited)
        {
            mNeedRunPort++;
        }
        else
        {
            mNeedRunPort = 0;
        }
    }
    else
    {
        mNeedRunPort = 0;
    }
    #endif

    for(i=0;i < mFdResult_Num;i++) {
        mFaceResult[i].rect[0] = fd_result->faces[i].rect[0];
        mFaceResult[i].rect[1] = fd_result->faces[i].rect[1];
        mFaceResult[i].rect[2] = fd_result->faces[i].rect[2];
        mFaceResult[i].rect[3] = fd_result->faces[i].rect[3];
    }

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
            faceCnt--;
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

    return faceCnt;
}

MINT32
halFDVT::halSDGetSmileResult()
{
    MINT32 SmileCnt = 0;
    MUINT32 pbuf1;

    #if (0 == SMILE_DETECT_SUPPORT)
    // If Smile detection is not support, return zero directly.
    return 0;
    #endif

    SmileCnt = mpMTKFDVTObj->FDVTGetSDResult(pbuf1);

    return SmileCnt;
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
    int i,j;

    int length = src_width*src_height*2;

    for(i=length;i != 0;i-=2)
    {
      *dstAddr++ = *srcAddr;
      srcAddr+=2;
    }

    MY_LOGD("DO Extract Y Out");

    return MHAL_NO_ERROR;
}

// Create RGB565 QVGA for Tracking directly
MINT32
halFDVT::halFTBufferCreate(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8 ucPlane,
MUINT32 src_width,
MUINT32 src_height
)
{
    if((src_width == 640) && (ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW) In");
        doRGB565BufferP1_SW(dstAddr, srcAddr, src_width, src_height);
        MY_LOGD("DO RGB565 (SW) Out");
    }
    else if((src_width == 320) && (ucPlane ==1))
    {
        MY_LOGD("DO RGB565 (SW_QVGA) In");
        doRGB565BufferQVGAP1_SW(dstAddr, srcAddr, src_width, src_height);
        MY_LOGD("DO RGB565 (SW_QVGA) Out");
    }
    else
    {
        MY_LOGD("DO RGB565 (DDP) In");
        mFTWidth =  MHAL_FDVT_FTBUF_W;
        mFTHeight = mFTWidth * src_height / src_width;
        doRGB565Buffer_DDP(dstAddr, srcAddr, ucPlane);
        MY_LOGD("DO RGB565 (DDP) Out");
    }

    if(mDoDumpImage)
        dumpFDImages(srcAddr, dstAddr);

    return MHAL_NO_ERROR;
}

// Create RGB565 QVGA for Tracking directly
MINT32
halFDVT::halGetFTBufferSize(
MUINT32 *width,
MUINT32 *height
)
{
    *width = mFTWidth;
    *height = mFTHeight;
    return MHAL_NO_ERROR;
}

// Create RGB565 QVGA for Tracking use another thread
MINT32
halFDVT::halFTBufferCreateAsync(
MUINT8 *dstAddr,
MUINT8 *srcAddr,
MUINT8 ucPlane,
MUINT32 src_width,
MUINT32 src_height
)
{
    mFTParameter.dstAddr = dstAddr;
    mFTParameter.srcAddr = srcAddr;
    mFTParameter.ucPlane = ucPlane;
    mFTParameter.src_width = src_width;
    mFTParameter.src_height = src_height;
    LockFTBuffer(this);
    mFTBufReady = 0;
    ::sem_post(&mSemFTExec);
    return MHAL_NO_ERROR;
}

/*******************************************************************************
* Private function
********************************************************************************/

#if USE_PORTRAIT
MINT32
halFDVT::DoDLPortrait(
    CNN_THREAD_PARA *pInput
)
{
    P_CNN_THREAD_PARA pPara = pInput;

    int fdcnn_on = ::property_get_int32("vendor.debug.FDCNN.On", 1);
    int thr_100x = ::property_get_int32("vendor.debug.FDCNN.Thr", 50);

    static int dump_cnt = 0;
    char szFileName[256];
    FILE* pFp ;
    void* pMDPDstImgBuf;

    //MY_LOGD("fdvt portrait : %p", gpPortrait.get());

    int target_w = gpPortrait->get_input_width();
    int target_h = gpPortrait->get_input_height();
    pMDPDstImgBuf = gpPortrait->map_input_buffer();
    //MY_LOGD("fdvt portrait : input : %p", pMDPDstImgBuf);

    #ifdef IMG2ODUMP
    sprintf(szFileName, "/sdcard/FDDump/FD_img2odump_%dx%d_%03d.yuy2", srcWidth, srcHeight, dump_cnt);
    pFp = fopen(szFileName, "wb");
    if (NULL == pFp) {
        MY_LOGE("[DumpToFile] failed to create img2o file: %s", szFileName);
        return -1;
    }
    fwrite(Param.pImageBufferVirtual, sizeof(MUINT8), srcWidth*srcHeight*2, pFp);
    fclose(pFp);
    #endif
    YUY2toRGB888(pPara->pFDBufVirtual, pPara->InWidth, pPara->InHeight, target_w, target_h, pMDPDstImgBuf);
    gpPortrait->unmap_input_buffer();

    #ifdef MDPDUMP
    sprintf(szFileName, "/sdcard/FDDump/MdpReformat_%dx%d_%03d.rgb", target_w, target_h, dump_cnt++);
    pFp = fopen(szFileName, "wb");
    if (NULL == pFp) {
        MY_LOGE("[DumpToFile] failed to create mdp file: %s", szFileName);
    }
    fwrite(pMDPDstImgBuf, sizeof(MUINT8), target_w*target_h*3, pFp);
    fclose(pFp);
    #endif

    //MY_LOGD("CNN MDP Done\n");

    //Execute squeezenent.
    mvCNNout = gpPortrait->forward();
    MY_LOGD("CNN Result %d (%3.6f, %3.6f)\n", (mvCNNout[1] >= mvCNNout[0] ? 1 : 0), mvCNNout[0], mvCNNout[1]);

    pMDPDstImgBuf = NULL;

    return 0;
}
#endif

// Create RGB565 QVGA for Tracking, use another thread
void* halFDVT::onFTThreadLoop(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    while(1) {
        ::sem_wait(&(_this->mSemFTExec));
        if(_this->mFTStop) {
            break;
        }
        {
            _this->halFTBufferCreate(_this->mFTParameter.dstAddr,
                                     _this->mFTParameter.srcAddr,
                                     _this->mFTParameter.ucPlane,
                                     _this->mFTParameter.src_width,
                                     _this->mFTParameter.src_height);
            _this->mFTBufReady = 1;
        }
        _this->UnlockFTBuffer(_this);
    }
    return NULL;
}
void halFDVT::LockFTBuffer(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    ::sem_wait(&(_this->mSemFTLock)); // lock FT Buffer
}
void halFDVT::UnlockFTBuffer(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    ::sem_post(&(_this->mSemFTLock)); // lock FT Buffer
}

#if USE_PORTRAIT
// Create RGB565 QVGA for Tracking, use another thread
void* halFDVT::onPortThreadLoop(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    while(1) {
        ::sem_wait(&(_this->mSemPortExec));
        if(_this->mPortStop) {
            break;
        }
        {
            _this->DoDLPortrait(&(_this->mPortraitPara));
        }
        _this->ReleasePortrait(_this);
    }
    return NULL;
}
void halFDVT::RunPortrait(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    ::sem_post(&(_this->mSemPortExec));
}
void halFDVT::WaitPortrait(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    ::sem_wait(&(_this->mSemPortLock));
}

void halFDVT::ReleasePortrait(void* arg)
{
    halFDVT *_this = reinterpret_cast<halFDVT*>(arg);
    ::sem_post(&(_this->mSemPortLock));
}

#endif
// Create RGB565 buffer
bool halFDVT::doRGB565BufferP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth, MUINT32 SrcHeight)
{
     bool ret = true;

    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;

    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[2], G[2], B[2], r, g, b;
    unsigned int dImgW = SrcWidth;
    unsigned int dImgH = SrcHeight;
    //int dImgW = g_FDW;
    //int dImgH = g_FDH;

    pucYBuf = src_yp;

    for(i=0;i<dImgH;i=i+2)
    {
        for(j=0;j<dImgW*2;j=j+4)
        {
            Y[0] = *(pucYBuf + ((i+0) * dImgW*2) + j);
            Y[1] = *(pucYBuf + ((i+0) * dImgW*2) + j+2) ;
            Y[2] = *(pucYBuf + ((i+1) * dImgW*2) + j);
            Y[3] = *(pucYBuf + ((i+1) * dImgW*2) + j+2) ;

            Y [0]= (Y[0]+Y[1]+Y[2]+Y[3]) >> 2;
            U  =  (*(pucYBuf + ((i+0) * dImgW*2) + j+1) + *(pucYBuf + ((i+1) * dImgW*2) + j+1) ) >> 1;
            V  =  (*(pucYBuf + ((i+0) * dImgW*2) + j+3) + *(pucYBuf + ((i+1) * dImgW*2) + j+3) ) >> 1;

            for(k=0;k<1;k++)
            {
                r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
                g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
                b = (32 * Y[k] + 57 * (U-128) + 16) / 32;

                R[k] = (r<0) ? 0: (r>255) ? 255 : r;
                G[k]= (g<0) ? 0: (g>255) ? 255 : g;
                B[k] = (b<0) ? 0: (b>255) ? 255 : b;
            }

            *(dst_rgb + ((i>>1)  * dImgW ) + (j>>1)+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
            *(dst_rgb + ((i>>1)  * dImgW ) + (j>>1)+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));
        }
    }

        return ret;
}
bool halFDVT::doRGB565BufferQVGAP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth1, MUINT32 SrcHeight1)
{
     bool ret = true;

    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;

    unsigned char* pucYBuf;
    unsigned char* pucUVBuf;
    unsigned int i, j, k;
    int Y[4], U, V, R[2], G[2], B[2], r, g, b;
    unsigned int dImgW = SrcWidth1;
    unsigned int dImgH = SrcHeight1;
    //int dImgW = g_FDW;
    //int dImgH = g_FDH;

    pucYBuf = src_yp;

    for(i=0;i<dImgH;i++)
    {
        for(j=0;j<dImgW*2;j=j+4)
        {
            Y[0] = *(pucYBuf + (i * dImgW*2) + j+0);
            Y[1] = *(pucYBuf + (i * dImgW*2) + j+2);

            U  =  *(pucYBuf + (i * dImgW*2) + j+1);
            V  =  *(pucYBuf + (i * dImgW*2) + j+3);

            for(k=0;k<2;k++)
            {
                r = (32 * Y[k] + 45 * (V-128) + 16) / 32;
                g = (32 * Y[k] - 11 * (U-128) - 23 * (V-128) + 16) / 32;
                b = (32 * Y[k] + 57 * (U-128) + 16) / 32;

                R[k] = (r<0) ? 0: (r>255) ? 255 : r;
                G[k] = (g<0) ? 0: (g>255) ? 255 : g;
                B[k] = (b<0) ? 0: (b>255) ? 255 : b;
            }

            *(dst_rgb + ((i<<1)  * dImgW ) + j+0) = ((G[0] & 0x1C) <<3 ) + ((B[0] & 0xF8) >>3 );
            *(dst_rgb + ((i<<1)  * dImgW ) + j+1) = ((G[0] & 0xE0) >>5 ) + ((R[0] & 0xF8));

            *(dst_rgb + ((i<<1)  * dImgW ) + j+2) = ((G[1] & 0x1C) <<3 ) + ((B[1] & 0xF8) >>3 );
            *(dst_rgb + ((i<<1)  * dImgW ) + j+3) = ((G[1] & 0xE0) >>5 ) + ((R[1] & 0xF8));

        }
    }

        return ret;
}
bool halFDVT::doRGB565Buffer_DDP(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 planes)
{
    bool ret = true;

#ifdef FDVT_DDP_SUPPORT
    DpBlitStream FDstream;
    unsigned char *src_yp = (unsigned char *)a_srcAddr;
    unsigned char *dst_rgb = (unsigned char *)a_dstAddr;
    void* src_addr_list[3];
    unsigned int src_size_list[3];
    void* dst_addr_list[3];
    unsigned int dst_size_list[3];

    int src_ysize = mFDW * mFDH;
    int src_usize, src_vsize;
    src_usize = src_vsize = src_ysize / 4;

    //*****************************************************************************//
    //*******************src  YUY2/NV21/YV12 **************************************//
    //*****************************************************************************//
    if(planes ==1)
    {
        FDstream.setSrcBuffer((void *)src_yp, mFDW*mFDH*2);
        FDstream.setSrcConfig(mFDW, mFDH, DP_COLOR_YUYV);
    }

    else if(planes ==2)
    {

        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = (void *)(src_yp + src_ysize);
        src_size_list[0] = src_ysize;
        src_size_list[1] = src_usize + src_vsize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, planes);
        FDstream.setSrcConfig(mFDW, mFDH, DP_COLOR_NV21); //82&72
    }

    else if(planes ==3)
    {
        src_addr_list[0] = (void *)src_yp;
        src_addr_list[1] = (void *)(src_yp + src_ysize);
        src_addr_list[2] = (void *)(src_yp + src_ysize * 5/4);

        src_size_list[0] = src_ysize;
        src_size_list[1] = src_vsize;
        src_size_list[2] = src_usize;
        FDstream.setSrcBuffer((void**)src_addr_list, src_size_list, planes);
        FDstream.setSrcConfig(mFDW, mFDH, DP_COLOR_YV12); //82&72
    }

    //***************************dst RGB565********************************//
    FDstream.setDstBuffer((void *)dst_rgb, mFTWidth*mFTHeight*2);
    FDstream.setDstConfig(mFTWidth, mFTHeight, eRGB565);
    FDstream.setRotate(0);

    //***********************************************************************//
    //Adjust FD thread priority to RR
    //int const policy    = SCHED_OTHER;
#if MTKCAM_HAVE_RR_PRIORITY
    int policy    = SCHED_RR;
    int priority  = PRIO_RT_FD_THREAD;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
    //***********************************************************************//

    // set & add pipe to stream
    if (0>FDstream.invalidate())  //trigger HW
    {
          MY_LOGD("FDstream invalidate failed");
          //***********************************************************************//
          //Adjust FD thread priority to Normal and return false
          //***********************************************************************//
#if MTKCAM_HAVE_RR_PRIORITY
          policy    = SCHED_OTHER;
          priority  = 0;
          ::sched_getparam(0, &sched_p);
          sched_p.sched_priority = priority;  //  Note: "priority" is nice value
          sched_setscheduler(0, policy, &sched_p);
          setpriority(PRIO_PROCESS, 0, priority);
#endif
          return false;
    }

    //***********************************************************************//
    //Adjust FD thread priority to Normal
#if MTKCAM_HAVE_RR_PRIORITY
     policy    = SCHED_OTHER;
     priority  = 0;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
#endif
    //***********************************************************************//

#endif
     return ret;
}
// Dump FD and FT image
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
    MY_LOGD("WorkingBufAddr = %p", FDFTInitInfo.WorkingBufAddr);
    MY_LOGD("WorkingBufSize = %d", FDFTInitInfo.WorkingBufSize);
    MY_LOGD("FDThreadNum = %d", FDFTInitInfo.FDThreadNum);
    MY_LOGD("FDThreshold = %d", FDFTInitInfo.FDThreshold);
    MY_LOGD("DelayThreshold = %d", FDFTInitInfo.DelayThreshold);
    MY_LOGD("MajorFaceDecision = %d", FDFTInitInfo.MajorFaceDecision);
    MY_LOGD("OTRatio = %d", FDFTInitInfo.OTRatio);
    MY_LOGD("SmoothLevel = %d", FDFTInitInfo.SmoothLevel);
    MY_LOGD("FDSkipStep = %d", FDFTInitInfo.FDSkipStep);
    MY_LOGD("FDRectify = %d", FDFTInitInfo.FDRectify);
    MY_LOGD("FDRefresh = %d", FDFTInitInfo.FDRefresh);
    MY_LOGD("FDBufWidth = %d", FDFTInitInfo.FDBufWidth);
    MY_LOGD("FDBufHeight = %d", FDFTInitInfo.FDBufHeight);
    MY_LOGD("FDTBufWidth = %d", FDFTInitInfo.FDTBufWidth);
    MY_LOGD("FDTBufHeight = %d", FDFTInitInfo.FDTBufHeight);
    MY_LOGD("FDImageArrayNum = %d", FDFTInitInfo.FDImageArrayNum);
    MY_LOGD("FDImageWidthArray = ");
    for (int i = 0; i < FD_SCALES; i++)
    {
        MY_LOGD("%d, ",FDFTInitInfo.FDImageWidthArray[i]);
    }
    MY_LOGD("\n");
    MY_LOGD("FDImageHeightArray = ");
    for (int i = 0; i < FD_SCALES; i++)
    {
        MY_LOGD("%d, ",FDFTInitInfo.FDImageHeightArray[i]);
    }
    MY_LOGD("\n");
    MY_LOGD("FDMinFaceLevel = %d", FDFTInitInfo.FDMinFaceLevel);
    MY_LOGD("FDMaxFaceLevel = %d", FDFTInitInfo.FDMaxFaceLevel);
    MY_LOGD("FDImgFmtCH1 = %d", FDFTInitInfo.FDImgFmtCH1);
    MY_LOGD("FDImgFmtCH2 = %d", FDFTInitInfo.FDImgFmtCH2);
    MY_LOGD("SDImgFmtCH1 = %d", FDFTInitInfo.SDImgFmtCH1);
    MY_LOGD("SDImgFmtCH2 = %d", FDFTInitInfo.SDImgFmtCH2);
    MY_LOGD("SDThreshold = %d", FDFTInitInfo.SDThreshold);
    MY_LOGD("SDMainFaceMust = %d", FDFTInitInfo.SDMainFaceMust);
    MY_LOGD("GSensor = %d", FDFTInitInfo.GSensor);
    MY_LOGD("GenScaleImageBySw = %d", FDFTInitInfo.GenScaleImageBySw);
    MY_LOGD("FDManualMode = %d", FDFTInitInfo.FDManualMode);
    MY_LOGD("mUserScaleNum = %d", mUserScaleNum);
    #if MTK_ALGO_PSD_MODE
    MY_LOGD("FDVersion = %d", FDFTInitInfo.FDVersion);
    #endif
    #if (USE_HW_FD)&&(HW_FD_SUBVERSION >= 2)
    MY_LOGD("FDMINSZ = %d", FDFTInitInfo.FDMINSZ);
    #endif
    MY_LOGD("Version = %d", halFDGetVersion());
    MY_LOGD("DisLimit = %d", FDFTInitInfo.DisLimit);
    MY_LOGD("DecreaseStep = %d", FDFTInitInfo.DecreaseStep);
}
