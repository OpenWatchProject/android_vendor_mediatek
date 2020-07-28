/******************************************************************************
Copyright Statement:

This software/firmware and related documentation ("MediaTek Software") are
protected under relevant copyright laws. The information contained herein
is confidential and proprietary to MediaTek Inc. and/or its licensors.
Without the prior written permission of MediaTek inc. and/or its licensors,
any reproduction, modification, use or disclosure of MediaTek Software,
and information contained herein, in whole or in part, shall be strictly prohibited.

MediaTek Inc. (C) 2018. All rights reserved.

BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.

The following software/firmware and/or related documentation ("MediaTek Software")
have been modified by MediaTek Inc. All revisions are subject to any receiver's
applicable license agreements with MediaTek Inc.
*****************************************************************************/
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <linux/inotify.h>

#include "file_monitor.h"
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
//For directory operation
#include <dirent.h>

#include <cutils/properties.h>
#include <cutils/misc.h>
#include <sys/ioctl.h>
#include <android/log.h>

#define MAX_WAIT_SECOND		0xefffffff
#define MAX_RETRY_COUNT					5
#define NVRAM_MAC_ADDRESS_OFFSET			4
#define BUF_SIZE					1024
#define WIFI_NVRAM_DATA_LEN				2048	/* nvram data len is always 512 bytes */
#define WIFI_LOADER_DEV					"/dev/wmtWifi"
#define WIFI_DRV_CFG_MISC_PATH				"/data/misc/wifi"
#define WIFI_DRV_CFG_SD_PATH				"/storage/sdcard0"
#define WIFI_NVRAM_PATH   				"/mnt/vendor/nvdata/APCFG/APRDEB"
#define WIFI_FW_CFG_PATH				"/vendor/firmware"
#define WIFI_NVRAM_INI_FILE				"/data/vendor/nvramwifi"
#define WIFI_MACADDR_FILE				"/data/vendor/macwifi"

#define FILE_REMOVE_MASK (IN_DELETE_SELF | IN_MOVE_SELF)
#define FILE_MODIFY_MASK IN_MODIFY
#define WATCH_FILE_MASK (FILE_REMOVE_MASK | FILE_MODIFY_MASK)
#define WATCH_PATH_MASK (IN_MOVED_TO | IN_CREATE)

#define DRV_CFG_CHANGED 1
#define FW_CFG_CHANGED	2
#define NVRAM_CHANGED	4

enum sync_kernel_action_t {
	no_action,
	delete_from_kernel,
	update_to_kernel,
};

void tag_log(int type, const char* tag, const char *fmt, ...) {
    char out_buf[1100] = {0};
    char buf[BUF_SIZE] = {0};
    va_list ap;

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    sprintf(out_buf, "%s %s", tag, buf);
    if(type == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, "agps", "%s", out_buf);
    } else {
        __android_log_print(ANDROID_LOG_ERROR, "agps", "%s", out_buf);
    }
}

int write_data_to_driver(char *data, size_t length)
{
	int ret = -1;
	int fd = -1;

	if (!data || !length)
		return ret;

	fd = open(WIFI_LOADER_DEV, O_RDWR);

	if (fd == -1) {
		LOGD("can't open device node(%s) failed reason is %s \n", WIFI_LOADER_DEV, strerror(errno));
		return ret;
	}

	/* write data to kernel */
	ret = write(fd, data, length);

	if (ret < 0)
		LOGD("write data to driver failed reason is %s \n", strerror(errno));

	close(fd);
	return ret;
}

int get_custom_mac_address(char mac[])
{
	int fd = -1;
	char buf[BUF_SIZE] = {0};
	int read_len = 0, i = 0;
	int ret_val = 1;

	memset(buf, 0, BUF_SIZE);
	fd = open(WIFI_MACADDR_FILE, O_RDONLY);
	if (fd == -1) {
		LOGD("Unable to access mac file\n");
		return 0;
	}

	read_len = read(fd, buf, BUF_SIZE - 1);
	if (read_len >= 17) {
		LOGD("MAC ADDR = %s\n", buf);
		for(i = 0; i < 17; i++)
			if (buf[i] == ':')
				buf[i] = 0;
		for(i = 0; i < 6; i++) {
			mac[i] = strtol(&buf[i * 3], NULL, 16);
			LOGD("mac[%d] = %x\n", i, mac[i]);
		}
	} else {
		ret_val = 0;
	}
	close(fd);
	return ret_val;
}

