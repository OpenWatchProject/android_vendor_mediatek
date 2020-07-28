#define LOG_TAG "lomoh_151206"

#define MTK_LOG_ENABLE 1
#include <mtkcam/utils/std/Log.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>  // For property_get().
#include <sys/stat.h>

// LOG(from drv_log.h) -- start

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        "{WWWWWWWWWW} "
//#include "imageio_log.h"                    // Note: DBG_LOG_TAG will be used in header file, so header must be included after definition.
//DECLARE_DBG_LOG_VARIABLE(YYYYYYYYYY);
//EXTERN_DBG_LOG_VARIABLE(YYYYYYYYYY);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (YYYYYYYYYY_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (YYYYYYYYYY_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#if (MTKCAM_DRV_ISP_VERSION==30)
#define  ISP_PRE_FIX(REG) CAM_##REG
#else
#define  ISP_PRE_FIX(REG) DIP_X_##REG
#endif

// Vent@20120615: Don't know why "__func__" and "__FUNCTION__" act like
//     "__PRETTY_FUNCTION__". Add following lines as a workaround to make
//     their behavior correct.
#undef	__func__
#define	__func__	__FUNCTION__

///////////////////////////////////////////////////////////////////////////
//                          Default Settings.                            //
///////////////////////////////////////////////////////////////////////////
#ifndef DBG_LOG_TAG     // Set default DBG_LOG_TAG.
    #define DBG_LOG_TAG     ""
#endif  // DBG_LOG_TAG

