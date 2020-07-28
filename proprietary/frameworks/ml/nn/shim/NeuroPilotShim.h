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

#ifndef ANDROID_ML_NN_RUNTIME_NEURO_PILOT_SHIM_H
#define ANDROID_ML_NN_RUNTIME_NEURO_PILOT_SHIM_H

#if __ANDROID_API__ >= __ANDROID_API_O_MR1__

#include <dlfcn.h>
#include <android/log.h>
#include <vector>

#include "NeuroPilotDef.h"

/*************************************************************************************************/
/// Partition Extension
typedef int (*ANeuroPilotCompilation_setPartitionExtType_fn)(
        ANeuralNetworksCompilation* compilation, uint32_t type);

// Profiler
typedef int (*ANeuroPilotExecution_getProfilerInfo_fn)(
        const ANeuralNetworksExecution *execution, std::vector<ProfilerResult> *result);

/*************************************************************************************************/
/// Partition Extension Start
/**
 * Set NeuroPoilt extended partition policy.
 * The setting will overwrite original preference setting {@link PreferenceCode}
 * Only works when partition is allowed:
 *     kPartitioningWithFallback or kPartitioningWithoutFallback is set.
 *
 * @param compilation The compilation to be modified.
 * @param type The partition policy{@link PartitionExtension}.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if the NeuroPilot is not supported.
 *         ANEURALNETWORKS_BAD_STATE if the compilation is already finished.
 *         ANEURALNETWORKS_BAD_DATA if type is invaild.
 */
inline int ANeuroPilotCompilationWrapper_setPartitionExtType(
        ANeuralNetworksCompilation* compilation, uint32_t type) {
    LOAD_NP_FUNCTION(ANeuroPilotCompilation_setPartitionExtType);
    EXECUTE_NP_FUNCTION_RETURN_INT(compilation, type);
}
/// Partition Extension End

// Profiler Start
/**
 * Get Profiler information.
 *
 * It's a debug function, use turn on feature before using it.
 * Command: adb setprop debug.nn.profiler.supported X
 *                 X = 1: on
 *                 X = 0: off
 *
 * @param execution The execution to be computed.
 * @param result The profiler result{@link ProfilerResult}.
 *                      Do nothing if no profiler data existed.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if the NeuroPilot is not supported.
 *         ANEURALNETWORKS_BAD_STATE if the profiler is not turned on.
 */
inline int ANeuroPilotExecutionWrapper_getProfilerInfo(
       const ANeuralNetworksExecution *execution, std::vector<ProfilerResult> *result) {
    LOAD_NP_FUNCTION(ANeuroPilotExecution_getProfilerInfo);
    EXECUTE_NP_FUNCTION_RETURN_INT(execution, result);
}
// Profiler End

#endif  //  __ANDROID_API__ >= 27
#endif  // ANDROID_ML_NN_RUNTIME_NEURO_PILOT_SHIM_H

