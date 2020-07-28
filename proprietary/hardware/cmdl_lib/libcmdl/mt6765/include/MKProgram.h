/*------------------------------------------------------------------------------
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 *----------------------------------------------------------------------------*/

#ifndef __MK_PROGRAM_H__
#define __MK_PROGRAM_H__

#include "MKType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create MKProgram instance.
 *
 * @returns MKProgram on success
 * @returns NULL if creation failed
 */
MKProgram MKProgram_Create(
    MKProgramType eType);   ///< [in] A enum value to MKProgramType type.

/**
 * Release MKProgram instance.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_Release(
    MKProgram program);   ///< [in] A pointer to structure MKProgram.

/**
 * Run MKProgram tree.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_Run(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    MKProgramType eType);   ///< [in] A enum value to MKProgramType type.

/**
 * Get command count in MKProgram tree.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_GetCommandCount(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    int* riCount);   ///< [out] command count.

/**
 * Save json file for MKProgram tree.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_DUMP_FILE if save file failed
 */
MKErrorType MKProgram_DumpJsonFile(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    const char* szFilepath);  ///< [in] File name path.

/**
 * Set input data in host side.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_SET_HOST_PTR_FAIL if inputTensors not found
 */
MKErrorType MKProgram_SetInputHostPtr(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    int id,   ///< [in] Input id in host side.
    void* ptr);   ///< [in] A pointer of input data in host side.

/**
 * Set output data in host side.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_SET_HOST_PTR_FAIL if inputTensors not found
 */
MKErrorType MKProgram_SetOutputHostPtr(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    int id,   ///< [in] Output id in host side.
    void* ptr);   ///< [in] A pointer of output data in host side.

/**
 * Get dummy command.
 *
 * @returns MKCommand on success
 * @returns nullptr if other error
 */
MKCommand MKProgram_GetDummyCommand(
    MKProgram mk_program);   ///< [in] A pointer to structure MKProgram.

/**
 * Check input tensors.
 *
 * @returns 1 if Yes
 * @returns 0 if No or other error
 */
int MKProgram_CheckInputTensors(
    MKProgram mk_program);   ///< [in] A pointer to structure MKProgram.

/**
 * Set boolean condition.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_SetBooleanCondition(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    MKBool mk_bool,   ///< [in] A pointer to structure MKBool.
    int iRunOn);   ///< [in] Run on

/**
 * Attach program.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_AttachProgram(
    MKProgram mk_parent,   ///< [in] A pointer to structure MKProgram for parent.
    MKProgram mk_child);   ///< [in] A pointer to structure MKProgram for child.

/**
 * Deattach program.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_DetachProgram(
    MKProgram mk_parent,   ///< [in] A pointer to structure MKProgram for parent.
    MKProgram mk_child);   ///< [in] A pointer to structure MKProgram for child.

/**
 * Deattach command.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_DetachCommand(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram for program.
    MKCommand mk_command);   ///< [in] A pointer to structure MKProgram for command.

/**
 * Attach memory copy.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_AttachMemoryCopy(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram for program.
    void* pDst,   ///< [in] A pointer to DST
    const void* pSrc,   ///< [in] A pointer to SRC
    size_t copyLength,   ///< [in] Length
    size_t dstStride[4],   ///< [in] Stride to DST
    size_t srcStride[4],   ///< [in] Stride to SRC
    size_t dimension[4]);   ///< [in] Dimension

/**
 * Attach update bool.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_OUT_OF_MEMORY if out-of-memory
 */
MKErrorType MKProgram_AttachUpdateBool(
    MKProgram mk_program,  ///< [in] A pointer to structure MKProgram.
    MKBool mk_bool,  ///< [in] A pointer to structure MKBool.
    int iValueToBeUpdated); ///< [in] Value to be updated.

/**
 * Attach dummy command.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_OUT_OF_MEMORY if out-of-memory
 */
MKErrorType MKProgram_AttachDummyCommand(
    MKProgram mk_program);  ///< [in] A pointer to structure MKProgram.

/**
 * Attach static data.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 * @returns MK_ERROR_OUT_OF_MEMORY if out-of-memory
 */
MKErrorType MKProgram_AttachStaticData(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    const void* ptr,   ///< [in] A pointer of data buffer.
    size_t size);   ///< [in] data buffer size

/**
 * Attach input tensor in host side.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_AttachInputTensor(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    int id,   ///< [in] Input id
    const void* ptr,   ///< [in] A pointer of input buffer.
    size_t size);   ///< [in] input buffer size

/**
 * Attach output tensor in host side.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if invalid parameter
 */
MKErrorType MKProgram_AttachOutputTensor(
    MKProgram mk_program,   ///< [in] A pointer to structure MKProgram.
    int id,   ///< [in] Output id
    const void* ptr,   ///< [in] A pointer of output buffer.
    size_t size);   ///< [in] output buffer size

#ifdef __cplusplus
}
#endif

#endif  /* __MK_PROGRAM_H__ */
