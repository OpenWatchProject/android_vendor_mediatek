/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 */

#ifndef _MTK_ISP_REPROCESS_METADATA_H_
#define _MTK_ISP_REPROCESS_METADATA_H_

#include <stdint.h>
#include <vector>

typedef struct mtkisp_metadata_entry {
    uint32_t tag;
    uint32_t type;
    uint32_t count;
    union {
        uint8_t u8[32];
        int32_t i32[8];
        float   f32[8];
        int64_t i64[4];
    } data;
    uint32_t padding; // align 8 bytes
} mtkisp_metadata_entry_t;

typedef struct mtkisp_metadata {
    uint32_t buffersize; // including sizeof(mtkisp_metadata)
    uint32_t datasize;
    uint32_t version;
    uint32_t entry_count;
    uint32_t entry_offset;
    uint32_t padding;
} mtkisp_metadata_t;

mtkisp_metadata_t *allocate_isp_metadata_bysize(uint32_t size);

mtkisp_metadata_t *allocate_isp_metadata_byentrynum(uint32_t totalentry);

int32_t initial_isp_metadata_vector(std::vector<uint8_t> &vec, uint32_t totalentry);

int32_t add_isp_metadata_entry(mtkisp_metadata_t *pmeta, uint32_t tag, uint32_t type,
                                         void *pdata, uint32_t count, uint32_t *datasize);

int32_t add_isp_metadata_entry(std::vector<uint8_t> &vec, uint32_t tag, uint32_t type,
                                         void *pdata, uint32_t count, uint32_t *datasize);

int32_t get_isp_metadata_entry(mtkisp_metadata_t *pmeta, uint32_t tag, mtkisp_metadata_entry_t **pentry);

int32_t get_isp_metadata_entry(std::vector<uint8_t> &vec, uint32_t tag, mtkisp_metadata_entry_t **pentry);

uint32_t get_isp_metadata_datasize(mtkisp_metadata_t *pmeta);

int32_t free_isp_metadata(mtkisp_metadata_t *pmeta);

#endif // _MTK_ISP_REPROCESS_METADATA_H_

