/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __MTK_NEUROPILOT_C_API_H__
#define __MTK_NEUROPILOT_C_API_H__

#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"

/************************************************************************************************/
// Must be synchronized with NeuroPilotTFLiteShim.h
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

__BEGIN_DECLS

/**
 * Create an {@link ANeuralNetworksTFLiteOptions} with the following default
 * setting.
 * - Compute with parallel execution by default.
 * - Compute float model with FP16 precision by default.
 *
 * <p>{@link ANeuroPilotTFLiteOptions_free} should be called once the instance
 * is no longer needed.</p>
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} to be created.
 *               Set to NULL if unsuccessful.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLiteOptions_create(ANeuralNetworksTFLiteOptions** options);

/**
 * Specifies whether {@link ANeuralNetworksTFLiteOptions} is allowed to be
 * calculated with range and/or precision as low as that of the IEEE 754 16-bit
 * floating-point format.
 * This function is only used with float model.
 * A float model is calculated with FP16 precision by default.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param allow True to allow FP16 precision if possible.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLiteOptions_setAllowFp16PrecisionForFp32(
    ANeuralNetworksTFLiteOptions* options, bool allow);

/**
 * Set preferred acceleration mode.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param mode Refer to {@link NpAccelerationMode} enum definition.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
int ANeuroPilotTFLiteOptions_setAccelerationMode(
    ANeuralNetworksTFLiteOptions* options, AccelerationMode mode);

/**
 * Set compilation cache directory.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param user define cache directory.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
int ANeuroPilotTFLiteOptions_setCacheDir(
    ANeuralNetworksTFLiteOptions* options, const char* cache_dir);

/**
 * Set encryption level.
 *
 * @param options The {@link ANeuralNetworksTFLiteOptions} instance.
 * @param encryption level refer to {@link NpEncryptionLevel} enum definition.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
*/
int ANeuroPilotTFLiteOptions_setEncryptionLevel(
    ANeuralNetworksTFLiteOptions* options,
    int encryption_level);

/**
 * Change the dimensionality of a given input tensor.
 *
 * @param options The instance.
 * @param inputIndex The index of the input tensor.
 * @param inputDims List of the dimensions.
 * @param inputDimsSize Number of the dimensions.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLiteOptions_resizeInputTensor(
    ANeuralNetworksTFLiteOptions* options, int32_t index, const int* dims,
    int32_t dimsSize);

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
void ANeuroPilotTFLiteOptions_free(ANeuralNetworksTFLiteOptions* options);

/**
 * Create an {@link ANeuralNetworksTFLite} with the TFlite model stored in a
 * file.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLite_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLite_free} should be called once the instance
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *               Set to NULL if unsuccessful.
 * @param modelPath The full path of the tflite model file.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_create(ANeuralNetworksTFLite** tflite,
                             const char* modelPath);
int ANeuroPilotTFLite_createAdv(ANeuralNetworksTFLite** tflite,
                                const char* modelPath,
                                ANeuralNetworksTFLiteOptions* options);
/**
 * Create an {@link ANeuralNetworksTFLite} with the TFLite model stored in a
 * data buffer pointer. The data buffer will be duplicated in
 * ANeuralNetworksTFLite instance. Caller could free the input data buffer after
 * calling this API.
 *
 * <p>This only creates the object. Computation is performed once
 * {@link ANeuroPilotTFLite_invoke} is invoked.
 *
 * <p>{@link ANeuroPilotTFLite_free} should be called once the instance
 * is no longer needed.</p>
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to be created.
 *              Set to NULL if unsuccessful.
 * @param buffer The pointer to the tflite model buffer.
 * @param bufferSize The number of bytes of the tflite model buffer.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_createWithBuffer(ANeuralNetworksTFLite** tflite,
                                       const char* buffer, size_t bufferSize);
int ANeuroPilotTFLite_createAdvWithBuffer(
    ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize,
    ANeuralNetworksTFLiteOptions* options);

/**
 * Get the number of input/output tensors associated with the model.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/out
 * tensor.
 * @param btype Input or output tensor.
 * @param count the number of input/output tensors.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getTensorCount(ANeuralNetworksTFLite* tflite,
                                     TFLiteBufferType btype, int32_t* count);

/**
 * Get the dimensional information of the input/output tensor with the given
 * index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/out
 * tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param rank The rank of the tensor.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getTensorRank(ANeuralNetworksTFLite* tflite,
                                    TFLiteBufferType btype, int index,
                                    int* rank);

/**
 * Get the dimensional information of the input/output tensor with the given
 * index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/out
 * tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param dimensions The dimension array to be filled. The size of the array
 *                   must be exactly as large as the rank.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getTensorDimensions(ANeuralNetworksTFLite* tflite,
                                          TFLiteBufferType btype, int index,
                                          int* dimensions);

/**
 * Get the size of the underlying data in bytes.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output
 * tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param size The tensor's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getTensorByteSize(ANeuralNetworksTFLite* tflite,
                                        TFLiteBufferType btype, int index,
                                        size_t* size);

/**
 * Get the data type information of the input/output tensor with the given
 * index.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/output
 * tensor.
 * @param btype Input or output tensor.
 * @param index Zero-based index of tensor.
 * @param ttpte The tensor's data type.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getTensorType(ANeuralNetworksTFLite* tflite,
                                    TFLiteBufferType btype, int index,
                                    TFLiteTensorType* ttype);

/**
 * Copies from the provided input buffer into the tensor's buffer.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the input/out
 * tensor.
 * @param index Zero-based index of the input tensor.
 * @param data The input buffer.
 * @param size The input buffer's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_setInputTensorData(ANeuralNetworksTFLite* tflite,
                                         int index, const void* data,
                                         size_t size);

/**
 * Copies to the provided output buffer from the output tensor's buffer.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} which holds the output
 * tensor.
 * @param index Zero-based index of the output tensor.
 * @param data The output buffer.
 * @param size The output buffer's size in bytes.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 */
