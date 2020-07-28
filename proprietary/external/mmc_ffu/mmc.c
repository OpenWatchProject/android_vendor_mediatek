#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <linux/mmc/sd_misc.h>

#include "mmc_ffu.h"

#define  MMC_CUST_DEVICE "/dev/misc-sd"

#define MMC_FFU_AT_ONE_DATA_XFR

__u8 g_cid[16];
__u8 g_ext_csd[512];
char *progname;

static int atoh(char *str, __u64 *hval)
{
	unsigned int i;
	__u64 val=0;

	for ( i = 0; i < strlen((const char *)str); i++) {
		if (str[i]>='a' && str[i]<='f')
			val = (val << 4) + (str[i] - 'a' + 10);
		else if (str[i] >= 'A' && str[i] <= 'F')
			val = (val << 4) + (str[i] - 'A' + 10);
		else if (str[i] >= '0' && str[i] <= '9')
			val = (val << 4) + (str[i] - '0');
		else
			return -1;
	}

	*hval=val;

	return 0;
}

static int read_extcsd(int fd, __u8 *ext_csd)
{
	int ret = 0;
	struct mmc_ioc_cmd mmc_ioc_cmd;

	memset(ext_csd, 0, sizeof(__u8) * 512);
	memset(&mmc_ioc_cmd, 0, sizeof(mmc_ioc_cmd));
	mmc_ioc_cmd.blocks = 1;
	mmc_ioc_cmd.blksz = 512;
	mmc_ioc_cmd.opcode = MMC_SEND_EXT_CSD;
	mmc_ioc_cmd.flags = MMC_CMD_ADTC | MMC_RSP_R1;
	mmc_ioc_cmd_set_data(mmc_ioc_cmd, ext_csd);

	ret = ioctl(fd, MMC_IOC_CMD, &mmc_ioc_cmd);

	if (ret)
		perror("ioctl");

	return ret;
}

static char *get_progname(char *progWithPath)
{
	char *cptr;

	cptr = strrchr(progWithPath,'/');
	if (cptr)
		cptr++;
	else
		cptr = progWithPath;

	return cptr;
}

static void print_usage(char *programname)
{
	printf("Usage:\n");
	printf("    %s check $device\n", programname);
	printf("    %s do $image_path $device $manf_id [$new_fw_revision $old_fw_revision]\n", programname);
	printf("Parmeters:\n");
	printf("    $device: device node, e.g., /dev/block/mmcblk0\n");
	printf("    $manf_id: manufacturer id, hex value without leading 0x\n");
	printf("                0: skip checking for vendor_id and fw_revision\n");
	printf("                02: Sandisk old\n");
	printf("                11: Toshiba\n");
	printf("                13: Micron\n");
	printf("                15: Samsung\n");
	printf("                45: Sandisk\n");
	printf("                70: Kinston\n");
	printf("                90: Hynix\n");
	printf("    $new_fw_revision and $old_fw_revision: hex value without leading 0x\n");
}

static int do_read_cid(__u8 *cid)
{
	int fd, ret;
	char *device;
	struct msdc_ioctl l_ioctl_arg;

	device= MMC_CUST_DEVICE;

	fd = open(device, O_RDONLY);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	memset(&l_ioctl_arg,0,sizeof(struct msdc_ioctl));
	l_ioctl_arg.host_num = 0;
	l_ioctl_arg.total_size = 16;
	l_ioctl_arg.opcode = MSDC_GET_CID;
	l_ioctl_arg.buffer = (unsigned int *)cid;

	ret = ioctl(fd, MSDC_GET_CID, &l_ioctl_arg);
	if (ret) {
		fprintf(stderr, "Could not read CID from %s\n", device);
		exit(1);
	}

	printf("Manufacture ID 0x%02x\n", cid[3]);
	printf("Product Name 0x%02x%02x%02x%02x%02x%02x\n",
		cid[0], cid[7], cid[6], cid[5], cid[4], cid[11]);
	printf("Product Revision 0x%02x\n", cid[10]);

	close(fd);

	return 0;
}

