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

#include "include/mtkisp_metadata.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <cassert>

#include <log/log.h>
//#include <vendor/mediatek/hardware/camera/isphal/1.0/types.h>

#define OK              0
#define ERROR           (-1)
#define NOT_FOUND       (-2)
#define OUT_OF_MEM      (-3)

//using namespace vendor::mediatek::hardware::camera::isphal::V1_0;

void initial_isp_metadata_header(mtkisp_metadata_t *buffer, size_t buffer_size)
{
    buffer->buffersize = buffer_size;
    buffer->datasize = sizeof(mtkisp_metadata_t);
    buffer->version = 0;
    buffer->entry_count = 0;
    buffer->entry_offset = sizeof(mtkisp_metadata_t);
}

mtkisp_metadata_t *allocate_isp_metadata_bysize(uint32_t size)
{
    if (size == 0)
    {
        return nullptr;
    }
    size_t buffer_size = sizeof(mtkisp_metadata_t) + size;
    mtkisp_metadata_t *buffer = (mtkisp_metadata_t *)malloc(buffer_size);
    if (buffer == nullptr)
    {
        return nullptr;
    }
    initial_isp_metadata_header(buffer, buffer_size);
    return buffer;
}

mtkisp_metadata_t *allocate_isp_metadata_byentrynum(uint32_t totalentry)
{
    if (totalentry == 0)
    {
        return nullptr;
    }
    size_t buffer_size = sizeof(mtkisp_metadata_t) + (sizeof(mtkisp_metadata_entry_t) * totalentry);
    mtkisp_metadata_t *buffer = (mtkisp_metadata_t *)malloc(buffer_size);
    if (buffer == nullptr)
    {
        return nullptr;
    }
    initial_isp_metadata_header(buffer, buffer_size);
    return buffer;
}

int32_t initial_isp_metadata_vector(std::vector<uint8_t> &vec, uint32_t totalentry)
{
    if (totalentry == 0)
    {
        return ERROR;
    }
    size_t buffer_size = sizeof(mtkisp_metadata_t) + (sizeof(mtkisp_metadata_entry_t) * totalentry);
    vec.resize(buffer_size);
    mtkisp_metadata_t *buffer = (mtkisp_metadata_t *)(vec.data());
    initial_isp_metadata_header(buffer, buffer_size);
    return OK;
}

/*
static int32_t bytecount_of_type(uint32_t type)
{
    switch(type)
    {
        case MetaDataType::TYPE_BYTE:
            return 1;
        case MetaDataType::TYPE_SHORT:
            return 2;
        case MetaDataType::TYPE_INT32:
        case MetaDataType::TYPE_FLOAT:
            return 4;
        case MetaDataType::TYPE_INT64:
            return 8;
        default:
            return 0;
    }
}
*/

int32_t add_isp_metadata_entry(mtkisp_metadata_t *pmeta, uint32_t tag, uint32_t type,
                                         void *pdata, uint32_t count, uint32_t *datasize)
{
    if (pmeta == nullptr || pdata == nullptr || count == 0)
    {
        return ERROR;
    }
    uintptr_t data = (uintptr_t)pmeta;
    mtkisp_metadata_entry_t *entry = (mtkisp_metadata_entry_t *)(data + pmeta->datasize);
    if (count > sizeof(entry->data))
    {
        ALOGE("tag[0x%X] : data count(%d) > entry data size(%d)", tag, count, sizeof(entry->data));
        assert(false);
        return OUT_OF_MEM;
    }
    // update entry
    memcpy(entry->data.u8, pdata, count);
    entry->tag = tag;
    entry->type = type;
    entry->count = count;
    // update metadata info
    pmeta->datasize += sizeof(mtkisp_metadata_entry_t);
    pmeta->entry_count++;
    if (datasize != nullptr)
    {
        *datasize = pmeta->datasize;
    }

    return OK;
}

int32_t add_isp_metadata_entry(std::vector<uint8_t> &vec, uint32_t tag, uint32_t type,
                                         void *pdata, uint32_t count, uint32_t *datasize)
{
    return add_isp_metadata_entry((mtkisp_metadata_t *)(vec.data()), tag, type, pdata, count, datasize);
}

int32_t get_isp_metadata_entry(mtkisp_metadata_t *pmeta, uint32_t tag, mtkisp_metadata_entry_t **pentry)
{
    if (pmeta == nullptr || pentry == nullptr)
    {
        return ERROR;
    }
    bool found = false;
    uintptr_t data = (uintptr_t)pmeta;
    mtkisp_metadata_entry_t *entry = (mtkisp_metadata_entry_t *)(data + pmeta->entry_offset);

    for (int i = 0; i < pmeta->entry_count; i++)
    {
        if (entry->tag == tag)
        {
            *pentry = entry;
            found = true;
            break;
        }
        entry++;
    }
    if (!found)
    {
        return NOT_FOUND;
    }

    return OK;
}

int32_t get_isp_metadata_entry(std::vector<uint8_t> &vec, uint32_t tag, mtkisp_metadata_entry_t **pentry)
{
    return get_isp_metadata_entry((mtkisp_metadata_t *)(vec.data()), tag, pentry);
}

uint32_t get_isp_metadata_datasize(mtkisp_metadata_t *pmeta)
{
    if (pmeta != nullptr)
    {
        return pmeta->datasize;
    }
    return 0;
}

int32_t free_isp_metadata(mtkisp_metadata_t *pmeta)
{
    if (pmeta != nullptr)
    {
        free(pmeta);
        return OK;
    }
    return ERROR;
}


