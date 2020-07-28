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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "ufs-mtk-ioctl.h"	/* located in device/mediatek/common/kernel-headers/linux/scsi/ufs */
#include "ufs_util.h"

#define FFU_COMMON_BUF_SIZE	256  		/* Size for query operations */
#define FFU_FW_SIZE		512 * 1024	/* Size for firmware data */

char *ufs_mtk_util_name;

static int ufs_mtk_util_check_str_format(char *str)
{
	unsigned int i;

	for (i = 0; i < strlen((const char *)str); i++) {
		if ((str[i] >= 'a' && str[i] <= 'z') ||
			(str[i] >= 'A' && str[i] <= 'Z') ||
			(str[i] >= '0' && str[i] <= '9'))
			continue;
		else
			return -1;
	}

	return 0;
}

static int ufs_mtk_util_atoh(char *str, __u32 *hval)
{
	unsigned int i;
	__u32 val = 0;

	for (i = 0; i < strlen((const char *)str); i++) {
		if (str[i] >= 'a' && str[i] <= 'f')
			val = (val << 4) + (str[i] - 'a' + 10);
		else if (str[i] >= 'A' && str[i] <= 'F')
			val = (val << 4) + (str[i] - 'A' + 10);
		else if (str[i] >= '0' && str[i] <= '9')
			val = (val << 4) + (str[i] - '0');
		else
			return -1;
	}

	*hval = val;

	return 0;
}

static char *ufs_mtk_ffu_get_prog_name(char *progWithPath)
{
	char *cptr;

	cptr = strrchr(progWithPath, '/');
	if (cptr)
		cptr++;
	else
		cptr = progWithPath;

	return cptr;
}

static void ufs_mtk_ffu_print_usage(char *name)
{
	printf("\n-- Usage --\n");
	printf("  %s ffu_check <device>\n", name);
	printf("  %s ffu_do <device> <fw_path> <manu_id> <base_fw_revision>\n", name);
	printf("\n-- Parmeters --\n");
	printf("  <device>: Device node. E.g., /dev/block/sdc\n");
	printf("  <fw_path>: Location of firmware. E.g., /system/bin/ufs_fw.bin\n");
	printf("  <manu_id>: Manufacturer ID. HEX value without leading 0x. E.g., 1AD\n");
	printf("	  0: Skip checking <manu_id> and <base_fw_revision>\n");
	printf("	198: Toshiba\n");
	printf("	1AD: SK-Hynix\n");
	printf("	1CE: Samsung\n");
	printf("  <base_fw_revision>: Base firmware version. A string with at most 4 chars. E.g., T007\n\n");
}

