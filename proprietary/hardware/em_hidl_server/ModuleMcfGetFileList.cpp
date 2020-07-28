/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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

#define LOG_TAG "EM/HIDL/McfConfig"
#define MTK_LOG_ENABLE 1

#include <cutils/log.h>
#include <dirent.h>
#include <string>
#include <stdint.h>
#include "ModuleMcfGetFileList.h"

void ModuleMcfGetFileList::searchdir(const char* path, char* file_list)
{

    ALOGI("[McfConfig->searchdir] root dir %s", path);
    DIR *dp;
    struct dirent *dmsg;
    char addpath[PATH_MAX] = {'\0'};
    if ((dp = opendir(path)) != NULL) {
        while ((dmsg = readdir(dp)) != NULL)
        {
            if (!strcmp(dmsg->d_name, ".") || !strcmp(dmsg->d_name, ".."))
                continue;
            strncpy(addpath, path, strlen(path)+1);
            strncat(addpath, "/", PATH_MAX-strlen(addpath)-1);
            strncat(addpath, dmsg->d_name, PATH_MAX-strlen(addpath)-1);
            if (dmsg->d_type == DT_REG){
                ALOGI("[McfConfig->searchdir] read file: %s", addpath);
                strncat(file_list, addpath, SEND_BUFFER_SIZE-strlen(file_list)-1);
                long size = file_size(addpath);
                strncat(file_list, ":", SEND_BUFFER_SIZE-strlen(file_list)-1);
                char size_string[32];
                sprintf(size_string, "%ld", size);
                strncat(file_list, size_string, SEND_BUFFER_SIZE-strlen(file_list)-1);
                strncat(file_list, ";", SEND_BUFFER_SIZE-strlen(file_list)-1);
            } else if (dmsg->d_type == DT_DIR ){
                ALOGI("[McfConfig->searchdir] read dir: %s", addpath);
                char *temp;
                temp=dmsg->d_name;
                if(strchr(dmsg->d_name, '.'))
                {
                   if((strcmp(strchr(dmsg->d_name, '.'), dmsg->d_name)==0))
                   {
                     continue;
                   }
                }
                searchdir(addpath, file_list);
            }
        }
    } else {
        ALOGI("[McfConfig] read null from %s",path);
    }
    ALOGI("[McfConfig->searchdir] read:%s",file_list);
    closedir(dp);
}

long ModuleMcfGetFileList::file_size(char* filename)
{
    ALOGI("[McfConfig->file_size] get file size from %s", filename);
    FILE *fp=fopen(filename,"r");
    if(!fp) return -1;
    fseek(fp,0L,SEEK_END);
    long size=ftell(fp);
    fclose(fp);
    ALOGI("[McfConfig->file_size] get file size : %ld", size);

    return size;
}



