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
#ifndef _ISP_TUNING_CUSTOM_INSTANCE_H_
#define _ISP_TUNING_CUSTOM_INSTANCE_H_

#include "kd_imgsensor.h"

#include <private/aaa_hal_private.h>
#include <private/aaa_utils.h>
#include <array>

namespace NSIspTuning
{

/*******************************************************************************
*
*******************************************************************************/
class CTIspTuningCustom : public IspTuningCustom
{
public:
    virtual void destroyInstance() {}

    CTIspTuningCustom(MUINT32 const eSensorDev, MUINT32 const u4SensorID)
        : IspTuningCustom()
        , m_eSensorDev(eSensorDev)
        , m_u4SensorID(u4SensorID)
    {}

public:     ////    Attributes
    virtual MUINT32   getSensorDev() const { return m_eSensorDev; }
    virtual MUINT32   getSensorID() const { return m_u4SensorID; }

protected:  ////    Data Members.
    MUINT32 const m_eSensorDev;
    MUINT32 const m_u4SensorID;
};


/*******************************************************************************
* Customers can specialize CTIspTuningCustom<xxx>
* and then override default behaviors if needed.
*******************************************************************************/
#if 0
/*
    ps: where ESensorDev_xxx = ESensorDev_Main/ESensorDev_MainSecond/ESensorDev_Sub
*/
template <>
class CTIspTuningCustom< ESensorDev_Sub > : public IspTuningCustom
{
public:
    static
    IspTuningCustom*
    getInstance(MUINT32 const u4SensorID)
    {
        static CTIspTuningCustom<ESensorDev_Sub> singleton(u4SensorID);
        return &singleton;
    }
    virtual void destroyInstance() {}

    CTIspTuningCustom(MUINT32 const u4SensorID)
        : IspTuningCustom()
        , m_rIdxSetMgr(IdxSetMgrBase::getInstance())
        , m_u4SensorID(u4SensorID)
    {}

public:     ////    Attributes
    virtual ESensorDev_T   getSensorDev() const { return ESensorDev_Sub; }
    virtual MUINT32   getSensorID() const { return m_u4SensorID; }
    virtual INDEX_T const* getDefaultIndex(EIspProfile_T const eIspProfile,
                                           ESensorMode_T const eSensorMode,
                                           EIndex_Scene_T const eIdx_Scene,
                                           EIndex_ISO_T const eIdx_ISO,
                                           EZoom_T eIdx_Zoom) const
    {
        return m_rIdxSetMgr.get(eIspProfile, eSensorMode, eIdx_Scene, eIdx_ISO, eIdx_Zoom);
    }

public:     ////    Overrided Interfaces.
    // virtual EIndex_Shading_CCT_T evaluate_Shading_CCT_index (RAWIspCamInfo const& rCamInfo) const;

protected:  ////    Data Members.
    IdxSetMgrBase&  m_rIdxSetMgr;
    MUINT32 const m_u4SensorID;
};

#endif


template <ESensorDev_T const eSensorDev, MUINT uSensorDevID>
class CTIspTuningCustomBySensor : public IspTuningCustom
{
public:
    static
    IspTuningCustom*
    getInstance(MUINT32 const u4SensorID)
    {
        static CTIspTuningCustomBySensor<eSensorDev, uSensorDevID> singleton(u4SensorID);
        return &singleton;
    }
    virtual void destroyInstance() {}

    CTIspTuningCustomBySensor(MUINT32 const u4SensorID)
        : IspTuningCustom()
        , m_u4SensorID(u4SensorID)
    {}

public:     ////    Attributes
    virtual MUINT32   getSensorDev() const { return eSensorDev; }
    virtual MUINT32   getSensorID() const { return m_u4SensorID; }

protected:  ////    Data Members.
    MUINT32 const m_u4SensorID;

};

/*******************************************************************************
* Customers can specialize CTIspTuningCustomBySensor<xxx>
* and then override default behaviors if needed.
*******************************************************************************/
template <>
class CTIspTuningCustomBySensor<ESensorDev_Main, IMX258_SENSOR_ID> : public IspTuningCustom
{
public:
    static
    IspTuningCustom*
    getInstance(MUINT32 const u4SensorID)
    {
        static CTIspTuningCustomBySensor<ESensorDev_Main, IMX258_SENSOR_ID> singleton(u4SensorID);
        return &singleton;
    }
    virtual void destroyInstance() {}

    CTIspTuningCustomBySensor(MUINT32 const u4SensorID)
        : IspTuningCustom()
        , m_u4SensorID(u4SensorID)
    {}

public:     ////    Attributes
    virtual MUINT32   getSensorDev() const { return ESensorDev_Main; }
    virtual MUINT32   getSensorID() const { return m_u4SensorID; }

public:     ////    Overrided Interfaces.
    //virtual EIndex_Shading_CCT_T evaluate_Shading_CCT_index (RAWIspCamInfo const& rCamInfo) const;

protected:  ////    Data Members.
    MUINT32 const m_u4SensorID;

};
/*******************************************************************************
*
*******************************************************************************/

#define MY_INST NS3Av3::INST_T<IspTuningCustom>
    static std::array<MY_INST, SENSOR_IDX_MAX> gMultiton;

IspTuningCustom*
IspTuningCustom::
createInstance(MUINT32 const eSensorDev, MUINT32 const u4SensorID)
{
    auto i4SensorIdx = NS3Av3::mapSensorDevToIdx(eSensorDev);
    if(i4SensorIdx >= SENSOR_IDX_MAX || i4SensorIdx < 0) {
        ALOGE("[%s] Unsupport sensor Index: %d\n", __FUNCTION__, i4SensorIdx);
        return NULL;
    }
    MY_INST& rSingleton = gMultiton[i4SensorIdx];
    std::call_once(rSingleton.onceFlag, [&] {
        rSingleton.instance = std::make_unique<CTIspTuningCustom>(eSensorDev, u4SensorID);
    } );
    return rSingleton.instance.get();
}


};  //  NSIspTuning
#endif  //  _ISP_TUNING_CUSTOM_INSTANCE_H_

