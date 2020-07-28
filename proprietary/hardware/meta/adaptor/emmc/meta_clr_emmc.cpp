/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/reboot.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <linux/kdev_t.h>

#include <selinux/selinux.h>
#include <selinux/label.h>
#include <selinux/android.h>

#include <log/log.h>
#include <dirent.h>
#include "MetaPub.h"
#include "mounts.h"
#include "meta_clr_emmc_para.h"
#include <fstab.h> /* for fs_mgr_XXX APIs usage */
#include "cutils/properties.h" /* for property_get APIs usage */

#undef  TAG
#define TAG  "CLR_EMMC_META"
#define DATA_PARTITION "/data"
#define DATA_MEDIA_PARTITION "/data/media"
#define CACHE_PARTITION "/cache"
#define NVDATA_PARTITION "/mnt/vendor/nvdata"

/* #define REAL_FORMAT_PARTITION */

extern int WriteDataToPC(void *Local_buf, unsigned short Local_len, void *Peer_buf, unsigned short Peer_len);

#define LOGD ALOGD
#define LOGE ALOGE

using android::fs_mgr::Fstab;
using android::fs_mgr::FstabEntry;
using android::fs_mgr::GetEntryForMountPoint;
using android::fs_mgr::ReadDefaultFstab;

#define MAX_SERVICE_CNT (16)
#define MAX_SERVICE_LEN (32)
char g_data_related_services[MAX_SERVICE_CNT][MAX_SERVICE_LEN] = {
	"mobile_log_d",
	"mnld",
	"mobicore",
	"emdlogger1",
	"storageproxyd",
	"storage_persist",
	"storage_proinfo",
	"istorageproxyd",
	"connsyslogger",
	"NULL"
};

/********************************************************************************
//FUNCTION:
//              META_CLR_EMMC_init
//DESCRIPTION:
//              EMMC Init for META test.
//
//PARAMETERS:
//              void
//RETURN VALUE:
//              true : success
//      false: failed
//
********************************************************************************/
bool META_CLR_EMMC_init()
{
	LOGD("META_CLR_EMMC_INIT ...\n");
	return 1;
}

/********************************************************************************
//FUNCTION:
//              META_CLR_EMMC_deinit
//DESCRIPTION:
//              EMMC deinit for META test.
//
//PARAMETERS:
//              void
//RETURN VALUE:
//              void
//
********************************************************************************/
void META_CLR_EMMC_deinit()
{
	LOGD("META_CLR_EMMC_DEINIT ...\n");
	return;
}

int ensure_all_mount_point_unmounted(const char *root_path)
{
	const MountedVolume *volume;
	LOGE("ensure_all_mount_point_unmounted %s\n", root_path);
	while (1) {
		scan_mounted_volumes();
		volume = find_mounted_volume_by_device(root_path);
		if (volume == NULL)
			break;
		LOGE("unmount %s for %s\n", volume->mount_point, root_path);
		if (unmount_mounted_volume(volume) < 0) {
			sleep(2);
			LOGE("unmount fail: %s\n", strerror(errno));
			return -1;
		}
	}
	return 0;
}

int ensure_root_path_unmounted(const char *root_path)
{
	/* See if this root is already mounted. */
	const MountedVolume *volume;
	int ret = scan_mounted_volumes();
	if (ret < 0)
		return ret;

	volume = find_mounted_volume_by_mount_point(root_path);
	if (volume == NULL) {
		/* It's not mounted. */
		LOGD("The path %s is unmounted\n", root_path);
		return 0;
	}
	return unmount_mounted_volume(volume);
}

static bool is_permissive(void)
{
	int rc;
	bool result = false;

	rc = is_selinux_enabled();
	if (rc < 0) {
		LOGE("%s is_selinux_enabled() failed (%s)\n", __FUNCTION__, strerror(errno));
		return false;
	}
	if (rc == 1) {
		rc = security_getenforce();
		if (rc < 0) {
			LOGE("%s getenforce fail (%s)\n", __FUNCTION__, strerror(errno));
			return false;
		}
		if (rc == 0)
			result = true;
		else
			result = false;
	} else {
		result = true;
	}
	return result;
}

