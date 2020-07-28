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
#include <sys/stat.h>
#include <private/android_filesystem_config.h>
#include "libnvram.h"
#include "libnvram_log.h"
#include "nvram_battery.h"

void* NVRAM_TOUCHPANEL(void* arg) {
	char buf[1024];
	int fd, size;
	struct stat stat_dir, stat_file;
	NVRAM_LOG("touch panel calibration");
	stat("/data/misc/touchpanel", &stat_dir);
	stat("/data/misc/touchpanel/calibration", &stat_file);
	if (!S_ISREG(stat_dir.st_mode)) mkdir("/data/misc/touchpanel", 0755);
	if (!S_ISREG(stat_file.st_mode)) {
		fd = open("/data/misc/touchpanel/calibration", O_CREAT,
		          S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		close(fd);
		chown("/data/misc/touchpanel/calibration", AID_ROOT, AID_DIAG);
	}
	if ((fd = open("/data/misc/touchpanel/calibration", O_RDWR)) < 0) {
		NVRAM_LOG("failed to open touchpanel calibration data");
		return NULL;
	}
	size = read(fd, buf, 1024);
	close(fd);
	if ((fd = open("/sys/module/tpd_setting/parameters/tpd_calmat",
	               O_WRONLY)) < 0) {
		NVRAM_LOG("failed to open touch panel driver");
		return NULL;
	}
	write(fd, buf, size);
	close(fd);
	pthread_exit(NULL);
	return NULL;
}
