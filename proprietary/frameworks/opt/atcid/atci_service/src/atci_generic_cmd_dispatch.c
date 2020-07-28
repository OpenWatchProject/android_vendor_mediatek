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
#include <errno.h>
#include <cutils/sockets.h>
#include "atci_service.h"
#include "atci_generic_cmd_table.h"

extern int s_fdAtci_generic_command;
extern int s_fdAtci_mmi_command;

int generic_command_hdlr(char* line) {
    int i = 0, ret = 0, dataLen = 0;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);

    if (line_cut == NULL) {
        return 0;
    }
    memset(response, 0, sizeof(response));
    table_size = (sizeof(generic_cmd_table)/sizeof(generic_cmd_type));


    for (i = 0; i < table_size;i++) {
        if (strcmp(line_cut, generic_cmd_table[i].cmdName) == 0) {
            ATOP_t at_op = getAtOp(&line);
            ALOGD("The command op is %d; Support op is %d", at_op, generic_cmd_table[i].opType);

            if ((at_op & generic_cmd_table[i].opType) == 0) {
                sprintf(response, "\r\n\r\nNOT IMPLEMENTED\r\n\r\n");
                dataLen = strlen(response);
                process_mmi_response(response, dataLen);
                free(line_cut);
                return 1;
            }

            ret = generic_cmd_table[i].cmd_handle_func(line, at_op, response);
            dataLen = strlen(response);
            if (dataLen >= 0) {
                process_mmi_response(response, dataLen);
            } else {
                ALOGD("NULL response from command handler function:%s", generic_cmd_table[i].cmdName);
            }
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int process_generic_command(char* line, int size) {
    UNUSED(size);

    if (generic_command_hdlr(line)) {
       return 0;
    }

    return 0;
}

char* cut_cmd_line(char* line)
{
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for(i = 0; i < size; i++)
    {
        if (line[i] == '=' || line[i] == '?') {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        ALOGD("fail to malloc result, return NULL");
        return NULL;
    }
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}
