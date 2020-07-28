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
 * MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
//#include <dlfcn.h>
#include <linux/input.h>
#include <sys/resource.h>
#include <cutils/properties.h>
#include <errno.h>
#include <unistd.h>
#include <utils/String16.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fg_log.h"
//using namespace android;

#define FG_DATA_MOUNT_POINT	"/mnt/vendor/nvcfg"
#define FG_DATA_FOLDER		"/mnt/vendor/nvcfg/fg"
#define FG_CAR_TUNE_FILE	"/mnt/vendor/nvcfg/fg/car_tune_value"
#define Set_CARTUNE_TO_KERNEL _IOW('k', 15, int)

/* read NVRAM */
#include "libnvram.h"
#include "CFG_FG_File.h"
#include "CFG_FG_Default.h"
/*#include "../vendor/mediatek/proprietary/custom/common/cgen/inc/CFG_file_lid.h"*/
#include "Custom_NvRam_LID.h"

struct saved_car_tune_data
{
	int is_valid;
	int car_tune_value;
};

struct saved_car_tune_data car_tune_data;

int FG_DM_read_NVRAM(int* value)
{
	ap_nvram_fg_config_struct stFGReadback;
	// int fg_nvram_fd = 0;
	F_ID fg_nvram_fd;
	int rec_size;
	int retsize = 0;
	int rec_num;
	unsigned int numbers;
	bool IsRead = true;

	fg_nvram_fd.iFileDesc = 0;
	fg_nvram_fd.ifile_lid = 0;
	fg_nvram_fd.bIsRead = 0;

	FGLOG_ERROR("[FG_DM_read_NVRAM] LID %d, size %d\n", AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, sizeof(stFGReadback));

	memset(&stFGReadback, 0, sizeof(stFGReadback));

	fg_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, &rec_size, &rec_num, IsRead);
	FGLOG_ERROR("read NVRAM, rec_size %d, rec_num %d\n", rec_size, rec_num);
	if (fg_nvram_fd.iFileDesc > 0) {   /* >0 means ok */
		FGLOG_ERROR("FG read NVRam ok, fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
		numbers = rec_size*rec_num;
		if ( numbers > sizeof(stFGReadback) ) {
			FGLOG_ERROR("FG NVRam block size mismatch\n");
			NVM_CloseFileDesc(fg_nvram_fd);
			FGLOG_ERROR("FG Close NVRam fd ok, fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
			return -1;
		}

		retsize = read(fg_nvram_fd.iFileDesc, &stFGReadback , rec_size*rec_num);
		if (retsize == -1 || rec_size*rec_num != retsize ) {
			FGLOG_ERROR("read NVRAM error, %s\n", strerror(errno));
			FGLOG_ERROR("FG Close NVRam fd, fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
			NVM_CloseFileDesc(fg_nvram_fd);
			return -1;
		}
		NVM_CloseFileDesc(fg_nvram_fd);
		FGLOG_ERROR("FG Close NVRam fd ok, fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
	
		if ( strnlen(stFGReadback.dsp_dev, 20) !=0 ) {
			FGLOG_ERROR("FG NVRam (%d * %d) : \n", rec_size, rec_num);
			FGLOG_ERROR("dsp_dev : %s\n", stFGReadback.dsp_dev);
			FGLOG_ERROR("nvram_car_tune_value : %d\n", stFGReadback.nvram_car_tune_value);
			*value = stFGReadback.nvram_car_tune_value;
		} else {
			FGLOG_ERROR("FG NVRam mnl_config.dev_dsp == NULL \n");
			return -1;
		}
	} else {
		FGLOG_ERROR("FG read NVRam fail, fg_nvram_fd == %d \n", fg_nvram_fd.iFileDesc);
		return -1;
	}
	return 0;
}

int write_car_tune_file(void)
{
	int fd, ret;

	fd = open(FG_CAR_TUNE_FILE, O_CREAT|O_WRONLY|O_TRUNC, 0777);

	if (fd < 0) {
		FGLOG_ERROR("Error: open %s fail[%d:%d]!!!\n", FG_CAR_TUNE_FILE, fd, errno);
		return -1;
	}

	ret = write(fd, &car_tune_data, sizeof(saved_car_tune_data));

	if ((ret < 0) || (ret != sizeof(saved_car_tune_data))){
		FGLOG_ERROR("Error: write %s fail[%d][%d:%d]!!!\n", FG_CAR_TUNE_FILE, errno, ret, sizeof(saved_car_tune_data));
		close(fd);
		return -1;
	}

	close(fd);
	FGLOG_ERROR("write_car_tune_file write success [%d] isvalid:%d %d\n", ret,
		car_tune_data.is_valid, car_tune_data.car_tune_value);
	return 0;
}

int write_car_tune_data(int is_valid, int value)
{
	int ret;
	struct stat st;

	if (stat(FG_DATA_MOUNT_POINT, &st) == -1) {
		FGLOG_ERROR("%s is not mounted\n", FG_DATA_MOUNT_POINT);
		return -1;
	}

	if (stat(FG_DATA_FOLDER, &st) == -1) {
		FGLOG_ERROR("%s is not created\n", FG_DATA_FOLDER);
		if(mkdir(FG_DATA_FOLDER, 0777) == -1)
		{
			FGLOG_ERROR("write_car_tune_data mkdir error! %s\n", FG_DATA_FOLDER);
		}
	}

	if (stat(FG_CAR_TUNE_FILE, &st) == -1) {
		FGLOG_ERROR("%s does not exist\n", FG_CAR_TUNE_FILE);
	}

	car_tune_data.is_valid = is_valid;
	car_tune_data.car_tune_value = value;
	ret = write_car_tune_file();

	return ret;
}

int send_car_tune_to_kernel(int car_tune)
{
	int meta_adc_fd = 0;
	int adc_out_data[2] = {1, 1};
	int ret = 0;

        meta_adc_fd = open("/dev/MT_pmic_adc_cali", O_RDWR);
        if (meta_adc_fd == -1) {
                FGLOG_ERROR("fgauge_nvram ERROR!!!!!!Open /dev/MT_pmic_adc_cali : ERROR \n");
                FGLOG_ERROR("fgauge_nvram Open /dev/MT_pmic_adc_cali : ERROR \n");
                return -1;
        }

	adc_out_data[0] = 1;
	adc_out_data[1] = car_tune;
	FGLOG_ERROR("fgauge_nvram cali car tune %d, valid\n", car_tune);
	ret = ioctl(meta_adc_fd, Set_CARTUNE_TO_KERNEL, adc_out_data);

	if (ret == -1) {
		FGLOG_ERROR("fgauge_nvram Set_CARTUNE_TO_KERNEL,ERROR!!!!!!! \n");
		close(meta_adc_fd);
		return -1;
	} else {
		close(meta_adc_fd);
		return 0;
	}


}

int main(int argc, char *argv[])
{
	char nvram_init_val[PROPERTY_VALUE_MAX];
	int val = 0;
	int ret = 0;
	int timeout = 0;
	int fix_warning = 0;
	char **fix_warning2;

	fix_warning = argc;
	fix_warning2 = argv;

	fgdlog_init();
	fgdlog_set_level(6);

	FGLOG_ERROR("fgauge_nvram: Start!\n");
	while(1) {
		property_get("vendor.service.nvram_init", nvram_init_val, NULL);
		if(strcmp(nvram_init_val, "Ready") == 0) {
			ret = FG_DM_read_NVRAM(&val);
			if (ret == -1) {
				timeout++;
				if (timeout >= 10) {
					FGLOG_ERROR("fgauge_nvram read from nvram timeout\n");
					break;
				}
			} else {
				FGLOG_ERROR("fgauge_nvram read from nvram success [%d]\n", val);
				break;
			}
		}
		FGLOG_ERROR("fgauge_nvram: service.nvram_init=%s\n",nvram_init_val);
		usleep(5000000);
	}

	if ((val > 500) && (val < 1500)) {
		send_car_tune_to_kernel(val);
		write_car_tune_data(1, val);
		FGLOG_ERROR("fgauge_nvram: cali car tune %d, valid\n", val);
	} else {
		write_car_tune_data(0, 2048);
		FGLOG_ERROR("fgauge_nvram: cali car tune %d, invalid\n", val);
	}
	
	return 0;
}

