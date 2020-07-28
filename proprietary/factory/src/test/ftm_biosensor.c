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
#include "cust.h"

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
#include <linux/sensors_io.h>
#include "libbiosensor.h"

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[BIO] "
#define mod_to_bio_data(p) (struct bio_data*)((char*)(p) + sizeof(struct ftm_module))
#define FTGLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FTGLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
#define BIOSENSOR_NAME "/dev/biometric"
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t biosensor_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
#endif
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
typedef enum {
    RESULT_PPG_IR,
    RESULT_PPG_R,
    RESULT_EKG,

    RESULT_MAX,
}TEST_RESULT;
/*---------------------------------------------------------------------------*/
char *biosensor_result[] = {
    "PPG(IR)",
    "PPG(R)",
    "EKG",
};
/*---------------------------------------------------------------------------*/
struct bio_priv
{
    int fd;
    struct BioTestData data;
    struct BioThreshold threshold;
    unsigned int result_chk;
    struct BioTestData min_data;
    struct BioTestData max_data;
    struct BioTestData numOfPassData;
};
/*---------------------------------------------------------------------------*/
struct bio_data
{
    struct bio_priv bio;

    /*common for each factory mode*/
    char  info[1024];
    //bool  avail;
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

bool biosensor_thread_exit = false;
static int sp_ata_status = 0;
pthread_mutex_t biosensor_mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************
 * common interface
 *****************************************************************************/
/*---------------------------------------------------------------------------*/
static int biosensor_check_result(struct bio_priv *bio)
{
    if (bio->data.ppg_ir < bio->min_data.ppg_ir)
        bio->min_data.ppg_ir = bio->data.ppg_ir;
    if (bio->data.ppg_ir > bio->max_data.ppg_ir)
        bio->max_data.ppg_ir = bio->data.ppg_ir;
    if (bio->data.ppg_r < bio->min_data.ppg_r)
        bio->min_data.ppg_r = bio->data.ppg_r;
    if (bio->data.ppg_r > bio->max_data.ppg_r)
        bio->max_data.ppg_r = bio->data.ppg_r;

    if (bio->min_data.ppg_ir < bio->threshold.ppg_ir_threshold &&
        bio->threshold.ppg_ir_threshold < bio->max_data.ppg_ir)
        bio->result_chk |= 1 << RESULT_PPG_IR;

    if (bio->min_data.ppg_r < bio->threshold.ppg_r_threshold &&
        bio->threshold.ppg_r_threshold < bio->max_data.ppg_r)
        bio->result_chk |= 1 << RESULT_PPG_R;

    if (bio->data.ekg < bio->threshold.ekg_threshold) {
        bio->numOfPassData.ekg++;
        if (bio->numOfPassData.ekg >= 3)
            bio->result_chk |= 1 << RESULT_EKG;
    } else
        bio->numOfPassData.ekg = 0;
        
    return 0;
}
/******************************************************************************
 * Functions 
 *****************************************************************************/
static int biosensor_init_priv(struct bio_priv *bio)
{
    memset(bio, 0x00, sizeof(*bio));
    bio->fd = -1;
    bio->max_data.ppg_ir = -FLT_MAX;
    bio->min_data.ppg_ir = FLT_MAX;
    bio->max_data.ppg_r = -FLT_MAX;
    bio->min_data.ppg_r = FLT_MAX;
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *biosensor_update_iv_thread(void *priv)
{
    struct bio_data *dat = (struct bio_data *)priv; 
    struct bio_priv *bio = &dat->bio;
    struct itemview *iv = dat->iv;
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    biosensor_init_priv(bio);
    if (err = biosensor_open(&(bio->fd))) {
    	   memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
        iv->redraw(iv);
        FTGLOGE("biosensor_open() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }

    if ((err = biosensor_ftm_start(bio->fd))) {
        FTGLOGE("biosensor_ftm_start() = (%s), %d\n", strerror(errno), err);
        pthread_exit(NULL);
        return NULL;
    }

    while (1) {
        if (dat->exit_thd){
            FTGLOGE("dat -> exit_thd\n");
            break;
        }
        if ((err = biosensor_read_test_data(bio->fd, &(bio->data)))) {
            FTGLOGE("biosensor_update_info() = (%s), %d\n", strerror(errno), err);
            break;
        } else if (err = biosensor_read_test_threshold(bio->fd, &(bio->threshold))) {
            FTGLOGE("biosensor_check_result() = (%s), %d\n", strerror(errno), err);
            break;
        } else if (err = biosensor_check_result(bio)) {
            FTGLOGE("biosensor_check_result() = (%s), %d\n", strerror(errno), err);
            break;
        }

        len = 0;

        len = snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f\n",
                 biosensor_result[RESULT_PPG_IR], biosensor_result[RESULT_PPG_IR], bio->data.ppg_ir);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f, %s:%+7.3f, %s:%+7.3f\n",
                 uistr_info_bio_sensor_min, bio->min_data.ppg_ir,
                 uistr_info_bio_sensor_max, bio->max_data.ppg_ir,
                 uistr_info_bio_sensor_thres, bio->threshold.ppg_ir_threshold);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%s\n \n",
                 uistr_info_bio_sensor_result, (bio->result_chk & (1 << RESULT_PPG_IR)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing));

        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f\n",
                 biosensor_result[RESULT_PPG_R], biosensor_result[RESULT_PPG_R], bio->data.ppg_r);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f, %s:%+7.3f, %s:%+7.3f\n",
                 uistr_info_bio_sensor_min, bio->min_data.ppg_r,
                 uistr_info_bio_sensor_max, bio->max_data.ppg_r,
                 uistr_info_bio_sensor_thres, bio->threshold.ppg_r_threshold);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%s\n \n",
                 uistr_info_bio_sensor_result, (bio->result_chk & (1 << RESULT_PPG_R)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing));

        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f\n",
                 biosensor_result[RESULT_EKG], biosensor_result[RESULT_EKG], bio->data.ekg);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%+7.3f\n",
                 uistr_info_bio_sensor_thres, bio->threshold.ekg_threshold);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s:%s\n",
                 uistr_info_bio_sensor_result, (bio->result_chk & (1 << RESULT_EKG)) ? (uistr_info_sensor_pass) : (uistr_info_g_sensor_testing));

		if(len < 0)
		{
		   LOGE(TAG "%s: snprintf error \n", __FUNCTION__);
		   len = 0;
		}

        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
    }

    if ((err = biosensor_ftm_end(bio->fd)))
        FTGLOGE("biosensor_ftm_end() = (%s), %d\n", strerror(errno), err);

    biosensor_close(bio->fd);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

    return NULL;
}
/*---------------------------------------------------------------------------*/
int biosensor_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    struct bio_data *dat = (struct bio_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err, len;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);
       
    len = snprintf(dat->info, sizeof(dat->info), "%s\n", uistr_info_sensor_initializing);
    dat->exit_thd = false;  
    biosensor_thread_exit = false;


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
    iv->set_items(iv, biosensor_items, 0);
    iv->set_text(iv, &dat->text);
    sp_ata_status = param->test_type;
    pthread_create(&dat->update_thd, NULL, biosensor_update_iv_thread, priv);
    do {
        if(sp_ata_status == FTM_MANUAL_ITEM){
#ifdef FEATURE_FTM_TOUCH_MODE
			text_t lbtn;
			text_t cbtn;
			text_t rbtn;

			init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
			init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
			init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);

			iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif
        chosen = iv->run(iv, &biosensor_thread_exit);
        switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }           
            biosensor_thread_exit = true;            
            break;
#else /* FEATURE_FTM_TOUCH_MODE */
			case L_BTN_DOWN:
				dat->mod->test_result = FTM_TEST_FAIL;
				biosensor_thread_exit = true;
				break;
			case C_BTN_DOWN:
				biosensor_thread_exit = true;
				break;
			case R_BTN_DOWN:
				dat->mod->test_result = FTM_TEST_PASS;
				biosensor_thread_exit = true;
				break;
#endif /* FEATURE_FTM_TOUCH_MODE */
        }
        }
        iv->redraw(iv);
        pthread_mutex_lock (&biosensor_mutex);
        if (biosensor_thread_exit) {
            dat->exit_thd = true;
            pthread_mutex_unlock (&biosensor_mutex);
            break;
        }else{
            pthread_mutex_unlock (&biosensor_mutex);
            usleep(50000);
        }        
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int biosensor_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct bio_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_BIOSENSOR, sizeof(struct bio_data));
    dat  = mod_to_bio_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    biosensor_init_priv(&dat->bio);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, biosensor_entry, (void*)dat);

    return ret;
}
#endif 

