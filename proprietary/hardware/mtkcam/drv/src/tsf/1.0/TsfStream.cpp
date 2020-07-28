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
#define LOG_TAG "TSFStream"

#include "TsfStream.h"
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

#include "tsf_log.h"
DECLARE_DBG_LOG_VARIABLE(TsfStream);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (TsfStream_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (TsfStream_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (TsfStream_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (TsfStream_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (TsfStream_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (TsfStream_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_DBG("+");
#define FUNCTION_LOG_END        LOG_DBG("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace std;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSTsf;


TsfDrv* TsfStream::m_pTsfDrv = NULL;
list<TSFRequest>  TsfStream::m_QueueTsfReqList; //TSF Req List

Mutex               TsfStream::mModuleMtx;
MINT32              TsfStream::mInitCount;     //Record the user count

#define TSF_INT_WAIT_TIMEOUT_MS (5000) //Early Poting is slow.

//Debug Utility

/******************************************************************************
 *
 ******************************************************************************/
bool
TsfStream::
DrvMkdir(char const*const path, uint_t const mode)
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
MBOOL 
TsfStream::
DrvMakePath(char const*const path, uint_t const mode)
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
ITsfStream*
ITsfStream::
createInstance(
    char const* szCallerName)
{
    FUNCTION_LOG_START;
    //[1] create HalPipeWrapper
    FUNCTION_LOG_END;

    return new TsfStream(szCallerName);
    //return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
TsfStream::
destroyInstance(
    char const* szCallerName)
{
    FUNCTION_LOG_START;

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
TsfStream::
TsfStream(char const* szCallerName)
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
            LOG_ERR("CallerName(%s), CallerNameLen(%zu), mTsfInitCnt(%d)",szCallerName, strlen(szCallerName), mInitCount);
        }
        else
        {
            LOG_ERR("szCallerName is NULL, mTsfInitCnt(%d)\n", mInitCount);
        }
    }
    m_DeCryptionKey = 0x0;  //DeCryKey
    m_EnCryptionKey = 0x0;  //EnCryKey
    m_InputDeCryption = MFALSE;
    m_OutputEnCryption = MFALSE;
    m_bSaveReqToFile = 0;
    m_iTsfEnqueReq = 0;
   //LOG_INF("tag/sidx/sdev(%d/0x%x/0x%x),swPipe cID(0x%x)",mStreamTag,openedSensorIndex,mOpenedSensor,(&mDequeuedBufList));
}


/*******************************************************************************
*
********************************************************************************/
TsfStream::~TsfStream()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TsfStream::
init()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32 tmp=0;
    //[1] increase user count and record user
    tmp = android_atomic_inc(&mInitCount);
    LOG_INF("mTsfInitCnt(%d)",mInitCount);

    property_get("vendor.cam.tsfsavefile.enable", value, "0");
    m_bSaveReqToFile = atoi(value);


    //first user
    if(mInitCount==1)
    {
        //sem_init(&mSemTsfThread, 0, 0);
        m_pTsfDrv = TsfDrv::createInstance();
        //create dequeue thread
        m_QueueTsfReqList.clear();
        /* initialize random seed: */
        srand (time(NULL));

        if (m_pTsfDrv->init(m_UserName) == MFALSE){
            LOG_ERR("m_pTsfDrv init fail!!");
            return false;
        }
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);
        
        /* generate secret number between 0 and 65535: */
        m_EnCryptionKey = rand() % 65536;
        m_DeCryptionKey = (rand() % 65536) +1;
        m_pTsfDrv->writeReg ( TSF_CRYPTION_OFFSET, (m_DeCryptionKey<< 16) | m_EnCryptionKey);
        LOG_INF("m_AAA(%d), m_BBB(%d)",m_DeCryptionKey, m_EnCryptionKey);
        
        m_iTsfEnqueReq = 0;
        
    }

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TsfStream::
uninit()
{
    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mModuleMtx);
    bool ret = true;
    MINT32 tmp=0;

    //[1] decrease user count and record user
    tmp = android_atomic_dec(&mInitCount);
    LOG_INF("mTsfUnitCnt(%d)",mInitCount);

    //deq thread related variables are init only once
    if(mInitCount==0)
    {

        if (m_pTsfDrv->uninit(m_UserName) == MFALSE){
            LOG_ERR("m_pTsfDrv uninit fail!!");
            return false;
        }
        printf("UserName:%s, mInitCount:%d\n", m_UserName, mInitCount);

        m_iTsfEnqueReq = 0;
        //stop dequeue thread
        //::sem_wait(&mSemTsfThread);
    }


    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TsfStream::
getTSFKey(MUINT32& DeCryptionKey, MUINT32& EnCryptionKey)
{
    Mutex::Autolock autoLock(mModuleMtx);

    EnCryptionKey = m_EnCryptionKey;
    DeCryptionKey = m_DeCryptionKey;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TsfStream::
setCryption(MBOOL InputDeCryption, MBOOL OutputEnCryption)
{
    Mutex::Autolock autoLock(mModuleMtx);

    MUINT32 RegVal;
    m_InputDeCryption = InputDeCryption;
    m_OutputEnCryption = OutputEnCryption;
    RegVal = m_pTsfDrv->readReg(TSF_START_OFFSET);
    m_pTsfDrv->writeReg ( TSF_START_OFFSET, (RegVal & 0xFFFFEEFF) | (InputDeCryption << 12) | (OutputEnCryption << 8));
    LOG_INF("input_AAA(%d), ouput_BBB(%d)",m_InputDeCryption, m_OutputEnCryption);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
TsfStream::
saveToFile(char const* filepath, TSFBufInfo* buffer)
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
                LOG_ERR("fail to write %s, write-count:%d, written-bytes:%zu : %s", filepath, cnt, written, ::strerror(errno));
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
TsfStream::
TSFenque(TSFParams const& rTsfParams)
{
    FUNCTION_LOG_START;
    DRV_TRACE_CALL();
    Mutex::Autolock autoLock(mModuleMtx);

#define DUMP_DVEPREFIX "/sdcard/tsfdump_tsf"


#define DUMP_TSFBuffer( stream, pbuf, type, dir, cnt, cof1, cof2, cof3, cof4, fileExt)      \
    do{                                                        \
        TSFBufInfo* buffer = (TSFBufInfo*)pbuf;                \
        TsfStream* pStream = (TsfStream*)stream;               \
        char filename[256];                                    \
        sprintf(filename, "%s%s%s_%d_%d_%d_%d_%d_%d.%s",                   \
                dumppath,                                      \
                #type,                                         \
                dir,                                           \
                buffer->u4BufSize,                             \
                cnt,                                           \
                cof1,                                          \
                cof2,                                          \
                cof3,                                          \
                cof4,                                          \
                fileExt                                        \
               );                                              \
        pStream->saveToFile(filename, buffer);                 \
    }while(0)

    char dumppath[256];
    if (MTRUE == m_bSaveReqToFile)
    {
        sprintf( dumppath, "%s/", DUMP_DVEPREFIX);
        
        if(!this->DrvMakePath(dumppath,0660))
        {
            LOG_ERR("DVE makePath [%s] fail",dumppath);
            return false;
        }
    }


    bool ret = true;
    TSFRequest tsfreq;
    LOG_DBG("TSF Enq+");

    m_iTsfEnqueReq++;
    if (MTRUE == m_bSaveReqToFile)
    {
        LOG_INF("TSF Save Frame Start:%d", m_iTsfEnqueReq);
        DUMP_TSFBuffer(this, &rTsfParams.mTSFConfig.DMA_Tsfi, DMA_TSFI , "in" , m_iTsfEnqueReq , 
            rTsfParams.mTSFConfig.TSF_COEFF_1, rTsfParams.mTSFConfig.TSF_COEFF_2, rTsfParams.mTSFConfig.TSF_COEFF_3, rTsfParams.mTSFConfig.TSF_COEFF_4, "dat");
        DUMP_TSFBuffer(this, &rTsfParams.mTSFConfig.DMA_Tsfo, DMA_TSFO , "out" , m_iTsfEnqueReq ,
            rTsfParams.mTSFConfig.TSF_COEFF_1, rTsfParams.mTSFConfig.TSF_COEFF_2, rTsfParams.mTSFConfig.TSF_COEFF_3, rTsfParams.mTSFConfig.TSF_COEFF_4, "dat");
        LOG_INF("TSF Save Frame End:%d", m_iTsfEnqueReq);
    }


    //Enque TSF Request!!
    tsfreq.m_pTsfStream = (void*) this;
    tsfreq.m_TSFParams.mpfnCallback = rTsfParams.mpfnCallback;
    tsfreq.m_TSFParams.mpCookie = rTsfParams.mpCookie;
    tsfreq.m_TSFParams.mTSFConfig = rTsfParams.mTSFConfig;


    //addCommand(ECmd_DVE_ENQUE);
    if (MFALSE == m_pTsfDrv->enqueTSF(rTsfParams.mTSFConfig))
    {
        LOG_ERR("TSF Enque fail");
        return false;        
    }

    if (NULL != rTsfParams.mpfnCallback)
    {
        m_pTsfDrv->waitTSFFrameDone(TSF_INT_ST, TSF_INT_WAIT_TIMEOUT_MS);
        LOG_DBG("TSF Int");
        rTsfParams.mpfnCallback(tsfreq.m_TSFParams);
    }
    LOG_DBG("TSF Enq-");
    


    FUNCTION_LOG_END;
#undef DUMP_TSFBuffer
    return ret;
}


/******************************************************************************
*
******************************************************************************/

MBOOL
TsfStream::
TSFdeque(TSFParams& rTsfParams, MINT64 i8TimeoutNs)
{
    DRV_TRACE_CALL();
    (void)rTsfParams;

    FUNCTION_LOG_START;
    Mutex::Autolock autoLock(mDequeMtx);
    bool ret = true;
    LOG_DBG("TSF Deq+");

    if(i8TimeoutNs == -1)   //temp solution for infinite wait
    {
        LOG_INF("TSF no timeout set, infinite wait");
        i8TimeoutNs=8000000000;
    }

    LOG_DBG("TSF i8TimeoutNs(%jd)", i8TimeoutNs);
    m_pTsfDrv->waitTSFFrameDone(TSF_INT_ST, TSF_INT_WAIT_TIMEOUT_MS);

    //mTsfDequeCond.wait(mDequeMtx);   //wait for tsf thread

    LOG_DBG("TSF Deq-");
    FUNCTION_LOG_END;
    return ret;


}




