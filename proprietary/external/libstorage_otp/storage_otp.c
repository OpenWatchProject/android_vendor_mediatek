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
#include <sys/utsname.h>

#include "storage_otp.h"

/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)
static int get_boot_type(void) {
	int fd;
	ssize_t s;
	char boot_type[4] = {'0'};

	fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
	if (fd < 0) {
		OTP_LOG("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
		return -1;
	}

	s = read(fd, (void *)&boot_type, sizeof(boot_type) - 1);
	close(fd);

	if (s <= 0) {
		OTP_LOG("could not read boot type sys file\n");
		return -1;
	}

	boot_type[s] = '\0';

	return atoi((char *)&boot_type);
}

static int otp_init(struct otp *otp_device, unsigned int user_id)
{
	unsigned int ret = 0;
	unsigned int i = 0;
	int mt_boot_type = get_boot_type();

	if (mt_boot_type == BOOTDEV_UFS) {
		ret = ufs_storage_init(otp_device);
	} else {
		/* Use eMMC for default boot type */
		ret = emmc_storage_init(otp_device);
	}

	if (ret)
		return -EINVAL;

	otp_device->max_user_num = otp_device->otp_size /
				otp_device->wp_grp_size;

	if (otp_device->max_user_num < MAX_USER_NUM) {
		OTP_LOG("OTP partition have no enough space to support %d users\n",
				MAX_USER_NUM);
		return -EINVAL;
	} else {
		OTP_LOG("Device support %d group, OTP use %d group\n", otp_device->max_user_num,
				MAX_USER_NUM);
		otp_device->max_user_num = MAX_USER_NUM;
	}

	if (user_id >= MAX_USER_NUM) {
		OTP_LOG("user id is beyond max user: %d\n",
				MAX_USER_NUM);
		return -EINVAL;
	}
	OTP_LOG("otp parition start(block) is 0x%llx, size(block) is 0x%llx\n",
			otp_device->partition_start / otp_device->blk_sz,
			otp_device->otp_size / otp_device->blk_sz);

	/* region init
	 * otp region layer out
	 *
	 *  | offset |   part1   |  part2    |  part3    |  partn    |not used|
	 *  |<------>|<--------->|<--------->|<--------->|<--------->|XXXXXXXX|
	 *  |        |           |           |           |           |        |
	 *  region start address are divisible by wp group
	 *  region size is equal with wp_grp_size
	 *
	 * */

	for (i = 0; i < otp_device->max_user_num; i++) {
		otp_device->region[i].size = otp_device->wp_grp_size;
		otp_device->region[i].start = otp_device->otp_start +
					i * otp_device->region[i].size;
		otp_device->region[i].user_id = i;
	}
	otp_device->current_user_id = user_id;
	OTP_LOG("user%d start(block) is 0x%llx, size(block) is 0x%llx\n", user_id,
			otp_device->region[user_id].start / otp_device->blk_sz,
			otp_device->region[user_id].size / otp_device->blk_sz);

	return 0;
}
struct otp *otp_open(unsigned int user_id)
{
	int ret = 0;
	struct otp *device;

	device = (struct otp *) malloc(sizeof(struct otp));
	if (NULL == device) {
		perror("malloc otp device failed\n");
		return NULL;
	}
	memset(device, 0, sizeof(struct otp));
	ret = otp_init(device, user_id);
	if (ret < 0) {
		OTP_LOG("storage otp device init failed\n");
		free(device);
		return NULL;
	}
	return device;

}
void otp_close(struct otp *otp_device)
{
	free(otp_device);
}

int otp_get_region(struct otp *otp_device, unsigned long long *start, unsigned long long *size)
{
	unsigned int i;
	unsigned int current_user_id;

	current_user_id = otp_device->current_user_id;

	for (i = 0; i < MAX_USER_NUM; i++) {
		if (current_user_id == otp_device->region[i].user_id) {
			*start = otp_device->region[i].start;
			*size = otp_device->region[i].size;
			break;
		}
	}

	if (i == MAX_USER_NUM) {
		OTP_LOG("Can't find current otp region\n");
		return -EINVAL;
	}

	return 0;
}

int otp_ops_read_write(unsigned int type, struct otp *otp_device, char *buffer,
		unsigned long long offset, unsigned long long size)
{
	unsigned long long otp_start;
	unsigned long long otp_size;
	unsigned long long otp_start_block;
	unsigned long long otp_size_block;
	unsigned offset_block;
	unsigned block_number;
	char *buffer_tmp = NULL;
	unsigned long long offset_tmp;
	unsigned long long size_tmp;
	int ret;

	if (!buffer) {
		OTP_LOG("input buffer is NULL!\n");
		return -EINVAL;
	}

	if (!otp_device->block_write || !otp_device->block_read) {
		OTP_LOG("otp_device->block_write/read is NULL!\n");
		return -EINVAL;
	}

	offset_tmp = offset;
	size_tmp = size;

	if (offset % otp_device->blk_sz) {
		OTP_LOG("byte offset is not divisible by %llu\n", otp_device->blk_sz);
		offset_tmp = offset - (offset % otp_device->blk_sz);
		size_tmp = size + (offset % otp_device->blk_sz);
	}
	if (size_tmp % otp_device->blk_sz) {
		size_tmp += (otp_device->blk_sz - (size_tmp % otp_device->blk_sz));
	}

	if ((size_tmp % otp_device->blk_sz) || (offset_tmp % otp_device->blk_sz)) {
		printf("parameter error, size_tmp %llu offset_tmp %llu\n", size_tmp, offset_tmp);
		return -1;
	}

	buffer_tmp = malloc(size_tmp);

	offset_block = offset_tmp / otp_device->blk_sz;
	block_number = size_tmp / otp_device->blk_sz;

	otp_get_region(otp_device, &otp_start, &otp_size);

	otp_start_block = otp_start / otp_device->blk_sz;
	otp_size_block = otp_size / otp_device->blk_sz;

	/*
	 * if otp_start_block = 0, we will access "otp partition block device" directly thus
	 *                         below checking shall be skipped.
	 * if otp_start_block != 0, it indicates otp partition block device" is not aligned to
	 *                         write proetct group size.
	 */
	if (otp_start_block != 0) {
		if (offset_block + block_number >= otp_size_block) {
			OTP_LOG("%s size or offset exceed otp region!\n", __func__);
			free(buffer_tmp);
			return -EINVAL;
		}
	}

	if (!type)
		if (otp_device->write != NULL)
			ret = otp_device->write(otp_device, buffer,
				otp_start - otp_device->partition_start + offset,
				size);
		else {
			if (offset % otp_device->blk_sz || size % otp_device->blk_sz) {
				ret = otp_device->block_read(otp_device, buffer_tmp,
					otp_start_block + offset_block, block_number);
			}
			memcpy(buffer_tmp + (offset % otp_device->blk_sz), buffer, size);
			ret = otp_device->block_write(otp_device, buffer_tmp,
				otp_start_block + offset_block,
				block_number);
		}
	else
		if (otp_device->read != NULL)
			ret = otp_device->read(otp_device, buffer,
				otp_start - otp_device->partition_start + offset,
				size);
		else {
			ret = otp_device->block_read(otp_device, buffer_tmp,
				otp_start_block + offset_block, block_number);
			memcpy(buffer, buffer_tmp + (offset % otp_device->blk_sz), size);
		}
	free(buffer_tmp);
	return ret;
}

int otp_read(struct otp *otp_device, char *buffer,
		unsigned long long offset, unsigned long long size)
{
	if ((NULL == otp_device) || (NULL == buffer)) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}
	OTP_LOG("%s offset %llu, size %llu\n", __func__, offset, size);

	return otp_ops_read_write(1, otp_device, buffer, offset, size);
}

