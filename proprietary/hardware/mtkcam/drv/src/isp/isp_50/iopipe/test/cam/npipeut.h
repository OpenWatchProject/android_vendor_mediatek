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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

#include <queue>
#include <semaphore.h>
#include <pthread.h>

#include <utils/Mutex.h>
#include <utils/StrongPointer.h>
#include <utils/threads.h>

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h>
#include <mtkcam/def/PriorityDefs.h>

using namespace NSCam;
using namespace NSCam::Utils;
using namespace android;
using namespace std;

#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/IStatisticPipe.h>
#include <mtkcam/drv/iopipe/CamIO/ICamsvStatisticPipe.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <imageio/Cam_Notify_datatype.h>
#include <tuning_mgr.h>
#include <mtkcam/drv/IHwSyncDrv.h>
#include "cam_crop.h"              //for crop test

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

#include <imem_drv.h>

#include "../CamIO/FakeSensor.h"
#include "../CamIO/FakeIspClk.h"

//#define HWSYNC_TEST // Open this define to test hwsync
//#define HWSYNC_USE_AE_THREAD // Open this define to test hwsync via AE thread simulation
#define HWSYNC_AE_CYCLE_PERIOD  (3)

#define TEST_AE_HIGHSPEED       (0)

#if (TEST_AE_HIGHSPEED == 1)
#include <ispio_stddef.h>
#endif

