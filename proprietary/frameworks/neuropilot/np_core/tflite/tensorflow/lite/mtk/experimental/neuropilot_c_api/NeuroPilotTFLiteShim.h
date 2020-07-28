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

#ifndef __MTK_NEUROPILOT_TFLITE_SHIM_H__
#define __MTK_NEUROPILOT_TFLITE_SHIM_H__

#include <dlfcn.h>
#include <android/log.h>
#include <vector>

#define TFLITE_LOG_D(format, ...) \
    __android_log_print(ANDROID_LOG_DEBUG, "NeuroPilotTFLiteShim", format "\n", ##__VA_ARGS__);

#define TFLITE_LOG_E(format, ...) \
    __android_log_print(ANDROID_LOG_ERROR, "NeuroPilotTFLiteShim", format "\n", ##__VA_ARGS__);

#define LOAD_TFLITE_FUNCTION(name) \
  static name##_fn fn = reinterpret_cast<name##_fn>(loadTFLiteFunction(#name));

#define EXECUTE_TFLITE_FUNCTION(...) \
  if (fn != nullptr) {        \
    fn(__VA_ARGS__);          \
  }

#define EXECUTE_TFLITE_FUNCTION_RETURN_INT(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : ANEURALNETWORKS_BAD_STATE;

#define EXECUTE_TFLITE_FUNCTION_RETURN_BOOL(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : false;

#define EXECUTE_TFLITE_FUNCTION_RETURN_POINTER(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : nullptr;

/************************************************************************************************/

typedef struct ANeuralNetworksTFLite ANeuralNetworksTFLite;
typedef struct ANeuralNetworksTFLiteOptions ANeuralNetworksTFLiteOptions;
typedef struct ANeuralNetworksTFLiteTensor ANeuralNetworksTFLiteTensor;
typedef struct TfLiteContext TfLiteContext;

typedef enum {
    TFLITE_BUFFER_TYPE_INPUT = 0,
    TFLITE_BUFFER_TYPE_OUTPUT = 1,
} NpTFLiteBufferType;

typedef uint32_t TFLiteBufferType;

typedef enum {
    TFLITE_TENSOR_TYPE_NONE = 0,
    TFLITE_TENSOR_TYPE_FLOAT = 1,
    TFLITE_TENSOR_TYPE_UINT8 = 2,
} NpTFLiteTensorType;

typedef uint32_t TFLiteTensorType;

typedef enum {
    NP_INFERENCE_TYPE_NONE = 0,
    NP_INFERENCE_TYPE_QNAUT = 1,
    NP_INFERENCE_TYPE_FLOAT = 2,
} NpInferenceType;

typedef uint32_t InferenceType;

typedef enum {
    // Use CPU to inference the model
    NP_ACCELERATION_CPU = 0,
    // Turns on Android NNAPI for hardware acceleration when it is available.
    NP_ACCELERATION_NNAPI = 1,
} NpAccelerationMode;

typedef uint32_t AccelerationMode;

/*************************************************************************************************/
typedef int (*ANeuroPilotTFLiteOptions_create_fn)
        (ANeuralNetworksTFLiteOptions** options);

typedef int (*ANeuroPilotTFLiteOptions_free_fn)
        (ANeuralNetworksTFLiteOptions* options);

typedef int (*ANeuroPilotTFLiteOptions_setExecParallel_fn)
        (ANeuralNetworksTFLiteOptions* options, bool enableParallel);

typedef int (*ANeuroPilotTFLiteOptions_setAllowFp16PrecisionForFp32_fn)
        (ANeuralNetworksTFLiteOptions* options, bool allow);

typedef int (*ANeuroPilotTFLiteOptions_resizeInputTensor_fn)
        (ANeuralNetworksTFLiteOptions* options,
         int32_t index, const int* dims, int32_t dimsSize);

typedef int (*ANeuroPilotTFLiteOptions_setAccelerationMode_fn)
        (ANeuralNetworksTFLiteOptions* options, AccelerationMode mode);

typedef int (*ANeuroPilotTFLiteOptions_setEncryptionLevel_fn)(
    ANeuralNetworksTFLiteOptions* options,
    int encryption_level);

typedef int (*ANeuroPilotTFLiteOptions_setCacheDir_fn)(
    ANeuralNetworksTFLiteOptions* options, const char* cache_dir);

typedef int (*ANeuroPilotTFLite_create_fn)
        (ANeuralNetworksTFLite** tflite, const char* modelPath);

typedef int (*ANeuroPilotTFLite_createAdv_fn)
        (ANeuralNetworksTFLite** tflite, const char* modelPath,
         ANeuralNetworksTFLiteOptions* options);

typedef int (*ANeuroPilotTFLite_createWithBuffer_fn)
        (ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize);

typedef int (*ANeuroPilotTFLite_createAdvWithBuffer_fn)
        (ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize,
         ANeuralNetworksTFLiteOptions* options);

typedef int (*ANeuroPilotTFLite_getTensorCount_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         int32_t * count);

typedef int (*ANeuroPilotTFLite_getTensorRank_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         int index, int* rank);

typedef int (*ANeuroPilotTFLite_getTensorDimensions_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         int index, int* dimensions);

typedef int (*ANeuroPilotTFLite_getTensorByteSize_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         int index, size_t* size);

typedef int (*ANeuroPilotTFLite_getTensorType_fn)
        (ANeuralNetworksTFLite* tflite, TFLiteBufferType btype,
         int index, TFLiteTensorType* ttype);

typedef int (*ANeuroPilotTFLite_setInputTensorData_fn)
        (ANeuralNetworksTFLite* tflite, int index,
         const void* data, size_t size);

typedef int (*ANeuroPilotTFLite_getOutputTensorData_fn)
        (ANeuralNetworksTFLite* tflite, int index,
         void* data, size_t size);

typedef int (*ANeuroPilotTFLite_getDequantizedOutputByIndex_fn)
        (ANeuralNetworksTFLite* tflite, void* buffer,
         size_t bufferByteSize, int tensorIndex);

typedef int (*ANeuroPilotTFLite_invoke_fn)(ANeuralNetworksTFLite* tflite);

typedef int (*ANeuroPilotTFLite_free_fn)(ANeuralNetworksTFLite* tflite);

typedef int (*ANeuroPilotTFLite_setExecParallel_fn)
        (ANeuralNetworksTFLite* tflite, bool enableParallel);

typedef int (*ANeuroPilotTFLite_setAllowFp16PrecisionForFp32_fn)
        (ANeuralNetworksTFLite* tflite, bool allow);

typedef int (*ANeuroPilot_getInferencePreference_fn)(void);

/*************************************************************************************************/
// For add-on
static void* sTFLiteHandle;
inline void* loadTFLiteLibrary(const char* name) {
    sTFLiteHandle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    if (sTFLiteHandle == nullptr) {
        TFLITE_LOG_E("TFLite error: unable to open library %s", name);
    } else {
        TFLITE_LOG_D("TFLite : open library %s", name);
    }
    return sTFLiteHandle;
}

inline void* getTFLiteLibraryHandle() {
    if (sTFLiteHandle == nullptr) {
        // Load library for platform level development
        sTFLiteHandle = loadTFLiteLibrary("libtflite_mtk.so");
    }
    if (sTFLiteHandle == nullptr) {
        // Load library for APK JNI level development
        sTFLiteHandle = loadTFLiteLibrary("libtflite_mtk_static.so");
    }
    return sTFLiteHandle;
}

inline void* loadTFLiteFunction(const char* name) {
    void* fn = nullptr;
    if (getTFLiteLibraryHandle() != nullptr) {
        fn = dlsym(getTFLiteLibraryHandle(), name);
    }

    if (fn == nullptr) {
        TFLITE_LOG_E("TFLite error: unable to open function %s", name);
    }

    return fn;
}

/**
 * Create an {@link ANeuralNetworksTFLiteOptions} with default options.
 *
 * <p>{@link ANeuroPilotTFLiteOptionWrapper_free} should be called once the object
 * is no longer needed.</p>
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} to be created.
 *               Set to NULL if unsuccessful.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuralNetworksTFLiteOptions_create(ANeuralNetworksTFLiteOptions** options) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_create);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options);
}

/**
 * Specifies whether {@link ANeuralNetworksTFLiteOptions} is allowed to be calculated
 * with range and/or precision as low as that of the IEEE 754 16-bit
 * floating-point format.
 * This function is only used with float model.
 * A float model is calculated with FP16 precision by default.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param allow True to allow FP16 precision if possible.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuralNetworksTFLiteOptions_setAllowFp16PrecisionForFp32(
        ANeuralNetworksTFLiteOptions* options, bool allow) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_setAllowFp16PrecisionForFp32);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options, allow);
}

/**
 * Change the dimensionality of a given input tensor.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param index The index of the input tensor.
 * @param dims List of the dimensions.
 * @param dimsSize Number of the dimensions.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuralNetworksTFLiteOptions_resizeInputTensor(ANeuralNetworksTFLiteOptions* options,
        int32_t index, const int* dims, int32_t dimsSize) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_resizeInputTensor);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options, index, dims, dimsSize);
}

/**
 * Set preferred acceleration mode.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param mode Refer to {@link NpAccelerationMode} enum definition.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuralNetworksTFLiteOptions_setAccelerationMode(
        ANeuralNetworksTFLiteOptions* options, AccelerationMode mode) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_setAccelerationMode);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options, mode);
}

/**
 * Set compilation cache directory.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param user define cache directory.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuroPilotTFLiteOptions_setCacheDir(
    ANeuralNetworksTFLiteOptions* options, const char* cache_dir) {

    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_setCacheDir);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options, cache_dir);
}

/**
 * Set encryption level.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param encryption level refer to {@link NpEncryptionLevel} enum definition.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
inline int ANeuroPilotTFLiteOptions_setEncryptionLevel(
    ANeuralNetworksTFLiteOptions* options,
    int encryption_level) {

    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_setEncryptionLevel);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options, encryption_level);
}

/**
 * Delete a {@link ANeuralNetworksTFLiteOptions} object.
 *
 * Destroys the object used by the run time to keep track of the memory.
 * This will free the underlying actual memory if no other code has open
 * handles to this memory.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} object to be freed.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuralNetworksTFLiteOptions_free(ANeuralNetworksTFLiteOptions* options) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLiteOptions_free);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(options);
}

/**
 * Create an {@link ANeuralNetworksTFLite} with the TFlite model stored in a file.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLiteWrapper_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLiteWrapper_free} should be called once the object
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *               Set to NULL if unsuccessful.
 * @param modelPath The full path of the tflite model file.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the model can't be parsed correctly.
 */
inline int ANeuroPilotTFLiteWrapper_makeTFLite(ANeuralNetworksTFLite** tflite,
                                               const char* modelPath) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_create);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, modelPath);
}

