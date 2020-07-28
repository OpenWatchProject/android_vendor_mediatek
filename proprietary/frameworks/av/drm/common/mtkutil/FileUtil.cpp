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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmMtkUtil/FileUtil"
#include <utils/Log.h>

#include <FileUtil.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

using namespace android;

bool FileUtil::fopenx(const char* filename, const char* mode, FILE** fp)
{
    *fp = fopen(filename, mode);
    if (*fp == NULL)
    {
        ALOGE("fopenx: failed to open file [%s] using mode [%s], reason [%s]",
                filename, mode, strerror(errno));
        return false;
    }
    return true;
}

bool FileUtil::fclosex(FILE* fp)
{
    if (NULL == fp)
    {
        ALOGD("fclosex: NULL file pointer.");
        return true;
    }

    if (fclose(fp) != 0)
    {
        ALOGE("fclosex: failed to close file.");
        return false;
    }
    return true;
}

bool FileUtil::fseekx(FILE* fp, long int offset, int origin)
{
    if (fseek(fp, offset, origin) != 0)
    {
        ALOGE("fseekx: faild to seek to offset [%ld] from origin type [%d], reason [%s]",
                offset, origin, strerror(errno));
        return false;
    }
    return true;
}

bool FileUtil::fwritex(void* ptr, size_t size, FILE* fp)
{
    size_t writeSize = fwrite(ptr, size, 1, fp);
    if (writeSize != 1)
    {
        ALOGE("fwritex: failed to write [%d] amount of data to file, reason [%s]",
                size, strerror(errno));
        return false;
    }
    return true;
}

bool FileUtil::freadx(void* ptr, size_t size, FILE* fp)
{
    size_t readSize = fread(ptr, size, 1, fp);
    if (readSize != 1)
    {
        ALOGE("freadx: failed to read [%d] amount of data from file, reason [%s]",
                size, strerror(errno));
        return false;
    }
    return true;
}

bool FileUtil::ftellx(FILE* fp, long int& pos)
{
    pos = ftell(fp);
    if (pos == -1L)
    {
        ALOGE("ftellx: failed to get current fp position, reason [%s]", strerror(errno));
        return false;
    }
    return true;
}

// get one line from input file
// result contains CR LF, and NULL terminator
bool FileUtil::fgetsx(FILE* fp, char* str, int num)
{
    char* p = fgets(str, num, fp);
    if (p == NULL)
    {
        if (feof(fp) != 0) // reach end of file
        {
            ALOGE("fgetsx: reach EOF and nothing read.");
            return false;
        }
        else if (ferror(fp) != 0)
        {
            ALOGE("fgetsx: faild to get a line from file.");
            return false;
        }
    }
    return true;
}
