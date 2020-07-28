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
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG     "main_camio"

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

#include <ispio_pipe_ports.h>

/* For statistic dmao: eiso, lcso */
#include <mtkcam/drv/iopipe/CamIO/Cam_Notify.h>
#include <imageio/Cam_Notify_datatype.h>
#include <tuning_mgr.h>
#include <mtkcam/drv/IHwSyncDrv.h>
//for PDO test
#include "pic/bpci_tbl_s5k2x8.h"

using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe;
using namespace NSCam;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBuf2File(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


#include <imem_drv.h>

#include "../CamIO/FakeSensor.h"

#undef LOG_TAG
#define LOG_TAG "camiopipetest"

#define NPIPE_TM_KEY    "iopipeUseTM"

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
#define MY_LOGE(fmt, arg...)    printf("TS' [%s:%d][%s] " fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##arg)
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
#define LOG_VRB(fmt, arg...)       CAM_LOGV(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_DBG(fmt, arg...)       CAM_LOGD(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_INF(fmt, arg...)       CAM_LOGI(DBG_LOG_TAG "[%s] " fmt , __func__, ##arg)
#define LOG_WRN(fmt, arg...)       CAM_LOGW(DBG_LOG_TAG "[%s] WARNING: " fmt , __func__, ##arg)
#define LOG_ERR(fmt, arg...)       CAM_LOGE(DBG_LOG_TAG "[%s, %s, line%04d] ERROR: " fmt , __FILE__, __func__, __LINE__, ##arg)


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
#ifndef HWSYNC_TEST
#define RRZ_SCALING_RATIO_N (5) //divisor
#else
#define RRZ_SCALING_RATIO_N (10) //divisor
#endif
#define RRZ_SCALING_RATIO_M (10) //divisor

#define UT_MAIN_SNR         (0x01)
#define UT_SUB_SNR          (0x02)
#define UT_MAIN2_SNR        (0x04)
#define UT_MAIN_FKSNR       (0x10)
#define UT_SUB_FKSNR        (0x20)
#define UT_MAIN2_FKSNR      (0x40)
#define UT_SNR_MASK         (UT_MAIN_SNR|UT_SUB_SNR|UT_MAIN2_SNR)

#define __RRZO              (0)
#define __IMGO              (1)
#define __EISO              (2)
#define __LCSO              (3)
#define __UFEO              (4)
#define __MAXDMAO           (5)

#define __RRZO_ENABLE       (0x1)
#define __IMGO_ENABLE       (0x2)
#define __EISO_ENABLE       (0x4)
#define __LCSO_ENABLE       (0x8)
#define __UFEO_ENABLE       (0x10)
/* keep 0x100 ~ 0x1000 for PortBitDepth
0x 100: rrzo output 12bit (__RRZO_ENABLE <<8)
0x 200: imgo output 12bit (__IMGO_ENABLE <<8)
*/
#define __RRZO_4K2K_ENABLE  (0x10000)//special for RRZO output 4k2k size
#define __IMGO_PURE_RAW     (0x20000)

#define __AAO               (0)
#define __AFO               (1)
#define __FLKO              (2)
#define __PDO               (3)
#define __MAXDMAO_STT       (4)

#define __AAO_ENABLE        (0x0001)
#define __AFO_ENABLE        (0x0002)
#define __FLKO_ENABLE       (0x0004)
#define __PDO_ENABLE        (0x0008)

#define __CAMSV_IMGO_ENABLE (0x2000) /* sams as enum eDMAO */

#define _TG_1_              (0)
#define _TG_2_              (1)
#define _MAXTG_             (2)

#define EISO_SIZE           (256)
#define LCSO_SIZE           (100*1024)

#define RRZO_4K2K_WIDTH     (3840)
#define RRZO_4K2K_HEIGHT    (2160)

//for additional test
#define _test_efuse         (0x00000001)
#define _test_tgint         (0x00000002)

#define bitmap(bit)({\
        MUINT32 _fmt = 0;\
        switch(bit){\
        case 8: _fmt = eImgFmt_BAYER8; break;\
        case 10: _fmt = eImgFmt_BAYER10; break;\
        case 12: _fmt = eImgFmt_BAYER12; break;\
        default: _fmt = eImgFmt_BAYER10; break;\
        }\
        _fmt;})


#define __RRZ_FMT(fmt) ({                                           \
        MUINT32 __fmt;                                              \
        switch (fmt) {                                              \
        case eImgFmt_BAYER8: __fmt = eImgFmt_FG_BAYER8; break;      \
        case eImgFmt_BAYER10: __fmt = eImgFmt_FG_BAYER10; break;    \
        default: __fmt = eImgFmt_FG_BAYER10; break;                 \
        }                                                           \
        __fmt;                                                      \
})

//imgo do nothing, because of imgo have no resizer
#define __SCALE_PIX_SIZE(fmt,size,RatioN,RatioM,b4k2k) ({ \
        MSize __size;                                  \
        if(b4k2k) {                                     \
            __size.w  = RRZO_4K2K_WIDTH;               \
            __size.h  = RRZO_4K2K_HEIGHT;              \
        } else {                                       \
            __size.w  = (size.w*RatioN)/RatioM;            \
            __size.h  = (size.h*RatioN)/RatioM;            \
        }                                               \
        NormalPipe_QueryInfo _query;                    \
        getNormalPipeModule()->query(PORT_RRZO.index,   \
                ENPipeQueryCmd_X_PIX,                   \
                (EImageFormat)__RRZ_FMT(fmt),           \
                __size.w, _query);                      \
        __size.w = _query.x_pix;                        \
        __size;                                         \
})

enum {
    CMD_IDX_SNRDEVID = 1,
    CMD_IDX_SNRSCEN = 2,
    CMD_IDX_SNRFPS = 3,
    CMD_IDX_ENABLEPORTS = 4,
    CMD_IDX_CROPSEL = 5,
    CMD_IDX_STTPORTS = 6,
    CMD_IDX_BURSTNUM = 7,
    CMD_IDX_FRAMENUM = 8,
    CMD_IDX_STOPFLOW = 9,
    CMD_IDX_SWITCHCAM = 10,
    CMD_IDX_PIXELMODE = 11,
    CMD_IDX_ADDTIONAL   = 12,
    CMD_IDX_MAX
};

typedef struct HwPortConfig {
    PortID          mPortID;
    EImageFormat    mFmt;
    MSize           mSize;
    MRect           mCrop;
    MBOOL           mPureRaw;
    MUINT32         mStrideInByte[3];
} HwPortConfig_t;

#define BD_SEG_NUM (3+1) //13M, 16M, 20M, no limitration
struct efMappingTable
{
    int             TG_grabW;
    int             TG_grabH;
    const char*     resultStr;
};
efMappingTable efTable[BD_SEG_NUM]//TBD
{
    {5376, 4032,  "No limitation"},
    {5336, 4002,  "Bonding at 20M"},
    {5336, 3000,  "Bonding at 16M"},
    {4224, 3168,  "Bonding at 13M"},
};


/******************************************************************************
 *
 ******************************************************************************/
static INormalPipeModule* getNormalPipeModule()
{
    static auto pModule = INormalPipeModule::get();
    if  ( ! pModule ) {
        MY_LOGE("INormalPipeModule::get() fail");
    }
    return pModule;
}

static MUINT32 selectNormalPipeVersion(MUINT32 sensorIdx)
{
    MUINT32 selectedVersion = 0;
    auto pModule = getNormalPipeModule();

    if  ( ! pModule ) {
        MY_LOGE("getNormalPipeModule() fail");
        return selectedVersion;
    }

    MUINT32 const* version = NULL;
    size_t count = 0;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIdx);
    if  ( err < 0 || ! count || ! version ) {
        MY_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIdx, err, count, version
        );
        return selectedVersion;
    }

    selectedVersion = *(version + count - 1); //Select max. version
    MY_LOGD("[%d] count:%zu Selected CamIO Version:%0#x", sensorIdx, count, selectedVersion);
    return selectedVersion;
}


/******************************************************************************
 *
 ******************************************************************************/
class NPipeUT;
class TuningNotifyImp_EIS : public P1_TUNING_NOTIFY {
public:
    TuningNotifyImp_EIS(NPipeUT *_pNPipeUtObj) : mpNPipeUtObj(_pNPipeUtObj) {}
    ~TuningNotifyImp_EIS() {}
    virtual const char* TuningName() { return "UT_EIS"; }
    virtual void p1TuningNotify(MVOID* pIn, MVOID* pOut);

    NPipeUT     *mpNPipeUtObj;
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


typedef void* (*ThreadR_t)(void*);
class NPipeUT {
    NPipeUT()
    : mpNPipe(NULL)
    , mpSttPipe(NULL)
    , tuningMgr(NULL)
    , mpImemDrv(NULL)
    , mTgSize(MSize(0,0))
    , mEisNotify(this)
    , mSgg2Notify(this)
    , mLcsNotify(this)
    , mSensorIdx(0)
    , m_enablePort(0)
    , mEnableSttPort(0)
    , mCropTestSel(0)
    , mStaticEnqCnt(1)
    , mEnqCount(0)
    , mDeqCount(0)
    , mFailCount(0)
    , mLoopCount(0)
    , m_bStop(MFALSE)
    , mFrameNum(8)
    , mBurstQNum(1)
    , mPortBufDepth(0)
    , m_b4k2k(MFALSE)
    , m_bEfuse(MFALSE)
    , mEfuseIdx(0)
    , mEnableTgInt(MFALSE)
    , m_Dualpd(MFALSE)
    , m_aeCyclePeriod(HWSYNC_AE_CYCLE_PERIOD)
    , m_ufeo_en(MFALSE)
    , mArgc(0)
    , mArgv(NULL)
    {
        memset((void*)mpImgBuffer, 0, sizeof(mpImgBuffer));
        memset((void*)mImemBuf, 0, sizeof(mImemBuf));
        memset((void*)mpSttBuf, 0, sizeof(mpSttBuf));

        memset(&m_Thread, 0, sizeof(m_Thread));
        memset(&m_EnqThd, 0, sizeof(m_EnqThd));
        memset(&m_HwsyncThd, 0, sizeof(m_HwsyncThd));
        memset(&mSttThd, 0, sizeof(mSttThd));
        memset(&m_TgIntThd, 0, sizeof(m_TgIntThd));

        memset(&m_semSwitchThd, 0, sizeof(m_semSwitchThd));
        memset(&m_semThread, 0, sizeof(m_semThread));
        memset(&m_semHwsyncThd, 0, sizeof(m_semHwsyncThd));
        memset(&mSemSttProc, 0, sizeof(mSemSttProc));
        memset(&m_semTgIntThd, 0, sizeof(m_semTgIntThd));
        memset(&m_semEnqThd, 0, sizeof(m_semEnqThd));
    };
    ~NPipeUT(){};
public:
    INormalPipe*        mpNPipe;
    IStatisticPipe*     mpSttPipe;
    TuningMgr*          tuningMgr;
    IMemDrv*            mpImemDrv;
    vector<IImageBuffer*> mpImgBuffer[__MAXDMAO];
    vector<IMEM_BUF_INFO> mImemBuf[__MAXDMAO];
    //MVOID               *mpSttBuf[__MAXDMAO_STT];
    MUINT8              mpSttBuf[__MAXDMAO_STT][512*1024];
    MUINT32             mpSttBufSize[__MAXDMAO_STT];
    MSize               mTgSize;
    TuningNotifyImp_EIS     mEisNotify;
    TuningNotifyImp_SGG2    mSgg2Notify;
    TuningNotifyImp_LCS     mLcsNotify;
    IMEM_BUF_INFO           m_bpciBuf;
    MUINT32             mSensorIdx;
    MUINT32             m_enablePort;
    MUINT32             mEnableSttPort;
    MUINT32             mCropTestSel;
    MUINT32             mStaticEnqCnt;  /* enque times before start */
    MUINT32             mEnqCount;      /* buf index for next enque */
    MUINT32             mDeqCount;
    MUINT32             mFailCount;
    MUINT32             mLoopCount;
    MBOOL               m_bStop;
    MUINT32             m_aeCyclePeriod;
    MBOOL               m_ufeo_en;
    MUINT32             mFrameNum;
    MUINT32             mBurstQNum;
    MUINT32             mPortBufDepth;
    MBOOL               m_b4k2k;
    MBOOL               m_bEfuse;
    MINT32              mEfuseIdx;
    MBOOL               mEnableTgInt;
    MBOOL               m_Dualpd;

    int                 mArgc;
    char**              mArgv;
    sem_t               m_semSwitchThd;

    static NPipeUT*     create(void);
    void                destroy(void);
    void                startThread(void);
    void                stopThread(void);
    void                startHwsyncThread(void);
    void                stopHwsyncThread(void);
    void                setTuning(MUINT32* pMagic);
    void                setZHdrTuning(MUINT32* pMagic);
    static MVOID*       main_loop(void* arg);
    static MVOID*       enq_loop(void* arg);
    static MVOID*       hwsync_loop(void* arg);
    static MVOID*       tg_loop(void* arg);

    static void         sttProcLoop(MUINT32 dmao, NPipeUT* _this);
    static void*        _sttProcLoop0_ (void *arg);
    static void*        _sttProcLoop1_ (void *arg);
    static void*        _sttProcLoop2_ (void *arg);
    static void*        _sttProcLoop3_ (void *arg);
    static ThreadR_t    sttProcRoutine[__MAXDMAO_STT];

private:
    pthread_t           m_Thread;
    pthread_t           m_EnqThd;
    pthread_t           m_HwsyncThd;
    pthread_t           mSttThd[__MAXDMAO_STT];
    pthread_t           m_TgIntThd;
    sem_t               m_semThread;
    sem_t               m_semEnqThd;
    sem_t               m_semHwsyncThd;
    sem_t               mSemSttProc[__MAXDMAO_STT];
    sem_t               m_semTgIntThd;
};

static char *mThisName = (char*)LOG_TAG;
static PortID sttPortIDMap[__MAXDMAO_STT] = {PORT_AAO, PORT_AFO, PORT_FLKO, PORT_PDO};

//for PDO verify
static MBOOL m_bN3DTest;


/*
 * croping size have an alignment-rule in imgo/imgo_d (no resizer dma port)
 * rrzo do nothing, because of rrzo not using dmao cropping.
 * note :
 * the query operation, if cropsize != inputsize , use ISP_QUERY_CROP_X_PIX to query.
 */
static MSize __CROP_SIZE(PortID portId, EImageFormat fmt, MSize size, MUINT32 cropsel,MBOOL bin_en, MBOOL b4k2k)
{
    size.w = size.w>>bin_en;
    size.h = size.h>>bin_en;

    if (portId == PORT_RRZO) {
        NormalPipe_QueryInfo    qry;
        MSize _scaled= MSize(((size.w*RRZ_SCALING_RATIO_N)/RRZ_SCALING_RATIO_M + 64),
                ((size.h*RRZ_SCALING_RATIO_N)/RRZ_SCALING_RATIO_M + 64));

        if(b4k2k) {
           _scaled.w = RRZO_4K2K_WIDTH;
           _scaled.h = RRZO_4K2K_HEIGHT;
        }

        getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX, fmt, _scaled.w, qry);

        _scaled.w = qry.x_pix;

        if ((_scaled.w > size.w) || (_scaled.h > size.h)) {

            return size;
        }
        else {

            return _scaled;
        }
    }

    if (portId == PORT_IMGO) {
        NormalPipe_QueryInfo    qry;
        MSize __size;
        if (0 == cropsel) {
            __size = MSize(size.w, size.h);
        }
        else {
            __size = MSize(((size.w*RRZ_SCALING_RATIO_N)/RRZ_SCALING_RATIO_M + 64),
                            ((size.h*RRZ_SCALING_RATIO_N)/RRZ_SCALING_RATIO_M + 64));
        }

        getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX, fmt, __size.w, qry);

        __size.w = qry.x_pix;

        return __size;
    }

    return size;
}

