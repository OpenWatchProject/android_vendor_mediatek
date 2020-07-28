/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include "ccci_log.h"
#include "mdinit_ccci_log.h"

#define CCCI_PRINT_BUF_SIZE        4096

void ccci_log_print(int lvl, int md_id, const char *fmt, ...)
{
    va_list args;
    char *temp_log;
    int ret;

    temp_log = (char*)malloc(CCCI_PRINT_BUF_SIZE);
    if (temp_log == NULL)
        return;

    memset(temp_log, 0, CCCI_PRINT_BUF_SIZE);
    ret = snprintf(temp_log, CCCI_PRINT_BUF_SIZE, "(%d):", md_id);
    if (ret <= 0)
        goto _Exit;

    va_start(args, fmt);
    vsnprintf(&temp_log[ret], CCCI_PRINT_BUF_SIZE - ret, fmt, args);
    va_end(args);

    switch(lvl) {
    case LV_V:
        LOG_PRINT_V(temp_log, NULL);
        break;
    case LV_D:
        LOG_PRINT_D(temp_log, NULL);
        break;
    case LV_I:
        LOG_PRINT_I(temp_log, NULL);
        break;
    case LV_W:
        LOG_PRINT_W(temp_log, NULL);
        break;
    case LV_E:
        LOG_PRINT_E(temp_log, NULL);
        break;
    default:
        LOG_PRINT_D(temp_log, NULL);
        break;
    }

_Exit:
    free(temp_log);
}


