/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/reboot.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "mounts.h"

#include <selinux/selinux.h>
#include <selinux/label.h>
#include <selinux/android.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fstab.h>

#if defined(FEATURE_FTM_EMMC) || defined(FEATURE_FTM_UFS) || defined(FEATURE_FTM_MNTL)
extern sp_ata_data return_data;

#define DATA_MEDIA_PARTITION "/data/media"
#define DATA_PARTITION      "/data"
#define NVDATA_PARTITION    "/mnt/vendor/nvdata"
#define SYSTEM_NVDATA "/system/mnt/vendor/nvdata"

/* #define REAL_FORMAT_PARTITION */

/* should be moved to customized part */
#define MAX_NUM_SDCARDS	 (3)
#define MIN_SDCARD_IDX   (0)
#define MAX_SDCARD_IDX   (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

#define FSTAB_PREFIX "/fstab."
#define MAX_SERVICE_CNT (16)
#define MAX_SERVICE_LEN (32)

using android::fs_mgr::Fstab;
using android::fs_mgr::FstabEntry;
using android::fs_mgr::GetEntryForMountPoint;
using android::fs_mgr::ReadDefaultFstab;

char g_data_related_services[MAX_SERVICE_CNT][MAX_SERVICE_LEN] = {
	"mobicore",
	"mobile_log_d",
	"mnld",
	"emdlogger1",
	"storageproxyd",
	"storage_persist",
	"storage_proinfo",
	"istorageproxyd",
	"ccci_fsd",
	"nvram_daemon",
	"connsyslogger",
	"mdlogger",
	"NULL"
};

typedef unsigned int u32;
enum {
	ITEM_PASS,
	ITEM_FAIL,
};
typedef unsigned long long u64;

static item_t bootdev_items[] = {
	/* item(ITEM_PASS,   uistr_pass), */
	/* item(ITEM_FAIL,   uistr_fail), */
	item(-1, NULL),
};

static item_t bootdev_items_menu[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
	item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
#endif
	item(-1, NULL),
};

int bootdev_test_result = 0;

struct bootdev {
	int           id;
	char          sys_path[512];
	char          info[1024];
	u32           csd[4];
	unsigned char ext_csd[512];
	unsigned int  capacity;
	unsigned int  boot_size;
	bool          avail;
	text_t        title;
	text_t        text;
	bool          exit_thd;
	pthread_t     update_thd;
	struct ftm_module *mod;
	struct itemview   *iv;
};

#define mod_to_bootdev(p)  (struct bootdev *)((char *)(p) + sizeof(struct ftm_module))

#define FREEIF(p)   do { if (p) free(p); (p) = NULL; } while(0)

#define BOOTDEV_BLKDEV_SIZE_PATH_UFS   "/sys/class/block/sdc/size"
#define BOOTDEV_BLKDEV_SIZE_PATH_EMMC  "/sys/class/block/mmcblk0/size"
#define UISTR_INFO_BOOTDEV_UFS         uistr_info_ufs
#define UISTR_INFO_BOOTDEV_EMMC        uistr_info_emmc
#define TAG                            "[STORAGE] "
#define ITEM_BOOTDEV                   ITEM_STORAGE

/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)
static int get_boot_type(void) {
	int fd;
	size_t s;
	char boot_type[4] = {'0'};

	fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
	if (fd < 0) {
		LOGD(TAG "fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
		return -1;
	}

	s = read(fd, (void *)&boot_type, sizeof(boot_type) - 1);
	close(fd);

	if (s <= 0) {
		LOGD(TAG "could not read boot type sys file\n");
		return -1;
	}

	boot_type[s] = '\0';

	return atoi((char *)&boot_type);
}

static bool bootdev_avail(struct bootdev *dev)
{
	char name[20];
	char *ptr;
	DIR *dp;
	struct dirent *dirp;
	int mt_boot_type = get_boot_type();

	if (mt_boot_type == BOOTDEV_UFS) {
		sprintf(name, "host%d", dev->id);
		ptr  = &(dev->sys_path[0]);
		ptr += sprintf(ptr, "/sys/class/scsi_host");
	} else {
		if (dev->id < MIN_SDCARD_IDX || dev->id > MAX_SDCARD_IDX)
			return false;

		sprintf(name, "mmc%d", dev->id - MIN_SDCARD_IDX);
		ptr  = &(dev->sys_path[0]);
		ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);
	}

	LOGD(TAG "target path: %s\n", dev->sys_path);
	LOGD(TAG "target name: %s\n", name);

	if (NULL == (dp = opendir(dev->sys_path))) {
		LOGD(TAG "opendir %s fail!\n", dev->sys_path);
		goto error;
	}

	while (NULL != (dirp = readdir(dp))) {
		LOGD(TAG "file name: %s\n", dirp->d_name);
		if (strstr(dirp->d_name, name)) {
			ptr += sprintf(ptr, "/%s", dirp->d_name);
			break;
		}
	}

	closedir(dp);

	if (!dirp) {
		LOGD(TAG "NULL dirp!\n");
		goto error;
	}

	return true;

error:
	return false;
}

