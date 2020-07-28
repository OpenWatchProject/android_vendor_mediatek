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

#ifndef __LIFT_H
#define __LIFT_H
#include <string.h>
#include <stdbool.h>

#include "glance_setting.h"

/***********************************************************************
   Mediatek Lift-to-wake algorithm, packaged by Tingen Tseng (2017/3/9)
   Lift-to-wake description:
       1) When mobile is on table/holding and dangling/in pocket, it has
          a raising-up movement, recognize if user has a intention to
          wake up the mobile or see the notification on mobile.
       2) Identify which screen the mobile is facing to user during mobile
          is glanceing. (Our customer can use M_orientation provided by MTK)
       3) Identify whether the mobile is heading up/down during mobile is
          glanceing. (Our customer can use facing gesture provided by MTK)
       4) Lift-to-wake gesture can support when user is sitting/standing/walking

   The trigger condition of glance:
       1) The raising-up angle exceeds 30 degree.
       2) The glanceing angular velocity can't be slight.

   The following scenario cannot trigger glance in our design:
       1) Held the mobile and naturally dangling down.
       2) shake the mobile up and down.
       3) shake the mobile horizontally.
       4) Flip the mobile along long side.
       5) Put the cell phone into the pocket.
 ***********************************************************************/

void ut_glance_read_config(customer_gesture_config_t *);

// const uint8_t *option: indicate the scenario when glance is supported. Comment out for limiting triggering condition.
// transfer an array to reinit function which contains constant options
//  = {face_up head_up, face_down head_up, face_up head_down, face_down head_down}
void glance_original_reinit(customer_gesture_config_t *, const uint8_t *, bool);
int32_t glance_alg_enter_point(uint32_t, int32_t, int32_t, int32_t);
int current_glance_count_in_alg();
void reset_glance_count();

typedef enum glance_trans {
    ANY_TO_NO,
    NO_TO_ANY,
    DISABLE,
} glance_trans_t;
void glance_set_transition(glance_trans_t);

#endif