int write_nvram(char *filename)
{
	int ret = -1;
	int i = 0, fd = -1;
	char *acnvram;
	struct stat stat_nvram;
	int nvram_size = 0;
	char mac[6] = {0};
	int read_len = 0;

	/* sleep 1 more second in case that daemon is still writing */
	for (i = 0; i < MAX_RETRY_COUNT; i++) {
		sleep(1);
		if (stat(filename, &stat_nvram) == -1) {
			LOGD("stat %s error\n", filename);
			continue;
		}
		nvram_size = stat_nvram.st_size - 2;

		LOGD("nvram size = %d\n", nvram_size);
		if (nvram_size > 0 && (nvram_size & 0x0ff) == 0)
			break;
	}

	if (nvram_size <= 0 || (nvram_size & 0x0ff) != 0) {
		LOGD("invalid nvram size\n");
		return ret;
	}

	acnvram = (char *)malloc(nvram_size + 12);
	if (!acnvram) {
		LOGD("out of memory in allocating acnvram\n");
		return ret;
	}

	memset(acnvram, 0, nvram_size + 12);
	fd = open(filename, O_RDONLY);
	strncpy(acnvram, "WR-BUF:NVRAM", 12);
	if (fd == -1) {
		LOGD("Error opening nvram file\n");
		free(acnvram);
		return ret;
	}
	read_len = read(fd, acnvram + 12, nvram_size);
	close(fd);

	if (read_len <= 0) {
		free(acnvram);
		return ret;
	}

	if (get_custom_mac_address(mac))
		memcpy(acnvram + 12 + NVRAM_MAC_ADDRESS_OFFSET, mac, sizeof(mac));
	ret = write_data_to_driver(acnvram, 12 + nvram_size);
	if (ret < 0) {
		LOGD("write nvram to driver error \n");
	} else {
		property_set("vendor.mtk.nvram.ready", "1");
	}

	free(acnvram);
	return ret;
}

/*	IN int flag:
	0: clear
	1: set 		*/
int write_wificfg(char *file_name, char *prefix)
{
	int ret = -1;
	int read_len = 0;
	int cfg_len = 0;
	char *pcfg = NULL;
	FILE *fd;
	int prefix_len = strlen(prefix);

	fd = fopen(file_name, "r");
	if (!fd) {
		LOGD("file(%s) failed to open, err %s \n", file_name, strerror(errno));
		return ret;
	}
	fseek(fd, 0, SEEK_END);
	cfg_len = ftell(fd);
	if (cfg_len <= 0) {
		LOGD("file len is empty. \n");
		goto end;
	}

	pcfg = (char *)malloc(cfg_len + prefix_len);
	if (pcfg) {
		strncpy(pcfg, prefix, prefix_len);
		fseek(fd, 0, SEEK_SET);
		read_len = fread(pcfg + prefix_len, 1, cfg_len, fd);

		ret = write_data_to_driver(pcfg, read_len + prefix_len);
		if (ret < 0)
			LOGD("set cfg to driver error \n");
		free(pcfg);
	}
end:
	fclose(fd);
	return ret;
}


#define WRITE_DRV_CFG() write_wificfg(WIFI_DRV_CFG_SD_PATH"/wifi.cfg", "WR-BUF:DRVCFG")
#define WRITE_FW_CFG() write_wificfg(WIFI_FW_CFG_PATH"/wifi_fw.cfg", "WR-BUF:FWCFG")
#define REMOVE_DRV_CFG() write_data_to_driver("RM-BUF:DRVCFG", 13)
#define REMOVE_FW_CFG() write_data_to_driver("RM-BUF:FWCFG", 12)

