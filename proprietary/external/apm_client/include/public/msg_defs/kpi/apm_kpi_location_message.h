/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2020. All rights reserved.
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

#ifndef __APM_KPI_LOCATION_MESSAGE_STATE_H__
#define __APM_KPI_LOCATION_MESSAGE_STATE_H__

typedef enum {
    APM_LOCATION_GPS_UNKNOWN = 0,
    APM_LOCATION_NETWORK_PROVIDER = 1,
    APM_LOCATION_GPS_PROVIDER = 2
} APM_LOCATION_PROVIDER_e;

typedef enum {
    HAS_ALTITUDE_MASK = 1,
    HAS_SPEED_MASK = 2,
    HAS_BEARING_MASK = 4,
    HAS_HORIZONTAL_ACCURACY_MASK = 8,
    HAS_MOCK_PROVIDER_MASK = 16,
    HAS_VERTICAL_ACCURACY_MASK = 32,
    HAS_SPEED_ACCURACY_MASK = 64,
    HAS_BEARING_ACCURACY_MASK = 128
} APM_LOCATION_MESSAGE_FILTER_e;

// V1
#pragma pack(push, 1)
    typedef struct {
        int32_t    version;
        int32_t    provider; // Reference to APM_LOCATION_PROVIDER_e
        int64_t    time;
        double     latitude;
        double     longitude;
        double     altitude;
        float      speed;
        float      bearing;
        float      horizontalAccuracyMeters;
        float      verticalAccuracyMeters;
        float      speedAccuracyMetersPerSecond;
        float      bearingAccuracyDegrees;
        int32_t    fieldsMask; // Reference to APM_LOCATION_MESSAGE_FILTER_e
    } locationMessage_v1_t;
#pragma pack(pop)

#endif /* __APM_KPI_LOCATION_MESSAGE_STATE_H__ */
