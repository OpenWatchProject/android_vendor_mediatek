#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/major.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <fstab.h>
#include <cutils/properties.h>
#include <log/log.h>

#include "ufs.h"

#define TAG                      "[UFS] "

#define OTP_MOUNT_POINT          "/otp"
#define FSTAB_PREFIX             "/data/nvram/fstab."
#define UFS_BLOCK_SIZE           (4 * 1024)         /* 4 KB each block */
#define UFS_OTP_REGION_SIZE_BYTE (1 * 1024 * 1024)	/* 1 MB each region, total 4 regions */
#define UFS_SIZE_PATH            "/sys/block/sdc/size"
#define OTP_PARTITION_BLOCK_PATH "/dev/block/platform/bootdevice/by-name/otp"

/* Linux sysfs file "start" and "size" always use 512 bytes as unit size */
#define DEVICE_BLOCK_UNIT_SIZE (512)

static char wp_buffer[UFS_BLOCK_SIZE] = {0};

/* unit: bytes */
static unsigned long long ufs_get_region_size(void)
{
	/*
	 * UFS does not have concept of WP group,
	 * so we define "region size" ourselves instead.
	 */
	return UFS_OTP_REGION_SIZE_BYTE;
}

#if 0
static struct fstab* read_fstab(void)
{
	char fstab_filename[PROPERTY_VALUE_MAX + sizeof(FSTAB_PREFIX)];
	char propbuf[PROPERTY_VALUE_MAX];
	struct fstab *sys_fstab = NULL;

	property_get("ro.hardware", propbuf, "");
	snprintf(fstab_filename, sizeof(fstab_filename), FSTAB_PREFIX"%s", propbuf);
	sys_fstab = fs_mgr_read_fstab(fstab_filename);
	if (!sys_fstab) {
		OTP_LOG(TAG"failed to read %s \n", fstab_filename);
		return NULL;
	}

	return sys_fstab;
}

static char *get_device_path(struct fstab *sys_fstab, const char *mount_point)
{
	struct fstab_rec *rec = NULL;
	char *source = NULL;

	rec = fs_mgr_get_entry_for_mount_point(sys_fstab, mount_point);
	if (!rec) {
		OTP_LOG(TAG"failed to get entry for %s \n", mount_point);
		return NULL;
	}
	source = rec->blk_device;
	return source;
}

static void free_fstab(struct fstab *sys_fstab)
{
	fs_mgr_free_fstab(sys_fstab);
}
#endif

int ufs_block_read(struct otp *otp_device, char *buffer,
		unsigned long long start_blk, unsigned long long blknr)
{
	const char *path = OTP_PARTITION_BLOCK_PATH;
	int fd;
	int ret;

	OTP_LOG(TAG"%s, start_blk %lld, blknr %lld, buf 0x%llx\n", __func__, start_blk, blknr,
		(unsigned long long)buffer);

	if (!buffer || !otp_device) {
		OTP_LOG("%s: NULL parameter!\n", __func__);
		return -EINVAL;
	}

	fd = open(path, O_RDONLY);

	if (fd < 0) {
		OTP_LOG("failed to open %s\n", path);
		return errno;
	}

	if (start_blk) {
		if (lseek64(fd, start_blk * UFS_BLOCK_SIZE, SEEK_SET) < 0) {
			ret = errno;
			OTP_LOG("failed to seek %s to ofs %lld, ret %d\n", path, start_blk * UFS_BLOCK_SIZE, ret);
			goto out_close_file;
		}
	}

	ret = read(fd, buffer, blknr * UFS_BLOCK_SIZE);

	if (ret < 0) {
		OTP_LOG("failed to read %s, ret %d\n", path, ret);
		goto out_close_file;
	} else
		ret = 0;

out_close_file:
	close(fd);

	return ret;
}

