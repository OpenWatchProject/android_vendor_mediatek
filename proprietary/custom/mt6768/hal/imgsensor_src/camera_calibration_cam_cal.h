/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#ifndef __CAMERA_CALIBRATION_CAM_CAL_H
#define __CAMERA_CALIBRATION_CAM_CAL_H

#include <mtkcam/drv/mem/icam_cal_custom.h>
#include <mtkcam/drv/mem/icam_cal_helper.h>

UINT32 CAM_CALGetCalData(UINT32* pGetSensorCalData);

using namespace NSCam;

namespace NSCam {

class CamCalCustomImpl : public ICamCalCustom
{
public:
    CamCalCustomImpl(ICamCalHelper *pCamCalHelper);
    virtual ~CamCalCustomImpl() {}

    /**
     * Get the calibration data
     *
     * @param calCfg The configuration of calibration data
     * @return error code
     */
    virtual unsigned int getCalData(unsigned int* pGetSensorCalData);

    /**
     * Query the EEPROM sensor dev id by sensor dev id and sensor id.
     *
     * @param sensorDev Sensor dev id
     * @param sensorId Sensor id
     * @return Sensor dev id of EEPROM
     */
    virtual unsigned int getEepromSensorDev(unsigned int const sensorDev, unsigned int sensorId);

private:
    /**
     * Query has cal data or not by sensor dev id and sensor id.
     *
     * @param sensorDev Sensor dev id
     * @param sensorId Sensor id
     * @return {@code false} if this module doesn't have cal data
     */
    virtual bool hasCalData(unsigned int const sensorDev, unsigned int sensorId);
};

};  //namespace NSCam

#endif /* __CAMERA_CALIBRATION_H */

