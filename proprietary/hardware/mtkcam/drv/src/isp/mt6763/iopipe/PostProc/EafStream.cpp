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
#define LOG_TAG "EAFStream"

#include "EafStream.h"
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include <stdio.h>
#include <stdlib.h>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(EafStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (EafStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (EafStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (EafStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (EafStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (EafStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (EafStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSEaf;


EafDrv* EafStream::m_pEafDrv = NULL;
//MINT32              EafDrv::m_Fd = -1;
list<EAFRequest>    EafStream::m_QueueEAFReqList; //EAF Req List
Mutex               EafStream::mModuleMtx;
MINT32              EafStream::mInitCount;     //Record the user count

//Debug Utility

/******************************************************************************
 *
 ******************************************************************************/
static bool DrvMkdir(char const*const path, uint_t const mode)
{
#if 0 //QQ
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
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
#endif	
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
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
IEafStream*
IEafStream::
createInstance(
    char const* szCallerName)
{
    return new EafStream(szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
EafStream::
destroyInstance(
    char const* szCallerName)
{
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
EafStream::
EafStream(char const* szCallerName)
{
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        strcpy((char*)m_UserName,(char const*)szCallerName);
    }
    else
    {
        if (szCallerName !=NULL)
            LOG_ERR("CallerName(%s), CallerNameLen(%d), mGepfInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        else
            LOG_ERR("szCallerName is NULL, mEafInitCnt(%d)\n", mInitCount);
    }
    m_EafStreamInit = MFALSE;
    m_iEafEnqueReq = 0;
    m_bSaveReqToFile = 0;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
EafStream::~EafStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EafStream::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    char szBuf[MAX_USER_NAME_SIZE];
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int n;
    //[1] increase user count and record user
    android_atomic_inc(&mInitCount);
    LOG_INF("mEafInitCnt(%d)",mInitCount);

    property_get("vendor.cam.eafsavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mSemEafThread, 0, 0);
        m_pEafDrv = EafDrv::createInstance();
        //create dequeue thread
        createThread();
        m_QueueEAFReqList.clear();
    }

    //sprintf(m_UserName,"eafStream_%d",mInitCount);
    if (MFALSE == m_EafStreamInit)
    {
        sprintf(szBuf,"_eaf_%d",mInitCount);
        strcat(m_UserName, szBuf);
        
        m_pEafDrv->init(m_UserName);
        LOG_INF("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        m_iEafEnqueReq = 0;
        m_EafStreamInit = MTRUE;
    }



    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EafStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;


    //[1] decrease user count and record user
    android_atomic_dec(&mInitCount);
    LOG_INF("mEafUnitCnt(%d)",mInitCount);

    if (MTRUE == m_EafStreamInit)
    {
        m_pEafDrv->uninit(m_UserName);
        strcpy(m_UserName,"");
        LOG_INF("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_EafStreamInit = MFALSE;
        m_iEafEnqueReq = 0;
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {
        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemEafThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EafStream::
saveToFile(char const* filepath, EAFBufInfo* buffer)
{
    MBOOL ret = MFALSE;
    int fd = -1;

#if 0 //QQ
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
                    "fail to write %s, write-count:%d, written-bytes:%d : %s",
                    filepath, cnt, written, ::strerror(errno)
                );
                goto lbExit;
            }
            written += nw;
            cnt ++;
        }
        LOG_DBG("write %d bytes to %s", size, filepath);
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
#endif	
    return  ret;
    
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EafStream::
EAFenque(EAFParams const& rEafParams)
{
    FUNCTION_LOG_START;
//    DRV_TRACE_CALL();
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_EAFPREFIX "/sdcard/eafdump_dve"

#define DUMP_EAFBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        EAFBufInfo* buffer = (EAFBufInfo*)pbuf;                \
        EafStream* pStream = (EafStream*)stream;               \
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
        sprintf( dumppath, "%s/", DUMP_EAFPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("EAF makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }

    bool ret = true;

    EAFRequest eafreq;
    EAFConfig eafconfig;
    vector<EAFConfig>::const_iterator iter;
    eafreq.m_Num = rEafParams.mEAFConfig.size(); //Request Number.
    eafreq.m_pEafStream = (void*) this;
    eafreq.m_EAFParams.mpfnCallback = rEafParams.mpfnCallback;
    eafreq.m_EAFParams.mpCookie = rEafParams.mpCookie;
    iter = rEafParams.mEAFConfig.begin();

	LOG_DBG("EafStream::EAFenque \n");

    for (;iter<rEafParams.mEAFConfig.end();iter++) {
        eafconfig = (*iter);
        if (MTRUE == m_bSaveReqToFile)
        {
//            DUMP_EAFBuffer(this, &eafconfig.Dve_Imgi_l, DMA_EAF_IMGI , "left" , m_iEafEnqueReq++ , "dat");
        }
        eafreq.m_EAFParams.mEAFConfig.push_back(eafconfig);
    }

    //Enque EAF Request!!
    m_QueueEAFReqList.push_back(eafreq);
    addCommand(ECmd_EAF_ENQUE);
	
	LOG_DBG("ECmd_EAF_ENQUE \n");
    m_pEafDrv->enqueEAF(eafreq.m_EAFParams.mEAFConfig);

    FUNCTION_LOG_END;
#undef DUMP_EAFBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
EafStream::
EAFdeque(EAFParams& rEafParams, MINT64 i8TimeoutNs)
{
 //   DRV_TRACE_CALL();
 	LOG_DBG("EafStream::EAFdeque \n");

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("EAF no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("EAF i8TimeoutNs(%lld)", i8TimeoutNs);

    mEafDequeCond.wait(mDequeMtx);   //wait for eaf thread

    FUNCTION_LOG_END;
    return ret;

}