///////////////////////////////////////////////////////////////////////////
//                      Base Debug Log Functions                         //
///////////////////////////////////////////////////////////////////////////
#ifndef USING_MTK_LDVT   // Not using LDVT.
    #include <mtkcam/utils/std/Log.h>
    #define NEW_LINE_CHAR   ""      // ALOG?() already includes a new line char at the end of line, so don't have to add one.

    #define BASE_LOG_VRB(fmt, arg...)       do {   CAM_LOGV(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg);  } while(0)    // <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #define BASE_LOG_DBG(fmt, arg...)       do {   CAM_LOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg);  } while(0)    // <Debug>: Show general debug information. E.g. Change of state machine; entry point or parameters of Public function or OS callback; Start/end of process thread...
    #define BASE_LOG_INF(fmt, arg...)       do {   CAM_LOGD(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg);  } while(0)    // <Info>: Show general system information. Like OS version, start/end of Service...
    #define BASE_LOG_WRN(fmt, arg...)       CAM_LOGW(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)    // <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #define BASE_LOG_ERR(fmt, arg...)       CAM_LOGE(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg)    // When MP, will only show log of this level. // <Fatal>: Serious error that cause program can not execute. <Error>: Some error that causes some part of the functionality can not operate normally.
    #define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                XLOGE("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#else   // Using LDVT.
    #include "uvvf.h"
    #define NEW_LINE_CHAR   "\n"

    #define BASE_LOG_VRB(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Verbose>: Show more detail debug information. E.g. Entry/exit of private function; contain of local variable in function or code block; return value of system function/API...
    #define BASE_LOG_DBG(fmt, arg...)        {\
        struct timespec tt1;\
        clock_gettime(CLOCK_REALTIME, &tt1);\
        VV_MSG(DBG_LOG_TAG "[%d][%d.%ld][%s] " fmt NEW_LINE_CHAR,gettid(),tt1.tv_sec,tt1.tv_nsec, __func__, ##arg);\
    }

    #define BASE_LOG_INF(fmt, arg...)        {\
        struct timespec tt1;\
        clock_gettime(CLOCK_REALTIME, &tt1);\
        VV_MSG(DBG_LOG_TAG "[%d][%d.%ld][%s] " fmt NEW_LINE_CHAR,gettid(),tt1.tv_sec,tt1.tv_nsec, __func__, ##arg);\
    }

    #define BASE_LOG_WRN(fmt, arg...)        VV_MSG(DBG_LOG_TAG "[%s] WARNING: " fmt NEW_LINE_CHAR, __func__, ##arg)	// <Warning>: Some errors are encountered, but after exception handling, user won't notice there were errors happened.
    #define BASE_LOG_ERR(fmt, arg...)        {\
        struct timespec tt1;\
        clock_gettime(CLOCK_REALTIME, &tt1);\
        VV_ERRMSG(DBG_LOG_TAG "[%d][%d.%ld][%s, %s, line%04d] ERROR: " fmt NEW_LINE_CHAR,gettid(),tt1.tv_sec,tt1.tv_nsec, __FILE__, __func__, __LINE__, ##arg);\
    }

    #define BASE_LOG_AST(cond, fmt, arg...)     \
        do {        \
            if (!(cond))        \
                VV_ERRMSG("[%s, %s, line%04d] ASSERTION FAILED! : " fmt NEW_LINE_CHAR, __FILE__, __func__, __LINE__, ##arg);        \
        } while (0)

#endif  // USING_MTK_LDVT


///////////////////////////////////////////////////////////////////////////
//          Macros for dynamically changing debug log level              //
///////////////////////////////////////////////////////////////////////////
// [Set default debug log level here] (when DBG_LOG_LEVEL_SET() is not called.)
// Change the desired level (and following levels) to true.
// e.g. Change DEBUG and all level below DEBUG to true, then debug level is DEBUG.
#define DECLARE_DBG_LOG_VARIABLE(ModuleName)    \
    bool ModuleName ## _DbgLogEnable_VERBOSE   = false; \
    bool ModuleName ## _DbgLogEnable_DEBUG     = false; \
    bool ModuleName ## _DbgLogEnable_INFO      = true;  \
    bool ModuleName ## _DbgLogEnable_WARN      = true;  \
    bool ModuleName ## _DbgLogEnable_ERROR     = true;  \
    bool ModuleName ## _DbgLogEnable_ASSERT    = true;  \

#define EXTERN_DBG_LOG_VARIABLE(ModuleName)    \
    extern bool ModuleName ## _DbgLogEnable_VERBOSE;    \
    extern bool ModuleName ## _DbgLogEnable_DEBUG;      \
    extern bool ModuleName ## _DbgLogEnable_INFO;       \
    extern bool ModuleName ## _DbgLogEnable_WARN;       \
    extern bool ModuleName ## _DbgLogEnable_ERROR;      \
    extern bool ModuleName ## _DbgLogEnable_ASSERT;     \

// [Set default debug log level here] (when DBG_LOG_LEVEL_SET() is called.)
// Note: The default value in the property_get() does not controls the default
//       debug log level. It's the position of "default:" in the switch() controls
//       the default debug log level. E.g. put "default:" to case '5' will make
//       default debug level to WARN.
// PropertyStr: the Android property name you will use to control the debug level.
#define DBG_LOG_CONFIG(GroupName, ModuleName)                       \
    do {                                                            \
        char acDbgLogLevel[PROPERTY_VALUE_MAX] = {'\0'};                            \
        property_get("vendor.debuglog." #GroupName "." #ModuleName, acDbgLogLevel, "0");             \
        ModuleName ## _DbgLogEnable_VERBOSE   = false;              \
        ModuleName ## _DbgLogEnable_DEBUG     = false;              \
        ModuleName ## _DbgLogEnable_INFO      = true;              \
        ModuleName ## _DbgLogEnable_WARN      = true;              \
        ModuleName ## _DbgLogEnable_ERROR     = true;              \
        ModuleName ## _DbgLogEnable_ASSERT    = true;              \
        if (acDbgLogLevel[0] == '0')                                \
        {                                                           \
            property_get("vendor.debuglog." #GroupName, acDbgLogLevel, "9");   \
        }                                                           \
        switch (acDbgLogLevel[0])                                   \
        {                                                           \
            case '2':                                               \
                ModuleName ## _DbgLogEnable_VERBOSE   = true;       \
            case '3':                                               \
                ModuleName ## _DbgLogEnable_DEBUG     = true;       \
            default:                                                \
            case '4':                                               \
                ModuleName ## _DbgLogEnable_INFO      = true;       \
            case '5':                                               \
                ModuleName ## _DbgLogEnable_WARN      = true;       \
            case '6':                                               \
                ModuleName ## _DbgLogEnable_ERROR     = true;       \
            case '7':                                               \
                ModuleName ## _DbgLogEnable_ASSERT    = true;       \
            case '8':                                               \
                break;                                              \
        }                                                           \
    } while (0)


// LOG(from drv_log.h -- end

#include <stdio.h>
#include <stdarg.h>
#include <utils/StrongPointer.h>
using namespace android;

#include <ui/GraphicBuffer.h> //test
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;

#include "lomo_hal_jni_imp.h"
#include <sys/prctl.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h> //for Everest
#include <mtkcam/utils/imgbuf/IImageBuffer.h> //for Everest
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h> //for Everest
#include <mtkcam/utils/std/Format.h> //for Everest
using namespace NSCam::Utils::Format;
using namespace NSCam;


#include <drv/isp_reg.h>
#if DIP_REG_NEEDED // defined in Android.mk
    #include <dip_reg.h>
#endif

#include "camera_custom_lomo_param_jni.h"

/*************************************************************************************
* for extra reg setting
*************************************************************************************/
int lomoApplyTuningSettingExt(void *ppIspPhyReg);

/*************************************************************************************
* for BlitSteam
*************************************************************************************/
#include "DpBlitStream.h"

/*************************************************************************************
* Log Utility
*************************************************************************************/

#undef   DBG_LOG_TAG
#define  DBG_LOG_TAG    "{LomoHalJniImp} "

#include "aaa_types.h"
#include "aaa_log.h"
#include "camera_custom_nvram.h"
#include "isp_tuning.h"
#include "isp_tuning_cam_info.h"
#if !DIP_REG_NEEDED // defined in Android.mk
    #include "isp_tuning_idx.h"
#endif
#include "isp_tuning_custom.h"
#include "isp_tuning_custom_instance.h"
#include <ui/gralloc_extra.h>

using namespace NSIspTuning;

#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
using NSCam::IImageBufferAllocator;
using NSCam::IImageBuffer;

#include <utils/Trace.h>
#define LOMO_CONFIG_FOR_SYSTRACE  1

#if LOMO_CONFIG_FOR_SYSTRACE
#ifdef ATRACE_TAG
#undef ATRACE_TAG
#endif
#define ATRACE_TAG ATRACE_TAG_ALWAYS //ATRACE_TAG_CAMERA
#define LOMO_TRACE_CALL()                    ATRACE_CALL()
#define LOMO_TRACE_BEGIN(name)           ATRACE_BEGIN(name) //android::Tracer::traceBegin(ATRACE_TAG_ALWAYS, name)
#define LOMO_TRACE_END()                      ATRACE_END()// android::Tracer::traceEnd(ATRACE_TAG_ALWAYS)

#else
#define LOMO_TRACE_CALL()

#endif  // LOMO_CONFIG_FOR_SYSTRACE



#define LOMO_STRING "Lomo_Hal_Jni"//"Lomo_jni"

DECLARE_DBG_LOG_VARIABLE(lomohalJ);

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST

#ifdef MY_LOG
#undef MY_LOG
#endif

#define MY_LOG(fmt, arg...) CAM_LOGD(fmt, ##arg)

#define LOG_VRB(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (lomohalJ_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (lomohalJ_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define USE_GRAPHIC_BUFFER_HEAP 1
#define LOMO_EFFECT_DST_BUFF_NUM 48
#define LOMO_EFFECT_TIME_OUT 10000000000 //10 sec

#define LOMO_P2_DEQUE_CALLBACK 1

#define PIXEL2BYTE(PLANE, PIXEL, FORMAT)                                        \
    (PIXEL * NSCam::Utils::Format::queryPlaneBitsPerPixel(FORMAT, PLANE)) >> 3         \

#define DEBUG_LOMO_DUMPSRC_ENABLED "vendor.debug.lomo.dumpsrc.enabled"

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
   static IImageBufferAllocator* allocatorlomo;//=NSCam::IImageBufferAllocator::getInstance();// = IImageBufferAllocator::getInstance(); //    //IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
   static  DpBlitStream   *pStream = NULL;//new DpBlitStream();
   sp<IImageBuffer> pBufSrc ;
   sp<IImageBuffer> pBufDst ;
   sp<IImageBuffer> pBufDstHeap[LOMO_EFFECT_DST_BUFF_NUM] ;
   static MUINT8* pBufDstHeapBuffIdx[LOMO_EFFECT_DST_BUFF_NUM]={0};
   static MBOOL pBufDstHeapBuffMarked[LOMO_EFFECT_DST_BUFF_NUM]={MFALSE};
   volatile MINT32  mInitCountLomoHalJni=0;
   static IspTuningCustom*  m_pIspTuningCustom =NULL;
   static INormalStream* mpIStream=NULL;  //For Pass2
   static MBOOL brawdumpEnable = MFALSE;
   static MUINT32 lu32BuffeffectIdx = 0;
   static MBOOL bAddrDumpEnable = MFALSE;
   static MBOOL bDebugLomoTimerEnabled = MFALSE;
   void *pTuningQueBuf;
//   dip_x_reg_t/*isp_reg_t*/ *pIspPhyReg;

   #define TUNING_NUM  (MTK_CONTROL_EFFECT_MODE_NUM) //LOMO_EFFECT_DST_BUFF_NUM //20  //20
   MINT8* pTuningQueBufAddrbymalloc[TUNING_NUM];


    static LomoHalJniImp singleton;
    static int g_dumpSrcEnabled = ::property_get_int32(DEBUG_LOMO_DUMPSRC_ENABLED, 0);
/**************************************************************************
 *                         LOCAL CLASS                            *
 **************************************************************************/

class AllocInfo
{
    public:
        MUINT32 w;
        MUINT32 h;
        MUINT32 fmt;
        MINT    usage; //IImageBuffer usage: SW R/w
        MUINT32 strideInByte[3];
        MBOOL   isGralloc;
        //to be added
    public:
        AllocInfo(
                MUINT32 const _w,
                MUINT32 const _h,
                MUINT32 const _fmt,
                MINT    const _usage = eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN,
                MBOOL   const _gralloc = MFALSE)
            : w(_w)
            , h(_h)
            , fmt(_fmt)
            , usage(_usage)
            , isGralloc(_gralloc)
        {
            for (MUINT32 i = 0; i < 3; i++)
            {
                strideInByte[i] = 0;

            }
        }

        AllocInfo(  MUINT32 const _w,
                MUINT32 const _h,
                MUINT32 const _fmt,
                MINT    const _usage,
                MUINT32 const _strideInByte[],
                MBOOL   const _gralloc = MFALSE )
            : w(_w)
            , h(_h)
            , fmt(_fmt)
            , usage(_usage)
            , isGralloc(_gralloc)
        {
            for (MUINT32 i = 0; i < 3; i++)
            {
                strideInByte[i] = _strideInByte[i];
            }
        }
};

/**************************************************************************
*
**************************************************************************/
LomoHalJniImp::LomoHalJniImp()
            :LomoHalJni()
            , gu32pvCBWidth(0)
            , gu32pvCBHeight(0)
            , gu32pvCBformat(0)
            , gu32ppSrcImgWidth(0)
            , gu32ppSrcImgHeight(0)
            , gu32ppDstImgWidth(0)
            , gu32ppDstImgHeight(0)
            , gu32ppDstImgformat(0)
            , mLomoDequeThread(0)
{
    int i = 0;
    LOG_INF("+");
    if(allocatorlomo==NULL)
    {
        allocatorlomo = NSCam::IImageBufferAllocator::getInstance();
    }
    if(m_pIspTuningCustom==NULL)
    {
       ESensorDev_T m_sesnorDev = ESensorDev_Main;
       MY_LOG("IspTuningCustom::createInstance(m_sesnorDev, 0x0306)");
       m_pIspTuningCustom = IspTuningCustom::createInstance(m_sesnorDev, 0x0306);
    }

    // init memory buffer
    for (i = 0; i < LOMO_HAL_JNI_MAX_PLANE; ++i)
    {
        gu32ppSrcImgVA[i] = gu32ppSrcImgMVA[i] = NULL;
        gu32ppDstImgVA[i] = gu32ppDstImgMVA[i] = NULL;
    }

    // init semphore
    mDequeSem.count = 0;
    mDequeDoneSem.count = 0;
    mDequeSemEnd.count = 0;

    mState = LOMO_STATE_NONE;

    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
LomoHalJniImp::~LomoHalJniImp()
{
    LOG_INF("+");
//    int i;
//    Mutex::Autolock lock(mLockLomoHalJni);
#if 0
    if(pStream != NULL)
    {
        delete pStream;
        MY_LOG("delete pStream");
        pStream = NULL;
    }
#endif
    LOG_INF("-");

}

/**************************************************************************
*
**************************************************************************/

extern "C"
{
    LomoHalJni * getLomoHalJniInstance(void)
    {
        return LomoHalJni::createInstance();
    }
}

LomoHalJni *LomoHalJni::createInstance( void )
{
    LOG_INF("+");
    DBG_LOG_CONFIG(featureio, lomohalJ);
    LOG_INF("-");
    return &singleton;
}

/**************************************************************************
*
**************************************************************************/
MVOID LomoHalJni::destroyInstance( void )
{
    LOG_INF("+");
    LOG_INF("-");
}

/**************************************************************************
*
**************************************************************************/
MINT32 LomoHalJniImp::init()
{
    LOG_INF("+,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    brawdumpEnable = ::property_get_int32("vendor.lomo.jni.rawdump", 0);
    lu32BuffeffectIdx = ::property_get_int32("vendor.lomo.jni.effectidx", 0);
    bAddrDumpEnable = ::property_get_int32("vendor.lomo.jni.addrdump", 0);
    bDebugLomoTimerEnabled = ::property_get_int32("vendor.debug.lomo.timer.enabled", 0);

    mState = LOMO_STATE_NONE;

    gu32ppSrcImgVA[0]=NULL;
    gu32ppSrcImgMVA[0]=NULL;

    if(pStream == NULL)
    {
       MY_LOG("new DpBlitStream()");
        pStream = new DpBlitStream();
    }
    if(mpIStream == NULL)
    {
        mpIStream=NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(0xffff);  //0xffff means pure p2 //non-legacy
        MY_LOG("LomoHalJniImp::init- mpIStream->createInstance()");
        if (mpIStream->init(LOMO_STRING))
        {
            MY_LOG("LomoHalJniImp::init - mpIStream->init() OK !");
        }
        else
        {
            LOG_ERR("LomoHalJniImp::init - mpIStream->init() FAILED!!");
        }
    }
    else
    {
        MY_LOG("NO! LomoHalJniImp::init - mpIStream->createInstance()");
        MY_LOG("NO! LomoHalJniImp::init - mpIStream->init()");
    }
    MY_LOG("LomoHalJniImp - mpIStream->createInstance()");


    android_atomic_inc(&mInitCountLomoHalJni);
    //====== Set State ======

    SetLomoState(LOMO_STATE_ALIVE);

    //====== sem_init ======
    ::sem_init(&mDequeSem, 0, 0);
    ::sem_init(&mDequeDoneSem, 0, 0);
    ::sem_init(&mDequeSemEnd, 0, 0);
//    MY_LOG("sem_init done");
    //====== Create Thread  ======

    // create thread
    pthread_create(&mLomoDequeThread, NULL, LomoDequeThreadLoop, this);

//    android_atomic_inc(&mUsers);

    MY_LOG("MEM_BY_MALLOC + tuning setting 1 time");
    for(MUINT32 i=0;i<TUNING_NUM;i++)
    {
       if (mpIStream != NULL)
       {
           pTuningQueBufAddrbymalloc[i]=(MINT8*)malloc(mpIStream->getRegTableSize());
           memset((MUINT8*)(pTuningQueBufAddrbymalloc[i]), 0x0, mpIStream->getRegTableSize());
           LomoApplyTuningSetting(i, (void *)pTuningQueBufAddrbymalloc[i]);
       }
    }


INIT_EXIT:
    LOG_INF("-,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);

    return err;
}

/**************************************************************************
*
**************************************************************************/
MINT32 LomoHalJniImp::uninit()
{

    LOG_INF("+,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
    //
//    Mutex::Autolock lock(mLockLomoHalJni);
    //
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    //
    android_atomic_dec(&mInitCountLomoHalJni);
    //
    if(mInitCountLomoHalJni > 0) {
        err = LOMOHALJNI_STILL_USERS;
        goto UNINIT_EXIT;
    }
    //====== Set State ======
    SetLomoState(LOMO_STATE_UNINIT);
    //====== Destroy Thread ======
    MY_LOG("post mDequeSem");
    if(1)
    {
        int mu32Val;
        ::sem_getvalue(&mDequeSem, &mu32Val);
        MY_LOG("mDequeSem %d",mu32Val);
    }
    ::sem_post(&mDequeSem);
    //wait sem
    MY_LOG("wait mDequeSemEnd");
    ::sem_wait(&mDequeSemEnd);
    MY_LOG("got mDequeSemEnd");

    pthread_join(mLomoDequeThread, NULL);
    MY_LOG("close thread :  mLomoDequeThread");

    for(MUINT32 i=0;i<TUNING_NUM;i++)
    {
       free(pTuningQueBufAddrbymalloc[i]);
       pTuningQueBufAddrbymalloc[i] = NULL;
    }

//    Mutex::Autolock lock(mLockLomoHalJni);
    if(pStream != NULL)
    {
        delete pStream;
        MY_LOG("delete pStream");
        pStream = NULL;
    }
    if(mpIStream != NULL)
    {
        mpIStream->uninit(LOMO_STRING);
        MY_LOG("LomoHalJniImp::uninit - mpIStream->uninit()");
        mpIStream->destroyInstance();//(LOMO_STRING);
        MY_LOG("LomoHalJniImp::uninit - mpIStream->destroyInstance()");
        mpIStream = NULL;
    }
    else
    {
        MY_LOG("NO! LomoHalJniImp::uninit - mpIStream->uninit()");
        MY_LOG("NO! LomoHalJniImp::uninit - mpIStream->destroyInstance()");
    }

    // destory semphore
    ::sem_destroy(&mDequeSem);
    ::sem_destroy(&mDequeDoneSem);
    ::sem_destroy(&mDequeSemEnd);
//    MY_LOG("sem_destroy done");

UNINIT_EXIT:
    LOG_INF("-,mInitCountLomoHalJni(%d)",mInitCountLomoHalJni);
    return err;
}


MINT32 LomoHalJniImp::AllocLomoSrcImage(MUINT32 pvCBWidth, \
                                                                     MUINT32 pvCBHeight, \
                                                                     MUINT32 pvCBformat, \
                                                                     MUINT32 ppSrcImgWidth, \
                                                                     MUINT32 ppSrcImgHeight)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    MUINT32 InPA, InVA;
    MUINT32 plane=1;
    MUINT32 bufStridesInBytes[3]={0};
    MUINT32 imgFormat;
    MUINT32 i ;
    MUINT32 count ;
     MSize bufStridesInPixels[3] = {0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    gu32pvCBWidth = pvCBWidth;
    gu32pvCBHeight = pvCBHeight;
    gu32pvCBformat = pvCBformat;
    gu32ppSrcImgWidth = ppSrcImgWidth;
    gu32ppSrcImgHeight = ppSrcImgHeight;
    gu32ppDstImgWidth = gu32ppSrcImgWidth;
    gu32ppDstImgHeight = gu32ppSrcImgHeight;
    MY_LOG("gu32pvCBWidth = %d",gu32pvCBWidth);
    MY_LOG("gu32pvCBHeight = %d",gu32pvCBHeight);
    MY_LOG("gu32ppDstImgWidth = %d",gu32ppDstImgWidth);
    MY_LOG("gu32ppDstImgHeight = %d\n",gu32ppDstImgHeight);

    AllocInfo allocinfoIn(ppSrcImgWidth, ppSrcImgHeight, \
    	NSCam::eImgFmt_YUY2, \
    	eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );


    AllocInfo allocinfoOut(gu32ppDstImgWidth, gu32ppDstImgHeight, \
    	NSCam::eImgFmt_YV12/*eImgFmt_I420*/, \
    	eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );


    for ( i = 0; i < plane; i++)
    {
        bufStridesInPixels[i] =
        	    MSize(queryPlaneWidthInPixels(allocinfoIn.fmt,i, allocinfoIn.w),
        	    queryPlaneHeightInPixels(allocinfoIn.fmt,i, allocinfoIn.h));
        bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoIn.fmt);
    }

    IImageBufferAllocator::ImgParam imgParam(
    	allocinfoIn.fmt,
    	MSize(allocinfoIn.w,allocinfoIn.h),
    	bufStridesInBytes, //SL for new arguments bufStridesInPixels,
    	bufBoundaryInBytes,
    	plane
    	);
    pBufSrc = allocatorlomo->alloc_ion(LOG_TAG, imgParam);
    pBufSrc->lockBuf( LOG_TAG, allocinfoIn.usage ) ;
    for ( count = 0; count < 1/*just YUY2 1 plane LOMO_HAL_JNI_MAX_PLANE*/; count++)
    {
        gu32ppSrcImgVA [count] = (MUINT8*)pBufSrc.get() ->getBufVA(count);  //This is the source image VA ADDR to postprocess (YUY2)
        gu32ppSrcImgMVA[count] =  (MUINT8*)pBufSrc.get() ->getBufPA(count);//This is the source image MVA ADDR to postprocess (YUY2)
    }


//Output Image
    return err;
}

MINT32 LomoHalJniImp::AllocLomoDstImage(MUINT32 Number, \
                                                                     MUINT8** ppDstImgVA \
                                                                       )
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    if(pBufDstHeapBuffIdx[Number]!=NULL)
    {
        if(pBufDstHeapBuffIdx[Number]!= ppDstImgVA[0])
        {
            LOG_ERR("ERROR !! pBufDstHeapBuffIdx[%d] is NOT matching SOURCE 0x%p",Number,(MUINT8*)ppDstImgVA[0]);
            err = LOMOHALJNI_WRONG_PARAM;
        }
        return err;
    }
    AllocInfo allocinfoIn(gu32ppDstImgWidth, gu32ppDstImgHeight, \
                                            NSCam::eImgFmt_YV12/*eImgFmt_I420*/, \
                                            eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
    MSize bufStridesInPixels[3] = {0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3]={0};
    MUINT32 plane = 3;
    MUINT32 i, count;
    for ( i = 0; i < plane; i++)
    {
        bufStridesInPixels[i] =
                           MSize(queryPlaneWidthInPixels(allocinfoIn.fmt,i, allocinfoIn.w),
                                   queryPlaneHeightInPixels(allocinfoIn.fmt,i, allocinfoIn.h));
                       bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoIn.fmt);
    }
    IImageBufferAllocator::ImgParam imgParam(
                  	allocinfoIn.fmt,
                  	MSize(allocinfoIn.w,allocinfoIn.h),
                  	bufStridesInBytes, //SL for new arguments bufStridesInPixels,
                  	bufBoundaryInBytes,
                  	plane
                  	);
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                        (-1), (MUINTPTR)ppDstImgVA[0],
                                        0, 0, 0);
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                        LOG_TAG,
                                                        imgParam,
                                                        portBufInfo);
    if(pHeap == 0)
    {
        MY_LOG("pHeap is NULL- skip this time");
        return MFALSE;
    }
    pBufDstHeap[Number] = pHeap->createImageBuffer();
    pBufDstHeap[Number]->incStrong(pBufDstHeap[Number].get());
    pBufDstHeap[Number]->lockBuf(
                        LOG_TAG,
                        allocinfoIn.usage );// eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    if (pBufDstHeap[Number].get() == NULL)
    {
        MY_LOG("NULL Buffer!- skip this time");
        return LOMOHALJNI_NULL_OBJECT;
    }
    pBufDstHeapBuffIdx[Number]= (MUINT8*)(pBufDstHeap[Number].get() ->getBufVA(0));
    pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
    if(pBufDstHeap[Number].get() ->getBufVA(0)==0) //NULL
    {
        MY_LOG("No MVA addr !!! - skip this time");
        return LOMOHALJNI_WRONG_PARAM;
    }
    if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
    {
        for(MUINT32 NumberIdx=0; NumberIdx<LOMO_EFFECT_DST_BUFF_NUM; NumberIdx++)
        {
            MY_LOG("pBufDstHeapBuffIdx[%d]=0x%p",NumberIdx,(MUINT8*)pBufDstHeapBuffIdx[NumberIdx]);
        }
    }
    return err;
}


MINT32 LomoHalJniImp::AllocLomoDstImage(MUINT32 Number, \
    MUINT8** ppDstImgVA, \
    MINT32 ionFD \
    )
{

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    if(pBufDstHeapBuffIdx[Number]!=NULL)
    {
        if(pBufDstHeapBuffIdx[Number]!= ppDstImgVA[0])
        {
            LOG_ERR("ERROR !! pBufDstHeapBuffIdx[%d] is NOT matching SOURCE 0x%p",Number,(MUINT8*)ppDstImgVA[0]);
            err = LOMOHALJNI_WRONG_PARAM;
        }
        return err;
    }
    AllocInfo allocinfoIn(gu32ppDstImgWidth, gu32ppDstImgHeight, \
                                            NSCam::eImgFmt_YV12/*eImgFmt_I420*/, \
                                            eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
    MSize bufStridesInPixels[3] = {0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3]={0};
    MUINT32 plane = 3;
    MUINT32 i, count;
    for ( i = 0; i < plane; i++)
    {
        bufStridesInPixels[i] =
                           MSize(queryPlaneWidthInPixels(allocinfoIn.fmt,i, allocinfoIn.w),
                                   queryPlaneHeightInPixels(allocinfoIn.fmt,i, allocinfoIn.h));
                       bufStridesInBytes[i]  = PIXEL2BYTE(i, bufStridesInPixels[i].w, allocinfoIn.fmt);
    }
    IImageBufferAllocator::ImgParam imgParam(
    	allocinfoIn.fmt,
    	MSize(allocinfoIn.w,allocinfoIn.h),
    	bufStridesInBytes, //SL for new arguments bufStridesInPixels,
    	bufBoundaryInBytes,
    	plane
    	);
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
    	ionFD/*  (-1)*/, (MUINTPTR)ppDstImgVA[0],
    	0, 0, 0);
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
    	LOG_TAG,
    	imgParam,
    	portBufInfo);
    if(pHeap == 0)
    {
        LOG_INF("pHeap is NULL- skip this time");
        return MFALSE;
    }
    pBufDstHeap[Number] = pHeap->createImageBuffer();
    pBufDstHeap[Number]->incStrong(pBufDstHeap[Number].get());
    pBufDstHeap[Number]->lockBuf(
    	LOG_TAG,
    	allocinfoIn.usage );// eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    if (pBufDstHeap[Number].get() == NULL)
    {
        LOG_INF("NULL Buffer!- skip this time");
        return LOMOHALJNI_NULL_OBJECT;
    }
    pBufDstHeapBuffIdx[Number]= (MUINT8*)(pBufDstHeap[Number].get() ->getBufVA(0));
    pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
    if(pBufDstHeap[Number].get() ->getBufVA(0)==0) //NULL
    {
            LOG_INF("No MVA addr !!! - skip this time");
            return LOMOHALJNI_WRONG_PARAM;
    }
    MY_LOG("pBufDstHeap[%d].get() ->getBufVA(0)=%p",Number,(MINT8*)pBufDstHeap[Number].get() ->getBufVA(0));
    return err;
}

static void dumpSrcBuffer(IImageBuffer *pIMGBuffer, MUINT32 requestNo)
{
    static MSize s_con_write_size;
    static MINT32 s_con_write_countdown = 0;
    if (!s_con_write_countdown && (s_con_write_size != pIMGBuffer->getImgSize()))
    {
        s_con_write_size = pIMGBuffer->getImgSize();
        s_con_write_countdown = 5;
    }

    if (s_con_write_countdown)
    {
        MINT imgFormat = pIMGBuffer->getImgFormat();
        MUINT32 u4PixelToBytes = 0;

        if (imgFormat == eImgFmt_YUY2) u4PixelToBytes = 2;
        if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;

        if (NSCam::Utils::makePath("/sdcard/matrixEffect/",0660) == false)
        {
            MY_LOGW("makePath(\"/sdcard/matrixEffect\") error");
        }
        char filename[256] = {0};
        sprintf(filename, "/sdcard/matrixEffect/lomo_src_%dx%d_S%zu_p2b_%d_N%d.yuy2",
            pIMGBuffer->getImgSize().w,
            pIMGBuffer->getImgSize().h,
            pIMGBuffer->getBufStridesInBytes(0),
            u4PixelToBytes,
            requestNo);
        pIMGBuffer->saveToFile(filename);
        s_con_write_countdown--;
    }
}

MINT32 LomoHalJniImp::UploadLomoSrcImage(MUINT8* pvCBVA)
{

    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    void           *pBuffer[3];
    void           *pBufferMva[3];
    MUINT32       size[3];
    DpRect         tmpRect;
    MUINT i,count;
    LomoTimer ldequeTmr("DpBlitStream deque",size[0],bDebugLomoTimerEnabled);
    LOMO_TRACE_CALL();
    if(pStream == NULL)
    {
        pStream = new DpBlitStream();
    }
    // Setup buffer address
    pBuffer[0] = (void*)pvCBVA;  //YV12 YUV420
    pBuffer[1] = (void*)((MUINTPTR)pBuffer[0]+((gu32pvCBWidth*gu32pvCBHeight))); //YV12 YUV420
    pBuffer[2] = (void*)((MUINTPTR)pBuffer[1]+((gu32pvCBWidth*gu32pvCBHeight)>>2));//YV12 YUV420

    // Setup buffer size
    size[0] = (gu32pvCBWidth*gu32pvCBHeight);
    size[1] = (gu32pvCBWidth*gu32pvCBHeight) >> 2;
    size[2] = (gu32pvCBWidth*gu32pvCBHeight) >> 2;

    pStream->setSrcBuffer(pBuffer, size, 3);
    tmpRect.x = 0;
    tmpRect.y = 0;
    tmpRect.w = gu32pvCBWidth;
    tmpRect.h = gu32pvCBHeight;
    pStream->setSrcConfig(gu32pvCBWidth, gu32pvCBHeight, DP_COLOR_YV12, eInterlace_None, &tmpRect);

    // Setup Src Image buffer address
    for(i=0;i<LOMO_HAL_JNI_MAX_PLANE;i++)
    {
        pBuffer[i]=(void*)gu32ppSrcImgVA[i];
        pBufferMva[i]=(void*)gu32ppSrcImgMVA[i];
    }
    // Setup buffer size
    size[0] = (gu32ppSrcImgWidth*gu32ppSrcImgHeight)<<1;
    size[1] = 0;
    size[2] = 0;

    ldequeTmr.start("DpBlitStream deque",size[0],bDebugLomoTimerEnabled);
    pStream->setDstBuffer(pBuffer,pBufferMva, size, 1);
    pStream->setDstConfig(gu32ppSrcImgWidth, gu32ppSrcImgHeight, DP_COLOR_YUY2);
    if (pStream->invalidate() != DP_STATUS_RETURN_SUCCESS)
    {
        LOG_ERR("pStream->invalidate() failed!");
    }
    ldequeTmr.printTime();

    if (g_dumpSrcEnabled)
    {
        static int s_magic =0;
        s_magic++;
        dumpSrcBuffer(pBufSrc.get(), s_magic);
    }

    return err;
}


MINT32 LomoHalJniImp::FreeLomoSrcImage(void)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    if(pBufSrc.get()==NULL)
    {
        LOG_WRN("pBufSrc.get() is NULL & failure in allocatorlomo->free(pBufSrc.get()!");
    }
    else
    {
        if( !pBufSrc->unlockBuf( LOG_TAG ) )
        {
            LOG_WRN("!pBufSrc->unlockBuf( LOG_TAG ) failed !");
        }
        allocatorlomo->free(pBufSrc.get());
        MY_LOG("FreePostProcessSrcImage()");
    }
    for (MUINT32 count = 0; count < LOMO_HAL_JNI_MAX_PLANE; count++)
    {
//        MY_LOG("Free\n gu32ppSrcImgVA[%d] 0x%p",count,gu32ppSrcImgVA[count]);
//        MY_LOG("gu32ppSrcImgVA[%d] 0x%p",count,gu32ppSrcImgVA[count]);
        gu32ppSrcImgVA[count] = NULL;
        gu32ppSrcImgVA[count] = NULL;
    }
    pBufSrc = NULL;

    return err;
}


MUINT32 LomoHalJniImp::LomoApplyTuningSetting(MUINT32 caseNum, void *ppIspPhyReg)
{
    int ret = 0;
//    color setting>>
//    TUNING_MGR_REG_IO_STRUCT  regIo[500];
    MUINT32 i;
    MUINT32 eIdx_Effect = caseNum;//(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+(caseNum%7);
    RAWIspCamInfo m_rIspCamInfov3 = NSIspTuning::RAWIspCamInfo();

    EIndex_Effect_T m_eIdx_Effect = (EIndex_Effect_T)eIdx_Effect;
    IspUsrSelectLevel_T m_IspUsrSelectLevel;// = NSIspTuning::IspUsrSelectLevel_T();
    ISP_NVRAM_G2C_T g2c;
    ISP_NVRAM_G2C_SHADE_T g2c_shade;
    ISP_NVRAM_SE_T se;
    ISP_NVRAM_GGM_T ggm;
    ESensorDev_T m_sesnorDev = ESensorDev_Sub;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("vendor.lomo.jni.regdump", value, "0");
    MBOOL bRegDumpEnable = atoi(value);

#if (MTKCAM_DRV_ISP_VERSION==30)
    isp_reg_t* ppIspPhyRegLocal = (isp_reg_t*)ppIspPhyReg;
#else
    dip_x_reg_t* ppIspPhyRegLocal = (dip_x_reg_t*)ppIspPhyReg;
#endif
    if(bRegDumpEnable)
    {
        MY_LOG("vendor.lomo.jni.regdump=%d ",bRegDumpEnable);
        MY_LOG("eIdx_Effect(%d) #0",eIdx_Effect);
    }
#if DIP_REG_NEEDED
    // isp 5.0 code
    //Fix effect unstable in WhiteBoard/BlackBoard/Posterize Effect    140821 >>
    if(bRegDumpEnable)//if(bAddrDumpEnable)
    {
        MY_LOG("se.edge_ctrl_1.val=0x%8x \n",se.se_edge_ctrl_1.val);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL=%d \n",se.se_edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C=%d \n",se.se_edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL=%d \n",se.se_edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C=%d \n",se.se_edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C);
    }
    //se.se_edge_ctrl.val=0;
    //se.y_ctrl.val=0;
    //se.edge_ctrl_1.val=0;
    //se.edge_ctrl_2.val=0;
    //se.edge_ctrl_3.val=0;
    //se.special_ctrl.val=0;
    //se.core_ctrl_1.val=0;
    //se.core_ctrl_2.val=0;
    se.top_ctrl.val = 0;
    se.se_y_specl_ctrl.val = 0;
    se.se_edge_ctrl_1.val = 0;
    se.se_edge_ctrl_2.val = 0;
    se.se_core_ctrl_1.val = 0;
    se.se_core_ctrl_2.val = 0;
#else
    //Fix effect unstable in WhiteBoard/BlackBoard/Posterize Effect    140821 >>
    if(bRegDumpEnable)//if(bAddrDumpEnable)
    {
        MY_LOG("se.edge_ctrl_1.val=0x%8x \n",se.edge_ctrl_1.val);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL=%d \n",se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C=%d \n",se.edge_ctrl_1.bits.SEEE_SE_HORI_EDGE_GAIN_C);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL=%d \n",se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_SEL);
        MY_LOG("se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C=%d \n",se.edge_ctrl_1.bits.SEEE_SE_VERT_EDGE_GAIN_C);
    }