static void bootdev_update_info(struct bootdev *dev, char *info)
{
	char *ptr;
	int inode = 0;
	u64 sector_cnt = 0;
	bootdev_test_result = 0;  /* 0: test no pass, 1: test pass */
	char buf[1024];
	ssize_t byte_read;
	int mt_boot_type = get_boot_type();

	dev->avail = bootdev_avail(dev);

	LOGD(TAG "avail: %d\n", dev->avail);

	if (mt_boot_type == BOOTDEV_UFS) {
		inode = open(BOOTDEV_BLKDEV_SIZE_PATH_UFS, O_RDONLY);

		if (inode < 0) {
			LOGD(TAG "open %s fail!\n", BOOTDEV_BLKDEV_SIZE_PATH_UFS);
			return;
		}
	} else {
		/* Use eMMC for default boot type */
		inode = open(BOOTDEV_BLKDEV_SIZE_PATH_EMMC, O_RDONLY);

		if (inode < 0) {
			LOGD(TAG "open %s fail!\n", BOOTDEV_BLKDEV_SIZE_PATH_EMMC);
			return;
		}
	}

	memset(buf, 0, sizeof(buf));
	byte_read = read(inode, buf, sizeof(buf) - 1);

	LOGD(TAG "byte_read: %d\n", (int)byte_read);

	sector_cnt = atol(buf);

	LOGD(TAG "sector_cnt: %llu\n", sector_cnt);

	dev->capacity = (float)(sector_cnt * 512 / 1024);

	LOGD(TAG "capacity: %d MB\n", dev->capacity / 1024);

	close(inode);

	ptr  = info;
	if (mt_boot_type == BOOTDEV_UFS)
		ptr += sprintf(ptr, "%s\n", UISTR_INFO_BOOTDEV_UFS);
	else
		ptr += sprintf(ptr, "%s\n", UISTR_INFO_BOOTDEV_EMMC);
	ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail,
		dev->avail ? uistr_info_emmc_sd_yes : uistr_info_emmc_sd_no);
	ptr += sprintf(ptr, "%s: %.2f GB\n", uistr_info_emmc_sd_total_size,
		(float)(dev->capacity) / (1024 * 1024));

	/*
	 * UFS and emmc share the same below structure: ftm_ata_emmc.
	 *
	 * Notice: Any change here must sync with SP ATA Tool's structure definition
	 *         in PC side.
	 */

	return_data.emmc.capacity = (float)(dev->capacity)/(1024 * 1024);

	bootdev_test_result = 1;  /* all things done, mark successful */

	return;
}

static void *bootdev_update_iv_thread(void *priv)
{
	struct bootdev *dev = (struct bootdev *)priv;
	struct itemview *iv = dev->iv;

	LOGD(TAG "%s: Start\n", __func__);

	bootdev_update_info(dev, dev->info);
	iv->start_menu(iv, 0);
	iv->redraw(iv);

	LOGD(TAG "%s: Exit\n", __func__);

	return NULL;
}

