#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <linux/mmc/ioctl.h>
#include <linux/major.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "emmc_otp.h"

unsigned long otp_start, otp_size;

#define MAX_LINE_LEN    256
static unsigned long get_emmc_size(void)
{
	char buffer[MAX_LINE_LEN] = {0};
	FILE *fp;
	unsigned long size = 0;
#define EMMC_SIZE_PATH "/sys/block/mmcblk0/size"

	fp = fopen(EMMC_SIZE_PATH, "r");
	if (!fp) {
		printf("failed to open %s, errno: %d\n", EMMC_SIZE_PATH, errno);
		return 0;
	}
	if (fgets(buffer, MAX_LINE_LEN, fp))
		size = strtoul(buffer, NULL, 0);
	fclose(fp);

	return size << 9;
}

static unsigned long get_wp_grp_size(void)
{
	char buffer[MAX_LINE_LEN] = {0};
	FILE *fp;
	unsigned long size = 0;
#define WP_GRP_SIZE_PATH "/sys/class/mmc_host/mmc0/mmc0:0001/wp_grp_size"

	fp = fopen(WP_GRP_SIZE_PATH, "r");
	if (!fp) {
		printf("failed to open %s, errno: %d", WP_GRP_SIZE_PATH, errno);
		return 0;
	}
	if (fgets(buffer, MAX_LINE_LEN, fp))
		size = strtoul(buffer, NULL, 0);
	fclose(fp);

	return size;
}

#ifdef MTK_GPT_SCHEME_SUPPORT
static int get_otp_block_name(char *buffer, unsigned int size)
{
	/* in general, the otp path is like below */
	const char *otp_path = "/dev/block/platform/soc/11230000.mmc/by-name/otp";
	ssize_t r;

	if (!buffer) {
		perror("input buffer is NULL!\n");
		return -EINVAL;
	}
	r = readlink(otp_path, buffer, size);
	if (r == -1) {
		printf("faild to get link of %s\n", otp_path);
		return r;
	}

	return r;
}

