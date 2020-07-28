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

#ifndef NN_MODEL_ENCRYPT_H
#define NN_MODEL_ENCRYPT_H

#if __ANDROID_API__ >= __ANDROID_API_Q__

#include <dlfcn.h>
#include <android/log.h>
#include "tensorflow/lite/nnapi/NeuralNetworksTypes.h"

typedef struct ANnModelEncryptor ANnModelEncryptor;

typedef enum {
    ENCRYPTION_LEVEL_FASTEST = 32,
    ENCRYPTION_LEVEL_GOOD    = 64,
    ENCRYPTION_LEVEL_SAFEST = (1 << 27),
} EncryptionLevel;

#define NN_MD_ENCRYPT_ERR(format, ...) \
    __android_log_print(ANDROID_LOG_ERROR, "NN_MDER", format "\n", ##__VA_ARGS__);

#define LOAD_NN_MD_ENCRYPT_FUNCTION(name) \
  static name##_fn fn = reinterpret_cast<name##_fn>(loadNnModelEncryptionFunction(#name));

#define EXECUTE_NN_MD_ENCRYPT_FUNCTION(...) \
  if (fn != nullptr) {        \
    fn(__VA_ARGS__);          \
  }

#define EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : -1;

#define EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_BOOL(...) \
    return fn != nullptr ? fn(__VA_ARGS__) : false;

static void *sHandle;
inline void *loadNnModelEncryptionLibrary(const char *name) {
    sHandle = dlopen(name, RTLD_LAZY | RTLD_LOCAL);
    if (sHandle == nullptr) {
        NN_MD_ENCRYPT_ERR("error: unable to open library %s", name);
    }
    return sHandle;
}

inline void *getNnModelEncryptionLibraryHandle() {
    if (sHandle == nullptr) {
        sHandle = loadNnModelEncryptionLibrary("libnnmodelencryption.so");
    }
    return sHandle;
}

inline void *loadNnModelEncryptionFunction(const char *name) {
    void *fn = nullptr;
    if (getNnModelEncryptionLibraryHandle() != nullptr) {
        fn = dlsym(getNnModelEncryptionLibraryHandle(), name);
    }

    if (fn == nullptr) {
        NN_MD_ENCRYPT_ERR("error: unable to open function %s", name);
    }

    return fn;
}

/*************************************************************************************************/
typedef int (*ANnModelEncryption_create_fn)(
        ANnModelEncryptor **encryptor, void *model,
        EncryptionLevel encryptLevel, uint8_t encryptionKey[16]);

typedef int (*ANnModelEncryption_getExtensionOperationType_fn)(ANnModelEncryptor *encryptor,
        const char *extensionName, uint16_t operationCodeWithinExtension,
        ANeuralNetworksOperationType *type);

typedef int (*ANnModelEncryption_addOperation_fn)(ANnModelEncryptor *encryptor,
        ANeuralNetworksOperationType type, uint32_t inputCount,
        const uint32_t *inputs, uint32_t outputCount, const uint32_t *outputs);

typedef int (*ANnModelEncryption_getExtensionOperandType_fn)(ANnModelEncryptor *encryptor,
        const char *extensionName, uint16_t operandCodeWithinExtension, int32_t *type);

typedef int (*ANnModelEncryption_setOperandExtensionData_fn)(
        ANnModelEncryptor *encryptor, uint32_t index, const void *data, size_t length);

typedef int (*ANnModelEncryption_setOperandSymmPerChannelQuantParams_fn)(
        ANnModelEncryptor *encryptor, uint32_t index,
        const ANeuralNetworksSymmPerChannelQuantParams *channelQuant);

typedef int (*ANnModelEncryption_addOperand_fn)(ANnModelEncryptor *encryptor,
        const ANeuralNetworksOperandType *type);

typedef int (*ANnModelEncryption_setOperandValue_fn)(ANnModelEncryptor *encryptor,
        int32_t index, const void *buffer, size_t length);

typedef int (*ANnModelEncryption_setOperandValueFromFd_fn)(ANnModelEncryptor *encryptor,
        int32_t index, const int fd, size_t offset, size_t length);

typedef int (*ANnModelEncryption_identifyInputsAndOutputs_fn)(ANnModelEncryptor *encryptor,
        uint32_t inputCount, const uint32_t *inputs, uint32_t outputCount, const uint32_t *outputs);

typedef int (*ANnModelEncryption_relaxComputationFloat32toFloat16_fn)(
        ANnModelEncryptor *encryptor, bool allow);

typedef int (*ANnModelEncryption_finish_fn)(ANnModelEncryptor *encryptor, bool needCargo);

typedef void (*ANnModelEncryption_delete_fn)(ANnModelEncryptor *encryptor);
/*************************************************************************************************/

inline int ANnModelEncryption_create(
        ANnModelEncryptor **encryptor, void *model,
        EncryptionLevel encryptLevel, uint8_t encryptionKey[16]) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_create);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, model, encryptLevel, encryptionKey);
}