int file_event_hander(int inot_fd, int wd, struct inotify_event *event, char *path, char *file_name)
{
	char file_path[512];

	snprintf(file_path, sizeof(file_path), "%s/%s", path, file_name);
	if ((event->mask & WATCH_PATH_MASK) && !strcmp(event->name, file_name)) {
		inotify_rm_watch(inot_fd, wd);
		wd = inotify_add_watch(inot_fd, file_path, WATCH_FILE_MASK);
	} else if (event->mask & FILE_REMOVE_MASK) {
		inotify_rm_watch(inot_fd, wd);
		if (access(file_path, R_OK) >= 0)
			wd = inotify_add_watch(inot_fd, file_path, WATCH_FILE_MASK);
		else
			wd = inotify_add_watch(inot_fd, path, WATCH_PATH_MASK);
	}
	return wd;
}

void get_custom_nvram_file_name(char *filename)
{
	int fd = -1;
	char buf[BUF_SIZE] = {0};
	int read_len = 0;

	memset(buf, 0, BUF_SIZE);
	fd = open(WIFI_NVRAM_INI_FILE, O_RDONLY);
	if (fd == -1) {
		strncat(filename, "WIFI", 4);
	} else {
		read_len = read(fd, buf, BUF_SIZE - 1);
		if (read_len > 0 && read_len < BUF_SIZE - 1) {
			buf[read_len] = 0;
			strncat(filename, buf, read_len);
		} else
			strncat(filename, "WIFI", 4);
		close(fd);
	}
	LOGD("custom nvram filename = %s\n", filename);
}

