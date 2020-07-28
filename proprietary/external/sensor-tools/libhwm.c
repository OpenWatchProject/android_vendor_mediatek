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

#define __LIBHWM_C__
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
#include "libhwm.h"
#include "libnvram.h"
#include "CFG_file_lid.h"
#include "CFG_module_file.h"
#include "libfile_op.h"

/*---------------------------------------------------------------------------*/
#undef  LOG_TAG
#define LOG_TAG "HWMLIB"
#include <log/log.h>
#include <linux/sensors_io.h>
/*---------------------------------------------------------------------------*/
#define HWMLOGD(fmt, arg ...) do {printf(fmt, ##arg); ALOGD(fmt, ##arg);} while(0)
#define HWMLOGE(fmt, arg ...) do {printf("%s: " fmt, __func__, ##arg); ALOGE("%s: " fmt, __func__, ##arg);} while(0)
/*----------------------------------------------------------------------------*/
#define DIVERSE_X   0x0008
#define DIVERSE_Y   0x0004
#define DIVERSE_Z   0x0002
#define DIVERSE_XYZ 0x0001
#define C_MAX_HWMSEN_EVENT_NUM 4

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
//
//      Accelerometer Self-Test
//
/*---------------------------------------------------------------------------*/
int gsensor_selftest(int enable)
{
	int err, fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
	char buf[] = {enable + '0'};

	if(fd == -1)
	{
		HWMLOGE("open gsensor err = %s\n", strerror(errno));
		return -errno;
	}
	
	do{
		err = write(fd, buf, sizeof(buf) ); 
	}while(err < 0 && errno == EINTR);

	if(err != sizeof(buf))
	{ 
		HWMLOGE("write fails = %s\n", strerror(errno));
		err = -errno;
	}
	else
	{
		err = 0;    /*no error*/
	}
	
	if(close(fd) == -1)
	{
		HWMLOGE("close fails = %s\n", strerror(errno));
		err = (err) ? (err) : (-errno);
	}
	return err;    
}
/*----------------------------------------------------------------------------*/
static int gsensor_judge_selftest_result(int prv[C_MAX_HWMSEN_EVENT_NUM], int nxt[C_MAX_HWMSEN_EVENT_NUM])
{
    struct criteria {
        int min;
        int max;
    };
	struct criteria self[4][3] = {
		{{50, 540}, {-540, -50}, {75, 875}},
		{{25, 270}, {-270, -25}, {38, 438}},
		{{12, 135}, {-135, -12}, {19, 219}},
		{{ 6,  67}, {-67,  -6},  {10, 110}},
	};
	struct criteria (*ptr)[3] = &self[0]; /*choose full resolution*/
	int res = 0;

	if(((nxt[0] - prv[0]) > (*ptr)[0].max) ||((nxt[0] - prv[0]) < (*ptr)[0].min))
	{
		HWMLOGE("X is over range\n");
		res = -EINVAL;
	}
	
	if(((nxt[1] - prv[1]) > (*ptr)[1].max) ||((nxt[1] - prv[1]) < (*ptr)[1].min))
	{
		HWMLOGE("Y is over range\n");
		res = -EINVAL;
	}
	
	if(((nxt[2] - prv[2]) > (*ptr)[2].max) ||((nxt[2] - prv[2]) < (*ptr)[2].min))
	{
		HWMLOGE("Z is over range\n");
		res = -EINVAL;
	}
	return res;
}

/*---------------------------------------------------------------------------*/
int gsensor_enable_selftest(int enable)
{ 
	int err, fd = open(GSENSOR_ATTR_SELFTEST, O_RDWR);
	char buf[] = {enable + '0'};

	if(fd == -1)
	{
		HWMLOGD("open gsensor err = %s\n", strerror(errno));
		return -errno;
	}
	
	do{
		err = write(fd, buf, sizeof(buf) ); 
	}while(err < 0 && errno == EINTR);

	if(err != sizeof(buf))
	{ 
		HWMLOGD("write fails = %s\n", strerror(errno));
		err = -errno;
	}
	else
	{
		err = 0;    /*no error*/
	}
	
	if(close(fd) == -1)
	{
		HWMLOGD("close fails = %s\n", strerror(errno));
		err = (err) ? (err) : (-errno);
	}
	return err;
    
}

