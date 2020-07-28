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
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include <unistd.h>
#include <pthread.h>
#ifdef FEATURE_FTM_WIFI

#define TAG                 "[FT_WIFI] "

enum {
	ITEM_PASS,
	ITEM_FAIL,
	ITEM_RENEW,
};

static item_t wifi_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
	item(ITEM_PASS,   uistr_info_wifi_test_pass),
	item(ITEM_FAIL,   uistr_info_wifi_test_fail),
	item(ITEM_RENEW,  uistr_info_wifi_renew),
#endif
	item(-1, NULL),
};

static item_t wifi_auto_items[] = {
/*	item(ITEM_PASS,   uistr_info_wifi_test_pass),
	item(ITEM_FAIL,   uistr_info_wifi_test_fail),
	item(ITEM_RENEW,  uistr_info_wifi_renew),*/
	item(-1, NULL),
};

struct wifi_factory {
	char	info[1024];
	bool	exit_thd;
	int	result;

	/* for UI display */
	text_t	title;
	text_t	text;

	pthread_t update_thd;
	struct ftm_module *mod;
	struct textview tv;
	struct itemview *iv;
	bool renew;
};
extern char result[3][16];

#define mod_to_wifi(p)     (struct wifi_factory*)((char*)(p) + sizeof(struct ftm_module))

extern void update_Text_Info(void *pApInfo, char *output_buf, int buf_len);
extern int FM_WIFI_init(char *output_buf, int buf_len, int *p_result);
extern int FM_WIFI_deinit(void);
extern int wifi_disconnect(void);
extern int wifi_fm_test(void);
extern int wifi_update_status(void);

static void *wifi_update_thread(void *priv)
{
	struct wifi_factory *wififm = (struct wifi_factory *)priv;
	//struct textview *tv = &wififm->tv;
	struct itemview *iv = wififm->iv;
	char *wifi_test_error_str[] = {uistr_info_wifi_iface_err,
				       uistr_info_wifi_fail_scan,
				       uistr_info_wifi_no_scan_res,
				       uistr_info_wifi_connect_err,
				       uistr_info_wifi_no_ap};
	int error_index = 0;

	LOGD(TAG "%s: Start\n", __FUNCTION__);

	//update_Text_Info(NULL, wififm->info, sizeof(wififm->info));
	memset(wififm->info, 0, sizeof(wififm->info));
	sprintf(wififm->info,
		"%s : %s\n",
		uistr_info_wifi_status,
		uistr_info_wifi_start);
	iv->redraw(iv);

	if (FM_WIFI_init(wififm->info, sizeof(wififm->info), &wififm->result) < 0) {
		LOGE("[WIFI] FM_WIFI_init failed!\n");
		sprintf(wififm->info,
			"%s : %s\n",
			uistr_info_wifi_status,
			uistr_info_wifi_init_fail);
		iv->redraw(iv);
		goto init_fail;
	}

	while (!wififm->exit_thd) {
		if (wififm->renew) {
			wififm->renew = false;
			memset(wififm->info,0, sizeof(wififm->info));
			sprintf(wififm->info,
				"%s : %s\n",
				uistr_info_wifi_status,
				uistr_info_wifi_scanning);
			iv->redraw(iv);

			error_index = wifi_fm_test();
			if (error_index > 0) {
				LOGD("[wifi_update_thread] %s!\n", wifi_test_error_str[error_index-1]);
				sprintf(wififm->info,
					"%s : %s\n",
					uistr_info_wifi_error,
					wifi_test_error_str[error_index-1]);
				iv->redraw(iv);
			} else {
				int count =50;
				iv->redraw(iv);
				while (count-- >0) {
					if (wififm->exit_thd || wififm->renew)
						break;
					if (wifi_update_status() < 0) {
						if (count)
							usleep(100000);
						else {
							memset(wififm->info,0, sizeof(wififm->info));
							sprintf(wififm->info,
								"%s : %s\n",
								uistr_info_wifi_status,
								uistr_info_wifi_timeout);
						}
					}
					iv->redraw(iv);
				}
			}
		}
		usleep(200000);
	}

init_fail:
	LOGD(TAG "%s: Exit\n", __FUNCTION__);
	pthread_exit(NULL);
	return NULL;
}

