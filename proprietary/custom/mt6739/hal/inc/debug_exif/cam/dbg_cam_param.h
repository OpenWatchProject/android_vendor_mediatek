/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _DBG_CAM_PARAM_H
#define _DBG_CAM_PARAM_H

#include "../../../../../common/hal/inc/custom/debug_exif/dbg_exif_param.h"
#include <cstdint>
#include <map>

typedef struct
{
    uint32_t u4FieldID;
    uint32_t u4FieldValue;
} DEBUG_CAM_TAG_T;

/******************************************************************************
 * !! NOTICE !!
 * - this define is important for DP tool, please update it if necessary.
 * - nly necessary if header contains //BEGIN_OF_EXIF_TAG and //END_OF_EXIF_TAG.
 * - format:
 *   # define [FIXED_MODULE_NAME] [UPPERCASE_OF_THIS FILENAME]
 ******************************************************************************/
#define CHKSUM_DBG_COMM_PARAM CHKSUM_DBG_CAM_COMMON_PARAM1
#define CHKSUM_DBG_MF_PARAM CHKSUM_DBG_CAM_MF_PARAM10
#define CHKSUM_DBG_N3D_PARAM CHKSUM_DBG_CAM_N3D_PARAM4
#define CHKSUM_DBG_SENSOR_PARAM CHKSUM_DBG_CAM_SENSOR_PARAM0
#define CHKSUM_DBG_RESERVEA_PARAM CHKSUM_DBG_CAM_RESERVEA_PARAM3
#define CHKSUM_DBG_RESERVEB_PARAM CHKSUM_DBG_CAM_RESERVEB_PARAM0
//
using namespace dbg_cam_common_param_1;
using namespace dbg_cam_mf_param_10;
using namespace dbg_cam_n3d_param_4;
using namespace dbg_cam_sensor_param_0;
using namespace dbg_cam_reservea_param_3;
using namespace dbg_cam_reserveb_param_0;
using namespace dbg_cam_reservec_param_0;
//
#define DEBUF_CAM_TOT_MODULE_NUM    7 //should be modified
#define DEBUF_CAM_TAG_MODULE_NUM    6 //should be modified

#define MAXIMUM_CAM_DEBUG_COMM_SIZE 32

typedef union
{
    struct  {
        uint32_t chkSum;
        uint32_t ver;
    };
    uint8_t   Data[MAXIMUM_CAM_DEBUG_COMM_SIZE];
} CAM_DEBUG_COMM_T;
static_assert( sizeof(CAM_DEBUG_COMM_T) == MAXIMUM_CAM_DEBUG_COMM_SIZE,
        "CAM_DEBUG_COM_T size mismatch");
        
typedef struct
{
    uint32_t          u4Size;
    CAM_DEBUG_COMM_T  rCMN;
    CAM_DEBUG_COMM_T  rMF;
    CAM_DEBUG_COMM_T  rN3D;
    CAM_DEBUG_COMM_T  rSENSOR;
    CAM_DEBUG_COMM_T  rRESERVEA;
    CAM_DEBUG_COMM_T  rRESERVEB;
} COMMON_DEBUG_INFO_T;