unsigned int otp_write(struct otp *otp_device, char *buffer,
		unsigned long long offset, unsigned long long size)
{
	unsigned int status = 0;
	unsigned int type = 0;

	if ((NULL == otp_device) || (NULL == buffer)) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}

	otp_get_status(otp_device, &status, &type);
	if (status) {
		OTP_LOG("Region has already been locked, type is %d\n",
			type);
		return -EINVAL;
	}
	OTP_LOG("%s offset %llu, size %llu\n", __func__, offset, size);
	return otp_ops_read_write(0, otp_device, buffer, offset, size);
}


int otp_lock(struct otp * otp_device, unsigned int type)
{
	unsigned long long otp_start;
	unsigned long long otp_size;
	unsigned long long otp_start_block;
	unsigned long long otp_size_block;
	unsigned int wp_group_count;

	if (NULL == otp_device) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}

	if (type == WP_TEMPORARY) {
		OTP_LOG("Don't Support Temporary Lock\n");
		return -EINVAL;
	}

	OTP_LOG("%s parameter: %d\n", __func__, type);

	otp_get_region(otp_device, &otp_start, &otp_size);

	otp_start_block = otp_start / otp_device->blk_sz;
	otp_size_block = otp_size / otp_device->blk_sz;
	wp_group_count = otp_size_block / (otp_device->wp_grp_size / 512);
	OTP_LOG("%s otp_start_block: %llu\n", __func__, otp_start_block);

	if (otp_device->lock)
		return otp_device->lock(otp_device, type, otp_start_block,
				wp_group_count, otp_device->wp_grp_size);
	else
		return -EINVAL;
}