static void list_services_usage(const char *root)
{
	DIR *dir_proc, *dir_fd;
	int fd;
	int len, len_process_cmd;
	char buf_fd_link[1025] ="";
	char process_cmd[1025] ="";
	char path_buf[1025] ="";
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

int unmount_root_device(const char *root)
{
	int ret = 0;
	const MountedVolume *volume1;

	volume1 = find_mounted_volume_by_mount_point(root);
	if (volume1) {
		LOGD("device %s is mounted\n", root);
	}
	else
		LOGD("device %s is not mounted\n", root);

	/* Don't try to format a mounted device. */
	LOGD("ensure root path unmounted: %s\n", root);
	ret = ensure_root_path_unmounted(root);
	if (ret < 0) {
		LOGE("unmount_root_device: can't unmount \"%s\"\n", root);
		list_services_usage(root);
		ret = -1;
		goto out;
	}

out:
	return ret;
}

static int format_root_device(const char *root)
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
	struct selinux_opt seopts[] = {
		{ SELABEL_OPT_PATH, "/file_contexts" }
	};

	volume1 = find_mounted_volume_by_mount_point(root);
	if (volume1) {
		devp = strdup(volume1->device);
		LOGD("device %s is mounted\n", root);
	} else
		LOGD("device %s is not mounted\n", root);

	/* Don't try to format a mounted device. */
	LOGD("ensure root path unmounted: %s\n", root);
	ret = ensure_root_path_unmounted(root);
	if (ret < 0) {
		LOGE("format_root_device: can't unmount \"%s\"\n", root);
		list_services_usage(root);
		ret = -1;
		goto out;
	}

#if defined(MTK_GPT_SCHEME_SUPPORT) || defined(MNTL_SUPPORT)
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
		p = (char*) realloc(rec->blk_device, strlen(devp)+1);
		if (p) {
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
	fsync(fd);
	ret = 0;
	#endif
#else
	struct selabel_handle *sehnd =
		selabel_open(SELABEL_CTX_FILE, seopts, 1);
	if (!strcmp(root, DATA_PARTITION))
		ret = make_ext4fs("/emmc@usrdata", 0, root, sehnd);
	else if (!strcmp(root, CACHE_PARTITION))
		ret = make_ext4fs("/emmc@cache", 0, 0, 0);
	else if (!strcmp(root, NVDATA_PARTITION))
		ret = make_ext4fs("/emmc@nvdata", 0, 0, 0);
	LOGE("format_volume: make_ext4fs %s\n",
		(ret == 0) ? "succeed" : "failed");
	if (ret != 0)
		ret = -3;
	if(sehnd)
		selabel_close(sehnd);
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

static void stop_data_related_services()
{
	int i;
	char cmd[MAX_SERVICE_LEN + 6]; //len("stop " + "\0") = 6

	for (i = 0; i < MAX_SERVICE_CNT; i++) {
		if (!strcmp(g_data_related_services[i], "NULL"))
			break;
		snprintf(cmd, sizeof(cmd) - 1, "stop %s", g_data_related_services[i]);
		LOGE("stopping service with command: %s\n", cmd);
		if (system(cmd) < 0)
			LOGE("%s fail: %s\n", cmd, strerror(errno));
	}
}

int clear_emmc_entry()
{
	int result = 0, cnt = 0;
	LOGD("before clear emmc ...\n");

	stop_data_related_services();

	/* /data/media might mounted after ensure_all_mount_point_unmounted before format_root_device(DATA_PARTITION)
	 * which will cause data partition unmount fail, add retry to make sure /data/media and /data are both unmount successfully
	 */
	while (cnt++ <= 3) {
		result = ensure_all_mount_point_unmounted(DATA_MEDIA_PARTITION);
		result += format_root_device(DATA_PARTITION);
		if (result)
			sleep(1);
		else
			break;
	}
	LOGD("after clear DATA %s, %d...\n", DATA_PARTITION, result);
	if (result) {
		sleep(5);
		goto end;
	}
	sync();

	result = format_root_device(CACHE_PARTITION);
	LOGD("after clear CACHE %s, %d...\n", CACHE_PARTITION, result);
	if (result) {
		if (result == -2) {
			LOGE("has no device path: \"%s\"\n", CACHE_PARTITION);
		} else {
			sleep(5);
			goto end;
		}
	}
	sync();

	result = format_root_device(NVDATA_PARTITION);
	LOGD("after clear NVDATA %s, %d...\n", NVDATA_PARTITION, result);
	if (result) {
		if (result == -2) {
			LOGE("has no device path: \"%s\"\n", NVDATA_PARTITION);
		} else {
			sleep(5);
			goto end;
		}
	}
	sync();

end:
	return result;
}

int unmount_emmc_entry()
{
	int ret = 0, cnt = 0;
	LOGD("before unmount emmc ...\n");

	stop_data_related_services();

	while (cnt++ <= 3) {
		ret = ensure_all_mount_point_unmounted(DATA_MEDIA_PARTITION);
		ret += unmount_root_device(DATA_PARTITION);
		if (ret)
			sleep(1);
		else
			break;
	}
	LOGD("after unmount DATA %s, %d...\n", DATA_PARTITION, ret);
	if (ret)
		goto end;
	sync();

	ret = unmount_root_device(CACHE_PARTITION);
	LOGD("after unmount CACHE %s, %d...\n", CACHE_PARTITION, ret);
	if (ret)
		goto end;
	sync();

	ret = unmount_root_device(NVDATA_PARTITION);
	LOGD("after unmount NVDATA %s, %d...\n", NVDATA_PARTITION, ret);
	if (ret)
		goto end;
	sync();

end:
	if (ret)
		sleep(5);

	return ret;
}


#ifdef MTK_SHARED_SDCARD
int format_internal_sd_partition()
{
	int result = 0;
	result = system("/system/bin/rm -r /data/media");
	if (result) {
		LOGE("can NOT rm /data/media\n");
		return result;
	}
	sync();
	return result;
}

#else

static int get_dev_major_minor( const char *dev, int *major, int *minor)
{
	struct stat s;
	char linkto[256] = {0};
	int len;

	if(lstat(dev, &s) < 0) {
		LOGE("%s:lstat error\n", dev);
		return -1;
	}

	while( linkto[0] == 0) {
		if ((s.st_mode & S_IFMT) == S_IFCHR || (s.st_mode & S_IFMT) == S_IFBLK) {
			LOGD("major:%d minor:%d\n",(int) MAJOR(s.st_rdev), (int) MINOR(s.st_rdev));
			*major = (int) MAJOR(s.st_rdev);
			*minor = (int) MINOR(s.st_rdev);
			return 1;
		} else if ((s.st_mode & S_IFMT) == S_IFLNK) {
			len = readlink(dev, linkto, 256);
			if (len < 0) {
				LOGE("readlink error");
				return -1;
			}

			if (len > 255) {
				linkto[252] = '.';
				linkto[253] = '.';
				linkto[254] = '.';
				linkto[255] = 0;
				return -1;
			} else {
				linkto[len] = 0;
			}
			LOGD("linkto:%s\n",linkto);
		} else {
			LOGE("no major minor\n");
			return -1;
		}
		if (lstat(linkto, &s) < 0) {
			LOGE("%s:lstat error\n", dev);
			return -1;
		}
		linkto[0] = 0;
	}

	return 1;

}

static int get_mounts_dev_dir(const char *arg, char *dir)
{
	static FILE *f;
	char mount_dev[256];
	char mount_dir[256];
	char mount_type[256];
	char mount_opts[256];
	int mount_freq;
	int mount_passno;
	int match;
	char rd_line[128];
	char buf[255] = {0};
	int mount_major=-1, mount_minor=-1;
	static int major=0;
	static int minor=0;

	/**
	 **     parse the mounts to iterate all the mount points
	 **
	 **/
	if (arg != NULL) {
		if (f != NULL) {
			fclose(f);
			f = NULL;
		}
		get_dev_major_minor(arg, &major, &minor);
		f = fopen("/proc/mounts", "r");
	}

	if (!f) {
		LOGE("could not open /proc/mounts\n");
		return -1;
	}

	do {
		match = fscanf(f, "%255s %255s %255s %255s %d %d\n",
		mount_dev, mount_dir, mount_type,
		mount_opts, &mount_freq, &mount_passno);

		if (match == EOF)
			break;

		LOGD("mount_dev:%s\n", mount_dev);

		mount_dev[255] = 0;
		mount_dir[255] = 0;
		mount_type[255] = 0;
		mount_opts[255] = 0;

		//check the major & minor number
		if (match == 6 && get_dev_major_minor( mount_dev, &mount_major, &mount_minor) == 1 &&
		    major == mount_major && minor == mount_minor) {
			strcpy(dir, mount_dir);
			return 0;
		}
	} while (match != EOF);

	fclose(f);
	f = NULL;
	return -1;
}

#include <cutils/sockets.h>
#include <private/android_filesystem_config.h>
int do_vold_command(char* cmd) {
	int sock;
	char buffer[4096];
	int ret;
	char final_cmd[255] = "0 "; /* 0 is a (now required) sequence number */
	ret = strlcat(final_cmd, cmd, sizeof(final_cmd));
	if (ret >= sizeof(final_cmd)) {
		LOGE("Fail: the cmd is too long (%s)", final_cmd);
		return (-1);
	}

	if ((sock = socket_local_client("vold", ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM)) < 0) {
		LOGE("%s: Error connecting (%s)\n", __FUNCTION__, strerror(errno));
		exit(4);
	}

	if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
		LOGE("%s: write commnad error: (%s)\n", __FUNCTION__, strerror(errno));
		return errno;
	}

	while(1) {
		fd_set read_fds;
		struct timeval to;
		int rc = 0;

		to.tv_sec = 10;
		to.tv_usec = 0;

		FD_ZERO(&read_fds);
		FD_SET(sock, &read_fds);

		if ((rc = select(sock +1, &read_fds, NULL, NULL, &to)) < 0) {
			LOGE("%s: Error in select (%s) \n", __FUNCTION__, strerror(errno));
			return errno;
		} else if (!rc) {
			continue;
			LOGE("%s: [TIMEOUT] \n", __FUNCTION__);
			return ETIMEDOUT;
		} else if (FD_ISSET(sock, &read_fds)) {
			memset(buffer, 0, 4096);
			if ((rc = read(sock, buffer, 4096)) <= 0) {
				if (rc == 0)
					LOGE("%s: Lost connection to Vold - did it crash?\n", __FUNCTION__);
				else
					LOGE("%s: Error reading data (%s)\n", __FUNCTION__, strerror(errno));

				if (rc == 0)
					return ECONNRESET;
				return errno;
			}

			int offset = 0;
			int i = 0;

			for (i = 0; i < rc; i++) {
				if (buffer[i] == '\0') {
					int code;
					char tmp[4];

					strncpy(tmp, buffer + offset, 3);
					tmp[3] = '\0';
					code = atoi(tmp);

					LOGD("%s\n", buffer + offset);
					if (code >= 200 && code < 600) {
			   			if (code >= 400) {
							LOGD("%s: vold command(%s) return fail(%d)\n", __FUNCTION__, cmd, code);
				   			return -1;
			   			} else {
							return 0;
			   			}
		   			}
				  	offset = i + 1;
				}
			}
		}
	}
	return 0;
}

#define MEDIA_DIR "/mnt/media_rw/"
int format_internal_sd_partition()
{
	pid_t pid;
	char mnt_media_dir[256];
	char mnt_dir[256];
	char cmd[256];
	int get_mount_rtn = 0;
	int format_sd_ok = 0;
	Fstab fstab;
	FstabEntry *rec;
	char *source = NULL;

#if defined(MTK_GPT_SCHEME_SUPPORT) || defined(MNTL_SUPPORT)
	if (!ReadDefaultFstab(&fstab)) {
		LOGE("failed to get fstab to get partition path");
		goto out_without_free;
	}

	rec = GetEntryForMountPoint(&fstab, "/intsd");
	if (rec == nullptr) {
		LOGE("failed to mount intsd\n");
		goto out_without_free;
	}

	asprintf(&source, "%s", rec->blk_device.c_str());
	get_mount_rtn = get_mounts_dev_dir(source, mnt_media_dir);
	//get_mount_rtn = get_mounts_dev_dir("/dev/block/platform/mtk-msdc.0/by-name/intsd", mnt_media_dir);
#else
	get_mount_rtn = get_mounts_dev_dir("/emmc@fat", mnt_media_dir);
#endif

	if (get_mount_rtn || !strstr(mnt_media_dir, MEDIA_DIR)) {
		int rtn = 0;
#if defined(MTK_GPT_SCHEME_SUPPORT) || defined(MNTL_SUPPORT)
		//LOGD("/dev/block/platform/mtk-msdc.0/by-name/intsd is NOT mounted. Format it via newfs_msdos dirctly. \n", source);
		//rtn = system("/system/bin/newfs_msdos /dev/block/platform/mtk-msdc.0/by-name/intsd");
		LOGD("%s is NOT mounted. Format it via newfs_msdos dirctly. \n", source);
		free(source);
		asprintf(&source, "/system/bin/newfs_msdos %s", rec->blk_device.c_str());
		rtn = system(source);
#else
		LOGD("/emmc@fat is NOT mounted. Format it via newfs_msdos dirctly. \n");
		rtn = system("/system/bin/newfs_msdos /emmc@fat");
#endif
		return WEXITSTATUS(rtn);
	} else {
#if defined(MTK_GPT_SCHEME_SUPPORT) || defined(MNTL_SUPPORT)
		//LOGD("/dev/block/platform/mtk-msdc.0/by-name/intsd is mounted. Format it via VOLD.\n");
		LOGD("%s is NOT mounted. Format it via newfs_msdos dirctly. \n", source);
#else
		LOGD("/emmc@fat is mounted. Format it via VOLD.\n");
#endif
	}

	#if 0
	fs_mgr_free_fstab(fstab);
	#endif
	LOGD("mnt_media_dir %s\n", mnt_media_dir);
	snprintf(mnt_dir, sizeof(mnt_dir), "/storage/%s", mnt_media_dir+strlen(MEDIA_DIR));

	LOGD("Start to format %s\n", mnt_dir);
	sprintf(cmd,"volume unmount %s force", mnt_dir);
	if (do_vold_command(cmd)) {
		LOGE("vold command (%s) return fail\n", cmd);
		goto out_with_free;
	}

	sprintf(cmd,"volume format %s", mnt_dir);
	if (do_vold_command(cmd)) {
		LOGE("vold command (%s) return fail\n", cmd);
		goto out_with_free;
	}

	sprintf(cmd,"volume mount %s", mnt_dir);
	if (do_vold_command(cmd)) {
		LOGE("vold command (%s) return fail\n", cmd);
		goto out_with_free;
	}
	format_sd_ok = 1;

out_with_free:

	free(source);

out_without_free:

	return (format_sd_ok ? 0: -1);
}
#endif

int clear_emmc_internal_sd()
{
	int result = 0;
	LOGD("before clear internal sd ...\n");
	result = format_internal_sd_partition();
	LOGD("after clear internal sd, %d...\n", result);

	sync();
	return result;
}


/********************************************************************************
//FUNCTION:
//              META_FM_OP
//DESCRIPTION:
//              META FM test main process function.
//
//PARAMETERS:
//              req: FM Req struct
//      peer_buff: peer buffer pointer
//      peer_len: peer buffer length
//RETURN VALUE:
//              void
//
********************************************************************************/
void META_CLR_EMMC_OP(FT_EMMC_REQ *req)
{
	int ret = 0;
	FT_EMMC_CNF emmc_cnf;

	LOGD("req->op:%d\n", req->op);
	memcpy(&emmc_cnf, req, sizeof(FT_H) + sizeof(FT_EMMC_OP));
	emmc_cnf.header.id ++;
	switch (req->op) {
	case FT_EMMC_OP_CLEAR:
		ret = clear_emmc_entry();
		LOGD("clr emmc clear ret is %d\n", ret);
		emmc_cnf.m_status = META_SUCCESS;
		if (!ret) {
			emmc_cnf.result.clear_cnf.status = 1;
		} else {
			emmc_cnf.result.clear_cnf.status = 0;
		}
		WriteDataToPC(&emmc_cnf, sizeof(FT_EMMC_CNF), NULL, 0);
		break;

	case FT_EMMC_OP_FORMAT_TCARD:
		ret = clear_emmc_internal_sd();
		LOGD("clr emmc clear internal sd ret is %d\n", ret);
		emmc_cnf.m_status = META_SUCCESS;
		if (!ret) {
			emmc_cnf.result.form_tcard_cnf.status = 1;
		} else {
			emmc_cnf.result.form_tcard_cnf.status = 0;
		}
		WriteDataToPC(&emmc_cnf, sizeof(FT_EMMC_CNF), NULL, 0);
		break;

	case FT_EMMC_OP_UNMOUNT:
		ret = unmount_emmc_entry();

		LOGD("emmc unmount is %d\n", ret);
		emmc_cnf.m_status = META_SUCCESS;
		if (!ret) {
			emmc_cnf.result.unmount_cnf.status = 1;
		} else {
			emmc_cnf.result.unmount_cnf.status = 0;
		}
		WriteDataToPC(&emmc_cnf, sizeof(FT_EMMC_CNF), NULL, 0);
		break;

	default:
		emmc_cnf.m_status = META_SUCCESS;
		emmc_cnf.result.clear_cnf.status = META_STATUS_FAILED;
		WriteDataToPC(&emmc_cnf, sizeof(FT_EMMC_CNF), NULL, 0);
		break;
	}
}

