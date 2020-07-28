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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
/**
 *  Multi Frame Noise Reduction Version Information
 *
 *  Versions Included:
 *  1.0.0   - First version release.
 *  1.1.0   - Supported ZSD.
 *  1.2.0   - Supported Memory Reduce Plan.
 *  1.2.1   - Fixed bug: Apply MRP with MFNR v1.1.x may have quality issue.
 *  1.3.0   - Supported RWB sensor.
 *  1.4.0   - Support parallel ME/MC, this feature needs new algorithm support,
 *            MEMC_FLOW_V2 should be defined.
 *            (See also include/mtkcam/core/featureio/pipe/mfll/mfll_hal.h)
 *
 *  1.5.0   - Supported ZSD Dual Phase Mechanism
 *  1.5.1   - Full support ZSD Dual Phase, much better performance
 */
#ifndef _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MFLLSHOT_H_
#define _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MFLLSHOT_H_

#include <mtkcam/camnode/ShotCallbackNode.h>
namespace NSCamNode{
    class AllocBufHandler;
    class ICamGraph;
    class ICamNode;
    class Pass1Node;
    class SShotCtrlNode;
    class ShotCallbackNode;
    class Pass2Node;
    class Pass2Node;
    class JpegEncNode;
    class JpegEncNode;
};

#include <semaphore.h>
#include <utils/Mutex.h>
#include <utils/RefBase.h>
#include <mtkcam/featureio/eis_type.h>
#include "MfllCommon.h" //@todo remove this

#include <debug_exif/cam/dbg_cam_param.h>
#include <camera_custom_nvram.h>

#include "MfllDebugExif.h"
// CamExif usage
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
#include <mtkcam/exif/IDbgInfoContainer.h>

using namespace std;
using namespace android;
using namespace NSCamNode;

/* using std::vector as vector */
#include <vector>
#include <functional>

using std::vector;

/*******************************************************************************
*
********************************************************************************/
class MfbllHal;
class IDbgInfoContainer;

namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

class CamShotImp;
class IBurstShot;
class CapBufMgr;


enum {MFB_MODE_OFF
    , MFB_MODE_MFLL
    , MFB_MODE_AIS
    , MFB_MODE_MFLL_FOR_ENGMODE
    , MFB_MODE_LENGTH
    };

enum MFB_REGTABLE {MF_REGTABLE_BASE
                    , MF_REGTABLE_BEFORE_MFB
                    , MF_REGTABLE_IN_MFB
                    , MF_REGTABLE_AFTER_MFB
                    , MF_MAX_REGTABLE
};

enum {MFB_MAX_REG_SIZE = 512};
enum {MFB_MAX_DUMP_FILENAME_LENGTH = 128};

//
struct DEBUG_MF_MFLL_INFO {
    //
    MUINT32 mfbMode;
    MUINT32 maxFrameNumber;
    MUINT32 processingNumber;
    MUINT32 exposure;
    MUINT32 iso;
    MUINT32 iso_hi_th;
    MUINT32 iso_low_th;
    MUINT32 rawWidth;
    MUINT32 rawHeight;

    //
    struct BSS {
        MUINT32 enable;
        MUINT32 roiWidth;
        MUINT32 roiHeight;
        MUINT32 scaleFactor;
        MUINT32 clipTh0;
        MUINT32 clipTh1;
        MUINT32 inputSequenceAfterBss[MAX_REFERENCE_FRAMES+1];
    } bss;

    //
    struct MEMC {
        MUINT32 width;
        MUINT32 height;
        MUINT32 inputFormat;
        MUINT32 outputFormat;
        MUINT32 skip;
        MUINT32 badMvRange;
        MUINT32 badMbCount[MAX_REFERENCE_FRAMES];
        MUINT32 badMbTh;
    } memc;

    MUINT32 reg_mfb[5/*mfb*/];
    MUINT32 reg_mixer3[5/*mfb*/+3/*mixer3*/];
    MUINT32 reg[MF_MAX_REGTABLE][49/*anr*/ + 24/*ee*/];
};