/*---------------------------------------------------------------------------*/
int gsensor_self_test(int fd, int count, HwmPrivate *prv)
{
	struct item{
		int raw[C_MAX_HWMSEN_EVENT_NUM];
	};
	int idx, res, x,y,z;
	struct item *pre = NULL, *nxt = NULL;
	char buf[60];  
	int avg_prv[C_MAX_HWMSEN_EVENT_NUM] = {0, 0, 0};
	int avg_nxt[C_MAX_HWMSEN_EVENT_NUM] = {0, 0, 0};

	pre = malloc(sizeof(*pre) * count);
	nxt = malloc(sizeof(*nxt) * count);
	if(!pre || !nxt)
	{
		goto exit;
	}
	
	memset(pre, 0x00, sizeof(*pre) * count); 
	memset(nxt, 0x00, sizeof(*nxt) * count); 
	HWMLOGD("NORMAL:\n");
	for (idx = 0; idx < count; idx++)
	{
		res = ioctl(fd, GSENSOR_IOCTL_READ_RAW_DATA, &buf);

		if(res)
		{            
			HWMLOGD("read data fail: %d\n", res);
			goto exit;
		}
		sscanf(buf, "%x %x %x", &x, &y, &z);
        nxt[idx].raw[0] = x;
        nxt[idx].raw[1] = y;
        nxt[idx].raw[2] = z;
        avg_nxt[0] += nxt[idx].raw[0];
        avg_nxt[1] += nxt[idx].raw[1];
        avg_nxt[2] += nxt[idx].raw[2];  		
		       
		HWMLOGD("[%5d %5d %5d]\n", pre[idx].raw[0], pre[idx].raw[1], pre[idx].raw[2]);
	}
	avg_prv[0] /= count;
	avg_prv[1] /= count;
	avg_prv[2] /= count;    

	/*initial setting for self test*/
	if(0 != (res = gsensor_enable_selftest(1)))
	{
		goto exit;
	}
	
	HWMLOGD("SELFTEST:\n");    
	for (idx = 0; idx < count; idx++)
	{
		res = ioctl(fd, GSENSOR_IOCTL_READ_RAW_DATA, &buf);
		if(res)
		{            
			HWMLOGD("read data fail: %d\n", res);
			goto exit;
		}
		sscanf(buf, "%x %x %x", &x, &y, &z);
        nxt[idx].raw[0] = x;
        nxt[idx].raw[1] = y;
        nxt[idx].raw[2] = z;                
		avg_nxt[0] += nxt[idx].raw[0];
		avg_nxt[1] += nxt[idx].raw[1];
		avg_nxt[2] += nxt[idx].raw[2];        
		HWMLOGD("[%5d %5d %5d]\n", nxt[idx].raw[0], nxt[idx].raw[1], nxt[idx].raw[2]);
	}
	avg_nxt[0] /= count;
	avg_nxt[1] /= count;
	avg_nxt[2] /= count;    

	HWMLOGD("X: %5d - %5d = %5d \n", avg_nxt[0], avg_prv[0], avg_nxt[0] - avg_prv[0]);
	HWMLOGD("Y: %5d - %5d = %5d \n", avg_nxt[1], avg_prv[1], avg_nxt[1] - avg_prv[1]);
	HWMLOGD("Z: %5d - %5d = %5d \n", avg_nxt[2], avg_prv[2], avg_nxt[2] - avg_prv[2]); 

	if(0 == gsensor_judge_selftest_result(avg_prv, avg_nxt))
	{
		HWMLOGD("SELFTEST : PASS\n");
	}
	else
	{
		HWMLOGD("SELFTEST : FAIL\n");
	}
	exit:
	/*restore the setting*/    
	res = gsensor_enable_selftest(0);
	free(pre);
	free(nxt);
	if(prv && prv->ptr && (prv->len == 2*sizeof(HwmData)))
	{
		HwmData *dat = (HwmData*)prv->ptr;
		dat[0].rx = avg_prv[0];
		dat[0].ry = avg_prv[1];
		dat[0].rz = avg_prv[1];
		dat[1].rx = avg_nxt[0];
		dat[1].ry = avg_nxt[1];
		dat[1].rz = avg_nxt[1];
	}
	return res;    
}
/*---------------------------------------------------------------------------*/
//
//      Accelerometer Interface function
//
/*---------------------------------------------------------------------------*/
int gsensor_open(int *fd)
{
	if(*fd <= 0)
	{
		*fd = open(GSENSOR_NAME, O_RDONLY);	
	
		if(*fd < 0)
		{
			HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
			return -errno;
		}    
	}
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_close(int fd)
{
    if(fd > 0)
	{
		close(fd);        
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_init(int fd)
{
    int err;
    unsigned long flag=1;
	
    if(0 != (err = ioctl(fd, GSENSOR_IOCTL_INIT, &flag)))
    {
        HWMLOGE("Accelerometer init err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    }
    return 0;
}

int gsensor_read(int fd, HwmData *dat)
{
	int err;
	int x, y, z;
	char buf[64];
	if(fd < 0)
	{
		HWMLOGE("invalid file handle!\n");
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GSENSOR_IOCTL_READ_SENSORDATA, buf)))
	{
		HWMLOGE("read err: %d %d (%s)\n", fd, err, strerror(errno));    
		return err;
	}
	else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
	{
		HWMLOGE("parsing error\n");
		return -EINVAL;
	}
	else
	{
		dat->x = (float)(x) / 1000;
		dat->y = (float)(y) / 1000;
		dat->z = (float)(z) / 1000;
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gsensor_get_cali(int fd, HwmData *dat)
{
	int err;
	SENSOR_DATA cali;

	if(fd < 0)
	{
	    HWMLOGE("invalid file handle: %d\n", fd);
	    return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GSENSOR_IOCTL_GET_CALI, &cali)))
	{
	    HWMLOGE("get_cali err: %d\n", err);
	    return err;
	}
	else
	{
	    dat->x = (float)(cali.x) / 1000;
	    dat->y = (float)(cali.y) / 1000;
	    dat->z = (float)(cali.z) / 1000;
	    HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x,cali.y, cali.z);                
	    return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gsensor_set_cali(int fd, HwmData *dat)
{
	int err;
	SENSOR_DATA cali;

	cali.x = round(dat->x * 1000);
	cali.y = round(dat->y * 1000);
	cali.z = round(dat->z * 1000);
	HWMLOGD("[WD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x,cali.y, cali.z);
	if(fd < 0)
	{
		HWMLOGE("invalid file handle: %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GSENSOR_IOCTL_SET_CALI, &cali)))
	{
		HWMLOGE("set_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gsensor_rst_cali(int fd)
{
	int err, flags = 0;   
	if(fd < 0)
	{
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GSENSOR_IOCTL_CLR_CALI, &flags)))
	{
		HWMLOGE("rst_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gsensor_enable_cali(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, GSENSOR_IOCTL_ENABLE_CALI, &flags))) {
		HWMLOGE("enable cali err: %d\n", err);
		return err;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_do_selftest(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, GSENSOR_IOCTL_SELF_TEST, &flags))) {
		HWMLOGE("self_test err: %d\n", err);
		return err;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_read_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_ACC_LID;    
    int rec_size;
    int rec_num;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	int res, fd=FileID.iFileDesc;

    NVRAM_HWMON_ACC_STRUCT hwmonAcc =
    {    
        {0, 0, 0},
    };    
    
    if(fd < 0)
	{
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }
    
   	res = read(fd, &hwmonAcc , rec_size*rec_num);
    if(res < 0)
	{
        dat->x = dat->y = dat->z = 0.0;
        HWMLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
    }
	else
	{
        dat->x = (hwmonAcc.offset[0]*LIBHWM_GRAVITY_EARTH)/LIBHWM_ACC_NVRAM_SENSITIVITY;
        dat->y = (hwmonAcc.offset[1]*LIBHWM_GRAVITY_EARTH)/LIBHWM_ACC_NVRAM_SENSITIVITY;
        dat->z = (hwmonAcc.offset[2]*LIBHWM_GRAVITY_EARTH)/LIBHWM_ACC_NVRAM_SENSITIVITY;
        HWMLOGD("[RN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonAcc.offset[0], hwmonAcc.offset[1], hwmonAcc.offset[2]);        
    }
        
    NVM_CloseFileDesc(FileID);
#else
    dat->x = 0;
    dat->y = 0;
    dat->z = 0;
#endif
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_write_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
	int file_lid = AP_CFG_RDCL_HWMON_ACC_LID;    
	int rec_size;
	int rec_num;
	bool bRet = false;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	int res, fd=FileID.iFileDesc;
	NVRAM_HWMON_ACC_STRUCT hwmonAcc =
	{    
		{0, 0, 0},
	};    

	if(fd < 0)
	{
		HWMLOGE("nvram open = %d\n", fd);
		return fd;
	}

	hwmonAcc.offset[0] = round((dat->x*LIBHWM_ACC_NVRAM_SENSITIVITY)/LIBHWM_GRAVITY_EARTH);
	hwmonAcc.offset[1] = round((dat->y*LIBHWM_ACC_NVRAM_SENSITIVITY)/LIBHWM_GRAVITY_EARTH);
	hwmonAcc.offset[2] = round((dat->z*LIBHWM_ACC_NVRAM_SENSITIVITY)/LIBHWM_GRAVITY_EARTH);
	HWMLOGD("[WN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonAcc.offset[0], hwmonAcc.offset[1], hwmonAcc.offset[2]);
	res = write(fd, &hwmonAcc , rec_size*rec_num);
	if(res < 0)
	{
		HWMLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
	}
	NVM_CloseFileDesc(FileID); 

	//NVM_AddBackupFileNum(AP_CFG_RDCL_HWMON_ACC_LID); // risk of lost NVRAM BIN region data, when lose power
	bRet = FileOp_BackupToBinRegion_All();
	sync();

  return !bRet;
#else
    HWMLOGD("gsensor [WN] %9.4f %9.4f %9.4f\n", dat->x, dat->y, dat->z);
#endif
    return 0;
}


/*---------------------------------------------------------------------------*/
int libhwm_wait_delay(int ms) 
{
	struct timespec req = {.tv_sec = 0, .tv_nsec = ms*1000000};
	struct timespec rem;
	int ret = nanosleep(&req, &rem);

	while(ret)
	{
		if(errno == EINTR)
		{
			req.tv_sec  = rem.tv_sec;
			req.tv_nsec = rem.tv_nsec;
			ret = nanosleep(&req, &rem);
		}
		else
		{
			perror("nanosleep");
			return errno;
		}
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
long libhwm_current_ms(void) 
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	//HWMLOGD("%ld:%ld\n", ts.tv_sec, ts.tv_nsec/1000000);
	return (long)(ts.tv_nsec/1000000) + ts.tv_sec*1000;
}

/*---------------------------------------------------------------------------*/
//
//      Accelerometer Implementation
//
/*---------------------------------------------------------------------------*/
typedef struct {
    long long time;
    HwmData dat;
} AccItem;    

/*---------------------------------------------------------------------------*/
int gsensor_update_nvram(HwmData *dat)
{
	int err;
	HwmData old, cur;

	if(0 != (err = gsensor_read_nvram(&old)))
	{
		return err;
	}
	
	cur.x = old.x + dat->x;
	cur.y = old.y + dat->y;
	cur.z = old.z + dat->z;

	if(0 != (err = gsensor_write_nvram(&cur)))
	{
		return err;
	}

	return 0;    
}
/*---------------------------------------------------------------------------*/
int gsensor_reset_nvram()
{
	int err;
	HwmData cur;

	cur.x = cur.y = cur.z = 0.0;
	if(0 != (err = gsensor_write_nvram(&cur)))
	{
		return err;
	}

	return 0;
}
/*----------------------------------------------------------------------------*/
int checkAccelerometerData(AccItem *list, int num, HwmData *avg, int tolerance) 
{
	float maxdiff = (LIBHWM_GRAVITY_EARTH*tolerance)/100.0;
	HwmData min, max;
	char  chkstr[1024];
	float diffx, diffy, diffz, varx = 0, vary = 0, varz = 0;
	int idx;
	int diverse = 0, curdiv;   

	min.x = min.y = min.z = +100*LIBHWM_GRAVITY_EARTH;
	max.x = max.y = max.z = -100*LIBHWM_GRAVITY_EARTH;
	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("                         Calibration Data                       \n");
	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("maxdiff = %+9.4f\n", maxdiff);    
	HWMLOGD("average = %+9.4f, %+9.4f %+9.4f\n", avg->x, avg->y, avg->z);    
	HWMLOGD("----------------------------------------------------------------\n");   
	for(idx = 0; idx < num; idx++)
	{       
		if(max.x < list[idx].dat.x)
		{
			max.x = list[idx].dat.x;
		}

		if(max.y < list[idx].dat.y)
		{
			max.y = list[idx].dat.y;
		}
		
		if(max.z < list[idx].dat.z)
		{
			max.z = list[idx].dat.z;
		}
		
		if(min.x > list[idx].dat.x)
		{
			min.x = list[idx].dat.x;
		}

		if(min.y > list[idx].dat.y)
		{
			min.y = list[idx].dat.y;
		}

		if(min.z > list[idx].dat.z)
		{
			min.z = list[idx].dat.z;
		}
		
		diffx = list[idx].dat.x - avg->x;
		diffy = list[idx].dat.y - avg->y;
		diffz = list[idx].dat.z - avg->z;
		varx += diffx * diffx;
		vary += diffy * diffy;
		varz += diffz * diffz; 

		curdiv = 0;
		if(ABS(diffx) > maxdiff) 
		{
			curdiv |= DIVERSE_X;
		}

		if(ABS(diffy) > maxdiff)
		{
			curdiv |= DIVERSE_Y;
		}

		if(ABS(diffz) > maxdiff)
		{
			curdiv |= DIVERSE_Z;
		}

		if((diffx*diffx + diffy*diffy + diffz*diffz) > maxdiff*maxdiff) 
		{
			curdiv |= DIVERSE_XYZ;       
		}
		diverse |= curdiv;

		if (curdiv)
		{
			snprintf(chkstr, sizeof(chkstr), "=> UNSTABLE: 0x%04X, %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f)", 
				curdiv, diffx, diffx/LIBHWM_GRAVITY_EARTH, diffy, diffy/LIBHWM_GRAVITY_EARTH, diffz, diffz/LIBHWM_GRAVITY_EARTH, 
				sqrt(diffx*diffx + diffy*diffy + diffz*diffz), sqrt(diffx*diffx + diffy*diffy + diffz*diffz)/LIBHWM_GRAVITY_EARTH);
		}
		else
		{
			snprintf(chkstr, sizeof(chkstr), " ");
		}
		
		HWMLOGD("[%8lld] (%+9.4f, %+9.4f, %+9.4f) %s\n", list[idx].time/1000000,
		list[idx].dat.x, list[idx].dat.y, list[idx].dat.z, chkstr);
	}
	
	varx = sqrt(varx/num);
	vary = sqrt(vary/num);
	varz = sqrt(varz/num);

	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("X-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.x, avg->x, max.x, (min.x-avg->x)/LIBHWM_GRAVITY_EARTH, (max.x-avg->x)/LIBHWM_GRAVITY_EARTH, varx);  
	HWMLOGD("Y-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.y, avg->y, max.y, (min.y-avg->y)/LIBHWM_GRAVITY_EARTH, (max.y-avg->y)/LIBHWM_GRAVITY_EARTH, vary);  
	HWMLOGD("Z-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.z, avg->z, max.z, (min.z-avg->z)/LIBHWM_GRAVITY_EARTH, (max.z-avg->z)/LIBHWM_GRAVITY_EARTH, varz);  
	HWMLOGD("----------------------------------------------------------------\n");    
	if(diverse)
	{
		return -EINVAL;
	}

	return 0;    
}
/*---------------------------------------------------------------------------*/
//Parse event and return event type
/*---------------------------------------------------------------------------*/
int gsensor_poll_data(int fd, int period, int count) 
{
	int64_t nt;
	struct timespec time;    
    int err, num = 0;
	AccItem *item = NULL;    
	HwmData avg, dat;

	avg.x = avg.y = avg.z = 0.0;
	if(fd < 0)
	{
		return -EINVAL;
	}
	if(NULL == (item = calloc(count, sizeof(*item))))
	{
		return -ENOMEM;
	}
	
	while(0 == (err = gsensor_read(fd, &dat))) 
	{
		clock_gettime(CLOCK_MONOTONIC, &time);        
		nt = time.tv_sec*1000000000LL+time.tv_nsec;                 
		item[num].dat.x = dat.x;
		item[num].dat.y = dat.y;
		item[num].dat.z = dat.z;
		item[num].time = nt;            
		avg.x += dat.x;
		avg.y += dat.y;
		avg.z += dat.z;

		if(++num >= count)
		{
			break;
		}
		libhwm_wait_delay(period);
	}
	
	avg.x /= count;
	avg.y /= count;
	avg.z /= count;   
	checkAccelerometerData(item, count, &avg, 0.0);

	free(item);
	return err;
}

int getGravityStandard(float in[C_MAX_HWMSEN_EVENT_NUM], HwmData *out)
{

    out->x = in[0]/LIBHWM_GRAVITY_EARTH;
    out->y = in[1]/LIBHWM_GRAVITY_EARTH;
    out->z = in[2]/LIBHWM_GRAVITY_EARTH;

    HWMLOGD("%9.4f %9.4f %9.4f => %9.4f %9.4f %9.4f\n", in[0], in[1], in[2],
            out->x, out->y, out->z);
    return 0;
}

/*---------------------------------------------------------------------------*/
int calculateStandardCalibration(HwmData *avg, HwmData *cali)
{
    HwmData golden;
    golden.x = golden.y = 0.0; 
    golden.z = LIBHWM_GRAVITY_EARTH;

    cali->x = golden.x - avg->x;
    cali->y = golden.y - avg->y;
    cali->z = golden.z - avg->z;
    HWMLOGD("%s (%9.4f, %9.4f, %9.4f)\n", __func__, cali->x, cali->y, cali->z);
    return 0;
}

/*---------------------------------------------------------------------------*/
int gsensor_calibration(int fd, int period, int count, int tolerance, HwmData *cali) 
{   
    int err = 0, num = 0;
	AccItem *item = NULL;
	HwmData avg, dat;
	int64_t nt;
    struct timespec time;

	avg.x = avg.y = avg.z = 0.0;    
	if(fd < 0)
	{
		return -EINVAL;
	}
	if(NULL == (item = calloc(count, sizeof(*item))))
	{
		return -ENOMEM;
	}
	
	while(num < count)
	{
		/* read the next event */
		err = gsensor_read(fd, &dat);

		if(err)
		{
			HWMLOGE("read data fail: %d\n", err);
			goto exit;
		}
		else
		{
			clock_gettime(CLOCK_MONOTONIC, &time);        
			nt = time.tv_sec*1000000000LL+time.tv_nsec;                 
			item[num].dat.x = dat.x;
			item[num].dat.y = dat.y;
			item[num].dat.z = dat.z;
			item[num].time = nt;
			avg.x += item[num].dat.x;
			avg.y += item[num].dat.y;
			avg.z += item[num].dat.z;            
		}
		
		num++;
		libhwm_wait_delay(period);
	}

	/*calculate average*/
	avg.x /= count;
	avg.y /= count;
	avg.z /= count;

	if(0 != (err = checkAccelerometerData(item, count, &avg, tolerance)))
	{        
		HWMLOGE("check accelerometer fail\n");
	}
	else if(0 != (err = calculateStandardCalibration(&avg, cali)))
	{
		HWMLOGE("calculate standard calibration fail\n");
	}
	exit:    
	free(item);
	return err;
}
/*---------------------------------------------------------------------------*/
int gsensor_start_static_calibration(void)
{
    int fd = 0;
    int err, flags = 1;

    fd = open(GSENSOR_NAME, O_RDONLY);
    if (fd < 0) {
        HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
        return -errno;
    }

    if (0 != (err = ioctl(fd, GSENSOR_IOCTL_ENABLE_CALI, &flags))) {
        HWMLOGE("enable cali err: %d\n", err);
        close(fd);
        return err;
    }

    close(fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
int gsensor_get_static_calibration(struct caliData *caliDat)
{
    int err;
    SENSOR_DATA cali;
    int fd = 0;

    fd = open(GSENSOR_NAME, O_RDONLY);
    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    }
    if (0 != (err = ioctl(fd, GSENSOR_IOCTL_GET_CALI, &cali))) {
        HWMLOGE("get_cali err: %d\n", err);
        close(fd);
        return err;
    }

    caliDat->data[0] = (float)(cali.x) / 1000;
    caliDat->data[1] = (float)(cali.y) / 1000;
    caliDat->data[2] = (float)(cali.z) / 1000;
    HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", caliDat->data[0], caliDat->data[1], caliDat->data[2], cali.x,cali.y, cali.z);

    close(fd);
    return 0;

}
/*---------------------------------------------------------------------------*/
/* Als cali */
int als_start_static_calibration(void)
{
    int fd = 0;
    int err, flags = 1;

    fd = open(ALSPS_NAME, O_RDONLY);
    if (fd < 0) {
        HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
        return -errno;
    }

    if (0 != (err = ioctl(fd, ALSPS_ALS_ENABLE_CALI, &flags))) {
        HWMLOGE("enable cali err: %d\n", err);
        close(fd);
        return err;
    }

    close(fd);
    return 0;

}
int als_get_static_calibration(struct caliData *caliDat)
{
    int err;
    SENSOR_DATA cali;
    int fd = 0;

    fd = open(ALSPS_NAME, O_RDONLY);
    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    }
    if (0 != (err = ioctl(fd, ALSPS_IOCTL_ALS_GET_CALI, &cali))) {
        HWMLOGE("get_cali err: %d\n", err);
        close(fd);
        return err;
    }

    caliDat->data[0] = (float)(cali.x) / 1000;
    HWMLOGD("[RD] %9.4f  =>  %5d \n", caliDat->data[0], cali.x);

    close(fd);
    return 0;

}
/* gyroscope calibration */
int gyroscope_open(int *fd)
{
	if(*fd <= 0)
	{
		*fd = open(GYROSCOPE_NAME, O_RDONLY);	
	
		if(*fd < 0)
		{
			HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
			return -errno;
		}    
	}
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_close(int fd)
{
    if(fd >= 0)
	{
		close(fd);        
    }
    
    return 0;
}

int gyroscope_init(int fd)
{
    int err;
    unsigned long flag =0;
	
    if(0 != (err = ioctl(fd, GYROSCOPE_IOCTL_INIT, &flag)))
    {
        HWMLOGE("Gyroscope init err: %d %d (%s)\n", fd, err, strerror(errno));
        return err;
    }
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_read(int fd, HwmData *dat)
{
	int err;
	int x, y, z;
	char buf[64];
	if(fd < 0)
	{
		HWMLOGE("invalid file handle!\n");
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GYROSCOPE_IOCTL_READ_SENSORDATA, buf)))
	{
		HWMLOGE("read err: %d %d (%s)\n", fd, err, strerror(errno));    
		return err;
	}
	else if(3 != sscanf(buf, "%x %x %x", &x, &y, &z))
	{
		HWMLOGE("parsing error\n");
		return -EINVAL;
	}
	else
	{
		dat->x = (float)(x);
		dat->y = (float)(y);
		dat->z = (float)(z);
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gyroscope_get_cali(int fd, HwmData *dat)
{
	int err;
	SENSOR_DATA cali;

	if(fd < 0)
	{
	    HWMLOGE("invalid file handle: %d\n", fd);
	    return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GYROSCOPE_IOCTL_GET_CALI, &cali)))
	{
	    HWMLOGE("get_cali err: %d\n", err);
	    return err;
	}
	else
	{
	    dat->x = (float)(cali.x);
	    dat->y = (float)(cali.y);
	    dat->z = (float)(cali.z);
	    HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x,cali.y, cali.z);                
	    return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gyroscope_set_cali(int fd, HwmData *dat)
{
	int err;
	SENSOR_DATA cali;

	cali.x = round(dat->x);
	cali.y = round(dat->y);
	cali.z = round(dat->z);
	HWMLOGD("[WD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, cali.x,cali.y, cali.z);
	if(fd < 0)
	{
		HWMLOGE("invalid file handle: %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GYROSCOPE_IOCTL_SET_CALI, &cali)))
	{
		HWMLOGE("set_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gyroscope_rst_cali(int fd)
{
	int err, flags = 0;   
	if(fd < 0)
	{
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, GYROSCOPE_IOCTL_CLR_CALI, &flags)))
	{
		HWMLOGE("rst_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int gyroscope_enable_cali(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_ENABLE_CALI, &flags))) {
		HWMLOGE("enable cali err: %d\n", err);
		return err;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_do_selftest(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_SELF_TEST, &flags))) {
		HWMLOGE("self_test err: %d\n", err);
		return err;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_read_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_GYRO_LID;    
    int rec_size;
    int rec_num;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	int res, fd=FileID.iFileDesc;
    NVRAM_HWMON_GYRO_STRUCT hwmonGyro =
    {    
        {0, 0, 0},
    };    
    
    if(fd < 0)
	{
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }
    
   	res = read(fd, &hwmonGyro , rec_size*rec_num);
    if(res < 0)
	{
        dat->x = dat->y = dat->z = 0.0;
        HWMLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
    }
	else
	{
        dat->x = hwmonGyro.offset[0]/LIBHWM_GYRO_NVRAM_SENSITIVITY;
        dat->y = hwmonGyro.offset[1]/LIBHWM_GYRO_NVRAM_SENSITIVITY;
        dat->z = hwmonGyro.offset[2]/LIBHWM_GYRO_NVRAM_SENSITIVITY;
        HWMLOGD("[RN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonGyro.offset[0], hwmonGyro.offset[1], hwmonGyro.offset[2]);        
    }
        
    NVM_CloseFileDesc(FileID);
#else
    dat->x = 0;
    dat->y = 0;
    dat->z = 0;
#endif
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_write_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
	int file_lid = AP_CFG_RDCL_HWMON_GYRO_LID;    
	int rec_size;
	int rec_num;
	bool bRet = false;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	int res, fd=FileID.iFileDesc;
	NVRAM_HWMON_GYRO_STRUCT hwmonGyro =
	{    
		{0, 0, 0},
	};    

	if(fd < 0)
	{
		HWMLOGE("nvram open = %d\n", fd);
		return fd;
	}

	hwmonGyro.offset[0] = round(dat->x*LIBHWM_GYRO_NVRAM_SENSITIVITY);
	hwmonGyro.offset[1] = round(dat->y*LIBHWM_GYRO_NVRAM_SENSITIVITY);
	hwmonGyro.offset[2] = round(dat->z*LIBHWM_GYRO_NVRAM_SENSITIVITY);
	HWMLOGD("[WN] %9.4f %9.4f %9.4f => %5d %5d %5d\n", dat->x, dat->y, dat->z, hwmonGyro.offset[0], hwmonGyro.offset[1], hwmonGyro.offset[2]);
	res = write(fd, &hwmonGyro , rec_size*rec_num);
	if(res < 0)
	{
		HWMLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
	}
	NVM_CloseFileDesc(FileID); 
	//NVM_AddBackupFileNum(AP_CFG_RDCL_HWMON_GYRO_LID);
	bRet = FileOp_BackupToBinRegion_All();
	sync();
  return !bRet;
#else
    HWMLOGD("gyro [WN] %9.4f %9.4f %9.4f\n", dat->x, dat->y, dat->z);
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/
int checkGyroscopeData(AccItem *list, int num, HwmData *avg, int tolerance) 
{
	float maxdiff = tolerance;
	HwmData min, max;
	char  chkstr[1024];
	float diffx, diffy, diffz, varx = 0, vary = 0, varz = 0;
	int idx;
	int diverse = 0, curdiv;   

	min.x = min.y = min.z = +10*tolerance;
	max.x = max.y = max.z = -10*tolerance;
	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("                         Calibration Data                       \n");
	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("maxdiff = %+9.4f\n", maxdiff);    
	HWMLOGD("average = %+9.4f, %+9.4f %+9.4f\n", avg->x, avg->y, avg->z);    
	HWMLOGD("----------------------------------------------------------------\n");   
	for(idx = 0; idx < num; idx++)
	{       
		if(max.x < list[idx].dat.x)
		{
			max.x = list[idx].dat.x;
		}

		if(max.y < list[idx].dat.y)
		{
			max.y = list[idx].dat.y;
		}
		
		if(max.z < list[idx].dat.z)
		{
			max.z = list[idx].dat.z;
		}
		
		if(min.x > list[idx].dat.x)
		{
			min.x = list[idx].dat.x;
		}

		if(min.y > list[idx].dat.y)
		{
			min.y = list[idx].dat.y;
		}

		if(min.z > list[idx].dat.z)
		{
			min.z = list[idx].dat.z;
		}
		
		diffx = list[idx].dat.x - avg->x;
		diffy = list[idx].dat.y - avg->y;
		diffz = list[idx].dat.z - avg->z;
		varx += diffx * diffx;
		vary += diffy * diffy;
		varz += diffz * diffz; 

		curdiv = 0;
		if(ABS(diffx) > maxdiff) 
		{
			curdiv |= DIVERSE_X;
		}

		if(ABS(diffy) > maxdiff)
		{
			curdiv |= DIVERSE_Y;
		}

		if(ABS(diffz) > maxdiff)
		{
			curdiv |= DIVERSE_Z;
		}

		if((diffx*diffx + diffy*diffy + diffz*diffz) > maxdiff*maxdiff) 
		{
			curdiv |= DIVERSE_XYZ;       
		}
		diverse |= curdiv;

		if (curdiv)
		{
			snprintf(chkstr, sizeof(chkstr), "=> UNSTABLE: 0x%04X, %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f), %+9.4f(%+5.2f)", 
				curdiv, diffx, diffx, diffy, diffy, diffz, diffz, sqrt(diffx*diffx + diffy*diffy + diffz*diffz), sqrt(diffx*diffx + diffy*diffy + diffz*diffz));
		}
		else
		{
			snprintf(chkstr, sizeof(chkstr), " ");
		}
		
		HWMLOGD("[%8lld] (%+9.4f, %+9.4f, %+9.4f) %s\n", list[idx].time/1000000,
		list[idx].dat.x, list[idx].dat.y, list[idx].dat.z, chkstr);
	}
	
	varx = sqrt(varx/num);
	vary = sqrt(vary/num);
	varz = sqrt(varz/num);

	HWMLOGD("----------------------------------------------------------------\n");
	HWMLOGD("X-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.x, avg->x, max.x, (min.x-avg->x), (max.x-avg->x), varx);  
	HWMLOGD("Y-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.y, avg->y, max.y, (min.y-avg->y), (max.y-avg->y), vary);  
	HWMLOGD("Z-Axis: min/avg/max = (%+9.4f, %+9.4f, %+9.4f), diverse = %+9.4f ~ %+9.4f, std = %9.4f\n", 
			min.z, avg->z, max.z, (min.z-avg->z), (max.z-avg->z), varz);  
	HWMLOGD("----------------------------------------------------------------\n");    
	if(diverse)
	{
		return -EINVAL;
	}

	return 0;    
}


/*---------------------------------------------------------------------------*/
int gyroscope_calibration(int fd, int period, int count, int tolerance, HwmData *cali) 
{   
    int err = 0, num = 0;
	AccItem *item = NULL;
	HwmData avg, dat;
	int64_t nt;
    struct timespec time;

	avg.x = avg.y = avg.z = 0.0;    
	if(fd < 0)
	{
		return -EINVAL;
	}
	if(NULL == (item = calloc(count, sizeof(*item))))
	{
		return -ENOMEM;
	}
	
	while(num < count)
	{
		/* read the next event */
		err = gyroscope_read(fd, &dat);

		if(err)
		{
			HWMLOGE("read data fail: %d\n", err);
			goto exit;
		}
		else
		{
			clock_gettime(CLOCK_MONOTONIC, &time);        
			nt = time.tv_sec*1000000000LL+time.tv_nsec;                 
			item[num].dat.x = dat.x;
			item[num].dat.y = dat.y;
			item[num].dat.z = dat.z;
			item[num].time = nt;
			avg.x += item[num].dat.x;
			avg.y += item[num].dat.y;
			avg.z += item[num].dat.z;            
		}
		
		num++;
		libhwm_wait_delay(period);
	}

	/*calculate average*/
	avg.x /= count;
	avg.y /= count;
	avg.z /= count;

	if(0 != (err = checkGyroscopeData(item, count, &avg, tolerance)))
	{        
		HWMLOGE("check accelerometer fail\n");
	}
	else
	{
		cali->x = -avg.x;
		cali->y = -avg.y;
		cali->z = -avg.z;		
	}
	exit:    
	free(item);
	return err;
}
/*---------------------------------------------------------------------------*/
int gyroscope_start_static_calibration(void)
{
    int fd = 0;
    int err, flags = 1;

    fd = open(GYROSCOPE_NAME, O_RDONLY);
    if (fd < 0) {
        HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
        return -errno;
    }
    if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_ENABLE_CALI, &flags))) {
        HWMLOGE("enable cali err: %d\n", err);
        close(fd);
        return err;
    }

    close(fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
int gyroscope_get_static_calibration(struct caliData *caliDat)
{
    int err;
    SENSOR_DATA cali;
    int fd = 0;

    fd = open(GYROSCOPE_NAME, O_RDONLY);

    if (fd < 0) {
        HWMLOGE("invalid file handle: %d\n", fd);
        return -EINVAL;
    }
    if (0 != (err = ioctl(fd, GYROSCOPE_IOCTL_GET_CALI, &cali))) {
        HWMLOGE("get_cali err: %d\n", err);
        close(fd);
        return err;
    }

    caliDat->data[0] = (float)(cali.x) / 1000;
    caliDat->data[1] = (float)(cali.y) / 1000;
    caliDat->data[2] = (float)(cali.z) / 1000;
    HWMLOGD("[RD] %9.4f %9.4f %9.4f => %5d %5d %5d\n", caliDat->data[0], caliDat->data[1], caliDat->data[2], cali.x,cali.y, cali.z);

    close(fd);
    return 0;

}
/*---------------------------------------------------------------------------*/
int msensor_open(int *fd)
{
	if(*fd <= 0)
	{
		*fd = open(MSENSOR_NAME, O_RDONLY);	
	
		if(*fd < 0)
		{
			HWMLOGE("Couldn't find or open file sensor (%s)", strerror(errno));
			return -errno;
		}    
	}
    return 0;
}
/*---------------------------------------------------------------------------*/
int msensor_close(int fd)
{
    if(fd > 0)
	{
		close(fd);        
    }
    
    return 0;
}
/*---------------------------------------------------------------------------*/
int msensor_do_selftest(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, MSENSOR_IOCTL_SELF_TEST, &flags))) {
		HWMLOGE("self_test err: %d\n", err);
		return err;
	}
	return 0;
}
/****************yucong add for alsps factory mode cali**********************/
/* alsps calibration */
int alsps_open(int fd)
{
    fd = open("/dev/als_ps", O_RDONLY);
    if (fd < 0) {
        HWMLOGE("Couldn't open als_ps (%s)", strerror(errno));
        return -1;
    }

    //HWMLOGD("%s() %d\n", __func__, fd);
    return fd;
}

/*---------------------------------------------------------------------------*/
int alsps_close(int fd)
{
    if(fd >= 0)
	{
		close(fd);        
    }else{
    	HWMLOGE("%s() %d\n", __func__, fd);
		return -1;
	}
    //HWMLOGD("%s() %d\n", __func__, fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
int alsps_read(int fd, HwmData *dat)
{
	int err;
	int ps;
	if(fd <= 0)
	{
		HWMLOGE("invalid file handle!\n");
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, ALSPS_GET_PS_RAW_DATA, &ps)))
	{
		HWMLOGE("read err: %d %d (%s)\n", fd, err, strerror(errno));    
		return err;
	}
	else
	{
		dat->ps_data = ps;
		//HWMLOGE("read ps data: %d\n", ps);
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int alsps_set_threshold(int fd, HwmData *dat)
{
	int err;
	int cali;
	int threshold[2];

	cali = dat->ps_cali;
	threshold[0] = dat->ps_threshold_high;
	threshold[1] = dat->ps_threshold_low;
	if((threshold[0] != 0)&&(threshold[1] != 0)){
		if(fd < 0)
		{
			HWMLOGE("invalid file handle: %d\n", fd);
			return -EINVAL;
		}else if(0 != (err = ioctl(fd, ALSPS_SET_PS_THRESHOLD, &threshold)))
		{
			HWMLOGE("set err: %d %d (%s)\n", fd, err, strerror(errno));    
			return err;
		}
		else
		{
			return 0;
		}
	}else{
		HWMLOGD("threshold won't set\n"); 
		return 0;
	}
}

/*---------------------------------------------------------------------------*/
int alsps_get_cali(int fd, HwmData *dat)//need to repare
{
	int err;
	int cali;

	if(fd < 0)
	{
	    HWMLOGE("invalid file handle: %d\n", fd);
	    return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, ALSPS_IOCTL_GET_CALI, &cali)))
	{
	    HWMLOGE("get_cali err: %d\n", err);
	    return err;
	}
	else
	{
	    dat->ps_cali = cali;           
	    return 0;
	}
}
/*---------------------------------------------------------------------------*/
int alsps_set_cali(int fd, HwmData *dat)
{
	int err;
	int cali;
	int threshold[2];

	cali = dat->ps_cali;
	threshold[0] = dat->ps_threshold_high;
	threshold[1] = dat->ps_threshold_low;
	
	if(fd < 0)
	{
		HWMLOGE("invalid file handle: %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, ALSPS_IOCTL_SET_CALI, &cali)))
	{
		HWMLOGE("set_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}

int als_set_cali(int fd, struct caliData *caliDat)
{
	int err;
	float cali;

	cali = caliDat->data[0] * 1000;

	if(fd < 0)
	{
		HWMLOGE("invalid file handle: %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, ALSPS_ALS_SET_CALI, &cali)))
	{
		HWMLOGE("set_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}

/*---------------------------------------------------------------------------*/
int alsps_rst_cali(int fd)
{
	int err, flags = 0;   
	if(fd < 0)
	{
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	else if(0 != (err = ioctl(fd, ALSPS_IOCTL_CLR_CALI, &flags)))
	{
		HWMLOGE("rst_cali err: %d\n", err);
		return err;
	}
	else
	{
		return 0;
	}
}
/*---------------------------------------------------------------------------*/
int alsps_ps_enable_cali(int fd)
{
	int err, flags = 1;   
	if (fd < 0) {
		HWMLOGE("invalid file handle %d\n", fd);
		return -EINVAL;
	}
	if (0 != (err = ioctl(fd, ALSPS_PS_ENABLE_CALI, &flags))) {
		HWMLOGE("enable cali err: %d\n", err);
		return err;
	}
	return 0;
}
/*---------------------------------------------------------------------------*/
int alsps_read_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
    int file_lid = AP_CFG_RDCL_HWMON_PS_LID;    
    int rec_size;
    int rec_num;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISREAD);
	int res, fd=FileID.iFileDesc;
    NVRAM_HWMON_PS_STRUCT hwmonPS =
    {    
        {0,0,0}
    };    
    
    if(fd < 0)
	{
        HWMLOGE("nvram open = %d\n", fd);
        return fd;
    }
    
   	res = read(fd, &hwmonPS , rec_size*rec_num);
    if(res < 0)
	{
        dat->ps_data = dat->ps_cali = dat->ps_threshold_high = dat->ps_threshold_low = 0;
        HWMLOGE("nvram read = %d(%s)\n", errno, strerror(errno));
    }
	else
	{
		HWMLOGD("nvram read: %d, %d, %d \n", hwmonPS.ps_cali[0],hwmonPS.ps_cali[1],hwmonPS.ps_cali[2]);
        dat->ps_cali = hwmonPS.ps_cali[0];
		dat->ps_threshold_high = hwmonPS.ps_cali[1];
		dat->ps_threshold_low = hwmonPS.ps_cali[2];
	}
        
    NVM_CloseFileDesc(FileID);
#else
    dat->ps_cali = 0;
    dat->ps_threshold_high = 0;
    dat->ps_threshold_low = 0;
#endif
    return 0;
}
/*---------------------------------------------------------------------------*/
int alsps_write_nvram(HwmData *dat)
{
#ifdef SUPPORT_SENSOR_ACCESS_NVRAM
	int file_lid = AP_CFG_RDCL_HWMON_PS_LID;    
	int rec_size;
	int rec_num;
	bool bRet = false;
	F_ID FileID = NVM_GetFileDesc(file_lid, &rec_size, &rec_num, ISWRITE);
	int res, fd=FileID.iFileDesc;
	NVRAM_HWMON_PS_STRUCT hwmonPS =
	{    
		{0,0,0}
	};    

	if(fd < 0)
	{
		HWMLOGE("nvram open = %d\n", fd);
		return fd;
	}

    hwmonPS.ps_cali[0] = dat->ps_cali;
	hwmonPS.ps_cali[1] = dat->ps_threshold_high;
	hwmonPS.ps_cali[2] = dat->ps_threshold_low;
	HWMLOGD("nvram write: %d, %d, %d \n", dat->ps_cali ,hwmonPS.ps_cali[1],hwmonPS.ps_cali[2]);
	res = write(fd, &hwmonPS , rec_size*rec_num);
	if(res < 0)
	{
		HWMLOGE("nvram write = %d(%s)\n", errno, strerror(errno));
		return res;
	}
	NVM_CloseFileDesc(FileID); 
	bRet = FileOp_BackupToBinRegion_All();
	sync();
  return !bRet;
#else
    HWMLOGD("alsps nvram write: %d, %d, %d \n",
        dat->ps_cali, dat->ps_threshold_high, dat->ps_threshold_low);
#endif
    return 0;
}

/*----------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
int alsps_calibration(int fd, int period, int count, HwmData *cali) 
{   
    int err = 0, num = 0;
	int *item = NULL;
	HwmData max, dat;  
    
	if(fd < 0)
	{
		return -EINVAL;
	}
	if(NULL == (item = calloc(count, sizeof(*item))))
	{
		return -ENOMEM;
	}
	
	while(num < count)
	{
		/* read the next event */
		err = alsps_read(fd, &dat);

		if(err)
		{
			HWMLOGE("read data fail: %d\n", err);
			goto exit;
		}
		else
		{   
			/*caucalt max ps data as cali data*/
			item[num] = dat.ps_data;
			if(max.ps_data <= item[num]){
				max.ps_data = item[num];
				}
		}
		
		num++;
		libhwm_wait_delay(period);
	}
	
	cali->ps_cali = max.ps_cali = max.ps_data;
		
	
	exit:    
	free(item);
	return err;
}

//
/*---------------------------------------------------------------------------*/
enum {
    ITEM_HIGH,
 	ITEM_LOW,	
};

int calculate_psensor_unit_value(void)
{
	HwmData dat;
	int i,err = 0;
	int ps_cali;
	int ps_min_smallest;
	int ps_min_largest;
	int ps_fd = -1;
	int unit = 0;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return -1;
	}
	
	//HWMLOGD("read als_ps fd: (%d)", ps_fd);
	
	err = alsps_get_cali(ps_fd, &dat);	
	if(err < 0)
	{
		HWMLOGE("get cali in driver fail in calculate_psensor_min_value");
		alsps_close(ps_fd);
		return -1;
	}
	ps_cali = dat.ps_cali;
	HWMLOGD("calculate_psensor_min_value cali in driver data: %d",ps_cali);

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		HWMLOGE("reset cali in driver fail in calculate_psensor_min_value");
		alsps_close(ps_fd);
		return -1;
	}
	
	if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return -1;
		}
	ps_min_largest = dat.ps_data;
	ps_min_smallest = dat.ps_data;
	HWMLOGD("calculatr before ps_min_smallest:%d, ps_min_largest:%d", ps_min_smallest,ps_min_largest);
	for(i = 0;i < 10; i++)
	{
		if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return -1;
		}
		if(ps_min_smallest > dat.ps_data)
			ps_min_smallest = dat.ps_data;
		else if((ps_min_largest < dat.ps_data))
			ps_min_largest = dat.ps_data;
		HWMLOGD("calculatr after ps_min_smallest:%d, ps_min_largest:%d", ps_min_smallest,ps_min_largest);
		libhwm_wait_delay(200);
	}
	unit = ps_min_largest - ps_min_smallest;
	HWMLOGD("yucong als_ps data min value (%d)", unit);

	err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("set cali in driver fail in calculate_psensor_min_value");
		alsps_close(ps_fd);
		return -1;
	}		

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return -1;
	}
	
	return unit;
}

int get_psensor_data(void)
{
	HwmData dat;
	int ps_data;
	int err = 0;
	int ps_fd = -1;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return -1;
	}

	err = alsps_read(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("read als_ps fail (%s)", strerror(errno));
		alsps_close(ps_fd);
		return -1;
	}
	ps_data = dat.ps_data;
	HWMLOGD("read ps data: (%d)", ps_data);

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return -1;
	}
	
	return ps_data;
} 
/*------------------------------------------------------------*/
int calculate_psensor_min_value(void)
{
	HwmData dat;
    int err = 0;
	int ps_cali;
	int ps_fd = -1;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}
	
	//HWMLOGD("read als_ps fd: (%d)", ps_fd);
	
	err = alsps_get_cali(ps_fd, &dat);	
	if(err < 0)
	{
		HWMLOGE("get cali in driver fail in calculate_psensor_min_value");
		alsps_close(ps_fd);
		return 0;
	}
	ps_cali = dat.ps_cali;
	HWMLOGD("calculate_psensor_min_value cali in driver data: %d",ps_cali);

	/*this function is useless*/

	err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("set cali in driver fail in calculate_psensor_min_value");
		alsps_close(ps_fd);
		return 0;
	}		

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}
	
	return 1;
}
int get_psensor_min_value(void)
{
	HwmData dat;
	int i,err = 0;
	int ps_min;
	int ps_fd = -1;
	int ps_cali;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}

	//HWMLOGD("read als_ps min data fd: (%d)", ps_fd);

	err = alsps_get_cali(ps_fd, &dat);	
	if(err < 0)
	{
		HWMLOGE("get cali in driver fail in get_psensor_min_value");
		alsps_close(ps_fd);
		return 0;
	}
	ps_cali = dat.ps_cali;
	HWMLOGD("get_psensor_min_value cali in driver data: %d",ps_cali);

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		HWMLOGE("reset cali in driver fail in get_psensor_min_value");
		alsps_close(ps_fd);
		return 0;
	}

	if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return 0;
		}
	ps_min = dat.ps_data;
	HWMLOGD("read als_ps min data first (%d)", ps_min);
	for(i = 0;i < 20; i++)
	{
		if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return 0;
		}
		
		if(ps_min < dat.ps_data)
			ps_min = dat.ps_data;
	}
	HWMLOGD("read als_ps min data (%d)", ps_min);

/******************************************************/
		err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("set cali in driver fail in get_psensor_min_value");
		alsps_close(ps_fd);
		return 0;
	}
/******************************************************/

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}
	
	return ps_min;

}
/*------------------------------------------------------------*/

int calculate_psensor_max_value(void)
{
	HwmData dat;
    int err = 0;
	int ps_fd = -1;
	int ps_cali;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}

	err = alsps_get_cali(ps_fd, &dat);	
	if(err < 0)
	{
		HWMLOGE("get cali in driver fail in calculate_psensor_max_value");
		alsps_close(ps_fd);
		return 0;
	}
	ps_cali = dat.ps_cali;
	HWMLOGD("calculate_psensor_max_value cali in driver data: %d",ps_cali);

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		HWMLOGE("reset cali in driver fail in calculate_psensor_max_value");
		alsps_close(ps_fd);
		return 0;
	}

	/*this function is useless*/
	
	err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("set cali in driver fail in calculate_psensor_min_value");

	}

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}

	return 1;
}

int get_psensor_max_value(void)
{
	HwmData dat;
	int i,err = 0;
	int ps_max;
	int ps_fd = -1;
	int ps_cali;
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}
	err = alsps_get_cali(ps_fd, &dat);	
	if(err < 0)
	{
		HWMLOGE("get cali in driver fail in get_psensor_max_value");
		alsps_close(ps_fd);
		return 0;
	}
	ps_cali = dat.ps_cali;
	HWMLOGD("get_psensor_max_value cali in driver data: %d",ps_cali);

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		HWMLOGE("reset cali in driver fail in get_psensor_max_value");
		alsps_close(ps_fd);
		return 0;
	}
	
	if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return -1;
		}
	ps_max = dat.ps_data;
	
	for(i = 0;i < 20; i++)
	{
		if((err = alsps_read(ps_fd, &dat)) < 0)
		{
			HWMLOGE("read als_ps fail (%s)", strerror(errno));
			alsps_close(ps_fd);
			return -1;
		}
		if(ps_max > dat.ps_data)
			ps_max = dat.ps_data;
	}

	err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("set cali in driver fail in get_psensor_max_value");
		alsps_close(ps_fd);
		return 0;
	}
	
	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}

	return ps_max;
}

int do_calibration(int min, int max)
{
	int threshold_high, threshold_low ,err;
	HwmData dat;
	int ps_fd = -1;
	int temp_max,temp_min;
	int unit = 0;
	temp_max = max;
	temp_min = min;
	
	if(max <= min)
	{
		HWMLOGE("max value is not suitable for calibration!");
		return 0;
	}
	unit = calculate_psensor_unit_value();
	if(unit < 0)
	{
		HWMLOGE("calculate unit fail(%s)", strerror(errno));
		return 0;
	}
	HWMLOGE("max value is %d min value is %d unit %d!",temp_max, temp_min, unit);
	
	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}

    dat.ps_cali = 0;
	dat.ps_threshold_high = 0;
	dat.ps_threshold_low = 0;
	err = alsps_write_nvram(&dat);
	if(err < 0)
	{
		alsps_close(ps_fd);
		HWMLOGE("clear NVRAM fail in do calibraction");
		return 0;
	}

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		alsps_close(ps_fd);
		HWMLOGE("reset cali in driver fail in do calibration");
		return 0;
	}

	threshold_high = (max - min)/3;
	threshold_low = threshold_high - 3*unit;
	if(threshold_low < 0)
	{
		threshold_low = 0;
		HWMLOGE("cali threshold_low warning");
	}

    dat.ps_cali = min;
	dat.ps_threshold_high = threshold_high;
	dat.ps_threshold_low = threshold_low;
	HWMLOGD("min value is %d high: %d, low: %d!\n", dat.ps_cali, dat.ps_threshold_high, dat.ps_threshold_low);
	
	err = alsps_write_nvram(&dat);
	if(err < 0)
	{
		HWMLOGE("write NVRAM fail in do calibraction");
		alsps_close(ps_fd);
		return 0;
	}

	err = alsps_set_cali(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("alsps_set_cali fail in do calibraction");
		alsps_close(ps_fd);
		return 0;
	}

	err = alsps_set_threshold(ps_fd, &dat);
	if(err < 0)
	{
		HWMLOGE("alsps_set_threshold fail in do calibraction");
		alsps_close(ps_fd);
		return 0;
	}
	
	err = alsps_close(ps_fd);
	if(err < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}	
	return 1;
} 