typedef struct DEBUG_CAM_INFO_S
{
    struct Header
    {
        uint32_t  u4KeyID;
        uint32_t  u4ModuleCount;
        uint32_t  u4DbgCMNInfoOffset;
        uint32_t  u4DbgMFInfoOffset;
        uint32_t  u4DbgN3DInfoOffset;
        uint32_t  u4DbgSENSORInfoOffset;
        uint32_t  u4DbgRESERVEAInfoOffset;
        uint32_t  u4DbgRESERVEBInfoOffset;
        uint32_t  u4DbgRESERVECInfoOffset;
        COMMON_DEBUG_INFO_T  rCommDebugInfo;
    } hdr;

    DEBUG_CMN_INFO_T        rDbgCMNInfo;
    DEBUG_MF_INFO_T         rDbgMFInfo;
    DEBUG_N3D_INFO_T        rDbgN3DInfo;
    DEBUG_SENSOR_INFO_T     rDbgSENSORInfo;
    DEBUG_RESERVEA_INFO_T   rDbgRESERVEAInfo;
    DEBUG_RESERVEB_INFO_T   rDbgRESERVEBInfo;
    DEBUG_RESERVEC_INFO_T   rDbgRESERVECInfo;

} DEBUG_CAM_INFO_T;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace Custom {
static const DEBUG_CAM_INFO_T::Header
sDbgExifBufHeader = {
    .u4KeyID                = DEBUG_EXIF_KEYID_CAM,
    .u4ModuleCount          = DBGEXIF_MODULE_NUM(DEBUF_CAM_TOT_MODULE_NUM, DEBUF_CAM_TAG_MODULE_NUM),
    .u4DbgCMNInfoOffset     = offsetof(DEBUG_CAM_INFO_T, rDbgCMNInfo),
    .u4DbgMFInfoOffset      = offsetof(DEBUG_CAM_INFO_T, rDbgMFInfo),
    .u4DbgN3DInfoOffset     = offsetof(DEBUG_CAM_INFO_T, rDbgN3DInfo),
    .u4DbgSENSORInfoOffset  = offsetof(DEBUG_CAM_INFO_T, rDbgSENSORInfo),
    .u4DbgRESERVEAInfoOffset= offsetof(DEBUG_CAM_INFO_T, rDbgRESERVEAInfo),
    .u4DbgRESERVEBInfoOffset= offsetof(DEBUG_CAM_INFO_T, rDbgRESERVEBInfo),
    .u4DbgRESERVECInfoOffset= offsetof(DEBUG_CAM_INFO_T, rDbgRESERVECInfo),
    .rCommDebugInfo         = { .rCMN = { .chkSum = CHKSUM_DBG_COMM_PARAM, .ver = CMN_DEBUG_TAG_VERSION_DP }, \
                                .rMF = { .chkSum = CHKSUM_DBG_MF_PARAM, .ver = MF_DEBUG_TAG_VERSION_DP }, \
                                .rN3D = { .chkSum = CHKSUM_DBG_N3D_PARAM, .ver = N3D_DEBUG_TAG_VERSION_DP }, \
                                .rSENSOR = { .chkSum = CHKSUM_DBG_SENSOR_PARAM, .ver = SENSOR_DEBUG_TAG_VERSION_DP }, \
                                .rRESERVEA = { .chkSum = CHKSUM_DBG_RESERVEA_PARAM, .ver = RESERVEA_DEBUG_TAG_VERSION_DP }, \
                                .rRESERVEB = { .chkSum = CHKSUM_DBG_RESERVEB_PARAM, .ver = RESERVEB_DEBUG_TAG_VERSION_DP } \
                              }
};


static const debug_exif_buffer_info
sDbgExifBufInfo_cam = {
    .header_size    = sizeof(DEBUG_CAM_INFO_T::Header),
    .body_size      = sizeof(DEBUG_CAM_INFO_T) - sizeof(DEBUG_CAM_INFO_T::Header),

    .header_context = &sDbgExifBufHeader,

    .body_layout    = {
        #define SET_MODULE_INFO(module_id, module_version, module_field) \
            {module_id, \
            {module_id, \
             module_version, \
               sizeof(DEBUG_CAM_INFO_T::module_field), \
             offsetof(DEBUG_CAM_INFO_T, module_field)} \
            }

        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_CMN,          CMN_DEBUG_TAG_VERSION, rDbgCMNInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_MF,            MF_DEBUG_TAG_VERSION, rDbgMFInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_N3D,          N3D_DEBUG_TAG_VERSION, rDbgN3DInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_SENSOR,    SENSOR_DEBUG_TAG_VERSION, rDbgSENSORInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_RESERVE1,RESERVEA_DEBUG_TAG_VERSION, rDbgRESERVEAInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_RESERVE2,RESERVEB_DEBUG_TAG_VERSION, rDbgRESERVEBInfo),
        SET_MODULE_INFO(DEBUG_EXIF_MID_CAM_RESERVE3,RESERVEC_DEBUG_TAG_VERSION, rDbgRESERVECInfo),

        #undef  SET_MODULE_INFO
    },
};
};  //namespace Custom
};  //namespace NSCam


#endif  //_DBG_CAM_PARAM_H