int ANeuroPilotTFLite_getOutputTensorData(ANeuralNetworksTFLite* tflite,
                                          int index, void* data, size_t size);

/**
 * Store dequantized contents of the given output tensor to user-allocated
 * buffer. This function is only used with quantized model.
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to get dequantized data from
 * a given output tensor.
 * @param buffer The pointer to the user-allocated buffer for storing
 * dequantized contents.
 * @param bufferByteSize Specifies the buffer size in bytes.
 * @param tensorIndex Zero-based index of the output tensor.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_getDequantizedOutputByIndex(ANeuralNetworksTFLite* tflite,
                                                  void* buffer,
                                                  size_t bufferByteSize,
                                                  int tensorIndex);

/**
 * Invoke inference. (run the whole graph in dependency order).
 *
 * @param tflite The {@link ANeuralNetworksTFLite} to invoke inference.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 *         ANEURALNETWORKS_BAD_STATE if NeuroPilot is not supported.
 *         ANEURALNETWORKS_OP_FAILED if the operation is failed.
 */
int ANeuroPilotTFLite_invoke(ANeuralNetworksTFLite* tflite);

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
void ANeuroPilotTFLite_free(ANeuralNetworksTFLite* tflite);

int ANeuroPilot_getInferencePreference(void);

/**
 * Deprecated
 */
#define TFLITE_TENSOR_MAX_DIMENSTIONS 4

typedef struct {
  // The data type specification for data stored in `data`. This affects
  // what member of `data` union should be used.
  TFLiteTensorType type;
  // Tensor shapes
  int dimsSize;
  int dims[TFLITE_TENSOR_MAX_DIMENSTIONS];
  // Data pointer. The appropriate type should be used for a typed
  // tensor based on `type`.
  // The memory pointed by this data pointer is managed by ANeuralNetworksTFLite
  // instance. Caller should not try to free this pointer.
  void* buffer;

  // Correct the error naming from TFLiteTensor, this is actual buffer size in
  // byte.
  size_t bufferSize;
} TFLiteTensorExt;

/**
 * Deprecated
 * Get a tensor data structure. This function returns the first input or output
 * tensor.
 *
 * @param tflite The instance to get input/out tensor.
 * @param btype Input or output tensor.
 * @param tfliteTensor A pointer to store the tensor data structure.
 */
int ANeuroPilotTFLite_getTensor(ANeuralNetworksTFLite* tflite,
                                TFLiteBufferType btype,
                                TFLiteTensorExt* tfliteTensor);

/**
 * Deprecated
 * Get a tensor data structure. This function returns the input or output tensor
 * by the given index.
 *
 * @param tflite The instance to get input/out tensor.
 * @param btype Input or output tensor.
 * @param tfliteTensor A pointer to store the tensor data structure.
 * @param tensorIndex Zero-based index of tensor.
 */
int ANeuroPilotTFLite_getTensorByIndex(ANeuralNetworksTFLite* tflite,
                                       TFLiteBufferType btype,
                                       TFLiteTensorExt* tfliteTensor,
                                       int tensorIndex);

/**
 * Deprecated
 * Bind a {@link ANeuralNetworksTFLite} instance to the specified
 * device.(CPU/GPU/APU)
 *
 * @param tflite The instance.
 * @param device Device
 * ID.(ANEURALNETWORKS_CPU/ANEURALNETWORKS_GPU/ANEURALNETWORKS_APU)
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_bindToDeivce(ANeuralNetworksTFLite* tflite,
                                   uint32_t device);

/**
 * Deprecated
 * Set a {@link ANeuralNetworksTFLite} instance to use parallel execution or not
 *
 * @param tflite The instance.
 * @param enableParallel True to enable parallel execution.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_setExecParallel(ANeuralNetworksTFLite* tflite,
                                      bool enableParallel);

/**
 * Deprecated
 * Set a {@link ANeuralNetworksTFLite} instance to use parallel execution or not
 * Specifies whether {@link ANeuralNetworksTFLite} is allowed to be calculated
 * with range and/or precision as low as that of the IEEE 754 16-bit
 * floating-point format.
 * This function is only used with float model.
 * A float is calculated with FP16 by default.
 *
 * @param tflite The instance.
 * @param enableParallel True to enable parallel execution.
 *
 * @return ANEURALNETWORKS_NO_ERROR if successful.
 */
int ANeuroPilotTFLite_setAllowFp16PrecisionForFp32(
    ANeuralNetworksTFLite* tflite, bool allow);

__END_DECLS

#endif  // __MTK_NEUROPILOT_C_API_H__