static int ufs_mtk_ffu_get_dev_info(char *device, struct ufs_ffu_data *fdata)
{
	int fd;
	int err = 0;
#ifdef UFS_FFU_QUERY_FW_VER_BY_STRING_DESCR
	int prl_idx;
#endif
	struct ufs_ioctl_query_data idata;
	struct ufs_ioctl_query_fw_ver_data idata_fw;

	if (!fdata) {
		printf("BUG: fdata shall not be NULL\n");
		err = UFS_UTIL_ERR_MEM_FAIL;
		goto out;
	}

	fd = open(device, O_RDONLY);

	if (fd < 0) {
		printf("Open device %s failed, err: %d\n", device, fd);
		/* err = UFS_UTIL_ERR_OPEN_DEV_FAIL; */
		err = -fd;
		goto out;
	}

	/* get device manufacturer ID and check if device supports ffu */

	idata.opcode = UPIU_QUERY_OPCODE_READ_DESC;
	idata.idn = QUERY_DESC_IDN_DEVICE;
	idata.idx = 0;
	idata.buf_ptr = malloc(FFU_COMMON_BUF_SIZE);
	if (idata.buf_ptr == NULL) {
		printf("idata.buf_ptr malloc fail!\n");
		err = -1;
		close(fd);
		goto out;
	}
	memset(idata.buf_ptr, 0, FFU_COMMON_BUF_SIZE);
	idata.buf_byte = QUERY_DESC_DEVICE_MAX_SIZE;

	err = ioctl(fd, UFS_IOCTL_QUERY, &idata);

	if (err) {
		printf("Read device descriptor failed, err: %d\n", err);
		err = UFS_UTIL_ERR_IOCTL_FAIL;
		goto out_close_fd;
	}

	fdata->dev_manu_id = idata.buf_ptr[DEVICE_DESC_PARAM_MANF_ID] << 8 |
							idata.buf_ptr[DEVICE_DESC_PARAM_MANF_ID + 1];
	fdata->ffu_supported = (idata.buf_ptr[DEVICE_DESC_UFS_FEATURES_SUPPORT] &
							UFS_FEATURES_FFU) ? 1 : 0;

#ifdef UFS_FFU_QUERY_FW_VER_BY_STRING_DESCR
	prl_idx = idata.buf_ptr[DEVICE_DESC_PARAM_PRL];
#endif

	/* check if device disables fw update */

	idata.opcode = UPIU_QUERY_OPCODE_READ_FLAG;
	idata.idn = QUERY_FLAG_IDN_PERMANENTLY_DISABLE_FW_UPDATE;
	idata.idx = 0;
	memset(idata.buf_ptr, 0, FFU_COMMON_BUF_SIZE);
	idata.buf_byte = 1;

	err = ioctl(fd, UFS_IOCTL_QUERY, &idata);

	if (err) {
		printf("Read device flag failed, err: %d\n", err);
		err = UFS_UTIL_ERR_IOCTL_FAIL;
		goto out_close_fd;
	}

	fdata->ffu_disabled = idata.buf_ptr[0];

	printf("-- Device FFU Information --\n");
	printf("  dev_manu_id: 0x%X\n", fdata->dev_manu_id);
	printf("  ffu_supported: %d\n", (int)fdata->ffu_supported);
	printf("  ffu_disabled: %d\n", (int)fdata->ffu_disabled);

#ifdef UFS_FFU_QUERY_FW_VER_BY_INQUIRY_DATA

	/* get product revision level (firmware version) by scsi device instance */

	idata_fw.buf_ptr = &fdata->base_fw_ver[0];
	idata_fw.buf_byte = UFS_IOCTL_FFU_MAX_FW_VER_BYTES;		 /* only get 4 bytes here */
	memset(fdata->base_fw_ver, 0, UFS_FFU_BASE_RW_VER_BUF_SIZE);  /* remember to include tailed NULL, total 5 bytes */

	err = ioctl(fd, UFS_IOCTL_GET_FW_VER, &idata_fw);

	if (err) {
		printf("Query fw ver failed, err: %d\n", err);
		err = UFS_UTIL_ERR_IOCTL_FAIL;
		goto out_close_fd;
	}

#elif defined(UFS_FFU_QUERY_FW_VER_BY_STRING_DESCR)

	/* get product revision level (firmware version) by string descriptor */

	printf(" PRL index: %d\n", prl_idx);

	idata.opcode = UPIU_QUERY_OPCODE_READ_DESC;
	idata.idn = QUERY_DESC_IDN_STRING;
	idata.idx = prl_idx;
	memset(idata.buf_ptr, 0, FFU_COMMON_BUF_SIZE);
	idata.buf_byte = UFS_IOCTL_FFU_MAX_FW_VER_STRING_DESCR_BYTES;

	err = ioctl(fd, UFS_IOCTL_QUERY, &idata);

	if (err) {
		printf("Read string descriptor failed, err: %d\n", err);
		err = UFS_UTIL_ERR_IOCTL_FAIL;
		goto out_close_fd;
	}

	prl_idx = (idata.buf_ptr[0] - 2) / 2;

	if (prl_idx > UFS_IOCTL_FFU_MAX_FW_VER_BYTES) {
		printf("Warning: fw_ver size in string descr %d is larger than %d\n",
				prl_idx, UFS_IOCTL_FFU_MAX_FW_VER_BYTES);
		prl_idx = UFS_IOCTL_FFU_MAX_FW_VER_BYTES;
	}

	for (err = 0; err < prl_idx; err++)
		fdata->base_fw_ver[err] = idata.buf_ptr[2 + err * 2]);

#endif

	printf("  base_fw_ver: %s\n", fdata->base_fw_ver);

	printf("----------------------------\n");

out_close_fd:
	free(idata.buf_ptr);
	close(fd);
out:
	return err;
}

static int ufs_mtk_ffu_do(int fw_fd, int dev_fd)
{
	int ret = 0;
	int file_size;
	__u8 *ptr;
	struct ufs_ioctl_ffu_data idata;

	ptr = malloc(FFU_FW_SIZE);
	if (ptr == NULL) {
		printf("ptr malloc fail!\n");
		ret = -1;
		goto out;
	}
	memset(ptr, 0, FFU_FW_SIZE);

	/* get file size */
	file_size = lseek(fw_fd, 0, SEEK_END);

	if (file_size < 0) {
		ret = -1;
		printf("seek file error\n");
		goto out_free;
	}

	lseek(fw_fd, 0, SEEK_SET);

	/* Read FW data from file */
	ret = read(fw_fd, ptr, file_size);

	if (-1 == ret) {
		printf("read fw file failed\n");
		goto out_free;
	}

	printf("Read new fw file ok.\n");

	/* prepare and send ioctl */
	memset(&idata, 0, sizeof(struct ufs_ioctl_ffu_data));

	printf("FW size %d bytes, limit %d bytes\n", file_size, (unsigned int)UFS_IOCTL_FFU_MAX_FW_SIZE_BYTES);

	idata.buf_byte = file_size;
	idata.buf_ptr = ptr;

	ret = ioctl(dev_fd, UFS_IOCTL_FFU, &idata);

	if (ret)
		printf("ioctl FW download failed\n");

out_free:
	free(ptr);

out:
	return ret;
}

