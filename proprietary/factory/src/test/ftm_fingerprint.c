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

#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include <cutils/properties.h>



#define TAG   "[Fingerprint] "
extern int status;

#ifdef FEATURE_FTM_FINGERPRINT

#define FINGERPRINT_ID "/sys/devices/soc/11012000.spi/spi_master/spi1/spi1.0/diag/chip_id"

extern int item_testing;
bool fingerprint_test_exit = false;
//static pthread_t vibrator_thread;

struct fingerprint_stu {    
    char   info[1024];
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct itemview *iv;
    pthread_t update_thread;
    bool   exit_thread;
	bool   test_done;
    int    test_case;
};


enum {
	ITEM_FINGERPRINT,
	ITEM_PASS,
	ITEM_FAIL
};

#define mod_to_fingerprint_stu(p)     (struct fingerprint_stu*)((char*)(p) + sizeof(struct ftm_module))

static item_t items[] = {
	item(ITEM_FINGERPRINT, uistr_fingerprint),
	item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1, NULL),
};

static int
read_str(char const* path, char *buf, int size)
{
	int fd;
    int ret = -1;

    if (path == NULL)
    	return -1;

    if (buf == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        char buffer[32] = {0};

    	read(fd, buffer, size);

    	//FLPLOGD("%d [%s] [%s] \n", (int) buf, buf, buffer);

        sprintf(buf, "%s", buffer);

    	//read(fd, *buf, size);

    	//FLPLOGD("%d [%s] [%s] \n", (int) buf, buf, buffer);

    	close(fd);
    	return -1;
    }
    else
    {
        sprintf(buf, "not exist\n");
    }
    return -errno;
}

static void *fingerprint_test_update_thread(void *priv)
{
    struct fingerprint_stu *fingerprintstu = (struct fingerprint_stu *)priv;

    LOGD(TAG "%s: Start1\n", __FUNCTION__);
    
    while (1) {
        LOGD(TAG "%s: in while, exit_thread:%d\n", __FUNCTION__, (fingerprintstu->exit_thread) ? 1 : 0);

        usleep(5000000);
        if (fingerprintstu->exit_thread) {
            LOGD(TAG "%s: Exit. fingerprintstu->exit_thread:%d\n", __FUNCTION__, fingerprintstu->exit_thread);
            break;
        }

        if (!fingerprintstu->test_done) {
            fingerprintstu->test_done = true;
        }
    }
    
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    
    return NULL;
}

/*
int fingerprint_manual_entry(struct ftm_param *param, void *priv)
{
	int chosen = -1;
	bool exit = false;
	struct itemview *iv;
	text_t    title;
	char* buffer = NULL;
	//struct ftm_module *mod = (struct ftm_module *)priv;
	struct fingerprint_stu *fingerprintstu = (struct fingerprint_stu *)priv;

	LOGD(TAG "%s\n", __FUNCTION__);

	fingerprint_test_exit = false;

	iv = ui_new_itemview();
	if (!iv) {
		LOGD("No memory\n");
		return -1;
	}
    init_text(&title, param->name, COLOR_YELLOW);
	LOGD("11111111111111111\n");
	iv->set_title(iv, &title);
	iv->set_items(iv, items, 0);

	iv->start_menu(iv, 0);
	iv->redraw(iv);

	fingerprintstu->exit_thread = false;
    fingerprintstu->test_done = true;
	
	pthread_create(&fingerprintstu->update_thread, NULL, fingerprint_test_update_thread, priv);
	
	LOGD("2222222222222222222222\n");
	do {
		if(param->test_type == FTM_MANUAL_ITEM){
		chosen = iv->run(iv, &exit);
		LOGD(TAG "%s -chosen = %d\n", __FUNCTION__, chosen);
		fingerprintstu->mod->test_result = FTM_TEST_FAIL;
		
		switch (chosen) {
			case ITEM_FINGERPRINT:
				read_str(FINGERPRINT_ID, buffer, 15);
				if (!strcmp(buffer, "fpc1145x rev.1")) {
					LOGD("fpc1145x is ok\n");
					fingerprintstu->mod->test_result = FTM_TEST_PASS;
				} else {
					LOGD("fpc1145x is failed, %s\n", buffer);
					fingerprintstu->mod->test_result = FTM_TEST_FAIL;
				}
			
				exit = true;
			break;
			case ITEM_PASS:
				fingerprintstu->mod->test_result = FTM_TEST_PASS;
			break;
			case ITEM_FAIL:
			default:
				fingerprintstu->exit_thread = true;
                fingerprintstu->test_done = true;
                exit = true;
		}

		if (exit) {
			fingerprintstu->exit_thread = true;
			fingerprint_test_exit = true;
			break;
		}
		}
		else{
			LOGD("33333333333333333333\n");
			fingerprintstu->exit_thread = true;
			fingerprint_test_exit = true;
			break;
		}
		usleep(1000000);
	} while (1);
	//pthread_join(vibrator_thread, NULL);
	fingerprintstu->exit_thread = true;
	LOGD(TAG "%s Exist.\n", __FUNCTION__);
	return 0;
}

int fingerprint_entry(struct ftm_param *param, void *priv)
{
	int ret = 0;
	struct fingerprint_stu *fingerprintstu = (struct fingerprint_stu *)priv;
	
	ret = fingerprint_manual_entry(param, fingerprintstu);

	if (ret) {
		LOGD(TAG "%s fail!! ret:%d\n", __FUNCTION__, ret);
	}
	return 0;
}
*/