static MPoint __CROP_START(PortID portId, MSize in, MSize crop, MINT32 fmt, MUINT32 cropsel,MBOOL bin_en)
{
    NormalPipe_QueryInfo qry;
    MINT32 width;
    MINT32 height;

    switch (cropsel) {
        case 1:
            width = (((in.w-crop.w)/2)>>bin_en);
            height = (((in.h-crop.h)/2)>>bin_en);
            break;
        case 2:
            width = (((in.w-crop.w)/2-5)>>bin_en);
            height = (((in.h-crop.h)/2)>>bin_en);
            break;
        case 0:
        default:
            return MPoint(0, 0);
    }
    if (width < 0) {
        MY_LOGE("Wrong crop width %d of %d", crop.w, in.w);
        width = 0;
    }
    if (height < 0) {
        MY_LOGE("Wrong crop height %d of %d", crop.h, in.h);
        height = 0;
    }

    getNormalPipeModule()->query(portId.index, ENPipeQueryCmd_CROP_START_X, (EImageFormat)fmt, width, qry);

    return MPoint(qry.crop_x, height);
}

void onDumpBuffer(const char* usr, MUINT32 sensor_idx, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
//#define DUMP_PREFIX "/storage/sdcard0/cameradump_"
#define DUMP_PREFIX "/data/cameradump_"
//#define DUMP_PREFIX "/system/bin/cameradump_"
#define DUMP_IImageBuffer( type, pbuf, fileExt )                \
    do {                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;             \
        char filename[256];                                     \
        sprintf(filename, "%s%d_%d_%dx%d_%d_%d.%s",             \
                dumppath,                                       \
                sensor_idx,                                     \
                type,                                           \
                buffer->getImgSize().w,buffer->getImgSize().h,  \
                (MINT32)buffer->getBufStridesInBytes(0),        \
                ext,                                            \
                fileExt                                         \
               );                                               \
        buffer->saveToFile(filename);                           \
        MY_LOGD("Dump x%x: %dx%d, stride %d to %s", (MUINT32)buffer->getBufPA(0), (MUINT32)buffer->getImgSize().w,\
                (MUINT32)buffer->getImgSize().h, (MUINT32)buffer->getBufStridesInBytes(0), filename);\
    } while(0)
    char dumppath[256];

    sprintf(dumppath, "%s%s/", DUMP_PREFIX, usr);

    if (!makePath(dumppath, 0660)) {
        MY_LOGD("makePath [%s] fail",dumppath);
        return;
    }

    DUMP_IImageBuffer(data, buf, "raw");
}

static bool saveBuf(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %p, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return true;
}

void *NPipeUT::_sttProcLoop0_ (void *arg)
{
    NPipeUT::sttProcLoop(0, (NPipeUT*)arg);
    return 0;
}
void *NPipeUT::_sttProcLoop1_ (void *arg)
{
    NPipeUT::sttProcLoop(1, (NPipeUT*)arg);
    return 0;
}
void *NPipeUT::_sttProcLoop2_ (void *arg)
{
    NPipeUT::sttProcLoop(2, (NPipeUT*)arg);
    return 0;
}

void *NPipeUT::_sttProcLoop3_ (void *arg)
{
    NPipeUT::sttProcLoop(3, (NPipeUT*)arg);
    return 0;
}


ThreadR_t NPipeUT::sttProcRoutine[__MAXDMAO_STT] = {
        NPipeUT::_sttProcLoop0_,
        NPipeUT::_sttProcLoop1_,
        NPipeUT::_sttProcLoop2_,
        NPipeUT::_sttProcLoop3_,
};

NPipeUT* NPipeUT::create(void)
{
    return new NPipeUT();
}

void NPipeUT::destroy(void)
{
    delete this;
}

void NPipeUT::startThread(void)
{
    #if 1
    ::sem_init(&m_semThread, 0, 0);
    ::sem_init(&m_semEnqThd, 0, 0);

    pthread_create(&m_Thread, NULL, main_loop, this);
    pthread_create(&m_EnqThd, NULL, enq_loop, this);

    if (mEnableSttPort) {
        MUINT32 sdma = 0;
        for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
            if (mEnableSttPort & (1 << sdma)) {
                MY_LOGD("S_%d: Statistic port enable : %d", this->mSensorIdx, sdma);
                ::sem_init(&mSemSttProc[sdma], 0, 0);

                pthread_create(&mSttThd[sdma], NULL, sttProcRoutine[sdma], this);
            }
        }
        //tg interrupt
        if (mEnableTgInt){
            ::sem_init(&m_semTgIntThd, 0, 0);
            pthread_create(&m_TgIntThd, NULL, tg_loop, this);
        }
    }
    #else
    pthread_attr_t attr;
    attr.flags = 0;
    attr.stack_base = NULL;
    attr.stack_size = 1024*1024;
    attr.guard_size = 4096;
    attr.sched_policy = SCHED_RR;
    attr.sched_priority = PRIO_RT_ISPDEQUEUE_THREAD;

    ::sem_init(&m_semThread, 0, 0);

    MY_LOGD("temp borrow isp dequeue thread priority");

    pthread_create(&m_Thread, &attr, main_loop, this);
    #endif
}

void NPipeUT::stopThread(void)
{
    MY_LOGD("wait stop");

    if (mEnableSttPort) {
        MUINT32 sdma = 0;
        for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
            if (mEnableSttPort & (1 << sdma)) {
                MY_LOGD("S_%d: Statistic wait stop : %d", this->mSensorIdx, sdma);
                ::sem_wait(&mSemSttProc[sdma]);
            }
        }
    }

    MY_LOGD("Wait deq thread exit");
    ::sem_wait(&m_semThread);
    MY_LOGD("Wait enq thread exit");
    ::sem_wait(&m_semEnqThd);

    if (mEnableTgInt){
        MY_LOGD("Wait tg thread exit");
        ::sem_wait(&m_semTgIntThd);
    }
    //pthread_join(m_Thread, NULL);
    //pthread_join(m_EnqThd, NULL);
}


void NPipeUT::startHwsyncThread(void)
{
#ifdef HWSYNC_USE_AE_THREAD
    if(m_bN3DTest == true)
    {
    ::sem_init(&m_semHwsyncThd, 0, 0);
    pthread_create(&m_HwsyncThd, NULL, hwsync_loop, this);
    }
#endif
}

void NPipeUT::stopHwsyncThread(void)
{
#ifdef HWSYNC_USE_AE_THREAD
    if(m_bN3DTest == true)
    {
    MY_LOGD("wait hwsync stop");
    ::sem_wait(&m_semHwsyncThd);
    }
#endif

    //pthread_join(m_Thread, NULL);
    //pthread_join(m_EnqThd, NULL);
}

MVOID NPipeUT::setTuning(MUINT32* pMagic)
{
#define AF_WIN_NUM_X  64
#define AF_WIN_NUM_Y  64

    MUINT32 tmp=0;
    MUINT32 h_size,win_h_size;
    MUINT32 v_size,win_v_size;
    MUINT32 xsize,ysize;
    MUINT32 af_v_avg_lvl,af_v_gonly;
    MUINT32 start_x=0,start_y=0;
    MUINT32 sensorIndex = 0;

    if (0 == mEnableSttPort) {
        return;
    }

    tuningMgr->dequeBuffer((MINT32*)pMagic);

    for(MUINT32 i=0;i<this->mBurstQNum;i++){
        MY_LOGD("TG(%d,%d) M(%d)", mTgSize.w, mTgSize.h, pMagic[i]);


        if(this->mEnableSttPort & __AFO_ENABLE){
            tuningMgr->updateEngine(eTuningMgrFunc_AF, MTRUE, i);
            //AF image wd
            this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&h_size, (MINTPTR)&v_size, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_CON, 0, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_SIZE, h_size, i);//case for no frontal binning

            //
            af_v_avg_lvl = 0;
            af_v_gonly = 0;


            //ofset
            start_x = ((start_x & 0x1)?(0x1):(0));
            start_y = ((start_y & 0x1)?(0x1):(0));
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_VLD, (start_y<<16 | start_x), i);

            //window num
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_BLK_1, ((AF_WIN_NUM_Y<<16)|AF_WIN_NUM_X), i);

            win_h_size = (h_size-start_x) / AF_WIN_NUM_X;
            if (win_h_size > 254) {
                win_h_size = 254;
            }
            else {//min constraint
                if((af_v_avg_lvl == 3) && (af_v_gonly == 1)){
                    win_h_size = (win_h_size < 32)? (32):(win_h_size);
                }
                else if((af_v_avg_lvl == 3) && (af_v_gonly == 0)){
                    win_h_size = (win_h_size < 16)? (16):(win_h_size);
                }
                else if((af_v_avg_lvl == 2) && (af_v_gonly == 1)){
                    win_h_size = (win_h_size < 16)? (16):(win_h_size);
                }
                else{
                    win_h_size = (win_h_size < 8)? (8):(win_h_size);
                }
            }
            if (af_v_gonly == 1)
                win_h_size = win_h_size/4 * 4;
            else
                win_h_size = win_h_size/2 * 2;

            win_v_size = (v_size-start_y) / AF_WIN_NUM_Y;
            if (win_v_size > 255)
                win_v_size = 255;
            else
                win_v_size = (win_v_size < 1)? (1):(win_v_size);

            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AF_BLK_0, ((win_v_size<<16)|win_h_size), i);

            //xsize/ysize
            xsize = AF_WIN_NUM_X*16;
            ysize = AF_WIN_NUM_Y;
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_XSIZE, xsize, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_YSIZE, ysize, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AFO_STRIDE, xsize, i);

            tuningMgr->updateEngine(eTuningMgrFunc_SGG1, MTRUE, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_PGN, 0x200, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_GMRC_1, 0xffffffff, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_SGG1_GMRC_2, 0xffffffff, i);
        }
    ////////////////////////////////////////////////////////////////////////////////////////
    //AAO
        if(this->mEnableSttPort & __AAO_ENABLE){
            tuningMgr->updateEngine(eTuningMgrFunc_AA, MTRUE, i);

            MUINT32 ae_hst_sel,aa_in_h,aa_in_v,aa_start_x,aa_start_y,_cnt=0;
            MUINT32 aa_isize_h,aa_isize_v;
            MUINT32 aa_win_size_h,aa_win_size_v;
            MUINT32 aa_xsize = 0;
            MUINT32 awb_pix_cnt;

#define AA_WIN_H    120
#define AA_WIN_V    90

#define HDR_ST_EN   1
#define AE_OE_EN    1
#define TSF_ST_EN   1
#define AE_HST_SEL  1
            //
            ae_hst_sel = AE_HST_SEL;
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_NUM, (AA_WIN_H | (AA_WIN_V<<16)), i);

            //aa input size , form bmx output
            this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&aa_in_h, (MINTPTR)&aa_in_v, i);

            //aa win/pit size, win size <= pit size
            aa_win_size_h = (aa_in_h / AA_WIN_H) /2 * 2;
            aa_win_size_v = (aa_in_v / AA_WIN_V) /2 * 2;

            //aa start coordinate
            aa_start_x = (aa_in_h - aa_win_size_h*AA_WIN_H)/ 2;
            aa_start_y = (aa_in_v - aa_win_size_v*AA_WIN_V)/ 2;

            if(aa_win_size_h < 4)
                MY_LOGD("h-min is 4 , error\n");
            if(aa_win_size_v < 2)
                MY_LOGD("v-min is 2 , error\n");

            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_SIZE, (aa_win_size_h| (aa_win_size_v<<16)), i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_PIT,  (aa_win_size_h| (aa_win_size_v<<16)), i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_WIN_ORG, (aa_start_x| (aa_start_y<<16)), i);

            aa_xsize = (AA_WIN_H*AA_WIN_V*4);//awb , 4 bytes each win
            aa_xsize += (AA_WIN_H*AA_WIN_V);// ae , 1 byte each win
            //
            _cnt=0;
            if(HDR_ST_EN){
                _cnt|= 0x4;
                aa_xsize += (AA_WIN_H*AA_WIN_V*4/8);    // 4 bits each win
            }
            if(AE_OE_EN){
                _cnt |= 0x2;
                aa_xsize += (AA_WIN_H*AA_WIN_V); // 1 byte each win
            }
            if(TSF_ST_EN){
                _cnt |= 0x1;
                aa_xsize += (AA_WIN_H*AA_WIN_V*2); // 2 bytes each win
            }
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_STAT_EN, _cnt, i);

            //awb
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_GAIN1_0, 0x02000200, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_GAIN1_1, 0x00000200, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LMT1_0, 0x0fff0fff, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LMT1_1, 0x00000fff, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_LOW_THR, 0x00000000, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_HI_THR, 0x00ffffff, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_RC_CNV_4, 0x02000200, i);

            awb_pix_cnt = (1<<24) / (aa_win_size_h * aa_win_size_v) *2;
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT0, awb_pix_cnt*2, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT1, awb_pix_cnt, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AWB_PIXEL_CNT2, awb_pix_cnt*2, i);

            //ae
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_GAIN2_0, 0x00090200, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_GAIN2_1, 0x00000000, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_LMT2_0, 0x00000000, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_HST_SEL, ae_hst_sel, i);//0x1A004A54
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_HST_CTL, 0x00000000, i);
            /*TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AE_HST_CTL, 7, 0);//??
            aa_xsize += 2*128*3;*/

            //aao
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_XSIZE, aa_xsize, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_YSIZE, 1, i);
            TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_AAO_STRIDE, aa_xsize, i);
        }
    ////////////////////////////////////////////////////////////////////////////////////////
    //PDO
        if(this->mEnableSttPort & __PDO_ENABLE){
            #define SPECIAL_TOKEN 0xc000
            xsize = ysize = 0;
            //native pd
            if (!this->m_Dualpd) {
                MY_LOGD("native PD");
                tuningMgr->updateEngine(eTuningMgrFunc_BNR, MTRUE, i);
                tuningMgr->updateEngineFD(eTuningMgrFunc_BNR, i, m_bpciBuf.memID,m_bpciBuf.virtAddr);
                //bpci
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_BPCI_BASE_ADDR,m_bpciBuf.phyAddr,i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_BPCI_XSIZE,sizeof(bpci_array_s5k2x8),i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_BPCI_YSIZE,1,i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_BPCI_STRIDE,sizeof(bpci_array_s5k2x8),i);

                //pdo
                //TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_BASE_ADDR,m_bpciBuf.virtAddr,0); // doesn't need to cfg pdo addr here.
                //use full TG size here is just easiest example,
                //theoratically, PDO size msut be smaller than TG size,
                //e.g.: PDO width = 1/8 TG_W,  PDO height = 1/4 TG_H
                xsize = mTgSize.w;
                xsize *= 2;//PDO is 2byte per pix
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_XSIZE,xsize,i);

#if 1
                        {
                            unsigned short int* ptr = (unsigned short int*)&bpci_array_s5k2x8[0];
                            MUINT32 n_2bytejump = 0;
                            for(int i=0; i<(sizeof(bpci_array_s5k2x8)); )
                            {
                                //
                                if( ((*ptr)&SPECIAL_TOKEN)==SPECIAL_TOKEN){
                                    ysize++;
                                    //jump ofset & cnum
                                    n_2bytejump = 3;
                                    i += (n_2bytejump*2);
                                    ptr += n_2bytejump;
                                    //jump number of pnum
                                    n_2bytejump = (*ptr) + 1 + 1;//+ 1 for next special token , +1 for pnum indexing is start from 0
                                    i += (n_2bytejump*2);
                                    ptr += n_2bytejump;
                                }
                            }
                        }
#else
                        for(int i=0;i<(sizeof(bpci_array_s5k2x8)/sizeof(MUINT32));i++){
                        if( (bpci_array_s5k2x8[i] & 0x0000ff00) == SPECIAL_TOKEN)
                            ysize += 1;
                        if( ((bpci_array_s5k2x8[i]>>16) & 0x0000ff00) == SPECIAL_TOKEN)
                            ysize += 1;
                    }
