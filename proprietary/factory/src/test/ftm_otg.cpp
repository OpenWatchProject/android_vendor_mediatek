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
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <linux/ioctl.h>

#include "common.h"
#include "miniui.h"
#undef BLOCK_SIZE
#include "ftm.h"
#define TEST_IDDIG_5V_ONLY 1
#define TEST_CHECK_BUSDEVICE 1
#ifdef FEATURE_FTM_OTG

#ifdef __cplusplus
extern "C" {
#endif


#define TAG                 "[OTG] "
#define OTG_STATE_PATH "/sys/class/switch/otg_state/state"
#define MOUNT_PATH "/storage/usbotg"
#define DEV_PATH "/dev/block/sda"
#define DEV_PATH2 "/dev/block/sda1"
#define DEV_BUS_PATH "/sys/bus/usb/devices/1-1/speed"
#define BUF_LEN 1
char r_buf[BUF_LEN] = {'\0'};
char w_buf[BUF_LEN] = {'1'};


enum {
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t otg_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
	item(ITEM_PASS, uistr_pass),
	item(ITEM_FAIL, uistr_fail),
#endif
    item(-1, NULL),
};

struct otg {
    char  info[1024];
	bool  avail;
    bool  exit_thd;

    pthread_t otg_update_thd;
    struct ftm_module *mod;
    struct itemview *iv;

    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
};

#define mod_to_otg(p)     (struct otg*)((char*)(p) + sizeof(struct ftm_module))


// use for ioctl for otg driver
static int OtgFd =0;

extern int usb_com_port;
extern int usb_plug_in;
extern int idle_current_done;
extern int com_type;
extern int uart_exit;

static void otg_update_info(struct otg *hds, char *info)
{
  char *ptr;
  int rc;
	int fd = -1;
	int fd_state = -1;
	int hb_status = 0;
	int ret = 0;
	int flags;
	int device_state = 0;
	struct statfs stat;

	hds->avail = false;
#if 0
	fd = open(OTG_STATE_PATH, O_RDONLY, 0);
	if (fd < 0) {
		LOGD(TAG "Can't open %s\n", OTG_STATE_PATH);
		goto EXIT;
	}
	if (read(fd, r_buf, BUF_LEN) < 0) {
		LOGD(TAG "Can't read %s\n",OTG_STATE_PATH);
		goto EXIT2;
	}

    LOGD("OTG state is %s\n",r_buf);
#endif
#if TEST_CHECK_BUSDEVICE
	fd_state = open(DEV_BUS_PATH, O_RDONLY, 0);
	if (fd_state < 0) {
		LOGD(TAG "Can't open %s\n", DEV_BUS_PATH);
		goto EXIT;
	}
	else
		device_state = 1;

#endif

#if !TEST_IDDIG_5V_ONLY
    if (strncmp(w_buf, r_buf, BUF_LEN)) { /*the same*/
        goto EXIT;
    }

	flags = MS_NODEV | MS_NOEXEC | MS_NOSUID | MS_DIRSYNC;
	rc = mount(DEV_PATH, MOUNT_PATH, "vfat", flags,
		"utf8,uid=1000,gid=1015,fmask=702,dmask=702,shortname=mixed");
	if(rc)
		rc = mount(DEV_PATH2, MOUNT_PATH, "vfat", flags,
			"utf8,uid=1000,gid=1015,fmask=702,dmask=702,shortname=mixed");

    if(rc){
		LOGE(TAG "%s: mount fail, %d (%s)\n", __FUNCTION__, errno, strerror(errno));
		goto EXIT;
	}

	rc = statfs(MOUNT_PATH, &stat);
	if(rc){
		LOGE(TAG "%s: statfs fail, %d (%s)\n", __FUNCTION__, errno, strerror(errno));
		goto EXIT3;
	}

	hds->avail = true;

EXIT3:
	umount(MOUNT_PATH);
#endif
EXIT22:
	close(fd_state);
#if 0
EXIT2:
	close(fd);
#endif
EXIT:
    /* preare text view info */
    ptr  = info;
    #if !TEST_IDDIG_5V_ONLY
    if(hds->avail)
            ptr += sprintf(ptr, "Total Size: %d MB\n",(unsigned int)(stat.f_blocks * stat.f_bsize >> 20));
    else
            ptr += sprintf(ptr, "Cannot access storage\n");
    #else
#if 0
    if (strcmp(r_buf, "0") == 0)
            ptr += sprintf(ptr, "Device mode, NO OTG devices!\n");
    else if (strcmp(r_buf, "1") == 0) {
		if(device_state == 1)
            ptr += sprintf(ptr, "Host mode, OTG devices plugged in, 5V applied!!\n");
		else
            ptr += sprintf(ptr, "Host mode, OTG cable plugged in!!\n");
    }
    else if (strcmp(r_buf, "2") == 0)
            ptr += sprintf(ptr, "Host mode, OTG devices plugged in, 5V applied!!\n");
    else
            ptr += sprintf(ptr, "Fail!!\n");
#else
    if(device_state == 1)
        ptr += sprintf(ptr, "OTG devices plugged in, 5V applied!!\n");
    else
        ptr += sprintf(ptr, "OTG test fail !!\n");
#endif
    #endif
    return;
}


static void *otg_update_iv_thread(void *priv)
{
    struct otg *hds = (struct otg *)priv;
    struct itemview *iv = hds->iv;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    while (1) {
        usleep(200000);

        if (hds->exit_thd)
            break;

		otg_update_info(hds, hds->info);
		iv->set_text(iv, &hds->text);
		iv->redraw(iv);
    }

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

	return NULL;
}

int otg_entry(struct ftm_param *param, void *priv)
{
    char *ptr;
    int chosen;
    bool exit = false;
    struct otg *hds = (struct otg *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    otg_update_info(hds, hds->info);

    hds->exit_thd = false;

	if (!hds->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;

    #ifdef FEATURE_FTM_TOUCH_MODE
    text_t lbtn ;
    text_t cbtn ;
    text_t rbtn ;
    init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
    init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
    init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
    iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
    #endif

    iv->set_title(iv, &hds->title);
    iv->set_items(iv, otg_items, 0);
    iv->set_text(iv, &hds->text);

    pthread_create(&hds->otg_update_thd, NULL, otg_update_iv_thread, priv);
    do {
        if(!get_is_ata()) {
            chosen = iv->run(iv, &exit);
        }
        else {
			if(1 == com_type) //UART com port
			{
				while(!get_uart_exit())
				{
					sleep(100);
				}
				uart_exit = 0;
				chosen = ITEM_PASS;
			}
			else if(2 == com_type) //USB com port
			{
				LOGD(TAG "wait for USB disconnect\n");
				while(is_usb_state_plugin())
				{
					sleep(1);
				}
				LOGD(TAG "close usb\n");
				close_usb();
				LOGD(TAG "wait for USB connect\n");
				while(!is_usb_state_plugin())
				{
					sleep(1);
				}
				usb_plug_in = 1;
				idle_current_done = 1;
				chosen = ITEM_PASS;
		    }
		    else
		    {
				LOGD(TAG "Invalid com type : %d\n", com_type);
				return -1;
			}
        }
        
        switch (chosen) {
        #ifndef FEATURE_FTM_TOUCH_MODE
        case ITEM_PASS:
        case ITEM_FAIL:
            if (chosen == ITEM_PASS) {
                hds->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                hds->mod->test_result = FTM_TEST_FAIL;
            }
            exit = true;
            break;
        #endif

        #ifdef FEATURE_FTM_TOUCH_MODE
        case L_BTN_DOWN:
            hds->mod->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case C_BTN_DOWN:
            exit = true;
            break;
        case R_BTN_DOWN:
            hds->mod->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        #endif
        }

        if (exit) {
            hds->exit_thd = true;
            break;
        }
    } while (1);
    pthread_join(hds->otg_update_thd, NULL);

    return 0;
}

int otg_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct otg *hds;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_OTG, sizeof(struct otg));
    hds = mod_to_otg(mod);

    hds->mod    = mod;
		hds->avail	= false;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, otg_entry, (void*)hds);

    return ret;
}


#ifdef __cplusplus
}
#endif

#endif
