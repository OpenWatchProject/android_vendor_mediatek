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
#define LOG_TAG "RSCStream"

#include "RscStream.h"
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(RscStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (RscStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (RscStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (RscStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (RscStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (RscStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (RscStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSRsc;


RscDrv* RscStream::m_pRscDrv = NULL;
//MINT32              RscDrv::m_Fd = -1;
list<RSCRequest>   RscStream::m_QueueRSCReqList; //RSC Req List
Mutex               RscStream::mModuleMtx;
MINT32              RscStream::mInitCount;     //Record the user count

//Debug Utility

/******************************************************************************
 *
 ******************************************************************************/
static bool DrvMkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  (errno != EEXIST && ::mkdir(path, mode) != 0)
        {
            LOG_ERR("fail to mkdir [%s]: %d[%s]", path, errno, ::strerror(errno));
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        LOG_ERR("!S_ISDIR");
        return  false;
    }
    //
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
static MBOOL DrvMakePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    char*copypath = strdup(path);
    char*pp = copypath;
    char*sp;
    if (pp != NULL) {
        while ( ret && 0 != (sp = strchr(pp, '/')) )
        {
            if (sp != pp)
            {
                *sp = '\0';
                ret = DrvMkdir(copypath, mode);
                *sp = '/';
            }
            pp = sp + 1;
        }
        if (ret) {
            ret = DrvMkdir(path, mode);
        }
        free(copypath);
    }
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
IRscStream*
IRscStream::
createInstance(
    char const* szCallerName)
{
    static RscStream singleton(szCallerName);
    return &singleton;
}

/******************************************************************************
 *
 ******************************************************************************/

IRscStream*
NSCam::NSIoPipe::NSRsc::getRscStreamImp(
    char const* szCallerName)
{
    static RscStream singleton(szCallerName);
    return &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RscStream::
destroyInstance(
    char const* szCallerName)
{
    (void) szCallerName;
    FUNCTION_LOG_START;


    FUNCTION_LOG_END;
}
/*******************************************************************************
*
********************************************************************************/
RscStream::
RscStream(char const* szCallerName)
{
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        strncpy((char*)m_UserName,(char const*)szCallerName, strlen(szCallerName));
	m_UserName[strlen(szCallerName)] = '\0';
    }
    else if (szCallerName != NULL)
    {
        LOG_ERR("CallerName(%s), CallerNameLen(%zd), mRscInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
    }
    m_RscStreamInit = MFALSE;
    m_iRscEnqueReq = 0;
    m_bSaveReqToFile = MFALSE;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
RscStream::~RscStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RscStream::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    char szBuf[MAX_USER_NAME_SIZE];
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int32_t old_cnt;
    //[1] increase user count and record user
    old_cnt = android_atomic_inc(&mInitCount);
    LOG_INF("mRscInitCnt(%d)",mInitCount);

    property_get("vendor.cam.rscsavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mSemRscThread, 0, 0);
        m_pRscDrv = RscDrv::createInstance();
        //create dequeue thread
        createThread();
        m_QueueRSCReqList.clear();
    }

    //sprintf(m_UserName,"rscStream_%d",mInitCount);
    if (MFALSE == m_RscStreamInit)
    {
        snprintf(szBuf, MAX_USER_NAME_SIZE, "_rsc_%d",mInitCount);
        strncat(m_UserName, szBuf, strlen(szBuf));

        m_pRscDrv->init(m_UserName);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        m_iRscEnqueReq = 0;
        m_RscStreamInit = MTRUE;
    }



    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RscStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    int32_t old_cnt;

    //[1] decrease user count and record user
    old_cnt = android_atomic_dec(&mInitCount);
    LOG_INF("mRscUnitCnt(%d)",mInitCount);

    if (MTRUE == m_RscStreamInit && mInitCount == 0)
    {
        m_pRscDrv->uninit(m_UserName);
        strncpy(m_UserName,"",MAX_USER_NAME_SIZE);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_RscStreamInit = MFALSE;
        m_iRscEnqueReq = 0;
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {
        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemRscThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RscStream::
saveToFile(char const* filepath, RSCBufInfo* buffer)
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
                    "fail to write %s, write-count:%d, written-bytes:%zd : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        LOG_DBG("write %zd bytes to %s", size, filepath);
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

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
RscStream::
RSCenque(RSCParams const& rRscParams)
{
    DRV_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_RSCPREFIX "/sdcard/rscdump_rsc"

#define DUMP_RSCBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        RSCBufInfo* buffer = (RSCBufInfo*)pbuf;                \
        RscStream* pStream = (RscStream*)stream;               \
        char filename[256];                                    \
        sprintf(filename, "%s%s%s_%d_%d.%s",                   \
                dumppath,                                      \
                #type,                                         \
                dir,                                           \
                buffer->u4BufSize,                             \
                cnt,                                           \
                fileExt                                        \
               );                                              \
        pStream->saveToFile(filename, buffer);                 \
    }while(0)


    char dumppath[256];
    if (MTRUE == m_bSaveReqToFile)
    {
        snprintf(dumppath, 256, "%s/", DUMP_RSCPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("RSC makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }

    bool ret = true;

    RSCRequest rscreq;
    RSCConfig rscconfig;
    vector<RSCConfig>::const_iterator iter;

    rscreq.m_Num = rRscParams.mRSCConfigVec.size(); //Request Number.
    rscreq.m_pRscStream = (void*) this;
    rscreq.m_RSCParams.mpfnCallback = rRscParams.mpfnCallback;
    rscreq.m_RSCParams.mpCookie = rRscParams.mpCookie;
    iter = rRscParams.mRSCConfigVec.begin();

    for (;iter<rRscParams.mRSCConfigVec.end();iter++) {
        rscconfig = (*iter);
        if (MTRUE == m_bSaveReqToFile)
        {
#if 0
            if (MTRUE == rscconfig.Rsc_Ctrl_0.Rsc_Enable)
            {
                DUMP_RSCBuffer(this, &rscconfig.Rsc_Ctrl_0.Rsc_Imgi, DMA_RSC_IMGI , "rsc0" , m_iRscEnqueReq , "dat");
                DUMP_RSCBuffer(this, &rscconfig.Rsc_Ctrl_0.Rsc_Dpi, DMA_RSC_DPI , "rsc0" , m_iRscEnqueReq , "dat");
                DUMP_RSCBuffer(this, &rscconfig.Rsc_Ctrl_0.Rsc_Tbli, DMA_RSC_TBLI , "rsc0" , m_iRscEnqueReq , "dat");
                DUMP_RSCBuffer(this, &rscconfig.Rsc_Ctrl_0.Rsc_Dpo, DMA_RSC_DPO , "rsc0" , m_iRscEnqueReq , "dat");
            }
#endif
        }
        m_iRscEnqueReq++;


        rscreq.m_RSCParams.mRSCConfigVec.push_back(rscconfig);
    }

    //Enque RSC Request!!
    m_QueueRSCReqList.push_back(rscreq);
    addCommand(ECmd_RSC_ENQUE);
    m_pRscDrv->enqueRSC(rscreq.m_RSCParams.mRSCConfigVec);

    FUNCTION_LOG_END;
#undef DUMP_RSCBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
RscStream::
RSCdeque(RSCParams& rRscParams, MINT64 i8TimeoutNs)
{
    (void) rRscParams;
    DRV_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("RSC no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    /*LOG_DBG("RSC i8TimeoutNs(%ld)", i8TimeoutNs);*/

    mRscDequeCond.wait(mDequeMtx);      //wait for rsc thread

    FUNCTION_LOG_END;
    return ret;

}



