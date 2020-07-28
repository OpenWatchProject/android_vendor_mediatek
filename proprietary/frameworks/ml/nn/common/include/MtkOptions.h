/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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


#ifndef MEDIATEK_ML_NN_RUNTIME_OPTIONS_H
#define MEDIATEK_ML_NN_RUNTIME_OPTIONS_H

#include <string>

namespace android {
namespace nn {

typedef enum {
    ANEUROPILOT_FEATURE_ION = 0,                    // A (Not Used)
    ANEUROPILOT_FEATURE_ENHANCE_PERFORMANCE = 1,    // B
    ANEUROPILOT_FEATURE_CPU_SCHEDULING = 2,         // C (Phased Out)
    ANEUROPILOT_FEATURE_EARA_QOS = 3,               // D (Refactored feature)
    ANEUROPILOT_FEATURE_OEM_STRING_SCALAR = 4,      // E (Not Used)
    ANEUROPILOT_FEATURE_SPORTS_MODE = 5,            // F
    ANEUROPILOT_FEATURE_SYNC_EXEC = 6,              // G (Phased Out)
    ANEUROPILOT_FEATURE_NEUROPILOT_ALL = 25,        // Z
    ANEUROPILOT_FEATURE_MAX = ANEUROPILOT_FEATURE_NEUROPILOT_ALL
} NpOptionType;

#define NP_VLOG         \
    if (!isNeuroPilotVLogSupported()) \
        ;                 \
    else                  \
        LOG(INFO)

bool isFeatureSupported(int feature);
bool forceOverwriteOptions();

// Utils
bool isNeuroPilotVLogSupported();
bool isFallbackCpuSupported();
bool isCpuOnly();

// Profiler
bool isProfilerSupported();

// Debug
bool ignoreSortRunOrder();

// Partition Extension
bool getPartitionExtTypeFromProperty(char* type);

// Ion
bool isIonMemorySupported();

// Performance
bool isPreCheckOperandTypeSupported();

// Sports Mode
bool isSportsModeSupported();

// Eara Qos
bool isEaraQosSupported();

// NeuroPilot
bool isNeuroPilotSupported();

}  // namespace nn
}  // namespace android

#endif  //  MEDIATEK_ML_NN_RUNTIME_OPTIONS_H