void* wlan_files_monitor(void* pdata)
{
	int inot_fd = 0;
	int dev_wd = -1;
#if 0
	int drv_cfg_wd = -1;
	int fw_cfg_wd = -1;
#endif
	int nvram_wd = -1;
	int readlen = 0;
	ssize_t readbytes = 0;
	int changed = 0;
	char buf[BUF_SIZE] = {0};
	struct stat stat_buf;
	struct inotify_event *event = NULL;
	int running = 1;
	fd_set inot_fd_set;
	struct timeval timeout;
	long timeout_sec = MAX_WAIT_SECOND;
	char nvram_filename[BUF_SIZE] = {0};

	LOGD("Running wlan_files_monitor\n");
	(void)pdata;
	while (access(WIFI_LOADER_DEV, F_OK) < 0)
		sleep(1);
#if 0
	if (access(WIFI_LOADER_DEV, F_OK) < 0) {
		LOGD(WIFI_LOADER_DEV" is not exist\n");
		event_loop_terminate();
		return NULL;
	}
#endif
	if (stat(WIFI_LOADER_DEV, &stat_buf) == -1) {
		LOGD("stat WIFI_LOADER_DEV fail\n");
		return NULL;
	}
	if (!S_ISCHR(stat_buf.st_mode)) {
		LOGD(WIFI_LOADER_DEV" is not a char device\n");
		return NULL;
	}
	inot_fd = inotify_init();
	if (inot_fd < 0) {
		LOGD("inotify_init error \n");
		return NULL;
	}
	dev_wd = inotify_add_watch(inot_fd, WIFI_LOADER_DEV, FILE_REMOVE_MASK);

	memset(nvram_filename, 0, sizeof(nvram_filename));
	snprintf(nvram_filename, sizeof(nvram_filename), "%s/", WIFI_NVRAM_PATH);
	get_custom_nvram_file_name(nvram_filename);
	LOGD("nvram_path_file = %s, nvrma_file = %s\n",
					nvram_filename,
					nvram_filename + strlen(WIFI_NVRAM_PATH) + 1);
	while (access(nvram_filename, R_OK) < 0)
		sleep(1);

	nvram_wd = inotify_add_watch(inot_fd, nvram_filename, WATCH_FILE_MASK);
	write_nvram(nvram_filename);

#if 0 /* wifi.cfg and wifi_fw.cfg, no need to read it in native currently */
	if (access(WIFI_DRV_CFG_SD_PATH"/wifi.cfg", R_OK) >= 0) {
		drv_cfg_wd = inotify_add_watch(inot_fd, WIFI_DRV_CFG_SD_PATH"/wifi.cfg", WATCH_FILE_MASK);
		WRITE_DRV_CFG();
	} else
		drv_cfg_wd = inotify_add_watch(inot_fd, WIFI_DRV_CFG_SD_PATH, WATCH_PATH_MASK);

	if (access(WIFI_FW_CFG_PATH"/wifi_fw.cfg", R_OK) >= 0) {
		fw_cfg_wd = inotify_add_watch(inot_fd, WIFI_FW_CFG_PATH"/wifi_fw.cfg", WATCH_FILE_MASK);
		WRITE_FW_CFG();
	} else
		fw_cfg_wd = inotify_add_watch(inot_fd, WIFI_FW_CFG_PATH, WATCH_PATH_MASK);
#endif
	while(running) {
		timeout.tv_sec = timeout_sec;
		timeout.tv_usec = 0;
		FD_ZERO(&inot_fd_set);
		FD_SET(inot_fd, &inot_fd_set);
		readlen = select(inot_fd + 1, &inot_fd_set, NULL, NULL, &timeout);
		if (readlen < 0 && errno == EINTR)
			continue;
		if (!readlen) {
			LOGD("will sync to driver, changed %#x\n", changed);
			if (changed & NVRAM_CHANGED)
				write_nvram(nvram_filename);
#if 0 /* wifi.cfg and wifi_fw.cfg, no need to read it in native currently */
			if (changed & DRV_CFG_CHANGED) {
				if (access(WIFI_DRV_CFG_SD_PATH"/wifi.cfg", R_OK) >= 0)
					WRITE_DRV_CFG();
				else
					REMOVE_DRV_CFG();
			}
			if (changed & FW_CFG_CHANGED) {
				if (access(WIFI_FW_CFG_PATH"/wifi_fw.cfg", R_OK) >= 0)
					WRITE_FW_CFG();
				else
					REMOVE_FW_CFG();
			}
#endif
			changed = 0;
			timeout_sec = MAX_WAIT_SECOND;
			continue;
		}
		readbytes = read(inot_fd, buf, sizeof(buf)-1);
		readlen = 0;
		while(readbytes > readlen) {
			event = (struct inotify_event *)&buf[readlen];

			readlen = readlen + sizeof(struct inotify_event) + event->len;
			if (event->mask & IN_IGNORED)
				continue;

			if (event->wd == dev_wd && (event->mask & FILE_REMOVE_MASK)) {
				LOGD("/dev/wmtWifi was removed, need to exit\n");
				running = 0;
				break;
	 		}

			if (event->wd == nvram_wd) {
				nvram_wd = file_event_hander(inot_fd, nvram_wd, event, WIFI_NVRAM_PATH, nvram_filename + strlen(WIFI_NVRAM_PATH) + 1);
				changed |= NVRAM_CHANGED;

#if 0 /* wifi.cfg and wifi_fw.cfg, no need to read it in native currently */
			} else if (event->wd == drv_cfg_wd) {
				drv_cfg_wd = file_event_hander(inot_fd, drv_cfg_wd, event, WIFI_DRV_CFG_SD_PATH, "wifi.cfg");
				changed |= DRV_CFG_CHANGED;
			} else if (event->wd == fw_cfg_wd) {
				fw_cfg_wd = file_event_hander(inot_fd, fw_cfg_wd, event, WIFI_FW_CFG_PATH, "wifi_fw.cfg");
				changed |= FW_CFG_CHANGED;
#endif
			}
		}
		if (changed > 0)
			timeout_sec = 1;
		memset(buf, 0, sizeof(buf));
	}

	if (dev_wd > 0)
		inotify_rm_watch(inot_fd, dev_wd);

#if 0
	if (drv_cfg_wd > 0)
		inotify_rm_watch(inot_fd, drv_cfg_wd);
#endif

	if (nvram_wd > 0)
		inotify_rm_watch(inot_fd, nvram_wd);

	return NULL;
}
