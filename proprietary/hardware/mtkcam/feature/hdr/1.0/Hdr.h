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
#ifndef _HDR_H_
#define _HDR_H_

#include <utils/Vector.h>
#include <utils/StrongPointer.h>

#include <mtkcam/common.h>    //workaround for hwstddef.h
#include <mtkcam/common/hw/hwstddef.h> //workaround for _params.h
#include <mtkcam/drv_common/imem_drv.h>

#include <mtkcam/featureio/hdr_hal_base.h>
#include "camera_custom_hdr.h"    // For HDR Customer Parameters in Customer Folder.
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/hal/aaa_hal_base.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>

#include <pthread.h>
#include <semaphore.h>

#include <mtkcam/feature/hdr/1.0/Hdr_defs.h>

/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define JPG_SAVING_OPTIMIZE     1   // Save JPEG while HDR thread are doing things.
typedef MBOOL (*HDRProcCompleteCallback_t)(MVOID* user,MBOOL ret);

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/******************************************************************************
 *
 ******************************************************************************/
struct HDRProc_ShotParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Picture.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for captured pictures.
    int                             miPictureFormat;

    // The dimensions for captured pictures in pixels (width x height).
    int32_t                         mi4PictureWidth;
    int32_t                         mi4PictureHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Postview Image.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The image format for postview display.
    int                             miPostviewDisplayFormat;

    // The image format for postview client.
    int                             miPostviewClientFormat;

    // The dimensions for postview in pixels (width x height).
    int32_t                         mi4PostviewWidth;
    int32_t                         mi4PostviewHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The zoom ratio is in 1/100 increments. Ex: 320 refers to a zoom of 3.2x.
    //@deprecated; should be replaced w/ mScalerCropRegion
    uint32_t                        mu4ZoomRatio;

    // This control can be used to implement digital zoom
    MRect                           mScalerCropRegion;

    // The transform: includes rotation and flip
    // The rotation angle in degrees relative to the orientation of the camera.
    //
    // For example, suppose the natural orientation of the device is portrait.
    // The device is rotated 270 degrees clockwise, so the device orientation is
    // 270. Suppose a back-facing camera sensor is mounted in landscape and the
    // top side of the camera sensor is aligned with the right edge of the
    // display in natural orientation. So the camera orientation is 90. The
    // rotation should be set to 0 (270 + 90).
    //
    // Flip: horizontally/vertically
    // reference value: mtkcam/ImageFormat.h
    uint32_t                        mu4Transform;

#ifdef __cplusplus
    HDRProc_ShotParam()
    : miPictureFormat(NSCam::eImgFmt_UNKNOWN),
      mi4PictureWidth(0),
      mi4PictureHeight(0),
      miPostviewDisplayFormat(NSCam::eImgFmt_UNKNOWN),
      miPostviewClientFormat(NSCam::eImgFmt_UNKNOWN),
      mi4PostviewWidth(0),
      mi4PostviewHeight(0),
      mu4ZoomRatio(100),
      mScalerCropRegion(),
      mu4Transform(0)
    {}
#endif
};


/******************************************************************************
 *
 ******************************************************************************/
struct HDRProc_JpegParam
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Jpeg quality of captured picture. The range is 1 to 100, with 100 being
    // the best.
    uint32_t                        mu4JpegQuality;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Jpeg Thumb.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The quality of the EXIF thumbnail in Jpeg picture. The range is 1 to 100,
    // with 100 being the best.
    uint32_t                        mu4JpegThumbQuality;

    // The width (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbWidth;

    // The height (in pixels) of EXIF thumbnail in Jpeg picture.
    int32_t                         mi4JpegThumbHeight;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Stereo JPS
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // The width (in pixels) of EXIF thumbnail in Jps picture.
    int32_t                         mi4JpsWidth;

    // The height (in pixels) of EXIF thumbnail in Jps picture.
    int32_t                         mi4JpsHeight;

#ifdef __cplusplus
    HDRProc_JpegParam()
    : mu4JpegQuality(100),
      mu4JpegThumbQuality(100),
      mi4JpegThumbWidth(0),
      mi4JpegThumbHeight(0),
      mi4JpsWidth(0),
      mi4JpsHeight(0)
    {}