#if 1
#define MY_LOGV(fmt, arg...) \
    do { \
        struct timespec ts;\
        clock_gettime(CLOCK_REALTIME, &ts);\
        printf("TS' [%d][%d.%03d][%s] " fmt"\n", (MUINT32)gettid(), (MUINT32)(ts.tv_sec&1023),\
                (MUINT32)(ts.tv_nsec/1000000), __FUNCTION__, ##arg);\
    } while (0)
#define MY_LOGD(fmt, arg...)    MY_LOGV(fmt, ##arg)
#define MY_LOGI(fmt, arg...)    MY_LOGV(fmt, ##arg)
#define MY_LOGW(fmt, arg...)    MY_LOGV(fmt, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s:%d][%s] \033[1;31m" fmt"\033[0m\n", __FILE__, __LINE__, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("TS' [%s] " fmt"\n", __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("TS' [%s:%d][%s] " fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##arg)
#endif


/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG

#undef  __func__
#define __func__    __FUNCTION__


#ifndef USING_MTK_LDVT
#define LOG_VRB(fmt, arg...)       CAM_ULOGV(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_DBG(fmt, arg...)       CAM_ULOGD(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_INF(fmt, arg...)       CAM_ULOGI(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_WRN(fmt, arg...)       CAM_ULOGW(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, DBG_LOG_TAG "[%s] WARNING: " fmt , __func__, ##arg)
#define LOG_ERR(fmt, arg...)       CAM_ULOGE(NSCam::Utils::ULog::MOD_DRV_NORMAL_PIPE, DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt , __FILE__, __func__, __LINE__, ##arg)

#else   // LDVT
    #include "uvvf.h"

#define LOG_MSG(fmt, arg...)    VV_MSG("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_DBG(fmt, arg...)    VV_MSG("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_INF(fmt, arg...)    VV_MSG("[%s]"fmt, __FUNCTION__, ##arg)
#define LOG_WRN(fmt, arg...)    VV_MSG("[%s]Warning(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)
#define LOG_ERR(fmt, arg...)    VV_MSG("[%s]Err(%5d):"fmt, __FUNCTION__, __LINE__, ##arg)

#endif

#define ENQUE_BURST_CNT     (3)

#define SEN_PIX_BITDEPTH    (10)

#define UT_MAIN_SNR         (0x01)
#define UT_SUB_SNR          (0x02)
#define UT_MAIN2_SNR        (0x04)
#define UT_SUB2_SNR         (0x08)
#define UT_MAIN_FKSNR       (0x10)
#define UT_SUB_FKSNR        (0x20)
#define UT_MAIN2_FKSNR      (0x40)
#define UT_SUB2_FKSNR       (0x80)
#define UT_SNR_MASK         (UT_MAIN_SNR|UT_SUB_SNR|UT_MAIN2_SNR|UT_SUB2_SNR)
#define UT_FKSNR_MASK       (UT_MAIN_FKSNR|UT_SUB_FKSNR|UT_MAIN2_FKSNR|UT_SUB2_FKSNR)

#define __RRZO_ENABLE       (0x1)
#define __IMGO_ENABLE       (0x2)
#define __EISO_ENABLE       (0x4)
#define __LCSO_ENABLE       (0x8)
#define __UFEO_ENABLE       (0x10)
#define __RSSO_ENABLE       (0x20)
#define __UFGO_ENABLE       (0x40)
/* keep 0x100 ~ 0x1000 for PortBitDepth
0x 100: rrzo output 12bit (__RRZO_ENABLE <<8)
0x 200: imgo output 12bit (__IMGO_ENABLE <<8)
*/
#define __IMGO_PURE_RAW     (0x20000)

#define __AAO               (0)
#define __AFO               (1)
#define __FLKO              (2)
#define __PDO               (3)
#define __PSO               (4)
#define __MAXDMAO_STT       (5)

#define __AAO_ENABLE        (0x0001)
#define __AFO_ENABLE        (0x0002)
#define __FLKO_ENABLE       (0x0004)
#define __PDO_ENABLE        (0x0008)
#define __PSO_ENABLE        (0x0010)
#define __CAMSV_IMGO_ENABLE (0x2000) /* sams as enum eDMAO */

#define _TG_1_              (0)
#define _TG_2_              (1)
#define _MAXTG_             (2)

#define RRZO_4K2K_WIDTH     (3840)
#define RRZO_4K2K_HEIGHT    (2160)
#define EISO_SIZE           (256)

typedef void* (*ThreadR_t)(void*);

class NPipeUT;

enum {
    __RRZO = 0,
    __IMGO,
    __EISO,
    __LCSO,
    __UFEO,
    __RSSO,
    __UFGO,
    __UFEO_META,
    __UFGO_META,
    __MAXDMAO
};

enum {
    CMD_IDX_SNRDEVID = 1,
    CMD_IDX_SNRSCEN = 2,
    CMD_IDX_ENABLEPORTS,
    CMD_IDX_CROPSEL,
    CMD_IDX_STTPORTS,
    CMD_IDX_BURSTNUM,
    CMD_IDX_FRAMENUM,
    CMD_IDX_STOPFLOW,
    CMD_IDX_SWITCHCAM,
    CMD_IDX_PIXELMODE,
    CMD_IDX_CMDS_FILED,
    CMD_IDX_SUSPEND,
    CMD_IDX_N3DEN,
    CMD_IDX_FPS_CTRL,
    CMD_IDX_RANDOM_TEST,
    CMD_IDX_DATA_PATTERN,
    CMD_IDX_DTWINEN,
    CMD_IDX_SCALESEL,
    CMD_IDX_SECURECAM,
    CMD_IDX_MAX
};

enum SWITCH_CAM_ENUM{
    ONE_SENSOR = 0,
    SWITCH_AFTER_UNINIT,
    TWO_SENSOR,
    NONSTOP_DYNAMIC_TWIN,
    NONSTOP_DYNAMIC_TWIN_BY_SUSPEND,
    NONSTOP_RANDOM_SWITCH_BY_SUSPEND,
    IQ_LEVEL_TEST,
    SWITCH_CAM_ENUM_MAX,
};
enum SUSPEND_FLOW{
    ONE_TIME_SUSPEND = 0,
    NONSTOP_SUSPEND,
    SUSPEND_STOP,
    SUSPEND_FLOW_MAX,
};

enum SWITCH_CAM_STATE_ENUM{
    SWITCH_CASE_1 = 0, /* TG1 suspend -> TG2 start twin */
    SWITCH_CASE_2,     /* TG1 suspend, TG2 suspend */
    SWITCH_CASE_3,     /* TG1 stop twin -> TG2 resume */
    SWITCH_CASE_4,     /* TG1 suspend -> TG2 start twin -> TG2 resume */
    SWITCH_CASE_5,     /* TG1 suspend */
    SWITCH_CASE_6,     /* TG1 stop twin -> TG1&TG2 resume */
    SWITCH_CASE_7      /* TG1 start twin -> TG1 resume */
};

enum TUNING_TYPE_ENUM{
    TYPE_SET_TUNING = 0,
    TYPE_TUNING_CB
};

enum SCALE_TYPE_ENUM{
    SCALE_NONE = 0,
    SCALE_DEFAULT,
    SCALE_SPECIFIC
};

struct _arg
{
    int      _argc;
    char**   _argv;
    NPipeUT* Ut_obj = NULL;
    MUINT32  TG_Num = 0;
};

class TuningNotifyImp_EIS : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_EIS(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_EIS() {}
    virtual const char* TuningName() { return "UT_EIS"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    MSize queryEisOutSize();

    NPipeUT *mpNPipeUtObj;
};

class TuningNotifyImp_RSS : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_RSS(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_RSS() {}
    virtual const char* TuningName() { return "UT_RSS"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    MSize queryRssOutSize();

    NPipeUT *mpNPipeUtObj;
};


class TuningNotifyImp_SGG2 : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_SGG2(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_SGG2() {}
    virtual const char* TuningName() { return "UT_SGG2"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    NPipeUT     *mpNPipeUtObj;
};

class TuningNotifyImp_LCS : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_LCS(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_LCS() {}
    virtual const char* TuningName() { return "UT_LCS"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    MSize       queryLcsOutSize(MSize TgSize);
    MSize       queryLcsInSize(MSize TgSize);

    NPipeUT     *mpNPipeUtObj;
};

class TuningNotifyImp_IQ : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_IQ(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_IQ() {}
    virtual const char* TuningName() { return "UT_IQ"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    NPipeUT     *mpNPipeUtObj;
};

class TuningNotifyImp_RRZCB : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_RRZCB(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    TuningNotifyImp_RRZCB() {}
    virtual const char* TuningName() { return "UT_RRZCB"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    NPipeUT     *mpNPipeUtObj;
};

class TuningNotifyImp_TuningCB : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_TuningCB(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    TuningNotifyImp_TuningCB() {}
    virtual const char* TuningName() { return "UT_TuningCB"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    NPipeUT     *mpNPipeUtObj;
};

class TuningNotifyImp_Dump : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_Dump(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {mReg = NULL; mSize= 0;mCnt=0;}
    ~TuningNotifyImp_Dump() {if(mReg)free(mReg);}
    virtual const char* TuningName() { return "UT_DUMP"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    void dumpRegister();

    NPipeUT     *mpNPipeUtObj;
private:
    mutable Mutex mCntLock;
    MUINT32 *mReg;
    MUINT32 mSize;
    MUINT32 mCnt;
};

class TuningNotifyImp_AWB : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_AWB(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    TuningNotifyImp_AWB() {}
    virtual const char* TuningName() { return "UT_AWB"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);
    NPipeUT     *mpNPipeUtObj;
};

class NPipeUT {
    NPipeUT();
    ~NPipeUT(){};
public:
    INormalPipe*             mpNPipe;
    IStatisticPipe*          mpSttPipe;
    ICamsvStatisticPipe*     mpCamsvSttPipe[EPIPE_CAMSV_FEATURE_NUM];
    TuningMgr*               tuningMgr;
    IMemDrv*                 mpImemDrv;
    vector<IImageBuffer*>    mpImgBuffer[__MAXDMAO];
    vector<IMEM_BUF_INFO>    mImemBuf[__MAXDMAO];
    vector<IMEM_BUF_INFO>    mImemSecBuf[__MAXDMAO];
	vector<sp<IImageBufferHeap>> mpHeap[__MAXDMAO];
    MUINT8                   mpSttBuf[__MAXDMAO_STT][512*1024];
    MUINT32                  mpSttBufSize[__MAXDMAO_STT];
    MUINTPTR                 mCamsvBufVa[EPIPE_CAMSV_FEATURE_NUM];
    MUINT32                  mCamsvBufSize[EPIPE_CAMSV_FEATURE_NUM];
    MUINT32                  mCamsvIdx;
    MUINT32                  mCamsvCnt;
    MSize                    mTgSize;
    TuningNotifyImp_EIS      mEisNotify;
    TuningNotifyImp_SGG2     mSgg2Notify;
    TuningNotifyImp_LCS      mLcsNotify;
    TuningNotifyImp_RSS      mRssNotify;
    TuningNotifyImp_Dump     mDumpNotify;
    IMEM_BUF_INFO            m_bpciBuf;
    MUINT32                  mSensorIdx;
    MUINT32                  m_enablePort;
    MUINT32                  mEnableSttPort;
    MUINT32                  mStaticEnqCnt;  /* enque times before start */
    MUINT32                  mEnqCount;      /* buf index for next enque */
    MUINT32                  mDeqCount;
    MUINT32                  mFailCount;
    MUINT32                  mLoopCount;
    MBOOL                    mSuspendTest;
    MUINT32                  mSuspendFrm;
    MUINT32                  mSuspendFlow;
    MUINT32                  mSuspendState;
    MINT32                   mSofUserKey;
    MBOOL                    mSuspendStart;
    MBOOL                    mResumeStart;
    MBOOL                    m_bStop;
    MUINT32                  m_aeCyclePeriod;
    MBOOL                    m_ufeo_en;
    MBOOL                    m_ufgo_en;
    MUINT32                  mFrameNum;
    MUINT32                  mBurstQNum;
    MUINT32                  mPortBufDepth;
    MUINT32                  mMagicNum;
    MBOOL                    m_bEfuse;
    MINT32                   mEfuseIdx;
    MBOOL                    mEnableTgInt;
    MBOOL                    mbCamsvEn;
    MBOOL                    m_uniSwitch;
    TuningNotifyImp_IQ       mIQNotify;
    MBOOL                    mbIQNotify;
    TuningNotifyImp_RRZCB    mRRZCBNotify;
    TuningNotifyImp_TuningCB mTuningCBNotify;
    TuningNotifyImp_AWB      mAwbCBNotify;
    MBOOL                    mbConfig;
    MBOOL                    mbEnque;
    MUINT32                  mSttEnqCnt;
    MUINT32                  mSttDeqCnt;
    Mutex                    mSttCntMutex;
    MBOOL                    mbRanUF;
    MBOOL                    mbRanImgoFmt;
    MUINT32                  mScaleSel;
    MUINT32                  mScaleFactor;
    MUINT32                  mSecEnable;
    union{
        struct{
                MUINT32 bDual   :   31;
                MUINT32 Density :   1;
        }Bits;
        MUINT32 Raw;
    }m_Dualpd;
    int                     mArgc;
    char**                  mArgv;
    CROP_TEST               m_crop;

    ThreadR_t               routineEnq;
    ThreadR_t               routineDeq;
    ThreadR_t               routineHwsync;
    ThreadR_t               routineTg;
    ThreadR_t               routineSttProc[__MAXDMAO_STT];
    ThreadR_t               routineCamsv;

    pthread_t               m_DeqThd;
    pthread_t               m_EnqThd;
    pthread_t               m_HwsyncThd;
    pthread_t               mSttThd[__MAXDMAO_STT];
    pthread_t               m_TgIntThd;
    pthread_t               m_CamsvThd;

    sem_t                   m_semSwitchThd;
    sem_t                   m_semDeqThd;
    sem_t                   m_semEnqThd;
    sem_t                   m_semHwsyncThd;
    sem_t                   m_semSttProc[__MAXDMAO_STT];
    sem_t                   m_semTgIntThd;
    sem_t                   m_semCamsvThd;

public:
    static NPipeUT*         create(void);
    void                    destroy(void);
    void                    startThread(void);
    void                    stopThread(void);
    void                    startHwsyncThread(void);
    void                    stopHwsyncThread(void);
    void                    setTuning(MUINT32* pMagic, TUNING_TYPE_ENUM tuningType = TYPE_SET_TUNING, MVOID* pIn = NULL, MVOID* pOut = NULL);
    void                    setZHdrTuning(MUINT32* pMagic);
};