/**
 * Create an {@link ANeuralNetworksTFLite} with the TFlite model stored in a file.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLiteWrapper_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLiteWrapper_free} should be called once the object
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *               Set to NULL if unsuccessful.
 * @param modelPath The full path of the tflite model file.
 * @param option Option of the {@link ANeuralNetworksTFLite} object.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the model can't be parsed correctly.
 */
inline int ANeuroPilotTFLiteWrapper_makeAdvTFLite(ANeuralNetworksTFLite** tflite,
                                                  const char* modelPath,
                                                  ANeuralNetworksTFLiteOptions* options) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_createAdv);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, modelPath, options);
}

/**
 * Create an {@link ANeuralNetworksTFLite} with the TFLite model stored in a data buffer pointer.
 * The data buffer will be duplicated in ANeuralNetworksTFLite instance.
 * Caller could free the input data buffer after calling this API.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLiteWrapper_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLiteWrapper_free} should be called once the object
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *              Set to NULL if unsuccessful.
 * @param buffer The pointer to the tflite model buffer.
 * @param bufferSize The number of bytes of the tflite model buffer.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the model can't be parsed correctly.
 */
inline int ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(
        ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_createWithBuffer);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, buffer, bufferSize);
}

/**
 * Create an {@link ANeuralNetworksTFLite} with the TFLite model stored in a data buffer pointer.
 * The data buffer will be duplicated in ANeuralNetworksTFLite instance.
 * Caller could free the input data buffer after calling this API.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLiteWrapper_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLiteWrapper_free} should be called once the object
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *              Set to NULL if unsuccessful.
 * @param buffer The pointer to the tflite model buffer.
 * @param bufferSize The number of bytes of the tflite model buffer.
 * @param option Option of the {@link ANeuralNetworksTFLite} object.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the model can't be parsed correctly.
 */