int bootdev_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;
	struct bootdev *dev = (struct bootdev *)priv;
	struct itemview *iv;

	LOGD(TAG "%s\n", __func__);

	init_text(&dev->title, param->name, COLOR_YELLOW);
	init_text(&dev->text, &dev->info[0], COLOR_YELLOW);

	bootdev_update_info(dev, dev->info);

	dev->exit_thd = false;

	if (!dev->iv) {
		iv = ui_new_itemview();
		if (!iv) {
			LOGD(TAG "No memory");
			return -1;
		}
		dev->iv = iv;
	}

	iv = dev->iv;
	iv->set_title(iv, &dev->title);
	if (FTM_AUTO_ITEM == param->test_type) {
		iv->set_items(iv, bootdev_items, 0);
	} else {
#ifdef FEATURE_FTM_TOUCH_MODE
		text_t lbtn;
		text_t cbtn;
		text_t rbtn;

		init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
		init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
		init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
		iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif
		iv->set_items(iv, bootdev_items_menu, 0);
	}
	iv->set_text(iv, &dev->text);
	iv->start_menu(iv, 0);
	iv->redraw(iv);

	if (FTM_AUTO_ITEM == param->test_type) {
		bootdev_update_iv_thread(priv);
	} else if (FTM_MANUAL_ITEM == param->test_type) {
		pthread_create(&dev->update_thd, NULL, bootdev_update_iv_thread, priv);
		do {
			chosen = iv->run(iv, &exit);

			switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
				case ITEM_PASS:
				case ITEM_FAIL:
					if (chosen == ITEM_PASS) {
						dev->mod->test_result = FTM_TEST_PASS;
					} else if (chosen == ITEM_FAIL) {
						dev->mod->test_result = FTM_TEST_FAIL;
					}
					exit = true;
					break;
#endif
#ifdef FEATURE_FTM_TOUCH_MODE
				case L_BTN_DOWN:
					dev->mod->test_result = FTM_TEST_FAIL;
					exit = true;
					break;
				case C_BTN_DOWN:
					exit = true;
					break;
				case R_BTN_DOWN:
					dev->mod->test_result = FTM_TEST_PASS;
					exit = true;
					break;
#endif
			}

			if (exit) {
				dev->exit_thd = true;
				break;
			}

		} while (1);
		pthread_join(dev->update_thd, NULL);
	}

	if (bootdev_test_result > 0)
		dev->mod->test_result = FTM_TEST_PASS;
	else
		dev->mod->test_result = FTM_TEST_FAIL;

	return 0;
}

int bootdev_init(void)
{
	int ret = 0;
	struct ftm_module *mod;
	struct bootdev *dev = NULL;

	LOGD(TAG "%s\n", __func__);

	mod = ftm_alloc(ITEM_BOOTDEV, sizeof(struct bootdev));

	if (!mod)
		return -ENOMEM;

	dev = mod_to_bootdev(mod);
	dev->mod = mod;

	dev->id = 0;

	dev->avail = false;

	bootdev_update_info(dev, dev->info);

	ret = ftm_register(mod, (ftm_entry_fn)bootdev_entry, (void *)dev);

	return ret;
}

#endif /* FEATURE_FTM_EMMC || FEATURE_FTM_UFS */

/*nfy add for clear bootdev*/
#ifdef FEATURE_FTM_CLEAREMMC
int ensure_all_mount_point_unmounted(const char *root_path)
{
	const MountedVolume *volume;
	LOGE(TAG "ensure_all_mount_point_unmounted %s\n", root_path);
	while (1) {
		scan_mounted_volumes();
		volume = find_mounted_volume_by_device(root_path);
		if (volume == NULL)
			break;
		LOGE(TAG "unmount %s for %s\n", volume->mount_point, root_path);
		if (unmount_mounted_volume(volume) < 0) {
			sleep(2);
			LOGE(TAG "unmount fail: %s\n", strerror(errno));
			ui_printf("unmount %s fail: %s\n", volume->mount_point, strerror(errno));
			return -1;
		}
	}
	return 0;
}

int ensure_root_path_unmounted(const char *root_path)
{
	/* See if this root is already mounted. */
	int ret = scan_mounted_volumes();
	const MountedVolume *volume;

	if (ret < 0) {
		LOGD(TAG "scan_mounted_volumes fail, ret: %d\n", ret);
		return ret;
	}

	volume = find_mounted_volume_by_mount_point(root_path);

	if (volume == NULL) {
		/* It's not mounted. */
		LOGD(TAG "The path %s is unmounted\n", root_path);
		return 0;
	}

	ret = unmount_mounted_volume(volume);

	if (ret < 0)
		LOGD(TAG "unmount_mounted_volume fail, ret: %d\n", ret);

	return ret;
}

#if 0
static int read_fstab(void)
{
	fstab = fs_mgr_read_fstab_default();
	if (!fstab) {
		SLOGE("failed to open fstab\n");
		return -1;
	}

	return 0;
}

static char *get_device_path_in_fstab(const char *partition)
{
	struct fstab_rec *rec = NULL;
	char *source = NULL;

	rec = fs_mgr_get_entry_for_mount_point(fstab, partition);
	if (!rec) {
		SLOGE("failed to get entry for %s\n", partition);
		return NULL;
	}

	asprintf(&source, "%s", rec->blk_device);
	return source;
}

static int free_fstab(void)
{
	fs_mgr_free_fstab(fstab);
	return 0;
}

static char *get_device_path(const char *partition)
{
	char *path = NULL;

	read_fstab();
	path = get_device_path_in_fstab(partition);
	free_fstab();
	return path;
}
#endif