//
struct DEBUG_MF_AIS_INFO {
    MUINT32 exposure;
    MUINT32 iso;
    MUINT32 calculatedExposure;
    MUINT32 calculatedIso;
    MUINT32 exposure_th0;
    MUINT32 iso_th0;
    MUINT32 exposure_th1;

    struct AndvandedControlParameters {
        MUINT32 enable;
        MUINT32 maxExposureGain;
        MUINT32 maxIsoGain;
    } advanced;
};

//
struct DEBUG_MF_EIS_INFO {
    MINT32  gmvX;
    MINT32  gmvY;
    MINT32  mvX[EIS_MAX_WIN_NUM];
    MINT32  mvY[EIS_MAX_WIN_NUM];
    MINT32  trustX[EIS_MAX_WIN_NUM];
    MINT32  trustY[EIS_MAX_WIN_NUM];
};

/*******************************************************************************
*
********************************************************************************/
class MfllShot : public CamShotImp
{
public:
    enum eMrpMode {
        eMrpMode_None = 0, // Not apply MRP
        eMrpMode_LowestMemory = 1, // Release buffers ASAP but encode only Base YUV and keeps Base RAW for late encode Golden YUV
        eMrpMode_Balance = 2, // Release buffers ASAP but encode both Base&Golden YUV
    };

    /* RWB sensor support enumeration */
    enum eRwbSupport {
        eRwbSupport_None = 0, // Not support
        eRwbSupport_GPU  = 1, // Support GPU mode
        eRwbSupport_MDP  = 2, // Support MDP mode
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CamShotImp
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Constructor/Destructor.
                    MfllShot(EShotMode const eShotMode, char const*const szCamShotName, MUINT32 mfb);

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam);
    virtual MBOOL   startOne(SensorParam const  & rSensorParam);
    virtual MBOOL   startOne(SensorParam const  & rSensorParam, IImageBuffer const *pImgBuffer);
    virtual MBOOL   startOne(SensorParam const & rSensorParam,
                            IImageBuffer const *pImgBuf_0, IImageBuffer const *pImgBuf_1,
                            IImageBuffer const *pBufPrv_0=NULL, IImageBuffer const *pBufPrv_1=NULL) { return MTRUE; }
    virtual MBOOL   stop();

public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);
    virtual MBOOL   setDump(MUINT32 const & rParam);
    virtual MBOOL   setDumpFilePathAndName(const char * const pFilePathAndName);


public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer);

public:     ////    Info.
    virtual MBOOL   setPrvBufHdl(MVOID* pCamBufHdl);
    virtual MBOOL   setCapBufMgr(MVOID* pCapBufMgr);

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  MFBLL.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //tuning
    static  MBOOL   getCaptureInfo(MUINT32 mfbMode, MUINT32 sensorId, MUINT32 &frameNumber, MUINT32 &exp, MUINT32 &iso,MBOOL &needUpdateAE);

protected:  ////                    Thread
    static MVOID*   memoryProc(void *arg);
    static MVOID*   captureProc(void *arg);
    static MVOID*   capturePostProc(void *arg);
    static MVOID*   rawToYuy2Proc(void *arg);
    static MVOID*   yuy2ToYv16Proc(void *arg);
    static MVOID*   mfllProc(void *arg);
    static MVOID*   memcProc(void *arg);
    static MVOID*   parallelMemcProc(void *arg); // *arg is combinated by &this, and an integer
    static MVOID*   blendingProc(void *arg);
    static MVOID*   postviewProc(void *arg);
    static MVOID*   fullProc(void *arg);
    static MVOID*   thumbnailProc(void *arg);
    static MVOID*   rwbMemoryProc(void *arg); // only needed while using GPU mode

protected:  ////                    flow
    MBOOL           flowMemory();
    MBOOL           flowCapture();
    MBOOL           flowCapturePostProc();
    MBOOL           flowRawToYuy2();
    MBOOL           flowYuy2ToYv16();
    MBOOL           flowMfll();
    MBOOL           flowMemc();
    MBOOL           flowParallelMemc(unsigned ref); // for parallel MEMC
    MBOOL           flowBlending();
    MBOOL           flowPostview();
    MBOOL           flowFull();
    MBOOL           flowThumbnail();

    /**
     * To create an image buffer for GPU if using GPU mode RWB sesnor support.
     * The thread of memory allocation will be triggered just before Mixing
     * operation because we can use this free time (Mixing uses HW computing)
     * to allocate memory.
     */
    MBOOL           flowRwbMemory(void);

