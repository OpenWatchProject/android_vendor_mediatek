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
/******************************************************************************
 *
 * Filename:
 * ---------
 *   utils.h
 *
 * Description:
 * ------------
 *  Logger utility function implementation
 *
 *****************************************************************************/
#include <cutils/properties.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "Utils.h"
#include "ConsysLog.h"
#include "ConsysLogger.h"
#include <dirent.h>


bool makeDirWithPermission(const char *szPath, unsigned short mMode) {
    if (szPath == NULL) {
        return false;
    }
    size_t len = strlen(szPath) + 1;
    char currentLevelDir[len];
    char createdPath[len];
    char *pCreatedPath;
    char *pCurrentLevelDir;
    memset(createdPath, 0, len);
    pCreatedPath = createdPath;
    pCurrentLevelDir = currentLevelDir;
    for (unsigned long i = 0; i < len; i++) {
        *pCreatedPath = *szPath;
        *pCurrentLevelDir = *szPath;
        pCreatedPath++;
        szPath++;
        if ('/' == *pCurrentLevelDir || len - 1 == i) {
            *pCurrentLevelDir = '\0';
            if (strlen(currentLevelDir) != 0) {
                // Create current level path if not exist
                if (access(createdPath, F_OK) == -1) {
                    if (mkdir(createdPath, mMode) != -1) {
                        LOGD("Make Dir: Create directory [%s]", createdPath);
                    } else {
                        LOGE("Make Dir: Failed to create directory [%s], errno=%d", createdPath, errno);
                        if (errno == 17) {
                            usleep(500000);
                            if (access(createdPath, F_OK) == -1) {
                                LOGE("Make Dir: Failed to access directory after sleep 200ms, errno=%d", errno);
                                if (mkdir(createdPath,
                                        S_IRWXU | S_IRWXG | S_IRWXO) != -1) {
                                    return false;
                                }
                                return true;
                            } else {
                                return true;
                            }
                        } else {
                            return false;
                        }
                    }
                }
            }
            pCurrentLevelDir = currentLevelDir;
            continue;
        }
        pCurrentLevelDir++;
    }
    return true;
}

bool makeDir(const char *szPath) {
    if (szPath == NULL) {
        return false;
    }
    size_t len = strlen(szPath) + 1;
    char currentLevelDir[len];
    char createdPath[len];
    char *pCreatedPath;
    char *pCurrentLevelDir;
    memset(createdPath, 0, len);
    pCreatedPath = createdPath;
    pCurrentLevelDir = currentLevelDir;
    for (unsigned long i = 0; i < len; i++) {
        *pCreatedPath = *szPath;
        *pCurrentLevelDir = *szPath;
        pCreatedPath++;
        szPath++;
        if ('/' == *pCurrentLevelDir || len - 1 == i) {
            *pCurrentLevelDir = '\0';
            if (strlen(currentLevelDir) != 0) {
                // Create current level path if not exist
                if (access(createdPath, F_OK) == -1) {
                    if (mkdir(createdPath, S_IRWXU | S_IRWXG | S_IRWXO) != -1) {
                        LOGD("Make Dir: Create directory [%s]", createdPath);
                    } else {
                        LOGE("Make Dir: Failed to create directory [%s], errno=%d", createdPath, errno);
                        if (errno == 17) {
                            usleep(500000);
                            if (access(createdPath, F_OK) == -1) {
                                LOGE("Make Dir: Failed to access directory after sleep 200ms, errno=%d", errno);
                                if (mkdir(createdPath,
                                        S_IRWXU | S_IRWXG | S_IRWXO) != -1) {
                                    LOGD("ReMake Dir: Create directory [%s]", createdPath);
                                } else {
                                    LOGE("ReMake Dir: Fail Create directory [%s]", createdPath);
                                    return false;
                                }
                            } else {
                                LOGD("Make Dir: Can Access exist directory [%s]", createdPath);
                            }
                        } else {
                            return false;
                        }
                    }
                }
            }
            pCurrentLevelDir = currentLevelDir;
            continue;
        }
        pCurrentLevelDir++;
    }
    return true;
}