#endif
};



/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/
class CamExif;


namespace android {
class HdrShot
{
protected:  ////    Multi-Frame
    enum    { eMaxOutputFrameNum = 3 };

protected:  ////    Resolutions.
    MUINT32         mu4W_first;        //  YUV Width for first run     // always be 1600
    MUINT32         mu4H_first;        //  YUV Height for first run    // always be 1200
    MUINT32         mu4W_se;        //  SW EIS Image Width    // Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32         mu4H_se;        //  SW EIS Image Height    // Obtained in requestOtherBufs()\QuerySEImgResolution().
    MUINT32         mu4W_dsmap;        //  Down-sized Weighting Map Width    // Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_width.
    MUINT32         mu4H_dsmap;        //  Down-sized Weighting Map Height    // Obtained in requestDownSizedWeightMapBuf(). This should be after obtaining OriWeight[0]->weight_table_height.
    MUINT32         mPostviewWidth;
    MUINT32         mPostviewHeight;
    EImageFormat    mPostviewFormat;

public:
    MUINT32         mu4W_yuv;        //  YUV Width    // Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4H_yuv;        //  YUV Height    // Obtained in updateInfo()\queryIspYuvResolution().
    MUINT32         mu4W_small;        //  Small Image Width    // Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mu4H_small;        //  Small Image Height    // Obtained in requestOtherBufs()\QuerySmallImgResolution().
    MUINT32         mSensorCaptureWidth;   //  sensor full width
    MUINT32         mSensorCaptureHeight;  //  sensor full height
    MUINT32         mSensorType;        // sensor type raw / yuv
    MUINT32         mPhotoTransform;   //  jpeg rotation

    MINT32          mu4AOEMode;
    MUINT32         mu4MaxSensorAnalogGain; // 1x=1024
    MUINT32         mu4MaxAEExpTimeInUS;    // unit: us
    MUINT32         mu4MinAEExpTimeInUS;    // unit: us
    MUINT32         mu4ShutterLineTime;     // unit: 1/1000 us
    MUINT32         mu4MaxAESensorGain;     // 1x=1024
    MUINT32         mu4MinAESensorGain;     // 1x=1024
    MUINT32         mu4ExpTimeInUS0EV;      // unit: us
    MUINT32         mu4SensorGain0EV;       // 1x=1024
    MUINT8          mu1FlareOffset0EV;
    MINT32          mi4GainBase0EV;          // AOE application for LE calculation
    MINT32          mi4LE_LowAvg;            // AOE application for LE calculation, def: 0 ~ 39 avg
    MINT32          mi4SEDeltaEVx100;        // AOE application for SE calculation
    MUINT32         mu4Histogram[128];
    HDRProcCompleteCallback_t mCompleteCb;
    MVOID*          mpCbUser;

protected:  ////    Thread
    sem_t           mSaveNormalJpegDone;
    sem_t           mEncodeHdrThumbnailJpegDone;

    pthread_t       mSaveJpegThread;
    pthread_t       mCaptureIMemThread;
    pthread_t       mProcessIMemThread;

protected:  ////    Pipes.
    HdrHalBase      *mpHdrHal;
    CamExif         *mpCamExif[3];

protected:  ////    Buffers.
    IMemDrv *mpIMemDrv;
    MUINT32         mTotalBufferSize;
    MUINT32         mTotalKernelBufferSize;
    MUINT32         mTotalUserBufferSize;

    //@TODO use ImgBufInfo to replace IMEMINFO
    //IMEM_BUF_INFO   mpSourceImgBuf[eMaxOutputFrameNum];
    //MUINT32         mu4SourceSize;    // Source Image Size.

    IMEM_BUF_INFO   mpFirstRunSourceImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4FirstRunSourceSize;    // First Run Source Image Size.

    //IMEM_BUF_INFO   mpSmallImgBuf[eMaxOutputFrameNum];
    //MUINT32         mu4SmallImgSize;    // Small Image Size.

    IMEM_BUF_INFO   mpSEImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SEImgSize;    // SW EIS Image Size.

    IMEM_BUF_INFO   mWeightingBuf[eMaxOutputFrameNum];
    MUINT32         mWeightingBufSize;