#endif


                ysize = (ysize > 0) ? (ysize): (ysize+1);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_YSIZE,ysize,i);
                //pdo stride must be 16-alignment
                if((xsize %16) != 0)
                    xsize = (xsize + 15) / 16 * 16;
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_STRIDE,xsize,i);

                //pdo function_en cfg
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_BPC_CON,BPC_LUT_EN,1,i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_BPC_CON,BPC_EN,1,i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_PDC_CON,PDC_EN,1,i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_PDC_CON,PDC_OUT,1,i);

                //BPC TABLE
                MUINT32 bpc_in_h, bpc_in_v;
                this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&bpc_in_h, (MINTPTR)&bpc_in_v, i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_BPC_TBLI2,BPC_XSIZE,bpc_in_h-1,i);//DMX_crop_w-1
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_BNR_BPC_TBLI2,BPC_YSIZE,bpc_in_v-1,i);
            } else {
                MY_LOGD("dual PD");
                //dual pd
                tuningMgr->updateEngine(eTuningMgrFunc_PBN, MTRUE, i);
                //
                this->mpNPipe->sendCommand(ENPipeCmd_GET_PMX_INFO,(MINTPTR)&xsize, (MINTPTR)&ysize, i);
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_XSIZE,xsize*2,i);//PDO is 2byte per pix
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_YSIZE,ysize,i);
                MY_LOGD("PDO SIZE: (%d_%d)", xsize*2, ysize);

                //pdo stride must be 16-alignment
                if((xsize %16) != 0)
                    xsize = (xsize + 15) / 16 * 16;
                TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_PDO_STRIDE,xsize*2,i);
                MY_LOGD("PDO STRIDE: (%d)", xsize*2);

                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_TYPE,PBN_TYPE,0,i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_LST,PBN_LST,1,i);
                TUNING_MGR_WRITE_BITS_CAM(tuningMgr, CAM_PBN_VSIZE,PBN_VSIZE,mTgSize.h,i);
            }
            #undef SPECIAL_TOKEN
        }

#define FLK_WIN_H   2
#define flk_WIN_V   2
        if (this->mEnableSttPort & __FLKO_ENABLE) {
            //input size w/h: ref SGG_SEL
            //SGG_SEL: 0: from BMX, 1 from TG

            xsize = 0;

            this->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO,(MINTPTR)&h_size,(MINTPTR)&v_size,i);
            MY_LOGD(" ____________ QUERY HBIN INF: (%d_%d)", h_size, v_size);

            tuningMgr->updateEngine(eTuningMgrFunc_FLK, MTRUE, i);

            //start
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLK_A_OFST, 0, i); //this->m_pIspDrv->writeReg(0x3534,0x0);

            //win num
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLK_A_NUM, ((flk_WIN_V<<4)|FLK_WIN_H), i); //this->m_pIspDrv->writeReg(0x353c,((flk_WIN_V<<4)|FLK_WIN_H));

            //win size
            h_size = (h_size / FLK_WIN_H)/2*2;
            v_size = (v_size / flk_WIN_V)/2*2;
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLK_A_SIZE, ((v_size<<16)|h_size), i); //this->m_pIspDrv->writeReg(0x3538,((size_v<<16)|size_h));

            //xsize
            xsize = (FLK_WIN_H*flk_WIN_V*v_size * 2);
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLKO_XSIZE, xsize, i); //this->m_pIspDrv->writeReg(0x3260,x_size);

            //ysize
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLKO_YSIZE, 1, i); //this->m_pIspDrv->writeReg(0x3264,0);

            //stride
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_FLKO_STRIDE, xsize, i);//this->m_pIspDrv->writeReg(0x3268,x_size + 1);

            tuningMgr->updateEngine(eTuningMgrFunc_SGG3, MTRUE, i);
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_SGG3_A_PGN, 0x200, i);
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_SGG3_A_GMRC_1, 0x10080402, i);
            TUNING_MGR_WRITE_REG_UNI(tuningMgr, CAM_UNI_SGG3_A_GMRC_2, 0x804020, i);
        }

    }
    tuningMgr->enqueBuffer();
}

void NPipeUT::setZHdrTuning(MUINT32* pMagic)
{
#if (TEST_ZHDR_HW == 1)

    tuningMgr->dequeBuffer((MINT32*)pMagic);

    for(MUINT32 i=0;i<this->mBurstQNum;i++){

        MY_LOGD("zvHdr(%d,%d) M(%d)", mTgSize.w, mTgSize.h, pMagic[i]);


        tuningMgr->updateEngine(eTuningMgrFunc_DBS, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SIGMA,      0x00150005, i); /* 0x1A004B40 */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_0,    0x00010204, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_1,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_2,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_BSTBL_3,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_CTL,        0x0840F000, i);
                                                                /* DBS_EDGE: 0xF */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_CTL_2,      0x00000FFA, i); /*TBD*/
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SIGMA_2,    0x0f0f0f0f, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_YGN,        0x20202020, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_Y12,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_Y34,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_G12,     0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_DBS_SL_G34,     0x00000000, i);

        tuningMgr->updateEngine(eTuningMgrFunc_RMG, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG,    (0xFFA000E2|0x10/*ZHDR_EN*/), i); /* 0x1A0045A0 */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_GAIN,   0x00400040, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMG_HDR_CFG2,   0x00000400, i);

        tuningMgr->updateEngine(eTuningMgrFunc_RMM, MTRUE, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_OSC,        0xF01143EC, i); /* 0x1A0045C0 */
                                                                /* RMM_EDGE: 0xF */
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MC,         0x00040245, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_REVG_1,     0x04000400, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_REVG_2,     0x04000400, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_LEOS,       0x00000200, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MC2,        0x00050080, i); /*TBD*/
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_DIFF_LB,    0x00000000, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_MA,         0x08088408, i);
        TUNING_MGR_WRITE_REG_CAM(tuningMgr, CAM_RMM_TUNE,       0x30800013, i);
    }
    tuningMgr->enqueBuffer();
#endif
}


MVOID* NPipeUT::main_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    QPortID         deqPorts;
    QBufInfo        dequeBufInfo;
    MUINT32         i = 0;

    MY_LOGD("start main_loop");

    ::pthread_detach(::pthread_self());

    if (_this->m_enablePort & __IMGO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_IMGO);
    }
    if (_this->m_enablePort & __RRZO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_RRZO);
        //deqPorts.mvPortId.push_back(PORT_UFEO);
    }
    if (_this->m_enablePort & __EISO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_EISO);
    }
    if (_this->m_enablePort & __LCSO_ENABLE) {
        deqPorts.mvPortId.push_back(PORT_LCSO);
    }

    for ( ; _this->mLoopCount < _this->mFrameNum; ) {

        if (_this->mpNPipe->deque(deqPorts, dequeBufInfo, 19527) == MTRUE) {
            for (i = 0; i < dequeBufInfo.mvOut.size(); i++) {
                if (0 == dequeBufInfo.mvOut.at(i).mBuffer) {
                    MY_LOGE("deque wrong");
                    break;
                }
                MY_LOGD("S_%d[deque]: dma:0x%x, PA:%lx, crop:%d_%d_%d_%d_%d_%d_%d_%d, size:%dx%d, mag:0x%x, bufidx:0x%x",\
                    _this->mSensorIdx,
                    dequeBufInfo.mvOut.at(i).mPortID.index, dequeBufInfo.mvOut.at(i).mBuffer->getBufPA(0),\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_s.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.x,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.p.y,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.w,dequeBufInfo.mvOut.at(i).mMetaData.mCrop_d.s.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.w,dequeBufInfo.mvOut.at(i).mMetaData.mDstSize.h,\
                    dequeBufInfo.mvOut.at(i).mMetaData.mMagicNum_tuning,\
                    dequeBufInfo.mvOut.at(i).mBufIdx);

                #if 0
                onDumpBuffer("camio", _this->mSensorIdx, dequeBufInfo.mvOut.at(i).mPortID.index,
                        (MUINTPTR)dequeBufInfo.mvOut.at(i).mBuffer, _this->mLoopCount);
                #endif
            }

            _this->mDeqCount += _this->mBurstQNum;//dequeBufInfo.mvOut.size();
            _this->mLoopCount++;
        }
        else {
            _this->mFailCount++;
            MY_LOGE("S_%d deque fail", _this->mSensorIdx);

            if (_this->mFailCount > 1) {
                MY_LOGE("S_%d deque break", _this->mSensorIdx);
                break;
            }
        }

        if(_this->m_bEfuse == MTRUE) {
            MY_LOGD("S_%d deque break", _this->mSensorIdx);
            break;
        }
    }

    _this->m_bStop = MTRUE;

    MY_LOGD("S_%d exit main_loop, stop", _this->mSensorIdx);

    if(_this->m_bEfuse == MTRUE) {
        MY_LOGD("BOUNDING Result: %s %s",\
            efTable[_this->mEfuseIdx].resultStr, \
            (_this->mFailCount > 0 ? "FAIL":"PASS"));
    }

    ::sem_post(&_this->m_semThread);

    return NULL;
}

MVOID* NPipeUT::enq_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    QBufInfo        enqQ;
    MUINT32         i = 0, sof_idx = 0, enqBufIdx = 0;
    MUINT32         magicNum = _this->mEnqCount;
    MUINT32         *pMag = NULL;
    MINT32          userKey = 0;

    MY_LOGD("start enq_loop");

    ::pthread_detach(::pthread_self());

    userKey = _this->mpNPipe->attach("UTEnqLoop");
    if (userKey < 0) {
        MY_LOGE("Wrong userkey : %d", userKey);
        return NULL;
    }

    while (!_this->m_bStop) {
        #if 0
        /*
         * Check drop frame or not, if current frame is drop frame, and
         * can't enque this frame into drv in order to prevent sensor Gain/
         * ExpT latch timing async with isp setting.
         * not opened this option is because of this test case having only 1 thread.
         */
        NormalPipe_FRM_STATUS _drop_status = _drop_frame_status;
        while (_drop_status == _drop_frame_status) {
            _this->mpNPipe->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS,(MINT32)&_drop_status,0,0)
            usleep(5000);
        }
        #endif

        /*
         * Check deque buffer index to prevent overrun buffer
         */
        if ((_this->mEnqCount + _this->mBurstQNum) > (_this->mDeqCount + _this->mPortBufDepth)) {
            MY_LOGD("No empty buffer for enque, wait");
            do {
                MY_LOGD("    enq/deq : %d/%d", _this->mEnqCount, _this->mDeqCount);
                usleep(100000); //sleep 100ms to wait deque
            } while (((_this->mEnqCount + _this->mBurstQNum) > (_this->mDeqCount + _this->mPortBufDepth)) && (MFALSE == _this->m_bStop));
            if (_this->m_bStop) {
                break;
            }
        }

        enqBufIdx = _this->mEnqCount % _this->mPortBufDepth;

        _this->mpNPipe->wait(EPipeSignal_SOF, EPipeSignal_ClearWait, userKey, 7788);

        if (_this->m_bStop) {
            MY_LOGD("Stop after wait SOF");
            break;
        }

        _this->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);
        MY_LOGD("@SOF_%d", sof_idx);

        #if 0
        usleep(5000);//sleep 5ms to cross over vsync and simulate 3a calculation
        if (_this->mLoopCount % 100) {
            usleep(20000);//make drop frame status happened intenionally every 100 frames
        }
        #endif

        enqQ.mvOut.clear();
        pMag = (MUINT32*)malloc(sizeof(MUINT32)*_this->mBurstQNum);
        for (i = enqBufIdx; i < (enqBufIdx + _this->mBurstQNum); i++, magicNum++) {
            MBOOL bin_en = MFALSE;
            MUINT32 bin_w,bin_h;

            _this->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
            bin_en = (bin_w != _this->mTgSize.w)? (1):(0);

            if (_this->m_enablePort & __IMGO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__IMGO].at(i);
                MSize _cropsize = __CROP_SIZE(PORT_IMGO, (EImageFormat)pIBuf->getImgFormat(),\
                                    _this->mTgSize, _this->mCropTestSel,bin_en,0);

                enqQ.mvOut.push_back(BufInfo(PORT_IMGO, pIBuf, _cropsize,
                                        MRect(__CROP_START(PORT_IMGO, _this->mTgSize, _cropsize,\
                                            (EImageFormat)pIBuf->getImgFormat(), 0,bin_en), _cropsize),\
                                        magicNum, sof_idx, (_this->m_enablePort & __IMGO_PURE_RAW)?1:0, 0xFFFF));
            }
            if (_this->m_enablePort & __RRZO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__RRZO].at(i);
                MSize _cropsize = __CROP_SIZE(PORT_RRZO, (EImageFormat)pIBuf->getImgFormat(), _this->mTgSize, _this->mCropTestSel,bin_en, _this->m_b4k2k);
                MSize _rrz = __SCALE_PIX_SIZE((EImageFormat)pIBuf->getImgFormat(), _this->mTgSize, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M, _this->m_b4k2k);

                if ((_cropsize.w < _rrz.w) || (_cropsize.h < _rrz.h)) {
                    _rrz = _cropsize;
                    MY_LOGD("RRZ in constraint: %dx%d", _rrz.w, _rrz.h);
                }

                MY_LOGD("rrzo_%d_%d TG_%d_%d", _rrz.w, _rrz.h, _this->mTgSize.w, _this->mTgSize.h);

                enqQ.mvOut.push_back(BufInfo(PORT_RRZO, pIBuf, _rrz,\
                                        MRect(__CROP_START(PORT_RRZO, _this->mTgSize, _cropsize,\
                                            (EImageFormat)pIBuf->getImgFormat(), _this->mCropTestSel,bin_en), _cropsize),\
                                        magicNum, sof_idx, 0xFFFF));
#if 1
                if(_this->m_ufeo_en == 1){
                    //UFEO
                    IImageBuffer* pIBuf_UFEO = _this->mpImgBuffer[__UFEO].at(i);
                    MY_LOGD("ufeo_%d_%d TG_%d_%d", _rrz.w, _rrz.h, _this->mTgSize.w, _this->mTgSize.h);
                    enqQ.mvOut.push_back(BufInfo(PORT_UFEO, pIBuf_UFEO, _rrz,\
                                        MRect(__CROP_START(PORT_RRZO, _this->mTgSize, _cropsize,\
                                            (EImageFormat)pIBuf_UFEO->getImgFormat(), _this->mCropTestSel,bin_en), _cropsize),\
                                        magicNum, sof_idx, 0xFFFF));
                }
#endif
            }
            if (_this->m_enablePort & __EISO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__EISO].at(i);
                MSize _ssize(EISO_SIZE, 1);

                enqQ.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), magicNum, sof_idx, 0xFFFF));
            }
            if (_this->m_enablePort & __LCSO_ENABLE) {
                IImageBuffer*   pIBuf = _this->mpImgBuffer[__LCSO].at(i);
                MSize _ssize = _this->mLcsNotify.queryLcsOutSize(_this->mTgSize);

                enqQ.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _ssize, MRect(MPoint(0,0), _ssize), magicNum, sof_idx, 0xFFFF));
            }
            //
            pMag[i-enqBufIdx] = magicNum;

        }
        //
        if (_this->mEnableSttPort) {
                _this->setTuning(pMag);
            }

        #if (TEST_ZHDR_HW == 1)
        _this->setZHdrTuning(pMag);
        #endif

        free(pMag);

        for (i = 0; i < enqQ.mvOut.size(); i++) {
            MY_LOGD("S_%d[enque] dma:0x%x sof:%d PA:%lx crop:%d_%d_%d_%d size:%dx%d mag:0x%x",\
                    _this->mSensorIdx, enqQ.mvOut.at(i).mPortID.index, sof_idx, enqQ.mvOut.at(i).mBuffer->getBufPA(0),\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.p.x,enqQ.mvOut.at(i).FrameBased.mCropRect.p.y,\
                    enqQ.mvOut.at(i).FrameBased.mCropRect.s.w,enqQ.mvOut.at(i).FrameBased.mCropRect.s.h,\
                    enqQ.mvOut.at(i).FrameBased.mDstSize.w,enqQ.mvOut.at(i).FrameBased.mDstSize.h,\
                    enqQ.mvOut.at(i).FrameBased.mMagicNum_tuning);
        }

        if (_this->mpNPipe->enque(enqQ) == MTRUE) {
            #if (TEST_AE_HIGHSPEED == 1)
            static MUINT32 _last_sof = 0;
            MUINT32 sof_diff = (sof_idx > _last_sof)? (sof_idx - _last_sof): (_last_sof - sof_idx);
            if (sof_diff >= (_this->mBurstQNum*2)) {
                NSImageio::NSIspio::ISPIO_REG_CFG reg1;
                list<NSImageio::NSIspio::ISPIO_REG_CFG> input;

                reg1.Addr = 0x0D70;
                reg1.Data = _this->mEnqCount;
                input.push_back(reg1);

                _this->mpNPipe->sendCommand(ENPipeCmd_HIGHSPEED_AE, 2, (MINTPTR)&input, 1);
            }
            _last_sof = sof_idx;
            #endif
            _this->mEnqCount += _this->mBurstQNum;
        }
        else {
            MY_LOGD("S_%d[enque] NG try again", _this->mSensorIdx);
        }
    }

    MY_LOGD("exit enq_loop");

    ::sem_post(&_this->m_semEnqThd);

    return NULL;
}