int clear_psensor_calibration(void)
{
	HwmData dat;
	int ps_fd = -1;
	int err;

	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return 0;
	}

    dat.ps_cali = 0;
	dat.ps_threshold_high = 0;
	dat.ps_threshold_low = 0;
	
	err = alsps_write_nvram(&dat);
	if(err < 0)
	{
		HWMLOGE("clear NVRAM fail in clear_psensor_calibration");
		alsps_close(ps_fd);
		return 0;
	}

	err = alsps_rst_cali(ps_fd);	
	if(err < 0)
	{
		HWMLOGE("reset cali in driver fail in clear_psensor_calibration");
		alsps_close(ps_fd);
		return 0;
	}
	alsps_close(ps_fd);
	return 1;
}

int get_psensor_threshold(int flag)
{
	int err;
	int ps = 0;
	int ps_fd = -1;

	ps_fd = alsps_open(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("open als_ps fail (%s)", strerror(errno));
		return -1;
	}

	switch (flag) {
	case ITEM_HIGH:
		if(ps_fd <= 0)
		{
			HWMLOGE("invalid file handle!\n");
			break;
		}
		else if(0 != (err = ioctl(ps_fd, ALSPS_GET_PS_THRESHOLD_HIGH, &ps)))
		{
			HWMLOGE("read err: %d %d (%s)\n", ps_fd, err, strerror(errno));    
			break;
		}
		HWMLOGD("get threshold_high: %d\n", ps);  
		break;

	case ITEM_LOW:
		if(ps_fd <= 0)
		{
			HWMLOGE("invalid file handle!\n");
			break;
		}
		else if(0 != (err = ioctl(ps_fd, ALSPS_GET_PS_THRESHOLD_LOW, &ps)))
		{
			HWMLOGE("read err: %d %d (%s)\n", ps_fd, err, strerror(errno));    
			break;
		}
		HWMLOGD("get threshold_high: %d\n", ps);  
		break;
	}	

	err = alsps_close(ps_fd);
	if(ps_fd < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}
	
	return ps;
}
int set_psensor_threshold(int high,int low)
{
	int threshold[2];
	HwmData dat;
	int err = 0;
	int ps_fd = -1;
	
	ps_fd = alsps_open(ps_fd);

	threshold[0] = high;
	threshold[1] = low;
	
	err = alsps_read_nvram(&dat);
	if(err < 0)
	{
		HWMLOGE("alsps_read_nvram fail in set_psensor_threshold");
		alsps_close(ps_fd);
		return 0;
	}

	dat.ps_threshold_high = threshold[0];
	dat.ps_threshold_low = threshold[1];

	err = alsps_write_nvram(&dat);
	if(err < 0)
	{
		HWMLOGE("alsps_write_nvram fail in set_psensor_threshold");
		alsps_close(ps_fd);
		return 0;
	}

	if(ps_fd <= 0)
	{
		HWMLOGE("invalid file handle!\n");
		alsps_close(ps_fd);
		return 0;
	}
	else if(0 != (err = ioctl(ps_fd, ALSPS_SET_PS_THRESHOLD, &threshold)))
	{
		HWMLOGE("set err: %d %d (%s)\n", ps_fd, err, strerror(errno));    
		alsps_close(ps_fd);
		return err;
	}

	err = alsps_close(ps_fd);
	if(err < 0)
	{
		HWMLOGE("close als_ps fail (%s)", strerror(errno));
		return 0;
	}
	
	return 1;
}

