/*
* MediaTek Inc. (C) 2016. All rights reserved.
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly
* prohibited.
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <linux/fs.h>
#include <selinux/android.h>
#include <selinux/selinux.h>
#include <selinux/label.h>
#include <cutils/klog.h>
#include "mntl_link.h"
#define ERROR(x...)   KLOG_ERROR("mntl_gpt_init", x)
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define FOLDER_DEFAULT_MODE (S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
struct selabel_handle *sehandle;

struct {
	char const *name;
	mode_t mode;
} modelist[15] =
{
	{"preloader", 00640},
	{"pro_info",  00660},
	{"boot",      00640},
	{"recovery",  00640},
	{"nvram",     00660},
	{"seccnfg",   00660},
	{"misc",      00660},
	{"para",      00660},
	{"secstatic", 00640},

	{"cache",     00660},
	{"metadata",  00660},
	{"nvcfg",     00660},
	{"nvdata",     00660},
	{"system",     00660},
	{"userdata",   00660},
};

gid_t system_gid = 0;

static gid_t get_system_gid()
{
	struct passwd *pwd;

	pwd = getpwnam("system");   /* Try getting UID for username */
	if (pwd == NULL) {
		ERROR("getpwnam fail: %s\n", strerror(errno));
		return -1;
	}
	return pwd->pw_gid;
}

static time_t gettime(void)
{
    struct timespec ts;
    int ret;

    ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret < 0) {
        ERROR("clock_gettime(CLOCK_MONOTONIC) failed: %s\n", strerror(errno));
        return 0;
    }

    return ts.tv_sec;
}


static int wait_for_file(const char *filename, int timeout)
{
    struct stat info;
    time_t timeout_time = gettime() + timeout;
    int ret = -1;

    while (gettime() < timeout_time && ((ret = stat(filename, &info)) < 0))
        usleep(10000);

    return ret;
}

static mode_t get_mode(char const *path)
{
	int i;

	for(i = 0; i < 15; i++) {
		if(strstr(path, modelist[i].name)) {
			return modelist[i].mode;
		}
	}
	return 0;
}

static void wait_mtd_device(void)
{
	int fd;
	char buf[1024], target[255], source[255];
	char *bufp;
	ssize_t size;
	int r;
	char *secontext = NULL;
	mode_t mode;
	int num = 0, mtd_size, mtd_erasesize;

	fd = open("/proc/mtd", O_RDONLY);
	if (fd < 0)
		return;

	buf[sizeof(buf) - 1] = '\0';
	size = read(fd, buf, sizeof(buf) - 1);
	bufp = buf;
	while (size > 0) {
		char name[16];
		name[0] = '\0';
		r = sscanf(bufp, "mtd%d: %x %x %15s",
				&num, &mtd_size, &mtd_erasesize, name);
		while (size > 0 && *bufp != '\n') {
			bufp++;
			size--;
		}
		if (size > 0) {
			bufp++;
			size--;
		}
	}
	close(fd);
	sprintf(source, "/dev/block/mtdblock%d", num);
	wait_for_file(source, 50);
	printf("wait for %s done\n", source);
}

static void create_mtd_link(void)
{
	char const *para = "para";
	int fd, num = -1, mtd_size, mtd_erasesize;
	char name[16], buf[1024], target[255], source[255];
	char *bufp = buf;
	ssize_t size;
	int r;
	char *secontext = NULL;
	mode_t mode;

	buf[1023] = '\0';

	fd = open("/proc/mtd", O_RDONLY);
	if (fd < 0)
		return;

	size = read(fd, buf, 1023);
	while (size > 0) {

		name[0] = '\0';
		r = sscanf(bufp, "mtd%d: %x %x %15s",
				&num, &mtd_size, &mtd_erasesize, name);
		if ((r == 4) && (name[0] == '"')) {
			char *x = strchr(name + 1, '"');
			if (x) {
				*x = 0;
			}
			if (strcmp(name+1, "misc")==0) {
				sprintf(target, "/dev/block/platform/bootdevice/by-name/para");
				mode = get_mode(para);
			} else {
				sprintf(target, "/dev/block/platform/bootdevice/by-name/%s", name+1);
				mode = get_mode(name+1);
			}
			sprintf(source, "/dev/block/mtdblock%d", num);
			r = symlink(source, target);
			if (r)
				ERROR("symlink %s to %s, err %d\n", target, source, r);
			chown(target, -1, system_gid);
			chown(source, -1, system_gid);
			if (mode) {
				chmod(target, mode);
				chmod(source, mode);
			}

			if (sehandle) {
				selabel_lookup(sehandle, &secontext, target, mode);
				printf("target=%s source=%s context=%s\n", target, source, secontext);
				r = lsetfilecon(target, secontext);
				if (r)
					ERROR("target=%s context=%s errno=%d\n", target, secontext, errno);
				r = lsetfilecon(source, secontext);
				if (r)
					ERROR("source=%s context=%s errno=%d\n", source, secontext, errno);
			}

			sprintf(target, "/dev/%s", name+1);
			sprintf(source, "/dev/mtd/mtd%d", num);
			r = symlink(source, target);
			if (r)
				ERROR("symlink %s to %s, err %d\n", target, source, r);
			chown(target, -1, system_gid);
			chown(source, -1, system_gid);
			if (mode) {
				chmod(target, mode);
				chmod(source, mode);
			}

			if (sehandle) {
				selabel_lookup(sehandle, &secontext, target, mode);
				printf("target=%s source=%s context=%s\n", target, source, secontext);
				r = lsetfilecon(target, secontext);
				if (r)
					ERROR("target=%s context=%s errno=%d\n", target, secontext, errno);
				r = lsetfilecon(source, secontext);
				if (r)
					ERROR("source=%s context=%s errno=%d\n", source, secontext, errno);
			}
#if 0
			sprintf(target, "/mtd@%s", name+1);
			r = symlink(source, target);
			if (r)
				ERROR("symlink %s to %s, err %d\n", target, source, r);
			chown(target, -1, system_gid);
			file_chmod(target, mode);
			lsetfilecon(target, secontext);
#endif
		}
		while (size > 0 && *bufp != '\n') {
			bufp++;
			size--;
		}
		if (size > 0) {
			bufp++;
			size--;
		}
	}
	close(fd);
}

