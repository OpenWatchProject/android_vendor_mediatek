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

#ifndef ANDROID_ML_NN_NEURO_EARA_H
#define ANDROID_ML_NN_NEURO_EARA_H

#if __ANDROID_API__ >= __ANDROID_API_O_MR1__

#include <dlfcn.h>
#include <vector>

using std::vector;
using std::string;

typedef struct ANeuroEara ANeuroEara;

const uint64_t INVALID_EXEC_TIME = 0xFFFFFFFFFFFFFFFE;

/**
 * Result codes.
 */
typedef enum {
    ANEUROEARA_NO_ERROR = 0,
    ANEUROEARA_OUT_OF_MEMORY = 1,
    ANEUROEARA_INCOMPLETE = 2,
    ANEUROEARA_UNEXPECTED_NULL = 3,
    ANEUROEARA_BAD_DATA = 4,
    ANEUROEARA_OP_FAILED = 5,
    ANEUROEARA_UNMAPPABLE = 5,
    ANEUROEARA_BAD_STATE = 6,
    ANEUROEARA_NOT_SUPPORT = 7,
} NeuroEaraResultCode;

/**
 * Execution preferences.
 */
typedef enum {
  ANEUROEARA_PREFER_LOW_POWER = 0,
  ANEUROEARA_PREFER_FAST_SINGLE_ANSWER = 1,
  ANEUROEARA_PREFER_SUSTAINED_SPEED = 2,
} ExecPreference;

typedef enum {
    JOB_PRIORITY_HIGH = 0,
    JOB_PRIORITY_MED  = 10,
    JOB_PRIORITY_LOW  = 20,
} JobPriority;

/// M: Device Component @{
typedef enum {
    DEVICE_COMP_CPU     = 0,
    DEVICE_COMP_GPU     = 1,
    DEVICE_COMP_VPU     = 2,
    DEVICE_COMP_MDLA    = 3,
    DEVICE_COMP_NEON    = 4,
    DEVICE_COMP_UNKNOWN = 100,
} DeviceComponent;
/// M: Device Component @}

/// M: Device @{
typedef enum {
    DEVICE_TYPE_UNKNOWN = -1,
    DEVICE_TYPE_CPU     = 0,
    DEVICE_TYPE_GPU     = 1,
    DEVICE_TYPE_APU     = 2,
    DEVICE_TYPE_NEON    = 3,
} DeviceType;
/// M: Device @}

__BEGIN_DECLS
int ANeuroEara_isEaraQosSupported(bool *isEaraSupport);

int ANeuroEara_create(ANeuroEara **eara, vector<string> &plan);

int ANeuroEara_setExecutionPreference(ANeuroEara *eara,
        ExecPreference preference);

int ANeuroEara_getEaraUsage(ANeuroEara *eara, string comp,
        int *deviceUsage, int *bandwidth);

int ANeuroEara_suggestOffload(ANeuroEara *eara, string comp, bool *execOffloadPlan);

int ANeuroEara_configSuspendParams(ANeuroEara *eara, string comp,
        int usageThreshold, uint32_t sleepTimeInMs);

int ANeuroEara_suggestSuspendTime(ANeuroEara *eara, string comp, uint32_t *time);

int ANeuroEara_notifyExecutionBegin(ANeuroEara *eara, int tid,
        uint64_t modelId, vector<int32_t> &suggestedBoostValue);

int ANeuroEara_notifyExecutionEnd(ANeuroEara *eara, int tid, uint64_t modelId,
        vector<bool> &hasFallback, vector<uint64_t> &execTime, vector<int32_t> &bandwidth);

int ANeuroEara_notifyExecutionFail(ANeuroEara *eara, int tid,
        uint64_t modelId, uint32_t stepId);

int ANeuroEara_getJobPriority(ANeuroEara *eara, int *jobPriority);

void ANeuroEara_delete(ANeuroEara *eara);

__END_DECLS

#endif  //  __ANDROID_API__ >= 27

#endif  // ANDROID_ML_NN_NEURO_EARA_H