static int do_check_ffu(char *device, __u8 *ext_csd)
{
	int fd, ret;

	fd = open(device, O_RDWR);
	if (fd < 0) {
		perror("open");
		exit(1);
	}

	ret = read_extcsd(fd, ext_csd);
	if (ret) {
		fprintf(stderr, "Could not read EXT_CSD from %s\n", device);
		close(fd);
		exit(1);
	}

	if (ext_csd[EXT_CSD_REV] >= 7) {
		printf("FFU support 0x%02x\n",
			ext_csd[EXT_CSD_SUPPORTED_MODE] & 0x1);
		if (ext_csd[EXT_CSD_SUPPORTED_MODE] & 0x1) {
			printf("FFU_ARG 0x%02x%02x%02x%02x\n",
				ext_csd[EXT_CSD_FFU_ARG + 3], ext_csd[EXT_CSD_FFU_ARG + 2],
				ext_csd[EXT_CSD_FFU_ARG + 1], ext_csd[EXT_CSD_FFU_ARG]);
			printf("FIRMWARE_VERSION: 0x%02x%02x%02x%02x"
				" 0x%02x%02x%02x%02x\n",
				ext_csd[261], ext_csd[260], ext_csd[259], ext_csd[258],
				ext_csd[257], ext_csd[256], ext_csd[255], ext_csd[254]);
				printf("SUPPORTED_MODE_OPERATION_CODES 0x%02x\n",
				ext_csd[EXT_CSD_FFU_FEATURES]);
			printf("FFU_STATUS 0x%02x\n", ext_csd[26]);
		}
	} else {
		printf("FFU is not supported by device before eMMC 5.0\n");
	}

	close(fd);

	return ret;
}

static int check_current_fw(__u64 new_manfid, __u64 new_fwrev, __u64 old_fwrev_desired, char *device)
{
	__u8 manfid;
	__u64 fwrev;
	char *manf;
	const char *str;
	int  match = 0, ret = -1;

	/* get EXT_CSD to check FFU support and status */
	ret = do_check_ffu(device, g_ext_csd);
	if ( ret )
		exit(0);

	if (g_ext_csd[EXT_CSD_REV] >= 7) {
		if (g_ext_csd[EXT_CSD_SUPPORTED_MODE] & 0x1) {
			fwrev= ((__u64)g_ext_csd[261]) << 56 | ((__u64)g_ext_csd[260]) << 48
				| ((__u64)g_ext_csd[259]) << 40 | ((__u64)g_ext_csd[258]) << 32
				| ((__u64)g_ext_csd[257]) << 24 | ((__u64)g_ext_csd[256]) << 16
				| ((__u64)g_ext_csd[255]) << 8 | g_ext_csd[254];
		}
	} else {
		printf("Device is not eMMC 5.0 or later!\n");
		exit(0);
	}

	if (new_manfid == 0) {
		printf("--> SW checking: skip\n");
		ret = 0;
		goto out;
	}

	do_read_cid(g_cid);

	manfid=g_cid[3];

	//printf("new_manfid %u, orig manfid %u\n", (unsigned int)new_manfid, manfid);
	//printf("new_fwrev %u, fwrev %u, fwrev_desired %u\n", (unsigned int)new_fwrev, fwrev, (unsigned int)old_fwrev_desired);
	if (new_manfid != manfid) {
		ret = -1;
		goto out;
	}

	match = 1;

	if (manfid == 0x2) {
		/*Sandisk*/
		manf = "SANDISK old";
	} else if (manfid == 0x11) {
		/*Toshiba*/
		manf = "Toshiba";
	} else if (manfid == 0x13) {
		/*Micron*/
		manf = "Micron";
	} else if (manfid == 0x15) {
		/*Samsung*/
		manf = "Samsung";
	} else if (manfid == 0x45) {
		/*Sandisk new*/
		manf = "SANDISK";
	} else if (manfid == 0x70) {
		/*KSI - Kingston*/
		manf = "Kingston";
	} else if (manfid == 0x90) {
		/*HYNIX*/
		manf = "Hynix";
	} else {
		match = 0;
		printf("Manufacturer not matched: desired 0x%x\n", (unsigned int)new_manfid);
	}

	if (match == 1) {
		printf("Manufacturer matched\n");
		if ((old_fwrev_desired == fwrev) || (old_fwrev_desired == 0)) {
			if ((new_fwrev != fwrev) || (new_fwrev == 0))
				match=2;
		}
	}

	if (match >= 1) {
		printf("  Manufacturer: %s\n", manf);
		if (old_fwrev_desired == 0)
			printf("  Prev FW rev: 0x%02x --> don't care\n", (unsigned int)fwrev);
		else
			printf("  Prev FW rev: 0x%02x\n", (unsigned int)fwrev);

		if (new_fwrev != 0)
			printf("  New FW rev: 0x%02x\n", (unsigned int)new_fwrev);
		else if (new_fwrev == 0)
			printf("  New FW rev: don't care\n");
	}

	if (match == 2) {
		printf("--> SW checking: pass\n");
		ret = 0;
	} else {
		printf("--> SW checking: fail\n");
		ret = -1;
	}

out:

	return ret;
}

