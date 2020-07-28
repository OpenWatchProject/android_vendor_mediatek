/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef ANDROID_ML_NN_NEURO_EARA_SHIM_H
#define ANDROID_ML_NN_NEURO_EARA_SHIM_H

#if __ANDROID_API__ >= __ANDROID_API_O_MR1__

#include <dlfcn.h>
#include <android/log.h>
#include <vector>

using std::vector;
using std::string;

typedef struct ANeuroEara ANeuroEara;

#define NN_EARA_LOG(format, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, "NeuroEara", format "\n", ##__VA_ARGS__);

#define LOAD_NN_EARA_FUNCTION(name) \
  static name##_fn fn = reinterpret_cast<name##_fn>(loadNnEaraFunction(#name));

#define EXECUTE_NN_EARA_CREATE_INSTANCE(...) \
    if (fn != nullptr) {\
        *ret = fn(__VA_ARGS__);\
        return true;\
    } else {\
        return false;\
    }

#define EXECUTE_NN_EARA_FUNCTION(...) \
  if (fn != nullptr) {        \
    fn(__VA_ARGS__);          \
  }

#define EXECUTE_NN_EARA_FUNCTION_RETURN_INT(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : -1;

#define EXECUTE_NN_EARA_FUNCTION_RETURN_BOOL(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : false;

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

static void* sHandle;
inline void* loadNnEaraLibrary(const char *name) {
    sHandle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    if (sHandle == nullptr) {
        NN_EARA_LOG("nneara error: unable to open library %s", name);
    }
    return sHandle;
}

inline void* getNnEaraLibraryHandle() {
    if (sHandle == nullptr) {
        sHandle = loadNnEaraLibrary("libneuroeara.so");
    }
    return sHandle;
}

inline void* loadNnEaraFunction(const char *name) {
    void* fn = nullptr;
    if (getNnEaraLibraryHandle() != nullptr) {
        fn = dlsym(getNnEaraLibraryHandle(), name);
    }

    if (fn == nullptr) {
        NN_EARA_LOG("nneara error: unable to open function %s", name);
    }

    return fn;
}

/*************************************************************************************************/
typedef int (*ANeuroEara_isEaraQosSupported_fn)(bool *isEaraSuport);

typedef int (*ANeuroEara_create_fn)(ANeuroEara **eara, vector<string> &plan);

typedef int (*ANeuroEara_setExecutionPreference_fn)(ANeuroEara *eara,
        ExecPreference preference);

typedef int (*ANeuroEara_getEaraUsage_fn)(ANeuroEara *eara, string comp,
        int *deviceUsage, int *bandwidth);

typedef int (*ANeuroEara_suggestOffload_fn)(ANeuroEara *eara, string comp, bool *execOffloadPlan);

typedef int (*ANeuroEara_configSuspendParams_fn)(ANeuroEara *eara, string comp,
        int usageThreshold, uint32_t sleepTimeInMs);

typedef int (*ANeuroEara_suggestSuspendTime_fn)(ANeuroEara *eara, string comp, uint32_t *time);

typedef int (*ANeuroEara_notifyExecutionBegin_fn)(ANeuroEara *eara, int tid,
        uint64_t modelId, vector<int32_t> &suggestedBoostValue);

typedef int (*ANeuroEara_notifyExecutionEnd_fn)(ANeuroEara *eara, int tid, uint64_t modelId,
        vector<bool> &hasFallback, vector<uint64_t> &execTime, vector<int32_t> &bandwidth);

typedef int (*ANeuroEara_notifyExecutionFail_fn)(ANeuroEara *eara, int tid,
        uint64_t modelId, uint32_t stepId);

typedef int (*ANeuroEara_getJobPriority_fn)(ANeuroEara *eara, int *jobPriority);

typedef void (*ANeuroEara_delete_fn)(ANeuroEara *eara);
/*************************************************************************************************/

inline int ANeuroEara_isEaraQosSupported(bool *isEaraSuport) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_isEaraQosSupported);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(isEaraSuport);
}

inline int ANeuroEara_create(ANeuroEara **eara, vector<string> &plan) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_create);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, plan);
}

inline int ANeuroEara_setExecutionPreference(ANeuroEara *eara,
        ExecPreference preference) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_setExecutionPreference);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, preference);
}

/*
 * NeuroEara queries device usage and system bandwidth from resource symphony
 * deviceUsage will  be set to -1 if device does not exist
 */
inline int ANeuroEara_getEaraUsage(ANeuroEara *eara, string comp,
        int *deviceUsage, int *bandwidth) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_getEaraUsage);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, comp, deviceUsage, bandwidth);
}

/*
 * NeuroEara will suggest caller to execute offload plan according to job priority and device usage
 * Default device bound threshold is 50
 */
inline int ANeuroEara_suggestOffload(ANeuroEara *eara, string comp, bool *execOffloadPlan) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_suggestOffload);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, comp, execOffloadPlan);
}

/*
 * This API is used to configure device bound threshold and the corresponding suspend time
 */
inline int ANeuroEara_configSuspendParams(ANeuroEara *eara, string comp,
        int usageThreshold, uint32_t sleepTimeInMs) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_configSuspendParams);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, comp, usageThreshold, sleepTimeInMs);
}

/*
 * NeuroEara will suggest a suspend time (in millisecond) for target device according to job
 * priority, device usage and system bandwidth.
 * Default device bound threshold is 50, suspend time is 200(millisecond)
 * Default bandwidth bound threshold is 60, suspend time is 50(millisecond)
 */
inline int ANeuroEara_suggestSuspendTime(ANeuroEara *eara, string comp, uint32_t *time) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_suggestSuspendTime);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, comp, time);
}

/*
 * NeuroEara will notify FPSGO that model execution is going to begin
 * suggestedBoostValue contains suggested boost value hint for each step
 */
inline int ANeuroEara_notifyExecutionBegin(ANeuroEara *eara, int tid,
        uint64_t modelId, vector<int32_t> &suggestedBoostValue) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_notifyExecutionBegin);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, tid, modelId, suggestedBoostValue);
}

/*
 * NeuroEara will notify FPSGO that model execution has complete
 * If a step has fallback to another device, the information should be contained in hasFallback
 */
inline int ANeuroEara_notifyExecutionEnd(ANeuroEara *eara, int tid, uint64_t modelId,
        vector<bool> &hasFallback, vector<uint64_t> &execTime, vector<int32_t> &bandwidth) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_notifyExecutionEnd);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, tid, modelId, hasFallback, execTime, bandwidth);
}

/*
 * NeuroEara will immediately notify FPSGO that model execution has failed
 */
inline int ANeuroEara_notifyExecutionFail(ANeuroEara *eara, int tid,
        uint64_t modelId, uint32_t stepId) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_notifyExecutionFail);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, tid, modelId, stepId);
}

/*
 * Return information about the job priority (LOW means the job should be a background inference)
 */
inline int ANeuroEara_getJobPriority(ANeuroEara *eara, int *jobPriority) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_getJobPriority);
    EXECUTE_NN_EARA_FUNCTION_RETURN_INT(eara, jobPriority);
}

inline void ANeuroEara_delete(ANeuroEara *eara) {
    LOAD_NN_EARA_FUNCTION(ANeuroEara_delete);
    EXECUTE_NN_EARA_FUNCTION(eara);
}

#endif  //  __ANDROID_API__ >= 27
#endif  // ANDROID_ML_NN_NEURO_EARA_SHIM_H