void NPipeUT::sttProcLoop(MUINT32 dmao, NPipeUT* _this)
{
    MUINT32 portIdx = dmao;

    ::pthread_detach(::pthread_self());

    MY_LOGD("S_%d: sttThread_%d: %d", _this->mSensorIdx, dmao, gettid());

    for ( ; MFALSE == _this->m_bStop; ) {
        QBufInfo    rDQBuf, rEQBuf;
        MUINT32     i;

        /**
         * Blocking wait deque done
         */
        if (MFALSE == _this->mpSttPipe->deque(sttPortIDMap[portIdx], rDQBuf)) {
            MY_LOGE("S_%d: dma0x%x deq failed...", _this->mSensorIdx, sttPortIDMap[portIdx].index);
        }

        /**
         * Do something after deque buf
         */
        #if 0 //TBD
        if (NULL == _this->mpSttBuf[portIdx]) {
            MY_LOGD("alloc dbg buf %d", rDQBuf.mvOut.at(0).mSize);
            _this->mpSttBuf[portIdx] = new char(rDQBuf.mvOut.at(0).mSize);
            if (_this->mpSttBuf[portIdx]) {
                MY_LOGD("dbg buf allocated");
                _this->mpSttBufSize[portIdx] = rDQBuf.mvOut.at(0).mSize;
            }
        }
        if (_this->mpSttBuf[portIdx]) {
            memcpy(_this->mpSttBuf[portIdx], (void*)rDQBuf.mvOut.at(0).mVa, _this->mpSttBufSize[portIdx]);
        }
        #elif 1
        if(dmao != 3){//pdo support no memcpy
            memcpy(_this->mpSttBuf[portIdx], (void*)rDQBuf.mvOut.at(0).mVa, _this->mpSttBufSize[portIdx]);
            _this->mpSttBufSize[portIdx] = rDQBuf.mvOut.at(0).mSize;
            MY_LOGD("Copy %p <- %p, %dbytes", _this->mpSttBuf[portIdx], (void*)rDQBuf.mvOut.at(0).mVa, _this->mpSttBufSize[portIdx]);
        }
        #endif

        /**
         * Enque back buffers
         * Only fill necessary fields
         */
        rEQBuf.mvOut.resize(rDQBuf.mvOut.size());
        for (i = 0; i < rDQBuf.mvOut.size(); i++) {
            MY_LOGD("S_%d: [deque_stt]dma:0x%x PA:%lx VA:%p Size(%d) mag:0x%x", _this->mSensorIdx,\
                    sttPortIDMap[portIdx].index,\
                    rDQBuf.mvOut.at(i).mPa, (void*)rDQBuf.mvOut.at(i).mVa, rDQBuf.mvOut.at(i).mSize,\
                    rDQBuf.mvOut.at(i).mMetaData.mMagicNum_tuning);
            rEQBuf.mvOut.at(i).mPortID  = sttPortIDMap[portIdx];
            rEQBuf.mvOut.at(i).mPa      = rDQBuf.mvOut.at(i).mPa;
            rEQBuf.mvOut.at(i).mVa      = rDQBuf.mvOut.at(i).mVa;
        }
        MY_LOGD("S_%d: [enque_stt] dma:0x%x", _this->mSensorIdx, sttPortIDMap[portIdx].index);
        _this->mpSttPipe->enque(rEQBuf);
    }

    ::sem_post(&_this->mSemSttProc[portIdx]);

    return;
}

MVOID* NPipeUT:: hwsync_loop(void* arg)
{
    NPipeUT*  _this = (NPipeUT*)arg;
    MINT32 ret = 0;
    IHalSensorList*     pHalSensorList;
    IHalSensor*         pSensorHalObj;
    MUINT32 index = 0;
    MUINT32 sof_idx;
    int i = 0, j = 0;
    MUINT32 sensorNum = 2;
    MUINT32 sensorSen[2] = {0,0};
    MUINT32 expTime[2], frmTime[2];
    MINT32 userKey = 0;
    MUINT32 *ptr_expTime = expTime;
    MUINT32 *ptr_frmTime = frmTime;
    MUINT32 senDev;
#if 1

    ::pthread_detach(::pthread_self());

    _this->m_aeCyclePeriod = HWSYNC_AE_CYCLE_PERIOD;


    pHalSensorList = IHalSensorList::get();
    pSensorHalObj = pHalSensorList->createSensor("HWSyncDrvThread", 0); //get sensor handler
    if (pSensorHalObj == NULL) {
        LOG_ERR("mpSensorHalObj is NULL");
        return NULL;
    }



    HWSyncDrv* pHwSyncDrv = HWSyncDrv::createInstance();
    pHwSyncDrv->init(HW_SYNC_USER_AE, 0, _this->m_aeCyclePeriod);

    while (!_this->m_bStop) {
        // wait vsync
        _this->mpNPipe->wait(EPipeSignal_VSYNC, EPipeSignal_ClearWait, userKey, 200);

        _this->mpNPipe->sendCommand(ENPipeCmd_GET_CUR_SOF_IDX, (MINTPTR)&sof_idx, 0, 0);
        LOG_INF("@SOF_%d, hwsync_loop", sof_idx);

        switch(i)
        {
            case 0:
                {
                    senDev = NSCam::SENSOR_DEV_MAIN;
                    expTime[0] = 30009;
                    expTime[1] = 30000;
                    LOG_DBG("senDev(%d), expTime[0]=%d, expTime[1]=%d, frmTime[0]=%d, frmTime[1]=%d",senDev, expTime[0],expTime[1],frmTime[0],frmTime[1]);
                    ret = pHwSyncDrv->sendCommand(HW_SYNC_CMD_GET_FRAME_TIME, senDev, (MUINTPTR)ptr_expTime, (MUINTPTR)ptr_frmTime);

                    if(ret > 0){
                    LOG_INF("[MAIN1] frmTime(%d) fps, expTime(%d) us", frmTime[0], expTime[0]);
                    LOG_INF("[MAIN2] frmTime(%d) fps, expTime(%d) us", frmTime[1], expTime[1]);
                    }
                    else{
                    LOG_ERR("sendCommand Fail");
                    }

                    if(frmTime[0] != 0) {
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_MAIN, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, \
                                                    (MINTPTR)&expTime[0],(MINTPTR)&frmTime[0], 0);

                    }
                    else{
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_MAIN, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_TIME, \
                                                    (MINTPTR)&expTime[0], 0, 0);
                    }

                    if(frmTime[1] != 0) {

                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_SUB, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_FRAME_TIME, \
                                                    (MINTPTR)&expTime[1],(MINTPTR)&frmTime[1], 0);
                    }
                    else{
                        pSensorHalObj->sendCommand(NSCam::SENSOR_DEV_SUB, \
                                                    SENSOR_CMD_SET_SENSOR_EXP_TIME, \
                                                    (MINTPTR)&expTime[1],0, 0);
                    }


                }
                break;
            default:
                break;
        }

        i++;

        if(i == _this->m_aeCyclePeriod)
            i = 0;

    }

    pHwSyncDrv->uninit(HW_SYNC_USER_AE, 0);
    pHwSyncDrv->destroyInstance();

    ::sem_post(&_this->m_semHwsyncThd);
#endif

    return NULL;
}

extern int test_camio(int argc, char** argv);
static void* open_another_cam(void* arg)
{
    NPipeUT*    _this = (NPipeUT*)arg;

    MUINT32     _sensorDev = 0;
    int         _argc = 0, i = 0;
    char**      _argv = NULL;

    _argc = _this->mArgc;

    MY_LOGD("switch cam: argc(%d)", _argc);

    _argv = (char **)malloc(sizeof(char *) * _argc);
    if (NULL == _argv) {
        MY_LOGD("switch cam: create another cam failed");
        goto _ANOTHER_FAIL;
    }
    memset(_argv, 0, sizeof(_argv));

    MY_LOGD("switch cam: copy paramters: ");

    for (i = 0; i < _argc; i++) {
        #define CAM_ARG_LEN         (32)
        MY_LOGD("argv[%d]=(%s)", i, _this->mArgv[i]);

        if (CMD_IDX_SNRDEVID == i) { // switch sensorDev idx
            MUINT32     _sensorDev2 = 0;

            _sensorDev = atoi(_this->mArgv[i]);

            _argv[i] = (char *)malloc(CAM_ARG_LEN);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            if(0 == _sensorDev){
                _sensorDev = UT_MAIN_FKSNR;
            }

            if (_sensorDev & UT_MAIN_FKSNR) {
                _sensorDev2 = UT_SUB_FKSNR;
            }
            if (_sensorDev & UT_SUB_FKSNR) {
                _sensorDev2 = UT_MAIN_FKSNR;
            }
            if (_sensorDev & UT_MAIN_SNR) {
                _sensorDev2 = UT_SUB_SNR;
            }
            if (_sensorDev & UT_SUB_SNR) {
                _sensorDev2 = UT_MAIN_SNR;
            }
            MY_LOGD("Old sensorDev: 0x%x New sensorDev: 0x%x", _sensorDev, _sensorDev2);

            snprintf(_argv[i], CAM_ARG_LEN, "%d", _sensorDev2);

        } else if (CMD_IDX_SWITCHCAM == i) { // off switchCam after switched
            _argv[i] = (char *)malloc(CAM_ARG_LEN);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            snprintf(_argv[i], CAM_ARG_LEN, "%d", 0);

        } else {

            int _size = strlen(_this->mArgv[i]) * sizeof(char);

            _argv[i] = (char *)malloc(_size);
            if (NULL == _argv[i]) {
                goto _ANOTHER_FAIL;
            }

            strncpy(_argv[i], _this->mArgv[i], _size);
        }

        MY_LOGD("    : (%s)", _argv[i]);
    }

    //MY_LOGD("switch cam: post for switch ready");
    //::sem_post(&_this->m_semSwitchThd);

    MY_LOGD("switch cam: open another camera ...");
    test_camio(_argc, _argv);

    MY_LOGD("switch cam: post for switch ready");
    ::sem_post(&_this->m_semSwitchThd);

_ANOTHER_FAIL:

    if (_argv) {
        MY_LOGD("free argv[] ...");

        for (i = 0; i < _argc; i++) {
            if (_argv[i]) {
                free(_argv[i]);
            }
        }

        free(_argv);
    }


    return NULL;
}

MVOID* NPipeUT::tg_loop(void* arg)
{
    NPipeUT*        _this = (NPipeUT*)arg;
    MUINT32         tglineno = 1000, nRet;
    MINT32          userKey = 1;
    MUINTPTR        bufAddr;

    MY_LOGD("start tg_loop");

    ::pthread_detach(::pthread_self());

    MUINT32 sdma = 0;
    PortID port;

    if(!_this->mpSttPipe){
        MY_LOGD("S_%d mpSttPipe=NULL\n", _this->mSensorIdx);
        goto EXIT_TG_LOOP;
    }

    //get any one stt opened port
    for (sdma = 0; sdma < __MAXDMAO_STT; sdma++) {
        if (_this->mEnableSttPort & (1 << sdma)) {
            break;
        }
    }
    if (sdma == __MAXDMAO_STT){
        MY_LOGD("S_%d no STT port enable\n", _this->mSensorIdx);
        goto EXIT_TG_LOOP;
    }

    port = sttPortIDMap[sdma];

    /*set TG interrupt*/
    MY_LOGD("S_%d set TG lineno(%d)\n", _this->mSensorIdx, tglineno);
    _this->mpSttPipe->sendCommand(ESPipeCmd_SET_TG_INT_LINE,(MINTPTR)tglineno, 0, 0);

    for ( ; MFALSE == _this->m_bStop; ) {
        /*wait tg irq*/
        MY_LOGD("S_%d wait TG interrupt\n", _this->mSensorIdx);
        nRet = _this->mpSttPipe->wait(EPipeSignal_TG_INT, EPipeSignal_ClearWait, userKey, 200);
        if(nRet == MFALSE) {
            MY_LOGD("S_%d wait TG int fail", _this->mSensorIdx);
            break;
        }
        /*get cur buf addr*/
        _this->mpSttPipe->sendCommand(ESPipeCmd_GET_STT_CUR_BUF,(MINTPTR)port, (MINTPTR)&bufAddr,0);
        MY_LOGD("S_%d: dma(x%x)cur_addr(%p)\n", _this->mSensorIdx, port.index, (MUINT8*)bufAddr);
    }

EXIT_TG_LOOP:

    MY_LOGD("S_%d exit tg_loop, stop", _this->mSensorIdx);
    ::sem_post(&_this->m_semTgIntThd);

    return NULL;
}

