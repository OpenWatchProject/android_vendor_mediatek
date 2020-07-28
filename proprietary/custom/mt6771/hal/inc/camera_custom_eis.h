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
#ifndef _EIS_CONFIG_H_
#define _EIS_CONFIG_H_

#include "camera_custom_eis_base.h"


class EISCustom : public EISCustomBase
{
private:
    // DO NOT create instance
    EISCustom()
    {
    }

public:
    using EISCustomBase::USAGE_MASK;
    using EISCustomBase::VIDEO_CFG;

    // EIS state
    static MUINT32 getEISMode(MUINT32 mask);

    // EIS customized data
    static void getEISData(EIS_Customize_Para_t *a_pDataOut);
    static void getEISPlusData(EIS_PLUS_Customize_Para_t *a_pDataOut, MUINT32 config);
    static void getEIS25Data(EIS25_Customize_Tuning_Para_t *a_pDataOut);
    static void getEIS30Data(EIS30_Customize_Tuning_Para_t *a_pDataOut);

    // EIS version support
    static MBOOL isForcedEIS12();
    static MBOOL isSupportAdvEIS_HAL3();
    static MBOOL isEnabledEIS22();
    static MBOOL isEnabledEIS25();
    static MBOOL isEnabledEIS30();
    static MBOOL isEnabledFixedFPS();
    static MBOOL isEnabledForwardMode(MUINT32 cfg = 0);
    static MBOOL isEnabledGyroMode();
    static MBOOL isEnabledImageMode();
    static MBOOL isEnabledLosslessMode();
    static MBOOL isEnabledFOVWarpCombine(MUINT32 cfg = 0);
    static MBOOL isEnabledLMVData();

    // EIS configurations
    static double getEISRatio(MUINT32 cfg = 0, MUINT32 mask = 0);
    static MUINT32 getEIS12Factor();
    static MUINT32 getEISFactor(MUINT32 cfg = 0,  MUINT32 mask = 0);
    static MUINT32 get4K2KRecordFPS();
    static MUINT32 getForwardStartFrame();
    static MUINT32 getForwardFrames(MUINT32 cfg = 0);
    static void    getMVNumber(MINT32 width, MINT32 height, MINT32 *mvWidth, MINT32 *mvHeight);
    static MBOOL   getGISParameter(MUINT32 sensorId, MDOUBLE* const gisParameter, MUINT32* defWidth,
        MUINT32* defHeight, MUINT32* defCrop);
    static MUINT32 getGyroIntervalMs();
private:
    static void appendEISMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS22Mode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS25FusionMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS25ImageMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS25GyroMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS30FusionMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS30ImageMode(MUINT32 mask, MUINT32 &eisMode);
    static MBOOL generateEIS30GyroMode(MUINT32 mask, MUINT32 &eisMode);
};

#endif /* _EIS_CONFIG_H_ */

