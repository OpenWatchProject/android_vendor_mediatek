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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATIONPROVIDER_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATIONPROVIDER_H_
//
#include <mtkcam/utils/calibration/ICalibrationProvider.h>
#include "CalibrationConvertor/CalibrationConvertor.h"

using namespace NSCam;
using namespace android::NSCalibrationConvertor;
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSCalibrationProvider {


/******************************************************************************
 *  Calibratoin Provider
 ******************************************************************************/
class CalibrationProvider : public ICalibrationProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.


public:     ////                    Instantiation.
    virtual                         ~CalibrationProvider();
                                    CalibrationProvider(
                                        int32_t const   i4OpenId
                                    );

    virtual bool                    getCalibration(ENUM_CALIBRATION_FORMAT format, CalibrationResult *result) const override;
    virtual std::vector<float>      getLensIntrinsicCalibrationBySpec(float physicalWidth, float physicalHeight, float focalLength) const override;

protected:
    CalibrationQueryParam           _queryParam;
    sp<CalibrationConvertor>        _3rdPartyConvertor;
    int                             _logicalDeviceId = -1;
    int32_t                         _mainSensorId = -1;

private:
    virtual void                    __init(int32_t const i4OpenId);
    virtual bool                    __loadMTKCalibrationInGoogleFormat(CalibrationResultInGoogleFormat *result) const;
    virtual bool                    __loadMTKCalibrationInGoogleFormatFromSpec(CalibrationResultInGoogleFormat *result) const;
    virtual bool                    __loadMTKCalibrationInGoogleFormatFromFile(CalibrationResultInGoogleFormat *result) const;

    virtual bool                    __loadMTKCalibration(CalibrationResultInMTKFormat *result) const;
    virtual bool                    __loadMTKCalibrationFromFile(CalibrationResultInMTKFormat *result) const;
    virtual bool                    __loadMTKCalibrationFrom3rdParty(CalibrationResultInMTKFormat *result) const;
    virtual bool                    __loadMTKCalibrationFromEEPROM(CalibrationResultInMTKFormat *result) const;
    virtual bool                    __loadMTKCalibrationFromSetting(CalibrationResultInMTKFormat *result) const;
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCalibrationProvider
};  // namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATIONPROVIDER_H_