/*---------------------------------------------------------------------------*/
int do_gsensor_calibration(int tolerance)
{
	int gs_fd = -1;
	int err = 0;
	HwmData cali;
	
	err = gsensor_open(&gs_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_open fail in do_gsensor_calibration");
		return 0;
	}
	//******************************
	if((err = gsensor_calibration(gs_fd, 50, 20, tolerance*10, &cali)) != 0)
	{
		HWMLOGE("calibrate acc: %d\n", err);
		gsensor_close(gs_fd);
		return 0;
	}
	else if((err = gsensor_set_cali(gs_fd, &cali)) != 0)
	{	 
		HWMLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
		gsensor_close(gs_fd);
		return 0;
	}
	else if((err = gsensor_get_cali(gs_fd, &cali)) != 0)
	{	 
		HWMLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
		gsensor_close(gs_fd);
		return 0;
	}
	else if((err = gsensor_write_nvram(&cali)) != 0)
	{
		HWMLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
		gsensor_close(gs_fd);
		return 0;
	}

	//******************************

	err = gsensor_close(gs_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_close fail in do_gsensor_calibration");
		return 0;
	}

	return 1;
} 

int get_gsensor_calibration(float *x, float *y, float *z)
{
	HwmData cali;
	int err = 0;
	err = gsensor_read_nvram(&cali);
	if(err < 0)
	{
		HWMLOGE("gsensor_read_nvram fail in get_gsensor_calibration");
		return 0;
	}

	*x = cali.x;
	*y = cali.y;
	*z = cali.z;
	
	return 1;
}