#if (MTKCAM_DRV_ISP_VERSION==30)
    se.out_edge_ctrl.val=0;
#else
    se.edge_ctrl.val=0;
#endif //MTKCAM_DRV_ISP_VERSION==30

    se.y_ctrl.val=0;
    se.edge_ctrl_1.val=0;
    se.edge_ctrl_2.val=0;
    se.edge_ctrl_3.val=0;
    se.special_ctrl.val=0;
    se.core_ctrl_1.val=0;
    se.core_ctrl_2.val=0;
#endif //DIP_REG_NEEDED


    m_IspUsrSelectLevel.eIdx_Edge = MTK_CONTROL_ISP_EDGE_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Hue = MTK_CONTROL_ISP_HUE_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Sat = MTK_CONTROL_ISP_SATURATION_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Bright = MTK_CONTROL_ISP_BRIGHTNESS_MIDDLE;
    m_IspUsrSelectLevel.eIdx_Contrast = MTK_CONTROL_ISP_CONTRAST_MIDDLE;

    //memset(&se, 0, sizeof(ISP_NVRAM_SE_T)); //Test case 140821
    //Fix effect unstable in WhiteBoard/BlackBoard/Posterize Effect    140821 <<
    m_pIspTuningCustom->userSetting_EFFECT(m_rIspCamInfov3, m_eIdx_Effect, m_IspUsrSelectLevel, g2c, g2c_shade, se, ggm);
    m_pIspTuningCustom->userSetting_EFFECT_GGM_JNI(m_rIspCamInfov3, m_eIdx_Effect, ggm);

    //G2G
