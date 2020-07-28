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

#ifndef __GESTURE_H
#define __GESTURE_H

#include "sensors_mtk.h"
#include "activity_algorithm.h"
#include "init_learning.h"
#include "gesture_setting.h"

#define MAX_SMOOTH_NUM 10

#define SHAKE_SMOOTH_NUM 5
#define SHAKE_SMOOTH_THRESHOLD 70
#define PICKUP_SMOOTH_NUM 8
#define PICKUP_SMOOTH_THRESHOLD 70
#define SNAPSHOT_SMOOTH_NUM 5
#define SNAPSHOT_SMOOTH_THRESHOLD 80
#define FREEFALL_SMOOTH_NUM 1
#define FREEFALL_SMOOTH_THRESHOLD 80

// because FREEFALL doesn't exist in gesture_type_t, so we append the old enum
typedef enum {
    FREEFALL = GESTURE_MAX + 1,
    GESTURE_TYPE_NUM
} gesture_freefall_t;

/** @brief The required probability information for gesture algorihm, used in gesture algorithm notify. */
typedef struct gesture_notify_result {
    int32_t last_result;    /* probability result of the last state */
    int32_t current_result; /* probability result of the current state */
    int32_t threshold;      /* probability threshold of notify */
} gesture_notify_result_t;

/* exchange information with adaptor */
uint32_t get_gesture_time_stamp();

/* Gesture functions */
int gesutre_alg_enter_point();
void gesture_set_prox(uint32_t);
void gesture_set_timestamp(uint32_t);

void gesture_get_alg_result(UINT8*, int);
void gesture_setting_init();

#endif