void TuningNotifyImp_EIS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
#define EIS_WIN_H   4
#define EIS_WIN_V   8
#define EIS_OFST_X  17
#define EIS_OFST_Y  17
#define MAX_STEP_H  16
#define MAX_STEP_V  8
#define EIS_XSIZE   0xFF
#define EIS_YSIZE   0x0
    EIS_INPUT_INFO  eis_in = *(EIS_INPUT_INFO*)pIn;
    LMV_CFG  lmv_cfg;
    EIS_REG_CFG     eis_cfg;
    MUINT32 win_h = 4, win_v = 8;
    MUINT32 ofst_x=0, ofst_y=0;
    MUINT32 FL_ofst_x = 0,FL_ofst_y=0;
    MUINT32 in_size_h, in_size_v;
    MUINT32 pix_mode = 0, bFG = MFALSE, eis_pipe_mode = 0;
    MUINT32 step_h = 16,step_v = 8, win_size_h, win_size_v, op_h=1, op_v=1;
    MUINT32 first_frm_flag = ((0 == mpNPipeUtObj->mDeqCount)? 1: 0);

    MY_LOGD("EIS info: 1stFrm(%d) RMXOut(%d,%d) HBinOut(%d,%d) TG(%d,%d) Fmt(x%x) PixMod(%d)",\
            first_frm_flag, eis_in.sRMXOut.w, eis_in.sRMXOut.h, eis_in.sHBINOut.w, eis_in.sHBINOut.h,\
            eis_in.sTGOut.w, eis_in.sTGOut.h, eis_in.bYUVFmt, eis_in.pixMode);

    //max window number
    win_h = (win_h > EIS_WIN_H)?(EIS_WIN_H):(win_h);
    win_v = (win_v > EIS_WIN_V)?(EIS_WIN_V):(win_v);

    //ofst,ofst have min constraint
    ofst_x = (ofst_x < EIS_OFST_X)?(EIS_OFST_X):(ofst_x);
    ofst_y = (ofst_y < EIS_OFST_Y)?(EIS_OFST_Y):(ofst_y);

    //floating ofset, current frame ofst from previous frame, normally set 0.

    pix_mode    = eis_in.pixMode; //TBD
    in_size_h   = eis_in.sHBINOut.w;
    in_size_v   = eis_in.sHBINOut.h;

    step_h = (step_h > MAX_STEP_H)?(MAX_STEP_H):(step_h);
    step_v = (step_v > MAX_STEP_V)?(MAX_STEP_V):(step_v);

    win_size_h = ((in_size_h/op_h) - (step_h*16) - ofst_x)/win_h;
    while(win_size_h < ((step_h + 1) * 16 + 2)){
        if( op_h> 1)
            op_h = op_h>>1;
        else{
            if(step_h > 1)
                step_h--;
            else{
                if(win_h > 1)
                    win_h -=1;
                else{
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("EIS H WINDOW SIZE FAIL\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    break;
                }
            }
        }
        win_size_h = ((((in_size_h - ofst_x)/op_h)/16) - step_h) / win_h ;
    }

    win_size_v = ((in_size_v/op_v) - (step_v*16) - ofst_y)/win_v;
    while(win_size_v < ((step_v + 1) * 16 + 2)){
        if(op_v > 1)
            op_v = op_v>>1;
        else{
            if(step_v > 1)
                step_v--;
            else{
                if(win_v > 1)
                    win_v -=1;
                else{
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("EIS V WINDOW SIZE FAIL\n");
                    MY_LOGD("#######################\n");
                    MY_LOGD("#######################\n");
                    break;
                }
            }
        }
        win_size_v = ((((in_size_v - ofst_y)/op_v)/16) - step_v) / win_v ;
    }

    eis_cfg.bEIS_Bypass = MFALSE;
    eis_cfg.bEIS_EN     = MTRUE;
    /* IN ISP4.0 ALWAYS USE GSE to replace SUBG_EN */
    /* 0x3550 TBD */ eis_cfg._EIS_REG.CTRL_1 = ((win_v<<28)|(win_h<<25)|(step_v<<21)|(0<<16/*knee1 RSTVAL*/)
                                |(0<<12/*knee2 RSTVAL*/)|(step_h<<8)|(0/*SUBG_EN*/<<5)|(op_v<<3)|(op_h));
    /* 0x3554 */ eis_cfg._EIS_REG.CTRL_2 = (((first_frm_flag)<<14) | 0x0000506C/*RSTVAL*/);
    /* 0x3558 TBD */ eis_cfg._EIS_REG.LMV_TH = 0/*RSTVAL*/;
    /* 0x355C */ eis_cfg._EIS_REG.FL_ofs = (((FL_ofst_x&0xFFF)<<16) | (FL_ofst_y&0xFFF));
    /* 0x3560 */ eis_cfg._EIS_REG.MB_ofs = (((ofst_x&0xFFF)<<16) | (ofst_y&0xFFF));
    /* 0x3564 */ eis_cfg._EIS_REG.MB_int = (((win_size_h&0xFFF)<<16)| (win_size_v&0xFFF));
    /* 0x3568 */ eis_cfg._EIS_REG.GMV    = 0; /* RO, GMV */
    /* 0x356C TBD */ eis_cfg._EIS_REG.ERR_CTRL = 0x000F0000/*RSTVAL*/;
    /* 0x3570 TBD*/ eis_cfg._EIS_REG.IMG_CTRL = ((eis_pipe_mode/*RSTVAL*/<<31)|((in_size_h&0x3FFF)<<16)
                                |(in_size_v&0x3FFF));

    lmv_cfg.bypassLMV = eis_cfg.bEIS_Bypass;
    lmv_cfg.enLMV = eis_cfg.bEIS_EN;
    lmv_cfg.cfg_lmv_prep_me_ctrl1 = eis_cfg._EIS_REG.CTRL_1;
    lmv_cfg.cfg_lmv_prep_me_ctrl2 = eis_cfg._EIS_REG.CTRL_2;
    lmv_cfg.cfg_lmv_lmv_th = eis_cfg._EIS_REG.LMV_TH;
    lmv_cfg.cfg_lmv_fl_offset = eis_cfg._EIS_REG.FL_ofs;
    lmv_cfg.cfg_lmv_mb_offset = eis_cfg._EIS_REG.MB_ofs;
    lmv_cfg.cfg_lmv_mb_interval = eis_cfg._EIS_REG.MB_int;
    lmv_cfg.cfg_lmv_gmv = eis_cfg._EIS_REG.GMV;
    lmv_cfg.cfg_lmv_err_ctrl = eis_cfg._EIS_REG.ERR_CTRL;
    lmv_cfg.cfg_lmv_image_ctrl = eis_cfg._EIS_REG.IMG_CTRL;

    *(LMV_CFG*)pOut = lmv_cfg;
}

void TuningNotifyImp_SGG2::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    SGG2_REG_CFG sgg2_cfg;
    (void)pIn;

    sgg2_cfg.bSGG2_Bypass   = 1; //TBD
    sgg2_cfg.bSGG2_EN       = 1; //TBD
    sgg2_cfg._SGG2_REG.PGN  = 0x10/*RSTVAL*/;
    sgg2_cfg._SGG2_REG.GMRC_1 = 0x10080402/*RSTVAL*/;
    sgg2_cfg._SGG2_REG.GMRC_2 = 0x00804020/*RSTVAL*/;

    //TBD CAM_UNI_TOP_MOD_EN |= SGG2_A_EN ???

    *(SGG2_REG_CFG*)pOut = sgg2_cfg;
}

void TuningNotifyImp_LCS::p1TuningNotify(MVOID* pIn, MVOID* pOut)
{
    LCS_INPUT_INFO  lcs_input = *(LCS_INPUT_INFO*)pIn;
    LCS_REG_CFG     lcs_cfg;
    MSize           lcs_in = queryLcsInSize(mpNPipeUtObj->mTgSize);
    MSize           lcs_out = queryLcsOutSize(mpNPipeUtObj->mTgSize);
    MUINT32         lrzr_x, lrzr_y;

#if 0//get HBIN info by sendCommand
    MUINT32 binW = 0;
    MUINT32 binH = 0;
    mpNPipeUtObj->mpNPipe->sendCommand(ENPipeCmd_GET_HBIN_INFO, (MINTPTR)(&binW), (MINTPTR)(&binH), 0);
    MY_LOGD("LCS HBIN info: %d_%d", binW,binH);
    if( binW <  lcs_in.w)
        lcs_in.w = binW;
    if( binH <  lcs_in.h)
        lcs_in.h = binH;
#else//get HBIN info by parameter
    lcs_in.w = lcs_input.sHBINOut.w;
    lcs_in.h = lcs_input.sHBINOut.h;
#endif

    lrzr_x = ((lcs_out.w-1)*1048576)/((lcs_in.w>>1)-1);
    lrzr_y = ((lcs_out.h-1)*1048576)/((lcs_in.h>>1)-1);

    MY_LOGD("LCS info: in_%d_%d out_%d_%d lrzr_%d_%d", lcs_in.w, lcs_in.h, lcs_out.w, lcs_out.h,
            lrzr_x, lrzr_y);

    lcs_cfg.bLCS_EN = MTRUE;
    lcs_cfg.bLCS_Bypass = MFALSE;
    lcs_cfg.u4LCSO_Stride = 0;
    lcs_cfg._LCS_REG.LCS_CON = (((lcs_out.h&0x1FF)<<20) | ((lcs_out.w&0x1FF)<<8));
    lcs_cfg._LCS_REG.LCS_ST  = 0; // LCS_START_I=0, LCS_START_J=0
    lcs_cfg._LCS_REG.LCS_AWS = (((lcs_in.h&0x1FFF)<<16) | (lcs_in.w&0x1FFF));
    lcs_cfg._LCS_REG.LCS_FLR = 0x00010000;/*RSTVAL*/
    lcs_cfg._LCS_REG.LCS_LRZR_1 = (lrzr_x & 0xFFFFF);
    lcs_cfg._LCS_REG.LCS_LRZR_2 = (lrzr_y & 0xFFFFF);
    lcs_cfg._LCS_REG.LCS_SATU1 = 0x0FFF0FFF;//default
    lcs_cfg._LCS_REG.LCS_SATU2 = 0x0FFF0FFF;//default
    lcs_cfg._LCS_REG.LCS_GAIN_1 = 0x02000200;//default
    lcs_cfg._LCS_REG.LCS_GAIN_2 = 0x02000200;//default
    lcs_cfg._LCS_REG.LCS_OFST_1 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_OFST_2 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_1 = 0x00000200;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_2 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_3 = 0x00000200;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_4 = 0x0;//default
    lcs_cfg._LCS_REG.LCS_G2G_CNV_5 = 0x00090200;//default
    lcs_cfg._LCS_REG.LCS_LPF = 0;

    *(LCS_REG_CFG*)pOut = lcs_cfg;
}

MSize TuningNotifyImp_LCS::queryLcsOutSize(MSize TgSize)
{
    (void)TgSize;
    return MSize(8, 8);
}

MSize TuningNotifyImp_LCS::queryLcsInSize(MSize TgSize)
{
    return MSize(TgSize);
}

//MBOOL getSensorPixelMode(MUINT32* pPixelMode,MUINT32 sensorIdx,MUINT32 scenario,MUINT32 sensorFps)
//{
//    IHalSensor* pSensorHalObj = NULL;
//    IHalSensorList* const pHalSensorList = IHalSensorList::get();
//
//    if( !pHalSensorList ) {
//        MY_LOGE("error:pHalSensorList == NULL");
//        return MFALSE;
//    }
//
//    pSensorHalObj = pHalSensorList->createSensor(
//            mThisName,
//            sensorIdx);
//
//    if (pSensorHalObj == NULL) {
//        MY_LOGE("error:pSensorHalObj is NULL");
//        return MFALSE;
//    }
//
//    pSensorHalObj->sendCommand(
//            pHalSensorList->querySensorDevIdx(sensorIdx),
//            SENSOR_CMD_GET_SENSOR_PIXELMODE,
//            (MINT32)&scenario,
//            (MINT32)&sensorFps,
//            (MINT32)pPixelMode);
//    MY_LOGD("sensorScenario(%d),sensorFps(%d),pixelMode(%d)",
//            scenario,
//            sensorFps,
//            *pPixelMode);
//
//    pSensorHalObj->destroyInstance(mThisName);
//
//    return MTRUE;
//}


int getSensorSize (MSize* pSize, SensorStaticInfo mSensorInfo, MUINT32 SenScenario)
{
    MBOOL ret = MTRUE;

#define scenario_case(scenario, KEY, pSize)       \
        case scenario:                            \
            (pSize)->w = mSensorInfo.KEY##Width;  \
            (pSize)->h = mSensorInfo.KEY##Height; \
            break;

    switch(SenScenario)
    {
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, pSize );
        default:
            MY_LOGD("not support sensor scenario(0x%x)\n", SenScenario);
            ret = MFALSE;
            break;
    }
#undef scenario_case

    return ret;
}

MUINT32 getPortBitDepth(PortID _port, MUINT32 PortEnableMap)
{
    MUINT32 bitDepth = 0;

    if (_port.index == PORT_IMGO.index) {
        bitDepth = (PortEnableMap & ((__IMGO_ENABLE) << 8))? 12: 10;
    }
    else if (_port.index == PORT_RRZO.index) {
        bitDepth = (PortEnableMap & ((__RRZO_ENABLE) << 8))? 12: 10;
    }
    else {
        MY_LOGE("Unsupported port depth query");
    }

    return bitDepth;
}