int clear_gsensor_calibration(void)
{
	int err;
	int gs_fd = -1;
	HwmData cali_nvram;

	cali_nvram.x = 0;
	cali_nvram.y = 0;
	cali_nvram.z = 0;
	
	err = gsensor_open(&gs_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_open fail in clear_gsensor_calibration");
		return 0;
	}
	//******************************	
	err = gsensor_rst_cali(gs_fd);
	if(err)
	{
		HWMLOGE("rst calibration: %d\n", err);
		gsensor_close(gs_fd);
		return 0;
	}
	else if((err = gsensor_write_nvram(&cali_nvram)) != 0)
	{
		HWMLOGE("write nvram: %d\n", err);
		gsensor_close(gs_fd);
		return 0;
	}	
	//******************************

	err = gsensor_close(gs_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_close fail in clear_gsensor_calibration");
		return 0;
	}	
	return 1;
}
/*---------------------------------------------------------------------------*/
int do_gyroscope_calibration(int tolerance)
{
	int gyro_fd = -1;
	int err = 0;
	HwmData cali;
	
	err = gyroscope_open(&gyro_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_open fail in do_gyroscope_calibration");
		return 0;
	}
	//******************************
	if((err = gyroscope_calibration(gyro_fd, 50, 20, tolerance*100, &cali)) != 0)
	{
		HWMLOGE("calibrate acc: %d\n", err);
		gyroscope_close(gyro_fd);
		return 0;
	}
	else if((err = gyroscope_set_cali(gyro_fd, &cali)) != 0)
	{	 
		HWMLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
		gyroscope_close(gyro_fd);
		return 0;
	}
	else if((err = gyroscope_get_cali(gyro_fd, &cali)) != 0)
	{	 
		HWMLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
		gyroscope_close(gyro_fd);
		return 0;
	}
	else if((err = gyroscope_write_nvram(&cali)) != 0)
	{
		HWMLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
		gyroscope_close(gyro_fd);
		return 0;
	}

	//******************************

	err = gyroscope_close(gyro_fd);
	if(err < 0)
	{
		HWMLOGE("gsensor_close fail in do_gyroscope_calibration");
		return 0;
	}

	return 1;

} 
int get_gyroscope_calibration(float *x, float *y, float *z)
{
	HwmData cali;
	int err = 0;
	err = gyroscope_read_nvram(&cali);
	if(err < 0)
	{
		HWMLOGE("gyroscope_read_nvram fail in get_gyroscope_calibration");
		return 0;
	}

	*x = cali.x;
	*y = cali.y;
	*z = cali.z;
	
	return 1;
}

int clear_gyroscope_calibration(void)
{
	int err;
	int gyro_fd = -1;
	HwmData cali_nvram;

	cali_nvram.x = 0;
	cali_nvram.y = 0;
	cali_nvram.z = 0;
	
	err = gyroscope_open(&gyro_fd);
	if(err < 0)
	{
		HWMLOGE("gyroscope_open fail in clear_gyroscope_calibration");
		return 0;
	}
	//******************************	
	err = gyroscope_rst_cali(gyro_fd);
	if(err)
	{
		HWMLOGE("rst calibration: %d\n", err);
		gyroscope_close(gyro_fd);
		return 0;
	}
	else if((err = gyroscope_write_nvram(&cali_nvram)) != 0)
	{
		HWMLOGE("write nvram: %d\n", err);
		gyroscope_close(gyro_fd);
		return 0;
	}	
	//******************************

	err = gyroscope_close(gyro_fd);
	if(err < 0)
	{
		HWMLOGE("gyroscope_close fail in clear_gyroscope_calibration");
		return 0;
	}	
	return 1;

}