protected:  ////                    utilites
    //misc
    void            readProperty(void);

    /**
     *  MFLL supports dual phase, invoke phase1Done to tell MW it's available
     *  to capture the next round. And invoke phase2Done to unlock mutex of
     *  phase1Done.
     *
     *  If using dual phase, MfllShot has responsibility to manager lifetime of
     *  MfllShot instances itself. In other words, we have to release all resource
     *  and delete instance in the last operation. To call selfRelease to release
     *  MfllShot, and it works if only if ZSD + dual phase enabled. Futhermore,
     *  to avoid modification of NormalShot, while using ZSD + dual phase, MfllShot
     *  also handle jpeg callback.
     */
    void            phase1Done(void);
    void            phase2Done(void);
    void            selfRelease(void);
    void            dualphaseHandleJpeg(void);

public:
    MBOOL           isUsingDualPhase(void);

protected:

    //flow
    inline  MBOOL   wait(pthread_mutex_t *mutex, char *note);
    inline  MBOOL   announce(pthread_mutex_t *mutex, char *note);

    //memory
    MBOOL           allocateMemory();
    MBOOL           releaseMemory();
    MBOOL           zsdReleaseBuffer(int index); // To return ZSD buffer.

    /**
     *  Memory Reduction Plan(MRP) usage
     *
     *  In MRP, we release image buffers immediately for reducing memory usage
     *  peak. If mReduceMemoryUsage is false, these functions return MFALSE
     *  immediately to avoid slow down MFLL performance.
     */
    /* to init MRP usage works */
    void            mrpInit(void);
    /* to release except base raw buffers memory (invoked if only if using Memory Reduction Plan */
    MBOOL           mrpReleaseRefRawBuffers(void);
    /* to release QYUV buffers (invoked if only if using Memory Reduction Plan */
    MBOOL           mrpReleaseQYuvBuffer(unsigned int index);
    /* to release ME/MC working buffer if possible */
    MBOOL           mrpReleaseMemcWorkingBuffer(unsigned int index);
    /* to release Mfb related buffers if possible */
    MBOOL           mrpReleaseMfbRelatedBuffers(void);
    /* to release Mixing related buffers if possible */
    MBOOL           mrpReleaseMixingRelatedBuffers(void);
    /* buffer lifetime control functions */
    MBOOL           mrpAllocateMemcBuffer(void);
    MBOOL           mrpAllocateYuvBaseBuffer(void);
    MBOOL           mrpAllocateMfbRelatedBuffers(void);
    MBOOL           mrpAllocateYuvMixingBuffer(void);
    MBOOL           mrpAllocateJpegBuffer(void);

    //capture
    virtual MBOOL   initInStartOne(SensorParam const  & rSensorParam);
    static MBOOL    captureDoneCallback(MVOID *user);   //@todo nobody use this
    static MBOOL    fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg);
    virtual MBOOL   fgCamShotNotifyCbImp(NSCamShot::CamShotNotifyInfo const& msg);
    static MBOOL    fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo  const msg);
    virtual MBOOL   fgCamShotDataCbImp(NSCamShot::CamShotDataInfo  const& msg);
    static MBOOL    fgCamShotDataCbFor15Raw(MVOID* user, NSCamShot::CamShotDataInfo  const msg);
    virtual MBOOL   fgCamShotDataCbImpFor15Raw(NSCamShot::CamShotDataInfo  const& msg);
    MBOOL           doBss();
    MBOOL           doCapture15Raw();

    //blending
    MBOOL           doEis(MUINT32 ref, MINT32 &gmvX, MINT32 &gmvY);
    MBOOL           doMemc(MUINT32 ref, MfbllHal *pMfb = NULL);
    MBOOL           doMfb(MUINT32 ref);
    MBOOL           doEncodeNormalYuvB();
    MBOOL           doEncodeYuvB();
    MBOOL           encodeRawToYuv(sp<IImageBuffer> &rawBuffer
                                    , sp<IImageBuffer> &yuvBuffer
                                    , NS3A::EIspProfile_T profile
                                    , void *privateData
                                    , MfllDebugExif::STAGE stage = MfllDebugExif::STAGE_UNDEFINED
                                    );
    MBOOL           doMixing();
    MBOOL           doSingleFrameOutput();  //another path of doMixing() when mSkipMfb == 1


    //postview
    MBOOL           doPostview();
    MBOOL           convertImage(sp<IImageBuffer> &pSrcImgBufInfo
                    			, sp<IImageBuffer> &pJpgImgBufInfo
    			                , MUINT32 const u4Transform = 0
    			                , MBOOL bIsCrop = MFALSE
    			                , MRect cropRect = {MPoint(0,0),MSize(0,0)}
    			                );


    //jpeg
    MBOOL           doFullJpeg();
    MBOOL           doThumbnailJpeg();
    MBOOL           encodeJpeg(sp<IImageBuffer> &pSrcImgBufInfo
                    			, NSCamShot::JpegParam const & rJpgParm
                    			, MUINT32 const u4Transform
                    			, sp<IImageBuffer> &pJpgImgBufInfo
                    			, MUINT32 & u4JpegSize);

                    /* to create an IImageBuffer by Gralloc */
    MBOOL           doRwbMemory(void);

                    /**
                     *  RWB color correction will be executed sequentially
                     *  after Mixing. Hence there's no need to create a new
                     *  thread here.
                     */
    MBOOL           doRwbColorCorrection(void);
