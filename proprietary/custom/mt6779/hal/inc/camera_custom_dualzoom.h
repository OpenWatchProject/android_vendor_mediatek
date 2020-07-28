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
#ifndef _DUALZOOM_CUSTOM_H_
#define _DUALZOOM_CUSTOM_H_

// define cam ID
#define DUALZOOM_WIDE_CAM_ID (0)
#define DUALZOOM_TELE_CAM_ID (2)
#define DUALZOOM_FOV_MAX_FPS (30)

// define dual zoom parameters
#define DUALZOOM_FOV_APPLIED_CAM (DUALZOOM_WIDE_CAM_ID)
#define DUALZOOM_FOV_MARGIN  (20) // 6% margin base on sensor size
#define DUALZOOM_FOV_MARGIN_COMBINE_EIS  (3) // 3% margin base on sensor size for combine EIS
#define DUALZOOM_FOV_MARGIN_PIXEL (384)
#define DUALZOOM_START_FOV_ZOOM_RATIO  (150) // 2.0x start do FOV
#define DUALZOOM_SWICH_CAM_ZOOM_RATIO  (200) // 2.0x switch camera
#define DUALZOOM_WAIT_STABLE_COUNT (120)      // wait count to set background camera go to lowpower
#define DUALZOOM_WAIT_LOW_POWER_COUNT (8)    // wait count to change state to lowpower state
#define DUALZOOM_WAIT_CAM_STANDBY_TO_ACT (8)   // wait count to change state(standby) to active state
#define DUALZOOM_WAIT_CAM_LOWFPS_TO_ACT  (0)   // wait count to change state(low fps) to active state
#define DUALZOOM_WIDE_STANDY_EN          (1)   // wide got to standby while equal to 1

// 3A policy tuning of dual zoom
#define DUALZOOM_AF_DAC_LOW_THRESHOLD (550)
#define DUALZOOM_AF_DAC_HIGH_THRESHOLD (600)
#define DUALZOOM_AE_LV_LOW_THRESHOLD (20)
#define DUALZOOM_AE_LV_HIGH_THRESHOLD (30)
#define DUALZOOM_AE_ISO_LOW_THRESHOLD (600)
#define DUALZOOM_AE_ISO_HIGH_THRESHOLD (1200)
#define DUALZOOM_AE_LV_DIFFERENCE (30)

// fov online calibration
#define DUALZOOM_FOV_ONLINE_ISO_MAX (2400)
#define DUALZOOM_FOV_ONLINE_EXPTIME_MAX (30000) // unit: us
#define DUALZOOM_FOV_ONLINE_DAC_WIDE_MAX (15)  // unit: 0.1%
#define DUALZOOM_FOV_ONLINE_DAC_TELE_MAX (20)  // unit: 0.1%
#define DUALZOOM_FOV_ONLINE_TEMP_MAX (100)     // unit: degree

#endif /* _DUALZOOM_CUSTOM_H_ */

