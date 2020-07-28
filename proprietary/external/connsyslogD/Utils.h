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
 *   Logger utility function definitions
 *
 *****************************************************************************/
#ifndef __UTILS_H__
#define __UTILS_H__
#include "GlbDefine.h"


#define PTHREAD_CREATE(name, pAddr, pAttr, callback, callbackParam) \
    createPthread(name, pAddr, pAttr, callback, callbackParam)

/**
 * Make directory recursively.
 *
 * TODO: rewrite this function to support existed folder checking.
 */
bool makeDir(const char *szPath);

/**
 * Make directory with permission
 *
 * TODO: rewrite this function to support create directory recursively.
 */
bool makeDirWithPermission(const char *szPath, unsigned short mMode);

/**
 * Create a pthread, and assign a name to it.
 */
int createPthread(const char *name,
        pthread_t *thread, pthread_attr_t const * attr,
        void *(*start_routine)(void *), void * arg);

/**
 * Check whether the given thread is alive or not.
 */
bool isPthreadAlive(pthread_t thread);

/**
 * Check system porperty "sys.boot_completed"
 * to judge if phone boot completely.
 *
 */
bool isBootCompleted();

/**
 * check whetcher system property is equal to the expected values.
 *
 * Parameter: propName: system property name string
 * Parameter: expectValue: the expected value
 * Parameter: defaultValue: system property default value if system property
 *                          not exists.
 * Return: BOOL: TRUE if euqal else FALSE.
 *
 */
bool checkSysPropEq(const char * propName,
        const char * expectValue, const char * defaultValue="");

/**
 * Get current timpstamp, and transform it to string.
 * Return the value of time it is used on transformation
 */
time_t getCurrentTimeStr(char *out, int *outlen);

/**
 * Transform int to string.
 */
void intToDateStr(char *out, int in);

/*
meta mode transfer log via usb or wifi
*/
typedef enum {
    ATM_WIFI_MODE = 0,
    ATM_USB_MODE
} ATM_ONLINE_MODE;

bool sendSourceFile(char* logPath,ATM_ONLINE_MODE mode=ATM_USB_MODE);
bool getATMFlag();
signed int createSocket();
/*
return:
1 usb connect
0 wifi conenct
*/
ATM_ONLINE_MODE checkConnectType();
bool deleteFiles(char *path);

#endif