#if (MTKCAM_DRV_ISP_VERSION==30)
    ppIspPhyRegLocal->CAM_CTL_EN_P2.Bits.G2G_EN=1;//0;// 1;
#else
    ppIspPhyRegLocal->DIP_X_CTL_RGB_EN.Bits.G2G_EN=1;//0;// 1;
#endif
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_1).Raw = 0x00000200;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_2).Raw = 0x000002CE;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_3).Raw = 0x1F500200;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_4).Raw = 0x00001E92;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_5).Raw = 0x038B0200;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CNV_6).Raw = 0x00000000;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2G_CTRL).Raw = 0x00000009;
        //G2C
#if (MTKCAM_DRV_ISP_VERSION==30)
    ppIspPhyRegLocal->CAM_CTL_EN_P2.Bits.G2C_EN=1;
#else
    ppIspPhyRegLocal->DIP_X_CTL_YUV_EN.Bits.G2C_EN=1;
#endif
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_0A).Raw=g2c.conv_0a.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_0B).Raw=g2c.conv_0b.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_1A).Raw=g2c.conv_1a.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_1B).Raw=g2c.conv_1b.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_2A).Raw=g2c.conv_2a.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_CONV_2B).Raw=g2c.conv_2b.val;


/*
    ppIspPhyRegLocal->DIP_X_G2C_SHADE_CON_1.Raw=g2c_shade.con_1.val;
    ppIspPhyRegLocal->DIP_X_G2C_SHADE_CON_2.Raw=g2c_shade.con_2.val;
    ppIspPhyRegLocal->DIP_X_G2C_SHADE_CON_3.Raw=g2c_shade.con_3.val;
    ppIspPhyRegLocal->DIP_X_G2C_SHADE_TAR.Raw=g2c_shade.tar.val;
    ppIspPhyRegLocal->DIP_X_G2C_SHADE_SP.Raw=g2c_shade.sp.val;
*/
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_1).Raw=g2c_shade.set[0];
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_2).Raw=g2c_shade.set[1];
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_3).Raw=g2c_shade.set[2];
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_TAR).Raw=0;// for TC's requests g2c_shade.tar.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_SP).Raw=g2c_shade.set[4];
    if(bRegDumpEnable)//if(bAddrDumpEnable)
    {
        if(m_eIdx_Effect==MTK_CONTROL_EFFECT_MODE_LOFI)
        {
            MY_LOG("LOFI eIdx_Effect(%d)SET - DIP_X_G2C_SHADE_CON_1.Raw 0x%8x",eIdx_Effect,ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_1).Raw);
            MY_LOG("LOFI  eIdx_Effect(%d)SET - DIP_X_G2C_SHADE_CON_2.Raw 0x%8x",eIdx_Effect,ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_2).Raw);
            MY_LOG("LOFI  eIdx_Effect(%d)SET - DIP_X_G2C_SHADE_CON_3.Raw 0x%8x",eIdx_Effect,ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_CON_3).Raw);
            MY_LOG("LOFI  eIdx_Effect(%d)SET - DIP_X_G2C_SHADE_TAR.Raw 0x%8x",eIdx_Effect,ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_TAR).Raw);
            MY_LOG("LOFI  eIdx_Effect(%d)SET - DIP_X_G2C_SHADE_SP.Raw 0x%8x",eIdx_Effect,ppIspPhyRegLocal->ISP_PRE_FIX(G2C_SHADE_SP).Raw);
        }
    }
    //SEEE
