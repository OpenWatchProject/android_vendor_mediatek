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
#define LOG_TAG "WPEStream"
//mutex
#include <pthread.h>
//#include <ispio_utility.h>

#include <mtkcam/drv/iopipe/PostProc/IHalWpePipeDef.h>
#include "HalPipeWrapper.h"


#include "WpeStream.h"
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include <stdio.h>
#include <stdlib.h>
//tpipe
#include <tpipe_config.h>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(WpeStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (WpeStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (WpeStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (WpeStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (WpeStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (WpeStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (WpeStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//

#define MAX_TPIPE_WIDTH_WPE                  (512)
#define MAX_TPIPE_HEIGHT_WPE                 (65536)
// tpipe irq mode
#define TPIPE_IRQ_FRAME_WPE     (0)
#define TPIPE_IRQ_LINE_WPE      (1)
#define TPIPE_IRQ_TPIPE_WPE     (2)
// for crop function
#define WPE_CROP_FLOAT_PECISE_BIT      31    // precise 31 bit
#define WPE_CROP_TPIPE_PECISE_BIT      24
#define WPE_CROP_TPIPE_PECISE_BIT_20   20

#define TPIPE_SHIFT (1 << 24)

// for  tpipe dump information
#undef DUMP_TPIPE_SIZE
#undef DUMP_TPIPE_NUM_PER_LINE
#define DUMP_TPIPE_SIZE         100
#define DUMP_TPIPE_NUM_PER_LINE 10

using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSWpe;


#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h

enum WPEStreamHint
   {
       HINT_WPE_TILE_MODE = 0x0,       // need to do tpipe calculation
       HINT_WPE_TPIPE_NO_CHANGE = 0x1, // can skip tpipe calculation
       HINT_WPE_FRAME_MODE = 0x2,      // ISP-only frame mode
   };

// Default setting
#define VGEN_EN                    1
#define CACHI_EN                   1
#define VECI_EN                    1
#define VEC2I_EN1                  1
#define CACHE_BURST_LEN            2

#define MSKO_EN                    0

#define CTL_EXTENSION              1
#define CACHE_VURST_LEN            2 // 1:burst-2; 2:burst-4; 3:burst-8
#define WPE_DONE_EN                1 
#define TDR_ERR_INT_EN             1
#define Wpe_SW_RST_ST              1
#define Wpe_VGEN_VERTICAL_FIRST    1
#define Wpe_ADDR_GEN_MAX_BURST_LEN 4

#define Wpe_WPEO_SOFT_RST_STAT     1
#define Wpe_MSKO_SOFT_RST_STAT     1
#define Wpe_VECI_SOFT_RST_STAT     1 
#define Wpe_VEC2I_SOFT_RST_STAT    1 
#define Wpe_VEC3I_SOFT_RST_STAT    1
#define ADDR_GEN_SOFT_RSTSTAT      1


//MINT32              WpeDrv::m_Fd = -1;
Mutex               WpeStream::mMutex_EQUser;
Mutex               WpeStream::mModuleMtx;
volatile MINT32     WpeStream::mInitCount = 0;     //Record the user count
Mutex               WpeStream::mMutex_EQDone;
volatile MINT32     WpeStream::m_wpeEQUserNum = 0;
volatile MINT32     WpeStream::m_wpeEQDupIdx = 0;;
volatile MINT32     WpeStream::m_WpeStreamInit = MFALSE;
volatile MINT32     WpeStream::m_iWpeEnqueReq = 0;
volatile MINT32     WpeStream::m_bSaveReqToFile = 0;
volatile MINT32     WpeStream::mDumpCount = 0;


ISP_TPIPE_CONFIG_STRUCT gWPELastTpipeStructure;

nsecs_t nsTimeoutToWaitEnQ = 500LL*1000LL*1000LL;//wait 500 msecs.


//Debug Utility
#define DUMP_WPEPREFIX "/sdcard/wpedump_wpe"


/******************************************************************************
 *
 ******************************************************************************/
WpeStream*
WpeStream::createInstance()
{
    return WpeStream::getWpeStreamImp();
}

static WpeStream singleton;

/******************************************************************************
 *
 ******************************************************************************/
