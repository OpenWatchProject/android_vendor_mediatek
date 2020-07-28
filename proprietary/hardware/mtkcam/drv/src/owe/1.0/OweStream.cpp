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
#define LOG_TAG "OWEStream"

#include "OweStream.h"
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

#include <engine_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(OweStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (OweStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (OweStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (OweStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (OweStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (OweStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (OweStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSOwe;


OweDrv* OweStream::m_pOweDrv = NULL;
//MINT32              OweDrv::m_Fd = -1;
list<OCCRequest>    OweStream::m_QueueOCCReqList; //OCC Req List
list<WMFERequest>   OweStream::m_QueueWMFEReqList; //WMFE Req List
Mutex               OweStream::mModuleMtx;
MINT32              OweStream::mInitCount;     //Record the user count

//Debug Utility

/******************************************************************************
 *
 ******************************************************************************/
bool DrvMkdir(char const*const path, uint_t const mode)
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
MBOOL DrvMakePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    char*copypath = strdup(path);
    char*pp = copypath;
    char*sp;

    if (copypath == NULL)
    {
        LOG_ERR("strdup failed.\n");
        return false;
    }

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
IOweStream*
IOweStream::
createInstance(
    char const* szCallerName)
{
    return new OweStream(szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
OweStream::
destroyInstance(
    char const* szCallerName)
{
    FUNCTION_LOG_START
    
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        if (strcmp (m_UserName, szCallerName) != 0){
            LOG_ERR("szCallerName(%s) of destroyInstance is not the same as user name(%s) of createInstance!!\n", szCallerName, m_UserName);
        }
    }
    else
    {
        if (szCallerName !=NULL)
        {
            LOG_ERR("CallerName(%s), CallerNameLen(%zu), mTsfInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        }
        else
        {
            LOG_ERR("szCallerName is NULL, mTsfInitCnt(%d)\n", mInitCount);
        }
    }

    FUNCTION_LOG_END;
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
OweStream::
OweStream(char const* szCallerName)
{
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        strncpy((char*)m_UserName,(char const*)szCallerName, strlen(szCallerName));
        m_UserName[strlen(szCallerName)] = '\0';
    }
    else
    {
        if (szCallerName !=NULL)
        {
        LOG_ERR("CallerName(%s), CallerNameLen(%zu), mOweInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        }
        else
        {
            LOG_ERR("szCallerName is NULL, mTsfInitCnt(%d)\n", mInitCount);
        }
    }
    m_OweStreamInit = MFALSE;
    m_iOccEnqueReq = 0;
    m_iWmfeEnqueReq = 0;
    m_bSaveReqToFile = 0;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
OweStream::~OweStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
OweStream::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 tmp=0;

    char szBuf[MAX_USER_NAME_SIZE];
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    int n;
    //[1] increase user count and record user
    tmp = android_atomic_inc(&mInitCount);
    LOG_INF("mOweInitCnt(%d)",mInitCount);

    property_get("vendor.cam.owesavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mSemOweThread, 0, 0);
        m_pOweDrv = OweDrv::createInstance();
        //create dequeue thread
        createThread();
        m_QueueOCCReqList.clear();
        m_QueueWMFEReqList.clear();
    }

    //sprintf(m_UserName,"oweStream_%d",mInitCount);
    if (MFALSE == m_OweStreamInit)
    {
        sprintf(szBuf,"_owe_%d",mInitCount);
        strncat(m_UserName, szBuf, strlen(szBuf));

        m_pOweDrv->init(m_UserName);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        m_iOccEnqueReq = 0;
        m_iWmfeEnqueReq = 0;
        m_OweStreamInit = MTRUE;
    }



    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
OweStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 tmp=0;


    //[1] decrease user count and record user
    tmp = android_atomic_dec(&mInitCount);
    LOG_INF("mOweUnitCnt(%d)",mInitCount);

    if (MTRUE == m_OweStreamInit)
    {
        m_pOweDrv->uninit(m_UserName);
        strncpy(m_UserName,"", MAX_USER_NAME_SIZE);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_OweStreamInit = MFALSE;
        m_iOccEnqueReq = 0;
        m_iWmfeEnqueReq = 0;
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {
        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemOweThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
OweStream::
saveToFile(char const* filepath, EGNBufInfo* buffer)
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


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
OweStream::
OCCenque(OCCParams const& rOccParams)
{
    FUNCTION_LOG_START;
    CAM_TRACE_CALL();
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_OCCPREFIX "/sdcard/owedump_occ"

#define DUMP_OWEBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        EGNBufInfo* buffer = (EGNBufInfo*)pbuf;                \
        OweStream* pStream = (OweStream*)stream;               \
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
        snprintf(dumppath, 256, "%s/", DUMP_OCCPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("OCC makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }


    MBOOL ret = MTRUE;

    OCCRequest occreq;
    OCCConfig occconfig;
    vector<OCCConfig>::const_iterator iter;
    occreq.m_Num = rOccParams.mOCCConfigVec.size(); //Request Number.
    occreq.m_pOweStream = (void*) this;
    occreq.m_OCCParams.mpfnCallback = rOccParams.mpfnCallback;
    occreq.m_OCCParams.mpCookie = rOccParams.mpCookie;
    iter = rOccParams.mOCCConfigVec.begin();

    for (;iter<rOccParams.mOCCConfigVec.end();iter++) {
        occconfig = (*iter);
        occreq.m_OCCParams.mOCCConfigVec.push_back(occconfig);
        if (MTRUE == m_bSaveReqToFile)
        {
#ifdef NOT_HERE
            DUMP_OWEBuffer(this, &occconfig.Occ_Imgi_l, DMA_OCC_IMGI , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Imgi_r, DMA_OCC_IMGI , "right" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Dvi_l, DMA_OCC_DVI , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Dvi_r, DMA_OCC_DVI , "right" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Maski_l, DMA_OCC_MASKI , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Maski_r, DMA_OCC_MASKI , "right" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Dvo_l, DMA_OCC_DVO , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Dvo_r, DMA_OCC_DVO , "right" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Confo_l, DMA_OCC_CONFO , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Confo_r, DMA_OCC_CONFO , "right" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Respo_l, DMA_OCC_RESPO , "left" , m_iOccEnqueReq++ , "dat");
            DUMP_OWEBuffer(this, &occconfig.Occ_Respo_r, DMA_OCC_RESPO , "right" , m_iOccEnqueReq++ , "dat");
#endif
        }

    }

    //Enque OCC Request!!
    m_QueueOCCReqList.push_back(occreq);
    addCommand(ECmd_OCC_ENQUE);
    if (m_pOweDrv->enqueOCC(occreq.m_OCCParams.mOCCConfigVec) == MFALSE)
    {
        return MFALSE;
    }

    FUNCTION_LOG_END;
#undef DUMP_OWEBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
OweStream::
OCCdeque(OCCParams& rOccParams, MINT64 i8TimeoutNs)
{
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("OCC no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("OCC i8TimeoutNs(%jd)", i8TimeoutNs);

    mOccDequeCond.wait(mDequeMtx);   //wait for owe thread
#ifdef FEEDBACK
    vector<OCCConfig>::iterator iter;
    vector<OCCConfig>::iterator dequeiter;
    iter = rOccParams.mOCCConfigVec.begin();
    dequeiter = m_OccParams.mOCCConfigVec.begin();
    for (;iter<rOccParams.mOCCConfigVec.end();iter++,dequeiter++) {
        (*iter).Occ_Vert_Sv = (*dequeiter).Occ_Vert_Sv;
        (*iter).Occ_Horz_Sv = (*dequeiter).Occ_Horz_Sv;
    }
#else
    (void)rOccParams;
#endif

    FUNCTION_LOG_END;
    return ret;


}




/******************************************************************************
 *
 ******************************************************************************/
MBOOL
OweStream::
WMFEenque(WMFEParams const& rWmfeParams)
{
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_WMFEPREFIX "/sdcard/owedump_wmfe"

#define DUMP_OWEBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        EGNBufInfo* buffer = (EGNBufInfo*)pbuf;                \
        OweStream* pStream = (OweStream*)stream;               \
        char filename[256];                                    \
        snprintf(filename, 256, "%s%s%s_%d_%d.%s",                   \
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
        snprintf(dumppath, 256, "%s/", DUMP_WMFEPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("WMFE makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }

    MBOOL ret = MTRUE;
    char szBuf[50];
    WMFERequest wmfereq;
    OWMFEConfig wmfeconfig;
    NSCam::NSIoPipe::OWMFECtrl wmfectrl;
    vector<NSCam::NSIoPipe::OWMFEConfig>::const_iterator iter;
    vector<NSCam::NSIoPipe::OWMFECtrl>::const_iterator iter_ctrl;
    int i=0;

    wmfereq.m_Num = rWmfeParams.mWMFEConfigVec.size(); //Request Number.
    wmfereq.m_pOweStream = (void*) this;
    wmfereq.m_WMFEParams.mpfnCallback = rWmfeParams.mpfnCallback;
    wmfereq.m_WMFEParams.mpCookie = rWmfeParams.mpCookie;
    iter = rWmfeParams.mWMFEConfigVec.begin();

    for (;iter!=rWmfeParams.mWMFEConfigVec.end();iter++) {


        wmfeconfig.mWMFECtrlVec.clear();
        for (i=0, iter_ctrl = (*iter).mWMFECtrlVec.begin();iter_ctrl != (*iter).mWMFECtrlVec.end();iter_ctrl++, i++) {
            wmfectrl = (*iter_ctrl);
            if (MTRUE == m_bSaveReqToFile)
            {
                sprintf( szBuf, "wmfe%d", i);
                if (MTRUE == wmfectrl.Wmfe_Enable)
                {
                    DUMP_OWEBuffer(this, &wmfectrl.Wmfe_Imgi, DMA_OWMFE_IMGI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_OWEBuffer(this, &wmfectrl.Wmfe_Dpi, DMA_OWMFE_DPI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_OWEBuffer(this, &wmfectrl.Wmfe_Tbli, DMA_OWMFE_TBLI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_OWEBuffer(this, &wmfectrl.Wmfe_Dpo, DMA_OWMFE_DPO , szBuf , m_iWmfeEnqueReq , "dat");
                    if (MTRUE == wmfectrl.Wmfe_Mask_En)
                    {
                        DUMP_OWEBuffer(this, &wmfectrl.Wmfe_Maski, DMA_OWMFE_MASKI , szBuf , m_iWmfeEnqueReq , "dat");
                    }
                }
            }
            wmfeconfig.mWMFECtrlVec.push_back(wmfectrl);
        }
        m_iWmfeEnqueReq++;


        wmfereq.m_WMFEParams.mWMFEConfigVec.push_back(wmfeconfig);
    }

    //Enque WMFE Request!!
    m_QueueWMFEReqList.push_back(wmfereq);
    addCommand(ECmd_WMFE_ENQUE);
    if (m_pOweDrv->enqueWMFE(wmfereq.m_WMFEParams.mWMFEConfigVec) == MFALSE)
    {
        return MFALSE;
    }

    FUNCTION_LOG_END;
#undef DUMP_OWEBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
OweStream::
WMFEdeque(WMFEParams& rWmfeParams, MINT64 i8TimeoutNs)
{
    (void)rWmfeParams;
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("WMFE no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("WMFE i8TimeoutNs(%jd)", i8TimeoutNs);

    mWmfeDequeCond.wait(mDequeMtx);      //wait for owe thread

    FUNCTION_LOG_END;
    return ret;

}