static int mt_get_sector_size(int *sector_size)
{
	FILE *fp = NULL;
	char buf[128];
	fp = fopen(MNTL_HW_SECTOR_SIZE, "r");
	if (fp) {
		fgets(buf, sizeof(buf), fp);
		if (sscanf(buf, "%d", sector_size) == 1) {
			ERROR("sector size=%d, gpt header sz=%zu\n", *sector_size, sizeof(GuidPartitionTableHeader_t));
		} else {
			ERROR("find sector size fail\n");
		}
		fclose(fp);
		return 0;
	} else {
		ERROR("open %s fail\n",MNTL_HW_SECTOR_SIZE);
		return 1;
	}
}

static int mt_gpt_get_part_entry(GuidPartitionTableHeader_t *pgpt_header, char** part_entry, int sector_size)
{
	char *ptr = NULL;
	int fd = 0;

	fd = open(MNTLBLK, O_RDONLY);
	if (fd < 0) {
		ERROR("open %s fail\n", MNTLBLK);
		return 1;
	}

	ssize_t len = 2 * sector_size;
	ptr = (char *)malloc(len);
	if (ptr == NULL) {
		close(fd);
		ERROR("malloc fail\n");
		return 1;
	}

	if (read(fd, ptr, len) != len) {
		ERROR("read sector fail\n");
		free(ptr);
		close(fd);
		return 1;
	}
	memcpy(pgpt_header, &ptr[sector_size], sizeof(*pgpt_header));
	free(ptr);

	if (lseek64(fd, pgpt_header->PartitionEntryLBA * sector_size, SEEK_SET) == -1) {
		ERROR("leesk %ju fail\n", pgpt_header->PartitionEntryLBA * sector_size);
		close(fd);
		return 1;
	}

	len = ((uint64_t) pgpt_header->NumberOfPartitionEntries * (uint64_t)pgpt_header->SizeOfPartitionEntry);
	if (((uint64_t)len >> 32) > 0) {
		ERROR("currently, we do not support allocate buffer size larger than 32-bit\n");
		close(fd);
		return 1;
	}
	*part_entry = (char *)malloc(len);

	if (part_entry == NULL) {
		close(fd);
		ERROR("malloc fail\n");
		return 1;
	}

	if (read(fd, *part_entry, len) != len) {
		ERROR("read PE fail\n");
		close(fd);
		free(*part_entry);
		return 1;
	}
	close(fd);
	return 0;
}

static const struct selinux_opt seopts_file_plat[] = {
	{ SELABEL_OPT_PATH, "/system/etc/selinux/plat_file_contexts" },
	{ SELABEL_OPT_PATH, "/plat_file_contexts" }
};
static const struct selinux_opt seopts_file_vendor[] = {
	{ SELABEL_OPT_PATH, "/vendor/etc/selinux/vendor_file_contexts" },
	{ SELABEL_OPT_PATH, "/vendor_file_contexts" },
	// TODO: remove nonplat* when no need to retain backward compatibility.
	{ SELABEL_OPT_PATH, "/vendor/etc/selinux/nonplat_file_contexts" },
	{ SELABEL_OPT_PATH, "/nonplat_file_contexts" }
};
static const struct selinux_opt seopts_file_odm[] = {
	{ SELABEL_OPT_PATH, "/odm/etc/selinux/odm_file_contexts" },
	{ SELABEL_OPT_PATH, "/odm_file_contexts" }
};

