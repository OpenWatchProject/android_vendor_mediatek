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

#include <stdio.h>
#include <stdlib.h>
//#include <dlfcn.h>
#include <linux/input.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
//#include <utils/String16.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "libbiosensor.h"

/* read NVRAM */
#include "libnvram.h"
#include "Custom_NvRam_LID.h"

#define BIONLOGE(fmt, arg ...) do {printf("%s: " fmt, __func__, ##arg); ALOGE("%s: " fmt, __func__, ##arg);} while(0)

int main(int argc, char *argv[]) 
{
    char nvram_init_val[PROPERTY_VALUE_MAX];
    int val, ret;
    int timeout = 0;
    struct BioData dat;
    int fd = 0;

    while(1) {
        property_get("service.nvram_init", nvram_init_val, NULL);
        if(strcmp(nvram_init_val, "Ready") == 0) { 
            ret = biosensor_read_nvram(&dat);
            if (ret) {
                timeout++;
                if (timeout >= 20) {
                    BIONLOGE("read from nvram timeout\n");
                    return 0;
                }
            } else {
                BIONLOGE("read from nvram success [%d]\n", val);
                break;
            }
        }
        usleep(500000);
    }

    ret = biosensor_open(&fd);
    if (ret) {
        BIONLOGE("biosensor_open fail\n");
    } else {
        biosensor_set_cali(fd, &dat);
        biosensor_close(fd);
    }

    return 0;
}

