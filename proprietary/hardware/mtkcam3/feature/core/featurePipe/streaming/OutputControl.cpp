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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define PIPE_CLASS_TAG "OutputControl"
#define PIPE_MODULE_TAG "OutputControl"
#include <featurePipe/core/include/PipeLog.h>

#include "OutputControl.h"

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

OutputControl::OutputControl(const std::vector<MUINT32> &allSensorID)
{
    mGeneralOut.store(0);
    for(MUINT32 id : allSensorID)
    {
        mSensorOut[id].store(0);
    }
}

OutputControl::~OutputControl()
{
}

MBOOL OutputControl::registerFillOut(GeneralTarget target, P2IO &out, MBOOL logw)
{
    MBOOL ret = registerGeneral(target, logw);
    if( !ret )
    {
        out = P2IO();
    }
    return ret;
}

MBOOL OutputControl::registerFillOut(GeneralTarget target, android::sp<IIBuffer> &out, MBOOL logw)
{
    MBOOL ret = registerGeneral(target, logw);
    if( !ret )
    {
        out = NULL;
    }
    return ret;
}

MBOOL OutputControl::registerFillOuts(GeneralTarget target, std::vector<P2IO> &outList, MBOOL logw)
{
    MBOOL ret = registerGeneral(target, logw);
    if( !ret )
    {
        outList.clear();
    }
    return ret;
}

MBOOL OutputControl::registerFillOut(SensorTarget target, MUINT32 sensorID, P2IO &out, MBOOL logw)
{
    MBOOL ret = registerSensor(target, sensorID, logw);
    if( !ret )
    {
        out = P2IO();
    }
    return ret;
}

MBOOL OutputControl::registerFillOuts(SensorTarget target, MUINT32 sensorID, std::vector<P2IO> &outList, MBOOL logw)
{
    MBOOL ret = registerSensor(target, sensorID, logw);
    if( !ret )
    {
        outList.clear();
    }
    return ret;
}

MBOOL OutputControl::registerGeneral(GeneralTarget target, MBOOL logw)
{
    MBOOL ret = MTRUE;
    if( mGeneralOut.fetch_or(target) & target )
    {
        MY_LOGW_IF(logw, " request gen output buffer in different place ! Set this buf empty ! target(%d)", target);
        ret= MFALSE;
    }
    return ret;
}

MBOOL OutputControl::registerSensor(SensorTarget target, MUINT32 sensorID, MBOOL logw)
{
    MBOOL ret = MTRUE;
    if( mSensorOut.count(sensorID) == 0 )
    {
        MY_LOGE("Unsupported Sensor ID (%d)", sensorID);
        ret = MFALSE;
    }
    else if( mSensorOut[sensorID].fetch_or(target) & target )
    {
        MY_LOGW_IF(logw, " request sensor output buffer in different place ! Set this buf empty ! target(%d)", target);
        ret= MFALSE;
    }
    return ret;
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