int createPthread(const char *name,
        pthread_t *thread, pthread_attr_t const * attr,
        void *(*start_routine)(void *), void * arg) {
    int err = pthread_create(thread, attr, start_routine, arg);
    if (err == 0) {
        int ret = pthread_setname_np(*thread, name);
        if (ret != 0) {
            const char * failmsg;
            switch(ret) {
            case EINVAL:
                failmsg = "thread is invalid.";
                break;
            case ERANGE:
                failmsg = "name length is out of range(16).";
                break;
            case ESRCH:
                failmsg = "thread does not specify a currently running thread in the process.";
                break;
            case EIO:
                failmsg = "IO error.";
                break;
            default:
                failmsg = strerror(ret);
            }
            LOGE("Can not set name %s for thread(%x), error:%d, err info:%s",
                    name, (int)(*thread), ret, failmsg);
        } else {
            LOGI("Set name %s for thread(%x)", name, (int)(*thread));
        }
    } else {
        LOGE("Failed to create thread %s. err info:%s",
                name, strerror(errno));
    }
    pthread_detach(pthread_t (*thread));
    return err;
}

bool isPthreadAlive(pthread_t thread_id) {
    if (thread_id == 0){
        return false;
    }
    LOGD("Thread thread (%lx) is alive.", thread_id);
    return true;
}
extern BOOT_MODE BootMode;

bool isBootCompleted() {
    if (BootMode == FACTORY_BOOT_MODE ||
        BootMode == ATE_FACTORY_BOOT_MODE ||
        BootMode == META_BOOT_MODE) {
        return true;
    }
    char systemReady[MAX_PATH_LEN];
    property_get(PROP_DEVICE_BOOT_COMPLETE, systemReady, "0");
    LOGD("systemReady:%s",systemReady);
    return (strcmp(systemReady, "0") == 0)? false: true;
}

bool checkSysPropEq(const char * propName,
        const char * expectValue, const char * defaultValue) {
    char prop[100]={0};
    unsigned int propLen = 0;
    property_get(propName, prop, defaultValue);
    if ((propLen = strlen(prop)) == 0) {
        return false;
    }
    if (propLen != strlen(expectValue)) {
        return false;
    }
    if (strncmp(prop, expectValue, propLen) == 0) {
        return true;
    }
    return false;
}

time_t getCurrentTimeStr(char *out, int *outlen) {
    if (NULL == out) {
        LOGE("Get current system time: Out buffer is null!");
        return false;
    }
    time_t pTime_T;
    struct tm *pCurrentTime;
    pTime_T = time(NULL);
    pCurrentTime = localtime(&pTime_T);
    if (NULL == pCurrentTime) {
        LOGE("Get current system time: Failed to get the time!");
        return (time_t)0;
    }
    // year
    sprintf(out, "%d", pCurrentTime->tm_year + 1900);
    *(out + 4) = '_';
    //month
    intToDateStr(out + 5, pCurrentTime->tm_mon + 1);
    //day
    intToDateStr(out + 7, pCurrentTime->tm_mday);
    *(out + 9) = '_';
    //hour
    intToDateStr(out + 10, pCurrentTime->tm_hour);
    //minute
    intToDateStr(out + 12, pCurrentTime->tm_min);
    //second
    intToDateStr(out + 14, pCurrentTime->tm_sec);
    *(out + 16) = '\0';
    if (NULL != outlen) {
        *outlen = 17;
    } LOGV("Get current System Time: %s", out);
    return pTime_T;
}

void intToDateStr(char *out, int in) {
    if (in > 9) {
        sprintf(out, "%d", in);
    } else {
        *out = '0';
        sprintf(out + 1, "%d", in);
    }
}

bool getATMFlag() {
    char prop[100];
    memset(prop,0,100);
    property_get(PROP_ATM_MODE_FLAG, prop, "disabled");
    LOGD("getATMFlag: %s=%s", PROP_ATM_MODE_FLAG,prop);
    if (strncmp(prop, "enable", strlen("enable")) == 0) {
        return true;
    }
     return false;
}

bool deleteFiles(char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) == -1) {
        LOGE("deleteFile: Failed to get the file status, errno=%d", errno);
        return true;
    } else {
        char filepath[256] = {0};
        if (file_stat.st_mode & S_IFDIR) {
            DIR *dp;
            struct dirent *dirp = NULL;
            dp = opendir(path);
            if (dp == NULL) {
                LOGE("delete Logs: %s is not exist!",path);
                return false;
            }
            while ((dirp = readdir(dp)) != NULL) {
                if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
                    continue;
                }
                memset(filepath, '\0', 256);
                snprintf(filepath,sizeof(filepath),"%s/%s",path,dirp->d_name);
                if (remove(filepath) < 0){
                    LOGE("delete log files failed:%s", path);
                }
            }
            closedir(dp);
            rmdir(path);
            dirp = NULL;
        } else {
            if (remove(path) < 0){
                LOGE("delete log file failed:%s", path);
                return false;
            }
        }
    }
    return true;
}