inline int ANnModelEncryption_getExtensionOperationType(
        ANnModelEncryptor *encryptor, const char *extensionName,
        uint16_t operationCodeWithinExtension, ANeuralNetworksOperationType *type) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_getExtensionOperationType);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(
            encryptor, extensionName, operationCodeWithinExtension, type);
}

inline int ANnModelEncryption_addOperation(ANnModelEncryptor *encryptor,
        ANeuralNetworksOperationType type, uint32_t inputCount,
        const uint32_t *inputs, uint32_t outputCount, const uint32_t *outputs) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_addOperation);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(
            encryptor, type, inputCount, inputs, outputCount, outputs);
}

inline int ANnModelEncryption_getExtensionOperandType(ANnModelEncryptor *encryptor,
        const char *extensionName, uint16_t operandCodeWithinExtension, int32_t *type) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_getExtensionOperandType);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(
            encryptor, extensionName, operandCodeWithinExtension, type);
}

inline int ANnModelEncryption_setOperandExtensionData(ANnModelEncryptor *encryptor,
        uint32_t index, const void *data, size_t length) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_setOperandExtensionData);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, index, data, length);
}

inline int ANnModelEncryption_setOperandSymmPerChannelQuantParams(
        ANnModelEncryptor *encryptor, uint32_t index,
        const ANeuralNetworksSymmPerChannelQuantParams *channelQuant) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_setOperandSymmPerChannelQuantParams);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, index, channelQuant);
}

inline int ANnModelEncryption_addOperand(
        ANnModelEncryptor *encryptor, const ANeuralNetworksOperandType *type) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_addOperand);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, type);
}

inline int ANnModelEncryption_setOperandValue(
        ANnModelEncryptor *encryptor, int32_t index, const void *buffer, size_t length) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_setOperandValue);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, index, buffer, length);
}

inline int ANnModelEncryption_setOperandValueFromFd(
        ANnModelEncryptor *encryptor, int32_t index, const int fd, size_t offset, size_t length) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_setOperandValueFromFd);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, index, fd, offset, length);
}


inline int ANnModelEncryption_identifyInputsAndOutputs(ANnModelEncryptor *encryptor,
        uint32_t inputCount, const uint32_t *inputs, uint32_t outputCount, const uint32_t *outputs) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_identifyInputsAndOutputs);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, inputCount, inputs, outputCount, outputs);
}

inline int ANnModelEncryption_relaxComputationFloat32toFloat16(
        ANnModelEncryptor *encryptor, bool allow) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_relaxComputationFloat32toFloat16);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, allow);
}

inline int ANnModelEncryption_finish(ANnModelEncryptor *encryptor, bool needCargo) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_finish);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION_RETURN_INT(encryptor, needCargo);
}

inline void ANnModelEncryption_delete(ANnModelEncryptor *encryptor) {
    LOAD_NN_MD_ENCRYPT_FUNCTION(ANnModelEncryption_delete);
    EXECUTE_NN_MD_ENCRYPT_FUNCTION(encryptor);
}

#endif  //  __ANDROID_API__ >= 29
#endif  // NN_MODEL_ENCRYPT_H
