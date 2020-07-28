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
#define LOG_TAG "RwbGpuProc"
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/common.h>
//
#include <list>
#include <queue>
using namespace std;
//
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/featureio/rwbGpuProc.h>
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <semaphore.h>
//
#include <mtkcam/common.h>
#include <mtkcam/featureio/aaa_hal_common.h>
//
#include <mtkcam/algorithm/librwb/MTKRWB.h>
#include <mtkcam/algorithm/librwb/MTKRWBErrCode.h>
//
#include <sys/prctl.h>
#include <utils/Mutex.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <vndk/hardware_buffer.h>
#include <ui/gralloc_extra.h>


/******************************************************************************
*
*******************************************************************************/

class RwbGpuProcImp : public Thread
                    , public RwbGpuProc
{
    public:

    struct Command
    {
         //  Command ID.
         enum EID
         {
             eCMD_INIT,
             eCMD_PROCESS,
             eCMD_UNINIT,
             eCMD_EXIT,
         };
         //
         //  Operations.
         Command(EID const _eId = eCMD_EXIT)
             : eId(_eId)
         {}
         //
         static  char const* getName(EID const _eId);
         inline  char const* name() const    { return getName(eId); }
         //
         //  Data Members.
         EID     eId;
    };

    protected:
        RwbGpuProcImp(MUINT32 sensorIdx);
        virtual ~RwbGpuProcImp();
    //
    public:
        static sp<RwbGpuProc> getInstance(MUINT32 sensorIdx);
        virtual MVOID   destroyInstance();
        virtual MUINT32 getSensorIdx() const    { return mSensorIdx; }

    private:
        //
        volatile MINT32     mUser;
        MUINT32             mSensorIdx;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Command Queue.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  ////                    Operations.

    virtual void                    postCommand(Command const& rCmd, MBOOL isWaitCondEn = MFALSE);

protected:

    virtual bool                    getCommand(Command& rCmd);
    //

protected:  ////                    Data Members.
    List<Command>                   mCmdQue;
    Mutex                           mCmdQueMtx;
    Condition                       mCmdQueCond;    //  Condition to wait: [ ! exitPending() && mCmdQue.empty() ]
    //

private:
    sem_t semGPUProcDone;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  .////                    Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

        virtual bool                   setSrcDstBuf(IImageBuffer* srcGpBuffer, IImageBuffer* dstGpBuffer);
        virtual void                   doGPUProc();

protected:
        virtual bool                   init();
        virtual void                   uninit();

        virtual bool                   doRWBInit();
        virtual bool                   doRWBUninit();
        virtual bool                   doMTKRWB();

private:

        MTKRWB*         mpRWB;
        RWBImageInfo    mRWB_info;
        GpuTuning       mGpu_tuning;
        //RWBTuning       rwb_tuning;
        //
        IImageBuffer const * mpSrcGpBuf;
        IImageBuffer const * mpDstGpBuf;

};

/******************************************************************************
*
*******************************************************************************/
char const*
RwbGpuProcImp::
Command::
getName(EID const _eId)
{
#define CMD_NAME(x) case x: return #x
    switch  (_eId)
    {
    CMD_NAME(eCMD_INIT);
    CMD_NAME(eCMD_PROCESS);
    CMD_NAME(eCMD_UNINIT);
    CMD_NAME(eCMD_EXIT);
    default:
        break;
    }
#undef  CMD_NAME
    return  "";
}


/******************************************************************************
*
*******************************************************************************/
static Mutex               rwbGpuProclock;
static list< sp<RwbGpuProcImp> > lspRwbGpuProcSingleton;
/******************************************************************************
*
*******************************************************************************/
RwbGpuProcImp::
RwbGpuProcImp(MUINT32 sensorIdx)
    : mUser(0)
    , mSensorIdx(sensorIdx)
    , mpRWB(NULL)
    , mpSrcGpBuf(NULL)
    , mpDstGpBuf(NULL)
{
    MY_LOGD("this %p, sensorIdx %d", this, mSensorIdx);
}