WpeStream*
WpeStream::getWpeStreamImp()
{
    LOG_INF("& WPE Singleton(%p)\n",&singleton);

    return &singleton;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
WpeStream::
destroyInstance()
{
    FUNCTION_LOG_START;


    FUNCTION_LOG_END;
    //delete this;
}
/*******************************************************************************
*
********************************************************************************/
WpeStream::
WpeStream()
{
    //if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
       // strcpy((char*)m_UserName,(char const*)szCallerName);
    }
    //else
    {
       // LOG_ERR("szCallerName error! , mWpeInitCnt(%d)", mInitCount);
    }
    this->mwarpengine = NULL;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
WpeStream::~WpeStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
WpeStream::
init(MUINT32 secTag)
{
    (void)secTag;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //[1] increase user count and record user
    android_atomic_inc(&mInitCount);
    LOG_INF("+,mWpeInitCnt(%d)",mInitCount);

    property_get("vendor.cam.wpesavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mWPESemEQDone, 0, 0);
        sem_init(&mSemWpeThread, 0, 0);
        //create dequeue thread
        createThread();

        ret = this->mwarpengine = NSImageio::NSIspio::IWarpEnginePipe::createInstance();
        if (NULL ==this->mwarpengine || ret == MFALSE)
        {
            LOG_ERR("WPEStram WarpEngine creat instancefail");
            return MFALSE;
        }

        ret = this->mwarpengine->init(WPE_MODULE_IDX_WPE_A);
        if(MFALSE == ret) {
            LOG_ERR("[Error]mwarpengine->init fail(%d)",ret);
            ret = MFALSE;
        }

    //sprintf(m_UserName,"wpeStream_%d",mInitCount);
    if (MFALSE == m_WpeStreamInit)
    {
        m_WpeStreamInit = MTRUE;
    }

        m_iWpeEnqueReq = 0;
    }
    //

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
WpeStream::
uninit()
{
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 subRet;


    //[1] decrease user count and record user
    android_atomic_dec(&mInitCount);
    LOG_INF("+,mWpeUnitCnt(%d)",mInitCount);

    if(mInitCount==0)
    {
        /*============================================
                 wpe command buffer
            =============================================*/
        if (MTRUE == m_WpeStreamInit)
        {   
            m_WpeStreamInit = MFALSE;
            m_iWpeEnqueReq = 0;
        }

        subRet = this->mwarpengine->uninit(WPE_MODULE_IDX_WPE_A);
        if(MFALSE == subRet) {
            LOG_ERR("[Error]mwarpengine->uninit fail(%d)",subRet);
            ret = MFALSE;
        }

        this->mwarpengine->destroyInstance();

        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemWpeThread);
    }


    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
WpeStream::
saveToFile(char const* filepath, WPEBufInfo* buffer)
{
    MBOOL ret = MFALSE;
    int fd = -1;

    //
    LOG_DBG("save to %s", filepath);
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if  ( fd < 0 )
    {
        LOG_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        MUINT8* pBuf = (MUINT8*)buffer->u4BufVA;
        size_t  size = buffer->u4BufSize;
        size_t  written = 0;
        int nw = 0, cnt = 0;
        while ( written < size )
        {
            nw = ::write(fd, pBuf+written, size-written);
            if  (nw < 0)
            {
                LOG_ERR(
                    "fail to write %s, write-count:%d, written-bytes:%zu : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        LOG_DBG("write %zu bytes to %s", size, filepath);
    }
    //
    ret = MTRUE;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    return  ret;

}


MBOOL
WpeStream::
dumpToFile(char const* filepath, MUINT8* pBufVA, size_t  size)
{
    MBOOL ret = MFALSE;
    int fd = -1;

    //
    LOG_DBG("save to %s", filepath);
    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if  ( fd < 0 )
    {
        LOG_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto lbExit;
    }
    //
    {
        size_t  written = 0;
        int nw = 0, cnt = 0;
        while ( written < size )
        {
            nw = ::write(fd, pBufVA+written, size-written);
            if  (nw < 0)
            {
                LOG_ERR(
                    "fail to write %s, write-count:%d, written-bytes:%zu : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        LOG_DBG("write %zu bytes to %s", size, filepath);
    }
    //
    ret = MTRUE;
lbExit:
    //
    if  ( fd >= 0 )
    {
        ::close(fd);
    }
    //
    return  ret;

}


MBOOL
WpeStream::
checkWPEEnqueuedUserNum()
{  
    getEQUserLock();
    if(m_wpeEQUserNum == MAX_WPE_ENQUEUE_USER)
    {
        releaseEQUserLock();
        return true;
    }
    else
    {
        releaseEQUserLock();
        return false;
    }
}

MBOOL
WpeStream::
wpeGlobalListCtrl(WpePackage wpeframepackage, MUINT32 framenumber, MUINT32 IsFirstFrame, MINT32& GlobalListIndex)
{
    MBOOL ret = MTRUE;

    if(wpeframepackage.wpeBurstIdx == 0 && IsFirstFrame == 0)
    {
        //top-level record number of buffer-batches
        WPEFramePackage bufPackage;

        bufPackage.callerID = wpeframepackage.callerID;
        bufPackage.rParams  = wpeframepackage.rParams;
        bufPackage.DupIdx   = wpeframepackage.wpeDupIdx;
        bufPackage.frameNum = framenumber;

        /******************************* BufBatch Lock Region Start ********************************/
        getWPELock(EWPELockEnum_wDQFramePackList);
        //top-level buffer-batches list
        bufPackage.idxofwaitDQFrmPackList = mLWPEDQFramePackList.size();	   //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)
        mLWPEDQFramePackList.push_back(bufPackage);

        GlobalListIndex = bufPackage.idxofwaitDQFrmPackList;
        //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
        getWPELock(EWPELockEnum_wDQFrameUnitList);
        WPEFrameUnit bufNode;
        bufNode.rParams = wpeframepackage.rParams;
        bufNode.wpeDupIdx = wpeframepackage.wpeDupIdx;
        bufNode.callerID = wpeframepackage.callerID;
        bufNode.wpeBurstIdx = wpeframepackage.wpeBurstIdx;
        bufNode.wpeDupCmdIdx = wpeframepackage.m_wpeDupCmdIdx;
        bufNode.idxofwaitDQFrmPackList=bufPackage.idxofwaitDQFrmPackList;    //avoid timing issue, addBatch -> addNode -> removeBatch
        mLWPEwaitDQFrameUnitList.push_back(bufNode);

        LOG_INF("EQ GlobalListCtrl,WPE WPE Caller(0x%x), GlobalListIndex (%d),  WARP Engine ID (%d),framenum_dupidx(%d/%d)", bufNode.callerID, GlobalListIndex, bufNode.wpeModeIdx, bufNode.wpeBurstIdx,bufNode.wpeDupIdx);

        releaseWPELock(EWPELockEnum_wDQFrameUnitList);
        releaseWPELock(EWPELockEnum_wDQFramePackList);
    }
    else
    {
        WPEFrameUnit bufNode;
        MINT32 tempshift=0;
        getWPELock(EWPELockEnum_wDQFramePackList);

        //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
        //case1, judge package size
        if(GlobalListIndex >= (MINT32)mLWPEDQFramePackList.size())
        {
            tempshift = GlobalListIndex - mLWPEDQFramePackList.size()+1;
            LOG_INF("HMyo tempshift(%d)",tempshift);
        }

        //case2, judge p2cq/dupcq to avoud racing between config_thread/start_thread
        vector<WPEFramePackage>::iterator it_pack = mLWPEDQFramePackList.begin();
        for(int i = 0;i<GlobalListIndex;i++)
        {
            it_pack++;
        }
        for(int minus_count = GlobalListIndex ; minus_count>=0; minus_count--)
        {
            if((wpeframepackage.wpeDupIdx == (*it_pack).DupIdx) && (wpeframepackage.callerID == (*it_pack).callerID))
           {
                LOG_INF("HMyo the same(%d_%d_%d)",minus_count, GlobalListIndex, tempshift);
                break;
            }
            else
            {
                tempshift+=1;
               it_pack--;
            }
        }
        //error handle
        if((GlobalListIndex - tempshift) < 0)
        {
            LOG_ERR("ERR shift(%d/%d/%zu)",tempshift, GlobalListIndex, mLWPEDQFramePackList.size());
            releaseWPELock(EWPELockEnum_wDQFramePackList);
            return MFALSE;
        }

        getWPELock(EWPELockEnum_wDQFrameUnitList);
        bufNode.rParams = wpeframepackage.rParams;
        bufNode.wpeDupIdx = wpeframepackage.wpeDupIdx;
        bufNode.wpeBurstIdx = wpeframepackage.wpeBurstIdx;
        bufNode.wpeDupCmdIdx = wpeframepackage.m_wpeDupCmdIdx;
        bufNode.callerID=wpeframepackage.callerID;
        bufNode.idxofwaitDQFrmPackList = GlobalListIndex - tempshift;
        LOG_INF("EQ GlobalListCtrl,WPE WPE Caller(0x%x), WPE module(%d),framenum_dupidx(%d/%d), GlobalListIndex (%d), idxofwaitDQFrmPackList(%d)",
                                  bufNode.callerID, bufNode.wpeModeIdx, bufNode.wpeBurstIdx,bufNode.wpeDupIdx, GlobalListIndex, bufNode.idxofwaitDQFrmPackList);
        mLWPEwaitDQFrameUnitList.push_back(bufNode);

        releaseWPELock(EWPELockEnum_wDQFrameUnitList);
        releaseWPELock(EWPELockEnum_wDQFramePackList);
    }

        return ret;
}


MBOOL
WpeStream::
WPEenque(QParams const& rParams, MUINT32 callerID)
{
    DRV_TRACE_CALL();

    Mutex::Autolock autoLock(mModuleMtx);

    MBOOL ret = MFALSE;
    MINT32 err = 0;
    WpePackage wpepackage;
    NSCam::NSIoPipe::NSWpe::WPEFrameUnit wpeframeunit;
    NSCam::NSIoPipe::NSWpe::WPERequest wpereq;
    NSCam::NSIoPipe::WPEConfig wpeconfig;
    MUINT32 m_curEQUserundex;
    WPEBufParamNode param;
    MUINT32 framenumber;
    MINT32 inStartIdx = 0;
    MINT32 config_ret = 1;
    MINT32 GlobalListIndex = -1;
    framenumber = rParams.mvFrameParams.size();

    while(checkWPEEnqueuedUserNum())
    {
        LOG_INF("Block WPE Enqueue Here");
        ::sem_wait(&mWPESemEQDone);
    }

    getEQUserLock();
    m_curEQUserundex = m_wpeEQDupIdx;
    m_wpeEQDupIdx = 1 - m_wpeEQDupIdx;
    releaseEQUserLock();

    getEQUserLock();
    m_wpeEQUserNum++;
    releaseEQUserLock();

    wpereq.m_pWpeStream = (void*) this;
    wpereq.m_wpepackage = wpepackage;
    wpereq.wpeEQDupIdx = m_curEQUserundex;
    wpereq.m_Num = rParams.mvFrameParams.size();

    wpepackage.rParams = rParams;
    wpepackage.wpeDupIdx = m_curEQUserundex;
    wpepackage.callerID = callerID;

    param.wpecqDupIdx = wpepackage.wpeDupIdx;
    param.wpecallerID = wpepackage.callerID;

    for(MUINT32 q=0;q < rParams.mvFrameParams.size();q++)
    {
        wpepackage.wpeBurstIdx = q;
        wpepackage.m_wpeDupCmdIdx = 0;

        config_ret = this->mwarpengine->configwpe(wpepackage);

        if(config_ret == 1)
        {
            LOG_INF("m_curEQUserundex(%d), q(%d), m_wpeDupCmdIdx(%d), GlobalListIndex(%d)", m_curEQUserundex, q, wpepackage.m_wpeDupCmdIdx, GlobalListIndex);

            err = this->mwarpengine->start(wpepackage.wpeDupIdx, wpepackage.wpeBurstIdx, wpepackage.m_wpeDupCmdIdx);
            if(err == 0)
            {
                ret = wpeGlobalListCtrl(wpepackage,framenumber,q+inStartIdx, GlobalListIndex);
                inStartIdx++;

                //getEQLock();
                //m_QueueWPEReqList.push_back(wpereq);
                //ret = ret & m_pWpeDrv->enqueWPE(wpereq.m_WPEParams.mWPEConfigVec);
                //releaseEQLock();
                if(GlobalListIndex >= 0)
                {
                    addCommand(ECmd_WPE_ENQUE);
                }
                else
                {
                    LOG_ERR("WPE Global index update failed: GlobalListIndex(%d)", ret);
                    return ret;
                }
            }
            else
            {
                LOG_ERR("WPE MDP StartFailed (%d)", err);
                return ret;
            }
            ret = MTRUE;
        }
        else
        {
            LOG_ERR("Config WPE Failed (%d)", config_ret);
            return ret;
         }
    }
#undef DUMP_WPEBuffer

    return ret;

}

MBOOL
WpeStream::
Dump_Tpipe(MUINT32 *tpipeTableVa_wpe)
{
    MUINT32 dumpTpipeLine;
    dumpTpipeLine = DUMP_TPIPE_SIZE / DUMP_TPIPE_NUM_PER_LINE;

    for(MUINT32 i=0;i<dumpTpipeLine;i++){
        LOG_INF("[Tpipe](%02d)-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x-0x%08x",i,
            tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+1],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+2],
            tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+3],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+4],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+5],
            tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+6],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+7],tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+8],
            tpipeTableVa_wpe[i*DUMP_TPIPE_NUM_PER_LINE+9]);
    }
    
    return MTRUE;
}