static bool is_permissive(void)
{
	int rc;
	bool result = false;

	rc = is_selinux_enabled();
	if (rc < 0) {
		printf("%s is_selinux_enabled() failed (%s)\n", __FUNCTION__, strerror(errno));
		return false;
	}
	if (rc == 1) {
		rc = security_getenforce();
		if (rc < 0) {
			printf("%s getenforce fail (%s)\n", __FUNCTION__, strerror(errno));
			return false;
		}
		if (rc == 0)
			result = true;
		else
			result = false;
	} else {	// 0 means selinux is not enabled, treated as permissive
		result = true;
	}
	return result;
}

static void list_services_usage(const char *root)
{
	DIR *dir_proc, *dir_fd;
	int fd;
	int len, len_process_cmd;
	char buf_fd_link[1025] = "";
	char process_cmd[1025] = "";
	char path_buf[1025] = "";
	struct dirent *ent_proc, *ent_fd;

	LOGD("list_services_usage\n");

	if (!is_permissive())
		LOGE("Cannot query services that are using %s, please set selinux to permissive mode\n", root);

	dir_proc = opendir("/proc/");
	if (!dir_proc)
		return;

	while ((ent_proc = readdir(dir_proc)) != NULL) {
		if (ent_proc->d_name[0] < '0' || ent_proc->d_name[0] > '9')
			continue;
		snprintf(path_buf, sizeof(path_buf), "/proc/%s/cmdline", ent_proc->d_name);
		fd = open(path_buf, O_RDONLY);
		if (fd == -1)
			continue;
		len_process_cmd = read(fd, process_cmd, sizeof(process_cmd)-1);
		if (len_process_cmd > 0)
			process_cmd[len_process_cmd] = 0;
		close(fd);
		snprintf(path_buf, sizeof(path_buf), "/proc/%s/fd", ent_proc->d_name);
		dir_fd = opendir(path_buf);
		if (!dir_fd)
			continue;
		while ((ent_fd = readdir(dir_fd)) != NULL) {
			snprintf(path_buf, sizeof(path_buf), "/proc/%s/fd/%s", ent_proc->d_name, ent_fd->d_name);
			len = readlink(path_buf, buf_fd_link, sizeof(buf_fd_link)-1);
			if (len > 0) {
				buf_fd_link[len] = 0;
				if (strstr(buf_fd_link, root) == buf_fd_link) {
					if (len_process_cmd)
						LOGD(TAG "process(%s):%s\n", ent_proc->d_name, process_cmd);
					LOGD(TAG "file in used: %s\n", buf_fd_link);
				}
			}
		}
		closedir(dir_fd);
	}
	closedir(dir_proc);
}