protected:
    //tuning
    static  MBOOL   readTuningData(NVRAM_CAMERA_FEATURE_MFLL_STRUCT &tuningData, MUINT32 sensorId);
    static  MBOOL   updateAE(MUINT32 sensorId, MUINT32 exp, MUINT32 iso);
    static  MBOOL   querySensorInfo(MUINT32 sensorId, NSCam::SensorStaticInfo &sensorInfo);

protected:
    //debug
    MBOOL updateDumpFilename();

protected:
    //exif
    MBOOL setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr);
    MBOOL dumpDebugInfoAsTextFile(const char * const filename) const;
    MINT32 Complement2(MUINT32 value, MUINT32 digit) const;
    MBOOL dumpRegAsTextFile(const char * const filename) const;

protected:
    //handle callback
    MBOOL handleRawCb(void); // Invoke this method to feedback RAW callback if user asked
    MBOOL handleDngInfoCb(void); // Invoke this method to feedback DNG info callback if user asked

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:  ////    Mode
    MUINT32         mMfbMode;

private:  ////
    MBOOL           mbDoShutterCb;
    ICamBufHandler* mpPrvBufHandler;
    CapBufMgr*      mpCapBufMgr;

private:  ////    rotate
    MUINT32         mRotPicWidth;
    MUINT32         mRotPicHeight;
    MUINT32         mRotThuWidth;
    MUINT32         mRotThuHeight;

private:  ////    parameters
    SensorParam     mSensorParam;
    ShotParam       mShotParam;
    JpegParam       mJpegParam;

private:  ////    parameters
    MUINT32         mRegistedBufferType;

private:  ////    Hal.
    /* Multiple instances for parallel ME/MC, if not defined MEMC_FLOW_V2, use mpMfllHal[0] only */
    MfbllHal        *mpMfllHal[MAX_REFERENCE_FRAMES];
    IBurstShot      *mpBurstShot;

private:  ////    Parameters.
	static MUINT32  mu4RunningNumber;		// A serial number for file saving. For debug.

	MUINT32			mfgIsForceBreak;		// A flag to indicate whether a cancel capture signal is sent.

    MBOOL           mShutterCBDone;
    MBOOL           mRawCBDone;
    MBOOL           mJpegCBDone;

    MUINT32         mPass1Index;
    MUINT32         mPass2Index;
    MUINT32         mEisId;

