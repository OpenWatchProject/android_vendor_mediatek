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
#ifndef _CAMERA_CUSTOM_IVHDR_H_
#define _CAMERA_CUSTOM_IVHDR_H_

#include "camera_custom_types.h"  // For MUINT*/MINT*/MVOID/MBOOL type definitions.

#define CUST_IVHDR_DEBUG          0   // Enable this will dump HDR Debug Information into SDCARD
#define IVHDR_DEBUG_OUTPUT_FOLDER   "/storage/sdcard1/" // For ALPS.JB.
/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *        P U B L I C    F U N C T I O N    D E C L A R A T I O N         *
 **************************************************************************/

/*******************************************************************************
* IVHDR exposure setting
*******************************************************************************/
typedef struct IVHDRExpSettingInputParam_S
{
    MBOOL bIs60HZ;                        // Flicker 50Hz:0, 60Hz:1
    MUINT32 u4ShutterTime;           // unit: us
    MUINT32 u4SensorGain;            // 1x=1024
    MUINT32 u41xGainISO;             // ISO value for 1x gain
    MUINT32 u4SaturationGain;      // saturation gain for Sensor min gain.
} IVHDRExpSettingInputParam_T;

typedef struct IVHDRExpSettingOutputParam_S
{
    MBOOL bEnableWorkaround;    // MTRUE : enable, MFALSE : disable
    MUINT32 u4SEExpTimeInUS;     // unit: us short exposure
    MUINT32 u4SESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4SEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LEExpTimeInUS;     // unit: us long exposure
    MUINT32 u4LESensorGain;        // 1x=1204 sensor gain
    MUINT32 u4LEISPGain;              // 1x=1204 isp gain
    MUINT32 u4LE_SERatio_x100;   // 100x
} IVHDRExpSettingOutputParam_T;

MVOID getIVHDRExpSetting(const IVHDRExpSettingInputParam_T& rInput, IVHDRExpSettingOutputParam_T& rOutput);

/**************************************************************************
 *                   C L A S S    D E C L A R A T I O N                   *
 **************************************************************************/

#endif // _CAMERA_CUSTOM_IVHDR_H_