static int get_otp_attr(unsigned long *otp_start, unsigned long *otp_size)
{
	char buffer[MAX_LINE_LEN] = {0};
	char otp_start_path[128] = {0};
	char otp_size_path[128] = {0};
	char otp_block_name[16] = {0};
	FILE *fp;
	unsigned long emmc_size = 0;
	unsigned long wp_grp_size = 0;
	unsigned long size = 0, start = 0;
	int ret;

	ret = get_otp_block_name(buffer, MAX_LINE_LEN);
	if (ret < 0)
		return ret;

	sscanf(buffer, "/dev/block/%s", otp_block_name);
	printf("otp_block_name: %s\n", otp_block_name);
	memset(buffer, 0, MAX_LINE_LEN);
	snprintf(buffer, MAX_LINE_LEN, "/sys/block/mmcblk0/%s", otp_block_name);
	printf("buffer: %s\n", buffer);
	if (strlen(buffer) >= 120) {
		printf("otp block path is too long");
		return -EINVAL;
	}

	strncpy(otp_start_path, buffer, 128);
	strncpy(otp_size_path, buffer, 128);
	strcat(otp_start_path, "/start");
	strcat(otp_size_path, "/size");
	printf("otp_start_path: %s\n", otp_start_path);
	printf("otp_size_path: %s\n", otp_size_path);

	memset(buffer, 0, sizeof(buffer));
	fp = fopen(otp_start_path, "r");
	if (!fp) {
		printf("failed to open %s, errno: %d\n", otp_start_path, errno);
		return errno;
	}
	if (fgets(buffer, sizeof(buffer), fp)) {
		start = strtoul(buffer, NULL, 0);	
		printf("start: %ld\n", start);
	}
	fclose(fp);

	memset(buffer, 0, sizeof(buffer));
	fp = fopen(otp_size_path, "r");
	if (!fp) {
		printf("failed to open %s, errno: %d\n", otp_size_path, errno);
		return errno;
	}
	if (fgets(buffer, sizeof(buffer), fp)) {
		size = strtoul(buffer, NULL, 0);	
		printf("size: %ld\n", size);
	}
	fclose(fp);

	wp_grp_size = get_wp_grp_size();
	printf("wp_grp_size: %lx\n", wp_grp_size);
	if (wp_grp_size == 0)
		return -EINVAL;

	*otp_start = start * 512;
	*otp_size = size * 512;;

	if (*otp_start % wp_grp_size) {
		*otp_start += (wp_grp_size - *otp_start % wp_grp_size);
		if (*otp_start > (start * 512 + *otp_size)) {
			printf("invalid otp partition, otp_start: %lx, size: %lx\n", *otp_start, *otp_size);
			return -EINVAL;
		} else {
			*otp_size = size * 512 + (*otp_start % wp_grp_size) - wp_grp_size;
		}
	}

	return 0;
}
#else
static int get_otp_attr(unsigned long *otp_start, unsigned long *otp_size)
{
	char buffer[MAX_LINE_LEN] = {0};
	FILE *fp;
	unsigned long emmc_size = 0;
	unsigned long wp_grp_size = 0;
	unsigned long size, start;

	if (!otp_start || !otp_size)
		return -EINVAL;

	fp = fopen("/proc/dumchar_info", "r");
	if (!fp) {
		printf("failed to open /proc/dumchar_info, errno: %d", errno);
		return errno;
	}
	while (fgets(buffer, MAX_LINE_LEN, fp)) {
		if (strstr(buffer, "otp")) {
			sscanf(buffer, "otp %lx %lx", &size, &start);
			printf("size: %lx, start: %lx\n", size, start);
			break;
		}		
	}
	fclose(fp);

	emmc_size = get_emmc_size();
	printf("emmc_size: %lx\n", emmc_size);
	wp_grp_size = get_wp_grp_size();
	printf("wp_grp_size: %lx\n", wp_grp_size);
	if (emmc_size == 0 || wp_grp_size == 0)
		return -EINVAL;

	*otp_start = emmc_size - start;
	*otp_size = size;
	if (*otp_start % wp_grp_size) {
		*otp_start += (wp_grp_size - *otp_start % wp_grp_size);
		if (*otp_start > (start + size)) {
			printf("invalid otp partition, otp_start: %lx, size: %lx\n", *otp_start, size);
			return -EINVAL;
		} else {
			*otp_size = size + (*otp_start % wp_grp_size) - wp_grp_size;
		}
	}

	return 0;
}
#endif

static int emmc_otp_read(char *buffer, unsigned int blknr, unsigned int size)
{
	const char *path = "/dev/block/mmcblk0";
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;

	if (!buffer) {
		perror("input buffer is NULL!\n");
		return -EINVAL;
	}
	if (size != 512) {
		perror("size must be 512 bytes !\n");
		return -EINVAL;
	}
	if (blknr >= (otp_size / 512)) {
		printf("blknr exceed otp partition size!\n");
		return -EINVAL;
	}

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_READ_SINGLE_BLOCK;
	idata.arg = otp_start / 512 + blknr;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blocks = 1;
	idata.blksz = 512;
	idata.write_flag = 0;
	mmc_ioc_cmd_set_data(idata, buffer);
	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	close(fd);

	return ret;
}

static int emmc_otp_write(char *buffer, unsigned int blknr, unsigned int size)
{
	const char *path = "/dev/block/mmcblk0";
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;

	if (!buffer) {
		perror("input buffer is NULL!\n");
		return -EINVAL;
	}
	if (size != 512) {
		perror("size must be 512 bytes !\n");
		return -EINVAL;
	}
	if (blknr >= (otp_size / 512)) {
		printf("blknr exceed otp partition size!\n");
		return -EINVAL;
	}

	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_WRITE_BLOCK;
	idata.arg = otp_start / 512 + blknr;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blocks = 1;
	idata.blksz = 512;
	idata.write_flag = 1;
	mmc_ioc_cmd_set_data(idata, buffer);
	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	close(fd);

	return ret;
}