#if (MTKCAM_DRV_ISP_VERSION==30)
    ppIspPhyRegLocal->CAM_CTL_EN_P2.Bits.SEEE_EN=1;
#else
    ppIspPhyRegLocal->DIP_X_CTL_YUV_EN.Bits.SEEE_EN=1;
#endif
#if DIP_REG_NEEDED // defined in Android.mk
    // isp5.0
    ppIspPhyRegLocal->DIP_X_SEEE_TOP_CTRL.Raw =  se.top_ctrl.val;
    ppIspPhyRegLocal->DIP_X_SEEE_SE_Y_SPECL_CTRL.Raw =  se.se_y_specl_ctrl.val;
    ppIspPhyRegLocal->DIP_X_SEEE_SE_EDGE_CTRL_1.Raw =  se.se_edge_ctrl_1.val;
    ppIspPhyRegLocal->DIP_X_SEEE_SE_EDGE_CTRL_2.Raw =  se.se_edge_ctrl_2.val;
    ppIspPhyRegLocal->DIP_X_SEEE_SE_CORE_CTRL_1.Raw =  se.se_core_ctrl_1.val;
    ppIspPhyRegLocal->DIP_X_SEEE_SE_CORE_CTRL_2.Raw =  se.se_core_ctrl_2.val;
