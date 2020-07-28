/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
/*
 ** $Log: cam_cal_helper.h $
 *
 *
 */

#ifndef _CAM_CAL_HELPER_H_
#define _CAM_CAL_HELPER_H_

#include <mtkcam/drv/mem/icam_cal_helper.h>
#include <mtkcam/drv/IHalSensor.h>

using namespace NSCam;

namespace NSCam {

class ICamCalCustom;

class CamCalHelper : public ICamCalHelper {

public:
    static CamCalHelper* getInstance();

    /**
     * Read eeprom data, it might return from memory or EEPROM.
     *
     * @param sensorId Sensor id
     * @param deviceId Sensor dev id
     * @param offset Offset to read
     * @param length Length to read
     * @param data Buffer to return the result
     * @return read data length, it might be negative number if error
     */
    virtual int readData(unsigned int sensorId, unsigned int deviceId,
            unsigned int offset, unsigned int length,
            unsigned char *data);

    /**
     * Get the calibration data
     *
     * @param calCfg The configuration of calibration data
     * @return error code
     */
    virtual unsigned int getCamCalData(unsigned int* pGetSensorCalData);

protected:
    CamCalHelper();

    virtual ~CamCalHelper();

private:
    /**
     * Reference of ICamCalCustom instance
     */
    ICamCalCustom *mpCamCalCustom;

    /**
     * References of IHalSensor instance for each sensor dev
     */
    IHalSensor *mpHalSensor[IDX_MAX_CAM_NUMBER];

    /**
     * Memory for preloaded data of each sensor dev
     */
    unsigned char *mpEepromPreload[IDX_MAX_CAM_NUMBER];

    /**
     * Initial class members
     */
    void init();

    /**
     * Power on sensor if needed
     *
     * @param sensorDev The sensor dev id to be powered
     * @param sensorId The sensor id to be powered
     * @return success or not
     */
    bool sensorPowerOn(unsigned int sensorDev, unsigned int sensorId);

    /**
     * Power off sensor if needed
     *
     * @param sensorDev The sensor dev id to be powered
     * @param sensorId The sensor id to be powered
     * @return success or not
     */
    bool sensorPowerOff(unsigned int sensorDev, unsigned int sensorId);

    /**
     * Convert sensor dev id to sensor index
     *
     * @param sensorDev The sensor dev id.
     */
    int getSensorIndexByDev(unsigned int sensorDev);

    /**
     * Read data from EEPROM
     *
     * @param sensorId Sensor id
     * @param deviceId Sensor dev id
     * @param offset Offset to read
     * @param length Length to read
     * @param data Buffer to return the result
     * @return read data length, it might be negative number if error
     */
    int readDataFromEeprom(unsigned int sensorId, unsigned int deviceId,
            unsigned int offset, unsigned int length,
            unsigned char *data);

};

};  //namespace NSCam

#endif
