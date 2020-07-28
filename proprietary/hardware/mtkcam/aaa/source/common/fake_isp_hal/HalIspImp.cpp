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
#define LOG_TAG "HalIspImp"

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/std/common.h>

//For Metadata
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#include <mtkcam/drv/IHalSensor.h>

#include <cutils/properties.h>

#include "HalIspImp.h"

#include <array>

using namespace NS3Av3;
using namespace NSCam;

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_ISP_HAL(String) \
          do { \
              aee_system_exception( \
                  "HalISP", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_ISP_HAL(String)
#endif

#define GET_PROP(prop, dft, val)\
{\
   val = property_get_int32(prop,dft);\
}

MUINT32 HalIspImp::m_u4LogEn = 0;


/*******************************************************************************
* implementations
********************************************************************************/
template <typename T>
struct INST_FOR_FAKE_ISP_T {
    std::once_flag onceFlag;
    std::unique_ptr<T> instance;
};
#define MY_INST_HAL_ISP_IMP INST_FOR_FAKE_ISP_T<HalIspImp>
#define SENSOR_IDX_MAX 20

static std::array<MY_INST_HAL_ISP_IMP, SENSOR_IDX_MAX> gMultitonHalIspImp;

HalIspImp*
HalIspImp::
createInstance(MINT32 const i4SensorIdx, const char* strUser)
{
    GET_PROP("vendor.debug.halisp.log", 0, m_u4LogEn);
    CAM_LOGD("[%s] sensorIdx(%d) %s", __FUNCTION__, i4SensorIdx, strUser);

    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        CAM_LOGE("Unsupport sensor Index: %d\n", i4SensorIdx);
        return nullptr;
    }

    MY_INST_HAL_ISP_IMP& rSingleton = gMultitonHalIspImp[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<HalIspImp>(i4SensorIdx);
        (rSingleton.instance)->init(strUser);
    } );

    return rSingleton.instance.get();
}

HalIspImp::
HalIspImp(MINT32 const i4SensorIdx)
    : m_Users(0)
    , m_Lock()
    , m_i4SensorIdx(i4SensorIdx)
    , m_i4SensorDev(0)
    , m_p3A(NULL)
{
    // query SensorDev from HalSensorList
    IHalSensorList* const pHalSensorList = MAKE_HalSensorList();
    if (!pHalSensorList)
        CAM_LOGE("[%s] MAKE HalSensorList fail", __FUNCTION__);
    else
        m_i4SensorDev = pHalSensorList->querySensorDevIdx(i4SensorIdx);

    CAM_LOGD("[%s] sensorIdx(0x%04x) sensorDev(%d)", __FUNCTION__, i4SensorIdx, m_i4SensorDev);
}

MVOID
HalIspImp::
destroyInstance(const char* strUser)
{
    CAM_LOGD("[%s]+ sensorIdx(%d)  User(%s)", __FUNCTION__, m_i4SensorIdx, strUser);
    uninit(strUser);
    CAM_LOGD("[%s]- ", __FUNCTION__);
}

MINT32
HalIspImp::
config(const ConfigInfo_T& rConfigInfo __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MVOID
HalIspImp::
setSensorMode(MINT32 i4SensorMode __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
}

MBOOL
HalIspImp::
init(const char* strUser __unused)
{
    CAM_LOGD("[%s] m_Users: %d, SensorDev %d, index %d \n", __FUNCTION__, std::atomic_load((&m_Users)), m_i4SensorDev, m_i4SensorIdx);

    // check user count
    std::lock_guard<std::mutex> lock(m_Lock);

    if (m_Users > 0)
    {
        CAM_LOGD("[%s] %d has created \n", __FUNCTION__, std::atomic_load((&m_Users)));
        MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
        return MTRUE;
    }

    m_p3A = MAKE_Hal3A(m_i4SensorIdx, LOG_TAG);
    if (m_p3A == NULL) {
        CAM_LOGE("[%s] create IHal3A fail", __FUNCTION__);
    }

    MINT32 ret __unused = std::atomic_fetch_add((&m_Users), 1);
    return MTRUE;
}

MBOOL
HalIspImp::
uninit(const char* strUser __unused)
{

    std::lock_guard<std::mutex> lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return MTRUE;
    }
    CAM_LOGD("[%s] m_Users: %d \n", __FUNCTION__, std::atomic_load((&m_Users)));

    // More than one user, so decrease one User.
    MINT32 ret __unused = std::atomic_fetch_sub((&m_Users), 1);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        if (m_p3A) {
            m_p3A->destroyInstance(LOG_TAG);
        }
    }
    else    // There are still some users.
    {
        CAM_LOGD("[%s] Still %d users \n", __FUNCTION__, std::atomic_load((&m_Users)));
    }
    return MFALSE;
}