int fingerprint_entry(struct ftm_param *param, void *priv)
{
	int chosen = -1;
	bool exit = false;
	struct itemview *iv;
	text_t    title;
	char buffer[32];
	//struct ftm_module *mod = (struct ftm_module *)priv;
	struct fingerprint_stu *fingerprintstu = (struct fingerprint_stu *)priv;

	LOGD(TAG "%s\n", __FUNCTION__);

	fingerprint_test_exit = false;

	iv = ui_new_itemview();
	if (!iv) {
		LOGD("No memory\n");
		return -1;
	}
    init_text(&title, param->name, COLOR_YELLOW);
	LOGD(TAG "11111111111111111\n");
	iv->set_title(iv, &title);
	iv->set_items(iv, items, 0);

	iv->start_menu(iv, 0);
	iv->redraw(iv);

	fingerprintstu->exit_thread = false;
    fingerprintstu->test_done = true;
	
	pthread_create(&fingerprintstu->update_thread, NULL, fingerprint_test_update_thread, priv);
	
	LOGD(TAG "2222222222222222222222\n");
	//param->test_type = FTM_MANUAL_ITEM;
	do {
		if(param->test_type == FTM_MANUAL_ITEM){
		chosen = iv->run(iv, &exit);
		}
		else {
		chosen = ITEM_FINGERPRINT;
		}
		if (1){
		LOGD(TAG "%s -chosen = %d\n", __FUNCTION__, chosen);
		fingerprintstu->mod->test_result = FTM_TEST_FAIL;
		
		switch (chosen) {
			case ITEM_FINGERPRINT:
				LOGD(TAG "4444444444444444444444\n");
				read_str(FINGERPRINT_ID, buffer, 14);
				LOGD(TAG "5555555555555555555555\n");
				if (!strcmp(buffer, "fpc1145x rev.1")) {
					LOGD(TAG "fpc1145x is ok\n");
					fingerprintstu->mod->test_result = FTM_TEST_PASS;
				} else {
					LOGD(TAG "fpc1145x is failed, %s\n", buffer);
					fingerprintstu->mod->test_result = FTM_TEST_FAIL;
				}
			
				exit = true;
			break;
			case ITEM_PASS:
				fingerprintstu->mod->test_result = FTM_TEST_PASS;
				fingerprintstu->exit_thread = true;
                fingerprintstu->test_done = true;
                exit = true;
			break;
			case ITEM_FAIL:
			default:
				fingerprintstu->exit_thread = true;
                fingerprintstu->test_done = true;
                exit = true;
			break;
		}

		if (exit) {
			fingerprintstu->exit_thread = true;
			fingerprint_test_exit = true;
			break;
		}
		}
		else{
			LOGD(TAG "33333333333333333333\n");
			fingerprintstu->exit_thread = true;
			fingerprint_test_exit = true;
			break;
		}
		usleep(1000000);
	} while (1);
	//pthread_join(vibrator_thread, NULL);
	fingerprintstu->exit_thread = true;
	LOGD(TAG "%s Exist.\n", __FUNCTION__);
	return 0;
}

int fingerprint_init(void)
{
	int ret = 0;
	struct ftm_module *mod;
	struct fingerprint_stu *fingerprintstu;

	LOGD(TAG "%s\n", __FUNCTION__);

	mod = ftm_alloc(ITEM_FINGERPRINT_TEST, sizeof(struct fingerprint_stu));
    if (!mod) {
        LOGD(TAG "%s - ftm_alloc error! err:%d\n", __FUNCTION__, errno);
        return -ENOMEM;
    }

	fingerprintstu = mod_to_fingerprint_stu(mod);

    /* init */
    fingerprintstu->mod = mod; 
	
    ret = ftm_register(mod, fingerprint_entry, (void *)fingerprintstu);

	if (ret) {
        LOGD(TAG "%s -register fingerprint_entry fail!! ret:%d\n", __FUNCTION__, ret);
    }

	return ret;
}

#endif // FEATURE_FTM_FINGERPRINT

