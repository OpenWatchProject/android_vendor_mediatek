/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include <unistd.h>
#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"


#ifdef FEATURE_FTM_AP_AUXADC

#define AUXADC_LOG_TAG   			"[ap-auxadc] "
#define AP_AUXADC_PATH	"/sys/class/mtk-adc-cali/mtk-adc-cali/AUXADC_read_channel"
#define AP_AUXADC_IIO_PATH	"/sys/bus/platform/drivers/mt6577-auxadc"

#define BUF_LEN   					1024
#define MAX_CHN_NUM				16/* channel number */
#define RECOG_STRING_LEN			32/* string number of recognition of read buffer */
#define RECOG_STRING_EFUSE_LEN	60/* string number of efuse of read buffer */

 /* Jump to the auxadc struct, and don't care the default header of  struct ftm_module */
#define mod_to_auxadc(p)	(struct st_auxadc *)((char *)(p) + sizeof(struct ftm_module))

 struct st_auxadc {
	/* common header of factory mode List */
	char	 info[BUF_LEN];
	bool thread_exit_flag;
	pthread_t thread_id;
	int fd;

	struct ftm_module *mod;
	struct itemview *iv;

	text_t title;
	text_t text;
	text_t left_btn;
	text_t center_btn;
	text_t right_btn;
 };

enum
{
   ITEM_PASS,
   ITEM_FAIL,
};

static item_t auxadc_items[] =
{
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
#endif
    item(-1, NULL),
};

/* efuse param */
static unsigned int cali_reg;
static unsigned int gain;
static unsigned int cali_ge_a;
static unsigned int cali_oe_a;
static int cali_oe;
static int cali_ge;

static int pre_total = RECOG_STRING_EFUSE_LEN + (MAX_CHN_NUM * RECOG_STRING_LEN) -1;

#if 0
static int auxadc_open(void *priv)
{
	struct st_auxadc *auxadc_data = (struct st_auxadc *)priv;

	/* open fd */
	auxadc_data->fd = open(AP_AUXADC_PATH, O_RDONLY, 0);
	if(auxadc_data->fd < 0) {
		auxadc_data->fd = -1;
		LOGE(AUXADC_LOG_TAG"ap_auxadc_open file(%s) failed\n", AP_AUXADC_PATH);
		return -1;
	}

	return 0;
}

static int auxadc_close(void *priv)
{
	struct st_auxadc *auxadc_data = (struct st_auxadc *)priv;

	/* close fd */
	 if(auxadc_data->fd >= 0) {
		 close(auxadc_data->fd);
		auxadc_data->fd = -1;
	 } else {
		return -1;
	 }
	return 0;
}
#endif
char *auxadc_sample_data[MAX_CHN_NUM] = {
	"in_voltage0_input",
	"in_voltage1_input",
	"in_voltage2_input",
	"in_voltage3_input",
	"in_voltage4_input",
	"in_voltage5_input",
	"in_voltage6_input",
	"in_voltage7_input",
	"in_voltage8_input",
	"in_voltage9_input",
	"in_voltage10_input",
	"in_voltage11_input",
	"in_voltage12_input",
	"in_voltage13_input",
	"in_voltage14_input",
	"in_voltage15_input"
};
/* Target to find /sys/bus/platform/drivers/mt6577-auxadc/XXX.auxadc/iio:deviceX */
int find_auxadc_iio_path(char * temp_path)
{
	DIR *dir;
	struct dirent *entry;
	/* str1 is like 11001000.auxadc and str2 is like iio:device2 */
	bool str1_exist = false, str2_exist = false;
	char *iio_path1 = NULL, *iio_path2 = NULL;
	dir = opendir(AP_AUXADC_IIO_PATH);
	if (dir) {
		while ((entry = readdir(dir)) != NULL) {
			if (strstr(entry->d_name, "auxadc")) {
				str1_exist = true;
				asprintf(&iio_path1, "%s/%s", AP_AUXADC_IIO_PATH, entry->d_name);
				break;
			}
		}
		closedir(dir);
		if (!str1_exist)
			goto err;

		if (iio_path1 == NULL)
			goto err;

		dir = opendir(iio_path1);
		if (dir) {
			while ((entry = readdir(dir)) != NULL) {
				if(strstr(entry->d_name, "iio")) {
					str2_exist = true;
					asprintf(&iio_path2, "%s/%s", iio_path1, entry->d_name);
					break;
				}
			}
			closedir(dir);
			if (!str2_exist) {
				free(iio_path1);
				goto err;
			} else if (iio_path2 != NULL){
				strncpy(temp_path, iio_path2, strlen(iio_path2));
				free(iio_path1);
				free(iio_path2);
				return 0;
			}
		}
	}
err:
	LOGE(AUXADC_LOG_TAG"ap_auxadc open file(%s) failed str1_exist:%d str2_exist:%d!!\n",
		AP_AUXADC_IIO_PATH, str1_exist, str2_exist);
	temp_path = NULL;
	return -1;
}

