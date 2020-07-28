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
//#include <fs_mgr.h>
/*libfs_mgr:vendor can't use,libfstab:vendor can use */
#include <fstab.h>
#include <cutils/properties.h>
#include <log/log.h>

#include "emmc.h"
#define TAG "[emmc] "

#define OTP_MOUNT_POINT     "/otp"
#define FSTAB_PREFIX        "/data/nvram/fstab."
#define EMMC_BLOCK_SIZE     512
#define EMMC_SIZE_PATH      "/sys/block/mmcblk0/size"
#define WP_GRP_SIZE_PATH    "/sys/class/mmc_host/mmc0/mmc0:0001/wp_grp_size"
#define OTP_BLOCK_PATH      "/dev/block/platform/bootdevice/by-name/otp"

static unsigned long long get_wp_grp_size(char* wp_grp_size_path)
{
	FILE *fp;
	unsigned long long size = 0;
	char buffer[256] = {0};

	fp = fopen(wp_grp_size_path, "r");
	if (!fp) {
		OTP_LOG(TAG"Open file failed\n");
		return 0;
	}
	memset(buffer, 0, 256);
	if (fgets(buffer, 256, fp))
		size = strtoul(buffer, NULL, 0) / 512;
	fclose(fp);
	return size;
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
		OTP_LOG(TAG"failed to read %s\n", fstab_filename);
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
		OTP_LOG(TAG"failed to get entry for %s\n", mount_point);
		return NULL;
	}
	source = rec->blk_device;
	return source;
}

static void free_fstab(struct fstab *sys_fstab)
{
	fs_mgr_free_fstab(sys_fstab);
}

static int emmc_read(struct otp *otp_device, char *buffer,
		unsigned long long start, unsigned long long size)
{
	int fd = 0;
	int ret = 0;

	fd = open(otp_device->otp_blk_path, O_RDONLY);
	if (fd < 0) {
		perror(TAG"failed to open\n");
		return -1;
	}
	ret = lseek(fd, start, SEEK_SET);
	OTP_LOG(TAG"set offset is %x\n", ret);
	ret = read(fd, buffer, size);
	if (ret == -1) {
		OTP_LOG(TAG"otp read data error\n");
		close(fd);
		return -1;
	}
	close(fd);

	return 0;

}

static unsigned int emmc_write(struct otp *otp_device, char *buffer,
		unsigned long long start, unsigned long long size)
{
	int fd = 0;
	unsigned int ret = 0;

	fd = open(otp_device->otp_blk_path, O_WRONLY);
	if (fd < 0) {
		OTP_LOG(TAG"Open otp block device failed\n");
		return errno;
	}
	if (-1 == lseek(fd, start, SEEK_SET)) {
		close(fd);
		OTP_LOG(TAG"otp lseek error\n");
		return errno;
	}
	ret = write(fd, buffer, size);
	if (ret != size) {
		OTP_LOG(TAG"otp write data error, write size\n");
		close(fd);
		return errno;
	}
	close(fd);

	/* success return 0 as ufs did */
	return 0;

}
#endif

int emmc_block_read(struct otp *otp_device, char *buffer,
		unsigned long long blk_offset, unsigned long long blknr)
{
	const char *path = OTP_BLOCK_PATH;
	int fd = 0;
	int ret = 0;
	unsigned int blksz = otp_device->blk_sz;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open\n");
		return -1;
	}

	ret = lseek(fd, blk_offset * blksz, SEEK_SET);
	//OTP_LOG(TAG"read seek to offset %llu, ret %d\n", blk_offset * blksz, ret);
	if (ret  < 0) {
		close(fd);
		OTP_LOG(TAG"otp lseek error\n");
		return errno;
	}

	ret = read(fd, buffer, blknr * blksz);
	if (ret < 0) {
		OTP_LOG(TAG"otp read data error\n");
		close(fd);
		return errno;
	}

	/*
	OTP_LOG(TAG"emmc_block_read: 0x%.2x 0x%.2x 0x%.2x 0x%.2x  0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
		buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
	*/

	close(fd);

	return 0;
}

