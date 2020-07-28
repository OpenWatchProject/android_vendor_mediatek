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
/*******************************************************************************
 *
 * Filename:
 * ---------
 * audio_custom_exp.h
 *
 * Project:
 * --------
 *   ALPS
 *
 * Description:
 * ------------
 * This file is the header of audio customization related function or definition.
 *
 * Author:
 * -------
 * ChiPeng
 *
***********************************************************************************/

#ifndef AUDIO_CUSTOM_EXP_H
#define AUDIO_CUSTOM_EXP_H

#include "Audio_Customization_Common.h"

/*********************************************************************************
*Common definitations  are  defined  in below  file 
*alps\mediatek\custom\common\hal\audioflinger\Audio_Customization_Common.h.
*if  some of  common definitations are not need, the specific customer  can mark the definitation in
* Audio_Customization_Common.h or  can undefine  the definitations in this file,just like:
*#undef ENABLE_AUDIO_COMPENSATION_FILTER
***********************************************************************************/

/*****************************************************************
** refine volume
*****************************************************************/
#ifdef DEVICE_MAX_VOLUME
#undef DEVICE_MAX_VOLUME
#endif
#define DEVICE_MAX_VOLUME       (8)
#ifdef DEVICE_VOICE_MAX_VOLUME
#undef DEVICE_VOICE_MAX_VOLUME
#endif
#define DEVICE_VOICE_MAX_VOLUME (8)
#ifdef DEVICE_AMP_MAX_VOLUME
#undef DEVICE_AMP_MAX_VOLUME
#endif
#define DEVICE_AMP_MAX_VOLUME   (15)
#ifdef DEVICE_MIN_VOLUME
#undef DEVICE_MIN_VOLUME
#endif
#define DEVICE_MIN_VOLUME       (-4)
#ifdef DEVICE_VOICE_MIN_VOLUME
#undef DEVICE_VOICE_MIN_VOLUME
#endif
#define DEVICE_VOICE_MIN_VOLUME (-4)
#ifdef DEVICE_AMP_MIN_VOLUME
#undef DEVICE_AMP_MIN_VOLUME
#endif
#define DEVICE_AMP_MIN_VOLUME   (6)
#ifdef DEVICE_VOLUME_RANGE
#undef DEVICE_VOLUME_RANGE
#endif
#define DEVICE_VOLUME_RANGE     (64)
#ifdef DEVICE_VOLUME_STEP
#undef DEVICE_VOLUME_STEP
#endif
#define DEVICE_VOLUME_STEP      (256)

/******************************************************************
** define Vibration SPK Default Center Freq and RMS
******************************************************************/
#ifdef VIBSPK_MV_RMS
#undef VIBSPK_MV_RMS
#endif
#define VIBSPK_MV_RMS           (350) //280~560, 70 per step
#ifdef VIBSPK_DEFAULT_FREQ
#undef VIBSPK_DEFAULT_FREQ
#endif
#define VIBSPK_DEFAULT_FREQ     (156) //141~330 Hz

/******************************************************************
** define using which flag
******************************************************************/
#ifdef USING_CLASSD_AMP
#undef USING_CLASSD_AMP
#endif
#define USING_CLASSD_AMP                // define using which flag
//#define USING_CLASSAB_AMP

/******************************************************************
** define chip delay for NXP
******************************************************************/
#ifdef CHIP_DELAY
#undef CHIP_DELAY
#endif
#define CHIP_DELAY				(22)

/***************************************************
* Define phonmic and headset mic mode.
typedef enum
{
    AUDIO_MIC_MODE_ACC = 1,
    AUDIO_MIC_MODE_DCC = 2,
    AUDIO_MIC_MODE_DMIC = 3,
    AUDIO_MIC_MODE_DMIC_LP = 4,
    AUDIO_MIC_MODE_DCCECMDIFF = 5,
    AUDIO_MIC_MODE_DCCECMSINGLE = 6,
} AUDIO_MIC_MODE;
*****************************************************/
#ifdef PHONE_MIC_MODE
#undef PHONE_MIC_MODE
#endif
#define PHONE_MIC_MODE (2)
#ifdef HEADSET_MIC_MODE
#undef HEADSET_MIC_MODE
#endif
#define HEADSET_MIC_MODE (6)
#ifdef MAIN_REF_MIC_INVERSE
#undef MAIN_REF_MIC_INVERSE
#endif
#define MAIN_REF_MIC_INVERSE 1
#endif

