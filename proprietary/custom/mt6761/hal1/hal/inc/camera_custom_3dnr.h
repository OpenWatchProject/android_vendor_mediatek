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
#ifndef _3DNR_CONFIG_H
#define _3DNR_CONFIG_H

// E.g. 600 means that THRESHOLD_LOW is ISO600.
#define ISO_ENABLE_THRESHOLD_LOW        600      

// E.g. 800 means that THRESHOLD_HIGH is ISO600.
#define ISO_ENABLE_THRESHOLD_HIGH       800      

#if 0   // Obsolete
// E.g. 60 means that use 60% of Max Current ISO as THRESHOLD_LOW.
#define ISO_ENABLE_THRESHOLD_LOW_PERCENTAGE        60      

// E.g. 80 means that use 80% of Max Current ISO as THRESHOLD_HIGH.
#define ISO_ENABLE_THRESHOLD_HIGH_PERCENTAGE       80      
#endif  // Obsolete

// E.g. 130 means thatrRaise max ISO limitation to 130% when 3DNR on.
// When set to 100, 3DNR is noise improvement priority. 
// When set to higher than 100, 3DNR is frame rate improvement priority.
#define MAX_ISO_INCREASE_PERCENTAGE     100     

// How many frames should 3DNR HW be turned off (for power saving) if it
// stays at inactive state. (Note: inactive state means ISO is lower than
// ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_OFF_THRESHOLD          60

// How many frames should 3DNR HW be turned on again if it returns from 
// inactive state and stays at active state. (Note: active state means
// ISO is higher than ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_REOPEN_DELAY           4

int get_3dnr_iso_enable_threshold_low(void);
int get_3dnr_iso_enable_threshold_high(void);
#if 0   // Obsolete
int get_3dnr_iso_enable_threshold_low_percentage(void);
int get_3dnr_iso_enable_threshold_high_percentage(void);
#endif  // Obsolete
int get_3dnr_max_iso_increase_percentage(void);
int get_3dnr_hw_power_off_threshold(void);
int get_3dnr_hw_power_reopen_delay(void);

#endif /* _3DNR_CONFIG_H */


