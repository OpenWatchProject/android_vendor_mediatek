/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#ifndef _CAMERA_CUSTOM_DSDN_H_
#define _CAMERA_CUSTOM_DSDN_H_

#include <stdint.h>

class DSDNCustom;
extern DSDNCustom* getDSDNCustomImpl() __attribute__((weak));

class DSDNCustom
{
public:
    enum DSDN_MODE
    {
        DSDN_MODE_OFF = 0,
        DSDN_MODE_20,
        DSDN_MODE_25,

    };

    enum DUAL_MODE
    {
        DUAL_MODE_SINGLE = 0,
        DUAL_MODE_HW_DEPTH,
        DUAL_MODE_OTHER,

    };
    enum AINR_MODE
    {
        AINR_MODE_OFF = 0,
        AINR_MODE_10,
    };
    class ScenarioParam
    {
    public:
        uint32_t videoW = 0;
        uint32_t videoH = 0;
        uint32_t fps = 0;
        uint32_t smvrFps = 0;
        DUAL_MODE dualMode = DUAL_MODE_SINGLE;
    };
    class Config
    {
    public:
        DSDN_MODE mode = DSDN_MODE_OFF;
        AINR_MODE ainrMode = AINR_MODE_OFF;
        // DSDN 2.0 E.g.  ratio = 1/3  Multiple = 1, Divider = 3
        uint32_t maxRatioMultiple = 1;
        uint32_t maxRatioDivider = 2;
    };
    class ParsedNVRam
    {
    public:
        uint32_t isoThreshold = 0;
        uint32_t ratioMultiple = 1;
        uint32_t ratioDivider = 1;
    };

    static DSDNCustom* getInstance()
    {
        static DSDNCustom sDefault;
        return (getDSDNCustomImpl != (DSDNCustom*(*)(void))0) ? getDSDNCustomImpl() : &sDefault;
    }

    DSDNCustom() {}
    virtual ~DSDNCustom() {}

    virtual bool isSupport()
    {
        return false;
    }

    virtual Config getConfig(const ScenarioParam &param)
    {
        (void)param;
        return Config();
    }

    virtual bool parseNVRam(const void *dsdnNVRam, ParsedNVRam &out)
    {
        (void)dsdnNVRam;
        (void)out;
        return false;
    }

};

#endif /* _CAMERA_CUSTOM_DSDN_H_ */

