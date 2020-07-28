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
* @file IEventIrq.h
* @brief Declarations of IRQ Event waiting interface
*/

#ifndef __DEFAULT_IEVENT_IRQ_H__
#define __DEFAULT_IEVENT_IRQ_H__

#include "IEventIrq.h"
#include <utils/Mutex.h>
#include <utils/threads.h>
#include <string>
#include <semaphore.h>
#include <aaa_types.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/Log.h>

namespace NS3Av3
{
using namespace android;
/******************************************************************************
 *  Default EventIrq
 ******************************************************************************/
class DefaultEventIrq
{
public:
    /**
     * @brief Create instance of IEventIrq
     * @param [in] rCfg config of sensor, tg, and event for listening.
     * @param [in] strUser username
     */
    static DefaultEventIrq*     createInstance(const IEventIrq::ConfigParam& rCfg, const char* strUser);

    /**
     * @brief Destroy instance of IEventIrq
     * @param [in] strUser username
     */
    virtual MVOID               destroyInstance(const char* strUser);

    /**
     * @brief Register to listen the event
     * @return
     * - MINT32 0 for success
     */
    virtual MINT32              mark();

    /**
     * @brief Wait for the event
     * @param [out] rDuration
     * @return
     * - 0 for blocking wait case
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              wait(IEventIrq::Duration& rDuration);

    /**
     * @brief Query for the event
     * @param [out] rDuration
     * @return
     * - 0 for indicating the event not yet happened
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              query(IEventIrq::Duration& rDuration);

    virtual MINT32              setTgInfo(MINT32 i4TgInfo);

protected:
    DefaultEventIrq();
    virtual ~DefaultEventIrq(){}

    MVOID                       init(const IEventIrq::ConfigParam& rCfg, const char* strUser);
    MVOID                       uninit(const char* strUser);

    static MVOID*               threadLoop(MVOID*);

    inline MINT32               getUsTime()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    MINT32                      m_i4User;
    MINT32                      m_i4EventCnt;
    MINT32                      m_i4TimeStampMark;
    MINT32                      m_i4TimeStampEvent;
    pthread_t                   m_Thread;
    mutable Mutex               m_Lock;
    mutable Mutex               m_EventMtx;
    Condition                   m_EventCond;
    sem_t                       m_SemEvent;
    MBOOL                       m_fgThreadOn;
    IEventIrq::ConfigParam      m_rCfgParam;
    std::string                 m_strName;
};
};
#endif //__IEVENT_IRQ_H__