static void *wifi_update_thread_test(void *priv)
{
	struct wifi_factory *wififm = (struct wifi_factory *)priv;
	//struct textview *tv;
	struct itemview *iv;
	int i = 0;
	iv = wififm->iv;
	char wifi_result[32] = {0};

	int curStatus = -1; //disconnect

	if (FM_WIFI_init(wififm->info, sizeof(wififm->info), &wififm->result) < 0) {
		LOGE("[WIFI] FM_WIFI_init failed!\n");
		sprintf(wififm->info,
			"%s : %s\n",
			uistr_info_wifi_status,
			uistr_info_wifi_init_fail);
		iv->redraw(iv);
		sprintf(wifi_result, "%d:%s", wififm->mod->id, result[FTM_TEST_FAIL]);
		LOGD(TAG "WIFI result:%s", wifi_result);
		write_data_to_pc(wifi_result, strlen(wifi_result));
		return NULL;
	}

	while (i++ < 3) {
		//1. disconnect the connection.
		wifi_disconnect();

		//2. connect to the AP.
		if (wifi_fm_test() > 0) {
			LOGD("[WIFI][wifi_update_thread_test] wifi_fm_test failed!\n");
		} else {
			int count =10;
			iv->redraw(iv);
			while (count-- >0) {
				/* if (wififm->exit_thd || wififm->renew)
					break; */
				if (wifi_update_status() < 0) {
					if (count)
						usleep(100000);
					else {
						memset(wififm->info, 0, sizeof(wififm->info));
						sprintf(wififm->info,
							"%s : %s\n",
							uistr_info_wifi_status,
							uistr_info_wifi_timeout);
					}
				} else {
					curStatus = 1;  //connected
					//show the message
				}
				iv->redraw(iv);
			}
		}

		//3. check the status
		if (curStatus == 1) {
			wififm->mod->test_result = FTM_TEST_PASS;
			LOGD(TAG "while %d connected\n", i);
			break;
		}
	}

	if (3 == i) {
		wififm->mod->test_result = FTM_TEST_FAIL;
		LOGD(TAG "while i == 3\n");
	}

	FM_WIFI_deinit();
	LOGD(TAG "wififm->mod->id:%d; wififm->mod->test_result:%d", wififm->mod->id, wififm->mod->test_result);
	//if(get_is_ata())
	{
		sprintf(wifi_result, "%d:%s", wififm->mod->id, result[wififm->mod->test_result]);
		LOGD(TAG "WIFI result:%s", wifi_result);
		write_data_to_pc(wifi_result, strlen(wifi_result));
	}

	return NULL;
}

/*
static int wifi_key_handler(int key, void *priv)
{
	int handled = 0, exit = 0;
	struct wifi_factory *wififm = (struct wifi_factory *)priv;
	struct textview *tv = &wififm->tv;
	struct ftm_module *fm = wififm->mod;

	switch (key) {
		case UI_KEY_RIGHT:
			exit = 1;
			break;
		case UI_KEY_LEFT:
			fm->test_result = FTM_TEST_FAIL;
			exit = 1;
			break;
		case UI_KEY_CENTER:
			fm->test_result = FTM_TEST_PASS;
			exit = 1;
			break;
		case UI_KEY_DOWN:
			wififm->renew = true;
			exit = 0;
			break;
		default:
			handled = -1;
			break;
	}

	if (exit) {
		LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
		wififm->exit_thd = true;
		tv->exit(tv);
	}
	return handled;
}
*/

