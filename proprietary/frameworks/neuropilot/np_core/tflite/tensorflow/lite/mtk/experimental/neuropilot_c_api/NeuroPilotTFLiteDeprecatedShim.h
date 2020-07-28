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

#ifndef __MTK_NEUROPILOT_TFLITE_DEPRECATED_SHIM_H__
#define __MTK_NEUROPILOT_TFLITE_DEPRECATED_SHIM_H__

#include "tensorflow/lite/mtk/experimental/neuropilot_c_api/NeuroPilotTFLiteShim.h"

#define TFLITE_TENSOR_MAX_DIMENSTIONS    4

typedef struct {
    // The data type specification for data stored in `data`. This affects
    // what member of `data` union should be used.
    TFLiteTensorType type;
    // Tensor shapes
    int dimsSize;
    int dims[TFLITE_TENSOR_MAX_DIMENSTIONS];
    // Data pointer. The appropriate type should be used for a typed
    // tensor based on `type`.
    // The memory pointed by this data pointer is managed by ANeuralNetworksTFLite instance.
    // Caller should not try to free this pointer.
    void* buffer;

    // Correct the error naming from TFLiteTensor, this is actual buffer size in byte.
    size_t bufferSize;
} TFLiteTensorExt;

typedef int (*ANeuroPilotTFLite_getTensor_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         TFLiteTensorExt *tfliteTensor);

typedef int (*ANeuroPilotTFLite_getTensorByIndex_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         TFLiteTensorExt *tfliteTensor, int tensorIndex);

typedef int (*ANeuroPilotTFLite_bindToDeivce_fn)
        (ANeuralNetworksTFLite* tflite, uint32_t device);

typedef int (*ANeuroPilotTFLite_setExecParallel_fn)
        (ANeuralNetworksTFLite* tflite, bool enableParallel);

typedef int (*ANeuroPilotTFLite_setAllowFp16PrecisionForFp32_fn)
        (ANeuralNetworksTFLite* tflite, bool allow);

/**
 * Deprecated
 * Get a tensor data structure.
 * This function returns the input or output tensor by index 0.
 *
 * @param tflite The instance to get input/out tensor.
 * @param btype Input or output tensor.
 * @param tfliteTensor A pointer to store the tensor data structure.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensor(ANeuralNetworksTFLite* tflite,
                                       TFLiteBufferType btype, TFLiteTensorExt *tfliteTensor) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensor);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, tfliteTensor);
}

/**
 * Deprecated
 * Get a tensor data structure.
 * This function returns the input or output tensor by the given index.
 *
 * @param tflite The instance to get input/out tensor.
 * @param btype Input or output tensor.
 * @param tfliteTensor A pointer to store the tensor data structure.
 * @param tensorIndex Zero-based index of tensor.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorByIndex(ANeuralNetworksTFLite* tflite,
                                       TFLiteBufferType btype, TFLiteTensorExt *tfliteTensor,
                                       int tensorIndex) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorByIndex);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, tfliteTensor, tensorIndex);
}

/**
 * Deprecated
 * Bind a {@link ANeuralNetworksTFLite} instance to the specified device.(CPU/GPU/APU)
 *
 * @param tflite The instance.
 * @param device Device ID.(ANEUROPILOT_CPU/ANEUROPILOT_GPU/ANEUROPILOT_APU)
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
*/
inline int ANeuroPilotTFLiteWrapper_bindToDeivce(
        ANeuralNetworksTFLite* tflite, uint32_t device) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_bindToDeivce);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, device);
}

/**
 * Deprecated
 * Set a {@link ANeuralNetworksTFLite} instance to use parallel execution when possible.
 * The parallel execution depends on the platform capability.
 *
 * @param tflite The instance.
 * @param enableParallel True to enable parallel execution.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuroPilotTFLiteWrapper_setExecParallel(ANeuralNetworksTFLite* tflite,
        bool enableParallel) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_setExecParallel);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, enableParallel);
}

/**
 * Deprecated
 * Specifies whether {@link ANeuralNetworksTFLite} is allowed to be calculated
 * with range and/or precision as low as that of the IEEE 754 16-bit
 * floating-point format.
 * This function is only used with float model.
 * A float mode is calculated with FP16 precision by default.
 *
 * @param tflite The instance.
 * @param enableParallel True to enable parallel execution.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuroPilotTFLiteWrapper_setAllowFp16PrecisionForFp32(ANeuralNetworksTFLite* tflite,
        bool allow) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_setAllowFp16PrecisionForFp32);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, allow);
}

#endif  // __MTK_NEUROPILOT_TFLITE_DEPRECATED_SHIM_H__