MBOOL
HalIspImp::
start()
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MBOOL
HalIspImp::
stop()
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MBOOL
HalIspImp::
setP1Isp(const vector<MetaSet_T*>& requestQ, MBOOL const fgForce __unused /*MINT32 const i4SensorDev, RequestSet_T const RequestSet, MetaSet_T& control, MBOOL const fgForce, MINT32 i4SubsampleIdex*/)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MBOOL
HalIspImp::
setP2Isp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
    MINT32 i4Ret = -1;
    CAM_LOGD_IF(m_u4LogEn, "[%s] support before isp60", __FUNCTION__);
    if(m_p3A)
    {
        i4Ret = m_p3A->setIsp(flowType, control, pTuningBuf, pResult);
    }
    else
        CAM_LOGE("[%s] m_p3A is NULL", __FUNCTION__);

    return i4Ret;
}

MBOOL
HalIspImp::
setFdGamma(MINT32 i4Index, MINT32* pTonemapCurveRed, MUINT32 u4size){
    return MTRUE;
}

MINT32
HalIspImp::
sendIspCtrl(EISPCtrl_T eISPCtrl, MINTPTR iArg1, MINTPTR iArg2)
{
    switch (eISPCtrl)
    {
        // ----------------------------------ISP----------------------------------
        case EISPCtrl_GetIspGamma:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetIspGamma, iArg1, iArg2);
            break;
        case EISPCtrl_ValidatePass1:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_ValidatePass1, iArg1, iArg2);
            break;
        case EISPCtrl_SetIspProfile:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_SetIspProfile, iArg1, iArg2);
            break;
        case EISPCtrl_GetOBOffset:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetOBOffset, iArg1, iArg2);
            break;
        case EISPCtrl_GetRwbInfo:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetRwbInfo, iArg1, iArg2);
            break;
        case EISPCtrl_SetOperMode:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_SetOperMode, iArg1, iArg2);
            break;
        case EISPCtrl_GetOperMode:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetOperMode, iArg1, iArg2);
            break;
        case EISPCtrl_GetMfbSize:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetMfbSize, iArg1, iArg2);
            break;
        case EISPCtrl_GetLCEGain:
            if(m_p3A)
                m_p3A->send3ACtrl(E3ACtrl_GetLCEGain, iArg1, iArg2);
            break;
        default:
            CAM_LOGD("[%s] Unsupport Command(%d)", __FUNCTION__, eISPCtrl);
            return MFALSE;
    }
    return MTRUE;
}

MINT32
HalIspImp::
attachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MINT32
HalIspImp::
detachCb(/*IHal3ACb::ECb_T eId, IHal3ACb* pCb*/)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MVOID
HalIspImp::
setFDEnable(MBOOL fgEnable __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
}

MBOOL
HalIspImp::
setFDInfo(MVOID* prFaces __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MBOOL
HalIspImp::
setOTInfo(MVOID* prOT __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MINT32
HalIspImp::
dumpIsp(MINT32 flowType, const MetaSet_T& control, TuningParam* pTuningBuf, MetaSet_T* pResult)
{
    CAM_LOGD("[%s] support before isp60", __FUNCTION__);
    MINT32 i4Ret = 0;
    if(m_p3A)
        i4Ret = m_p3A->dumpIsp(flowType, control, pTuningBuf, pResult);
    else
        CAM_LOGE("[%s] m_p3A is NULL", __FUNCTION__);
    return i4Ret;
}

MINT32
HalIspImp::
get(MUINT32 frmId __unused, MetaSet_T& result __unused)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MINT32
HalIspImp::
getCur(MUINT32 frmId, MetaSet_T& result)
{
    CAM_LOGD_IF(m_u4LogEn, "[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MVOID
HalIspImp::
resume(MINT32 MagicNum __unused)
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
}

MINT32
HalIspImp::
InitP1Cb()
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}


MINT32
HalIspImp::
UninitP1Cb()
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}


MINT32
HalIspImp::
ConfigP1Cb()
{
    CAM_LOGD("[%s] not support before isp60", __FUNCTION__);
    return MFALSE;
}

MBOOL
HalIspImp::
queryISPBufferInfo(Buffer_Info& bufferInfo){
    return MFALSE;
}