MBOOL getOutputFmt(PortID port, MUINT32 bitDepth, SensorStaticInfo& mSensorInfo, EImageFormat* pFmt)
{
    MBOOL ret = MFALSE;

#define case_Format( order_bit, mappedfmt, pFmt) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
        break;

    if ( mSensorInfo.sensorType == SENSOR_TYPE_YUV ) {
        switch( mSensorInfo.sensorFormatOrder ) {
        case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, pFmt);
        case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, pFmt);
        default:
            MY_LOGE("formatOrder not supported, 0x%x", mSensorInfo.sensorFormatOrder);
            goto lbExit;
        }
        //printf("sensortype:(0x%x), fmt(0x%x)\n", mSensorInfo.sensorType, *pFmt);
    }
    else if ( mSensorInfo.sensorType == SENSOR_TYPE_RAW ) {
        if (port.index == PORT_IMGO.index) {
            switch (bitDepth) {
            case_Format(  8, eImgFmt_BAYER8 , pFmt);
            case_Format( 10, eImgFmt_BAYER10, pFmt);
            case_Format( 12, eImgFmt_BAYER12, pFmt);
            case_Format( 14, eImgFmt_BAYER14, pFmt);
            default:
                MY_LOGE("IMGO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_RRZO.index) { //rrzo
            switch( bitDepth) {
            case_Format(  8, eImgFmt_FG_BAYER8 , pFmt);
            case_Format( 10, eImgFmt_FG_BAYER10, pFmt);
            case_Format( 12, eImgFmt_FG_BAYER12, pFmt);
            case_Format( 14, eImgFmt_FG_BAYER14, pFmt);
            default:
                MY_LOGE("RRZO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if (port.index == PORT_UFEO.index) { //ufeo
            switch( bitDepth) {
            case_Format( 10, eImgFmt_UFEO_BAYER10, pFmt);
            case_Format( 12, eImgFmt_UFEO_BAYER12, pFmt);
            default:
                MY_LOGE("UFEO bitdepth not supported, 0x%x", bitDepth);
                goto lbExit;
            }
        }
        else if ((port.index == PORT_EISO.index) ||
                (port.index == PORT_LCSO.index)) {
            MY_LOGE("port not supported, 0x%x", port.index);
            goto lbExit;
        }
    }
    else {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorInfo.sensorType);
        goto lbExit;
    }

    ret = MTRUE;

#undef case_Format

lbExit:
    return ret;
}


/*******************************************************************************
*  Main Function
********************************************************************************/
int test_camio(int argc, char** argv)
{
    char                filename[256];
    MUINT32             ret = 0, j = 0, sidx = 0, dma = 0, bSetSensorTP = 0;
    MUINT32             sensorDev, scenario, sensorFps, enablePort, croptestsel, sensorNum = 0, sttPorts = 0;
    MUINT32             pixelMode[2] = {0}, stopFlow = 0;
    MUINT32             twinCtrl = 1, sensorPixMode = 1, forceBinOff = 0,Dual_PD = 0;
    uint32_t            sensorArray[_MAXTG_];
    SensorStaticInfo    mSensorInfo[_MAXTG_];
    IHalSensor*         pSensorHalObj[_MAXTG_] = {NULL};
    vector<IHalSensor::ConfigParam> vSensorCfg[2];
    IHalSensorList*     pHalSensorList = NULL;
    IMEM_BUF_INFO       imgiBuf;
    NPipeUT*            pUt[_MAXTG_] = {NULL};
    MUINT32             _addtional = 0;
    MBOOL               stN3D, _switchCam = MFALSE, _efuse = MFALSE, waitTgInt = MFALSE;
    MINT32              _efuse_idx = BD_SEG_NUM - 1;
#ifndef HWSYNC_TEST
    MUINT32             _frameNum = 8;
#else
    MUINT32             _frameNum = 64;
#endif
    MUINT32             _burstQNum = 1;
    pthread_t           _switchThd;

#if 0//dbg only
#include "../../drv/inc/isp_drv_cam.h"//for cfg the test pattern of seninf only
    IspDrvImp *m_pIspDrv = (IspDrvImp*)IspDrvImp::createInstance(CAM_A);
    if (!m_pIspDrv->init("iopipeTest")) {
        MY_LOGE("isp drv init fail");
        return 0;
    }
    m_pIspDrv->setRWMode(ISP_DRV_RW_MMAP);
    m_pIspDrv->writeReg(0x0510, 0x10);
#endif

    MY_LOGD("sizeof long : %d", (int)sizeof(long));
    if (argc < 6) { // iopipeTest 1 0 0 30 2 0, test patter no crop
        MY_LOGI("Param: 1 <sensorDev> <scenario> <sensorFps> <enablePort> <cropTestSel>"\
            "<sttPorts> <burstQNum> <FrameNum> <stopFlow> <switchCam> <twinCtrl> <additionalcmds>");
        MY_LOGI("<sensorDev>    : 1 main, 2 sub, bitmap; 0 for seninf test model");
        MY_LOGI("<scenario>     : preview %d, capture %d, video %d, slim1 %d, slim2 %d, sensorTestPattern 255",\
                SENSOR_SCENARIO_ID_NORMAL_PREVIEW, SENSOR_SCENARIO_ID_NORMAL_CAPTURE,\
                SENSOR_SCENARIO_ID_NORMAL_VIDEO, SENSOR_SCENARIO_ID_SLIM_VIDEO1, SENSOR_SCENARIO_ID_SLIM_VIDEO2);
        MY_LOGI("<sensorFps>    : 30 for 30fps");
        MY_LOGI("<enablePort>   : 0x01 rrzo, 0x02 imgo, 0x04 eiso, 0x08 lcso, 0x10 rrzo+ufeo bitmap, 0x100");
        MY_LOGI("               : 0x10000 rrzo with 4k2k size, 0x20000 imgo pure raw");
        MY_LOGI("<cropTestSel>  : 0 no crop, 1 symmetric crop, 2 arbitrary crop");
        MY_LOGI("<sttPorts>     : 1 aao, 2 afo, 4 flko, 8 pdo, 8192 camsv imgo, bitmap");
        MY_LOGI("<burstQNum>    : 1 for normal");
        MY_LOGI("<FrameNum>     : 1 for 1 frame");
        MY_LOGI("<stopFlow>     : 0 : blocking stop, thread 1~2 vsync, cam 1 vsync");
        MY_LOGI("               : 1 : non-blocking stop, thread 0 vsync, cam 1 vsync");
        MY_LOGI("               : 2 : abort, no wait");
        MY_LOGI("<switchCam>    : 0 : no, 1 : after uninit switch to another cam");
        MY_LOGI("<twinCtrl>     : 0x01 1-pix, 0x02 2-pix");
        MY_LOGI("               : 0x10 : bin force off");
        MY_LOGI("               : 0x20 : Dbin force on");
        MY_LOGI("<additional>   : 0 : no, 1: efuse test , 2: tg interrupt test");
        return -1;
    }

    sensorDev = atoi(argv[CMD_IDX_SNRDEVID]);
    scenario  = atoi(argv[CMD_IDX_SNRSCEN]);
    sensorFps = atoi(argv[CMD_IDX_SNRFPS]);
    enablePort = atoi(argv[CMD_IDX_ENABLEPORTS]);
    croptestsel = atoi(argv[CMD_IDX_CROPSEL]);

    if (argc > CMD_IDX_STTPORTS) {
        sttPorts = atoi(argv[CMD_IDX_STTPORTS]);
    }
    if (argc > CMD_IDX_BURSTNUM) {
        _burstQNum = atoi(argv[CMD_IDX_BURSTNUM]);
    }
    else {
        _burstQNum = 1;
    }
    if (argc > CMD_IDX_FRAMENUM) {
        _frameNum = atoi(argv[CMD_IDX_FRAMENUM]);
    }
    if (argc > CMD_IDX_STOPFLOW) {
        /*
         * 0 : blocking stop, as-is 2 ~ 3 vsync
         * 1 : non-blocking stop, wait 1 vsync
         * 2 : abort, no wait
         */
        stopFlow = atoi(argv[CMD_IDX_STOPFLOW]);
        MY_LOGD("stopFlow : %d", stopFlow);

        if (stopFlow > 2) {
            stopFlow = 1;
            MY_LOGD("stopFlow set %d", stopFlow);
        }
    }
    if (argc > CMD_IDX_SWITCHCAM) {
        _switchCam = !!atoi(argv[CMD_IDX_SWITCHCAM]);
    }
    if (argc > CMD_IDX_PIXELMODE) {
        twinCtrl = atoi(argv[CMD_IDX_PIXELMODE]);
        sensorPixMode = twinCtrl & 0xF;
        forceBinOff = (twinCtrl & 0x10) >> 4;
        Dual_PD = (twinCtrl&0x20) >> 5;

        if(Dual_PD){
            if(sensorDev != 0){
                MY_LOGE("dbn support only in fake sensor\n");
                return 1;
            }
            if(scenario != SENSOR_SCENARIO_ID_NORMAL_PREVIEW ){
                MY_LOGE("dbn support only in preview, no capture\n");
                return 1;
            }
        }
        MY_LOGD("twinCtrl : %d", twinCtrl);
    }
    if (argc > CMD_IDX_ADDTIONAL) {
        _addtional = atoi(argv[CMD_IDX_ADDTIONAL]);
        if(_addtional&_test_efuse) {
            _efuse = MTRUE;
            scenario = 1;
            sensorDev = UT_MAIN_FKSNR;
            MY_LOGD("eFuse test: sce_%d, sdev_%d", scenario, sensorDev);
        }
        //
        if(_addtional&_test_tgint){
            waitTgInt = 1;
        }
    }

    MY_LOGD("sttPorts 0x%08x", sttPorts);
    if (sttPorts) {
        if (sttPorts & __AAO_ENABLE) {
            MY_LOGD("enable aao");
        }
        if (sttPorts & __AFO_ENABLE) {
            MY_LOGD("enable afo");
        }
        if (sttPorts & __FLKO_ENABLE) {
            MY_LOGD("enable FLKO");
        }
        if (sttPorts & __CAMSV_IMGO_ENABLE) {
            MY_LOGD("output CAMSV IMGO");
        }
        if(sttPorts & __PDO_ENABLE){
            MY_LOGD("enable PDO\n");
        }
    }

    sensorNum = 0;
    if (0 == sensorDev) {
        sensorDev = UT_MAIN_FKSNR;
    }
    if (sensorDev & (UT_MAIN_SNR | UT_MAIN_FKSNR)) {
        if (sensorNum >= _MAXTG_) {
            MY_LOGD("sensor number is more than maximum tg number");
            return -1;
        }
        sensorArray[sensorNum] = 0;
        sensorNum++;
    }
    if (sensorDev & (UT_SUB_SNR | UT_SUB_FKSNR)) {
        if (sensorNum >= _MAXTG_) {
            MY_LOGD("sensor number is more than maximum tg number");
            return -1;
        }
        sensorArray[sensorNum] = 1;
        sensorNum++;
    }
    if (sensorDev & (UT_MAIN2_SNR | UT_MAIN2_FKSNR)) {
        if (sensorNum >= _MAXTG_) {
            MY_LOGD("sensor number is more than maximum tg number");
            return -1;
        }
        sensorArray[sensorNum] = 2;
        sensorNum++;
    }
    if (sensorDev & (UT_MAIN_FKSNR | UT_SUB_FKSNR | UT_MAIN2_FKSNR)) {
        mThisName = (char*)NPIPE_TM_KEY;
    }
    if (_switchCam) {
        MY_LOGD("switch cam");
        if (sensorNum > 1) {
            MY_LOGD("switch cam need config only one cam a time");
            return -1;
        }
    }

#ifdef HWSYNC_TEST
    if(sensorDev & (UT_MAIN_SNR || UT_SUB_SNR))
        m_bN3DTest = true;
    else
        m_bN3DTest = false;
#endif

    if (255 == scenario) {
        MY_LOGD("using sensor test pattern");
        scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        bSetSensorTP = 1;
    }

    MY_LOGD("sensorDev_0x%x senario_0x%x sensorFps_%d enablePort_0x%x sensorNum_0x%x croptestsel_0x%x sttPort_0x%x",\
            sensorDev, scenario, sensorFps, enablePort, sensorNum, croptestsel, sttPorts);
    MY_LOGD("supported_bit_%d/12 rrz_ratio_%d/%d", SEN_PIX_BITDEPTH, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M);
    MY_LOGD("sensorArray: %d, %d", sensorArray[0], sensorArray[1]);
    MY_LOGD("burstNum: %d", _burstQNum);
    MY_LOGD("twinCtrl: %d , pix/bin_off/DBN(%d/%d/%d), ", twinCtrl, sensorPixMode, forceBinOff,Dual_PD);
    MY_LOGD("efuse: %d, waitTgInt: %d", _efuse, waitTgInt);
    /*
     * sensor create/query
     */
    if (sensorDev & UT_SNR_MASK) {
        pHalSensorList = IHalSensorList::get();
    }
    else {
        pHalSensorList = TS_FakeSensorList::getTestModel();
    }

SEARCH_SENSOR:

    MY_LOGD("search sensor...");
    pHalSensorList->searchSensors();

    for (sidx = 0; sidx < sensorNum; sidx++) {
        if(sidx < 0 || sidx >= _MAXTG_){
            MY_LOGD("sidx:%u is out of the range of the array", sidx);
            break;
        }

        pUt[sidx] = NPipeUT::create();
        pUt[sidx]->mEnableSttPort = sttPorts; //modify for uni
        pUt[sidx]->mCropTestSel = croptestsel;
        pUt[sidx]->mStaticEnqCnt = 2; //should > 0
        pUt[sidx]->mFrameNum = _frameNum;
        pUt[sidx]->mBurstQNum = _burstQNum;
        pUt[sidx]->mPortBufDepth = _burstQNum * ENQUE_BURST_CNT;
        pUt[sidx]->mArgc = argc;
        pUt[sidx]->mArgv = argv;
        pUt[sidx]->mEnableTgInt = waitTgInt;
        pUt[sidx]->m_Dualpd = Dual_PD;

        ::sem_init(&pUt[sidx]->m_semSwitchThd, 0, 0);

        if(enablePort & __UFEO_ENABLE){
            pUt[sidx]->m_ufeo_en = 1;
            enablePort ^= __UFEO_ENABLE;
            enablePort |= __RRZO_ENABLE;
        }
        else{
            pUt[sidx]->m_ufeo_en = 0;
        }
        //uni check
        if ((enablePort & __EISO_ENABLE) && (sidx > 0)) {
            pUt[sidx]->m_enablePort = (enablePort & ~__EISO_ENABLE); //uni
        }
        else {
            pUt[sidx]->m_enablePort = enablePort; //modify for uni
        }
        if ((sttPorts & __FLKO_ENABLE) && (sidx > 0)) {
            pUt[sidx]->mEnableSttPort = (sttPorts & ~__FLKO_ENABLE);
        }
        //rrzo 4k2k output
        if(enablePort & __RRZO_4K2K_ENABLE) {
            if(scenario != 1 && scenario != 255) {
                scenario = 1;
                MY_LOGD("%d scenario change to CAPTURE for output 4k2k", sidx);
            }
            if (scenario == 1) {
                pUt[sidx]->m_b4k2k = MTRUE;
                enablePort & ~ __RRZO_4K2K_ENABLE;
                MY_LOGD("%d rrzo output 4k2k", sidx);
            }
        }

        if (pUt[sidx]->mStaticEnqCnt > ENQUE_BURST_CNT) {
            pUt[sidx]->mStaticEnqCnt = ENQUE_BURST_CNT;
        }

        MY_LOGD("create %d obj : [%d] %p", sensorNum, sidx, (MUINT8*)pUt[sidx]);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pHalSensorList->querySensorStaticInfo(
                pHalSensorList->querySensorDevIdx(sensorArray[sidx]), &mSensorInfo[sidx]);
    }

    /*
     * power-on sensor
     */
    for (sidx = 0; sidx < sensorNum; sidx++) {
        pSensorHalObj[sidx] = pHalSensorList->createSensor(mThisName, sensorArray[sidx]);
        if (pSensorHalObj[sidx] == NULL) {
            MY_LOGE("mpSensorHalObj is NULL");
            return -1;
        }

        if (!(sensorDev & UT_SNR_MASK)) {
            ((TS_FakeSensor*)pSensorHalObj[sidx])->setPixelMode(sensorPixMode);
        }

        //sensorArray[sidx] = sidx;
        pSensorHalObj[sidx]->powerOn(mThisName, 1, &sensorArray[sidx]);
    }

    MY_LOGD("create...");
    /*
     * NormalPipe create
     */
    for (sidx = 0; sidx < sensorNum; sidx++) {
        MY_LOGD("SenIdx: %d", sensorArray[sidx]);

        pUt[sidx]->mSensorIdx = sensorArray[sidx];

        getNormalPipeModule()->createSubModule(
            sensorArray[sidx], mThisName, selectNormalPipeVersion(sensorArray[sidx]), (MVOID**)&(pUt[sidx]->mpNPipe));

        ret = pUt[sidx]->mpNPipe->init();
        if (MFALSE == ret) {
            MY_LOGE("normalPipe init error");
            return -1;
        }
    }

    if(_efuse){
        MY_LOGD("BOUNDING_%d_%s [%dx%d] ", _efuse_idx, \
            efTable[_efuse_idx].resultStr, \
            efTable[_efuse_idx].TG_grabW, \
            efTable[_efuse_idx].TG_grabH);
    }
    MY_LOGD("prepare sensor config...");
    /*
     * prepare sensor config
     */
    for (sidx = 0; sidx < sensorNum; sidx++) {
        getSensorSize(&pUt[sidx]->mTgSize, mSensorInfo[sidx], scenario);
        if(_efuse) {
            pUt[sidx]->m_bEfuse = MTRUE;
            pUt[sidx]->mEfuseIdx = _efuse_idx;
            pUt[sidx]->mTgSize.w = efTable[_efuse_idx].TG_grabW;
            pUt[sidx]->mTgSize.h = efTable[_efuse_idx].TG_grabH;
        }

        IHalSensor::ConfigParam sensorCfg =
        {
            (MUINT)pUt[sidx]->mSensorIdx,   /* index            */
            pUt[sidx]->mTgSize,             /* crop             */
            scenario,                       /* scenarioId       */
            0,                              /* isBypassScenario */
            1,                              /* isContinuous     */
            MFALSE,                         /* iHDROn           */
            sensorFps,                      /* framerate        */
            0,                              /* two pixel on     */
            0,                              /* debugmode        */
        };
        sensorCfg.twopixelOn = (sensorPixMode & 0x2) >> 1; // JSS, pixel mode

        vSensorCfg[sidx].push_back(sensorCfg);



        MY_LOGD("senidx:0x%x, sensor %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d, twopxl %d",
                pUt[sidx]->mSensorIdx, sensorCfg.crop.w, sensorCfg.crop.h, sensorCfg.scenarioId, sensorCfg.isBypassScenario,
                sensorCfg.isContinuous, sensorCfg.HDRMode, sensorCfg.framerate, sensorCfg.twopixelOn);
    }

    MY_LOGD("dmao cfg...");

    EImageFormat fmt;
    MSize _cropsize;
    list<HwPortConfig_t> lHwPortCfg[2];

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pSensorHalObj[sidx]->sendCommand(
                pHalSensorList->querySensorDevIdx(pUt[sidx]->mSensorIdx),
                SENSOR_CMD_GET_SENSOR_PIXELMODE,
                (MUINTPTR)&scenario,
                (MUINTPTR)&sensorFps,
                (MUINTPTR)&pixelMode[sidx]);

        //e_PixMode[sidx] =  ((pixelMode[sidx] == 0) ?  (NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE) : (NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE));

        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            NormalPipe_QueryInfo qry;

            MY_LOGD("0x%x: port IMGO", sidx);

            if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                MY_LOGE("get pix fmt error");
                return -1;
            }

            getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, pUt[sidx]->mTgSize.w, qry);

            //_cropsize = __CROP_SIZE(__IMGO,fmt,pUt[sidx]->mTgSize,e_PixMode[sidx]);
            _cropsize = pUt[sidx]->mTgSize;

            MY_LOGD("imgo query x_pix:(%d) x_strid_byte:(%d)", qry.x_pix, qry.stride_byte);
            MY_LOGD("sensor size width:(%d) x height:(%d)", pUt[sidx]->mTgSize.w, pUt[sidx]->mTgSize.h);
            MY_LOGD("crop size width:(%d) x height:(%d)", _cropsize.w, _cropsize.h);

            {
                HwPortConfig_t full = {
                    PORT_IMGO,
                    fmt,
                    _cropsize,
                    MRect(__CROP_START(PORT_IMGO, pUt[sidx]->mTgSize, _cropsize, fmt, 0,0), _cropsize),
                    (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW)?1:0, //if raw type != 1 -> pure-raw
                    { qry.stride_byte, 0, 0 } //mStrideInByte[3]
                };

                lHwPortCfg[sidx].push_back(full);
            }
        }

        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            NormalPipe_QueryInfo qry;
            MSize _scaled_size;
            if(pUt[sidx]->m_ufeo_en == 1){
                 if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return -1;
                 }
            }
            else{
                if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return -1;
                }
            }
            _scaled_size = __SCALE_PIX_SIZE(fmt, pUt[sidx]->mTgSize, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M, pUt[sidx]->m_b4k2k);

            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                fmt, _scaled_size.w, qry);

            //crop size can't < outsize
            //__CROP_SIZE(sensorSize) must > _size in rrzo
            _cropsize = __CROP_SIZE(PORT_RRZO, fmt, pUt[sidx]->mTgSize, pUt[sidx]->mCropTestSel,0, pUt[sidx]->m_b4k2k);

            if((_scaled_size.w > _cropsize.w) | (_scaled_size.h > _cropsize.h)){
                _scaled_size = _cropsize;
            }

            {
                HwPortConfig_t resized = {
                    PORT_RRZO,
                    fmt,
                    _scaled_size,
                    MRect(__CROP_START(PORT_RRZO, pUt[sidx]->mTgSize, _cropsize, fmt, pUt[sidx]->mCropTestSel,0), _cropsize),
                    0, //if raw type != 1 -> pure-raw
                    { qry.stride_byte, 0, 0 }
                };

                lHwPortCfg[sidx].push_back(resized);
            }

            MY_LOGD("rrzo query x_pix:(%d) x_strid_byte:(%d)", qry.x_pix, qry.stride_byte);
            MY_LOGD("scale_size: %d_%d", _scaled_size.w, _scaled_size.h);
            MY_LOGD("crop size width:(%d) x height:(%d)", _cropsize.w, _cropsize.h);
        }
        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            MSize _ssize(EISO_SIZE, 1);
            HwPortConfig_t eiso_cfg = {
                PORT_EISO,
                eImgFmt_BLOB,
                _ssize,
                MRect(MPoint(0, 0), _ssize),
                0,
                { (MUINT32)_ssize.w, 0, 0 }
            };

            MY_LOGD("0x%x: port EISO", sidx);

            lHwPortCfg[sidx].push_back(eiso_cfg);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            MSize _ssize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);
            HwPortConfig_t lcso_cfg = {
                PORT_LCSO,
                eImgFmt_BLOB,
                _ssize,
                MRect(MPoint(0, 0), _ssize),
                0,
                { (MUINT32)_ssize.w, 0, 0 }
            };

            MY_LOGD("0x%x: port LCSO, size_%d_%d", sidx, _ssize.w, _ssize.h);

            lHwPortCfg[sidx].push_back(lcso_cfg);
        }

        MY_LOGD("%d hwport size:%d", sidx, (MUINT32)lHwPortCfg[sidx].size());
    }

    /*
     * NormalPipe configPipe
     */
    MY_LOGD("configPipe...");

    vector<portInfo> vPortInfo[2];
    list<HwPortConfig_t>::const_iterator pPortCfg;

    for (sidx = 0; sidx < sensorNum; sidx++) {
        for (pPortCfg = lHwPortCfg[sidx].begin(); pPortCfg != lHwPortCfg[sidx].end(); pPortCfg++) {
            portInfo OutPort(
                    pPortCfg->mPortID,
                    pPortCfg->mFmt,
                    pPortCfg->mSize, //dst size
                    pPortCfg->mCrop, //crop
                    pPortCfg->mStrideInByte[0],
                    pPortCfg->mStrideInByte[1],
                    pPortCfg->mStrideInByte[2],
                    pPortCfg->mPureRaw, // pureraw
                    MTRUE               //packed
                    );
            vPortInfo[sidx].push_back(OutPort);

            MY_LOGD("configPipe: 0x%x id:0x%x, crop:%d,%d,%dx%d, size:%dx%d, fmt:0x%x, stride:%d, pureraw:%d",\
                    sidx,\
                    pPortCfg->mPortID.index,\
                    pPortCfg->mCrop.p.x,pPortCfg->mCrop.p.y,pPortCfg->mCrop.s.w,pPortCfg->mCrop.s.h,\
                    pPortCfg->mSize.w,pPortCfg->mSize.h,\
                    pPortCfg->mFmt,\
                    pPortCfg->mStrideInByte[0],\
                    pPortCfg->mPureRaw);
        }

        if(sensorNum == 2){
            stN3D = MTRUE;
            //stN3D.m_N3D_CLK = _HIGH_;
            //stN3D.m_Shutter = 30009;
        }
        else{
            stN3D = MFALSE;
        }

        QInitParam param(
        0, // 2: sensor uses pattern
        SEN_PIX_BITDEPTH,
        vSensorCfg[sidx],
        vPortInfo[sidx],
        MTRUE,
        stN3D); // N3D

        param.m_bOffBin = forceBinOff; // force bin off
        if(Dual_PD)
            param.m_Func.Bits.DATA_PATTERN = EPipe_Dual_pix;
        else
            param.m_Func.Bits.DATA_PATTERN = EPipe_Normal;

        MY_LOGD("m_bOffBin:%d, param.m_bOffBin:%d", param.m_bOffBin, forceBinOff);

        if (!pUt[sidx]->mpNPipe->configPipe(param, pUt[sidx]->mBurstQNum)) {
            MY_LOGE("senidx:0x%x configPipe failed", sidx);
            return -1;
        }


        if (bSetSensorTP) {
            MINT32 u32Enable = 1;

            MY_LOGD("Set sensor test pattern");
            pSensorHalObj[sidx]->sendCommand(
                    pHalSensorList->querySensorDevIdx(pUt[sidx]->mSensorIdx),
                    SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                    (MUINTPTR)&u32Enable,
                    0,
                    0);
        }

        #if (TEST_ZHDR_HW == 1)
        if (1)
        #else
        if (pUt[sidx]->mEnableSttPort)
        #endif
        {
            pUt[sidx]->tuningMgr = (TuningMgr*)TuningMgr::getInstance(pUt[sidx]->mSensorIdx);
            if (MFALSE == pUt[sidx]->tuningMgr->init(mThisName, pUt[sidx]->mBurstQNum)) {
                MY_LOGE("tuningMgr init error");
                return -1;
            }
        }

        if (pUt[sidx]->mEnableSttPort) {
            MUINT32 sdma = 0;
            std::vector<statPortInfo> vp;
            QInitStatParam statParm(vp);

            pUt[sidx]->mpSttPipe = IStatisticPipe::createInstance(pUt[sidx]->mSensorIdx, mThisName);
            if (MFALSE == pUt[sidx]->mpSttPipe->init()) {
                MY_LOGE("statisticPipe init error");
                return -1;
            }

            if (pUt[sidx]->mEnableSttPort & __AAO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AAO));
            }
            if (pUt[sidx]->mEnableSttPort & __AFO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_AFO));
            }
            if (pUt[sidx]->mEnableSttPort & __FLKO_ENABLE) {
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_FLKO));
            }
            if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
                //use full TG size here is just easiest example,
                //theoratically, PDO size msut be smaller than TG size,
                //e.g.: PDO width = 1/8 TG_W,  PDO height = 1/4 TG_H
                statParm.mStatPortInfo.push_back(statPortInfo(PORT_PDO,pUt[sidx]->mTgSize.w,pUt[sidx]->mTgSize.h));
            }

            MY_LOGD("stt configPipe");
            pUt[sidx]->mpSttPipe->configPipe(statParm);
        }

        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            MY_LOGD("Reg SGG2 CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_SGG2_CBFP, (MINTPTR)&pUt[sidx]->mSgg2Notify, 0, 0);
            MY_LOGD("Reg EIS CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_EIS_CBFP, (MINTPTR)&pUt[sidx]->mEisNotify, 0, 0);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            MY_LOGD("Reg LCS CBFP");
            pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_SET_LCS_CBFP, (MINTPTR)&pUt[sidx]->mLcsNotify, 0, 0);
        }
    }


    if (sttPorts & __CAMSV_IMGO_ENABLE) {
        MBOOL bRet = MFALSE;
        MUINT32 argu1 = 0;
        int f;

        do {
            MY_LOGD("#############################################");
            MY_LOGD("# Please select 1 feature: PDAF(0)/mVHDR(1) #");
            MY_LOGD("#############################################");
            f = getchar();
            if (f == '1' || f =='0') {
                f = f - '0';
                break;
            }
        }while(1);

        MY_LOGD("check if support virtual channel...");
        ICamsvStatisticPipe*        mpCamsvSttPipe;
        mpCamsvSttPipe = ICamsvStatisticPipe::createInstance(sensorArray[0], LOG_TAG , f); // alwasy use main
        if( !mpCamsvSttPipe ) {
            MY_LOGD("ICamsvStatisticPipe createInstance fail");
            return MFALSE;
        }
        if (MFALSE == mpCamsvSttPipe->init()) {
            MY_LOGD("ICamsvStatisticPipe init fail");
            return MFALSE;
        }
#if 0
        /* imx258mipiraw_Sensor.c, wxh=0x0_0 */
        argu1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        bRet = mpCamsvSttPipe->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&argu1, NULL,NULL);