#else
#if (MTKCAM_DRV_ISP_VERSION==30)
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_OUT_EDGE_CTRL).Raw =  se.out_edge_ctrl.val;
#else
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_OUT_EDGE_CTRL).Raw =  se.edge_ctrl.val;
#endif //MTKCAM_DRV_ISP_VERSION==30
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_Y_CTRL).Raw =  se.y_ctrl.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_EDGE_CTRL_1).Raw =  se.edge_ctrl_1.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_EDGE_CTRL_2).Raw =  se.edge_ctrl_2.val;
#endif //DIP_REG_NEEDED

#if DIP_REG_NEEDED // defined in Android.mk
    // isp5.0
    // this part is included in the above DIP_REG_NEEDED define
#else
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_EDGE_CTRL_3).Raw =  se.edge_ctrl_3.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_SPECL_CTRL).Raw =  se.special_ctrl.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_CORE_CTRL_1).Raw =  se.core_ctrl_1.val;
    ppIspPhyRegLocal->ISP_PRE_FIX(SEEE_SE_CORE_CTRL_2).Raw =  se.core_ctrl_2.val;
#endif


    switch ((EIndex_Effect_T)eIdx_Effect)
    {
        case MTK_CONTROL_EFFECT_MODE_NASHVILLE:
        case MTK_CONTROL_EFFECT_MODE_HEFE:
        case MTK_CONTROL_EFFECT_MODE_VALENCIA :
        case MTK_CONTROL_EFFECT_MODE_XPROII :
        case MTK_CONTROL_EFFECT_MODE_LOFI :
        case MTK_CONTROL_EFFECT_MODE_SIERRA :
        case MTK_CONTROL_EFFECT_MODE_KELVIN :
        case MTK_CONTROL_EFFECT_MODE_WALDEN :
        case MTK_CONTROL_EFFECT_MODE_F1977 :
        	//GGM_RB
#if (MTKCAM_DRV_ISP_VERSION==30)
            ppIspPhyRegLocal->CAM_CTL_EN_P2.Bits.GGM_EN=1;
#else
            ppIspPhyRegLocal->DIP_X_CTL_RGB_EN.Bits.GGM_EN=1;
#endif
            for(int idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {
#if DIP_REG_NEEDED // defined in Android.mk
    // isp5.0
                ppIspPhyRegLocal->DIP_X_GGM_LUT[idx].Raw=LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][idx];
#else
                ppIspPhyRegLocal->ISP_PRE_FIX(GGM_LUT_RB)[idx].Raw=LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_BR][idx];
                ppIspPhyRegLocal->ISP_PRE_FIX(GGM_LUT_G)[idx].Raw=LomoFilterGGMJni[(MUINT32)eIdx_Effect-(MUINT32)MTK_CONTROL_EFFECT_MODE_NASHVILLE+1][CUSTOM_LOMO_GGM_CHANNEL_G][idx];
#endif
            }

        break;
        case MTK_CONTROL_EFFECT_MODE_MONO:
        case MTK_CONTROL_EFFECT_MODE_NEGATIVE:
        case MTK_CONTROL_EFFECT_MODE_SOLARIZE:
        case MTK_CONTROL_EFFECT_MODE_SEPIA:
        case MTK_CONTROL_EFFECT_MODE_POSTERIZE:
        case MTK_CONTROL_EFFECT_MODE_WHITEBOARD:
        case MTK_CONTROL_EFFECT_MODE_BLACKBOARD:
        case MTK_CONTROL_EFFECT_MODE_AQUA:  //,
        case MTK_CONTROL_EFFECT_MODE_OFF:
        default:
            //GGM_G
#if (MTKCAM_DRV_ISP_VERSION==30)
            ppIspPhyRegLocal->CAM_CTL_EN_P2.Bits.GGM_EN=1;
#else
            ppIspPhyRegLocal->DIP_X_CTL_RGB_EN.Bits.GGM_EN=1;
#endif
            for(int idx=0; idx<CUSTOM_LOMO_GGM_GAIN_NUM;idx++)
            {
#if DIP_REG_NEEDED // defined in Android.mk
    // isp5.0
                ppIspPhyRegLocal->DIP_X_GGM_LUT[idx].Raw=LomoFilterGGMJni[0][idx];
#else
                ppIspPhyRegLocal->ISP_PRE_FIX(GGM_LUT_RB)[idx].Raw=LomoFilterGGMJni[0][CUSTOM_LOMO_GGM_CHANNEL_BR][idx];
                ppIspPhyRegLocal->ISP_PRE_FIX(GGM_LUT_G)[idx].Raw=LomoFilterGGMJni[0][CUSTOM_LOMO_GGM_CHANNEL_G][idx];
#endif
            }
            break;
    }
    if((EIndex_Effect_T)eIdx_Effect==MTK_CONTROL_EFFECT_MODE_LOFI)
    {
#if (MTKCAM_DRV_ISP_VERSION==30)
        MY_LOG("idx:%d CAM_CTL_EN_P2.Raw 0x%x",eIdx_Effect,ppIspPhyRegLocal->CAM_CTL_EN_P2.Raw);
#else
        MY_LOG("idx:%d DIP_X_CTL_RGB_EN.Raw 0x%x",eIdx_Effect,ppIspPhyRegLocal->DIP_X_CTL_RGB_EN.Raw);
        MY_LOG("idx:%d DIP_X_CTL_YUV_EN.Raw 0x%x",eIdx_Effect,ppIspPhyRegLocal->DIP_X_CTL_YUV_EN.Raw);
#endif
    }

    if ((ret=lomoApplyTuningSettingExt(ppIspPhyReg)) != 0)
    {
        LOG_ERR("lomoApplyTuningSettingExt failed: ret: %d", ret);
    }

    return ret;
}