    IMEM_BUF_INFO   mpBlurredWeightMapBuf[eMaxOutputFrameNum];
    MUINT32         muBlurredWeightMapSize;    // Blurred Weighting Map Size.

    IMEM_BUF_INFO   mpDownSizedWeightMapBuf[eMaxOutputFrameNum];
    MUINT32         mu4DownSizedWeightMapSize;    // Down-sized Weighting Map Size.

    IMEM_BUF_INFO   mpPostviewImgBuf;
    MUINT32         mu4PostviewImgSize;    // First Run HDR Result Image Size.

    IMEM_BUF_INFO   mpResultImgBuf;
    MUINT32         mu4ResultImgSize;    // HDR Result Image Size.

    IMEM_BUF_INFO   mpHdrWorkingBuf;
    MUINT32         mu4HdrWorkingBufSize;    // HDR Working Buf Size.

    IMEM_BUF_INFO   mpMavWorkingBuf;
    MUINT32         mu4MavWorkingBufSize;    // MAV Working Buf Size.

    IMEM_BUF_INFO   mRawBuf;
    MUINT32         mu4RawBufSize;    // Raw Image Size.

    IMEM_BUF_INFO   mNormalJpegBuf;
    MUINT32         mNormalJpegBufSize;

    IMEM_BUF_INFO   mNormalThumbnailJpegBuf;
    MUINT32         mNormalThumbnailJpegBufSize;

    IMEM_BUF_INFO   mHdrJpegBuf;
    MUINT32         mHdrJpegBufSize;

    IMEM_BUF_INFO   mHdrThumbnailJpegBuf;
    MUINT32         mHdrThumbnailJpegBufSize;

    // TODO: allocate the blending buffer as a type of image buffer to achive zero-copy
    IMEM_BUF_INFO   mBlendingBuf;
    MUINT32         mBlendingBufSize;

    HDR_PIPE_SET_BMAP_INFO mHdrSetBmapInfo;
    HDR_PIPE_WEIGHT_TBL_INFO** OriWeight;
    HDR_PIPE_WEIGHT_TBL_INFO** BlurredWeight;


protected:  ////    Parameters.
    static MUINT32  mu4RunningNumber;        // A serial number for file saving. For debug.

    MUINT32            mu4OutputFrameNum;        // Output frame number (2 or 3).    // Do not use mu4OutputFrameNum in code directly, use OutputFrameNumGet() instead.

    MUINT32            mu4FinalGainDiff[2];
    MUINT32            mu4TargetTone;


    HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;

    volatile MUINT32    mfgIsForceBreak;        // A flag to indicate whether a cancel capture signal is sent.

    HdrState_e        mHdrState;

    MUINT32         mHdrRound;
    MUINT32         mHdrRoundTotal;

    MBOOL           mShutterCBDone;
    MBOOL           mRawCBDone;
    MBOOL           mJpegCBDone;

    int             mCapturePolicy;
    int             mCapturePriority;