#else
        /* imx258mipiraw_Sensor.c, wxh=0xa0_0x780 */
        argu1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        bRet = mpCamsvSttPipe->sendCommand(ECAMSVSPipeCmd_SET_CAMSV_SENARIO_ID, (MINTPTR)&argu1, NULL,NULL);
#endif

        std::vector<statPortInfo> vp;
        QInitStatParam statParm(vp);
        mpCamsvSttPipe->configPipe(statParm);

#if 1 // test enque & deque flow

// should be same as CamsvStatisticPipe.h
#define CAMSV_EQ_DQ_ONCE_IN_CONFIGPIPE  (0)

#if !CAMSV_EQ_DQ_ONCE_IN_CONFIGPIPE
        mpCamsvSttPipe->start();
#endif

        for(MUINT32 i=0; i<5; i++){

            MY_LOGD("################################################################################## i(%d)", i);

#if CAMSV_EQ_DQ_ONCE_IN_CONFIGPIPE
            // enque
            {
                MY_LOGD("### ENQUE ###");
                QBufInfo _eq_rQBufInfo;
                _eq_rQBufInfo.mvOut.resize(1);
                _eq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning = i; // CAMSV : pass magic number
                if( MFALSE == mpCamsvSttPipe->enque(_eq_rQBufInfo)){
                    MY_LOGD("##############################");
                    MY_LOGD("# error:enque fail           #");
                    MY_LOGD("##############################");
                    break;
                }asdfasdfasdf
            }
#endif

            // deque
            {
                MY_LOGD("### DEQUE ###");
                PortID _dq_portID;
                QBufInfo _dq_rQBufInfo;
                _dq_portID.index = NSImageio::NSIspio::EPortIndex_IMGO;
                if( MFALSE == mpCamsvSttPipe->deque(_dq_portID, _dq_rQBufInfo) ){
                    MY_LOGD("##############################");
                    MY_LOGD("# error:deque fail           #");
                    MY_LOGD("##############################");
                    break;
                }

                if( _dq_rQBufInfo.mvOut.size() ) {
                    printf("#### magic num 0x%08x\n", _dq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning); // CAMSV : check magic number
                }
                else {
                    printf("#### magic num ----------, _dq_portID size is 0\n"); // CAMSV : check magic number
                }
            }

#if !CAMSV_EQ_DQ_ONCE_IN_CONFIGPIPE
            // enque
            {
                MY_LOGD("### ENQUE ###");
                QBufInfo _eq_rQBufInfo;
                _eq_rQBufInfo.mvOut.resize(1);
                _eq_rQBufInfo.mvOut.at(0).FrameBased.mMagicNum_tuning = i; // CAMSV : pass magic number
                if( MFALSE == mpCamsvSttPipe->enque(_eq_rQBufInfo)){
                    MY_LOGD("##############################");
                    MY_LOGD("# error:enque fail           #");
                    MY_LOGD("##############################");
                    break;
                }
            }
#endif

        }
        MY_LOGD("##################################################################################");
#endif


#if 1   // force to off CAMSV IMGO : FIX ME
        sttPorts = 0;
