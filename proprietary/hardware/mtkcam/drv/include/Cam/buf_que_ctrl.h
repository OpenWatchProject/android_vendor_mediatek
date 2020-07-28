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
 **************************************************/
#ifndef __BUF_QUE_CTRL_H__
#define __BUF_QUE_CTRL_H__

#include <vector>
#include <map>
#include <list>
#include <sys/prctl.h>
#include <semaphore.h>
#include <deque>
#include "utils/Mutex.h"    // android mutex


#ifndef MFALSE
#define MFALSE 0
#endif
#ifndef MTRUE
#define MTRUE 1
#endif
#ifndef MUINT8
typedef unsigned char MUINT8;
#endif

#ifndef MUINT32
typedef unsigned int MUINT32;
#endif
#ifndef MINT32
typedef int MINT32;
#endif
#ifndef MBOOL
typedef int MBOOL;
#endif
#ifndef MINTPTR
typedef intptr_t                MINTPTR;
#endif
#ifndef MUINTPTR
typedef uintptr_t               MUINTPTR;
#endif

template<typename typ>
class QueueMgr {
/////
protected:
    std::deque<typ>         m_v_pQueue;
    mutable android::Mutex           mQueueLock;
/////
public:
    QueueMgr()
        : m_v_pQueue()
    {}

    virtual ~QueueMgr(){};
    ////
            void    init();
            void    uninit();
    //FIFO cmd
    typedef enum{
        eCmd_getsize = 0,
        eCmd_push = 1,
        eCmd_pop = 2,
        eCmd_front = 3,
        eCmd_back = 4,
        eCmd_at = 5,
        eCmd_pop_back = 6,
        eCmd_erase = 7,
    }QueueMgr_cmd;
            MBOOL   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
};





#undef BUF_QUE_LOG_TAG
#define BUF_QUE_LOG_TAG "buf_que"

#ifndef USING_MTK_LDVT
#include <cutils/log.h>
#define QUE_LOG_WRN(fmt, arg...)       ALOGW("[%s] WARNING: " fmt , __FUNCTION__, ##arg)
#define QUE_LOG_ERR(fmt, arg...)       ALOGE("[%s, %s, line%04d] ERROR: " fmt , __FILE__, __FUNCTION__, __LINE__, ##arg)
#else   // LDVT
#include <stdlib.h>
#define QUE_LOG_WRN(fmt, arg...)    printf("[%s]Warning:(%5d): " fmt, __FUNCTION__, __LINE__, ##arg)
#define QUE_LOG_ERR(fmt, arg...)    printf("[%s]ERROR:(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#endif

using namespace std;

template<typename typ>
void QueueMgr<typ>::init(void)
{
    this->m_v_pQueue.clear();
}

template<typename typ>
void QueueMgr<typ>::uninit(void)
{
    this->m_v_pQueue.clear();
}

template<typename typ>
MBOOL QueueMgr<typ>::sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2)
{
    MBOOL ret = MTRUE;
    android::Mutex::Autolock lock(this->mQueueLock);

    switch(cmd){
        case eCmd_getsize:
            *(MUINT32*)arg1 = this->m_v_pQueue.size();
            break;
        case eCmd_push:
            this->m_v_pQueue.push_back(*(typ*)(arg1));
            break;
        case eCmd_pop:
            this->m_v_pQueue.pop_front();
            break;
        case eCmd_front:
            *(typ*)arg1 = this->m_v_pQueue.front();
            break;
        case eCmd_at:
            if(this->m_v_pQueue.size()>arg1){
                *(typ*)arg2 = this->m_v_pQueue.at(arg1);
            }
            else{
                QUE_LOG_WRN("size have been reduce to 0x%x(0x%lx) by enque\n",this->m_v_pQueue.size(),arg1);
                ret = MFALSE;
            }
            break;
        case eCmd_pop_back:
            this->m_v_pQueue.pop_back();
            break;
        case eCmd_erase:
            if(this->m_v_pQueue.size() > arg1)
                this->m_v_pQueue.erase(this->m_v_pQueue.begin() + (MUINT32)arg1);
            else{
                QUE_LOG_ERR("error: out of range:%d_%d\n",arg1,this->m_v_pQueue.size());
                ret = MFALSE;
            }
            break;
        case eCmd_back:
            *(typ*)arg1 = this->m_v_pQueue.back();
            break;
        default:
            QUE_LOG_ERR("error: unsopported cmd:0x%x\n",cmd);
            ret = MFALSE;
            break;
    }
    return ret;
}

#endif
