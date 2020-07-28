/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <dirent.h>
#include <cutils/properties.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/cdefs.h>
#include <malloc.h>

#include "pal_internal.h"

__BEGIN_DECLS

#define BUFF_SIZE 64

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pal_utils"

// concatenates two strings to new path
char* pal_strcat_path(const char* arg0, const char* arg1, char** out)
{
    if (out == NULL){
        return NULL;
    }
    FREE_IF(*out);
    size_t requested_size = strlen(arg0) + strlen(arg1) + 2;
    *out = (char*)malloc(sizeof(char) * requested_size);
    if (*out == NULL) {
        return NULL;
    }
    snprintf(*out, requested_size, "%s/%s", arg0, arg1);
    return *out;
}


#define BUFFER_SIZE 1024

// checks is it directory
int pal_is_path_dir(char* path)
{
    struct stat fsStat;
    BZEROTYPE(fsStat);
    if (stat(path, &fsStat) != 0) {
        DIR* dir = opendir(path);
        if (dir != NULL) {
            closedir(dir);
            return 1;
        }
    } else {
        if (S_ISDIR(fsStat.st_mode)) {
            return 1;
        } else if (S_ISLNK(fsStat.st_mode)) {
            char* new_path = (char*)malloc(BUFFER_SIZE * sizeof(char));
            char* old_path = (char*)malloc(BUFFER_SIZE * sizeof(char));
            if (new_path != NULL && old_path != NULL){
                strncpy(old_path, path, BUFFER_SIZE);
                do {
                    bzero(new_path, BUFFER_SIZE * sizeof(char));
                    readlink(old_path, new_path, BUFFER_SIZE);
                    strncpy(old_path, new_path, BUFFER_SIZE);
                    stat(old_path, &fsStat);
                } while(S_ISLNK(fsStat.st_mode));
            } else {
                PRINT_WRN("new_path = %p, old_path = %p", new_path, old_path);
            }
            FREE_IF(new_path);
            FREE_IF(old_path);
            if (S_ISDIR(fsStat.st_mode)) {
                return 1;
            }
        }
    }
    return 0;
}


#define PROC_INFO_BSZ 128

__END_DECLS
