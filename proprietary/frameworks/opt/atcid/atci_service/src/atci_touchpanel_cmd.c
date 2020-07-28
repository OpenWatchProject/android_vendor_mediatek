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

#include "atci_touchpanel_cmd.h"
#include "atci_service.h"
#include "atcid_util.h"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#define DEV_TOUCH_PATH  "/sys/module/tpd_setting/parameters/tpd_fw_version"
#define TPD_GET_FWVER   (1)


int touchpanel_fwver_handler(char * cmdline, ATOP_t at_op, char *response){

    int fd;

    char com_date[10]={'\0'}, value[10];
    char log_info[100] = {'\0'};
    int ret = 0;
    fd = open(DEV_TOUCH_PATH, O_RDWR, 0);
    if(fd < 0) {
        return -1;
    }
    ret = read(fd, value, sizeof(value)-1);
    if (ret <= 0) {
        close(fd);
        return -1;
    }
    value[ret] = '\0';
    sprintf(com_date, "%s", value);

    switch(at_op){
            case AT_ACTION_OP:
            case AT_READ_OP:
                sprintf(log_info, "\r\n%s\r\n", value);
                break;
            case AT_TEST_OP:
                break;
        case AT_SET_OP:
            if(!strcmp(cmdline,com_date))
                sprintf(log_info,"\r\n\r\n OK\r\n\r\n");
            else
                sprintf(log_info,"\r\n\r\n ERROR\r\n\r\n");
            break;
    default:
            break;
    }
    snprintf(response, strlen(log_info) + 5, "\r\n%s \n\r\n", log_info);
    close(fd);
    return 0;
}