int otp_unlock(struct otp *otp_device)
{
	unsigned long long otp_start;
	unsigned long long otp_size;
	unsigned long long otp_start_block;
	unsigned long long otp_size_block;
	unsigned int wp_group_count;

	if (NULL == otp_device) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}

	OTP_LOG("%s\n", __func__);

	otp_get_region(otp_device, &otp_start, &otp_size);

	otp_start_block = otp_start / otp_device->blk_sz;
	otp_size_block = otp_size / otp_device->blk_sz;
	wp_group_count = otp_size_block / (otp_device->wp_grp_size / 512);

	if (otp_device->unlock)
		return otp_device->unlock(otp_device, otp_start_block,
				wp_group_count, otp_device->wp_grp_size);
	else
		return -EINVAL;
}

int otp_get_status(struct otp *otp_device, unsigned int *status, unsigned int *type)
{
	unsigned long long otp_start;
	unsigned long long otp_size;
	unsigned long long otp_start_block;
	unsigned long long otp_size_block;
	unsigned int wp_group_count;

	if ((NULL == otp_device) || (NULL == status) || (NULL == type)) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}

	OTP_LOG("%s\n", __func__);

	otp_get_region(otp_device, &otp_start, &otp_size);

	otp_start_block = otp_start / otp_device->blk_sz;
	otp_size_block = otp_size / otp_device->blk_sz;
	wp_group_count = otp_size_block / (otp_device->wp_grp_size / 512);

	if (otp_device->get_status)
		return otp_device->get_status(otp_device, otp_start_block,
				wp_group_count, status, type);
	else
		return -EINVAL;
}

unsigned long long otp_get_size(struct otp *otp_device)
{
	unsigned long long otp_start;
	unsigned long long otp_size;

	if (NULL == otp_device) {
		OTP_LOG("NULL parameter\n");
		return -EINVAL;
	}

	OTP_LOG("%s\n", __func__);

	otp_get_region(otp_device, &otp_start, &otp_size);

	return otp_size;

}

/*
 * otp_get_libversion_if_legacy(void)
 *
 * Return:
 *   1: Legacy version, i.e., /dev/otp is existed.
 *   2: New version (>= 1.0), /dev/otp is not existed.
 */
unsigned int otp_get_libversion_if_legacy(void)
{
	FILE *fp;

	fp = fopen("/dev/otp", "r");

	if (fp) {
		OTP_LOG("/dev/otp is existed, legacy version.\n");
		fclose(fp);
		return 1;
	} else
		OTP_LOG("/dev/otp is not existed, not legacy version.\n");
		return 0;
}

unsigned int otp_get_libversion(void)
{
	struct utsname testbuff;
	int fb = 0;
	char c;

	fb = uname(&testbuff);
	if (fb < 0) {
		OTP_LOG("get kernel version failed\n");
	} else {
		OTP_LOG("Kernel version: %s\n", testbuff.release);
	}

	/* get first char from kernel version
	 * for example: 3.18.22 ->  c = 3
	 *              4.4.0   ->  c = 4
	 * from kernel version 4.4, we support user space
	 * otp lib API, lower version can not use this
	 */
	c = testbuff.release[0];

	if (c < '4') {
		return 0;
	} else {
		if (otp_get_libversion_if_legacy())
			return 0;
		else
			return 1;
	}
}

#if 0
/* Here is a test program */
#include <pthread.h>

#define BUFFER_SIZE 64 * 1024 * 1024
char buffer[BUFFER_SIZE];

