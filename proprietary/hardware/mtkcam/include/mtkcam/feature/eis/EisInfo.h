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

/**
* @file EisInfo.h
*
* EisInfo Header File
*/

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_FEATURE_EIS_EIS_INFO_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_FEATURE_EIS_EIS_INFO_H_
#include <math.h>

namespace NSCam{
namespace EIS{

enum VIDEO_CFG
{
    VIDEO_CFG_FHD,
    VIDEO_CFG_4K2K,
};

struct EisInfo
{
    EisInfo()
        : videoConfig(VIDEO_CFG_FHD)
        , queueSize(0)
        , startFrame(1)
        , factor(100)
        , mode(0){}

    EisInfo(const MUINT64 packedInfo)
    {
        videoConfig = getVideoConfig(packedInfo);
        queueSize   = getQueueSize(packedInfo);
        startFrame  = getStartFrame(packedInfo);
        factor      = getFactor(packedInfo);
        mode        = getMode(packedInfo);
    }

    /*
     * packedInfo layout:
     *   videoConfig : packedInfo % 10
     *   queueSize   : (packedInfo % 10^3) / 10
     *   startFrame  : (packedInfo % 10^5) / 10^3
     *   factor      : (packedInfo % 10^8) / 10^5
     *   mode        : (packedInfo % 10^12)/ 10^8
     *
     *   packedInfo composition : mmmmfffssqqv in decimal
     */

    static MINT32 getVideoConfig(const MUINT64 packedInfo)
    {
        return packedInfo % 10;
    }

    static MINT32 getQueueSize(const MUINT64 packedInfo)
    {
        return (packedInfo % (MUINT64)pow(10, 3)) / 10;
    }

    static MINT32 getStartFrame(const MUINT64 packedInfo)
    {
        return (packedInfo % (MUINT64)pow(10, 5)) / pow(10, 3);
    }

    static MINT32 getFactor(const MUINT64 packedInfo)
    {
        MINT32 factor = (packedInfo % (MUINT64)pow(10, 8)) / pow(10, 5);
        return ( factor > 100 ) ? factor : 100;
    }

    static MINT32 getMode(const MUINT64 packedInfo)
    {
        return (packedInfo % (MUINT64)pow(10, 12)) / pow(10, 8);
    }

    MUINT64 toPackedData()
    {
        MUINT64 packedInfo = 0;
        packedInfo += videoConfig;
        packedInfo += queueSize  * 10;
        packedInfo += startFrame * pow(10, 3);
        packedInfo += factor     * pow(10, 5);
        packedInfo += mode       * pow(10, 8);
        return packedInfo;
    }

    MINT32 videoConfig;
    MINT32 queueSize;
    MINT32 startFrame;
    MINT32 factor;
    MINT32 mode;
};

};//EIS
};//NSCAM
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_FEATURE_EIS_EIS_INFO_H_

