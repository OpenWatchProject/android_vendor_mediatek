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

#ifdef CUSTOM_KERNEL_HEART
#include <linux/sensors_io.h>
/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[HRM] "
#define mod_to_hrm_data(p) (struct hrm_data*)((char*)(p) + sizeof(struct ftm_module))
#define FBLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FBLOGE(fmt, arg ...) LOGE("%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t hrm_items[] = {
    item(ITEM_PASS,   "Test Pass"),
    item(ITEM_FAIL,   "Test Fail"),
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct hrm_priv
{
    /*specific data field*/
    char    *dev;
    int     fd;
    int hrm_raw[3];
};
/*---------------------------------------------------------------------------*/
struct hrm_data
{
    struct hrm_priv hrm;

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
static int hrm_init_priv(struct hrm_priv *hrm)
{
    memset(hrm, 0x00, sizeof(*hrm));
    hrm->fd = -1;
    hrm->dev = "/dev/hrm";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int hrm_open(struct hrm_priv *hrm)
{
    int err, max_retry = 3, retry_period = 100, retry;
    unsigned int flags = 1;
    err = 0;
    if (hrm->fd == -1) 
	{
        hrm->fd = open("/dev/hrm", O_RDONLY);
        if (hrm->fd < 0) 
		{
            FBLOGE("Couldn't open '%s' (%s)", hrm->dev, strerror(errno));
            return -1;
        }

		retry = 0;
		while ((err = ioctl(hrm->fd, HRM_IOCTL_INIT, &flags)) && (retry ++ < max_retry))
          	usleep(retry_period*1000);
        if (err) 
        {
            FBLOGE("read hrm : %d(%s)\n", errno, strerror(errno));
            return -1;            
        }
		  //
		  //if ((err = ioctl(hrm->fd, HRM_IOCTL_INIT, &flags))) 
		  //{
          //   FBLOGE("read hrm : %d(%s)\n", errno, strerror(errno));
          //   return err;
          //}

    }
    FBLOGD("%s() %d\n", __func__, hrm->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int hrm_close(struct hrm_priv *hrm)
{
    unsigned int flags = 0;
    int err;
    if (hrm->fd != -1) 
	{
        close(hrm->fd);
    }
    memset(hrm, 0x00, sizeof(*hrm));
    hrm->fd = -1;
    hrm->dev = "/dev/hrm";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int hrm_update_info(struct hrm_priv *hrm)
{
    int err = -EINVAL;
    int i, x, y, z;
    static char buf[128];
    
    if (hrm->fd == -1) {
        FBLOGE("invalid fd\n");
        return -EINVAL;
    } else if ((err = ioctl(hrm->fd, HRM_READ_SENSOR_DATA, buf))) {
        FBLOGE("read hrm : %d(%s)\n", errno, strerror(errno));
        return err;
    }

    err = sscanf(buf, "%d %d %d", &x, &y, &z);
    if(3!= err)
    {
        FBLOGE("read format fail: %s(%d)\n", strerror(err), err);
    }
    else
    {
        hrm->hrm_raw[0] = x;
        hrm->hrm_raw[1] = y;
        hrm->hrm_raw[2] = z;
    }

    for (i=0; i<3; i++) FBLOGE("@@@@@@==>@@@@ %d", hrm->hrm_raw[i]);
    return 0;
}
/*---------------------------------------------------------------------------*/
static void *hrm_update_iv_thread(void *priv)
{
    struct hrm_data *dat = (struct hrm_data *)priv; 
    struct hrm_priv *hrm = &dat->hrm;
    struct itemview *iv = dat->iv;    
    int err = 0, len = 0;
    char *status;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = hrm_open(hrm))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, "INIT FAILED\n");
        iv->redraw(iv);
        FBLOGE("hrm() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }
        
    while (1) {
        
        if (dat->exit_thd)
            break;
            
        if ((err = hrm_update_info(hrm)))
            continue;     

        len = 0;
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "Data: %d %d\n", hrm->hrm_raw[1], hrm->hrm_raw[2]);
        len += snprintf(dat->info+len, sizeof(dat->info)-len, "Heart Beat: %d \n", hrm->hrm_raw[0]);      
        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
    }
    hrm_close(hrm);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);    
    pthread_exit(NULL);    
    return NULL;
}
/*---------------------------------------------------------------------------*/
int hrm_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct hrm_data *dat = (struct hrm_data *)priv;
    struct textview *tv;
    struct itemview *iv;
    struct statfs stat;
    int err;

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
    iv->set_items(iv, hrm_items, 0);
    iv->set_text(iv, &dat->text);
    
    pthread_create(&dat->update_thd, NULL, hrm_update_iv_thread, priv);
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                dat->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                dat->mod->test_result = FTM_TEST_FAIL;
            }           
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
int hrm_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct hrm_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);
    
    mod = ftm_alloc(ITEM_HEART_MONITOR, sizeof(struct hrm_data));
    dat  = mod_to_hrm_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    hrm_init_priv(&dat->hrm);
        
    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/    
    dat->mod = mod; 
    
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, hrm_entry, (void*)dat);

    return ret;
}

#endif 

