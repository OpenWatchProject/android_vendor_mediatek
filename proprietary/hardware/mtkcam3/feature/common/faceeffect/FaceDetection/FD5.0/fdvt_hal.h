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
** $Log: fd_hal_base.h $
 *
*/

#ifndef _FDVT_HAL_H_
#define _FDVT_HAL_H_

#include <mtkcam3/feature/FaceDetection/fd_hal_base.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <utils/RefBase.h>
#include "MTKDetection.h"
#include <mtkcam/drv/iopipe/PostProc/IEgnStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalEgnPipe.h>
#include <mtkcam/drv/def/fdvtcommon.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

///////////////////////////////////////////////////////////////////////////
//! Helper macros to check error code
///////////////////////////////////////////////////////////////////////////
#define SUCCEEDED(Status)   ((MRESULT)(Status) >= 0)
#define FAILED(Status)      ((MRESULT)(Status) < 0)

#define MODULE_MTK_CNN (0) // Temp value

#define ERRCODE(modid, errid)           \
  ((MINT32)                              \
    ((MUINT32)(0x80000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(errid & 0xffff))          \
  )

#define OKCODE(modid, okid)             \
  ((MINT32)                              \
    ((MUINT32)(0x00000000) |             \
     (MUINT32)((modid & 0x7f) << 24) |   \
     (MUINT32)(okid & 0xffff))           \
  )


#define MTK_CNN_OKCODE(errid)         OKCODE(MODULE_MTK_CNN, errid)
#define MTK_CNN_ERRCODE(errid)        ERRCODE(MODULE_MTK_CNN, errid)


// Detection error code
#define S_CNN_OK                  MTK_CNN_OKCODE(0x0000)

#define E_CNN_NEED_OVER_WRITE     MTK_CNN_ERRCODE(0x0001)
#define E_CNN_NULL_OBJECT         MTK_CNN_ERRCODE(0x0002)
#define E_CNN_WRONG_STATE         MTK_CNN_ERRCODE(0x0003)
#define E_CNN_WRONG_CMD_ID        MTK_CNN_ERRCODE(0x0004)
#define E_CNN_WRONG_CMD_PARAM     MTK_CNN_ERRCODE(0x0005)
#define E_CNN_INSUFF_WORK_BUF     MTK_CNN_ERRCODE(0x0006)
#define E_CNN_WRONG_IMAGE_FMT     MTK_CNN_ERRCODE(0x0007)
#define E_CNN_ILLEGAL_CONFIG      MTK_CNN_ERRCODE(0x0008)
#define E_CNN_RECTIFY_FAIL        MTK_CNN_ERRCODE(0x0009)
#define E_CNN_ILLEGAL_CORE_NUM    MTK_CNN_ERRCODE(0x000A)
#define E_CNN_NULL_FILE_POINTER   MTK_CNN_ERRCODE(0x000B)
#define E_CNN_MDP_FAIL            MTK_CNN_ERRCODE(0x000C)
#define E_CNN_ABORT               MTK_CNN_ERRCODE(0x000D)

#define E_CNN_ERR                 MTK_CNN_ERRCODE(0x0100)

using FDVTConfig = NSCam::NSIoPipe::FDVTConfig;
using namespace NSCam::NSIoPipe::NSEgn;
using namespace NSCam;
using namespace android;

typedef struct CNN_THREAD_PARA
{
    MINT32 InWidth;
    MINT32 InHeight;
    MINT32 CNNFDFilterTH;
    MINT32 CNNFDOverlapTH;
    void* pFDBufVirtual;
    void* pCal_Data;

}CNN_THREAD_PARA, *P_CNN_THREAD_PARA;

class MTKDetection;
/*******************************************************************************
*
********************************************************************************/
class halFDVT: public halFDBase
{
public:
    //
    static halFDBase* getInstance();
    virtual void destroyInstance();
    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief FD Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    halFDVT();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual ~halFDVT();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDInit () -
    //! \brief init face detection
    //
    /////////////////////////////////////////////////////////////////////////
    //virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT32 WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL   SWResizerEnable);
    virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT8 *WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL isNormalWorld, MUINT8 Current_mode, MINT32 FldNum = 1);

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDGetVersion () -
    //! \brief get FD version
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDGetVersion();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTDo () -
    //! \brief process face detection
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDDo(struct FD_Frame_Parameters &Param);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTUninit () -
    //! \brief FDVT uninit
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDUninit();

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTGetFaceInfo () -
    //! \brief get face detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDGetFaceInfo(MtkCameraFaceMetadata *fd_info_result);

    /////////////////////////////////////////////////////////////////////////
    //
    // mHalFDVTGetFaceResult () -
    //! \brief get face detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDGetFaceResult(MtkCameraFaceMetadata * fd_result, MINT32 ResultMode = 1);

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDYUYV2ExtractY () -
    //! \brief create Y Channel
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDYUYV2ExtractY(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT32 src_width, MUINT32 src_height);

    // for FD5.0
    bool handleFDCallback(EGNParams<FDVTConfig>& rParams);
    bool doHWAttribute();

    virtual MINT32 halGetGammaSetting(MINT32* &gammaCtrl);

private:
    static void* onAttThreadLoop(void*);

    bool doHWFaceDetection(void *pCal_Data);

    void dumpFDImages(MUINT8* pFDImgBuffer, MUINT8* pFTImgBuffer);
    void dumpFDParam(MTKFDFTInitInfo &FDFTInitInfo);

protected:

    MTKDetection* mpMTKFDVTObj;

    MUINT32 mFDW;
    MUINT32 mFDH;
    MUINT32 mFDhw_W;
    MUINT32 mFDhw_H;
    MUINT32 mBuffCount;
    MBOOL   mInited;

    MINT32  mFdResult_Num;
    FD_RESULT mFdResult[15];
    MUINT32 mFTWidth;
    MUINT32 mFTHeight;
    MINT32  mCurrentMode;
    MUINT32 mDoDumpImage;
    float   mFDFilterRatio;

    MUINT32 mFrameCount;
    MUINT32 mDetectedFaceNum;
    MUINT32 mFDRefresh;
    MINT32  mSkipPartialFD; //only skip detect part
    MINT32  mSkipAllFD; // skip detect part and post-processing part
    NSCam::NSIoPipe::FDVTINPUTDEGREE  mFDDir;
    FDVTConfig mFdvtDrvResult;
    MINT32  mFLDNum;
    MINT32  mGenderNum;
    MINT32  mPoseNum;

    // 3A/ISP
    MINT32  mGammaCtrl[193]; // index0 = type, index1~192 : curve
    // async
    mutable Mutex       mDriverLock;
    mutable Mutex       mSyncLock;
    Condition           mFDCond;
    Condition           mAttrCond;
    MINT32              mAttrBusy;
    pthread_t           mAttThread;
    sem_t               mSemAttExec;
    MINT32              mAttStop;
    NSCam::NSIoPipe::FDVTINPUTDEGREE              mAttDir;
    // attribute
    mutable Mutex       mattribureMutex;
    short               mgender_FM[MAX_CROP_NUM][MAX_AIE_FMAP_SZ];
    short               mpose_FM[MAX_CROP_NUM][MAX_AIE_FMAP_SZ];
    unsigned char       mtodoList[MAX_AIE_ATTR_TYPE][MAX_CROP_NUM];
    unsigned char       mbufStatus[MAX_AIE_ATTR_TYPE][MAX_CROP_NUM];
    IImageBufferAllocator* mAllocator;
    sp<IImageBuffer>    mpAttrWorkBuf;
    unsigned char**     mworkbufList;
    MINTPTR             mworkbufListPA[MAX_CROP_NUM];
    int                 mpatchSize[MAX_CROP_NUM];
    int                 mAIEAttrTask[MAX_AIE_ATTR_TYPE];
    FDVTConfig          mAttDrvResult;
    // driver
    NSCam::NSIoPipe::NSEgn::IEgnStream<FDVTConfig>* mpFDStream;
    //secure
    bool                misSecureFD = false;


};

#endif