static int ufs_mtk_ffu_prepare(int argc, char **argv, struct ufs_ffu_data *fdata)
{
	int fd = 0;
	int fw_fd = 0;
	int err = 0;
	__u32 manu_id = 0ULL;

	/* ufs_util ffu_do <device> <image path> <vendor_id> <old_fw_revision> */

	/* check parameters */
	if (argc >= 5) {

		if (ufs_mtk_util_atoh(argv[4], &manu_id)) {
			printf("manu_id can contains only 0~9, a~f, or A-F\n");
			err = UFS_UTIL_ERR_MANUID_FORMAT_ERR;
			goto out;
		}

		if ((manu_id != 0) && (argc < 6)) {
			ufs_mtk_ffu_print_usage(ufs_mtk_util_name);
			err = UFS_UTIL_ERR_PARAM_ERR;
			goto out;
		}

		if (argc >= 6) {
			if (ufs_mtk_util_check_str_format(argv[5])) {
				printf("FW revision can contains only 0~9, a~f, or A-F\n");
				err = UFS_UTIL_ERR_FW_VER_FORMAT_ERR;
				goto out;
			}

			if (strlen(argv[5]) > UFS_IOCTL_FFU_MAX_FW_VER_BYTES) {
				printf("FW version length %d exceeds %d\n", (int)strlen(argv[5]),
						UFS_IOCTL_FFU_MAX_FW_VER_BYTES);
				err = UFS_UTIL_ERR_FW_VER_TOO_LONG;
				goto out;
			}
		}
	}

	printf("Parameter check ok.\n");

	/* check manu id */
	if (manu_id != 0) {
		if (manu_id != fdata->dev_manu_id) {
			printf("Manu ID is not matched, dev: 0x%X, desired: 0x%X\n", fdata->dev_manu_id, manu_id);
			err = UFS_UTIL_ERR_MANUID_MISMATCH;
			goto out;
		} else
			printf("Manu ID check ok.\n");

		/* check fw ver. only check this when manu_id != 0 */
		if (strcmp(argv[5], (const char *)fdata->base_fw_ver)) {
			printf("Base fw version mismatch, dev: %s, desired: %s\n", fdata->base_fw_ver, argv[5]);
			err = UFS_UTIL_ERR_FW_VER_MISMATCH;
			goto out;
		}
		else
			printf("Base fw version check ok.\n");
	}
	else
		printf("Manu ID/Current fw version check skipped.\n");

	/* open dev file */
	fd = open(argv[2], O_RDWR);

	if (fd < 0) {
		printf("Open device file failed\n");
		err = UFS_UTIL_ERR_OPEN_DEV_FAIL;
		goto out;
	} else
		printf("Open device file ok.\n");

	/* open firmware image file */
	fw_fd = open(argv[3], O_RDONLY);

	if (fw_fd < 0) {
		printf("Open firmware file failed\n");
		err = UFS_UTIL_ERR_OPEN_FW_FAIL;
		goto out;
	} else
		printf("Open new fw file ok.\n");

	/* now everything is ready, do ffu */
	err = ufs_mtk_ffu_do(fw_fd, fd);

	if (err)
		printf("FFU fail.\n");
	else {
		printf("FFU ok.\n");
		printf("Please reboot your system to let new firmware work.\n");
	}

out:
	if (fd && fd >= 0)
		close(fd);

	if (fw_fd && fw_fd >= 0)
		close(fw_fd);

	return err;
}

int ufs_mtk_ffu_check_dev(char *dev, struct ufs_ffu_data *fdata)
{
	int err;

	err = ufs_mtk_ffu_get_dev_info(dev, fdata);

	if (err) {
		printf("Fail to get device info\n");
		return err;
	}

	if (!fdata->ffu_supported) {
		printf("FFU is NOT supported by device\n");
		return UFS_UTIL_ERR_FFU_NOT_SUPPORTED;
	}

	if (fdata->ffu_disabled) {
		printf("FFU is supported but disabled by device\n");
		return UFS_UTIL_ERR_FFU_DISABLED;
	}

	return 0;
}

int main(int argc, char **argv)
{
	int err = 0;
	struct ufs_ffu_data fdata;

	memset(&fdata, 0, sizeof(struct ufs_ffu_data));

	ufs_mtk_util_name = ufs_mtk_ffu_get_prog_name(argv[0]);

	if (argc <= 2) {
		ufs_mtk_ffu_print_usage(ufs_mtk_util_name);
		exit(UFS_UTIL_ERR_PARAM_ERR);
	}

	if (argc > 2) {
		if (!strcmp(argv[1], "ffu_check")) {

			/* ufs_util ffu_check <device>  */

			err = ufs_mtk_ffu_check_dev(argv[2], &fdata);

			if (err)
				exit(err);

		} else if (!strcmp(argv[1], "ffu_do")) {

			/* ufs_util ffu_do <image path> <device> <vendor_id> <old_fw_revision> */

			if (argc < 5) {
				ufs_mtk_ffu_print_usage(ufs_mtk_util_name);
				exit(UFS_UTIL_ERR_PARAM_ERR);
			}

			err = ufs_mtk_ffu_check_dev(argv[2], &fdata);

			if (err)
				exit(err);

			err = ufs_mtk_ffu_prepare(argc, argv, &fdata);

			if (err)
				exit(err);
		}
	}

	return 0;
}

