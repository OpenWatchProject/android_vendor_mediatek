/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <dirent.h>
#include <linux/input.h>
#include <math.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
#include <linux/hwmsensor.h>
#include <linux/sensors_io.h>
#include "libbiosensor.h"
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[BSC] "
#define mod_to_bsc_data(p) (struct bsc_data*)((char*)(p) + sizeof(struct ftm_module))
#define BSCLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define BSCLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_CLEAR,
    ITEM_DO,
    ITEM_EXIT,
};
/*---------------------------------------------------------------------------*/
static item_t bio_cali_items[] = {
    item(ITEM_CLEAR,  uistr_info_sensor_cali_clear),
    item(ITEM_DO,   uistr_info_sensor_cali_do),
    item(ITEM_EXIT,   uistr_info_sensor_back),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
enum{
    BIO_OP_NONE,
    BIO_OP_CLEAR,
    BIO_OP_CALI_PRE,
    BIO_OP_CALI,
}; 
/*---------------------------------------------------------------------------*/
#define C_MAX_MEASURE_NUM (20)
/*---------------------------------------------------------------------------*/
struct bsc_priv
{
    pthread_mutex_t evtmutex;
    /*specific data field*/
    int fd;

    int  pending_op;
    bool bUpToDate; 
    struct BioData cali_drv;
    struct BioData cali_nvram;

    char status[1024];
};
/*---------------------------------------------------------------------------*/
struct bsc_data
{
    struct bsc_priv bsc;

    /*common for each factory mode*/
    char  info[1024];
    bool  avail;
    bool  exit_thd;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int bio_cali_init_priv(struct bsc_priv *bsc)
{
    memset(bsc, 0x00, sizeof(*bsc));
    return 0;
}
/*---------------------------------------------------------------------------*/
static int bio_cali_update_info(struct bsc_priv *bsc)
{
	int err = 0;
	if(!bsc->bUpToDate)
	{
		err = biosensor_read_nvram(&bsc->cali_nvram);
		if(err)
		{
			BSCLOGE("read nvram: %d(%s)\n", errno, strerror(errno));
		}
		else if((err = biosensor_get_cali(bsc->fd, &bsc->cali_drv)) != 0)
		{
			BSCLOGE("get calibration: %d(%s)\n", errno, strerror(errno));
		}
		else
		{
			bsc->bUpToDate = true;
		}        
	}

	return err;
}

/*---------------------------------------------------------------------------*/
static int bio_cali_close(struct bsc_priv *bsc)
{
	int err;
	if(!bsc || (bsc->fd < 0))
	{
		return -EINVAL;
	}
	
	err = biosensor_close(bsc->fd);
	memset(bsc, 0x00, sizeof(*bsc));
	return err;
}
/*---------------------------------------------------------------------------*/
static void *bio_cali_update_iv_thread(void *priv)
{
	struct bsc_data *dat = (struct bsc_data *)priv; 
	struct bsc_priv *bsc = &dat->bsc;
	struct itemview *iv = dat->iv;
	int err = 0, len = 0;
	char *status;
	struct BioData cali;
	static int op = -1;
	int  max_retry = 3, retry_period = 100, retry=0;
    unsigned int flags = 1;

	LOGD(TAG "%s: Start\n", __FUNCTION__);
	err = biosensor_open(&(bsc->fd));
	if(err)
	{
		memset(dat->info, 0x00, sizeof(dat->info));
		sprintf(dat->info, uistr_info_sensor_init_fail);
		iv->redraw(iv);
		BSCLOGE("bio_cali_open() err = %d(%s)\n", err, dat->info);
		pthread_exit(NULL);
		return NULL;
	}
    /** Enable bio-sensor **/
    while ((err = ioctl(bsc->fd, BIOMETRIC_IOCTL_INIT, &flags)) && (retry ++ < max_retry))
    usleep(retry_period*1000);
    if (err) {
        LOGD("enable bio-sensor fail: %s", strerror(errno));
        return NULL;
	}

	while(1)
	{

		if(dat->exit_thd)
		{
			break;
		}

		pthread_mutex_lock(&dat->bsc.evtmutex);
		if(op != dat->bsc.pending_op)
		{
			op = dat->bsc.pending_op;
			BSCLOGD("op: %d\n", dat->bsc.pending_op);
		}
		pthread_mutex_unlock(&dat->bsc.evtmutex);
		err = 0;

		if(op == BIO_OP_CLEAR)
		{
			memset(&dat->bsc.cali_nvram, 0x00, sizeof(dat->bsc.cali_nvram));
			memset(&dat->bsc.cali_drv, 0x00, sizeof(dat->bsc.cali_drv));
			err = biosensor_reset_nvram();
			if(err)
				BSCLOGE("biosensor_reset_nvram fail: %d\n", err);
			else if((err = biosensor_read_nvram(&cali)) != 0)
				BSCLOGE("biosensor_read_nvram fail: %d\n", err);
			else if ((err = biosensor_set_cali(bsc->fd, &cali)) != 0)
				BSCLOGE("biosensor_set_cali fail: %d\n", err);

			if(err)
				len = snprintf(dat->bsc.status, sizeof(dat->bsc.status), "%s\n", uistr_info_sensor_cali_fail);
			else
				len = snprintf(dat->bsc.status, sizeof(dat->bsc.status), "%s\n", uistr_info_sensor_cali_ok);

			bsc->bUpToDate = false;    
			pthread_mutex_lock(&dat->bsc.evtmutex);
			dat->bsc.pending_op = BIO_OP_NONE;
			pthread_mutex_unlock(&dat->bsc.evtmutex);
		}
		else if(op == BIO_OP_CALI_PRE)
		{
			err = 0;
			/*by-pass*/
			len = snprintf(dat->bsc.status, sizeof(dat->bsc.status), "%s\n", uistr_info_sensor_cali_ongoing);
			pthread_mutex_lock(&dat->bsc.evtmutex);
			dat->bsc.pending_op = BIO_OP_CALI;
			pthread_mutex_unlock(&dat->bsc.evtmutex);
		}
		else if(op == BIO_OP_CALI)
		{
			if((err = biosensor_calibration(bsc->fd, &cali)) != 0)
			{
				BSCLOGE("calibrate acc: %d\n", err);                
			}
			else if((err = biosensor_set_cali(bsc->fd, &cali)) != 0)
			{    
				BSCLOGE("set calibration fail: (%s) %d\n", strerror(errno), err);
			}
			else if((err = biosensor_get_cali(bsc->fd, &cali)) != 0)
			{    
				BSCLOGE("get calibration fail: (%s) %d\n", strerror(errno), err);
			}
			else if((err = biosensor_write_nvram(&cali)) != 0)
			{
				BSCLOGE("write nvram fail: (%s) %d\n", strerror(errno), err);
			}
			else
			{
				dat->bsc.bUpToDate = false;
			}
			
			if(err)
			{
				len = snprintf(dat->bsc.status, sizeof(dat->bsc.status), "%s\n", uistr_info_sensor_cali_fail);
				dat->mod->test_result = FTM_TEST_FAIL;
			}
			else
			{
				len = snprintf(dat->bsc.status, sizeof(dat->bsc.status), "%s\n", uistr_info_sensor_cali_ok);
				dat->mod->test_result = FTM_TEST_PASS;
			}

			pthread_mutex_lock(&dat->bsc.evtmutex);
			dat->bsc.pending_op = BIO_OP_NONE;
			pthread_mutex_unlock(&dat->bsc.evtmutex);
		}

		err = bio_cali_update_info(bsc);
		if(err)
		{
			BSCLOGE("bio_cali_update_info() = (%s), %d\n", strerror(errno), err);
			break;
		} 


		len = 0;
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "D: %+7.4f %+7.4f\n", bsc->cali_drv.pga6, bsc->cali_drv.ambdac5_5);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "N: %+7.4f %+7.4f\n", bsc->cali_nvram.pga6, bsc->cali_nvram.ambdac5_5);
		len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s\n", bsc->status);
		        
		iv->set_text(iv, &dat->text);
		iv->redraw(iv);
	}
	
	bio_cali_close(bsc);
	LOGD(TAG "%s: Exit\n", __FUNCTION__);    
	pthread_exit(NULL);

	return NULL;
}
/*---------------------------------------------------------------------------*/
int bio_cali_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct bsc_data *dat = (struct bsc_data *)priv;
    struct itemview *iv;
    struct statfs stat;
    int err, op, len;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);
       
    len = snprintf(dat->info, sizeof(dat->info), "%s\n", uistr_info_sensor_initializing);
    dat->exit_thd = false;  


    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        dat->iv = iv;
    }
    iv = dat->iv;
    iv->set_title(iv, &dat->title);
    iv->set_items(iv, bio_cali_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, bio_cali_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        pthread_mutex_lock(&dat->bsc.evtmutex);
        op = dat->bsc.pending_op;
        pthread_mutex_unlock(&dat->bsc.evtmutex);        
        if ((chosen != ITEM_EXIT) && (op != BIO_OP_NONE))   /*some OP is pending*/
            continue;
        switch (chosen) {
        case ITEM_CLEAR:
            pthread_mutex_lock(&dat->bsc.evtmutex);
            dat->bsc.pending_op = BIO_OP_CLEAR;
            BSCLOGD("chosen clear: %d\n", dat->bsc.pending_op);
            pthread_mutex_unlock(&dat->bsc.evtmutex);
            break;
        case ITEM_DO:
            pthread_mutex_lock(&dat->bsc.evtmutex);            
            dat->bsc.pending_op = BIO_OP_CALI_PRE;
            BSCLOGD("chosen DO\n");
            pthread_mutex_unlock(&dat->bsc.evtmutex);
            break;
        case ITEM_EXIT:
            exit = true;            
            break;
        }
        
        if (exit) {
            dat->exit_thd = true;
            break;
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int bio_cali_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct bsc_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_BIOSENSOR_CALI, sizeof(struct bsc_data));
    dat  = mod_to_bsc_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    dat->bsc.fd = -1;
    bio_cali_init_priv(&dat->bsc);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, bio_cali_entry, (void*)dat);
//    if (!ret)
//        mod->visible = false;

    return ret;
}
#endif 

