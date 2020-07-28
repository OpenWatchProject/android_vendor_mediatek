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


#include <camera_custom_dsdn.h>
#include <camera_custom_nvram.h>
#define MAX_RATIO_MULTIPLE 1
#define MAX_RATIO_DIVIDER 2

class DSDNCustomImpl : public DSDNCustom
{
public:

    DSDNCustomImpl() {}
    virtual ~DSDNCustomImpl() {}

    virtual bool isSupport();

    virtual DSDNCustom::Config getConfig(const ScenarioParam &param);
    virtual bool parseNVRam(const void *dsdnNVRam, DSDNCustom::ParsedNVRam &out);

private:
    DSDNCustom::DSDN_MODE getDsdnMode(const ScenarioParam &param);

};

/* Define for getHelper function used */
DSDNCustom* getDSDNCustomImpl()
{
    static DSDNCustomImpl sInst;
    return &sInst;
}

bool DSDNCustomImpl::isSupport()
{
    return true;
}

DSDNCustom::Config DSDNCustomImpl::getConfig(const ScenarioParam &param)
{
    Config config;
    config.maxRatioMultiple = MAX_RATIO_MULTIPLE;
    config.maxRatioDivider = MAX_RATIO_DIVIDER;
    config.mode = getDsdnMode(param);
    return config;
}

bool DSDNCustomImpl::parseNVRam(const void *dsdnNVRam, DSDNCustom::ParsedNVRam &out)
{
    if(dsdnNVRam)
    {
        FEATURE_NVRAM_DSDN_T *nvram = (FEATURE_NVRAM_DSDN_T*)dsdnNVRam;
        out.isoThreshold = nvram->downscale_thres;
        out.ratioMultiple = nvram->dsdn_dividend;
        out.ratioDivider = nvram->dsdn_divisor;
        return true;
    }
    return false;
}

DSDNCustom::DSDN_MODE DSDNCustomImpl::getDsdnMode(const ScenarioParam &param)
{
    if(param.videoW == 0 || param.videoH == 0) //photo preview
    {
        return DSDN_MODE_OFF;
    }

    if(param.smvrFps != 0) // smvr
    {
        return DSDN_MODE_OFF;
    }

    if(param.dualMode == DSDNCustom::DUAL_MODE_HW_DEPTH) // hw depth not support
    {
        return DSDN_MODE_OFF;
    }
    return DSDN_MODE_20;
}