enum WPEDUMPTYPE {
    DUMP_WPE_TDR = 0,
    DUMP_WPE_VECI,
    DUMP_WPE_VEC2I,
    DUMP_WPE_VEC3I,
    DUMP_WPE_REGS,
    DMA_WPE_NUM,
};

#if 0
MBOOL
WpeStream::
Dump_Warp_Tpipe(MUINT32 m_curEQUserundex, QParams rParams)
{
     NSCam::NSIoPipe::ExtraParam CmdInfo;
     android::Vector<NSCam::NSIoPipe::ExtraParam>::const_iterator iter;
     NSCam::NSIoPipe::NSWpe::WPEQParams *wpeqpararms;
     size_t  size = MAX_ISP_TILE_TDR_HEX_NO_WPE;
     char dumppath[256];
     sprintf( dumppath, "%s/", DUMP_WPEPREFIX);

    #define DUMP_WPEBuffer( stream, VAdress, size, type, fileExt)         \
         do{                                                        \
             MUINT8* BufVA = (MUINT8*)VAdress;                      \
             WpeStream* pStream = (WpeStream*)stream;               \
             char filename[256];                                    \
             sprintf(filename, "%s%s.%s",                           \
                     dumppath,                                      \
                     #type,                                         \
                     fileExt                                        \
                    );                                              \
             pStream->dumpToFile(filename, BufVA, size);            \
         }while(0)
    
    if (mDumpCount == 0)
    {
        for(iter = rParams.mvFrameParams[0].mvExtraParam.begin(); iter < rParams.mvFrameParams[0].mvExtraParam.end() ; iter++)
        {
            CmdInfo = (*iter);
            switch (CmdInfo.CmdIdx)
            {
                case NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD:  
                wpeqpararms = (NSCam::NSIoPipe::NSWpe::WPEQParams *)CmdInfo.moduleStruct;
                    break;

            }
        }
           
        Dump_Tpipe(gpWPEMdpMgrCfgData[m_curEQUserundex].ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpe);

           
        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("WPE makePath [%s] fail",dumppath);
            return MFALSE;
        }
        else
        {
            DUMP_WPEBuffer(this, wpeqpararms->warp_veci_info.virtAddr, (size_t)(wpeqpararms->warp_veci_info.stride * wpeqpararms->warp_veci_info.height), DUMP_WPE_VECI,"dat");
            DUMP_WPEBuffer(this, wpeqpararms->warp_vec2i_info.virtAddr, (size_t)(wpeqpararms->warp_vec2i_info.stride * wpeqpararms->warp_vec2i_info.height), DUMP_WPE_VEC2I,"dat");
            if(wpeqpararms->vgen_hmg_mode == 1)
                DUMP_WPEBuffer(this, wpeqpararms->warp_vec3i_info.virtAddr, (size_t)(wpeqpararms->warp_vec3i_info.stride * wpeqpararms->warp_vec3i_info.height), DUMP_WPE_VEC3I,"dat");
            DUMP_WPEBuffer(this, gpWPEMdpMgrCfgData[m_curEQUserundex].ispTpipeCfgInfo.drvinfo.tpipeTableVa_wpe, size, DUMP_WPE_TDR,"dat");
            DUMP_WPEBuffer(this, gpWPEMdpMgrCfgData[m_curEQUserundex].ispTpipeCfgInfo.drvinfo.wpecommand, size, DUMP_WPE_REGS,"dat"); 
        }      
    }
    mDumpCount ++;
    
    return MTRUE;
}
#endif

/******************************************************************************
*
******************************************************************************/

MBOOL
WpeStream::
WPEdeque(MUINT32 callerID, MINT64 i8TimeoutNs)
{
    DRV_TRACE_CALL();

    //FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    MBOOL ret = MTRUE;
    WPEBufParamNode param;
    param.wpecallerID = callerID;
    LOG_DBG("+ WPE Deque callerID (%d)", callerID);

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("WPE no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("WPE i8TimeoutNs(%" PRId64 ")", i8TimeoutNs);
    LOG_ERR("WPE DO NOT SUPPORT BLOCKING De-Q...");
    LOG_DBG("- WPE Deque");

    //FUNCTION_LOG_END;
    return ret;
}
