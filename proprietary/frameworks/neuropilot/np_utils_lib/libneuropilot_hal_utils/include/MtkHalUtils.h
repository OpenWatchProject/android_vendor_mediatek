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

#ifndef ANDROID_ML_NN_COMMON_HAL_UTILS_H
#define ANDROID_ML_NN_COMMON_HAL_UTILS_H

#include "HalInterfaces.h"
#include "MtkEncrypt.h"
#include <cutils/properties.h>

namespace android {
namespace nn {

#define NP_VLOG         \
    if (!isNeuroPilotVLogSupported()) \
        ;                 \
    else                  \
        LOG(INFO)

#define NP_ENCRYPT_VLOG(INPUT)         \
    LOG(INFO) << MTK_ENCRYPT_COUT(INPUT)

#define NP_ENCRYPT_PRINTF(INPUT, ...)         \
    printf("%s\n", MTK_ENCRYPT_PRINT(INPUT, ##__VA_ARGS__))

#define NP_ENCRYPT_FILE(INPUT)         \
    MTK_ENCRYPT_COUT(INPUT) << "\n"

inline bool isNeuroPilotVLogSupported() {
    bool ret = false;
    if (property_get_bool("debug.neuropilot.vlog", false)) {
        ret = true;
    }
    return ret;
}


/// M: OEM Operand @{
/**
 * Identify the type and value of the OEM Operand
 * type: The actual type of this operand
 * typeLen: The size(byte) of type.
 * data: The operand value.
 * dataLen: The size(byte) of data.
 */
struct OemOperandValue {
    uint8_t* type;
    uint8_t typeLen;
    uint8_t* data;
    uint32_t dataLen;
};
/// @}

/// M: NeuroPilot Extensions @{
enum {
    ANEUROPILOT_OPERATION_ELU = 0,
    ANEUROPILOT_OPERATION_MINPOOL = 1,
    ANEUROPILOT_OPERATION_REQUANT = 2,
    ANEUROPILOT_OPERATION_REVERSE = 3,
    ANEUROPILOT_OPERATION_MAX = ANEUROPILOT_OPERATION_REVERSE,

    ANEUROPILOT_OPERATION_INJECTION = 3000,
} NeuroPilotOperations;

const int ANEUROPILOT_OPERATION_INVAILD = -1;
const std::string ANEUROPILOT_EXTENSION_NAME = "com.mediatek.builtin";

/// @}


/// M: OEM Operand @{
/**
 * Encode the input type and operand value to raw data.
 * @param operand see OemOperandValue.
 * @param output the converted raw data.
 * @return ANEURALNETWORKS_NO_ERROR if the request completed normally.
 *
 * The format of data will be:
 *  -------------------------------------------------------------------------------
 *  | 1 byte typeLen  | N bytes type     | 4 bytes dataLen  | N bytes data        |
 *  -------------------------------------------------------------------------------
 *  1. The first byte of output is the size(length) of type.
 *      The following N bytes of the output is the string of input type.
 *      For example: int16 will be presented as {5,105,110,116,49,54}
 *
 *  2. The next 4 bytes(Little endian) of output is the size(length) of data.
 *      The following N bytes of the output is the actual data.
 *      For example: uint8_t data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0} will be
 *                         {10, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0}
 *
 *
 */
int encodeOperandValue(OemOperandValue *operand, uint8_t *output);

/**
 * Decode the input raw data to OemOperandValue.
 * @param input the raw data.
 * @param operand the output of this function, will decode raw data into struct.
 * @return ANEURALNETWORKS_NO_ERROR if the request completed normally.
 *
 * Note: This function will allocate memory for type and data, please make
 *          sure to free them when the not used.
 */
int decodeOperandValue(uint8_t *input, OemOperandValue *operand);
/// @}

/// M: NeuroPilot Extensions @{
/**
 * Get NeuroPilot extension type from name and type.
 * @param name the extension name(ex:com.mediatek.builtin).
 * @param opType operation type from model.
 * @return npType NeuroPilot defined extension type.
 *
 */
int getNeuroPilotExtensionType(
        std::vector<Model::ExtensionNameAndPrefix> extensionNameToPrefixVec, int opType);
/// @}

/// M: Log Model to Info @{
void logModelToInfoExt(V1_0::Model& rkModel);
void logModelToInfoExt(V1_1::Model& rkModel);
void logModelToInfoExt(V1_2::Model& rkModel);
/// M: Log Model to Info @}

}  // namespace nn
}  // namespace android

#endif  // ANDROID_ML_NN_COMMON_HAL_UTILS_H