    MUINT32         mCaptueIndex;
    HDRProc_ShotParam       mShotParam;
    HDRProc_JpegParam       mJpegParam;

public:     ////    for development.
    MUINT32         mTestMode;
    MUINT32         mDebugMode;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//protected:  ////    JPEG
//    virtual MUINT32 getJpgEncInAddr() const { return mu4RawDecAddr; }
//    virtual MUINT32 getJpgEncInSize() const { return mu4RawDecSize; }

public:     ////    Attributes.
    inline MUINT32    OutputFrameNumGet() const { return /*eMaxOutputFrameNum*/ mu4OutputFrameNum ;}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//kidd: remove this block
public:     ////    Interfaces.
    virtual MBOOL   uninit();
    virtual MBOOL   updateInfo_cam3();
    virtual MBOOL   EVBracketCapture_cam3(void);
    virtual MBOOL   decideCaptureMode_cam3();
    virtual MBOOL   init_cam3();
    virtual MBOOL	createSourceAndSmallImg_cam3(void);
    virtual MBOOL	registerSrcSmallImg_cam3(MINT32 frame_index, const android::sp<IImageBuffer>  imgbuf);
    virtual MBOOL	registerOutputImg_cam3(MINT32 frame_index,sp<IImageBuffer> imgbuf);
    virtual MBOOL	process_cam3();
    virtual MBOOL   release_cam3();
    virtual MBOOL   ImageRegistratoin_cam3(void);
    virtual MBOOL   Blending_cam3(void);
    virtual MBOOL   writeHDROutBuf_cam3();
    static  MVOID*  hdrProcess_cam3(MVOID* arg);
    virtual MBOOL   getCaptureInfo_cam3(android::Vector<NS3A::CaptureParam_T> & vCap3AParam, MINT32 &hdrFrameNum);
    static  MVOID*  allocateCaptureMemoryTask_All_cam3(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_First_cam3(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_Others_cam3(MVOID* arg);
    virtual MBOOL   getCaptureExposureSettings_cam3(HDRExpSettingOutputParam_T &strHDROutputSetting);

    IMEM_BUF_INFO   mpSourceImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SourceSize;    // Source Image Size.

    IMEM_BUF_INFO   mpSmallImgBuf[eMaxOutputFrameNum];
    MUINT32         mu4SmallImgSize;    // Small Image Size.

    IImageBuffer        *mHdrOutBuf_cam3[2]; // JPEG_YUV and Thumbnail_YUV

public:     ////                    Instantiation.
    virtual         ~HdrShot();
                    HdrShot(char const*const pszShotName
                            , uint32_t const u4ShotMode
                            , int32_t const i4OpenId
                            );

public:     ////                    Operations.

    virtual bool    setShotParam(void const* pParam, size_t const size);
    virtual bool    setJpegParam(void const* pParam, size_t const size);

protected:
    //virtual MBOOL   handleBayerData(MUINT8* const puBuf, MUINT32 const u4Size);
    virtual MBOOL   handleYuvData(MUINT8* const puBuf, MUINT32 const u4Size);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Utilities.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Buffers.
    static  MVOID*  allocateCaptureMemoryTask_First(MVOID* arg);
    static  MVOID*  allocateCaptureMemoryTask_Others(MVOID* arg);
    static  MVOID*  allocateProcessMemoryTask(MVOID* arg);

    virtual MBOOL   requestSourceImgBuf(void);
    virtual MBOOL   releaseSourceImgBuf(void);
    virtual MBOOL   requestFirstRunSourceImgBuf(void);
    virtual MBOOL   releaseFirstRunSourceImgBuf(void);
    virtual MBOOL   requestSmallImgBuf(void);
    virtual MBOOL   releaseSmallImgBuf(void);
    virtual MBOOL   requestSEImgBuf(void);
    virtual MBOOL   releaseSEImgBuf(void);
    virtual MBOOL   requestHdrWorkingBuf(void);
    virtual MBOOL   releaseHdrWorkingBuf(void);
    virtual MBOOL   requestOriWeightMapBuf(void);
    virtual MBOOL   releaseOriWeightMapBuf(void);
    virtual MBOOL   requestBlurredWeightMapBuf(void);
    virtual MBOOL   releaseBlurredWeightMapBuf(void);
    virtual MBOOL   requestDownSizedWeightMapBuf(void);
    virtual MBOOL   releaseDownSizedWeightMapBuf(void);
    virtual MBOOL   releasePostviewImgBuf(void);

    virtual MBOOL   requestBlendingBuf(void);
    virtual MBOOL   releaseBlendingBuf(void);


//protected:  ////    CDP.
public:  ////    CDP.
    virtual MBOOL   CDPResize(IMEM_BUF_INFO* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat, IMEM_BUF_INFO* desAdr, MUINT32 desWidth, MUINT32 desHeight, EImageFormat dstFormat, MUINT32 rotate);
    static  MBOOL   CDPResize_simple(IMEM_BUF_INFO* srcAdr, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srcFormat, IMEM_BUF_INFO* desAdr, MUINT32 desWidth, MUINT32 desHeight, EImageFormat dstFormat, MUINT32 rotate);
    static  MBOOL   GetStride(MUINT32 srcWidth, EImageFormat srcFormat, MUINT32 *pStride);
    static  MUINT32 getAlignedSize(MUINT32 const u4Size);

protected:  ////    Save.
    virtual MBOOL   touchVirtualMemory(MUINT8* vm, MUINT32 size);
    static unsigned int    dumpToFile(char const *fname, unsigned char *pbuf, unsigned int size);
    virtual MUINT32 allocMem(IMEM_BUF_INFO *memBuf);
    virtual MUINT32 allocMem_User(IMEM_BUF_INFO *memBuf, MBOOL touch, MBOOL mapping);
    virtual MUINT32 allocMem_Kernel(IMEM_BUF_INFO *memBuf);
    virtual MBOOL   deallocMem(IMEM_BUF_INFO *memBuf);

protected:  ////    Misc.
    //flow
    virtual MBOOL    configureForSingleRun(void);
    virtual MBOOL    configureForFirstRun(void);
    virtual MBOOL    configureForSecondRun(void);

    virtual MBOOL    WeightingMapGeneration(void);

    //virtual MBOOL    createFullFrame(void);
    virtual MBOOL    createSmallImg(void);

    //virtual MBOOL    saveSmallImgForDebug(void);
    virtual MBOOL    createSEImg(void);

    //virtual MVOID*  createHdrJpegImgMain(MVOID* arg);
    static  MVOID*  saveFileTask(MVOID* arg);

    virtual MBOOL    do_Normalization(void);
    virtual MBOOL    do_SE(void);
    virtual MBOOL    do_FeatureExtraction(void);
    virtual MBOOL    do_Alignment(void);
    virtual MBOOL    do_OriWeightMapGet(void);
    virtual MBOOL   do_SetBmapBuffer(void);

    virtual MBOOL    do_DownScaleWeightMap(void);
    virtual MBOOL    do_UpScaleWeightMap(void);
    virtual MBOOL    do_Fusion(void);
    virtual MBOOL    do_HdrCroppedResultGet(void);
    virtual MBOOL    do_CroppedResultResize(void);   //@deprecated

    virtual MBOOL    do_HdrSettingClear(void);
    virtual MBOOL   do_SecondRound(void);
    inline  MBOOL   wait(pthread_mutex_t *mutex, const char *note);
    inline  MBOOL   announce(pthread_mutex_t *mutex, const char *note);

public:        ////    Thread.
    virtual HdrState_e    GetHdrState(void);
    virtual void    SetHdrState(HdrState_e eHdrState);
    virtual MINT32    mHalCamHdrProc(HdrState_e eHdrState);
    static  MBOOL   SetThreadProp(int policy, int priority);
    static  MBOOL   GetThreadProp(int *policy, int *priority);
    pthread_t           mHDRProcessThread_cam3;
    pthread_mutex_t     mMemoryReady_SrcLargeImg_cam3[eMaxOutputFrameNum];
    pthread_mutex_t     mMemoryReady_SrcSmallImg_cam3[eMaxOutputFrameNum];
    pthread_mutex_t     mMemoryReady_DstImg_cam3[2]; // only support jpg and i420 two buffer
};


}; // namespace android

// Interface for HDR Proc
MBOOL  HDRProcInit();
MBOOL  HDRProcUnInit();
MBOOL HDRProcSetShotParam(void * pParam);
MBOOL HDRProcSetJpegParam(void * pParam);
MBOOL HDRProcPrepare();
MBOOL HDRProcAddOutFrame(MINT32 frame_index,android::sp<IImageBuffer> imgbuf);
MBOOL HDRProcAddInputFrame(MINT32 frame_index, const android::sp<IImageBuffer> imgbuf);
MBOOL HDRProcStart();
MBOOL HDRProcRelease();
MBOOL HDRProcGetBuf(IImageBuffer** src0, IImageBuffer** small0, IImageBuffer** src1, IImageBuffer** small1 );
MBOOL HDRProcGetCapInfo(android::Vector<NS3A::CaptureParam_T> & vCap3AParam, MINT32 &hdrFrameNum);
MBOOL HDRProcSetParam(MUINT32 paramId, MUINTPTR iArg1, MUINTPTR iArg2);
MBOOL HDRProcGetParam(MUINT32 paramId, MUINT32 & rArg1, MUINT32 & rArg2);
MVOID HDRProcSetCallBack(HDRProcCompleteCallback_t complete_cb, MVOID* user);

#endif  //  _HDR_H_