static int auxadc_update_info(void *priv)
{
	int ret = 0;
	int i = 0;
	int tmp_fd = -1;
	int tmp_chn = 0;
	int tmp_data = 0;
	int tmp_vol = 0;
	int tmp_len = 0;
	struct st_auxadc *auxadc_data = (struct st_auxadc *)priv;
	char tmp_name[32];
	char tmp_buf[BUF_LEN] = {'\0'};
	char *pchar = tmp_buf;
	char *iio_full_path = NULL;
	char iio_buf[6] = {'\0'};
	int sample_data = 0;
	int error = 0;

	/* open fd, try legacy auxadc path */
	tmp_fd = open(AP_AUXADC_PATH, O_RDONLY);
	if (tmp_fd < 0) {
		/* try iio auxadc */
		ret = find_auxadc_iio_path(tmp_buf);
		if (ret == 0)
			goto iio;

		LOGE(AUXADC_LOG_TAG"ap_auxadc open file(%s) failed!!\n", AP_AUXADC_PATH);
		return -4;
	} else {
		memset(tmp_buf, 0x00, sizeof(tmp_buf));
		/* memset(auxadc_data->info, 0x00, sizeof(auxadc_data->info)); //maybe needn't */
	}

	ret = read(tmp_fd, tmp_buf, sizeof(tmp_buf));
	if (ret > pre_total) {
		/* LOGD(AUXADC_LOG_TAG"read file(%d) OK, readbytes = %d\n", tmp_fd, ret); */
		for(i = 0; i < MAX_CHN_NUM; i++) {
			ret = sscanf(pchar, "[%2d,%4d,%4d]-%16s\n", &tmp_chn, &tmp_data, &tmp_vol, tmp_name);
			if(ret > 0) {
				if(tmp_vol > -1) {
					tmp_len += snprintf(auxadc_data->info+tmp_len, sizeof(auxadc_data->info)-tmp_len,
						"chn[%02d]: %16s=>code:%5d; vol:%5d mv\n", tmp_chn, tmp_name,tmp_data, tmp_vol);
				} else {
					tmp_len += snprintf(auxadc_data->info+tmp_len, sizeof(auxadc_data->info)-tmp_len,
						"chn[%02d]: %16s=>code:%5d; vol:%5d mv-->error data\n", tmp_chn, tmp_name, tmp_data, tmp_vol);
				}
				/* LOGD(AUXADC_LOG_TAG"[%d]%s=>code=(%d)-->chn[%d]=%d mv\n", i, tmp_name,tmp_data, tmp_chn, tmp_vol); */
			} else {
				LOGE(AUXADC_LOG_TAG"Parse data-0 failed[%d]:chn[%d]-->ret:%d\n", i, tmp_chn, ret);
				return -3;
			}
			pchar += RECOG_STRING_LEN;/* next */
		}

		ret = sscanf(pchar, "-->REG:0x%4x,GAIN:%4u,GE_A:%4u,OE_A:%4u,GE:%4d,OE:%4d\n",
			&cali_reg, &gain, &cali_ge_a, &cali_oe_a, &cali_ge, &cali_oe);
		if(ret > 0) {
			tmp_len += snprintf(auxadc_data->info+tmp_len, sizeof(auxadc_data->info)-tmp_len,
				"[EFUSE_CALI]\nREG:0x%04X,GE_A:%4u,OE_A:%4u\nGAIN:%4u, GE:%4d,OE:%4d\n",
				cali_reg, cali_ge_a, cali_oe_a, gain, cali_ge, cali_oe);
		} else {
			LOGE(AUXADC_LOG_TAG"Parse data-1 failed[%d]:chn[%d]-->ret:%d\n", i, tmp_chn, ret);
			return -2;
		}
		/* pchar += RECOG_STRING_EFUSE_LEN; */
	} else {
		LOGE(AUXADC_LOG_TAG"read file(%d) failed,readbytes=%d < %d(total)\n", tmp_fd, ret, pre_total);
		return -1;
	}

	/* close fd */
	close(tmp_fd);

	return 0;

iio:
	for (i = 0; i < MAX_CHN_NUM; i++) {
		asprintf(&iio_full_path, "%s/%s", tmp_buf, auxadc_sample_data[i]);

		if (iio_full_path == NULL)
			continue;

		tmp_fd = open(iio_full_path, O_RDONLY);
		if (tmp_fd < 0) {
			LOGE(AUXADC_LOG_TAG"ap_auxadc open file(%s) failed!!\n", iio_full_path);
			error = -3;
			free(iio_full_path);
			goto err;
		} else {
			ret = read(tmp_fd, iio_buf, sizeof(iio_buf));
			/* close fd */
			close(tmp_fd);
			free(iio_full_path);
			if (ret > 0) {
				sample_data = atoi(iio_buf);
				tmp_len += snprintf(auxadc_data->info+tmp_len, sizeof(auxadc_data->info)-tmp_len,
					"chn[%02d]: %16s=>code:%5d; vol:%5d mv\n", i, auxadc_sample_data[i],sample_data, sample_data *1500/4096);
			} else {
				LOGE(AUXADC_LOG_TAG"read file(%d) failed,readbytes ret=%d \n", tmp_fd, ret);
				error = -1;
				goto err;
			}
		}
	}

err:
	return error;
}

 static void *auxadc_update_iv_thread(void *priv)
 {
	int ret = 0;
	struct st_auxadc *auxadc_data = (struct st_auxadc *)priv;
	struct itemview *iv = auxadc_data->iv;
	bool exit_flag = auxadc_data->thread_exit_flag;

	LOGD(AUXADC_LOG_TAG "%s: Start..\n", __FUNCTION__);
	if(!exit_flag) {
		/* while read data */
		 while (!exit_flag) {
			ret = auxadc_update_info((void*)priv);
			if(ret == 0) {
				exit_flag = auxadc_data->thread_exit_flag;
				iv->set_text(iv, &auxadc_data->text);
				iv->redraw(iv);/* redraw */
			} else if(ret == -1) {
				/* memset(auxadc_data->info, 0x00, sizeof(auxadc_data->info)); */
				sprintf(auxadc_data->info, "read file FAILED(%d), and stop..\n", ret);
				iv->set_text(iv, &auxadc_data->text);
				iv->redraw(iv);
				pthread_exit(NULL);
	 			return NULL;
			} else if(ret == -2 || ret == -3) {
				/* memset(auxadc_data->info, 0x00, sizeof(auxadc_data->info)); */
				sprintf(auxadc_data->info, "Parse file FAILED(%d), and stop..\n", ret);
				iv->set_text(iv, &auxadc_data->text);
				iv->redraw(iv);
				pthread_exit(NULL);
	 			return NULL;
			} else if(ret == -4) {
				memset(auxadc_data->info, 0x00, sizeof(auxadc_data->info));
				sprintf(auxadc_data->info, "open file FAILED(%d), and stop...\n", ret);
				iv->set_text(iv, &auxadc_data->text);
				iv->redraw(iv);/* redraw */
				pthread_exit(NULL);
	 			return NULL;
			} else {
				LOGE(AUXADC_LOG_TAG "unknown retrun(%d)\n", ret);
				pthread_exit(NULL);
	 			return NULL;
		 	}
			usleep(1500000);/* delay 1.5sec */
		 }
 	} else {
 		LOGE(AUXADC_LOG_TAG "Auto test, Just read data once!\n");
		ret = auxadc_update_info((void*)priv);/* need think~~ */
		iv->set_text(iv, &auxadc_data->text);
		iv->start_menu(iv, 0);/* for auto? */
		iv->redraw(iv);/* redraw */
 	}

	 LOGD(AUXADC_LOG_TAG "%s: Exit..\n", __FUNCTION__);
	 pthread_exit(NULL);
	 return NULL;
 }