static int ufs_block_write(struct otp *otp_device, char *buffer,
		unsigned long long start_blk, unsigned long long blknr)
{
	const char *path = OTP_PARTITION_BLOCK_PATH;
	int fd;
	int ret;

	OTP_LOG(TAG"%s, start_blk %lld, blknr %lld, buf 0x%llx\n", __func__, start_blk, blknr,
		(unsigned long long)buffer);

	if (!buffer || !otp_device) {
		OTP_LOG("%s: NULL parameter!\n", __func__);
		return -EINVAL;
	}

	fd = open(path, O_RDWR);

	if (fd < 0) {
		OTP_LOG("failed to open %s\n", path);
		return errno;
	}

	if (start_blk) {
		if (lseek64(fd, start_blk * UFS_BLOCK_SIZE, SEEK_SET) < 0) {
			ret = errno;
			OTP_LOG("failed to seek %s to ofs %lld, ret %d\n", path, start_blk * UFS_BLOCK_SIZE, ret);
			goto out_close_file;
		}
	}

	ret = write(fd, buffer, blknr * UFS_BLOCK_SIZE);

	if (ret < 0) {
		OTP_LOG("failed to read %s, ret %d\n", path, ret);
		goto out_close_file;
	} else
		ret = 0;

out_close_file:
	close(fd);

	return ret;
}

static int ufs_set_wp(unsigned int type, unsigned long long start_block)
{
	const char *path = OTP_PARTITION_BLOCK_PATH;
	int fd;
	int ret;
	unsigned long long mark_blk;

	fd = open(path, O_RDWR);

	if (fd < 0) {
		OTP_LOG("failed to open %s\n", path);
		return errno;
	}

	if (type == WP_PERMANENT) {

		mark_blk = (UFS_OTP_REGION_SIZE_BYTE /UFS_BLOCK_SIZE) * MAX_USER_NUM;
		if (lseek64(fd, mark_blk * UFS_BLOCK_SIZE, SEEK_SET) < 0) {
			ret = errno;
			OTP_LOG("failed to seek %s to ofs %lld, ret %d\n", path, mark_blk * UFS_BLOCK_SIZE, ret);
			goto out_close_file;
		}

		OTP_LOG("%s start_block: %llu, mark_blk: %llu\n", __func__, start_block, mark_blk);

		memset(wp_buffer, 0, UFS_BLOCK_SIZE);

		memcpy(wp_buffer, "LOCK", 4);

		ret = write(fd, wp_buffer, UFS_BLOCK_SIZE);

		if (ret < 0) {
			OTP_LOG("failed to read %s, ret %d\n", path, ret);
			goto out_close_file;
		} else
			ret = 0;
	} else {
		ret = -EINVAL;;
	}

out_close_file:
	close(fd);

	return ret;
}

int ufs_otp_lock(struct otp *otp_device, unsigned int type, unsigned long long start_block,
		unsigned long long group_count, unsigned long long wp_grp_size)
{
	int ret = 0;

	if (!otp_device) {
		OTP_LOG("%s: NULL parameter!\n", __func__);
		return -EINVAL;
	}

	OTP_LOG(TAG"ufs_otp_lock: group_count %lld, wp_grp_size %lld\n",
		group_count, wp_grp_size);

	switch (type) {
	case WP_TEMPORARY:
		OTP_LOG(TAG"Tpye WP_TEMPORARY is not supported!\n");
		break;
	case WP_POWER_ON:
		OTP_LOG(TAG"Tpye WP_POWER_ON is not supported!\n");
		break;
	case WP_PERMANENT:
		OTP_LOG(TAG"Tpye is WP_PERMANENT\n");
		ret = ufs_set_wp(type, start_block);
		if (ret) {
			OTP_LOG(TAG"%s error\n", __func__);
			return -EINVAL;
		}
		break;
	default:
		OTP_LOG(TAG"wp type is invalid\n");
		break;
	}

	return 0;
}
int ufs_otp_unlock(struct otp *otp_device, unsigned long long start_block,
		unsigned long long group_count, unsigned long long wp_grp_size)
{
	if (!otp_device) {
		OTP_LOG("%s: NULL parameter!\n", __func__);
		return -EINVAL;
	}

	OTP_LOG(TAG"ufs_otp_unlock: start_block %lld group_count %lld, wp_grp_size %lld\n",
		start_block, group_count, wp_grp_size);

	OTP_LOG("OTP Unlock is not supported!\n");

	return 0;
}
int ufs_otp_get_status(struct otp *otp_device, unsigned long long start_block,
		unsigned long long group_count,
		unsigned int *status, unsigned int *type)
{
	if (!otp_device) {
		OTP_LOG("%s: NULL parameter!\n", __func__);
		return -EINVAL;
	}

	if (!status) {
		OTP_LOG("%s: NULL status!\n", __func__);
		return -EINVAL;
	}

	if (!type) {
		OTP_LOG("%s: NULL type!\n", __func__);
		return -EINVAL;
	}

	OTP_LOG(TAG"ufs_otp_get_status: start_block %lld, group_count %lld\n",
		start_block, group_count);

	OTP_LOG("OTP Get Status is not supported!\n");

	return 0;
}

