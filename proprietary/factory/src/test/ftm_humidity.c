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

#ifdef CUSTOM_KERNEL_HUMIDITY
#include <linux/sensors_io.h>
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[HUMIDITY] "
#define mod_to_hmdy_data(p) (struct hmdy_data*)((char*)(p) + sizeof(struct ftm_module))
#define FBLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FBLOGE(fmt, arg ...) LOGE(TAG"%s [%5d]: " fmt, __func__, __LINE__, ##arg)

#define INVALID_T_VAL -273.150*1000 //0K = -273.15
#define INVALID_H_VAL -1.0*1000
#define MAX_T_VAL 100.0 * 1000
#define MIN_T_VAL 0.0 * 1000
#define MAX_H_VAL 200.0*1000
#define MIN_H_VAL 0.0*1000

/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t humidity_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   "Test Pass"),
    item(ITEM_FAIL,   "Test Fail"),
#endif
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct hmdy_priv
{
    /*specific data field*/
    char    *dev;
    int     fd;
    float temp_raw;
    float humidity_raw;
};
/*---------------------------------------------------------------------------*/
struct hmdy_data
{
    struct hmdy_priv hmdy;

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
    //struct textview tv;
    struct itemview *iv;
};
/******************************************************************************
 * Functions
 *****************************************************************************/
static int humidity_init_priv(struct hmdy_priv *hmdy)
{
    memset(hmdy, 0x00, sizeof(*hmdy));
    hmdy->fd = -1;
    hmdy->dev = "/dev/humidity";
    hmdy->temp_raw = INVALID_T_VAL;
    hmdy->humidity_raw = INVALID_H_VAL;

    return 0;
}
/*---------------------------------------------------------------------------*/
static int humidity_open(struct hmdy_priv *hmdy)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    err = 0;
    if (hmdy->fd == -1)
	{
          hmdy->fd = open("/dev/humidity", O_RDONLY);
          if (hmdy->fd < 0)
		  {
            FBLOGE("Couldn't open '%s' (%s)", hmdy->dev, strerror(errno));
            return -1;
          }
		  //
		  while ((err = ioctl(hmdy->fd, HUMIDITY_IOCTL_INIT, &flags)) && (retry ++ < max_retry))
          	usleep(retry_period*1000);
          if (err)
		  {
             FBLOGE("read humidity : %d(%s)\n", errno, strerror(errno));
             return err;
         }


    }
    FBLOGD("%s() %d\n", __func__, hmdy->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int humidity_close(struct hmdy_priv *hmdy)
{
    unsigned int flags = 0;
    int err;
    if (hmdy->fd != -1)
	{
        close(hmdy->fd);
    }
    memset(hmdy, 0x00, sizeof(*hmdy));
    hmdy->fd = -1;
    hmdy->dev = "/dev/humidity";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int humidity_update_info(struct hmdy_priv *hmdy)
{
    int err = -EINVAL;
    int temp_dat = 0, humidity_dat = 0;
    if (hmdy->fd == -1) {
        FBLOGE("invalid fd\n");
        return -EINVAL;
    } else if ((err = ioctl(hmdy->fd, HUMIDITY_GET_HMDY_DATA, &humidity_dat))) {
        FBLOGE("read humidity : %d(%s)\n", errno, strerror(errno));
        return err;
    } else if ((err = ioctl(hmdy->fd, HUMIDITY_GET_TEMP_DATA, &temp_dat))) {
        FBLOGE("read temp: %d(%s)\n", errno, strerror(errno));
        return err;
    }
	
    hmdy->temp_raw = temp_dat;
    hmdy->humidity_raw = humidity_dat;
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *humidity_update_iv_thread(void *priv)
{
    struct hmdy_data *dat = (struct hmdy_data *)priv;
    struct hmdy_priv *hmdy = &dat->hmdy;
    struct itemview *iv = dat->iv;
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = humidity_open(hmdy))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
        iv->redraw(iv);
        FBLOGE("humidity() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }

    while (1) {

        if (dat->exit_thd)
            break;

        if ((err = humidity_update_info(hmdy)))
            continue;

        len = 0;
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "TEMP: %f \n", hmdy->temp_raw/1000);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "HUMIDITY : %f\n", hmdy->humidity_raw/1000);
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
    }
    humidity_close(hmdy);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    return NULL;
}
/*---------------------------------------------------------------------------*/
int humidity_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct hmdy_data *dat = (struct hmdy_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;
    int i;
    float humidity_tmp;
    bool humidity_constant = true;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    init_text(&dat->left_btn, "Fail", COLOR_YELLOW);
    init_text(&dat->center_btn, "Pass", COLOR_YELLOW);
    init_text(&dat->right_btn, "Back", COLOR_YELLOW);

    snprintf(dat->info, sizeof(dat->info), "Initializing...\n");
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
    iv->set_items(iv, humidity_items, 0);
    iv->set_text(iv, &dat->text);
    #ifdef FEATURE_FTM_TOUCH_MODE
    text_t lbtn ;
    text_t cbtn ;
    text_t rbtn ;
    init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
    init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
    init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
    iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
    #endif
    pthread_create(&dat->update_thd, NULL, humidity_update_iv_thread, priv);
    if (FTM_AUTO_ITEM == param->test_type) {
        dat->mod->test_result = FTM_TEST_PASS;
        usleep(1000000);
        humidity_tmp = dat->hmdy.humidity_raw;
        for (i=0; i<400; i++) {
            if (dat->hmdy.temp_raw > MAX_T_VAL || dat->hmdy.temp_raw < MIN_T_VAL ) {
                LOGE(TAG "Error: Temperature = %.4f out of range", dat->hmdy.temp_raw/1000);
                dat->mod->test_result = FTM_TEST_FAIL;
                break;
            }
            if (dat->hmdy.humidity_raw > MAX_H_VAL || dat->hmdy.humidity_raw < MIN_H_VAL ) {
                LOGE(TAG "Error: Humidity = %.4f out of range", dat->hmdy.humidity_raw/1000);
                dat->mod->test_result = FTM_TEST_FAIL;
                break;
            }
            if (humidity_tmp != dat->hmdy.humidity_raw)
                humidity_constant = false;
            usleep(5000);
        }
        if (humidity_constant == true) { //Humidity not change all the time, may something wrong
            LOGE(TAG "Error: Humidity = %.4f is constant!!!", dat->hmdy.humidity_raw/1000);
            dat->mod->test_result = FTM_TEST_FAIL;
        }
        dat->exit_thd = true;
    } else {
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
		#ifndef FEATURE_FTM_TOUCH_MODE
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
		#else
        case L_BTN_DOWN:
            dat->mod->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case C_BTN_DOWN:
            exit = true;
            break;
        case R_BTN_DOWN:
            dat->mod->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        #endif
        }

        if (exit) {
            dat->exit_thd = true;
            break;
        }
    } while (1);
    }
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int humidity_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct hmdy_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_HUMIDITY, sizeof(struct hmdy_data));
    dat  = mod_to_hmdy_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    humidity_init_priv(&dat->hmdy);

    /*NOTE: the assignment MUST be done, or exception happens when tester humidity Test Pass/Test Fail*/
    dat->mod = mod;

    if (!mod){
		LOGD(TAG "ftm_alloc error\n");
		return -ENOMEM;
	}


    ret = ftm_register(mod, humidity_entry, (void*)dat);

	LOGD(TAG "ftm_register ret:%d\n", ret);
    return ret;
}
#endif