int format_root_device(const char *root)
{
	Fstab fstab;
	FstabEntry *rec;
	int ret = 0;
#ifdef REAL_FORMAT_PARTITION
	bool crypt_footer;
	char *p = NULL;
#else
 /* change 4kb to 8kb,because f2fs can fix 4kb data corrupt,format will fail */
#define FORMAT_WRITE_BUF_SZ 8192
	int fd = -1;
	char *write_buf = NULL;
#endif
	const MountedVolume *volume1;
	char *devp = NULL;

	if (!strcmp(root, NVDATA_PARTITION)) {
		/*if no vendor partition , nvdata will be mount to /system/mnt/vendor/nvdata */
		volume1 = find_mounted_volume_by_mount_point(SYSTEM_NVDATA);

		if (volume1) {
			LOGD("device %s is mounted, ensure unmounted\n", SYSTEM_NVDATA);
			ret = ensure_root_path_unmounted(SYSTEM_NVDATA);
			if (ret < 0) {
				LOGE(TAG"format_root_device: can't unmount \"%s\"\n", SYSTEM_NVDATA);
				sleep(2);
				ui_printf("bootdev is busy can't format, please retry\n");
				list_services_usage(SYSTEM_NVDATA);
				ret = -1;
				goto out;
			}
		} else
			LOGD("device %s is not mounted\n", SYSTEM_NVDATA);
	}

	volume1 = find_mounted_volume_by_mount_point(root);
	if (volume1) {
		devp = strdup(volume1->device);
		LOGD("device %s is mounted, ensure unmounted\n", root);
		ret = ensure_root_path_unmounted(root);
		if (ret < 0) {
			LOGE(TAG"format_root_device: can't unmount \"%s\"\n", root);
			sleep(2);
			ui_printf("bootdev is busy can't format, please retry\n");
			list_services_usage(root);
			ret = -1;
			goto out;
		}
	} else
		LOGD("device %s is not mounted\n", root);

	if (!ReadDefaultFstab(&fstab)) {
		LOGE("failed to get fstab to get partition path");
		ret = -1;
		goto out;
	}

	rec = GetEntryForMountPoint(&fstab, root);
	if (rec == nullptr) {
		LOGE("failed to get device(%s) info from fstab for format\n", root);
	        ret = -2;
	        goto out;
	}

	#ifdef REAL_FORMAT_PARTITION
	if (devp) {
		p = (char *)realloc(rec->blk_device, strlen(devp)+1);
		if(p) {
			strcpy(p, devp);
			rec->blk_device = p;
		} else
			goto out;
	}

	crypt_footer = rec->is_encryptable() && !strcmp(rec->key_loc.c_str(), "footer");
	ret = fs_mgr_do_format(rec, crypt_footer);
	LOGE("format_volume: %s %s on %s\n",
		strcmp("f2fs", rec->fs_type.c_str())? "make_extf4fs":"make_f2fs", rec->blk_device.c_str(),
		(ret == 0) ? "succeed" : "failed");
	if (ret != 0)
		ret = -3;
	#else
	fd = open(rec->blk_device.c_str(), O_WRONLY | O_SYNC);
	if (fd < 0) {
		LOGE("%s: Fail to open %s, errno %d\n",
			__FUNCTION__, rec->blk_device.c_str(), errno);
		ret = -3;
		goto out;
	}

	write_buf = (char *)malloc(FORMAT_WRITE_BUF_SZ);
	if (!write_buf) {
		LOGE("%s: Fail to malloc, errno %d\n",
			__FUNCTION__, errno);
		ret = -3;
		goto out;
	}
	memset(write_buf, 0, FORMAT_WRITE_BUF_SZ);

	ret = write(fd, write_buf, FORMAT_WRITE_BUF_SZ);
	if (ret < 0) {
		LOGE("%s: Fail to write %s, errno %d\n",
			__FUNCTION__, rec->blk_device.c_str(), errno);
		ret = -3;
		goto out;
	}
	ret = 0;
	fsync(fd);
	#endif

out:
#ifndef REAL_FORMAT_PARTITION
	if (write_buf)
		free(write_buf);
	if (fd >= 0)
		close(fd);
#endif
	if(devp)
		free(devp);

	return ret;
}

static void stop_data_related_services() {
	int i;
	char cmd[MAX_SERVICE_LEN + 6]; //len("stop " + "\0") = 6

	for (i = 0; i < MAX_SERVICE_CNT; i++) {
		if (!strcmp(g_data_related_services[i], "NULL"))
			break;
		snprintf(cmd, sizeof(cmd) - 1, "stop %s", g_data_related_services[i]);
		LOGE(TAG "stopping service with command: %s\n", cmd);
		if (system(cmd) < 0)
			LOGE(TAG "%s fail: %s\n", cmd, strerror(errno));
	}
}

int clear_bootdev_entry(struct ftm_param *param, void *priv)
{
	int result = 0, cnt = 0;
	struct bootdev *dev = (struct bootdev *)priv;

	init_text(&dev->title, param->name, COLOR_YELLOW);
	init_text(&dev->text, &dev->info[0], COLOR_YELLOW);

	ui_printf("%s\n", uistr_info_emmc_format_data_start);
	sleep(1);

	stop_data_related_services();

	/* /data/media might be mounted after ensure_all_mount_point_unmounted()
	 * before format_root_device(DATA_PARTITION).
	 * This will cause data partition unmount failed.
	 * Add retry to make sure /data/media and /data are both unmount successfully
	 */
	while (cnt++ <= 3) {
		result = ensure_all_mount_point_unmounted(DATA_MEDIA_PARTITION);
		if (result) {
			sleep(1);
			continue;
		}
		result = format_root_device(DATA_PARTITION);
		if (result)
			sleep(1);
		else
			break;
	}
	if (result) {
		sleep(5);
		return false;
	}
	sync();

	result = format_root_device(NVDATA_PARTITION);
	if (result) {
		if (result == -2) {
			LOGE("has no device path: \"%s\"\n", NVDATA_PARTITION);
		} else {
			sleep(5);
			return false;
		}
	}
	sync();
	sleep(1);
	reboot(RB_AUTOBOOT);

	return result;
}

int clear_bootdev_init(void)
{
	int ret = 0;
	struct ftm_module *mod;
	struct bootdev *dev = NULL;

	mod = ftm_alloc(ITEM_CLREMMC, sizeof(struct bootdev));
	if (!mod)
		return -ENOMEM;

	dev = mod_to_bootdev(mod);

	ret = ftm_register(mod, (ftm_entry_fn)clear_bootdev_entry, (void *)dev);

	return ret;
}
#endif
