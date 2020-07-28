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

#define __LIBBIOSENSOR_C__
/*---------------------------------------------------------------------------*/
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <dirent.h>
#include "libbiosensor.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "CFG_module_file.h"
#include "libfile_op.h"
#include "CFG_Biosensor_File.h"
#include "CFG_Biosensor_Default.h"
#include "Custom_NvRam_LID.h"

/*---------------------------------------------------------------------------*/
#undef  LOG_TAG
#define LOG_TAG "BIOLIB"
#include <cutils/log.h>
#include <linux/sensors_io.h>
/*---------------------------------------------------------------------------*/
#define BIOLOGD(fmt, arg ...) do {printf(fmt, ##arg); ALOGD(fmt, ##arg);} while(0)
#define BIOLOGE(fmt, arg ...) do {printf("%s: " fmt, __func__, ##arg); ALOGE("%s: " fmt, __func__, ##arg);} while(0)
/*----------------------------------------------------------------------------*/
int biosensor_calibration(int fd, struct BioData *dat)
{
    int err;
    struct biometric_cali cali;

    if (fd < 0) {
        BIOLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_DO_CALI, &cali))) {
        BIOLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->pga6 = (float)(cali.pga6) / 1000;
        dat->ambdac5_5 = (float)(cali.ambdac5_5) / 1000;
        BIOLOGD("[CAL] %f, %f, %d, %d\n", dat->pga6, dat->ambdac5_5, cali.pga6, cali.ambdac5_5);
    }
    return 0;
}
int biosensor_reset_nvram(void)
{
	if (NVM_ResetFileToDefault(AP_CFG_CUSTOM_FILE_BIOSENSOR_LID))
		return 0;
	else
		return -1;
}
int biosensor_write_nvram(struct BioData *dat)
{
    int file_lid = AP_CFG_CUSTOM_FILE_BIOSENSOR_LID;    
    int rec_size;
    int rec_num;
    bool bRet = false;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
    int res, fd=FileID.iFileDesc;
    File_Biosensor_Struct bio_cali;

    if (fd < 0) {
        BIOLOGE("nvram open = %d\n", fd);
        return fd;
    }

    bio_cali.pga6 = round(dat->pga6 * 1000);
    bio_cali.ambdac5_5 = round(dat->ambdac5_5 * 1000);
    BIOLOGD("[WN] %d, %d, %d, %d\n", dat->pga6, dat->ambdac5_5, bio_cali.pga6, bio_cali.ambdac5_5);
    res = write(fd, &bio_cali , rec_size*rec_num);
    if (res < 0) {
        BIOLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
    }
    NVM_CloseFileDesc(FileID);

    bRet = FileOp_BackupToBinRegion_All();
    sync();

    return !bRet;
}
int biosensor_read_nvram(struct BioData *dat)
{
    int file_lid = AP_CFG_CUSTOM_FILE_BIOSENSOR_LID;
    int rec_size;
    int rec_num;
    F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
    int res, fd=FileID.iFileDesc;

    File_Biosensor_Struct bio_cali;

    if(fd < 0) {
        BIOLOGE("nvram open = %d\n", fd);
        return fd;
    }

    res = read(fd, &bio_cali , rec_size*rec_num);
    if (res < 0) {
        dat->pga6 = dat->ambdac5_5 = 0;
        BIOLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
        return -1;
    } else {
        dat->pga6 = (float)(bio_cali.pga6) / 1000;
        dat->ambdac5_5 = (float)(bio_cali.ambdac5_5) / 1000;
        BIOLOGE("[RN] %d, %d, %d, %d\n", dat->pga6, dat->ambdac5_5, bio_cali.pga6, bio_cali.ambdac5_5);
    }

    NVM_CloseFileDesc(FileID);
    return 0;
}
int biosensor_rst_cali(int fd)
{
    int err, flags = 0;   
    if (fd < 0) {
        BIOLOGE("invalid file handle %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_CLR_CALI, &flags))) {
        BIOLOGE("rst_cali err: %d\n", err);
        return err;
    } else
        return 0;
}
int biosensor_set_cali(int fd, struct BioData *dat)
{
    int err;
    struct biometric_cali cali;

    cali.pga6 = round(dat->pga6 * 1000);
    cali.ambdac5_5 = round(dat->ambdac5_5 * 1000);

    BIOLOGD("[WD] %d, %d, %d, %d\n", dat->pga6, dat->ambdac5_5, cali.pga6, cali.ambdac5_5);
    if (fd < 0) {
        BIOLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if (0 != (err = ioctl(fd, BIOMETRIC_IOCTL_SET_CALI, &cali))) {
        BIOLOGE("set_cali err: %d, %s\n", err, strerror(errno));
        return err;
    }

    return 0;
}
int biosensor_get_cali(int fd, struct BioData *dat)
{
    int err;
    struct biometric_cali cali;

    if (fd < 0) {
        BIOLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_GET_CALI, &cali))) {
        BIOLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->pga6 = (float)(cali.pga6) / 1000;
        dat->ambdac5_5 = (float)(cali.ambdac5_5) / 1000;
        BIOLOGD("[RD] %d, %d, %d, %d\n", dat->pga6, dat->ambdac5_5, cali.pga6, cali.ambdac5_5);
    }
    return 0;
}
int biosensor_ftm_start(int fd)
{
    int err, flags = 0;   
    if (fd < 0) {
        BIOLOGE("invalid file handle %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_FTM_START, &flags))) {
        BIOLOGE("biosensor_ftm_start err: %d\n", err);
        return err;
    } else
        return 0;
}
int biosensor_ftm_end(int fd)
{
    int err, flags = 0;   
    if (fd < 0) {
        BIOLOGE("invalid file handle %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_FTM_END, &flags))) {
        BIOLOGE("biosensor_ftm_end err: %d\n", err);
        return err;
    } else
        return 0;
}
int biosensor_read_test_data(int fd, struct BioTestData *dat)
{
    int err;
    struct biometric_test_data data;

    if (fd < 0) {
        BIOLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_FTM_GET_DATA, &data))) {
        BIOLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->ppg_ir = (float)(data.ppg_ir);
        dat->ppg_r = (float)(data.ppg_r);
        dat->ekg = (float)(data.ekg);
        BIOLOGD("[RD] %f, %f, %f\n", dat->ppg_ir, dat->ppg_r, dat->ekg);
        BIOLOGD("[RD] %d, %d, %d\n", data.ppg_ir, data.ppg_r, data.ekg);
    }

	return 0;
}
int biosensor_read_test_threshold(int fd, struct BioThreshold *dat)
{
    int err;
    struct biometric_threshold threshold;

    if (fd < 0) {
        BIOLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    } else if(0 != (err = ioctl(fd, BIOMETRIC_IOCTL_FTM_GET_THRESHOLD, &threshold))) {
        BIOLOGE("get_cali err: %d\n", err);
        return err;
    } else {
        dat->ppg_ir_threshold = (float)(threshold.ppg_ir_threshold);
        dat->ppg_r_threshold = (float)(threshold.ppg_r_threshold);
        dat->ekg_threshold = (float)(threshold.ekg_threshold);
        BIOLOGD("[RD] %f, %f, %f\n", dat->ppg_ir_threshold, dat->ppg_r_threshold, dat->ekg_threshold);
        BIOLOGD("[RD] %d, %d, %d\n", threshold.ppg_ir_threshold, threshold.ppg_r_threshold, threshold.ekg_threshold);
    }
	return 0;
}
int biosensor_close(int fd)
{
    if (fd >= 0)
        close(fd);
    else
        BIOLOGE("%s() %d\n", __func__, fd);

    return -1;
}
int biosensor_open(int *fd)
{
    *fd = open(BIOMETRIC_NAME, O_RDONLY);

    if (*fd < 0) {
        BIOLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
        return -errno;
    }
    return 0;
}
