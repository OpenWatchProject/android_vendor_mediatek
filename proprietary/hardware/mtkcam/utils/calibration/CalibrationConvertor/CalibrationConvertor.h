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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATION_CONVERTOR_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATION_CONVERTOR_H_
//
#include <utils/RefBase.h>
#include <mtkcam/utils/calibration/CalibrationTypes.h>

using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSCalibrationConvertor {


/******************************************************************************
 *  Calibratoin Convertor
 ******************************************************************************/
class CalibrationConvertor : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    CalibrationQueryParam           _sensorParam;

public:     ////                    Instantiation.
    CalibrationConvertor(const CalibrationQueryParam &param);

    /**
     * \brief Convert your calibration data to Google format
     * \details Google requires four static metadata for ITS, they are:
     *          1. Lens Pose Rotation: [x, y, z, w]
     *          2. Lens Pose Translation: [x, y, z]
     *          3. Lens Intrinsic Calibration: [f_x, f_y, c_x, c_y, s]
     *          4. Lens Distortion: [kappa_1, kappa_2, kappa_3, kappa_4, kappa_5]
     *          You can also refer to:
     *          https://developer.android.com/reference/android/hardware/camera2/CameraCharacteristics.html
     *
     * \param result Calibration in Google format
     * \return true if you have done the convertion, otherwise return false
     */
    bool convertToGoogleFormat(CalibrationResultInGoogleFormat &result);

    /**
     * \brief Convert your calibration to MTK format
     * \details If the project use hardware depth of MTK, you have to convert calibration data to MTK format
     *
     * \param result Calibration in MTK format
     * \return true if you have done the convertion, otherwise return false.
     */
    bool convertToMTKFormat(CalibrationResultInMTKFormat &result);
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSCalibrationConvertor
};  // namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_UTILS__CALIBRATIONCONVERTOR_H_