private:  ////    buffers
    IMemDrv             *mpIMemDrv;
    MUINT32             mTotalBufferSize;

    MUINT32             mRaw_Width;
    MUINT32             mRaw_Height;
    IImageBuffer *      mMemoryCapBufRaw[MAX_REFERENCE_FRAMES+1];
    sp<IImageBuffer>    mMemoryRaw[MAX_REFERENCE_FRAMES+1];
    sp<IImageBuffer>    mMemoryRawInCaptureOrder[MAX_REFERENCE_FRAMES+1];
    sp<IImageBuffer>    mMemoryRawInCaptureOrderFor15Raw[15];
    sp<IImageBuffer>    mMemoryRawB;
    sp<IImageBuffer>    mMemoryRawR[MAX_REFERENCE_FRAMES];

    MUINT32             mYuv_Width;
    MUINT32             mYuv_Height;
    sp<IImageBuffer>    mMemoryYuvB;
    sp<IImageBuffer>    mMemoryYuvGolden; // saving golden YUV for mixing

    MUINT32             mYuvQ_Width;
    MUINT32             mYuvQ_Height;
    sp<IImageBuffer>    mMemoryQYuvB;
    sp<IImageBuffer>    mMemoryQYuvR[MAX_REFERENCE_FRAMES];

    /* If not defined using parallel MEMC, only use mMemoryLocalMotion[0] */
    sp<IImageBuffer>    mMemoryLocalMotion[MAX_REFERENCE_FRAMES];
    sp<IImageBuffer>    mMemoryYuv; // YUV working buffer
    sp<IImageBuffer>    mMemoryWeightingMap[2];
    MUINT32             mMemoryFinalWeightingMapIndex;


    sp<IImageBuffer>    mMemoryYuvMixing;

    sp<IImageBuffer>    mMemoryJpeg;
    MUINT32             mFullJpegRealSize;

    MUINT32             mThunmbnail_Width;
    MUINT32             mThunmbnail_Height;
    sp<IImageBuffer>    mMemoryJpeg_Thumbnail;
    MUINT32             mThumbnailJpegRealSize;

    MUINT32             mPostview_Width;
    MUINT32             mPostview_Height;
    sp<IImageBuffer>    mMemoryPostview;

    sp<IImageBuffer>    mMemoryRwbImgBuffer;

    /* Image buffer pointers, we don't allocate memory to it */
    sp<IImageBuffer>    mPtrYuvBase; // represents base frame
    sp<IImageBuffer>    mPtrYuvBlended; // represents blended frame
    sp<IImageBuffer>    mPtrYuvGolden; // represents golden frame

private:  ////    private data
    MVOID           *mPrivateData;
    MUINT32         mPrivateDataSize;

private:  ////    thread
    pthread_t       mThreadMemory;
    pthread_t       mThreadCapture;
    pthread_t       mThreadCapturePostProc;
    pthread_t       mThreadRawToYuy2;
    pthread_t       mThreadYuy2ToYv16;
    pthread_t       mThreadMfll;
    pthread_t       mThreadMemc;
    pthread_t       mThreadParallelMemc[MAX_REFERENCE_FRAMES];
    pthread_t       mThreadBlending;
    pthread_t       mThreadPostview;
    pthread_t       mThreadFull;
    pthread_t       mThreadThumbnail;
    pthread_t       mThreadRwbMemory;

