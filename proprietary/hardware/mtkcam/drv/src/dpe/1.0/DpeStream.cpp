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
#define LOG_TAG "DPEStream"

#include "DpeStream.h"
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

#include <dpe_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(DpeStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (DpeStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (DpeStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (DpeStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (DpeStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (DpeStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (DpeStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSDpe;


DpeDrv* DpeStream::m_pDpeDrv = NULL;
//MINT32              DpeDrv::m_Fd = -1;
list<DVERequest>    DpeStream::m_QueueDVEReqList; //DVE Req List
list<WMFERequest>   DpeStream::m_QueueWMFEReqList; //WMFE Req List
Mutex               DpeStream::mModuleMtx;
MINT32              DpeStream::mInitCount;     //Record the user count

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
IDpeStream*
IDpeStream::
createInstance(
    char const* szCallerName)
{
    return new DpeStream(szCallerName);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
DpeStream::
destroyInstance(
    char const* szCallerName)
{
    FUNCTION_LOG_START

    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        if (strcmp (m_UserName, szCallerName) != 0){
            LOG_ERR("szCallerName(%s) of destroyInstance is not the same as user name(%s) of createInstance!!\n", szCallerName, m_UserName);
        }
        strncpy(m_UserName,"",MAX_USER_NAME_SIZE);
        m_UserName[0]='\0';
    }
    else
    {
        if (szCallerName !=NULL)
        {
            LOG_ERR("CallerName(%s), CallerNameLen(%zu), mDpeInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        }
        else
        {
            LOG_ERR("szCallerName is NULL, mDpeInitCnt(%d)\n", mInitCount);
        }
    }

    FUNCTION_LOG_END;
    delete this;
}
/*******************************************************************************
*
********************************************************************************/
DpeStream::
DpeStream(char const* szCallerName)
{
    if ((szCallerName !=NULL) && (strlen(szCallerName) <= (MAX_USER_NAME_SIZE-8))) //-8 will be used by internal
    {
        strncpy((char*)m_UserName,(char const*)szCallerName, strlen(szCallerName));
        m_UserName[strlen(szCallerName)]='\0';
    }
    else
    {
        if (szCallerName !=NULL)
        {
            LOG_ERR("CallerName(%s), CallerNameLen(%zu), mDpeInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        }
        else
        {
            LOG_ERR("szCallerName is NULL, mDpeInitCnt(%d)\n", mInitCount);
        }
    }
    m_DpeStreamInit = MFALSE;
    m_iDveEnqueReq = 0;
    m_iWmfeEnqueReq = 0;
    m_bSaveReqToFile = 0;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
DpeStream::~DpeStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DpeStream::
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
    LOG_INF("mDpeInitCnt(%d)",mInitCount);

    property_get("vendor.cam.dpesavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        sem_init(&mSemDpeThread, 0, 0);
        m_pDpeDrv = DpeDrv::createInstance();
        //create dequeue thread
        createThread();
        m_QueueDVEReqList.clear();
        m_QueueWMFEReqList.clear();
    }

    //sprintf(m_UserName,"dpeStream_%d",mInitCount);
    if (MFALSE == m_DpeStreamInit)
    {
        //sprintf(szBuf,"_dpe_%d",mInitCount);
        //strncat (m_UserName, szBuf, strlen(szBuf));

        m_pDpeDrv->init(m_UserName);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        m_iDveEnqueReq = 0;
        m_iWmfeEnqueReq = 0;
        m_DpeStreamInit = MTRUE;
    }



    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DpeStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 tmp=0;


    //[1] decrease user count and record user
    tmp = android_atomic_dec(&mInitCount);
    LOG_INF("mDpeUnitCnt(%d)",mInitCount);

    if (MTRUE == m_DpeStreamInit)
    {
        m_pDpeDrv->uninit(m_UserName);
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_DpeStreamInit = MFALSE;
        m_iDveEnqueReq = 0;
        m_iWmfeEnqueReq = 0;
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {
        //stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemDpeThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DpeStream::
saveToFile(char const* filepath, DPEBufInfo* buffer)
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
DpeStream::
DVEenque(DVEParams const& rDveParams)
{
    FUNCTION_LOG_START;
    CAM_TRACE_CALL();
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_DVEPREFIX "/sdcard/dpedump_dve"

#define DUMP_DPEBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        DPEBufInfo* buffer = (DPEBufInfo*)pbuf;                \
        DpeStream* pStream = (DpeStream*)stream;               \
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
        sprintf( dumppath, "%s/", DUMP_DVEPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("DVE makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }


    MBOOL ret = MTRUE;

    DVERequest dvereq;
    DVEConfig dveconfig;
    vector<DVEConfig>::const_iterator iter;
    dvereq.m_Num = rDveParams.mDVEConfigVec.size(); //Request Number.
    dvereq.m_pDpeStream = (void*) this;
    dvereq.m_DVEParams.mpfnCallback = rDveParams.mpfnCallback;
    dvereq.m_DVEParams.mpCookie = rDveParams.mpCookie;
    iter = rDveParams.mDVEConfigVec.begin();

    for (;iter<rDveParams.mDVEConfigVec.end();iter++) {
        dveconfig = (*iter);
        dvereq.m_DVEParams.mDVEConfigVec.push_back(dveconfig);
        if (MTRUE == m_bSaveReqToFile)
        {
            DUMP_DPEBuffer(this, &dveconfig.Dve_Imgi_l, DMA_DVE_IMGI , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Imgi_r, DMA_DVE_IMGI , "right" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Dvi_l, DMA_DVE_DVI , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Dvi_r, DMA_DVE_DVI , "right" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Maski_l, DMA_DVE_MASKI , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Maski_r, DMA_DVE_MASKI , "right" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Dvo_l, DMA_DVE_DVO , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Dvo_r, DMA_DVE_DVO , "right" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Confo_l, DMA_DVE_CONFO , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Confo_r, DMA_DVE_CONFO , "right" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Respo_l, DMA_DVE_RESPO , "left" , m_iDveEnqueReq++ , "dat");
            DUMP_DPEBuffer(this, &dveconfig.Dve_Respo_r, DMA_DVE_RESPO , "right" , m_iDveEnqueReq++ , "dat");
        }

    }

    //Enque DVE Request!!
    m_QueueDVEReqList.push_back(dvereq);
    addCommand(ECmd_DVE_ENQUE);
    if (m_pDpeDrv->enqueDVE(dvereq.m_DVEParams.mDVEConfigVec) == MFALSE)
    {
        return MFALSE;
    }

    FUNCTION_LOG_END;
#undef DUMP_DPEBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
DpeStream::
DVEdeque(DVEParams& rDveParams, MINT64 i8TimeoutNs)
{
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("DVE no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("DVE i8TimeoutNs(%jd)", i8TimeoutNs);

    mDveDequeCond.wait(mDequeMtx);   //wait for dpe thread

    vector<DVEConfig>::iterator iter;
    vector<DVEConfig>::iterator dequeiter;
    iter = rDveParams.mDVEConfigVec.begin();
    dequeiter = m_DveParams.mDVEConfigVec.begin();
    for (;iter<rDveParams.mDVEConfigVec.end();iter++,dequeiter++) {
        (*iter).Dve_Vert_Sv = (*dequeiter).Dve_Vert_Sv;
        (*iter).Dve_Horz_Sv = (*dequeiter).Dve_Horz_Sv;
    }


    FUNCTION_LOG_END;
    return ret;


}




/******************************************************************************
 *
 ******************************************************************************/
MBOOL
DpeStream::
WMFEenque(WMFEParams const& rWmfeParams)
{
    CAM_TRACE_CALL();

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_WMFEPREFIX "/sdcard/dpedump_wmfe"

#define DUMP_DPEBuffer( stream, pbuf, type, dir, cnt, fileExt)      \
    do{                                                        \
        DPEBufInfo* buffer = (DPEBufInfo*)pbuf;                \
        DpeStream* pStream = (DpeStream*)stream;               \
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
        sprintf( dumppath, "%s/", DUMP_WMFEPREFIX);

        if(!DrvMakePath(dumppath,0660))
        {
            LOG_ERR("WMFE makePath [%s] fail",dumppath);
            return MFALSE;
        }
    }

    MBOOL ret = MTRUE;
    char szBuf[50];
    WMFERequest wmfereq;
    WMFEConfig wmfeconfig;
    NSCam::NSIoPipe::WMFECtrl wmfectrl;
    vector<NSCam::NSIoPipe::WMFEConfig>::const_iterator iter;
    vector<NSCam::NSIoPipe::WMFECtrl>::const_iterator iter_ctrl;
    int i=0;

    wmfereq.m_Num = rWmfeParams.mWMFEConfigVec.size(); //Request Number.
    wmfereq.m_pDpeStream = (void*) this;
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
                    DUMP_DPEBuffer(this, &wmfectrl.Wmfe_Imgi, DMA_WMFE_IMGI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_DPEBuffer(this, &wmfectrl.Wmfe_Dpi, DMA_WMFE_DPI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_DPEBuffer(this, &wmfectrl.Wmfe_Tbli, DMA_WMFE_TBLI , szBuf , m_iWmfeEnqueReq , "dat");
                    DUMP_DPEBuffer(this, &wmfectrl.Wmfe_Dpo, DMA_WMFE_DPO , szBuf , m_iWmfeEnqueReq , "dat");
                    if (MTRUE == wmfectrl.Wmfe_Mask_En)
                    {
                        DUMP_DPEBuffer(this, &wmfectrl.Wmfe_Maski, DMA_WMFE_MASKI , szBuf , m_iWmfeEnqueReq , "dat");
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
    if (m_pDpeDrv->enqueWMFE(wmfereq.m_WMFEParams.mWMFEConfigVec) == MFALSE)
    {
        return MFALSE;
    }

    FUNCTION_LOG_END;
#undef DUMP_DPEBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MBOOL
DpeStream::
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

    mWmfeDequeCond.wait(mDequeMtx);      //wait for dpe thread

    FUNCTION_LOG_END;
    return ret;

}