int ut_otp_read(unsigned int userid, unsigned int offset,
	unsigned int size, int check, int value, int dump)
{
	int ret = 0;
	unsigned int i;
	char *buf = NULL;
	struct otp *device = NULL;

	printf("Read from region %x, offset: %d, size: %d\n",
		userid, offset, size);

	device = otp_open(userid);
	if (!device) {
		perror("failed to open emmc/ufs device!\n");
		ret = -1;
		goto out;
	}

	buf = malloc(BUFFER_SIZE);
	if (!buf) {
		perror("ut_otp_read alloc buffer error\n");
		ret = -1;
		goto out;
	}

	memset(buf, 0, BUFFER_SIZE);
	ret = otp_read(device, buf, offset, size);
	if (ret) {
		perror("ut_otp_read error\n");
		ret = -1;
		goto out;
	}

	if (dump) {
		printf("Read buffer : \n");
		for (i = 0; i < (size); i++) {
			if (i && i % 32 == 0)
				printf("\n");
			printf(" %d", buf[i]);
		}
	}

	if (check) {
		for (i = 0; i < (size); i++) {
			if (buf[i] != value) {
				printf("ERROR: buf[%d] = 0x%x != 0x%x\n", i, buf[i], value);
				break;
			}
		}
	}

	printf("\n");

out:
	if (buf)
		free(buf);
	if (device)
		otp_close(device);

	return ret;
}

int ut_otp_write(unsigned int userid, unsigned int offset,
	unsigned int size, unsigned int value)
{
	int ret = 0;
	char *buf = NULL;
	struct otp *device = NULL;

	printf("Write to region %x, offset: %d, size: %d, value: 0x%x\n",
			userid, offset, size, value);

	device = otp_open(userid);
	if (!device) {
		perror("failed to open emmc/ufs device!\n");
		ret = -1;
		goto out;
	}

	buf = malloc(BUFFER_SIZE);
	if (!buf) {
		perror("ut_otp_write alloc buffer error\n");
		ret = -1;
		goto out;
	}

	memset(buf, value, BUFFER_SIZE);
	ret = otp_write(device, buf, offset, size);
	if (ret) {
		perror("ut_otp_write error\n");
		ret = -1;
		goto out;
	}

out:
	if (buf)
		free(buf);
	if (device)
		otp_close(device);

	return ret;
}

void *ut_otp_run_thread(void *ptr)
{
	int ret = 0;
	int idx = *(int *)ptr;
	int rand_offset, rand_size, rand_val;
	int fail = 0, count = 0;

	printf("OTP USER%d: %s() start\n", idx, __func__);

	while (1) {
		count++;

		rand_offset = rand() % 512;
		rand_size = rand() % 512;
		rand_val = rand() % 256;

#if 0
		ret = ut_otp_write(idx, rand_offset, rand_size, rand_val);
		if (ret) {
			fail++;
			continue;
		}
#endif
		ret = ut_otp_read(idx, rand_offset, rand_size, 0, rand_val, 0);
		if (ret)
			fail++;

		printf("OTP USER%d: %d tests / %d fail\n", idx, count , fail);

		usleep(10 * 1000);
	}

	printf("OTP USER%d: %s() end\n", idx, __func__);
}