MVOID LocalLomoDequeCallBack(QParams& rParams)
{



    MUINT8* mu32VaAddr= NULL; //
    MUINT8* mu32FlsuhVaAddr= NULL; //
    MUINT32 mu32OutPortSize= 0; //
    LomoTimer ldequeTmr("lomo->deque_CB",rParams.mvFrameParams[0].mvOut.size(),bDebugLomoTimerEnabled);
    mu32OutPortSize = rParams.mvFrameParams[0].mvOut.size();
    LOMO_TRACE_CALL();
    for(unsigned int j=0;j<mu32OutPortSize;j++)
    {
        mu32VaAddr = (MUINT8*)rParams.mvFrameParams[0].mvOut[j].mBuffer->getBufVA(0);
        if(j==0)
        {
            mu32FlsuhVaAddr =mu32VaAddr;
        }
    }
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeapBuffIdx[Number]==mu32FlsuhVaAddr)//ppDstImgMVA[0])
        {
            //LomoTimer lDstFlushTmr("DstFlush_CB",Number,bDebugLomoTimerEnabled);
            //pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
            //lDstFlushTmr.printTime();
            pBufDstHeapBuffMarked[Number]=MTRUE;
            //singleton.LomoDequeCallBackDone();
            singleton.LomoDequeTrigger();
            break;
        }
        else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            LOG_ERR("deque buffer no matching! VA : 0x%p",mu32VaAddr);
        }
    }
    ldequeTmr.printTime();
//    MY_LOG("mpIStream->enque(params); callBack<<");
    return ;
}



MINT32 LomoHalJniImp::LomoImageEnque(MUINT8** ppDstImgVA, MINT32 ppEffectIdx)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    void           *pBuffer[3];
    void           *pBufferMva[3];
    MUINT32       size[3];
    DpRect         tmpRect;
    MUINT i,count;
    QParams params;
    FrameParams frameparams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MBOOL deqRet=0; // 1   ok, 0 NG
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG
    MCrpRsInfo crop2; //Add from Kenny's request in Everest
    MINT32 EffectTuningBuffIdx=0;

    NSCam::NSIoPipe::Input minput;
    NSCam::NSIoPipe::Output moutput;
    LOMO_TRACE_CALL();
    LomoTimer lLomoTmr("LomoTmr",ppEffectIdx,bDebugLomoTimerEnabled);
    LomoTimer lenqueTmr("lomo->enque",ppEffectIdx,bDebugLomoTimerEnabled);

    if(ppEffectIdx!=(-1))//0xFFFFFFFF)
   {
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeapBuffIdx[Number]==ppDstImgVA[0])
        {
            moutput.mBuffer = const_cast<IImageBuffer*>(pBufDstHeap[Number].get());
            //MY_LOG("pBufDstHeapBuffIdx[%d]= ppDstImgVA[0] -0x%8x",Number,ppDstImgVA[0]);
            break;
        }
        else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            NoMatchingVA= MTRUE;
            LOG_WRN("Buffer #%d is NOT matching 0x%p!",Number,ppDstImgVA[0]);
            return LOMOHALJNI_WRONG_PARAM;
        }
    }

    frameparams.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT/*ENormalStreamTag_Normal*/;

    frameparams.mvIn.clear();
    minput.mBuffer = const_cast<IImageBuffer*>(pBufSrc.get());
    minput.mPortID.inout=0; //in
    minput.mPortID.index=NSImageio::NSIspio::EPortIndex_IMGI; //new from Kenny 151022
    minput.mPortID.type=NSCam::NSIoPipe::EPortType_Memory;
    frameparams.mvIn.push_back(minput);

    crop2.mFrameGroup=0;
    crop2.mGroupID=2;
    crop2.mCropRect.p_fractional.x=0;
    crop2.mCropRect.p_fractional.y=0;
    crop2.mCropRect.p_integral.x=0;
    crop2.mCropRect.p_integral.y=0;
    crop2.mCropRect.s.w=gu32ppDstImgWidth;
    crop2.mCropRect.s.h=gu32ppDstImgHeight;
    crop2.mResizeDst.w=gu32ppDstImgWidth;
    crop2.mResizeDst.h=gu32ppDstImgHeight;
    frameparams.mvCropRsInfo.push_back(crop2);

    // Setup Dst Image buffer address
    frameparams.mvOut.clear();

    moutput.mPortID.inout=1; //out
    moutput.mPortID.index=NSImageio::NSIspio::EPortIndex_WDMAO; //new from Kenny 151022//SL NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG3O;//SL//NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG2O;
    moutput.mPortID.type=NSCam::NSIoPipe::EPortType_Memory;
    frameparams.mvOut.push_back(moutput);

    EffectTuningBuffIdx= ppEffectIdx;
//    MY_LOG("pTuningQueBufAddrbymalloc[EffectTuningBuffIdx-%d]=0x%8x",EffectTuningBuffIdx,pTuningQueBufAddrbymalloc[EffectTuningBuffIdx]);
//    memset((MUINT8*)(pTuningQueBufAddrbymalloc[EffectTuningBuffIdx]), 0x0,sizeof(dip_x_reg_t));
//    LomoApplyTuningSetting(ppEffectIdx/*1*/, (void *)pTuningQueBufAddrbymalloc[EffectTuningBuffIdx]);

    frameparams.mTuningData = (reinterpret_cast<MVOID*>(pTuningQueBufAddrbymalloc[EffectTuningBuffIdx]));

    params.mvFrameParams.push_back(frameparams);
#ifdef LOMO_P2_DEQUE_CALLBACK
    params.mpfnCallback  = LocalLomoDequeCallBack;
#endif

    lenqueTmr.start("lomo->enque",ppEffectIdx,bDebugLomoTimerEnabled);
//    MY_LOG("mpIStream->enque(params);>>");
    LOMO_TRACE_BEGIN("lomo_enque");
    if (mpIStream->enque(params) == MFALSE)
    {
        MY_LOG("!!warning: mpIStream->enque() failed");
    }
    LOMO_TRACE_END();
//    MY_LOG("mpIStream->enque(params);<<");
    lenqueTmr.printTime();
#ifndef LOMO_P2_DEQUE_CALLBACK
	    LomoDequeTrigger();
#endif
  }
    return err;
}


MINT32 LomoHalJniImp::LomoImageDeque(MUINT8** ppDstImgMVA, MINT32 ppEffectIdx)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    QParams dequeParams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MBOOL deqRet=0; // 1   ok, 0 NG
    NSCam::NSIoPipe::Output moutput;
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG
    MUINT8* mu32VaAddr= NULL; //
    MUINT8* mu32FlsuhVaAddr= NULL; //
    static MUINT32 frameCntEffectIdx=0xFFFF;

    if(brawdumpEnable==0)
    {
        frameCntEffectIdx=0xFFFF;
    }
    if(ppEffectIdx!=(-1))//0xFFFFFFFF)
    {
        LomoTimer ldequeTmr("lomo->deque_AP",ppEffectIdx,bDebugLomoTimerEnabled);

        ///deque
        ldequeTmr.start("lomo->deque_AP",ppEffectIdx,bDebugLomoTimerEnabled);
        LOMO_TRACE_CALL();
        ::sem_wait(&mDequeDoneSem); // wait here until someone use sem_post() to wake this semaphore up
///
        ldequeTmr.printTime();
        for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
        {
                if(pBufDstHeapBuffIdx[Number]==ppDstImgMVA[0])
                {
//                    MY_LOG("LomoImage_Deque buff#%d",Number);
                    mu32FlsuhVaAddr = ppDstImgMVA[0];
                    if(brawdumpEnable)
                    {
                        static MUINT32 frameCnt=0;
                        char fileName[64];
                        int fileSize=0;
                        MUINT32 shiftIdx = 0;
                        if(ppEffectIdx!=(-1))
                        {
                            shiftIdx = (MUINT32)ppEffectIdx;
                        }
                        if((lu32BuffeffectIdx>>shiftIdx)&0x01)
                        {
                            if(frameCntEffectIdx==shiftIdx)
                            {
                                MY_LOG(" lu32BuffIdx=0x%8x (%d)",lu32BuffeffectIdx,lu32BuffeffectIdx);
                                frameCnt++;
                                sprintf(fileName, "/sdcard/lomo/lomo%4dx%4d_%2d_%5d_YV12_s.bin", gu32ppDstImgWidth,gu32ppDstImgHeight,shiftIdx,frameCnt);
                            }
                            else
                            {
                                sprintf(fileName, "/sdcard/lomo/lomo%4dx%4d_%2d_%5d_YV12.bin", gu32ppDstImgWidth,gu32ppDstImgHeight,shiftIdx,frameCnt);
                            }
                            FILE *fp = fopen(fileName, "wb");
                            for(MUINT8 count=0;count<3;count++)
                            {
                                fileSize +=pBufDstHeap[Number].get() ->getBufSizeInBytes(count);
                                //MY_LOG("fileSize[%d] = %d", count, fileSize);
                            }
                            fileSize = gu32ppDstImgWidth*gu32ppDstImgHeight*3/2;
                            if (NULL == fp)
                            {
                                LOG_ERR("fail to open file to save img: %s", fileName);
                                MINT32 error = mkdir("/sdcard/lomo", S_IRWXU | S_IRWXG | S_IRWXO);
                                LOG_ERR("error = %d", error);
                            }
                            else
                            {
                                fwrite(reinterpret_cast<void *>(mu32FlsuhVaAddr), 1, fileSize, fp);
                                fclose(fp);
                            }
                            if(frameCntEffectIdx==0xFFFF)
                            {
                                frameCntEffectIdx=shiftIdx;
                                MY_LOG("frameCntEffectIdx = %d\n", frameCntEffectIdx);
                            }
                        }
                    }
                    break;
                }
                else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
                {
                    NoMatchingVA= MTRUE;
                    LOG_WRN("deque buffer no matching! VA : 0x%p",mu32VaAddr);
                    return LOMOHALJNI_WRONG_PARAM;
                }
        }
    }

    return err;
}