static int emmc_block_write(struct otp *otp_device, char *buffer,
		unsigned long long blk_offset, unsigned long long blknr)
{
	const char *path = OTP_BLOCK_PATH;
	int fd = 0;
	int ret = 0;
	unsigned int blksz = otp_device->blk_sz;

	fd = open(path, O_WRONLY);
	if (fd < 0) {
		OTP_LOG(TAG"Open otp block device failed\n");
		return errno;
	}

	//OTP_LOG(TAG"write seek to offset %llx, ret %d\n", blk_offset * blksz, ret);
	ret = lseek(fd, blk_offset * blksz, SEEK_SET);
	if (ret  < 0) {
		close(fd);
		OTP_LOG(TAG"otp lseek error\n");
		return errno;
	}

	ret = write(fd, buffer, blknr * blksz);
	if (ret != (int)blknr * 512) {
		OTP_LOG(TAG"otp write data error, write size\n");
		close(fd);
		return errno;
	}

	close(fd);

	/* success return 0 as ufs did */
	return 0;
}



static int emmc_set_usr_wp(unsigned char wp)
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}

	/* set for needing */
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SWITCH;
	idata.arg = (MMC_SWITCH_MODE_SET_BITS << 24) |
		(EXT_CSD_USR_WP << 16) |
		(wp << 8) |
		EXT_CSD_CMD_SET_NORMAL;
	idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	idata.write_flag = 1;
	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	close(fd);
	return ret;
}

static int emmc_clr_usr_wp()
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}

	/*clear probable previous permanent setting of EXT_CSD_USR_WP */
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SWITCH;
	idata.arg = (MMC_SWITCH_MODE_CLEAR_BITS << 24) |
		(EXT_CSD_USR_WP << 16) |
		((EXT_CSD_USR_WP_EN_PERM_WP | EXT_CSD_USR_WP_EN_PWR_WP) << 8) |
		EXT_CSD_CMD_SET_NORMAL;
	idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	idata.write_flag = 1;
	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	close(fd);
	return ret;
}

/* Usually, the blknr should set to 0, Per the Spec, this command sets
 * the write protection bit of the addressed group. in general, the write
 * protect group is very big(eg, 512KB or 4MB), set any block will do write
 * protect for the group totally.
 */
static int emmc_set_write_protect(unsigned int blknr)
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SET_WRITE_PROT;
	idata.arg = blknr;
	idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	idata.write_flag = 1;
	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	close(fd);
	return ret;
}

static int emmc_clear_write_protect(unsigned int blknr)
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_CLR_WRITE_PROT;
	idata.arg = blknr;
	idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	idata.write_flag = 1;
	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	close(fd);
	return ret;
}
static int emmc_get_write_protect_status(unsigned int blknr, unsigned int *wp_status)
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret = 0;
	unsigned int status;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SEND_WRITE_PROT;
	idata.arg = blknr;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blocks = 1;
	idata.blksz = 4;
	idata.write_flag = 0;
	mmc_ioc_cmd_set_data(idata, &status);
	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	if (ret) {
		OTP_LOG(TAG"ioctl MMC_SEND_WRITE_PROT error %d\n", ret);
		close(fd);
		return -1;
	}
	*wp_status = (status >> 24) & 0x01;

	close(fd);
	return ret;
}

