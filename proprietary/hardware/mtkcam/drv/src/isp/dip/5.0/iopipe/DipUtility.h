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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DIP_UTILITY_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DIP_UTILITY_H_

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include "isp_drv_dip_platform.h"

#define MAX_USER_NAME_LEN 64

#define DUMP_DIPPREFIX "/sdcard/dipdump"
#define DEBUG_DIPPREFIX "/data/vendor/dipdebug"



#define DIP_DUMP_IMGI_BUFFER    0x00000001
#define DIP_DUMP_IMGBI_BUFFER   0x00000002
#define DIP_DUMP_IMGCI_BUFFER   0x00000004
#define DIP_DUMP_VIPI_BUFFER    0x00000008
#define DIP_DUMP_DMGI_BUFFER    0x00000010
#define DIP_DUMP_DEPI_BUFFER    0x00000020
#define DIP_DUMP_LCEI_BUFFER    0x00000040
#define DIP_DUMP_UFDI_BUFFER    0x00000080
#define DIP_DUMP_TPIPE_BUFFER   0x00002000
#define DIP_DUMP_TUNING_BUFFER  0x00004000

#define DIP_DUMP_WDMA_BUFFER    0x00010000
#define DIP_DUMP_WROT_BUFFER    0x00020000
#define DIP_DUMP_IMG2O_BUFFER   0x00040000
#define DIP_DUMP_IMG3O_BUFFER   0x00080000
#define DIP_DUMP_PAK2O_BUFFER   0x00100000


/******************************************************************************
 *
 ******************************************************************************/

bool DrvMkdir(char const*const path, uint_t const mode);
bool DrvMakePath(char const*const path, uint_t const mode);
bool saveToFile(char const* filepath, unsigned char* pBuf, size_t  size);
bool saveToTextFile(char const* filepath, unsigned char* pBuf, size_t  size);
bool saveToRegFmtFile(char const* filepath, unsigned char* pBuf, size_t  size);

/******************************************************************************
 *
 ******************************************************************************/

#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_DIP_UTILITY_H_