/******************************************************************************
*
*******************************************************************************/
RwbGpuProcImp::
~RwbGpuProcImp()
{
    MY_LOGD("this %p, sensorIdx %d", this, mSensorIdx);
}

/******************************************************************************
*
*******************************************************************************/
sp<RwbGpuProc>
RwbGpuProc::
createInstance(MUINT32 sensorIdx)
{
    return RwbGpuProcImp::getInstance(sensorIdx);
}

/******************************************************************************
*
*******************************************************************************/
sp<RwbGpuProc>
RwbGpuProcImp::
getInstance(MUINT32 sensorIdx)
{
    Mutex::Autolock _l(rwbGpuProclock);
    //
    //
    status_t status = OK;
    sp<RwbGpuProcImp> pRwbGpuProc = NULL;
    list< sp<RwbGpuProcImp> >::const_iterator iter;
    //
    for(iter = lspRwbGpuProcSingleton.begin(); iter != lspRwbGpuProcSingleton.end(); iter++)
    {
        if((*iter)->getSensorIdx() == sensorIdx)
        {
            pRwbGpuProc = (*iter);
        }
    }
    //
    if(pRwbGpuProc == NULL)
    {
        //create new
        pRwbGpuProc = new RwbGpuProcImp(sensorIdx);
        lspRwbGpuProcSingleton.push_back(pRwbGpuProc);
        //
        if  ( !pRwbGpuProc.get() || OK != (status = pRwbGpuProc->run("RwbGpuProcImp") ) )
        {
            MY_LOGE(
                "Fail to run mpRwbGpuProcThread - pRwbGpuProc(%p), status[%s(%d)]", pRwbGpuProc.get(), ::strerror(-status), -status);
            goto lbExit;
        }
        //
        pRwbGpuProc->init();
    }
    //
    MY_LOGD("pRwbGpuProc(%p), userCnt(%d)", pRwbGpuProc.get(), pRwbGpuProc->mUser);
    pRwbGpuProc->mUser++;
lbExit:
    //
    return (OK == status) ? pRwbGpuProc : NULL;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
RwbGpuProcImp::
destroyInstance()
{
    Mutex::Autolock _l(rwbGpuProclock);
    //
    mUser--;
    MY_LOGD("this(0x%x), userCnt(%d)",
            this,
            mUser);
    //
    if(mUser == 0)
    {
        list< sp<RwbGpuProcImp> >::iterator iter;
        for(iter = lspRwbGpuProcSingleton.begin(); iter != lspRwbGpuProcSingleton.end(); iter++)
        {
            if((*iter) == this)
            {
                lspRwbGpuProcSingleton.erase(iter);
                break;
            }
        }
        //
        uninit();
        //
        requestExit();
        join();
    }
    //
    MY_LOGD("-");
}

/******************************************************************************
*
*******************************************************************************/
void
RwbGpuProcImp::
requestExit()
{
    MY_LOGD("+");
    //
    Thread::requestExit();
    //
    postCommand(Command(Command::eCMD_EXIT));
    //
    MY_LOGD("-");
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
RwbGpuProcImp::
readyToRun()
{
    // (1) set thread name
    ::prctl(PR_SET_NAME,(unsigned long)"RwbGpuProcImp", 0, 0, 0);
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_CAPTURE;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOGD(
        "tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , ::gettid(), policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //
    return NO_ERROR;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RwbGpuProcImp::
threadLoop()
{
    //
    MY_LOGD("+");
    Command cmd;
    while( getCommand(cmd) )
    {
        switch  (cmd.eId)
        {
            case Command::eCMD_INIT:
                doRWBInit(); //MTKRWB init
                break;
            case Command::eCMD_PROCESS:
                doMTKRWB(); //MTKRWB main
                sem_post(&semGPUProcDone);
                break;
            case Command::eCMD_UNINIT:
                doRWBUninit(); //release mem.
                sem_post(&semGPUProcDone);
                break;
            case Command::eCMD_EXIT:
            default:
                MY_LOGD("default - Command::%s", cmd.name());
                break;
        }
        //
    }
    //
    MY_LOGD("-");

    return  true;

}

/******************************************************************************
 *
 ******************************************************************************/
void
RwbGpuProcImp::
postCommand(Command const& rCmd,MBOOL isWaitCondEn)
{

    {
        Mutex::Autolock _lock(mCmdQueMtx);
        //

        if  ( ! mCmdQue.empty() )
        {
            Command const& rBegCmd = *mCmdQue.begin();
            MY_LOGW("que size:%d > 0 with begin cmd::%s", mCmdQue.size(), rBegCmd.name());
        }
        //

        mCmdQue.push_back(rCmd);
        mCmdQueCond.broadcast();
        //
        MY_LOGD("- que size(%d), new command::%s", mCmdQue.size(), rCmd.name());
    }
    if(isWaitCondEn)
    {
        //  Wait until GPU Process Done.
        sem_wait(&semGPUProcDone);

    }
    //
}

/******************************************************************************
 *
 ******************************************************************************/
bool
RwbGpuProcImp::
getCommand(Command& rCmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        rCmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%s", rCmd.name());
        //
    }
    //
    return  ret;

}

/******************************************************************************
  *serInOutBuf
  ******************************************************************************/
bool
RwbGpuProcImp::
setSrcDstBuf(IImageBuffer* pSrcGpBuffer, IImageBuffer* pDstGpBuffer)
{
    //
    mpSrcGpBuf = pSrcGpBuffer;
    mpDstGpBuf = pDstGpBuffer;
    MY_LOGD("rwbGpuProc - postSrcDstBuf: (%p)=(%p),(%p)=(%p) ", mpSrcGpBuf, pSrcGpBuffer, mpDstGpBuf, pDstGpBuffer);
    //
    return true;
}
/******************************************************************************
  *Init RWBGpuProc
  ******************************************************************************/
bool
RwbGpuProcImp::
init()
{
    sem_init(&semGPUProcDone, 0, 0);
    postCommand(Command(Command::eCMD_INIT));
    //
    return true;
}

/******************************************************************************
  *Unint RWBGpuProc
  ******************************************************************************/
void
RwbGpuProcImp::
uninit()
{
    postCommand(Command(Command::eCMD_UNINIT), MTRUE);
    sem_destroy(&semGPUProcDone);
}

/******************************************************************************
  *Do RWBGpuProc
  ******************************************************************************/
void
RwbGpuProcImp::
doGPUProc()
{
    //
    postCommand(Command(Command::eCMD_PROCESS), MTRUE);
    //
}

/******************************************************************************
 *eID_INIT
 ******************************************************************************/
bool
RwbGpuProcImp::
doRWBInit()
{
#define CHECK_NR_RET( NR_ret )                          \
        do{                                             \
            MRESULT nr_ret = (NR_ret);                  \
            if( nr_ret != S_RWB_OK ) {                  \
                MY_LOGE("NR with error 0x%x", nr_ret);  \
                goto lbExit;                           \
            }                                           \
        }                                               \
        while(0);
    //
    MY_LOGD("doRWBInit +");
    //
    MBOOL ret = MFALSE;
    //
    mpRWB = MTKRWB::createInstance(DRV_RWB_OBJ_GLES);
    if ( mpRWB == NULL )
    {
        MY_LOGE("mpRWB is NULL");
        goto lbExit;
    }
    //
    mRWB_info.InputGBNum = 1;
    mRWB_info.OutputGBNum = 1;
    mGpu_tuning.GLESVersion = 3;
    mRWB_info.pTuningParaGPU = &mGpu_tuning;
    //init MTK RWB
    CHECK_NR_RET (mpRWB->RWBInit((MUINT32*) &mRWB_info, NULL));
    //

#undef CHECK_NR_RET
    //
    MY_LOGD("doRWBInit -");
    //
    ret = MTRUE;
    //
lbExit:
    //
    return ret;

}

/******************************************************************************
 *eID_UNINIT
 ******************************************************************************/
bool
RwbGpuProcImp::
doRWBUninit()
{
    MY_LOGD("doRWBUninit +");
    //MTK RWBGPU reset
    if( mpRWB != NULL )
    {
        mpRWB->RWBReset();
        mpRWB->destroyInstance(mpRWB);
        mpRWB = NULL;
    }
    MY_LOGD("doRWBUninit -");
    return true;
}

/******************************************************************************
 *eID_PROCESS
 ******************************************************************************/
bool
RwbGpuProcImp::
doMTKRWB()
{
#define CHECK_NR_RET( NR_ret )                          \
        do{                                             \
            MRESULT nr_ret = (NR_ret);                  \
            if( nr_ret != S_RWB_OK ) {                  \
                MY_LOGE("NR with error 0x%x", nr_ret);  \
                goto lbExit;                           \
            }                                           \
        }                                               \
        while(0);
    //
    MY_LOGD("+");
    MBOOL ret = MFALSE;
    //
    //set color space for input GB
    void* gbuffer = mpSrcGpBuf->getImageBufferHeap()->getHWBuffer();
    AHardwareBuffer* gpuSrcBuf = (AHardwareBuffer*)gbuffer;
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(AHardwareBuffer_getNativeHandle(gpuSrcBuf), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
    gralloc_extra_perform(AHardwareBuffer_getNativeHandle(gpuSrcBuf), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);

    //getTuningInfo from 3A
    RWBTune_T rwb;
    RWBTuning rwbtuning(rwb);
    //
    IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getSensorIdx(), LOG_TAG);
    if( pHal3A == NULL )
    {
        MY_LOGE("pHal3A is NULL");
        goto lbExit;
    }
    pHal3A->send3ACtrl(E3ACtrl_GetRwbInfo, reinterpret_cast<MINTPTR>(&rwbtuning), 0);

    //setTuningInfo to ALGO
    mRWB_info.pTuningParaRWB = &rwbtuning;

    //setImageInfo
    mRWB_info.InputImgFmt   = RWB_IMAGE_YV12;
    mRWB_info.OutputImgFmt  = RWB_IMAGE_YV12;
    mRWB_info.InputWidth    = mpSrcGpBuf->getImgSize().w;
    mRWB_info.InputHeight   = mpSrcGpBuf->getImgSize().h;
    mRWB_info.OutputWidth   = mpDstGpBuf->getImgSize().w;
    mRWB_info.OutputHeight  = mpDstGpBuf->getImgSize().h;
    mRWB_info.GB_Width      = mpSrcGpBuf->getBufStridesInBytes(0); // TODO:
    mRWB_info.GB_Height     = mpSrcGpBuf->getImgSize().h;
    //
    mRWB_info.SrcGraphicBuffer = mpSrcGpBuf->getImageBufferHeap()->getHWBuffer();//GB
    mRWB_info.DstGraphicBuffer = mpDstGpBuf->getImageBufferHeap()->getHWBuffer();//GB
    //
    if( mpRWB == NULL )
    {
        MY_LOGE("mpRWB is NULL");
        goto lbExit;
    }
    //
    //AddImage
    CHECK_NR_RET( mpRWB->RWBFeatureCtrl(RWB_FEATURE_ADD_IMAGE, &mRWB_info, NULL) );

    //do RWBGPUmain
    CHECK_NR_RET( mpRWB->RWBMain() );
    //
    //debug memcpy
    //memcpy(reinterpret_cast<void*>(mpDstGpBuf->getBufVA(0)),reinterpret_cast<void*>(mpSrcGpBuf->getBufVA(0)), (mpSrcGpBuf->getBufStridesInBytes(0) * mpSrcGpBuf->getImgSize().h));
    //MY_LOGD("GB_addr : %p, %p size: %d", (mpDstGpBuf->getBufVA(0)), (mpSrcGpBuf->getBufVA(0)), (mpSrcGpBuf->getBufStridesInBytes(0) * mpSrcGpBuf->getImgSize().h));
    //

#undef CHECK_NR_RET
    MY_LOGD("-");
    ret = MTRUE;
    //
lbExit:
    //
    if  ( pHal3A != NULL )
    {
        pHal3A->destroyInstance(LOG_TAG);
        pHal3A = NULL;
    }
    return ret;

}
