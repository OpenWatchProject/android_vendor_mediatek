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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   PropParser.cpp
 *
 * Description:
 * ------------
 *    Support Log tool customization
 *
 *******************************************************************************/
#include "PropParser.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include "ConsysLogger.h"

using namespace consyslogger;
extern bool sendCommandToAPK(const int msgid, const char *msgdata);

#define MAX_LINE_LEN 512

PropParser::PropParser() {
    mRecycleSize = 2000;
    bAutoStart = false;
}

PropParser::~PropParser() {
}

int PropParser::getRecycleSizeFromPropFile() {
    return mRecycleSize;
}

char * PropParser::deteteSpace(char* str) {
    char strTemp[MAX_LINE_LEN];
    char strTemp2[MAX_LINE_LEN];
    unsigned int i, j;
    memset(strTemp, '\0', MAX_LINE_LEN);
    strncpy(strTemp2, str, sizeof(strTemp2) - 1);
    strTemp2[sizeof(strTemp2) - 1] = '\0';
    j = 0;
    if (str != NULL) {
        for (i = 0; i < strlen(str); i++) {
            if (strTemp2[i] != ' ' && strTemp2[i] != '\n'
                    && strTemp2[i] != '\r') {
                strTemp[j++] = strTemp2[i];
            }
        }
    }
    strncpy(str, strTemp, strlen(strTemp));
    str[strlen(strTemp)] = '\0';
    return (str);
}

bool PropParser::getAutoStartConfigure() {
    return bAutoStart;
}

void PropParser::getPropFileConfig() {
    char key_value[MAX_LINE_LEN];
    FILE* fp = NULL;
    char line[MAX_LINE_LEN];
    char* pstr = (char*) NULL;
    memset(key_value, '\0', MAX_LINE_LEN);

    if ((fp = fopen(LOGGER_CUST_FILE, "r")) == (FILE*) NULL) {
        LOGE("Can not open prop file");
        return;
    } else {
        while (fgets(line, MAX_LINE_LEN, fp) != (char*) NULL) {
            deteteSpace(line);
            if (line[0] == '#')
                continue;
            if (strstr(line, LOGGER_RUNNING_KEY) != NULL) {
                pstr = strstr(line, "=");
                if (pstr != NULL) {
                    ++pstr;
                    strncpy(key_value, pstr, sizeof(key_value) - 1);
                    key_value[sizeof(key_value) - 1] = '\0';
                }
                int len = strlen("true");
                if (!strncmp(key_value, "true", len)) {
                    bAutoStart = true;
                    LOGI("Auto Run start mAutoStart = %d,not set it to true", bAutoStart);
                }
                if(isEngineerBuild()){
                    LOGI("Auto Run com.mediatek.log.consys.enabled = %s.", key_value);
                }
            } else if (strstr(line, LOGGER_LOG_PATH_NEW_KEY) != NULL) {
                pstr = strstr(line, "=");
                if (pstr != NULL) {
                    ++pstr;
                    strncpy(key_value, pstr,sizeof(key_value) - 1);
                    key_value[sizeof(key_value) - 1] = '\0';
                }
                // for future use
            } else if (strstr(line, LOGGER_LOG_RECYCLE_SIZE) != NULL) {
                pstr = strstr(line, "=");
                if (pstr != NULL) {
                    ++pstr;
                    strncpy(key_value, pstr, sizeof(key_value) - 1);
                    key_value[sizeof(key_value) - 1] = '\0';
                }
                LOGI("Log Recycle size = %s.", key_value);
                int nValue = atoi(key_value);
                if (nValue > 0) {
                    mRecycleSize = nValue;
                }
            }
        }
    }
    fclose(fp);
}