static int emmc_get_write_protect_type(unsigned int blknr, unsigned int *wp_type)
{
	const char *path = OTP_BLOCK_PATH;
	struct mmc_ioc_cmd idata;
	int fd;
	int ret = 0;
	unsigned int type[2];
	unsigned int type_for_blknr = 0;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		OTP_LOG(TAG"failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SEND_WRITE_PROT_TYPE;
	idata.arg = blknr;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blocks = 1;
	idata.blksz = 8;
	idata.write_flag = 0;
	mmc_ioc_cmd_set_data(idata, type);
	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	if (ret) {
		OTP_LOG(TAG"ioctl MMC_SEND_WRITE_PROT_TYPE error %d\n", ret);
		close(fd);
		return -1;
	}

	type_for_blknr = (type[1] >> 24) & 0x03;

	if (type_for_blknr == 0x1)
		*wp_type = WP_TEMPORARY;
	else if (type_for_blknr == 0x2)
		*wp_type = WP_POWER_ON;
	else if (type_for_blknr == 0x3)
		*wp_type = WP_PERMANENT;
	else
		*wp_type = WP_DISABLE;

	close(fd);
	return ret;
}

int emmc_otp_lock(struct otp *otp_device, unsigned int type, unsigned long long start_block,
		unsigned long long group_count, unsigned long long wp_grp_size)
{
	unsigned int i;
	int ret = 0;

	if (!otp_device) {
		OTP_LOG("NULL parameter!\n");
		return -EINVAL;
	}

	ret = emmc_clr_usr_wp();
	if (ret) {
		OTP_LOG(TAG"%s error\n", __func__);
		return -EINVAL;
	}

	switch (type) {
	case WP_TEMPORARY:
		OTP_LOG(TAG"Type is WP_TEMPORARY\n");
		break;
	case WP_POWER_ON:
		OTP_LOG(TAG"Type is WP_POWER_ON\n");
		ret = emmc_set_usr_wp(EXT_CSD_USR_WP_EN_PWR_WP);
		if (ret) {
			OTP_LOG(TAG"%s error\n", __func__);
			return -EINVAL;
		}
		break;
	case WP_PERMANENT:
		OTP_LOG(TAG"Type is WP_PERMANENT\n");
#ifdef MTK_EMMC_SUPPORT_OTP_FOR_CUSTOMER
		ret = emmc_set_usr_wp(EXT_CSD_USR_WP_EN_PERM_WP);
#endif
		if (ret) {
			OTP_LOG(TAG"%s error\n", __func__);
			return -EINVAL;
		}
		break;
	default:
		OTP_LOG(TAG"wp type is invalid\n");
 		break;
	}

	for (i = 0; i < group_count; i++) {
		ret = emmc_set_write_protect(start_block + (i * wp_grp_size));
		OTP_LOG(TAG"set WP at %llx\n", start_block + (i * wp_grp_size));
		if (ret) {
			OTP_LOG(TAG"%s error\n", __func__);
			return -EINVAL;
		}
	}

	ret = emmc_clr_usr_wp();
	if (ret) {
		OTP_LOG(TAG"%s error\n", __func__);
		return -EINVAL;
	}

	return 0;
}
int emmc_otp_unlock(struct otp *otp_device, unsigned long long start_block,
		unsigned long long group_count, unsigned long long wp_grp_size)
{
	unsigned int i;
	int ret = 0;

	if (!otp_device) {
		OTP_LOG("NULL parameter!\n");
		return -EINVAL;
	}

	for (i = 0; i < group_count; i++) {
		ret = emmc_clear_write_protect(start_block + (i * wp_grp_size));
		OTP_LOG(TAG"clear WP at %llx\n", start_block + (i * wp_grp_size));
		if (ret) {
			OTP_LOG(TAG"%s error\n", __func__);
			return -EINVAL;
		}
	}
	return 0;
}
int emmc_otp_get_status(struct otp *otp_device, unsigned long long start_block,
		unsigned long long group_count,
		unsigned int *status, unsigned int *type)
{
	unsigned int wp_status = 0;
	unsigned int wp_type[2] = {0, 0};
	int ret = 0;

	if (!otp_device) {
		OTP_LOG("NULL parameter!\n");
		return -EINVAL;
	}

	if (group_count > 32) {
		OTP_LOG(TAG"%s group_count must < 32\n", __func__);
		return -EINVAL;
	}
	ret = emmc_get_write_protect_status(start_block, &wp_status);

	if (!ret)
		OTP_LOG(TAG"Address 0x%llx WP status is 0x%x\n", start_block, wp_status);
	else
		return -EINVAL;
	ret = emmc_get_write_protect_type(start_block, wp_type);
	if (!ret)
		OTP_LOG(TAG"Address 0x%llx WP type is 0x%.8x %.8x\n", start_block,
				wp_type[0], wp_type[1]);
	else
		return -EINVAL;

	*status = wp_status;
	*type = wp_type[0];

#ifndef MTK_EMMC_SUPPORT_OTP_FOR_CUSTOMER
	/* FOR OTP USER to test lock fake WP_PERMANENT, and we no need report
		correct lock status for WP_TEMPORARY */
	if (*type == WP_TEMPORARY)
		*type = WP_PERMANENT;
#endif
	return 0;
}

int emmc_storage_init(struct otp *otp_device)
{
	FILE *fp = NULL;
	unsigned long long otp_start = 0;
	unsigned long long otp_size = 0;
	unsigned long long wp_grp_size = 0;
	char buffer[256] = {0};
	char *otp_start_path = NULL;
	char *otp_size_path = NULL;
	char otp_block_name[129] = {0};
	char *path_buffer = NULL;
	int ret = 0;

	otp_device->block_read = emmc_block_read;
	otp_device->block_write = emmc_block_write;
	otp_device->lock = emmc_otp_lock;
	otp_device->unlock = emmc_otp_unlock;
	otp_device->get_status = emmc_otp_get_status;
#if 0
	otp_device->read = emmc_read;
	otp_device->write = emmc_write;
#endif

	/* otp_blk_path = /dev/block/mmcblk0pN */
	ret = readlink(OTP_BLOCK_PATH, otp_device->otp_blk_path, PATH_MAX);
	if (ret < 0) {
		OTP_LOG(TAG"%s does not exist\n", otp_device->otp_blk_path);
		ret = -ENOENT;
		goto out;
	}

	wp_grp_size = get_wp_grp_size((char*) WP_GRP_SIZE_PATH);
	if (!wp_grp_size) {
		OTP_LOG(TAG"get_wp_grp_size failed\n");
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

	/* retrieve "partition id (mmcblk0pN)" of otp partition */
	sscanf(otp_device->otp_blk_path, "/dev/block/%s", otp_block_name);

	/* finish sysfs path for otp partition */
	snprintf(path_buffer, PATH_MAX, "/sys/block/mmcblk0/%s", otp_block_name);

	otp_start_path = (char *) calloc(1, strlen(path_buffer) + 8);
	otp_size_path = (char *) calloc(1, strlen(path_buffer) + 8);
	if (!otp_start_path || !otp_size_path) {
		OTP_LOG(TAG"fail to allocate memory\n");
		ret = -EINVAL;
		goto out;
	}
	strncpy(otp_start_path, path_buffer, (strlen(path_buffer) + 1));
	strncpy(otp_size_path, path_buffer, (strlen(path_buffer) + 1));
	strncat(otp_start_path, "/start", strlen("/start"));
	strncat(otp_size_path, "/size", strlen("/size"));

	OTP_LOG(TAG"Device info: block device path: %s. sysfs start path: %s, %d\n",
		   otp_device->otp_blk_path,
 		   otp_start_path, PATH_MAX);

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

	otp_device->wp_grp_size = wp_grp_size;
	otp_device->otp_start = 0;
	otp_device->otp_size = otp_size;
	otp_device->partition_start = 0;

	if (otp_start % wp_grp_size) {
		otp_device->otp_start = (wp_grp_size - otp_start % wp_grp_size);
		if (otp_device->otp_start > (otp_start + otp_size)) {
			OTP_LOG(TAG"Invalid otp partition, otp_start: 0x%llx, size: 0x%llx\n",
				otp_start, otp_size);
			ret = -EINVAL;
			goto out;
		}
	}

	/* transfer from block to byte */
	otp_device->blk_sz = EMMC_BLOCK_SIZE;

	otp_device->wp_grp_size *= otp_device->blk_sz;
	otp_device->otp_start *= otp_device->blk_sz;
	otp_device->otp_size *= otp_device->blk_sz;

	OTP_LOG(TAG"wp_grp_size = 0x%llx, otp_start = 0x%llx, otp_size = 0x%llx\n",
		otp_device->wp_grp_size,
		otp_device->otp_start,
		otp_device->otp_size);

	ret = 0;

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