int ufs_storage_init(struct otp *otp_device)
{
	FILE *fp = NULL;
	unsigned long long otp_start = 0;
	unsigned long long otp_size = 0;
	char buffer[256] = {0};
	char *otp_start_path = NULL;
	char *otp_size_path = NULL;
	char otp_block_name[129] = {0};
	char *path_buffer = NULL;
	int region_cnt;
	int err, ret = 0;

	otp_device->block_read = ufs_block_read;
	otp_device->block_write = ufs_block_write;
	otp_device->lock = ufs_otp_lock;
	otp_device->unlock = ufs_otp_unlock;
	otp_device->get_status = ufs_otp_get_status;
	otp_device->read = NULL;
	otp_device->write = NULL;

	/* otp_blk_path = /dev/block/sdcN */
	err = readlink(OTP_PARTITION_BLOCK_PATH, otp_device->otp_blk_path, PATH_MAX);
	if (err < 0) {
		OTP_LOG("failed to get link %s\n", OTP_PARTITION_BLOCK_PATH);
		ret = errno;
		goto out;
	}

	otp_device->wp_grp_size = ufs_get_region_size();

	if (!otp_device->wp_grp_size) {
		OTP_LOG(TAG"ufs_get_region_size failed\n");
		ret = -EINVAL;
		goto out;
	}

	path_buffer = (char *)malloc(PATH_MAX);
	if (!path_buffer) {
		OTP_LOG(TAG"fail to alloc path_buffer, size = %d\n", PATH_MAX);
		ret = -EINVAL;
		goto out;
	}

	memset(path_buffer, 0, PATH_MAX);

	/* retrieve "partition id (sdcN)" of otp partition */
	sscanf(otp_device->otp_blk_path, "/dev/block/%s", otp_block_name);

	/* finish sysfs path for otp partition */
	snprintf(path_buffer, PATH_MAX, "/sys/block/sdc/%s", otp_block_name);

	otp_start_path = (char *)calloc(1, strlen(path_buffer) + 8);
	otp_size_path = (char *)calloc(1, strlen(path_buffer) + 8);
	if (!otp_start_path || !otp_size_path) {
		OTP_LOG(TAG"fail to allocate memory\n");
		ret = -EINVAL;
		goto out;
	}
	strncpy(otp_start_path, path_buffer, (strlen(path_buffer) + 1));
	strncpy(otp_size_path, path_buffer, (strlen(path_buffer) + 1));
	strncat(otp_start_path, "/start", strlen("/start"));
	strncat(otp_size_path, "/size", strlen("/size"));

	OTP_LOG(TAG"Device info: block device path: %s. sysfs start path: %s\n",
		   otp_device->otp_blk_path,
		   otp_start_path);

	memset(buffer, 0, sizeof(buffer));
	fp = fopen(otp_start_path, "r");
	if (!fp) {
		OTP_LOG(TAG"Open file failed\n");
		ret = errno;
		goto out;
	}
	if (fgets(buffer, sizeof(buffer), fp)) {
		otp_start = strtoul(buffer, NULL, 0);
	} else {
		OTP_LOG(TAG"get otp start fialed \n");
		ret = errno;
		goto out;
	}
	fclose(fp);

	memset(buffer, 0, sizeof(buffer));
	fp = fopen(otp_size_path, "r");
	if (!fp) {
		OTP_LOG(TAG"Open file failed\n");
		ret = errno;
		goto out;
	}
	if (fgets(buffer, sizeof(buffer), fp)) {
		otp_size = strtoul(buffer, NULL, 0);
	} else {
		OTP_LOG(TAG"get otp size fialed \n");
		ret = errno;
		goto out;
	}

	/* translate to byte for later calculation */
	otp_device->blk_sz = UFS_BLOCK_SIZE;

	/*
	 * we will access otp partition block device directly,
	 * thus otp_start shall be initialized as 0.
	 */

	/* aligned start, for ->block_read and ->block_write only */
	otp_device->otp_start = 0;

	/* real start, for ->read and ->write only */
	otp_device->partition_start = 0;

	otp_start = otp_start * DEVICE_BLOCK_UNIT_SIZE;

	OTP_LOG(TAG"init: otp_start (original) %lld (blk %lld)\n", otp_start,
				otp_start / otp_device->blk_sz);

	otp_size = otp_size * DEVICE_BLOCK_UNIT_SIZE;

	otp_device->otp_size = otp_size;

	OTP_LOG(TAG"init: otp_size (original) %lld\n", otp_device->otp_size);

	/*
	 * make sure region start is UFS_BLOCK_SIZE aligned.
	 *
	 * remove me if alignment is promised by partiton table.
	 */

	if (otp_start % UFS_BLOCK_SIZE) {

		otp_device->otp_start += (UFS_BLOCK_SIZE - otp_start % UFS_BLOCK_SIZE);

		if (otp_device->otp_start > otp_size) {
			OTP_LOG(TAG"Invalid otp partition, otp_start: 0x%llx, size: 0x%llx\n",
					otp_start, otp_size);
			ret = -ENOSPC;
			goto out;
		} else
			otp_device->otp_size -= (UFS_BLOCK_SIZE - otp_start % UFS_BLOCK_SIZE);
	}

	/* pre-calculate number of regions */
	region_cnt = otp_device->otp_size / otp_device->wp_grp_size;

	/* we need some space for UFS OTP locking mark, check if any space left */
	if (otp_device->otp_size % otp_device->wp_grp_size == 0) {

		/* Ohoh, no space left for us. Reserve 1 region for us */

		if (region_cnt <= 1) {
			OTP_LOG(TAG"UFS otp partition size is too small \n");
			OTP_LOG(TAG"Please make sure size at least %lld bytes \n", otp_device->wp_grp_size + 4);
			ret = -ENOSPC;
			goto out;
		}

		region_cnt--;
		otp_device->otp_size = region_cnt * otp_device->wp_grp_size;
	}

	OTP_LOG(TAG"init: blk_sz %lld\n", otp_device->blk_sz);
	OTP_LOG(TAG"init: otp_start %lld (blk %lld)\n", otp_device->otp_start,
	            otp_device->otp_start / otp_device->blk_sz);
	OTP_LOG(TAG"init: otp_size %lld (%lld blks)\n", otp_device->otp_size,
	            otp_device->otp_size / otp_device->blk_sz);
	OTP_LOG(TAG"init: wp_grp_size %lld (%lld blks)\n", otp_device->wp_grp_size,
	            otp_device->wp_grp_size / otp_device->blk_sz);
	OTP_LOG(TAG"init: region_cnt %d\n", region_cnt);

out:
	if (fp)
		fclose(fp);
	if (path_buffer)
		free(path_buffer);
	if (otp_start_path)
		free(otp_start_path);
	if (otp_size_path)
		free(otp_size_path);
	return ret;
}