private:  ////    mutex
    // memory
    pthread_mutex_t mTriggerAllocateMemory;
    pthread_mutex_t mMemoryReady_Pass1;
    pthread_mutex_t mMemoryReady_QYuvB;
    pthread_mutex_t mMemoryReady_QYuvR[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mMemoryReady_CapBuf[MAX_REFERENCE_FRAMES+1];
    pthread_mutex_t mMemoryReady_YuvB;
    pthread_mutex_t mMemoryReady_Memc[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mMemoryReady_Mfb[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mMemoryReady_Postview;
    pthread_mutex_t mMemoryReady_Jpeg;
    pthread_mutex_t mTriggerReleaseMemory;
    pthread_mutex_t mReleaseMemoryDone;
    // capture
    pthread_mutex_t mTriggerCapture;
    pthread_mutex_t mCapP2Ready[MAX_REFERENCE_FRAMES+1];
    pthread_mutex_t mYuy2ToYv16Done;
    pthread_mutex_t mQrReady[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mCaptureDone;
    pthread_mutex_t mGraphDone; //internal
    // memc
    pthread_mutex_t mTriggerMemc[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mMemcDone[MAX_REFERENCE_FRAMES];
    // blending
    pthread_mutex_t mTriggerBlending[MAX_REFERENCE_FRAMES];
    pthread_mutex_t mBlendingDone[MAX_REFERENCE_FRAMES];
    // postview
    pthread_mutex_t mTriggerPostview;
    pthread_mutex_t mPostviewDone;
    // jpeg
    pthread_mutex_t mTriggerFullJpeg;
    pthread_mutex_t mFullJpegDone;
    // thumbnail
    pthread_mutex_t mTriggerThumbnailJpeg;
    pthread_mutex_t mThumbnailJpegDone;
    // MRP usage Mutex
    /* These mutexes are for image buffers lifetime control */
    pthread_mutex_t mTriggerAllocateYuvBase;
    pthread_mutex_t mTriggerAllocateMfbRelatedBuffers;
    pthread_mutex_t mTriggerAllocateMemc;
    pthread_mutex_t mTriggerAllocateYuvMixing;
    pthread_mutex_t mTriggerAllocateJpeg;
    pthread_mutex_t mAllocateYuvBaseDone;
    pthread_mutex_t mAllocateMfbRelatedBuffersDone;
    pthread_mutex_t mAllocateMemcDone;
    pthread_mutex_t mAllocateYuvMixingDone;
    pthread_mutex_t mAllocateJpegDone;

    /* RWB sensor support (only GPU mode needs) */
    pthread_mutex_t mTriggerRwbMemory;
    pthread_mutex_t mRwbMemoryDone;

    /* Parallel ME usage */
    pthread_mutex_t mTriggerParallelMemc[MAX_REFERENCE_FRAMES];
private:  //tuning
    MBOOL           mSingleFrame;
    MUINT32         mReferenceFrameLength;
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT mTuningData;

private:  //debug
    MBOOL           mIsSetDumpFileName;
    MUINT32         mDump;
    char            mDumpFilename[MFB_MAX_DUMP_FILENAME_LENGTH];
    MUINT32         mSdcardIndex;

private:  //eis
    EIS_STATISTIC_STRUCT mEisStat[MAX_REFERENCE_FRAMES];
    MINT32  mGmv[MAX_REFERENCE_FRAMES][2];
    MFLOAT  mAccGmvX;
    MFLOAT  mAccGmvY;
    MFLOAT  mRawToQyuvFactor;

private:  //exif
    DEBUG_MF_MFLL_INFO mDbgMfllInfo;
    DEBUG_MF_AIS_INFO mDbgAisInfo;
    DEBUG_MF_EIS_INFO mDbgEisInfo[MAX_REFERENCE_FRAMES];
    MfllDebugExif     mDebugExif;

private:    //nr
    MUINT32         muNRType;

private:    //skip mfb
    MBOOL           mSkipMfb;

private:    //bss
    MBOOL           mEnableBss;

private:    //enable performance service or not
    MBOOL           mPerfService;

private:
    volatile enum eRwbSupport mUsingRwbSensor;
    volatile enum eMrpMode    mReduceMemoryUsage;
    volatile bool   mUsingParallelMemc;
    volatile int    mDualPhase;

private:
    CamExif         mCamExif; // saves 3A Information of exif for ZSD mode

public:
    /**
     *  Parallel MEMC is only supported when using MEMC_FLOW_V2 defined,
     *  if MEMC_FLOW_V2 is not defined, this function returns MFALSE.
     */
    MBOOL setUsingParallelMemc(MBOOL bEnable);

public:
    /**
     *  To dump processed RAW as unpacked RAW image
     *  @param index            Index of the RAW, index will be displayed in
     *                          file name.
     *  @param pBuffer          Buffer to dump
     */
    void dumpUnpackedRaw(int index, IImageBuffer *pBuffer);

protected:
    typedef std::function<void(NS3A::IHal3A*)>                  IHal3A_Destructor_T;
    typedef std::unique_ptr<NS3A::IHal3A, IHal3A_Destructor_T>  IHal3A_unique_ptr_T;

    IHal3A_Destructor_T mIHal3aDestructor;

};	// MfllShot

};  // NSCamShot
#endif  //  _MTK_CAMERA_CORE_CAMSHOT_INC_CAMSHOT_MFLLSHOT_H_

