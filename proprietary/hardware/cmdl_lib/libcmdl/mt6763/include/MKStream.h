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

#ifndef __MK_STREAM_H__
#define __MK_STREAM_H__

#include "MKType.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create MKStream instance.
 *
 * @returns MKStream on success
 * @returns NULL if creation failed
 */
MKStream MKStream_Create(void);

/**
 * Release MKStream instance.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 */
MKErrorType MKStream_Release(
    MKStream mk_stream);   ///< [in] A pointer to structure MKStream.

/**
 * Insert MKObject to MKStream tree.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 * @returns MK_ERROR_GENERAL_FAILED if other error
 */
MKErrorType MKStream_Insert(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    MKObject mk_object);  ///< [in] A pointer to structure MKObject.

/**
 * Load MKStream tree from a file.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 * @returns MK_ERROR_GENERAL_FAILED if other error
 */
MKErrorType MKStream_LoadByFile(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    const char* szFilename);  ///< [in] File name path.

/**
 * Save MKStream tree to a file.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 * @returns MK_ERROR_GENERAL_FAILED if other error
 */
MKErrorType MKStream_SaveToFile(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    const char* szFilename);  ///< [in] File name path.

/**
 * Load MKStream tree from a buffer.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 * @returns MK_ERROR_GENERAL_FAILED if other error
 */
MKErrorType MKStream_LoadByBuffer(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    const char* acBuffer,  ///< [in] A pointer to buffer.
    int iSize);  ///< [in] Buffer size.

/**
 * Save MKStream tree to a buffer.
 *
 * @returns MK_OK on success
 * @returns MK_ERROR_INVALID_PARAMS if Invalid parameter
 * @returns MK_ERROR_GENERAL_FAILED if other error
 */
MKErrorType MKStream_SaveToBuffer(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    char** pacBuffer,  ///< [out] A pointer to buffer.
    int* piSize);  ///< [out] A pointer to buffer size.

/**
 * Get MKObject count in MKStream tree.
 *
 * @returns object count on success
 */
int MKStream_GetObjectCount(
    MKStream mk_stream);   ///< [in] A pointer to structure MKStream.

/**
 * Get 'i'st MKObject in MKStream tree.
 *
 * @returns MKObject on success
 * @returns nullptr if Invalid parameter or other error
 */
MKObject MKStream_GetObjectOwnership(
    MKStream mk_stream,   ///< [in] A pointer to structure MKStream.
    int i);   ///< [in] MKObject order

#ifdef __cplusplus
}
#endif

#endif  /* __MK_STREAM_H__ */
