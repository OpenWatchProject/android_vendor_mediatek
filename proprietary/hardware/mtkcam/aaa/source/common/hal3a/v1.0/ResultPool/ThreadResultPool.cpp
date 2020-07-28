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
#define LOG_TAG "ResultPool_thread"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#include <utils/threads.h>
#include "aaa_utils.h"
#include <mtkcam/def/PriorityDefs.h>
#include <mtkcam/utils/std/common.h>

#include <pthread.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <sys/resource.h>

#include <mtkcam/utils/std/Log.h>

#include <cutils/properties.h>

#include "ThreadResultPool.h"

#include <mtkcam/utils/std/Trace.h>
#include <aaa_trace.h>

/*******************************************************************************
*
*   utilities
*
********************************************************************************/
#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define GET_PROP(prop, init, val)\
{\
    val = property_get_int32(prop, (init));\
}

using namespace NS3Av3;

ThreadResultPool*
ThreadResultPool::
createInstance(MINT32 i4SensorDev)
{
    ThreadResultPool* pObj = new ThreadResultPool(i4SensorDev);
    return pObj;
}

ThreadResultPool::
ThreadResultPool(MINT32 i4SensorDev)
    : m_i4SensorDev(i4SensorDev)
    , m_bDisableResultThd(0)
    , m_rThread()
    , m_pResultPoolObj(NULL)
    , m_u4LogEn(0)
{
    CAM_LOGD("[%s] construct sensor(%d)", __FUNCTION__, m_i4SensorDev);
}

MVOID
ThreadResultPool::
init()
{
    GET_PROP("vendor.debug.ThreadPool.log", 0, m_u4LogEn);
    CAM_LOGD("[%s] sensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    m_bDisableResultThd = MFALSE;
    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);
    m_pResultPoolObj = NULL;
}


ThreadResultPool::
~ThreadResultPool()
{
    CAM_LOGD("[%s] sensorDev(%d)", __FUNCTION__, m_i4SensorDev);
}

MBOOL
ThreadResultPool::
destroyInstance()
{
    CAM_LOGD("[%s] +", __FUNCTION__);
    m_bDisableResultThd = MTRUE;
    waitFinished();
    delete this;
    CAM_LOGD("[%s] -", __FUNCTION__);
    return MTRUE;
}

