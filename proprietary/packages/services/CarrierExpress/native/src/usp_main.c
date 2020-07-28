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


#define LOG_TAG "USP"

#include <sys/ioctl.h>
#include <utils/Log.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <cutils/properties.h>

#include <string.h>

#define ENV_MAGIC     'e'
#define ENV_READ         _IOW(ENV_MAGIC, 1, int)
#define ENV_WRITE        _IOW(ENV_MAGIC, 2, int)

#define BUF_MAX_LEN 20
#define NAME_E "mtk_usp_operator"

struct env_ioctl
{
    char *name;
    int name_len;
    char *value;
    int value_len;
};

int set_optr_via_ioctl(const char *optr_value) {
    /**
     * fd : For lk_env, boot animation customization
     * fd2: Kernel net driver need to know if SIM is Verizon
     */
    int fd, fd2;
    int ret = -1;

    struct env_ioctl en_ctl;
    char *name = NULL;
    char *value = NULL;
    memset(&en_ctl,0x00,sizeof(struct env_ioctl));
    // for Verizon
    int size;
    char buf[8];

    fd  = open("/proc/lk_env",O_WRONLY);
    fd2 = open("/proc/sys/net/optr",O_WRONLY);
    ALOGD("[USP][%s], optr_value:%s",__func__, optr_value);

    if (fd >= 0) {
        name = (char*) malloc(BUF_MAX_LEN);
        value = (char*) malloc(BUF_MAX_LEN);

        memset(name,0x00,BUF_MAX_LEN);
        memset(value,0x00,BUF_MAX_LEN);

        memcpy(name,NAME_E,strlen(NAME_E)+1);
        memcpy(value,optr_value,strlen(optr_value)+1);

        en_ctl.name = name;
        en_ctl.value = value;
        en_ctl.name_len = strlen(name)+1;
        en_ctl.value_len = strlen(value)+1;
        ret = ioctl(fd,ENV_WRITE,&en_ctl);
        ALOGE("[USP][%s]write ret value: %d, error=%d\n",__func__, ret, errno);
        free(name);
        free(value);
        close(fd);
    } else {
        ALOGE("[USP][%s]ERROR open /proc/lk_env fail %d, error=%d\n",__func__, fd, errno);
    }

    if (fd2 >= 0) {
        value = (char*) malloc(BUF_MAX_LEN);
        memset(value,0x00,BUF_MAX_LEN);
        memcpy(value,optr_value,strlen(optr_value)+1);

        size = snprintf(buf, 8, "%s", value);
        ret = write(fd2, buf, size);
        ALOGE("[USP][%s]write ret value: %d, error=%d\n",__func__, ret, errno);
        free(value);
        close(fd2);
    } else {
        ALOGE("[USP][%s]ERROR open /proc/sys/net fail %d, error=%d\n",__func__, fd2, errno);
    }
    return ret;
}

int main(int argc, char **argv)
{
    char optr[PROP_VALUE_MAX] = {0};
    char propStr[PROP_VALUE_MAX] = {0};
    int propValue = 0;
    property_get("persist.vendor.operator.optr", optr, "OP00");
    property_get("persist.vendor.mtk_usp_cfg_ctrl", propStr, "");
    ALOGE("[USP] main , optr %s", optr);
    if (strlen(optr) > 2) {
        set_optr_via_ioctl(&optr[2]);
    }
    //reset flag for configuration state
    if (strlen(propStr) > 0) {
        char propSetStr[PROP_VALUE_MAX] = {0};
        propValue = atoi(propStr);
        propValue &= 0xFFFFFFFB; //reset this with 2nd bit
        sprintf(propSetStr, "%d" , propValue);
        property_set("persist.vendor.mtk_usp_cfg_ctrl", propSetStr);
        ALOGE("[USP][%s]config state prop set",__func__);
    }
    if ((propValue & 0x8) == 8) {
        // returning as rebooting phone already
        return 0;
    }
    return 0;
}