MINT32 LomoHalJniImp::FreeLomoDstImage(void)
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeap[Number].get()!=NULL)
        {
            pBufDstHeap[Number]->unlockBuf(LOG_TAG);
            pBufDstHeap[Number]->decStrong(pBufDstHeap[Number].get());
            pBufDstHeap[Number] = NULL;
            pBufDstHeapBuffIdx[Number]=0;
        }
    }
    return err;
}

MUINT32 LomoHalJniImp::ColorEffectSetting(MUINT32 /*caseNum*/)
{
    return 0;
}


#include <mtkcam/def/PriorityDefs.h>

/*******************************************************************************
*
********************************************************************************/
MVOID *LomoHalJniImp::LomoDequeThreadLoop(MVOID *arg)
{
    LomoHalJniImp *_this = reinterpret_cast<LomoHalJniImp *>(arg);
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;

    //====== Change Thread Setting ======
    _this->ChangeThreadSetting();

    //====== Main Loop ======
    LOMO_STATE_ENUM eState = _this->GetLomoState();

    while(eState != LOMO_STATE_UNINIT)
    {
        ::sem_wait(&_this->mDequeSem); // wait here until someone use sem_post() to wake this semaphore up
        eState = _this->GetLomoState();
        switch(eState)
        {
            case LOMO_STATE_ALIVE:
                    err = _this->LomoDequeBuffer();
                    if(err != LOMOHALJNI_NO_ERROR)
                    {
                        LOG_ERR("Deque LOMO fail(%d)",err);
                    }
                break;
            case LOMO_STATE_UNINIT :
                MY_LOG("LOMO_STATE_UNINIT");
                break;
            default:
                LOG_ERR("State Error(%d)",eState);
        }

        eState = _this->GetLomoState();
    }

    ::sem_post(&_this->mDequeSemEnd);
    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::SetLomoState(const LOMO_STATE_ENUM &aState)
{
    MY_LOG("aState(%d),mState(%d)",aState,mState);
    mState = aState;
}


/*******************************************************************************
*
********************************************************************************/
LOMO_STATE_ENUM LomoHalJniImp::GetLomoState()
{
    return mState;
}

#define PR_SET_NAME    15
/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::ChangeThreadSetting()
{
    //> set name

    ::prctl(PR_SET_NAME,"LomoDeqThread", 0, 0, 0);

    //> set policy/priority
    const MINT32 policy   = SCHED_NORMAL;
    const MINT32 priority = NICE_CAMERA_LOMO;

    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    //>  get policy/priority
    ::sched_getparam(0, &sched_p);

    MY_LOG("policy:(expect, result)=(%d,%d), priority:(expect, result)=(%d, %d)", policy,
                                                                                    ::sched_getscheduler(0),
                                                                                    priority,
                                                                                    sched_p.sched_priority);
}


/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::LomoDequeTrigger()//(QParams& rParams)
{
//    MY_LOG("P2 LomoDequeTrigger()");
    ::sem_post(&mDequeSem);
    #if 0
    {
        int mu32Val;
        ::sem_getvalue(&mDequeSem, &mu32Val);
        MY_LOG("mDequeSem %d",mu32Val);
    }
    #endif
    return;

}
/*******************************************************************************
*
********************************************************************************/
MVOID LomoHalJniImp::LomoDequeCallBackDone()//(QParams& rParams)
{
//    MY_LOG("P2 LomoDequeCallBackDone()");
    ::sem_post(&mDequeDoneSem);
    return;

}



/*******************************************************************************
*
********************************************************************************/
LOMOHALJNI_RETURN_TYPE LomoHalJniImp::LomoDequeBuffer()
{
    LOMOHALJNI_RETURN_TYPE err = LOMOHALJNI_NO_ERROR;
    QParams dequeParams;
    MINT64 timeout=LOMO_EFFECT_TIME_OUT;  //5s,unit is nsec
    MUINT32 magicNum=0;
    MBOOL deqRet=0; // 1   ok, 0 NG
    MBOOL NoMatchingVA= MFALSE; // 1   ok, 0 NG
    MUINT8* mu32VaAddr= NULL; //
    MUINT8* mu32MVaAddr= NULL; //
    MUINT8* mu32FlsuhVaAddr= NULL; //
    MUINT32 mu32OutPortSize= 0; //
    NSCam::NSIoPipe::Output moutput;

    LOMO_TRACE_CALL();

#ifndef LOMO_P2_DEQUE_CALLBACK
        LOG_ERR("Only LOMO_P2_DEQUE_CALLBACK, now!");
#endif
    ///deque
    for(MUINT32 Number=0; Number<LOMO_EFFECT_DST_BUFF_NUM; Number++)
    {
        if(pBufDstHeapBuffMarked[Number]==MTRUE)//ppDstImgMVA[0])
        {
            LomoTimer lDstFlushTmr("DstFlush_THR",magicNum,bDebugLomoTimerEnabled);
            lDstFlushTmr.start("DstFlush_THR",magicNum,bDebugLomoTimerEnabled);
            LOMO_TRACE_BEGIN("lomo_flush");
            pBufDstHeap[Number].get()->syncCache(eCACHECTRL_INVALID);
            LOMO_TRACE_END();
            lDstFlushTmr.printTime();
            ::sem_post(&mDequeDoneSem);
            pBufDstHeapBuffMarked[Number]=MFALSE;
            break;
        }
        else if(Number==(LOMO_EFFECT_DST_BUFF_NUM-1))
        {
            NoMatchingVA= MTRUE;
            LOG_ERR("deque buffer no matching! VA : 0x%p",mu32VaAddr);
            return LOMOHALJNI_WRONG_PARAM;
        }
    }
    return err;
}


#ifdef MTK_METADATA_TAG_PATH
///mtkcam/common/include/metadata/client/mtk_metadata_tag.h:
// MTK_CONTROL_EFFECT_MODE
typedef enum mtk_camera_metadata_enum_android_control_effect_mode {
    MTK_CONTROL_EFFECT_MODE_OFF,
    MTK_CONTROL_EFFECT_MODE_MONO,
    MTK_CONTROL_EFFECT_MODE_NEGATIVE,
    MTK_CONTROL_EFFECT_MODE_SOLARIZE,
    MTK_CONTROL_EFFECT_MODE_SEPIA,
    MTK_CONTROL_EFFECT_MODE_POSTERIZE,
    MTK_CONTROL_EFFECT_MODE_WHITEBOARD,
    MTK_CONTROL_EFFECT_MODE_BLACKBOARD,
    MTK_CONTROL_EFFECT_MODE_AQUA,  //,//8
    MTK_CONTROL_EFFECT_MODE_SEPIAGREEN,
    MTK_CONTROL_EFFECT_MODE_SEPIABLUE,
    MTK_CONTROL_EFFECT_MODE_NASHVILLE ,        //LOMO //11
    MTK_CONTROL_EFFECT_MODE_HEFE ,
    MTK_CONTROL_EFFECT_MODE_VALENCIA ,
    MTK_CONTROL_EFFECT_MODE_XPROII ,
    MTK_CONTROL_EFFECT_MODE_LOFI ,
    MTK_CONTROL_EFFECT_MODE_SIERRA ,
    MTK_CONTROL_EFFECT_MODE_KELVIN ,
    MTK_CONTROL_EFFECT_MODE_WALDEN,
    MTK_CONTROL_EFFECT_MODE_F1977 ,        //LOMO
    MTK_CONTROL_EFFECT_MODE_NUM
} mtk_camera_metadata_enum_android_control_effect_mode_t;

#endif

