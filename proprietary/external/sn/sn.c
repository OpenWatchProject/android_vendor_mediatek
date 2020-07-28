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

#define MTK_LOG_ENABLE 1
#define SN_LOG_TAG "sn"
#define LOG_TAG SN_LOG_TAG

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/file.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>

#define DEBUG 0

#define SN_DBG_LOG(_fmt_, args...) \
    do { \
        if (DEBUG) { \
            SLOGI(_fmt_, ##args); \
        } \
    } while(0)

#define SN_INFO_LOG(_fmt_, args...) \
    do { SLOGI(_fmt_, ##args); } while(0)

#define FILENAME "opensesame"

#define PATH1 "/sdcard/" FILENAME
#define PATH2 "/storage/sdcard0/" FILENAME
#define PATH3 "/storage/sdcard1/" FILENAME
#define CMODE_SYS "/sys/devices/platform/mt_usb/cmode"
#define CMODE_SYS2 "/sys/class/udc/musb-hdrc/device/cmode"

#define SN_LEN 20 /* The rule of SN pattern = "^([0-9A-Za-z]{0,20})$" */

#define RETRY_TIMES 15 /* 15 time */
#define SLEEP_TIME  10 /* 10 sec */

FILE *chk_file(const char *path)
{
	FILE *fp = NULL;

	/*Check file*/
	if (0 == access(path, R_OK)) {
		if ((fp = fopen(path, "r")) != NULL) {
			 SN_DBG_LOG("Open %s\n", path);
		} else {
			 SN_INFO_LOG("Fail to open err=%x\n", errno);
		}
	} else {
		SN_INFO_LOG("Fail to access err=%x", errno);
	}

	return fp;
}

int chk_path_w(const char *path)
{
	int sys_fp = -1;

	/*Check file*/
	if (0 == access(path, W_OK)) {
		if ((sys_fp = open(path, O_WRONLY)) != -1) {
			 SN_DBG_LOG("Open %s\n", path);
		} else {
			 SN_INFO_LOG("Fail to open err=%x\n", errno);
		}
	} else {
		SN_INFO_LOG("Fail to access err=%x", errno);
	}

	return sys_fp;
}

int reconnect_usb()
{
	int sys_fp = -1;
	int ret = 0;

	if ( (sys_fp = chk_path_w(CMODE_SYS)) != -1) {
		goto read;
	} else if ( (sys_fp = chk_path_w(CMODE_SYS2)) != -1) {
		goto read;
	} else {
		SN_INFO_LOG("Check all possible sys paths\n");
		goto end;
	}

read:
	if (sys_fp != -1) {
		SN_INFO_LOG("write 0\n");
		ret = write(sys_fp, "0", sizeof(char));
		if (ret <= 0)	{
			SN_INFO_LOG("Fail to write 0 ret=%x\n", ret);
			goto end;
		}

		sleep(2);

		ret = write(sys_fp, "1", sizeof(char));
		SN_INFO_LOG("write 1\n");
		if (ret <= 0) {
			SN_INFO_LOG("Fail to write 1 ret=%x\n", ret);
			goto end;
		}

		SN_DBG_LOG("Success to write cmode\n");
	}
end:
	if (sys_fp != -1) close(sys_fp);
	return ret;
}

int main()
{
	int retry = 0;
	int retry_times = property_get_int32("persist.sys.sn.retry_times", RETRY_TIMES);
	int sleep_time = property_get_int32("persist.sys.sn.sleep_time", SLEEP_TIME);

	SN_INFO_LOG("Retry Times %d\n", retry_times);
	SN_INFO_LOG("Sleep Time %d sec\n", sleep_time);

	while(retry < retry_times) {
		/*READ Serial Number from file first*/
		char buf[SN_LEN+1] = {0};
		char sn[SN_LEN+1] = {0};
		int ret, len;
		FILE *fp = NULL;
		int sys_fp = -1;

		SN_INFO_LOG("Retry %d\n", retry);

		if( (fp = chk_file(PATH1)) != NULL) {
			goto read;
		} else if( (fp = chk_file(PATH2)) != NULL) {
			goto read;
		} else if( (fp = chk_file(PATH3)) != NULL) {
			goto read;
		} else {
			SN_INFO_LOG("Check all possible paths\n");
			goto fail;
		}
read:
		/*Read file*/
		len = fread(buf, 1, SN_LEN, fp);
		if (ferror(fp) || len == 0 || len > SN_LEN) {
			SN_INFO_LOG("err=%x: fread fail ret=%x\n", errno, len);
			goto fail;
		} else {
			buf[len] = '\0';
		}

		SN_DBG_LOG("Length=%x, Content=%s\n", len, buf);

		/*Write data to sysfs*/
		sys_fp = open("/sys/class/android_usb/android0/iSerial", O_RDWR);
		if (sys_fp != -1) {
			strncpy(sn, buf, sizeof(char)*(SN_LEN+1));
			sn[SN_LEN] = '\0';

			ret = write(sys_fp, sn, sizeof(char) * strlen(sn));

			if (ret <= 0) {
				SN_INFO_LOG("Fail to write %s ret=%x\n", sn, ret);
				goto fail;
			} else {
				SN_DBG_LOG("Success to write %s\n", sn);
				if (fp > 0) fclose(fp);
				if (sys_fp != -1) close(sys_fp);
				if (ret > 0) reconnect_usb();
				break;
			}
		}

fail:
		if (fp > 0) fclose(fp);
		if (sys_fp != -1) close(sys_fp);
		retry++;
		sleep(sleep_time);
	}
	return 0;
}