#endif
    }

    MY_LOGD("allocate image buffer...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pUt[sidx]->mpImemDrv = IMemDrv::createInstance();
        if (!pUt[sidx]->mpImemDrv) {
            MY_LOGE("IMem create fail");
            return -1;
        }
        ret = pUt[sidx]->mpImemDrv->init();
        if (!ret) {
            MY_LOGE("IMem init fail");
            return -1;
        }

        if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__RRZO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__RRZO].resize(pUt[sidx]->mPortBufDepth);

            if(pUt[sidx]->m_ufeo_en == 1){
                pUt[sidx]->mpImgBuffer[__UFEO].resize(pUt[sidx]->mPortBufDepth);
                pUt[sidx]->mImemBuf[__UFEO].resize(pUt[sidx]->mPortBufDepth);
            }
        }
        if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__IMGO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__IMGO].resize(pUt[sidx]->mPortBufDepth);
        }
        if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__EISO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__EISO].resize(pUt[sidx]->mPortBufDepth);
        }
        if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
            pUt[sidx]->mpImgBuffer[__LCSO].resize(pUt[sidx]->mPortBufDepth);
            pUt[sidx]->mImemBuf[__LCSO].resize(pUt[sidx]->mPortBufDepth);
        }

        for (j = 0; j < pUt[sidx]->mPortBufDepth; j++) {
            IImageBuffer* pIBuf = NULL;
            IImageBuffer* pIBuf_UFEO = NULL;
            if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                NormalPipe_QueryInfo    qry;
                MSize                   _size;
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                if(pUt[sidx]->m_ufeo_en == 1){
                     if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_UFEO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                        MY_LOGE("get pix fmt error");
                        return -1;
                     }
                }
                else{
                if (!getOutputFmt(PORT_RRZO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error\n");
                    return -1;
                }
                }

                _size = __SCALE_PIX_SIZE(fmt, pUt[sidx]->mTgSize, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M, pUt[sidx]->m_b4k2k);

                getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, _size.w, qry);

                imgiBuf.size = _size.h * qry.stride_byte;

                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                }
                pUt[sidx]->mImemBuf[__RRZO].at(j) = imgiBuf;
                MY_LOGD("rrzo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = qry.stride_byte;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID, imgiBuf.virtAddr, 0, imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(__RRZ_FMT(bitmap(getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort))), _size, bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam,portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__RRZO].at(j) = pIBuf;

                    MY_LOGD("rrzo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

            if(pUt[sidx]->m_ufeo_en == 1){
                if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                            NormalPipe_QueryInfo    qry;
                            MSize                   _size;
                            MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                            MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                            if (!getOutputFmt(PORT_UFEO, getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                                MY_LOGE("get pix fmt error\n");
                                return -1;
                            }

                            _size = __SCALE_PIX_SIZE(fmt, pUt[sidx]->mTgSize, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M, pUt[sidx]->m_b4k2k);

                            getNormalPipeModule()->query(PORT_RRZO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                                fmt, _size.w, qry);

                            imgiBuf.size = _size.h * qry.stride_byte;

                            if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                                MY_LOGE("error: imem alloc fail");
                            }

                            pUt[sidx]->mImemBuf[__UFEO].at(j) = imgiBuf;
                            MY_LOGD("ufeo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);
                            bufStridesInBytes[0] = qry.stride_byte;
                            {
                            PortBufInfo_v1 portBufInfo(imgiBuf.memID, imgiBuf.virtAddr, 0, imgiBuf.bufSecu, imgiBuf.bufCohe);

                            IImageBufferAllocator::ImgParam imgParam(__RRZ_FMT(bitmap(getPortBitDepth(PORT_RRZO, pUt[sidx]->m_enablePort))), _size, bufStridesInBytes,
                                                                bufBoundaryInBytes, 1);
                            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam,portBufInfo, MTRUE);

                            pIBuf = pHeap->createImageBuffer();
                            pIBuf->incStrong(pIBuf);
                            pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                            pUt[sidx]->mpImgBuffer[__UFEO].at(j) = pIBuf;

                            MY_LOGD("ufeo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                            }
                        }
            }
            if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
                NormalPipe_QueryInfo    qry;
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                if (!getOutputFmt(PORT_IMGO, getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort), mSensorInfo[sidx], &fmt)) {
                    MY_LOGE("get pix fmt error");
                    return -1;
                }

                getNormalPipeModule()->query(PORT_IMGO.index, ENPipeQueryCmd_X_PIX|ENPipeQueryCmd_STRIDE_BYTE,
                                    fmt, pUt[sidx]->mTgSize.w, qry);

                imgiBuf.size = pUt[sidx]->mTgSize.h * qry.stride_byte;

                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                }
                pUt[sidx]->mImemBuf[__IMGO].at(j) = imgiBuf;
                MY_LOGD("imgo imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = qry.stride_byte;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam((bitmap(getPortBitDepth(PORT_IMGO, pUt[sidx]->m_enablePort))), pUt[sidx]->mTgSize, bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( mThisName, imgParam,portBufInfo,MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__IMGO].at(j) = pIBuf;

                    MY_LOGD("imgo img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

            if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                imgiBuf.size = EISO_SIZE * 1; //TBD
                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                }

                pUt[sidx]->mImemBuf[__EISO].at(j) = imgiBuf;
                MY_LOGD("eiso imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = EISO_SIZE;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(eImgFmt_BAYER8, MSize(EISO_SIZE, 1), bufStridesInBytes,
                                                        bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__EISO].at(j) = pIBuf;

                    MY_LOGD("eiso img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

            if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
                MUINT32 bufStridesInBytes[3] = {0, 0, 0};
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

                imgiBuf.size = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).w *
                                pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).h; //TBD
                if (pUt[sidx]->mpImemDrv->allocVirtBuf(&imgiBuf) < 0) {
                    MY_LOGE("error: imem alloc fail");
                }

                pUt[sidx]->mImemBuf[__LCSO].at(j) = imgiBuf;
                MY_LOGD("lcso imem id:%d va:%p", imgiBuf.memID, (void*)imgiBuf.virtAddr);

                bufStridesInBytes[0] = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize).w;

                {
                    PortBufInfo_v1 portBufInfo(imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);

                    IImageBufferAllocator::ImgParam imgParam(eImgFmt_BAYER8,
                            pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize), bufStridesInBytes,
                            bufBoundaryInBytes, 1);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(mThisName, imgParam, portBufInfo, MTRUE);

                    pIBuf = pHeap->createImageBuffer();
                    pIBuf->incStrong(pIBuf);
                    pIBuf->lockBuf(mThisName, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    pUt[sidx]->mpImgBuffer[__LCSO].at(j) = pIBuf;

                    MY_LOGD("lcso img(%d) pa:x%lx", j, pIBuf->getBufPA(0));
                }
            }

        }
        if (pUt[sidx]->mEnableSttPort & __PDO_ENABLE) {
            //allocate bpci table
            //bpci
            pUt[sidx]->m_bpciBuf.bufCohe = MFALSE;
            pUt[sidx]->m_bpciBuf.size = sizeof(bpci_array_s5k2x8);
            pUt[sidx]->mpImemDrv->allocVirtBuf(&pUt[sidx]->m_bpciBuf);
            pUt[sidx]->mpImemDrv->mapPhyAddr(&pUt[sidx]->m_bpciBuf);

            memcpy((char*)pUt[sidx]->m_bpciBuf.virtAddr,(char*)&bpci_array_s5k2x8[0],sizeof(bpci_array_s5k2x8));
        }
    }


    MY_LOGD("enque...");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        QBufInfo    qBuf;
        MUINT32     magicNum;
        MUINT32     *pMag = NULL;
        MBOOL bin_en = MFALSE;
        MUINT32 bin_w,bin_h;
        MSize szIMGO[2];

        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_IMGO_INFO,(MINTPTR)&szIMGO[0], 0, 0);
        MY_LOGI("Process_IMGO(%dx%d), Pure_IMGO (%dx%d)", szIMGO[0].w, szIMGO[0].h, szIMGO[1].w, szIMGO[1].h);

        pUt[sidx]->mpNPipe->sendCommand(ENPipeCmd_GET_BIN_INFO, (MINTPTR)&bin_w, (MINTPTR)&bin_h, 0);
        bin_en = (bin_w != pUt[sidx]->mTgSize.w)? (1):(0);

        for (pUt[sidx]->mEnqCount = 0; pUt[sidx]->mEnqCount < pUt[sidx]->mBurstQNum*pUt[sidx]->mStaticEnqCnt;
                pUt[sidx]->mEnqCount += pUt[sidx]->mBurstQNum) {
            qBuf.mvOut.clear();
            //
            pMag = (MUINT32*)malloc(sizeof(MUINT32)*pUt[sidx]->mBurstQNum);
            for (j = pUt[sidx]->mEnqCount; j < (pUt[sidx]->mEnqCount + pUt[sidx]->mBurstQNum); j++) {
                magicNum = j;
                pMag[j-pUt[sidx]->mEnqCount] = j;

                if (pUt[sidx]->m_enablePort & __RRZO_ENABLE) {
                    //note:crop size can't < outsize
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__RRZO].at(j);

                    MY_LOGD("RRZ max input size: %dx%d", bin_w, bin_h);

                    MSize _rrz = __SCALE_PIX_SIZE((EImageFormat)pIBuf->getImgFormat(), pUt[sidx]->mTgSize, RRZ_SCALING_RATIO_N, RRZ_SCALING_RATIO_M, pUt[sidx]->m_b4k2k);
                    _cropsize = __CROP_SIZE(PORT_RRZO, (EImageFormat)pIBuf->getImgFormat(), pUt[sidx]->mTgSize, pUt[sidx]->mCropTestSel,bin_en, pUt[sidx]->m_b4k2k);
                    if ((_cropsize.w < _rrz.w) || (_cropsize.h < _rrz.h)) {
                        _rrz = _cropsize;
                        MY_LOGD("RRZ in constraint: %dx%d", _rrz.w, _rrz.h);
                    }

                    BufInfo _buf(PORT_RRZO, pIBuf, _rrz,
                                MRect(__CROP_START(PORT_RRZO, pUt[sidx]->mTgSize,\
                                        _cropsize, (EImageFormat)pIBuf->getImgFormat(), pUt[sidx]->mCropTestSel,bin_en),\
                                        _cropsize), magicNum, 0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);

                    if(pUt[sidx]->m_ufeo_en == 1){
                        IImageBuffer* pIBuf_UFEO = pUt[sidx]->mpImgBuffer[__UFEO].at(j);

                        BufInfo _buf_UFEO(PORT_UFEO, pIBuf_UFEO, _rrz,
                                    MRect(__CROP_START(PORT_RRZO, pUt[sidx]->mTgSize,\
                                            _cropsize, (EImageFormat)pIBuf_UFEO->getImgFormat(), pUt[sidx]->mCropTestSel,bin_en),\
                                            _cropsize), magicNum, 0, 0xFFFF);
                        qBuf.mvOut.push_back(_buf_UFEO);
                    }
                }
                if (pUt[sidx]->m_enablePort & __IMGO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__IMGO].at(j);

                    _cropsize = __CROP_SIZE(PORT_IMGO, (EImageFormat)pIBuf->getImgFormat(), pUt[sidx]->mTgSize, pUt[sidx]->mCropTestSel,bin_en,0);

                    BufInfo _buf(PORT_IMGO, pIBuf, _cropsize,
                                MRect(__CROP_START(PORT_IMGO, pUt[sidx]->mTgSize, _cropsize,\
                                        (EImageFormat)pIBuf->getImgFormat(), 0,bin_en), _cropsize), magicNum, (pUt[sidx]->m_enablePort & __IMGO_PURE_RAW)?1:0, 0xFFFF);
                    qBuf.mvOut.push_back(_buf);
                }
                if (pUt[sidx]->m_enablePort & __EISO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__EISO].at(j);

                    _cropsize = MSize(EISO_SIZE, 1);

                    qBuf.mvOut.push_back(BufInfo(PORT_EISO, pIBuf, _cropsize, MRect(MPoint(0,0), _cropsize), magicNum, 0, 0xFFFF));
                }
                if (pUt[sidx]->m_enablePort & __LCSO_ENABLE) {
                    IImageBuffer* pIBuf = pUt[sidx]->mpImgBuffer[__LCSO].at(j);

                    _cropsize = pUt[sidx]->mLcsNotify.queryLcsOutSize(pUt[sidx]->mTgSize);

                    qBuf.mvOut.push_back(BufInfo(PORT_LCSO, pIBuf, _cropsize, MRect(MPoint(0,0), _cropsize), magicNum, 0, 0xFFFF));
                }
            }
            //
            if (pUt[sidx]->mEnableSttPort) {
                pUt[sidx]->setTuning(pMag);
            }

            #if (TEST_ZHDR_HW == 1)
            pUt[sidx]->setZHdrTuning(pMag);
            #endif

            free(pMag);

            if (MFALSE == pUt[sidx]->mpNPipe->enque(qBuf)) {
                MY_LOGE("Enque fail...");
                return -1;
            }
        }
    }

    MY_LOGD("ISP START");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        if (pUt[sidx]->mEnableSttPort) {
            if (MFALSE == pUt[sidx]->mpSttPipe->start()) {
                MY_LOGE("Start STT fail...");
                return -1;
            }
        }

        if (MFALSE == pUt[sidx]->mpNPipe->start()) {
            MY_LOGE("Start fail...");
            return -1;
        }
    }

    /*
     * note: need to start isp fist(can't use deque first,than use another to start isp, or deadlock will hanppen)
     */

    MY_LOGD("start deque/denque thread");

    for (sidx = 0; sidx < sensorNum; sidx++) {
        pUt[sidx]->startThread();
    }

    if(m_bN3DTest == true){
        pUt[0]->startHwsyncThread();
    }

    while (1) {
        MBOOL running = MFALSE;

        for (sidx = 0; sidx < sensorNum; sidx++) {
            if (!pUt[sidx]->m_bStop) {
                running = MTRUE;
            }
        }

        if (MFALSE == running) {
            MY_LOGD("all stopped");
            break;
        }
        else {
            MUINT32 wait_us = 0;

            if (sensorFps) {
                wait_us = (1000000/2)/sensorFps;
            }
            if (0 == wait_us) {
                wait_us = 500000;
            }

            for (sidx = 0; sidx < sensorNum; sidx++) {
                MY_LOGD("[%d] wait %d us: %d/%d", pUt[sidx]->mSensorIdx, wait_us, pUt[sidx]->mLoopCount, pUt[sidx]->mFrameNum);
            }
            usleep(wait_us);
        }
    }

FAIL_EXIT:
    for (sidx = 0; sidx < sensorNum; sidx++) {
        switch (stopFlow) {
        case 2:
            MY_LOGD("THREAD STOP");
            pUt[sidx]->stopThread();
            break;
        case 1://non-blocking stop
            MY_LOGD("Not stop thread first");
        default:
            break;
        }

        if(m_bN3DTest == true){
            if(sidx == 0){
                pUt[0]->stopHwsyncThread();
            }
        }

        MY_LOGD("STT STOP");
        if (pUt[sidx]->mEnableSttPort) {
            pUt[sidx]->mpSttPipe->stop();
        }

        if (stopFlow == 2) {
            MY_LOGD("NPIPE ABORT");
            pUt[sidx]->mpNPipe->abort();
        }

        if (pUt[sidx]->tuningMgr) {
            pUt[sidx]->tuningMgr->uninit(mThisName);
        }

        if (stopFlow == 0) {
            MY_LOGD("NPIPE BLOCKING STOP");
            pUt[sidx]->mpNPipe->stop(MFALSE);
        } else if (stopFlow == 1) {
            MY_LOGD("NPIPE NONBLOCKING STOP");
            pUt[sidx]->mpNPipe->stop(MTRUE);
        }

        if (pUt[sidx]->mEnableSttPort) {
            if(pUt[sidx]->mEnableTgInt)
                pUt[sidx]->mpSttPipe->signal(EPipeSignal_TG_INT, 1);

            pUt[sidx]->mpSttPipe->uninit();
            pUt[sidx]->mpSttPipe->destroyInstance(mThisName);
        }

        pUt[sidx]->mpNPipe->uninit();

        if (_switchCam) {

            pthread_create(&_switchThd, NULL, open_another_cam, pUt[sidx]);

            MY_LOGD("Wait another cam create...");
            ::sem_wait(&pUt[sidx]->m_semSwitchThd);
            MY_LOGD("Wait another cam create done");
        }

        pUt[sidx]->mpNPipe->destroyInstance(mThisName);

        //power off sensor
        pSensorHalObj[sidx]->powerOff(mThisName,1, &pUt[sidx]->mSensorIdx);
    }

    for (sidx = 0; sidx < sensorNum; sidx++) { //Save File
        for (j = 0; j < 1/*mPortBufDepth*/; j++) {
            for (dma = 0; dma < __MAXDMAO; dma++) {
                if (pUt[sidx]->m_enablePort & (1 << dma)) {
                    onDumpBuffer("camio", sidx, dma, (MUINTPTR)pUt[sidx]->mpImgBuffer[dma].at(j), j);
#if 1
                    if((pUt[sidx]->m_ufeo_en == 1) & (dma == __RRZO)){
                        onDumpBuffer("camio", sidx, __UFEO, (MUINTPTR)pUt[sidx]->mpImgBuffer[__UFEO].at(j), j);
                    }
#endif
                }
            }
        }
        for (j = 0; j < 1; j++) {
            for (dma = 0; dma < __MAXDMAO_STT; dma++) {
                if (pUt[sidx]->mEnableSttPort & (1 << dma)) {
                    sprintf(filename, "/data/cameradump_camio/%d_stt_%d_%d.raw", sidx, dma, j);
                    saveBuf(filename, (MUINT8*)pUt[sidx]->mpSttBuf[dma], pUt[sidx]->mpSttBufSize[dma]);
                }
            }
        }
    }

    MY_LOGD("Clean up");
    for (sidx = 0; sidx < sensorNum; sidx++) {
        vSensorCfg[sidx].clear();
        lHwPortCfg[sidx].clear();
        for (dma = 0; dma < __MAXDMAO; dma++) {
            if (pUt[sidx]->m_enablePort & (1 << dma)) {
                for (j = 0; j < pUt[sidx]->mImemBuf[dma].size(); j++) {
                    MY_LOGD("Freeing VA: %p", (void*)pUt[sidx]->mImemBuf[dma].at(j).virtAddr);
                    pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->mImemBuf[dma].at(j));
                }
            }
        }
        #if 0
        for (dma = 0; dma < __MAXDMAO_STT; dma++) {
            if (pUt[sidx]->mpSttBuf[dma]) {
                delete (unsigned char*)pUt[sidx]->mpSttBuf[dma];
            }

        }
        #endif

        if(pUt[sidx]->mEnableSttPort & __PDO_ENABLE)
            pUt[sidx]->mpImemDrv->freeVirtBuf(&pUt[sidx]->m_bpciBuf);

        pUt[sidx]->mpImemDrv->uninit();
        pUt[sidx]->mpImemDrv->destroyInstance();

        MY_LOGD("destroy...");
        pUt[sidx]->destroy();

        if (_switchCam) {
            pthread_join(_switchThd, NULL);
        }
    }

    if(_efuse && _efuse_idx > 0) {
        _efuse_idx --;
        goto SEARCH_SENSOR;
    }

    MY_LOGD("EOT");

    return ret;
}
