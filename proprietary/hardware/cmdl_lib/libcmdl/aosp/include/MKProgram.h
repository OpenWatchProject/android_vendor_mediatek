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
#include "MKDebug.h"

#ifdef __cplusplus
extern "C" {
#endif

MKProgram MKProgram_Create(
    MKProgramType eType);

MKErrorType MKProgram_Release(
    MKProgram program);

MKErrorType MKProgram_Run(
    MKProgram mk_program,
    MKProgramType eType);

MKErrorType MKProgram_GetCommandCount(
    MKProgram mk_program,
    int* riCount);

MKErrorType MKProgram_DumpJsonFile(
    MKProgram mk_program,
    const char* szFilepath);

MKErrorType MKProgram_SetInputHostPtr(
    MKProgram mk_program,
    int id,
    void* ptr);

MKErrorType MKProgram_SetOutputHostPtr(
    MKProgram mk_program,
    int id,
    void* ptr);

MKCommand MKProgram_GetDummyCommand(
    MKProgram mk_program);

int MKProgram_CheckInputTensors(
    MKProgram mk_program);

MKErrorType MKProgram_SetBooleanCondition(
    MKProgram mk_program,
    MKBool mk_bool,
    int iRunOn);

MKErrorType MKProgram_AttachProgram(
    MKProgram mk_parent,
    MKProgram mk_child);

MKErrorType MKProgram_DetachProgram(
    MKProgram mk_parent,
    MKProgram mk_child);

MKErrorType MKProgram_DetachCommand(
    MKProgram mk_program,
    MKCommand mk_command);

MKErrorType MKProgram_AttachMemoryCopy(
    MKProgram mk_program,
    void* pDst,
    const void* pSrc,
    size_t copyLength,
    size_t dstStrideY,
    size_t srcStrideY,
    size_t dstStrideZ,
    size_t srcStrideZ,
    size_t dstStrideW,
    size_t srcStrideW,
    size_t loopY,
    size_t loopZ,
    size_t loopW);

MKErrorType MKProgram_AttachUpdateBool(
    MKProgram mk_program,
    MKBool mk_bool,
    int iValueToBeUpdated);

MKErrorType MKProgram_AttachDummyCommand(
    MKProgram mk_program);

MKErrorType MKProgram_AttachStaticData(
    MKProgram mk_program,
    const void* ptr,
    size_t size);

MKErrorType MKProgram_AttachInputTensor(
    MKProgram mk_program,
    int id,
    const void* ptr,
    size_t size);

MKErrorType MKProgram_AttachOutputTensor(
    MKProgram mk_program,
    int id,
    const void* ptr,
    size_t size);

#ifdef __cplusplus
}
#endif

#endif  /* __MK_PROGRAM_H__ */
