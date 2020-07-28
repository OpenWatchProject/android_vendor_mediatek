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

#pragma once

/******************************************************************************
 *
 ******************************************************************************/
#include <cstdint>
#include <map>
//
#include "dbg_id_param.h"

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 *
 */
struct debug_exif_field
{
    uint32_t    u4FieldID;
    uint32_t    u4FieldValue;
};


/**
 *
 */
struct debug_exif_module_info
{
    uint32_t    module_id;  //module id.
    uint32_t    version;    //module verson.
    uint32_t    size;       //module size, in bytes.
    uint32_t    offset;     //module offset.
};


/*****************************************************************************
 * @brief the buffer info.
 *
 * @note
 *  header_size and body_size are the size of buffer header, in bytes, and the
 *  size of buffer body, in bytes, respectively.
 *  The buffer size = head_size + body_size.
 *
 ******************************************************************************/
struct debug_exif_buffer_info
{
    uint32_t                        header_size;
    uint32_t                        body_size;

    void const*                     header_context;

    std::map<
        uint32_t,   //module_id
        debug_exif_module_info
    >                               body_layout;

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 *  |   8  |       8      |   8  |     8      |
 *  | 0x00 | total_module | 0x00 | tag_module |
 */
#define DBGEXIF_MODULE_NUM(total_module, tag_module)    \
 ((uint32_t)((total_module & 0xff) << 16) |             \
  (uint32_t)(tag_module & 0xff))

/**
 *  |     8     |      1    |   7  |    16    |
 *  | module_id | line_keep | 0x00 |  tag_id  |
 */
#define DBGEXIF_TAG(module_id, tag, line_keep)          \
  ((uint32_t)((module_id & 0xff) << 24) |               \
   (uint32_t)((line_keep & 0x01) << 23) |               \
   (uint32_t)(tag & 0xffff))

#define N3DAAATAG   DBGEXIF_TAG         //deprecated; (used by libn3d3a)