inline int ANeuroPilotTFLiteWrapper_makeAdvTFLiteWithBuffer(
        ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize,
        ANeuralNetworksTFLiteOptions* options) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_createAdvWithBuffer);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, buffer, bufferSize, options);
}

/**
 * Store dequantized contents of the given output tensor to user-allocated buffer.
 * This function is only used with quantized model.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to get dequantized data from the output tensor.
 * @param buffer The pointer to the user-allocated buffer for storing dequantized contents.
 * @param bufferByteSize Specifies the buffer size in bytes.
 * @param tensorIndex Zero-based index of the output tensor.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
inline int ANeuroPilotTFLiteWrapper_getDequantizedOutputByIndex(ANeuralNetworksTFLite* tflite,
                                                                void* buffer,
                                                                size_t bufferByteSize,
                                                                int tensorIndex) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getDequantizedOutputByIndex);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, buffer, bufferByteSize, tensorIndex);
}

/**
 * Invoke inference. (run the whole graph in dependency order).
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to invoke inference.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the operation is failed.
 */
inline int ANeuroPilotTFLiteWrapper_invoke(ANeuralNetworksTFLite* tflite) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_invoke);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite);
}

/**
 * Delete a {@link ANeuralNetworksTFLite} object.
 *
 * Destroys the object used by the run time to keep track of the memory.
 * This will free the underlying actual memory if no other code has open
 * handles to this memory.
 *
 * @param memory The {@link ANeuralNetworksTFLite} object to be freed.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_free(ANeuralNetworksTFLite* tflite) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_free);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite);
}

/**
 * Get the number of input/output tensors associated with the model.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param btype Input or output tensor.
 * @param count the number of input/output tensors.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorCount(ANeuralNetworksTFLite* tflite,
                                                TFLiteBufferType btype,
                                                int32_t * count) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorCount);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, count);
}

/**
 * Get the dimensional information of the input/output tensor with the given index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param rank The rank of the tensor.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorRank(ANeuralNetworksTFLite* tflite,
                                                TFLiteBufferType btype,
                                                int index,
                                                int* rank) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorRank);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, index, rank);
}

/**
 * Get the dimensional information of the input/output tensor with the given index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param dimensions The dimension array to be filled. The size of the array
 *                   must be exactly as large as the rank.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorDimensions(ANeuralNetworksTFLite* tflite,
                                                        TFLiteBufferType btype,
                                                        int index,
                                                        int* dimensions) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorDimensions);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, index, dimensions);
}

/**
 * Get the size of the underlying data in bytes.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param size The tensor's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorByteSize(ANeuralNetworksTFLite* tflite,
                                                    TFLiteBufferType btype,
                                                    int index,
                                                    size_t* size) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorByteSize);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, index, size);
}

/**
 * Get the data type information of the input/output tensor with the given index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param ttpte The tensor's data type.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getTensorType(ANeuralNetworksTFLite* tflite,
                                                TFLiteBufferType btype,
                                                int index,
                                                TFLiteTensorType* ttype) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getTensorType);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, btype, index, ttype);
}

/**
 * Copies from the provided input buffer into the input tensor's buffer.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output tensor.
 * @param index Zero-based index of the input tensor.
 * @param data The input buffer.
 * @param size The input buffer's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_setInputTensorData(ANeuralNetworksTFLite* tflite,
                                                    int index,
                                                    const void* data,
                                                    size_t size) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_setInputTensorData);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, index, data, size);
}

/**
 * Copies to the provided output buffer from the output tensor's buffer.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the output tensor.
 * @param index Zero-based index of the output tensor.
 * @param data The output buffer.
 * @param size The output buffer's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
inline int ANeuroPilotTFLiteWrapper_getOutputTensorData(ANeuralNetworksTFLite* tflite,
                                                        int index,
                                                        void* data,
                                                        size_t size) {
    LOAD_TFLITE_FUNCTION(ANeuroPilotTFLite_getOutputTensorData);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT(tflite, index, data, size);
}

/**
 * Get inference preference of current platform.
 *
 * @return NP_INFERENCE_TYPE_NONE if NeuroPilot is not supported.
 *         NP_INFERENCE_TYPE_QNAUT if quantization inference is preferred.
 *         NP_INFERENCE_TYPE_FLOAT if float inference is preferred.
*/

inline int ANeuroPilotWrapper_getInferencePreference(void) {
    LOAD_TFLITE_FUNCTION(ANeuroPilot_getInferencePreference);
    EXECUTE_TFLITE_FUNCTION_RETURN_INT();
}

#endif  // __MTK_NEUROPILOT_TFLITE_SHIM_H__