int ap_auxadc_entry(struct ftm_param *param, void *priv)
{
	bool iv_exit = false;
	int len = 0;
	int chosen = 0;
	int auxadc_test_type = 0;
	struct itemview *iv;
	struct st_auxadc *auxadc_data= (struct st_auxadc *)priv;

	LOGD(AUXADC_LOG_TAG"%s: item[%d]=%s\n", __FUNCTION__, param->test_type, param->name);

	init_text(&auxadc_data->title, param->name, COLOR_YELLOW);
	init_text(&auxadc_data->text, &auxadc_data->info[0], COLOR_YELLOW);
#if 0
	 init_text(&auxadc_data->left_btn, uistr_key_fail, COLOR_YELLOW);
	 init_text(&auxadc_data->center_btn, uistr_key_pass, COLOR_YELLOW);
	 init_text(&auxadc_data->right_btn, uistr_key_back, COLOR_YELLOW);
#endif

	len = snprintf(auxadc_data->info, sizeof(auxadc_data->info), "%s\n", uistr_info_auxadc_chn);
	if (!auxadc_data->iv) {
		iv = ui_new_itemview();
		if (!iv) {
			LOGE(AUXADC_LOG_TAG"No memory");
			return -1;
		 }
		auxadc_data->iv = iv;
	} else {
		iv = auxadc_data->iv;
		LOGE(AUXADC_LOG_TAG"item have always exist!!");
	}

	iv->set_title(iv, &(auxadc_data->title));
	iv->set_items(iv, auxadc_items, 0);
	iv->set_text(iv, &(auxadc_data->text));
	iv->redraw(iv);

	auxadc_data->thread_exit_flag = false;
 	auxadc_test_type = param->test_type;
	if (auxadc_test_type== FTM_MANUAL_ITEM) {
		pthread_create(&auxadc_data->thread_id, NULL, auxadc_update_iv_thread, priv);

#ifdef FEATURE_FTM_TOUCH_MODE
		text_t lbtn;
		text_t cbtn;
		text_t rbtn;

		init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
		init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
		init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);

		iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif
		chosen = iv->run(iv, &iv_exit);
		switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
			case ITEM_PASS:
			case ITEM_FAIL:
				if (chosen == ITEM_PASS) {
					auxadc_data->mod->test_result = FTM_TEST_PASS;
				} else if (chosen == ITEM_FAIL) {
					auxadc_data->mod->test_result = FTM_TEST_FAIL;
				}
				/* iv_exit = true; */
				break;
#else
			case L_BTN_DOWN:
				auxadc_data->mod->test_result = FTM_TEST_FAIL;
				/* iv_exit = true; */
				break;
			case C_BTN_DOWN:
				/* iv_exit = true; */
				break;
			case R_BTN_DOWN:
				auxadc_data->mod->test_result = FTM_TEST_PASS;
				/* iv_exit = true; */
				break;
#endif
			default:
				LOGE(AUXADC_LOG_TAG"unknown event(%d) happen~\n", chosen);
				/* iv_exit = true; *//* exit too */
				break;
		}

		LOGE(AUXADC_LOG_TAG"preparing to exit..\n");
		auxadc_data->thread_exit_flag = true;
	} else if(auxadc_test_type== FTM_AUTO_ITEM){
		auxadc_data->thread_exit_flag = true;
		LOGE(AUXADC_LOG_TAG"auto testtype:%d\n", auxadc_test_type);
		auxadc_update_iv_thread((void *)priv);
	} else {
		LOGE(AUXADC_LOG_TAG"err: unknown the testtype:%d\n", auxadc_test_type);
	}

 	auxadc_data->thread_exit_flag = true;
	pthread_join(auxadc_data->thread_id, NULL);
	LOGE(AUXADC_LOG_TAG" ap_auxadc_entry: Exit~\n");

	return 0;
}

static void auxadc_data_init(struct st_auxadc * auxadc_data)
{
	auxadc_data->iv = NULL;

	auxadc_data->thread_id = 0;
	auxadc_data->thread_exit_flag = false;

	auxadc_data->fd = -1;
	memset(auxadc_data->info, 0x00, BUF_LEN);
}
 int ap_auxadc_init(void)
 {
	 int ret = 0;
	 struct ftm_module *mod;
	 struct st_auxadc *auxadc_data;

	 LOGE(AUXADC_LOG_TAG"%s\n", __FUNCTION__);

	 mod = ftm_alloc(ITEM_AP_AUXADC, sizeof(struct st_auxadc));
	 if (!mod)
		 return -ENOMEM;

	auxadc_data = mod_to_auxadc(mod);
	/* init */
	auxadc_data_init(auxadc_data);
	auxadc_data->mod = mod; /* save the header pointer */

	 ret = ftm_register(mod, ap_auxadc_entry, (void*)auxadc_data);

	 return ret;
 }


#endif/* end FEATURE_FTM_AP_AUXADC */