static struct selabel_handle* selinux_mntl_file_context_handle(void)
{
	unsigned int i, size = 0;
	struct selabel_handle *se_handle;
	struct selinux_opt file_context[4];

	for (i = 0; i < ARRAY_SIZE(seopts_file_plat); i++) {
		if (access(seopts_file_plat[i].value, R_OK) != -1) {
			file_context[size++] = seopts_file_plat[i];
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(seopts_file_vendor); i++) {
		if (access(seopts_file_vendor[i].value, R_OK) != -1) {
			file_context[size++] = seopts_file_vendor[i];
			break;
		}
	}

	for (i = 0; i < ARRAY_SIZE(seopts_file_odm); i++) {
		if (access(seopts_file_odm[i].value, R_OK) != -1) {
			file_context[size++] = seopts_file_odm[i];
			break;
		}
	}

	file_context[size].type = SELABEL_OPT_BASEONLY;
	file_context[size].value = (char *)1;

	se_handle = selabel_open(SELABEL_CTX_FILE, file_context, ARRAY_SIZE(file_context));
	if (!se_handle) {
		ERROR("%s: Error getting file context handle (%s)\n",
			__FUNCTION__, strerror(errno));
		return NULL;
	}

	return se_handle;
}

//#ifdef MNTL_SUPPORT
int create_mntl_link()
{
	FILE *fp = NULL;
	GuidPartitionTableHeader_t pgpt_header;
	GuidPartitionEntry_t *pe;
	int sector_size = 0;
	char *part_entry = NULL;
	int fd = 0;
	int timeout = 10;
	int ret = 0;
	int part_idx = 0;
	unsigned int i = 0;
	uint64_t part_end_addr = 0;

	// get sector size
	if (mt_get_sector_size(&sector_size) != 0) {
		ERROR("Get sector size fail!\n");
		return 1;
	}

	// read partition entry
	if (mt_gpt_get_part_entry(&pgpt_header, &part_entry, sector_size) > 0) {
		ERROR("Get partition entry fail!\n");
		return 1;
	}

	pe = (GuidPartitionEntry_t *)part_entry;

	mkdir("/dev/block/platform", FOLDER_DEFAULT_MODE);
	mkdir("/dev/block/platform/bootdevice", FOLDER_DEFAULT_MODE);
	chmod("/dev/block/platform/bootdevice", FOLDER_DEFAULT_MODE);
	mkdir("/dev/block/platform/bootdevice/by-name", FOLDER_DEFAULT_MODE);
	chmod("/dev/block/platform/bootdevice/by-name", FOLDER_DEFAULT_MODE);
	for (i = 0; i < pgpt_header.NumberOfPartitionEntries; i++, pe++) {
		unsigned int j;
		char name[37];
		char target[255], source[255];
		char *secontext = NULL;
		mode_t mode;
		char const *system = "system";

		for (j = 0; j < 72 / sizeof(efi_char16_t); j++) {
			name[j] = (uint16_t)pe->PartitionName[j];
		}
		name[j] = 0;

		if (strcmp(name, "mntlblk_d1") == 0) {
			sprintf(target, "/dev/block/platform/bootdevice/by-name/system");
			mode = get_mode(system);
		} else if (strcmp(name, "mntlblk_d2") == 0) {
			sprintf(target, "/dev/block/platform/bootdevice/by-name/vendor");
			mode = get_mode(system);
		} else {
			sprintf(target, "/dev/block/platform/bootdevice/by-name/%s", name);
			mode = get_mode(name);
		}
		sprintf(source, "/dev/block/mntlblk_d%d", i+1);
		ret = symlink(source, target);
		if (ret)
			ERROR("symlink %s to %s, err %d\n", target, source, ret);
		chown(target, -1, system_gid);
		chown(source, -1, system_gid);
		if (mode) {
			chmod(target, mode);
			chmod(source, mode);
		}

		if (sehandle) {
			selabel_lookup(sehandle, &secontext, target, mode);

			printf("target=%s source=%s context=%s\n", target, source, secontext);
			ret = lsetfilecon(target, secontext);
			if (ret)
				ERROR("target=%s context=%s errno=%d\n", target, secontext, errno);
			ret = lsetfilecon(source, secontext);
			if (ret)
				ERROR("source=%s context=%s errno=%d\n", source, secontext, errno);
		}
	}

	return 0;
}
//#endif

int main()
{

	system_gid = get_system_gid();
	//sehandle = selinux_android_file_context_handle();
	sehandle = selinux_mntl_file_context_handle();
	//selinux_android_set_sehandle(sehandle);

	create_mntl_link();
	wait_mtd_device();
	create_mtd_link();

	exit(0);
}