int ut_otp_multi_thread(int count)
{
	int ret = 0, i;
	int data[MAX_USER_NUM];
	pthread_t task[MAX_USER_NUM];

	if (count > MAX_USER_NUM) {
		printf("OTP: %s() thread count (%d) > MAX USER )%d)\n",
			__func__ , count ,MAX_USER_NUM);
		ret = -1;
		goto out;
	}

	for (i = 0; i < count; i++) {
		data[i] = i;
		/* Now we will create the thread passing it data as a paramater*/
		pthread_create(&task[i], NULL, ut_otp_run_thread, &data[i]);
	}
	//pthread_join(task[i], NULL);

out:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = 0;
	unsigned int i;
	struct otp *device = NULL;
	unsigned int userid;
	unsigned int offset;
	unsigned int size;
	unsigned int offset_block;
	unsigned int block_number;
	unsigned int value;
	unsigned int type;
	unsigned int status;

	if (argc == 1) {
		userid = 0;
		device = otp_open(userid);
		if (NULL == device) {
			perror("failed to open emmc/ufs device!\n");
			return -1;
		}

		memset(buffer, 0, BUFFER_SIZE);
		ret = otp_read(device, buffer, 0, 512);
		printf("User0 read : size=512, offset=0\n");
                for (i = 0; i < 10; i++)
                        printf(" 0x%.2x\n", buffer[i]);

		for (i = 0; i < 512; i++)
			buffer[i] = 2;
		printf("User0 write: size=512, offset=0, value=2\n");
		otp_write(device, buffer, 0, 512);

		for (i = 0; i < 512; i++)
			buffer[i] = 0;

		ret = otp_read(device, buffer, 0, 512);
		printf("User0 read : size=512, offset=0\n");
		for (i = 0; i < 10; i++)
			printf(" 0x%.2x\n", buffer[i]);

		printf("User0 lock : WP_PERMANENT\n");

		otp_lock(device, WP_PERMANENT);
		ret = otp_get_status(device, &status, &type);
		printf("OTP STATUS is %x\n", status);
		printf("OTP type is %x\n", type);

		for (i = 0; i < 512; i++)
			buffer[i] = 3;
		printf("User0 write: size=512, offset=0, value=3\n");
		otp_write(device, buffer, 0, 512);
		for (i = 0; i < 512; i++)
			buffer[i] = 0;

		ret = otp_read(device, buffer, 0, 512);
		printf("User0 read: size=512, offset=0\n");
		for (i = 0; i < 10; i++)
			printf(" 0x%.2x\n", buffer[i]);

		printf("User0: unlock\n");
		otp_unlock(device);
		ret = otp_get_status(device, &status, &type);
		printf("OTP STATUS is %x\n", status);
		printf("OTP type is %x\n", type);

		for (i = 0; i < 512; i++)
			buffer[i] = 4;
		printf("User0 write: size=512, offset=0, value=4\n");
		otp_write(device, buffer, 0, 512);
		for (i = 0; i < 512; i++)
			buffer[i] = 0;

		ret = otp_read(device, buffer, 0, 512);
		printf("User0 read: size=512, offset=0\n");
		for (i = 0; i < 10; i++)
			printf(" 0x%.2x\n", buffer[i]);

		printf("User0: get");
		printf("otp lib version is %d\n", otp_get_libversion());
		printf("Get userid: 0x%x, region size is 0x%llx\n",
				userid, otp_get_size(device));

		otp_close(device);
		return 0;
	}
	/* Write operation: emmcotp_test wirte [userid] [offset] [size] [value] */
	if (!strcmp("write", argv[1])) {
		userid = strtoul(argv[2], NULL, 0);
		offset = strtoul(argv[3], NULL, 0);
		size = strtoul(argv[4], NULL, 0);
		value = strtoul(argv[5], NULL, 0);

		ret = ut_otp_write(userid, offset, size, value);

	} else if (!strcmp("read", argv[1])) {
		/* Read operation: emmcotp_test read [userid] [offset] [size] */
		userid = strtoul(argv[2], NULL, 0);
		offset = strtoul(argv[3], NULL, 0);
		size = strtoul(argv[4], NULL, 0);

		ret = ut_otp_read(userid, offset, size, 0, 0, 1);

	} else if (!strcmp("lock", argv[1])) {
		/* Lock and unlock operation: emmcotp_test lock [user_id] [type] */
		userid = strtoul(argv[2], NULL, 0);
		device = otp_open(userid);
		if (device == NULL) {
			perror("failed to open emmc/ufs device!\n");
			ret = -1;
			goto end;
		}

		printf("Lock region %x, opeartion: %s\n",
			userid, argv[3]);

		if (!strcmp(argv[3], "temporary"))
			ret = otp_lock(device, WP_TEMPORARY);
		else if (!strcmp(argv[3], "poweron"))
			ret = otp_lock(device, WP_POWER_ON);
		else if (!strcmp(argv[3], "WP_PERMANENT"))
			ret = otp_lock(device, 2);
		else if (!strcmp(argv[3], "unlock")) {
			ret = otp_unlock(device);
		} else if (!strcmp(argv[3], "status")) {
			ret = otp_get_status(device, &status, &type);
			printf("OTP STATUS is %x\n", status);
			printf("OTP type is %x\n", type);
		} else {
			printf("invalid lock parameter\n");
			ret = -EINVAL;
		}

		if (ret) {
			perror("otp_lock error\n");
			ret = -1;
		}

		otp_close(device);

		return 0;

	} else if (!strcmp("get", argv[1])) {
		printf("otp lib version is %d\n", otp_get_libversion());
		userid = strtoul(argv[2], NULL, 0);
		device = otp_open(userid);
		if (device == NULL) {
			perror("failed to open emmc/ufs device!\n");
			return -1;
		}

		printf("Get userid: 0x%x, region size is 0x%llx\n",
			userid, otp_get_size(device));


		otp_close(device);
	} else if (!strcmp("multi", argv[1])) {

		printf("otp multi test\n");

		ret = ut_otp_multi_thread(4);
		if (ret)
			goto end;

		while(1);
	}

end:
	return ret;
}
#endif


