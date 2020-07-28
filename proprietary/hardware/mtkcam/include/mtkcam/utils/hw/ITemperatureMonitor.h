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
 * @file ITemperatureMonitor.h
 * @brief The interface of temperature monitor.
*/
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_ITEMPERATUREMONITOR_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_ITEMPERATUREMONITOR_H_
//
//
// Standard C header file
// Android system/core header file
#include <utils/Thread.h>
// mtkcam custom header file
// mtkcam global header file
#include <mtkcam/def/common.h>
// Module header file
// Local header file
/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam{
namespace v3{
/*******************************************************************************
* Class Define
********************************************************************************/
/**
 * @class ITemperatureMonitor
 * @brief The interface of TemperatureMonitor
 */
class ITemperatureMonitor : public android::Thread
{
public:
    /**
     * @brief create temperature monitor instance
     * @return return instance
     */
    static android::sp<ITemperatureMonitor>
                                    create();
    /**
     * @brief init temperature monitor and create SensorTemperator instance.
     * @return if init success return MTRUE.
     */
    virtual MBOOL init() = 0;
    /**
     * @brief uninit temperature monitor and release SensorTemperator instance.
     * @return if uninit success return MTRUE.
     */
    virtual MBOOL uninit() = 0;
    /**
     * @brief add sensor to observe list.
     * @param [in] iOpenId open id.
     * @param [in] iSensorDev sensor dev id.
     * @return If add success return MTURE.
     */
    virtual MBOOL addToObserve(MUINT32 iOpenId, MUINT32 iSensorDev) = 0;
    /**
     * @brief update logic. It will query to sensor driver to get temperature.
     * @return always return true.
     */
    virtual bool threadLoop() = 0;
};
};
};
#endif