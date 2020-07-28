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

#ifndef __MMPROFILEFILE_H__
#define __MMPROFILEFILE_H__

#ifdef __cplusplus
extern "C"
{
#endif


#include <linux/mmprofile_internal.h>

#define MMPFILE_ID  0x21504D4D
#define MMPFILE_FILE_VERSION    1

#define MMProfileEvent_ShowIncludeChild 0x1
#define MMProfileEvent_Disabled         0x2

typedef struct 
{
    unsigned int FileID;
    unsigned int Version;
    unsigned int TagCount;
    unsigned int Reserved;
} MMPFile_Header_t;

typedef enum 
{
    MMPFILE_TAG_EVENTINFO = 0,
    MMPFILE_TAG_RECORD,
    MMPFILE_TAG_EVENTCOLOR,
    MMPFILE_TAG_EVENTTREEINFO,
    MMPFILE_TAG_FTRACERAWDATA,
    MMPFILE_TAG_METADATA,
    MMPFILE_TAG_MAX,
    MMPFILE_TAG_END = 0xFFFFFFFF
} MMPFile_TAG_Type;

typedef enum 
{
    MMPFILE_EVENTFLAG_UNICODE = 0x0,
    MMPFILE_EVENTFLAG_MULBYTE = 0x1
} MMPFile_EventFlag_E;

typedef struct 
{
    MMPFile_TAG_Type Type;
    unsigned int Size;
    unsigned int Offset;
    unsigned int Reserved;
} MMPFile_TAG_t;

typedef struct 
{
    unsigned int EventCount;
    unsigned int Flag;
    unsigned int NameLen;
    unsigned int Reserved[1];
} MMPFile_TAG_EventInfo_t;

typedef struct 
{
    unsigned int RecordCount;
    unsigned int FreqLow;
    unsigned int FreqHigh;
    unsigned int Flag;
} MMPFile_TAG_Record_t;

typedef struct  
{
    unsigned int DataCount;
    unsigned int DataSize;
    unsigned int Reserved[2];
} MMPFile_TAG_MetaData_t;

typedef struct  
{
    unsigned int parent;
    unsigned int match_with_data1;
    unsigned long long start_time;
    unsigned long long end_time;
    unsigned int flag;
    char name[MMProfileEventNameMaxLen+1];
} MMPFile_EventInfo_t;

typedef struct  
{
    unsigned int id;
    unsigned int timeLow;
    unsigned int timeHigh;
    unsigned int flag;
#if 0 //#ifdef MMPROFILE_KERNEL_64
    unsigned long long data1;
    unsigned long long data2;
    unsigned long long meta_data_cookie;
#else
    unsigned int data1;
    unsigned int data2;
    unsigned int meta_data_cookie;
#endif
} MMPFile_Record_t;

typedef struct  
{
#if 0 //#ifdef MMPROFILE_KERNEL_64
    unsigned long long cookie;
#else
    unsigned int cookie;
#endif
    MMP_MetaDataType data_type;
#if 0 //#ifdef MMPROFILE_KERNEL_64
    unsigned long long data_size;
    unsigned long long data_offset;
#else
    unsigned int data_size;
    unsigned int data_offset;
#endif
} MMPFile_MetaData_t;

#ifdef __cplusplus
}
#endif
#endif
