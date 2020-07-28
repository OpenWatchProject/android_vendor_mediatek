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

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#define DATA_SIZE_DEFAULT       "300"
#define SIZE_DEFAULT            "500"
#define TOTAL_SIZE_DEFAULT      "1000"


#define PROP_BUILD_TYPE     "ro.build.type"

#define CUSTOM_FILE         "/system/etc/mtklog-config.prop"

#define CUSTOM_PATH         "mtklog_path"
#define CUSTOM_SPEC_CUSTOM  "com.mediatek.log.mobile.customer"
#define CUSTOM_BOOT         "com.mediatek.log.mobile.enabled"
#define CUSTOM_SIZE         "com.mediatek.log.mobile.maxsize"
#define CUSTOM_TOTALSIZE    "com.mediatek.log.mobile.totalmaxsize"
#define CUSTOM_MOBILOG_PATH "com.mediatek.log.mobile.path"
#define CUSTOM_ANDROID_LOG  "com.mediatek.log.mobile.AndroidLog"
#define CUSTOM_KERNEL_LOG   "com.mediatek.log.mobile.KernelLog"
#define CUSTOM_AllMode      "com.mediatek.log.mobile.AllMode"


#define KEY_NUM             14   // the number of config command -- 'xxxx' or 'sublog_xxxx'
#define KEY_SPEC_CUSTOM     "SpecCustom"
#define KEY_BOOT            "autostart"
#define KEY_PATH            "logpath"
#define KEY_SIZE            "logsize"
#define KEY_TOTAL_SIZE      "totallogsize"
#define KEY_ANDROID         "AndroidLog"
#define KEY_KERNEL          "KernelLog"
#define KEY_ATF             "ATFLog"
#define KEY_BSP             "BSPLog"
#define KEY_MMEDIA          "MmediaLog"
#define KEY_SCP             "SCPLog"
#define KEY_SSPM            "SSPMLog"
#define KEY_ADSP            "ADSPLog"
#define KEY_MCUPM           "MCUPMLog"

#define ENABLE      "1"
#define DISABLE     "0"


typedef enum sd_type {
    INTERNAL_SD = 1,
    EXTERNAL_SD = 2
} sd_type_t;

typedef struct sd_context {
    sd_type_t location;
    char sdpath[256];
} sd_context_t;

// Special custom
typedef enum custom_id {
    CUSTOME_ID_MTK_IN = 0,
    CUSTOME_ID_GOD_A = 2,

    CUSTOME_ID_MAX
} custom_id_t;

extern sd_context_t sdContext;
extern bool gAllMode;
extern char spec_customer[16];
extern custom_id_t customId;

void init_custom_id(const char *name);
const char *read_config_value(const char *key);
void update_config_value(const char *key, const char *new_value);
int create_config_file(void);
bool ifAllModeConfigUnified();
int get_custom_mobilog_path(char[], int len);
int update_sd_context(const char* path);
#endif