int wifi_entry(struct ftm_param *param, void *priv)
{
	//char *ptr;
	int chosen;
	bool exit = false;
	struct wifi_factory *wififm = (struct wifi_factory *)priv;
	//struct textview *tv;
	struct itemview *iv;
	int i = 0;
	int curStatus = -1; //disconnect
	char *wifi_test_error_str[] = {uistr_info_wifi_iface_err,
				       uistr_info_wifi_fail_scan,
				       uistr_info_wifi_no_scan_res,
				       uistr_info_wifi_connect_err,
				       uistr_info_wifi_no_ap};
	int error_index = 0;

	LOGD(TAG "%s new\n", __FUNCTION__);

	memset(&wififm->info[0], 0, sizeof(wififm->info));
	memset(&wififm->info[0], '\n', 10);

	init_text(&wififm->title, param->name, COLOR_YELLOW);
	init_text(&wififm->text, &wififm->info[0], COLOR_YELLOW);

	if (!wififm->iv) {
		iv = ui_new_itemview();
		if (!iv) {
			LOGD(TAG "No memory for item view");
			return -1;
		}
		wififm->iv = iv;
	}

	iv = wififm->iv;
	iv->set_title(iv, &wififm->title);
	if (param->test_type == FTM_MANUAL_ITEM)
		iv->set_items(iv, wifi_items, 0);
	else
		iv->set_items(iv, wifi_auto_items, 0);
	iv->set_text(iv, &wififm->text);
	iv->start_menu(iv,0);

	iv->redraw(iv);
	LOGD(TAG "%s new2 test_type=%d\n", __FUNCTION__, param->test_type);
/*
	tv = &wififm->tv;
	ui_init_textview(tv, wifi_key_handler, (void*)wififm);
	tv->set(tv, "Wi-Fi", &wififm->info[0]);
	tv->set_btn(tv, "Fail", "Pass", "Back");
	tv->run(tv);
*/

	if (param->test_type == FTM_MANUAL_ITEM) {
		// manual test
#ifdef FEATURE_FTM_TOUCH_MODE
		text_t lbtn;
		text_t cbtn;
		text_t rbtn;
		init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
		init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
		init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);
		iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif

		/* initialize thread condition */
		wififm->exit_thd = false;
		wififm->result = false;
		wififm->renew = true;
		pthread_create(&wififm->update_thd, NULL, wifi_update_thread, priv);

		do {
			chosen = iv->run(iv, &exit);
			switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
			case ITEM_RENEW:
				wififm->renew = true;
				break;

			case ITEM_PASS:
			case ITEM_FAIL:
				if (chosen == ITEM_PASS)
					wififm->mod->test_result = FTM_TEST_PASS;
				else if (chosen == ITEM_FAIL)
					wififm->mod->test_result = FTM_TEST_FAIL;

				exit = true;
				break;
#endif

#ifdef FEATURE_FTM_TOUCH_MODE
			case L_BTN_DOWN:
				wififm->mod->test_result = FTM_TEST_FAIL;
				exit = true;
				break;
			case C_BTN_DOWN:
				exit = true;
				break;
			case R_BTN_DOWN:
				wififm->mod->test_result = FTM_TEST_PASS;
				exit = true;
				break;
#endif
			}

			if (exit) {
				wififm->exit_thd = true;
				break;
			}
		} while (1);
		pthread_join(wififm->update_thd, NULL);
	} else if (param->test_type == FTM_AUTO_ITEM) {
		// auto test
		if (FM_WIFI_init(wififm->info, sizeof(wififm->info), &wififm->result) < 0) {
			LOGE("[WIFI][FTM_AUTO_ITEM] FM_WIFI_init failed!\n");
			sprintf(wififm->info,
				"%s : %s\n",
				uistr_info_wifi_status,
				uistr_info_wifi_init_fail);
			iv->redraw(iv);
			return -1;
		}

		while (i++ < 3) {
			//1. disconnect the connection.
			wifi_disconnect();
			//2. connect to the AP.
			error_index = wifi_fm_test();
			if (error_index > 0) {
				LOGD("[WIFI][FTM_AUTO_ITEM] wifi_fm_test failed %d!\n", error_index);
				sprintf(wififm->info,
					"%s : %s\n",
					uistr_info_wifi_error,
					wifi_test_error_str[error_index-1]);
				iv->redraw(iv);
			} else {
				int count =10;
				iv->redraw(iv);
				while (count-- >0) {
					/* if (wififm->exit_thd || wififm->renew)
					    break; */
					if (wifi_update_status() < 0) {
						if(count)
							usleep(100000);
						else{
							memset(wififm->info,0, sizeof(wififm->info));
							sprintf(wififm->info, "%s : %s\n", uistr_info_wifi_status, uistr_info_wifi_timeout);
						}
					} else {
						curStatus = 1;  //connected
						//show the message
					}
					iv->redraw(iv);
				}
			}

			//3. check the status
			if (curStatus == 1) {
			    wififm->mod->test_result = FTM_TEST_PASS;
			    LOGD(TAG "while %d connected\n", i);
			    break;
			}
		}

		if (curStatus != 1) {
			wififm->mod->test_result = FTM_TEST_FAIL;
			LOGD(TAG "while i == %d failed\n", i);
		}

	} else if (param->test_type == FTM_ASYN_ITEM) {
		LOGD(TAG "param->test_type == FTM_ASYN_ITEM\n");
		pthread_create(&wififm->update_thd, NULL, wifi_update_thread_test, priv);
		// if(3 == param->test_type)
		{
			// pthread_join(wififm->update_thd, NULL);
		}
	}

	if (param->test_type != FTM_ASYN_ITEM)
		FM_WIFI_deinit();

	return 0;
}

int wifi_init(void)
{
	int ret = 0;
	struct ftm_module *mod;
	struct wifi_factory *wififm;

	LOGD(TAG "%s\n", __FUNCTION__);

	mod = ftm_alloc(ITEM_WIFI, sizeof(struct wifi_factory));
	if (!mod)
		return -ENOMEM;

	wififm = mod_to_wifi(mod);
	wififm->mod = mod;

	ret = ftm_register(mod, wifi_entry, (void*)wififm);

	return ret;
}
#endif
