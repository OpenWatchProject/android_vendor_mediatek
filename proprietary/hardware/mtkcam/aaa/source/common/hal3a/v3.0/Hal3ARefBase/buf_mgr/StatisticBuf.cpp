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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "StatisticBuf"

#include <string.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include "StatisticBuf.h"

using namespace NS3Av3;

MBOOL StatisticBufInfo::dump(const char* filename) const
{
#ifndef USING_MTK_LDVT
    char strFile[512] = {'\0'};
    sprintf(strFile, "%s_%d.raw", filename, mMagicNumber);
    FILE* fd = fopen(strFile, "wb");
    if (fd) {
        fwrite((void *)mVa, mSize, 1, fd);
        fclose(fd);
        return MTRUE;
    }
#endif
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

StatisticBuf::StatisticBuf()
    : m_fgLog(MFALSE)
    , m_fgCmd(MFALSE)
    , m_fgFlush(MFALSE)
    , m_i4TotalBufSize(0)
{
    m_rBufInfo.mSize = 0;
    m_rBufInfo.mVa = (MUINTPTR)NULL;
}

StatisticBuf::StatisticBuf(MUINT32 const i4BufSize)
{
    m_fgLog = property_get_int32("vendor.debug.statistic_buf.enable", 0);
    allocateBuf(i4BufSize);
}

StatisticBuf::~StatisticBuf()
{
    freeBuf();
}

MBOOL StatisticBuf::allocateBuf(MUINT32 const i4BufSize)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (!m_rBufInfo.mVa || !m_i4TotalBufSize) {
        m_i4TotalBufSize = i4BufSize;
        m_rBufInfo.mVa = (MUINTPTR) new MUINT8[m_i4TotalBufSize + 1];
        CAM_LOGD_IF(m_fgLog, "allocateBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
    }

    return MTRUE;
}

MBOOL StatisticBuf::freeBuf()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_rBufInfo.mVa && m_i4TotalBufSize) {
        CAM_LOGD_IF(m_fgLog, "freeBuf(): mVa(%p).", (void *)m_rBufInfo.mVa);
        delete [] (MUINT8 *)m_rBufInfo.mVa;
        m_i4TotalBufSize = 0;
    }

    return MTRUE;
}

MBOOL StatisticBuf::write(StatisticBufInfo rBufInfo)
{
    std::lock_guard<std::mutex> lock(m_Lock);

    if (!m_rBufInfo.mVa) {
        CAM_LOGE("write(): mVa is NULL.");
        return MFALSE;
    }

    if (rBufInfo.mSize > m_i4TotalBufSize){
        CAM_LOGE("write(): size overflow.");
        return MFALSE;
    }

    CAM_LOGD_IF(m_fgLog, "write(): m_fgCmd(%d), magic number(%d).",
            m_fgCmd, rBufInfo.mMagicNumber);

    /* copy buffer info */
    m_rBufInfo.mMagicNumber = rBufInfo.mMagicNumber;
    m_rBufInfo.mSize = rBufInfo.mSize;
    m_rBufInfo.mStride = rBufInfo.mStride;
    m_rBufInfo.mFrameCount = rBufInfo.mFrameCount;
    m_rBufInfo.mTimeStamp = rBufInfo.mTimeStamp;
    m_rBufInfo.mConfigNum = rBufInfo.mConfigNum;
    m_rBufInfo.mConfigLatency = rBufInfo.mConfigLatency;
    memcpy((void *)m_rBufInfo.mVa, (void *)rBufInfo.mVa, rBufInfo.mSize);

    /* update cmd and broadcast */
    m_fgCmd = MTRUE;
    m_Cond.notify_all();

    return MTRUE;
}

StatisticBufInfo* StatisticBuf::read()
{
    std::unique_lock<std::mutex> lock(m_Lock);

    if (!m_rBufInfo.mVa) {
        CAM_LOGE("read(): mVa is NULL.");
        return NULL;
    }

    if (m_fgFlush)
        return NULL;

    CAM_LOGD_IF(m_fgLog, "read(): m_fgCmd(%d).", m_fgCmd);

    /* wait for new data */
    if (!m_fgCmd)
        m_Cond.wait(lock);
    m_fgCmd = MFALSE;

    if (m_fgFlush)
        return NULL;
    else
        return &m_rBufInfo;
}

MBOOL StatisticBuf::flush()
{
    std::lock_guard<std::mutex> lock(m_Lock);

    CAM_LOGD_IF(m_fgLog, "flush(): m_fgCmd(%d).", m_fgCmd);

    /* update flush and broadcast */
    m_fgFlush = MTRUE;
    if (!m_fgCmd)
        m_Cond.notify_all();

    return MTRUE;
}

