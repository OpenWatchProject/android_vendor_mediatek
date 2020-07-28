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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include "hardware/ccci_intf.h"
#include "libterservice.h"


//namespace android
//{

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// for c libs

#define GEMINI_SIM_1 (0)
#define GEMINI_SIM_2 (1)

int getCcciSimType(int ccci_sys_fd, unsigned int* outVal) {
    int ret_ioctl_val = 0;
    int md1_value = 0;
    int md5_value = 0;
    char prop_value_MD1[PROPERTY_VALUE_MAX] = { 0 };
    char prop_value_MD5[PROPERTY_VALUE_MAX] = { 0 };
    char dev_node[32] = { 0 };

    property_get("ro.vendor.mtk_md1_support", prop_value_MD1, "0");
    property_get("ro.boot.opt_md5_support", prop_value_MD5, "0");
    RLOGD("opt_mdx_support=(%s, %s)", prop_value_MD1, prop_value_MD5);
    md1_value = atoi(prop_value_MD1);
    md5_value = atoi(prop_value_MD5);

    if (md1_value > 0) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_TERS_IOCTL, MD_SYS1));
        RLOGD("getCcciSimType ccci_get_node_name=%s", dev_node);
    } else if (md5_value > 0) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_TERS_IOCTL, MD_SYS5));
        RLOGD("getCcciSimType ccci_get_node_name=%s", dev_node);
    } else {
        RLOGD("getCcciSimType will open device on MD1/5 only. ");
    }

    int open_fd_locally = 0;
    if (ccci_sys_fd == 0) {
        open_fd_locally = 1;
        ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    }

    if (ccci_sys_fd < 0) {
        RLOGD("open CCCI ioctl2 port failed [%d, %d]", ccci_sys_fd, errno);
        return -1;
    }
//    unsigned int simType = 0xFFFFFFFF;
//    int ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_GET_SIM_TYPE, &simType);
    
//    RLOGD("##### getCcciSimType sizeof(*outVal)=%d, (%x)", sizeof(*outVal), *outVal);
    
    ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_GET_SIM_TYPE, outVal);
    if (ret_ioctl_val<0)
    {
        RLOGD("CCCI ioctl2 result: ret_val=%d, request=%d, [err: %s]", ret_ioctl_val, (unsigned int)CCCI_IOC_GET_SIM_TYPE, strerror(errno));
        *outVal = 0xFFFFFFFF;
    }
    else
    {
//        RLOGD("CCCI ioctl result: ret_val=%d, request=%d, simType=%d", ret_ioctl_val, CCCI_IOC_GET_SIM_TYPE, simType);
//        *outStr = simType;
        RLOGD("CCCI ioctl2 result: ret_val=%d, request=%d, outVal=%x", ret_ioctl_val, (unsigned int)CCCI_IOC_GET_SIM_TYPE, *outVal);
    }

//    char* mode;
//    asprintf(&mode, "%d", val_to_ret);
//    property_set(PROPERTY_SIM_SWITCH_MODE, mode);
//    free(mode);

    if (open_fd_locally==1)
        close(ccci_sys_fd); // remember to close it if it is opened locally.

    return ret_ioctl_val;
}

int enableCcciSimType(int ccci_sys_fd, bool enabled) {
    int ret_ioctl_val = 0;
    int md1_value = 0;
    int md5_value = 0;
    char prop_value_MD1[PROPERTY_VALUE_MAX] = { 0 };
    char prop_value_MD5[PROPERTY_VALUE_MAX] = { 0 };
    char dev_node[32] = {0};

    property_get("ro.vendor.mtk_md1_support", prop_value_MD1, "0");
    property_get("ro.boot.opt_md5_support", prop_value_MD5, "0");
    RLOGD("opt_mdx_support=(%s, %s)", prop_value_MD1, prop_value_MD5);
    md1_value = atoi(prop_value_MD1);
    md5_value = atoi(prop_value_MD5);

    if (md1_value > 0) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_TERS_IOCTL, MD_SYS1));
        RLOGD("getCcciSimType ccci_get_node_name=%s", dev_node);
    } else if (md5_value > 0) {
        snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_TERS_IOCTL, MD_SYS5));
        RLOGD("getCcciSimType ccci_get_node_name=%s", dev_node);
    } else {
        RLOGD("getCcciSimType will open device on MD1/5 only. ");
    }

    int open_fd_locally = 0;
    if (ccci_sys_fd == 0) {
        open_fd_locally = 1;
        ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    }

    if (ccci_sys_fd < 0) {
        RLOGD("open CCCI ioctl2 port failed [%d, %d]", ccci_sys_fd, errno);
        return -1;
    }

    unsigned int inVal = (enabled ? 1 : 0);
    ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_ENABLE_GET_SIM_TYPE, &inVal);
    if (ret_ioctl_val<0)
    {
        RLOGD("CCCI ioctl2 result: ret_val=%d, request=%d, enabled=%d, inVal=%d, [err: %s]", ret_ioctl_val, (unsigned int)CCCI_IOC_ENABLE_GET_SIM_TYPE, enabled, inVal, strerror(errno));
    }
    else
    {
        RLOGD("CCCI ioctl2 result: ret_val=%d, request=%d, enabled=%d, inVal=%d", ret_ioctl_val, (unsigned int)CCCI_IOC_ENABLE_GET_SIM_TYPE, enabled, inVal);
    }

//    char* mode;
//    asprintf(&mode, "%d", val_to_ret);
//    property_set(PROPERTY_SIM_SWITCH_MODE, mode);
//    free(mode);

    if (open_fd_locally==1)
        close(ccci_sys_fd); // remember to close it if it is opened locally.

    return ret_ioctl_val;
}

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus


//}; // namespace android
