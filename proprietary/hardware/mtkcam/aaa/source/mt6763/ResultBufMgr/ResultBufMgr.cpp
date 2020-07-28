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

/**
* @file ResultBufMgr.cpp
* @brief Implementation of Result Buffer Manager
*/

#define LOG_TAG "ResultBufMgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <mtkcam/utils/std/Log.h>

#include "ResultBufMgr.h"

using namespace NS3Av3;
using namespace android;

ResultBufMgr::
ResultBufMgr(MUINT32 u4Capacity)
    : m_u4Capacity(u4Capacity)
    , m_Lock()
{
}

ResultBufMgr::
~ResultBufMgr()
{
    clearAll();
    clearRepeatQueue();
}

MVOID
ResultBufMgr::
clearAll()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD("[%s] Size(%d)", __FUNCTION__, (MINT32)m_rResultQueue.size());
    m_rResultQueue.clear();
}

MINT32
ResultBufMgr::
updateResult(MINT32 i4FrmId, const Result_T& rResult)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
#if 0
    for (; it != m_rResultQueue.end(); it++)
    {
        CAM_LOGD("[%s] #(%d)", __FUNCTION__, it->i4FrmId);
    }
#endif
    for (it = m_rResultQueue.begin(); it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            *it = rResult;
            i4Ret = 1;
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist, add
        m_rResultQueue.push_back(rResult);
        i4Ret = 0;
    }

    // remove item
    if (m_rResultQueue.size() > m_u4Capacity)
    {
        m_rResultQueue.erase(m_rResultQueue.begin());
    }

    return i4Ret;
}

MINT32
ResultBufMgr::
getResult(MINT32 i4FrmId, Result_T& rResult)
{
    Mutex::Autolock lock(m_Lock);

    MINT32 i4Ret = 0;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
    for (; it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            rResult = *it;
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] NG i4Pos(%d)", __FUNCTION__, i4Pos);
        i4Ret = -1;
    }

    return i4Ret;
}

MINT32
ResultBufMgr::
getLast(Result_T& rResult)
{
    Mutex::Autolock lock(m_Lock);

    if (!m_rResultQueue.empty())
    {
        ResultList_T::iterator it = m_rResultQueue.end();
        it--;
        rResult = *it;
        return 0;
    }

    return -1;
}

MBOOL
ResultBufMgr::
lockBuf()
{
    m_Lock.lock();
    return MTRUE;
}

MBOOL
ResultBufMgr::unlockBuf()
{
    m_Lock.unlock();
    return MTRUE;
}

Result_T*
ResultBufMgr::getResult(MINT32 i4FrmId)
{
    //Mutex::Autolock lock(m_Lock);

    Result_T* pBuf = NULL;
    MINT32 i4Pos = 0;
    MINT32 i4Size = m_rResultQueue.size();
    ResultList_T::iterator it = m_rResultQueue.begin();
    for (; it != m_rResultQueue.end(); it++, i4Pos++)
    {
        if (it->i4FrmId == i4FrmId)
        {
            pBuf = &(*it);
            //CAM_LOGD("[%s] OK i4Pos(%d)", __FUNCTION__, i4Pos);
            break;
        }
    }

    if (i4Pos == i4Size)
    {
        // does not exist
        CAM_LOGD("[%s] Ref NG i4Pos(%d)", __FUNCTION__, i4Pos);
    }

    return pBuf;
}

MVOID
ResultBufMgr::
updateCapacity(MUINT32 u4FrmId)
{
    m_u4Capacity = u4FrmId;
}

MVOID
ResultBufMgr::
clearRepeatQueue()
{
    Mutex::Autolock lock(m_Lock);
    CAM_LOGD("[%s] Size(%d)", __FUNCTION__, (MINT32)m_rRepeatResultQueue.size());
    m_rRepeatResultQueue.clear();
}

MBOOL ResultBufMgr::
isRepeatQueueEmpty()
{
    return m_rRepeatResultQueue.empty();
}

MBOOL ResultBufMgr::
queryRepeatQueue(MINT32 maginNum)
{
    Mutex::Autolock lock(m_Lock);
    List<MINT32>::iterator it = m_rRepeatResultQueue.begin();
    for (; it != m_rRepeatResultQueue.end(); it++)
    {
        if (*it == maginNum)//found repeat magic# in RepeatResultQueue
        {
            //CAM_LOGD("[%s] Matched mag=%d", __FUNCTION__, maginNum);
            m_rRepeatResultQueue.erase(it);//erase from queue
            return MFALSE;
        }
    }
    return MTRUE;
}

MVOID ResultBufMgr::
updateRepeatQueue(MINT32 maginNum)
{
    Mutex::Autolock lock(m_Lock);
    //CAM_LOGD("[%s] update mag=%d",__FUNCTION__,maginNum);
    m_rRepeatResultQueue.push_back(maginNum);
}
