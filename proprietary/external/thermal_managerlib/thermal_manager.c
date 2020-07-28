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
 * MediaTek Inc. (C) 2012. All rights reserved.
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *      thermal_manager.c
 *
 * Project:
 * --------
 *      YuSu
 *
 * Description:
 * ------------
 *      Set default values to all thermal zone device drivers and cooling device drivers when
 *      system init.
 *
 * Author:
 * -------
 *      CT Fang (mtk02403)
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <dlfcn.h>
#include <log/log.h>


#define MTK_LOG_ENABLE	(1)
#define TM_LOG_TAG "thermal_mgr"

#if MTK_LOG_ENABLE
#define TM_LOG(_priority_, _fmt_, args...)  LOG_PRI(_priority_, TM_LOG_TAG, _fmt_, ##args)

#else
#define TM_LOG(_priority_, _fmt_, args...)
#endif

int (*loadmtc)(char *) = NULL;
//#define LIB_FULL_NAME "libmtcloader.so"
#define LIB_FULL_NAME "/vendor/lib/libmtcloader.so"
typedef int (*load)(char *);

int (*loadchange_policy)(char *, int) = NULL;
typedef int (*load_change_policy)(char *, int);

int main(int argc, char **argv)
{
    int i = 0;
    void *handle, *func, *func2;

    TM_LOG(ANDROID_LOG_INFO, "%s argc %d.\n", __func__, argc);
    for (; i < argc; i++) {
        TM_LOG(ANDROID_LOG_INFO, "argv[%d] %s.\n", i, argv[i]);
    }

    handle = dlopen(LIB_FULL_NAME, RTLD_NOW);
    /* TM_LOG(ANDROID_LOG_INFO, "dlopen.\n"); */
	if (handle == NULL) {
		TM_LOG(ANDROID_LOG_ERROR, "fails to load lib err: %s.\n", dlerror());
		return -1;
	}

	func = dlsym(handle, "loadmtc");
	loadmtc = (load)func;

	if (loadmtc == NULL) {
        TM_LOG(ANDROID_LOG_ERROR, "loadmtc err: %s.\n", dlerror());
		dlclose(handle);
		return -1;
	}

    if (argc - 1 > 0) {
        int ret = 0;

        if (argc == 2) {
            TM_LOG(ANDROID_LOG_INFO, "loadmtc %s\n", argv[1]);
            ret = loadmtc(argv[1]);
        } else if (argc == 3) {
  	    func2 = dlsym(handle, "change_policy");
  	    loadchange_policy = (load_change_policy)func2;

    	    if (loadchange_policy == NULL) {
                TM_LOG(ANDROID_LOG_ERROR, "change_policy err: %s.\n", dlerror());
    		dlclose(handle);
    		return -1;
    	    }
    	    ret = loadchange_policy(argv[1], atoi(argv[2]));
            TM_LOG(ANDROID_LOG_INFO, "change_policy ret: %d.\n", ret);
 	}
	dlclose(handle);
        return ret;
    } else {
        TM_LOG(ANDROID_LOG_INFO, "loadmtc thermal.conf\n");
        int ret = loadmtc("/vendor/etc/.tp/thermal.conf"); /* default policy */
        dlclose(handle);
        return ret;
    }
}