MBOOL
ThreadResultPool::
waitFinished()
{
    CAM_LOGD("[%s] + sensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    postCmd(NULL);
    ::pthread_join(m_rThread, NULL);
    CAM_LOGD("[%s] - sensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    return MTRUE;
}

MVOID
ThreadResultPool::
changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, "ThreadResultPool", 0, 0, 0);
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if 0
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_3A_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_ResultPool;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    CAM_LOGD(
        "[%s] sensorDev(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)", __FUNCTION__
        , m_i4SensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}

MBOOL
ThreadResultPool::
postCmd(MVOID* pArg)
{
    Mutex::Autolock autoLock(m_PoolMtx);

    ResultPoolCmd4Convert_T rCmd;
    // data clone
    if (pArg != NULL)
        rCmd = *reinterpret_cast<ResultPoolCmd4Convert_T*>(pArg);

    m_rCmdQ.push_back(rCmd);

    CAM_LOGD_IF(m_u4LogEn, "[%s] m_rCmdQ size(%d), Req(#%d), SubsampleCount(%d), ConvertType(%d)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.i4MagicNumReq, rCmd.i4SubsampleCount, rCmd.eConvertType);

    m_rCmdQCond.broadcast();
    return MTRUE;
}

MBOOL
ThreadResultPool::
getCmd(ResultPoolCmd4Convert_T& rCmd)
{
    Mutex::Autolock autoLock(m_PoolMtx);

    if (m_rCmdQ.size() == 0)
    {
        m_rCmdQCond.wait(m_PoolMtx);
    }

    rCmd = *m_rCmdQ.begin();
    CAM_LOGD_IF(m_u4LogEn, "[%s] m_rCmdQ size(%d), Req(#%d), SubsampleCount(%d), ConvertType(%d), m_bDisableResultThd(%d)",
        __FUNCTION__, (MINT32)m_rCmdQ.size(), rCmd.i4MagicNumReq, rCmd.i4SubsampleCount, rCmd.eConvertType, m_bDisableResultThd);

    m_rCmdQ.erase(m_rCmdQ.begin());

    if (m_bDisableResultThd)
    {
        return MFALSE;
    }

    return MTRUE;
}

MVOID*
ThreadResultPool::
onThreadLoop(MVOID* pArg)
{
    CAM_LOGD("[%s]+ " ,__FUNCTION__);

    ThreadResultPool* _this = reinterpret_cast<ThreadResultPool*>(pArg);
    MINT32 i4SensorDev = _this->m_i4SensorDev;
    IResultPool* pResultPoolObj = _this->m_pResultPoolObj;
    ResultPoolCmd4Convert_T rCmd;
    MINT32 i4IsConvert4MetaResult = E_NON_CONVERT;
    CAM_LOGD_IF(_this->m_u4LogEn, "[%s] SensorDev = (%d %d)", __FUNCTION__, i4SensorDev, _this->m_i4SensorDev);

    // (1) change thread setting
    _this->changeThreadSetting();

    // (2) thread-in-loop
    while(_this->getCmd(rCmd))
    {
        CAM_LOGD_IF(_this->m_u4LogEn, "[%s] 2A & AF ConvertToMetadata (Req, SubsampleCount) = (#%d, %d)", __FUNCTION__, rCmd.i4MagicNumReq, rCmd.i4SubsampleCount);

        //===== Slow motion =====
        AAA_TRACE_HAL(ConvertToMetadata);
        i4IsConvert4MetaResult = E_NON_CONVERT;
        if(rCmd.i4SubsampleCount > 1)
        {
            SLOW_MOTION_3A_INFO_T rLocalNew3AInfo;
            SLOW_MOTION_3A_INFO_T rLocalOld3AInfo;

            //===== Get new result =====
            AllResult_T *pAllResult = pResultPoolObj->getInstance(i4SensorDev)->getAllResult(rCmd.i4MagicNumReq);
            if(pAllResult == NULL)
                CAM_LOGE("[%s] pAllResult NULL", __FUNCTION__);
            else
            {
                //===== update new to local =====
                AEResultToMeta_T *pAEResult = (AEResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AE_RESULTTOMETA]->read());
                AFResultToMeta_T *pAFResult = (AFResultToMeta_T*)(pAllResult->ModuleResultAddr[E_AF_RESULTTOMETA]->read());
                rLocalNew3AInfo.u1AeState = pAEResult->u1AeState;
                rLocalNew3AInfo.i8SensorExposureTime = pAEResult->i8SensorExposureTime;
                rLocalNew3AInfo.i4SensorSensitivity = pAEResult->i4SensorSensitivity;
                rLocalNew3AInfo.rAfInfo.u1AfState = pAFResult->u1AfState;

                //===== old setting update to new setting =====
                rLocalOld3AInfo.u1AeState = pAllResult->rOld3AInfo.u1AeState;
                rLocalOld3AInfo.i8SensorExposureTime = pAllResult->rOld3AInfo.i8SensorExposureTime;
                rLocalOld3AInfo.i4SensorSensitivity = pAllResult->rOld3AInfo.i4SensorSensitivity;
                rLocalOld3AInfo.rAfInfo.u1AfState = pAllResult->rOld3AInfo.rAfInfo.u1AfState;

                pAEResult->u1AeState = rLocalOld3AInfo.u1AeState;
                pAEResult->i8SensorExposureTime = rLocalOld3AInfo.i8SensorExposureTime;
                pAEResult->i4SensorSensitivity = rLocalOld3AInfo.i4SensorSensitivity;
                pAFResult->u1AfState = rLocalOld3AInfo.rAfInfo.u1AfState;

                //===== One result to update 4 metadata =====
                for(MINT32 i=0; i < rCmd.i4SubsampleCount; i++)
                {
                    MINT32 i4Magic = pAllResult->rOld3AInfo.i4ConvertMagic[i];
                    if( pAFResult->u1AfState != rLocalNew3AInfo.rAfInfo.u1AfState)
                        pAFResult->u1AfState = rLocalNew3AInfo.rAfInfo.u1AfState;

                    if( pAEResult->u1AeState != rLocalNew3AInfo.u1AeState)
                        pAEResult->u1AeState = rLocalNew3AInfo.u1AeState;

                    if( (pAEResult->i8SensorExposureTime != rLocalNew3AInfo.i8SensorExposureTime) && (static_cast<MUINT32>(rCmd.i4SubsampleCount) >= pAllResult->rOld3AInfo.rGainDelay.u4SutterGain_Delay) )
                        pAEResult->i8SensorExposureTime = rLocalNew3AInfo.i8SensorExposureTime;

                    if( (pAEResult->i4SensorSensitivity != rLocalNew3AInfo.i4SensorSensitivity) && (static_cast<MUINT32>(rCmd.i4SubsampleCount) >= pAllResult->rOld3AInfo.rGainDelay.u4SensorGain_Delay) )
                        pAEResult->i4SensorSensitivity = rLocalNew3AInfo.i4SensorSensitivity;

                    AllMetaResult_T *pMetaResult = pResultPoolObj->getInstance(i4SensorDev)->getMetadata(i4Magic);

                    CAM_LOGD_IF(_this->m_u4LogEn, "[%s] SMVR (Req, ConvertType, pMetaResult, pAllResult) = (#%d, %d, %p, %p)", __FUNCTION__, i4Magic, rCmd.eConvertType, pMetaResult, pAllResult);

                    if(rCmd.eConvertType == E_2A_CONVERT) // 2A convert
                    {
                        if(pMetaResult != NULL)
                        {
                            {
                                Mutex::Autolock lock(pMetaResult->LockMetaResult);
                                i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                            }
                            if(!(i4IsConvert4MetaResult & E_2A_CONVERT))
                                pResultPoolObj->getInstance(i4SensorDev)->convertToMetadata(pAllResult, pMetaResult);
                        }
                        else
                            CAM_LOGE("[%s] pMetaResult NULL", __FUNCTION__);
                    }
                    else // AF convert
                    {
                        if(pMetaResult != NULL)
                        {
                            {
                                Mutex::Autolock lock(pMetaResult->LockMetaResult);
                                i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                            }
                            if(!(i4IsConvert4MetaResult & E_AF_CONVERT))
                                pResultPoolObj->getInstance(i4SensorDev)->convertToMetadata4AF(pAllResult, pMetaResult);
                        }
                        else
                            CAM_LOGE("[%s] SMVR  pMetaResult", __FUNCTION__);
                    }
                    // Only print log
                    if(pMetaResult != NULL)
                    {
                        {
                            Mutex::Autolock lock(pMetaResult->LockMetaResult);
                            i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                        }
                    }
                }
            }
        }
        else //===== Normal =====
        {
            AllMetaResult_T *pMetaResult = NULL;
            AllResult_T *pAllResult = NULL;

            CAM_LOGD_IF(_this->m_u4LogEn, "[%s] Normal (Req, ConvertType) = (#%d, %d)", __FUNCTION__, rCmd.i4MagicNumReq, rCmd.eConvertType);

            if(rCmd.eConvertType == E_2A_CONVERT) // 2A convert
            {
                pMetaResult = pResultPoolObj->getInstance(i4SensorDev)->getMetadata(rCmd.i4MagicNumReq);
                pAllResult = pResultPoolObj->getInstance(i4SensorDev)->getAllResult(rCmd.i4MagicNumReq);

                CAM_LOGD_IF(_this->m_u4LogEn, "[%s] (pMetaResult, pAllResult) = (%p, %p)", __FUNCTION__, pMetaResult, pAllResult);

                if(pMetaResult != NULL && pAllResult != NULL)
                {
                    {
                        Mutex::Autolock lock(pMetaResult->LockMetaResult);
                        i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                    }
                    if(!(i4IsConvert4MetaResult & E_2A_CONVERT))
                        pResultPoolObj->getInstance(i4SensorDev)->convertToMetadata(pAllResult, pMetaResult);
                }
                else
                    CAM_LOGE("[%s] 2A pMetaResult or pAllResult NULL",__FUNCTION__);
            }
            else // AF convert
            {
                pMetaResult = pResultPoolObj->getInstance(i4SensorDev)->getMetadata(rCmd.i4MagicNumReq);
                pAllResult = pResultPoolObj->getInstance(i4SensorDev)->getAllResult(rCmd.i4MagicNumReq);

                CAM_LOGD_IF(_this->m_u4LogEn, "[%s] (pMetaResult, pAllResult) = (%p, %p)", __FUNCTION__, pMetaResult, pAllResult);
                if(pMetaResult != NULL && pAllResult != NULL)
                {
                    AAA_TRACE_HAL(AFconvertToMetadata);
                    {
                        Mutex::Autolock lock(pMetaResult->LockMetaResult);
                        i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                    }
                    if(!(i4IsConvert4MetaResult & E_AF_CONVERT))
                        pResultPoolObj->getInstance(i4SensorDev)->convertToMetadata4AF(pAllResult, pMetaResult);
                    AAA_TRACE_END_HAL;
                }
                else
                    CAM_LOGE("[%s] AF  pMetaResult or pAllResult NULL", __FUNCTION__);
            }
            // Only print log
            if(pMetaResult != NULL)
            {
                {
                    Mutex::Autolock lock(pMetaResult->LockMetaResult);
                    i4IsConvert4MetaResult = pMetaResult->i4IsConverted;
                }
            }
        }
        CAM_LOGD_IF(_this->m_u4LogEn, "[%s] 2A & AF(%d) ConvertToMetadata done, IsConvert4MetaResult(%d)", __FUNCTION__, rCmd.eConvertType, i4IsConvert4MetaResult);
        AAA_TRACE_END_HAL;
    }

    CAM_LOGD("[%s]- " ,__FUNCTION__);

    return NULL;
}
