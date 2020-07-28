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
#include "MTKDetection.h"

#define FD_SCALES 14

#if USE_DL_FD || USE_PORTRAIT
//squeezenent
#if USE_PORTRAIT
#include <portrait/portrait.h>
#endif
#if USE_DL_FD
#include <fd/sqzrpnfd.h>
#endif
//#include "ui/GraphicBuffer.h"
//#include <ui/gralloc_extra.h>

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

typedef struct CNN_THREAD_PARA
{
    MINT32 InWidth;
    MINT32 InHeight;
    MINT32 CNNFDFilterTH;
    MINT32 CNNFDOverlapTH;
    void* pFDBufVirtual;
    void* pCal_Data;

}CNN_THREAD_PARA, *P_CNN_THREAD_PARA;

#endif

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
    virtual MINT32 halFDInit(MUINT32 fdW, MUINT32 fdH, MUINT8 *WorkingBuffer, MUINT32 WorkingBufferSize, MBOOL SWResizerEnable, MUINT8 Current_mode, MINT32 FldNum = 1);

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
    // mHalSDGetSmileResult () -
    //! \brief get smile detection result
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halSDGetSmileResult( ) ;

    /////////////////////////////////////////////////////////////////////////
    //
    // halFDYUYV2ExtractY () -
    //! \brief create Y Channel
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFDYUYV2ExtractY(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT32 src_width, MUINT32 src_height);

    /////////////////////////////////////////////////////////////////////////
    //
    // halFTBufferCreate () -
    //! \brief create face tracking buffer
    //
    /////////////////////////////////////////////////////////////////////////
    virtual MINT32 halFTBufferCreate(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT8  ucPlane, MUINT32 src_width, MUINT32 src_height);

    virtual MINT32 halGetFTBufferSize(MUINT32 *width, MUINT32 *height);

    virtual MINT32 halFTBufferCreateAsync(MUINT8 *dstAddr, MUINT8 *srcAddr, MUINT8  ucPlane, MUINT32 src_width, MUINT32 src_height);

private:
    static void* onFTThreadLoop(void*);
    static void LockFTBuffer(void* arg);
    static void UnlockFTBuffer(void* arg);

    bool doRGB565BufferP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth, MUINT32 SrcHeight);
    bool doRGB565BufferQVGAP1_SW(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 SrcWidth1, MUINT32 SrcHeight1);
    bool doRGB565Buffer_DDP(MUINT8 *a_dstAddr, MUINT8 *a_srcAddr, MUINT32 planes);
    bool doHWFaceDetection(void *pCal_Data);

    void dumpFDImages(MUINT8* pFDImgBuffer, MUINT8* pFTImgBuffer);
    void dumpFDParam(MTKFDFTInitInfo &FDFTInitInfo);

    #if USE_DL_FD
    MINT32 DoDLFD(CNN_THREAD_PARA *pInput);
    #endif

    #if USE_PORTRAIT
    static void* onPortThreadLoop(void*);
    static void RunPortrait(void* arg);
    static void WaitPortrait(void* arg);
    static void ReleasePortrait(void* arg);
    MINT32 DoDLPortrait(CNN_THREAD_PARA *pInput);
    #endif

    static void* onInitThreadFunc(void* arg);

protected:

    MTKDetection* mpMTKFDVTObj;

    MUINT32 mFDW;
    MUINT32 mFDH;
    MUINT32 mBuffCount;
    MBOOL   mInited;

    pthread_t   mFTThread;
    sem_t       mSemFTExec;
    sem_t       mSemFTLock;
    MINT32      mFTStop;
    MINT32      mFTBufReady;
    struct FTParam {
        MUINT8 *dstAddr;
        MUINT8 *srcAddr;
        MUINT8 ucPlane;
        MUINT32 src_width;
        MUINT32 src_height;
    };
    struct FTParam mFTParameter;

    MUINT32 mimage_width_array[FD_SCALES];
    MUINT32 mimage_height_array[FD_SCALES];
    MUINT32 mImageScaleTotalSize;
    MUINT8  *mpImageScaleBuffer;
    MBOOL   mEnableSWResizerFlag;
    MUINT8  *mpImageVGABuffer;
    MINT32  mFdResult_Num;
    FD_RESULT mFdResult[15];
    MUINT32 mFTWidth;
    MUINT32 mFTHeight;
    MBOOL   mCurrentMode;
    MUINT32 mDoDumpImage;
    float   mFDFilterRatio;

    MUINT32 mUseUserScale;
    MUINT32 mUserScaleNum;
    MUINT32 mFrameCount;
    MUINT32 mDetectedFaceNum;
    MUINT32 mFDRefresh;
    MINT32  mSkipPartialFD; //only skip detect part
    MINT32  mSkipAllFD; // skip detect part and post-processing part

    #if USE_DL_FD
    sp<mtkdl::SqzRpnFdBase> mpSqzRpnFd;
    vector<mtkdl::FACE_BBOX> mrNmsBBoxes;
    CNN_THREAD_PARA mCNNPara;
    #endif

    #if USE_PORTRAIT
    //static sp<mtkdl::PortraitNet> mpPortrait;
    CNN_THREAD_PARA mPortraitPara;
    vector<float> mvCNNout;
    MINT32      mNeedRunPort;

    MINT32      mPortStop;
    pthread_t   mPortThread;
    pthread_t   mInitThread;
    //static MINT32      mFitstPortInit;
    //static MINT32      mPortInited;
    sem_t       mSemPortExec;
    sem_t       mSemPortLock;
    #endif
};

#endif

