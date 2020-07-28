/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


#ifndef __SYSENV_UTIL_H___
#define __SYSENV_UTIL_H___

#ifdef __cplusplus
extern "C" {
#endif
#define SYSENV_LOG_TAG "SYSENV"
#include <log/log.h>

#define ERR_LOG(format, args...) \
    ALOGE("[%s]%s():%d , " format, SYSENV_LOG_TAG, __FUNCTION__, __LINE__, ##args)

#define WARN_LOG(format, args...) \
    ALOGW("[%s]%s():%d , " format, SYSENV_LOG_TAG, __FUNCTION__, __LINE__, ##args)

#define INFO_LOG(format, args...) \
    ALOGI("[%s]%s():%d , " format, SYSENV_LOG_TAG, __FUNCTION__, __LINE__, ##args)

#define DEBUG_LOG(format, args...) \
    ALOGD("[%s]%s():%d , " format, SYSENV_LOG_TAG, __FUNCTION__, __LINE__, ##args)

#define CFG_ENV_SIZE        0x4000     /*16KB*/
#define CFG_ENV_RW_OFFSET   0x20000
#define CFG_ENV_RO_OFFSET   0x24000

#define CFG_ENV_DATA_SIZE   (CFG_ENV_SIZE-sizeof(g_env[area].checksum)-sizeof(g_env[area].sig_head)-sizeof(g_env[area].sig_tail))
#define CFG_ENV_DATA_OFFSET (sizeof(g_env[area].sig_head))
#define CFG_ENV_SIG_1_OFFSET (CFG_ENV_SIZE - sizeof(g_env[area].checksum)-sizeof(g_env[area].sig_tail))
#define CFG_ENV_CHECKSUM_OFFSET (CFG_ENV_SIZE - sizeof(g_env[area].checksum))

#define TAG_UEVENT_PART_CNT     "NPARTS"
#define TAG_UEVENT_PART_NAME    "PARTNAME"

#define ENV_SIG "ENV_v1"
#define DATA_FREE_SIZE_TH_DEFAULT (50*1024*1024)
#ifdef CONFIG_MTK_SHARED_SDCARD
#define LIMIT_SDCARD_SIZE
extern long long data_free_size_th;
#endif // CONFIG_MTK_SHARED_SDCARD

struct env_struct {
    char sig_head[8];
    char *env_data;
    char sig_tail[8];
    int checksum;
};

enum {
    SYSENV_RW_AREA = 0,
    SYSENV_RO_AREA,
    SYSENV_AREA_MAX,
};

/* only used for user/kernel space sysenv coexist mechanism, removed if kernel module is phased out - start*/
#define KERNEL_SYSENV_EXIST
#ifdef KERNEL_SYSENV_EXIST
#define ENV_MAGIC   'e'
#define ENV_READ        _IOW(ENV_MAGIC, 1, int)
#define ENV_WRITE       _IOW(ENV_MAGIC, 2, int)
#define ENV_SET_PID     _IOW(ENV_MAGIC, 3, int)
#define ENV_USER_INIT   _IOW(ENV_MAGIC, 4, int)
#define TAG_SET_ENV "SYSENV_SET_ENV"
#define TAG_SHOW_ALL "SYSENV_SHOW_ALL"

struct env_ioctl
{
    char *name;
    int name_len;
    char *value;
    int value_len;
};
#endif

const char *sysenv_get(const char *name);
const char *sysenv_get_static(const char *name);
int sysenv_set(const char *name, const char *value);
int sysenv_set_static(const char *name, const char *value);
char* sysenv_get_all(int area);
#ifdef __cplusplus
} // extern "C"
#endif
#endif // __SYSENV_UTIL_H___