/* blknr is the sector number from OTP partition, if the first block, should 
 * set it to 0
 */
static int emmc_get_wp_status(unsigned int blknr, unsigned int *wp_status)
{
	const char *path = "/dev/block/mmcblk0";
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;
		
	if (!wp_status) {
		printf("input wp_status is NULL!\n");
		return -EINVAL;
	}

	if (blknr >= (otp_size / 512)) {
		printf("blknr exceed otp partition size!\n");
		return -EINVAL;
	}

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		printf("failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SEND_WRITE_PROT;
	idata.arg = otp_start / 512 + blknr;
	idata.flags = MMC_RSP_SPI_R1 | MMC_RSP_R1 | MMC_CMD_ADTC;
	idata.blocks = 1;
	idata.blksz = 4;
	idata.write_flag = 0;
	mmc_ioc_cmd_set_data(idata, wp_status);
	ret = ioctl(fd, MMC_IOC_CMD, &idata);
	close(fd);

	return ret;
}

static int emmc_blknr_locked(unsigned int wp_status)
{
	/* bit24 is for the first write protect group status, see spec */
	if ((wp_status >> 24) & 0x01)
		return 1;
	return 0;
}

static int emmc_set_usr_wp(unsigned char wp)
{
	const char *path = "/dev/block/mmcblk0";
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;

	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SWITCH;
	idata.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
		(EXT_CSD_USR_WP << 16) |
		(wp << 8) |
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
	const char *path = "/dev/block/mmcblk0";
	struct mmc_ioc_cmd idata;
	int fd;
	int ret;
	
	if (blknr >= (otp_size / 512)) {
		printf("blknr exceed otp partition size!\n");
		return -EINVAL;
	}
	fd = open(path, O_RDWR);
	if (fd < 0) {
		printf("failed to open %s\n", path);
		return errno;
	}
	memset(&idata, 0, sizeof(idata));
	idata.opcode = MMC_SET_WRITE_PROT;
	idata.arg = otp_start / 512 + blknr;
	idata.flags = MMC_RSP_SPI_R1B | MMC_RSP_R1B | MMC_CMD_AC;
	idata.write_flag = 1;
	ret = ioctl(fd, MMC_IOC_CMD, &idata);

	close(fd);
	return ret;
}

static int emmc_otp_lock(unsigned int blknr)
{
	int ret;
	ret = emmc_set_usr_wp(EXT_CSD_USR_WP_EN_PERM_WP);
	if (!ret)
		return emmc_set_write_protect(blknr);
	printf("failed to set permanent write protect!\n");
	return ret;
}

/* Here is a test program */
int main(int argc, char *argv[])
{
	int ret;
	char buffer[512] = {'M', 'E', 'D', 'I', 'A', 'T', 'E', 'K'};
	int i;
	unsigned int wp_status;
	ret = get_otp_attr(&otp_start, &otp_size);
	if (ret) {
		perror("failed to get otp attribute!\n");
		return ret;
	}

	printf("otp_start: 0x%lx, otp_size: 0x%lx\n", otp_start, otp_size);
	ret = emmc_otp_write(buffer, 0, 512);
	if (ret) {
		printf("failed to write!\n");
		return ret;
	}

	memset(buffer, 0, 512);
	ret = emmc_otp_read(buffer, 0, 512);
	for (i = 0; i < 512; i++) {
		if (i && i % 16 == 0)
			printf("\n");
		printf(" %c", buffer[i]);
	}

	ret = emmc_get_wp_status(0, &wp_status);
	if (!ret) {
		printf("wp_status: %x\n", wp_status);
		if (!emmc_blknr_locked(wp_status)) {
			ret = emmc_otp_lock(0);
		} else {
			printf("Already locked!\n");
		}

	} else {
		printf("failed to get wp_status!\n");
		return ret;
	}

	return ret;
}