static int ffu_download_image(int fw_fd, int mmc_fd) {
	int ret = 0;
	int file_size, size, image_length;
	struct mmc_ioc_cmd mmc_ioc_cmd;
	char image_buff[MMC_IOC_MAX_BYTES];

	/* get file size */
	file_size = lseek(fw_fd, 0, SEEK_END);
	if (file_size < 0) {
		ret =  -1;
		perror("seek file error \n");
		goto exit;
	}

	printf("FW size %d, MMC_IOC_MAX_BYTES %d\n", file_size, (unsigned int)MMC_IOC_MAX_BYTES);

#if defined(MMC_FFU_AT_ONE_DATA_XFR)
	if (file_size > MMC_IOC_MAX_BYTES)
		printf("Please enlarge MMC_IOC_MAX_BYTES to make FW transferred in a CMD!\n");
#endif

	lseek(fw_fd, 0, SEEK_SET);

	memset(image_buff, 0, sizeof(image_buff));

#if !defined(MMC_FFU_AT_ONE_DATA_XFR)
	do {
#endif
		if (file_size > MMC_IOC_MAX_BYTES)
			size = MMC_IOC_MAX_BYTES;
		else
			size = file_size;

		/* Read FW data from file */
		image_length = read(fw_fd, image_buff, size);
		if (image_length == -1) {
			ret = -1;
			goto exit;
		}
		/* prepare and send ioctl */
		memset(&mmc_ioc_cmd, 0, sizeof(mmc_ioc_cmd));
		mmc_ioc_cmd.arg =  0;
		mmc_ioc_cmd.write_flag = 1;
		mmc_ioc_cmd.blocks = image_length / CARD_BLOCK_SIZE;
		mmc_ioc_cmd.blksz = CARD_BLOCK_SIZE;
		mmc_ioc_cmd.opcode = FFU_DWONLOAD_OP;
		mmc_ioc_cmd.flags = MMC_CMD_ADTC | MMC_RSP_R1;
		mmc_ioc_cmd_set_data(mmc_ioc_cmd, image_buff);
		ret = ioctl(mmc_fd, MMC_IOC_CMD, &mmc_ioc_cmd);
		if (ret) {
			perror("ioctl FW download");
			goto exit;
		}

		file_size = file_size - size;
		printf("firmware file loading, remaining:   %d\n", file_size);

#if !defined(MMC_FFU_AT_ONE_DATA_XFR)
	} while (file_size > 0);
#endif

exit:

	return ret;
}

static int do_mmc_ffu(int argc, char **argv) {
	int fd, fw_fd, ret, i;
	__u64 new_manfid=0ULL;
	__u64 new_fwrev=0ULL;
	__u64 old_fwrev_desired=0ULL;

	if (argc>=5) {
		if (atoh(argv[4], &new_manfid)) {
			perror("fw revision can contains only 0~9, a~f, or A-F");
			exit(1);
		}
		if ((new_manfid!=0) && (argc<7)) {
			print_usage(progname);
			exit(1);
		}

		if (argc>=6) {
			if (atoh(argv[5], &new_fwrev)) {
				perror("fw revision can contains only 0~9, a~f, or A-F");
				exit(1);
			}
		}

		if (argc>=7) {
			if (atoh(argv[6], &old_fwrev_desired)) {
				perror("fw revision can contains only 0~9, a~f, or A-F");
				exit(1);
			}
		}
	}

	if (check_current_fw(new_manfid, new_fwrev, old_fwrev_desired, argv[3]))
		exit(1);

	fd = open(argv[3], O_RDWR);
	if (fd < 0) {
		perror("open device file");
		exit(1);
	}

	/* open eMMC5.0 firmware image file */
	fw_fd = open(argv[2], O_RDONLY);
	if (fw_fd < 0) {
		perror("open eMMC firmware file");
		ret = -1;
		goto exit;
	}

	ret = ffu_download_image(fw_fd, fd);
	if (ret)
		goto exit;

exit:
	close(fd);
	close(fw_fd);

	return ret;
}

int main(int argc, char **argv)
{
	int ret;
	progname = get_progname(argv[0]);

	if(argc <= 2) {
		print_usage(progname);
		exit(0);
	}

	if (argc>2) {
		if (!strcmp(argv[1], "check")) {
			ret = do_check_ffu(argv[2], g_ext_csd);
			if (ret)
				printf("Fail to check ffu status\n");
		} else if (!strcmp(argv[1], "do")) {
			//progname do <image path> <device> vendor_id [new_fw_revision old_fw_revision]
			if ((argc < 5) || (argc == 6)) {
				print_usage(progname);
				exit(0);
			}

			do_mmc_ffu(argc, argv);
		}
	}

	return 0;

}

