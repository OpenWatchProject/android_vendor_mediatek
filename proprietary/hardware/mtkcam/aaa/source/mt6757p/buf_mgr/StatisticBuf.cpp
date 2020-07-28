/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "statistic_buf"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Log.h>
#include "StatisticBuf.h"
#include <string.h>
#include <utils/Condition.h>
#include <cutils/properties.h>

using namespace android;
using namespace NS3Av3;

MBOOL
StatisticBufInfo::
dump(const char* filename) const
{
    char strFile[512] = {'\0'};
    sprintf(strFile, "%s_%d.raw", filename, mMagicNumber);
    FILE* fid = fopen(strFile, "wb");
    if (fid)
    {
        fwrite((void*)mVa, mSize, 1, fid);
        fclose(fid);
        return MTRUE;
    }
    return MFALSE;
}

class StatisticBuf::StatisticBufImp
{
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    mutable Mutex           m_Lock;
    MBOOL                   m_fgLog;
    MBOOL                   m_fgCmd;
    MBOOL                   m_fgFlush;
    Condition               m_Cond;
    StatisticBufInfo        m_rBufInfo;
    MUINT32                 m_i4TotalBufSize;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////    Disallowed.
    //  Copy constructor is disallowed.
    StatisticBufImp(StatisticBufImp const&);
    //  Copy-assignment operator is disallowed.
    StatisticBufImp& operator=(StatisticBufImp const&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    StatisticBufImp();
    StatisticBufImp(MUINT32 const i4BufSize);
    virtual ~StatisticBufImp();

    virtual MBOOL allocateBuf(MUINT32 const i4BufSize);
    virtual MBOOL freeBuf();
    virtual MBOOL write(StatisticBufInfo rBufInfo);
    virtual StatisticBufInfo* read();
    virtual MBOOL flush();
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

StatisticBuf::StatisticBufImp::
StatisticBufImp()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_i4TotalBufSize(0)
{
    m_rBufInfo.mVa = NULL;
    m_rBufInfo.mSize = 0;
}

StatisticBuf::StatisticBufImp::
StatisticBufImp(MUINT32 const i4BufSize)
    : StatisticBufImp()
{
    allocateBuf(i4BufSize);
    m_fgLog = property_get_int32("vendor.debug.statistic_buf.enable", 0);
}

StatisticBuf::StatisticBufImp::
~StatisticBufImp()
{
    freeBuf();
}

MBOOL
StatisticBuf::StatisticBufImp::
allocateBuf(MUINT32 const i4BufSize)
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.mVa == NULL || m_i4TotalBufSize == 0){
        CAM_LOGD_IF(m_fgLog, "[%s] before m_rBufInfo->mVa = 0x%x, \n", __FUNCTION__, m_rBufInfo.mVa);
        m_i4TotalBufSize = i4BufSize;
        m_rBufInfo.mVa = (MUINTPTR) new MUINT8[m_i4TotalBufSize + 1];
        CAM_LOGD_IF(m_fgLog, "[%s] after  m_rBufInfo->mVa = 0x%x, \n", __FUNCTION__, m_rBufInfo.mVa);
    }
    return MFALSE;
}

MBOOL
StatisticBuf::StatisticBufImp::
freeBuf()
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.mVa != NULL && m_i4TotalBufSize != 0){
        CAM_LOGD_IF(m_fgLog, "[%s] m_rBufInfo->mVa = 0x%x, \n", __FUNCTION__, m_rBufInfo.mVa);
        delete [] (MUINT8*)m_rBufInfo.mVa;
    }
    m_i4TotalBufSize = 0;
    return MTRUE;
}

MBOOL
StatisticBuf::StatisticBufImp::
write(StatisticBufInfo rBufInfo)
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.mVa == NULL){
        CAM_LOGE("m_rBufInfo.mVa == NULL");
        return MFALSE;
    }

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, rBufInfo.mMagicNumber);

    // copy buffer member
    m_rBufInfo.mMagicNumber = rBufInfo.mMagicNumber;
    m_rBufInfo.mSize = rBufInfo.mSize;
    m_rBufInfo.mStride = rBufInfo.mStride;
    m_rBufInfo.mFrameCount = rBufInfo.mFrameCount;
    m_rBufInfo.mTimeStamp = rBufInfo.mTimeStamp;
    m_fgCmd = MTRUE;
    // copy buffer data from input buffer.
    if(rBufInfo.mSize > m_i4TotalBufSize){
        CAM_LOGE("rBufInfo->mSize > m_i4TotalBufSize");
        return MFALSE;
    }
    ::memcpy((void*)m_rBufInfo.mVa, (void*)rBufInfo.mVa, rBufInfo.mSize);
    // broadcast for writing buffer done.
    m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, rBufInfo.mMagicNumber);
    return MTRUE;
}

StatisticBufInfo*
StatisticBuf::StatisticBufImp::
read()
{
    Mutex::Autolock lock(m_Lock);
    if(m_rBufInfo.mVa == NULL){
        CAM_LOGE("m_rBufInfo.mVa == NULL");
        return NULL;
    }
    if(m_fgFlush)
        return NULL;

    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)", __FUNCTION__, m_fgCmd);
    // wait for writing buffer done.
    if(!m_fgCmd)
    {
        CAM_LOGD_IF(m_fgLog, "[%s] wait\n", __FUNCTION__);
        m_Cond.wait(m_Lock);
    }
    m_fgCmd = MFALSE;
    CAM_LOGD_IF(m_fgLog, "[%s] - m_fgCmd(%d) Magic(%d)\n", __FUNCTION__, m_fgCmd, m_rBufInfo.mMagicNumber);
    if(m_fgFlush)
    {
        return NULL;
    }
    else
        return &m_rBufInfo;
}

MBOOL
StatisticBuf::StatisticBufImp::
flush()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD_IF(m_fgLog, "[%s] + m_fgCmd(%d)\n", __FUNCTION__, m_fgCmd);
    m_fgFlush = MTRUE;
    if(!m_fgCmd)
        m_Cond.broadcast();
    CAM_LOGD_IF(m_fgLog, "[%s] -\n", __FUNCTION__);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

StatisticBuf::
StatisticBuf()
:mImp(new StatisticBufImp())
{
}

StatisticBuf::
StatisticBuf(MUINT32 const i4BufSize)
:StatisticBuf()
{
    allocateBuf(i4BufSize);
}

StatisticBuf::
~StatisticBuf()
{
    if(mImp != NULL)
    {
        mImp->freeBuf();
        delete mImp;
    }
}

MBOOL
StatisticBuf::
allocateBuf(MUINT32 const i4BufSize)
{
    return mImp->allocateBuf(i4BufSize);
}

MBOOL
StatisticBuf::
freeBuf()
{
    return mImp->freeBuf();
}

MBOOL
StatisticBuf::
write(StatisticBufInfo rBufInfo)
{
    return mImp->write(rBufInfo);
}

StatisticBufInfo*
StatisticBuf::
read()
{
    return mImp->read();
}

MBOOL
StatisticBuf::
flush()
{
    return mImp->flush();
}

