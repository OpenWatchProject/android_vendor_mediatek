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

// ftm_cmmb.cpp - Factory Test Module of CMMB
//

//#ifdef FEATURE_FTM_CMMB
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include "ftm_cmmb_api.h"

//#define FTM_CMMB_AUTO_TEST

#define TAG		"[CMMB] "

// maximum valid CMMB channel count
#define MAX_CMMB_CHANNEL	10

#define MAX_TITLE_LEN				40
#define MAX_TEXT_LEN				600
#define MAX_SRV_LEN				10

void FormatSignalInfo(char* info, int val1, int val2, int val3);

struct itemview *record_iv=NULL;
char szBuf[100];
static char szText[MAX_TEXT_LEN];            // 

typedef struct
{
	int	channel_id;
	int	frequency;
} CMMB_CHANNEL_INFO;

CMMB_CHANNEL_INFO channel_info[MAX_CMMB_CHANNEL];
int	channel_count = 0;
int	channel_selected = 0;

enum {
    ITEM_PASS,
    ITEM_FAIL,
    ITEM_AUTO_SCAN,
    ITEM_CHNN_LIST,
};

static item_t cmmb_main_items[] = {
	item(ITEM_PASS,   "Test Pass"),
	item(ITEM_FAIL,   "Test Fail"),
	item(ITEM_AUTO_SCAN,   uistr_info_cmmb_autoscan),
	item(ITEM_CHNN_LIST,   uistr_info_cmmb_channellist),
	item(-1, NULL),
};

typedef enum
{
	CMMB_NONE = 0,
	CMMB_IDLE,				// after init
	CMMB_SCAN_OK,	// after auto-scan
	CMMB_READY,			// after Tune
	CMMB_PLAYING,		// after StartService
	CMMB_ERROR,
} CMMB_STATE;

typedef enum
{
	CMD_NONE,
	CMD_INIT,
	CMD_AUTO_SCAN,
	CMD_TUNE,
	CMD_START_SRV,
    CMD_UPDATE_SIGNAL,
} CMMB_CMD;

typedef struct
{
	char	info[1024];
	bool		exit_thd;

	text_t title;
	text_t text;

	CMMB_STATE	cmmb_state;
	CMMB_CMD		cmmb_cmd;
	int						cmd_param;
	bool					cmd_done;

	pthread_t update_thd;
	struct ftm_module *mod;
	struct itemview *iv;
} cmmb_t;

#define mod_to_cmmb(p)  (cmmb_t*)((char*)(p) + sizeof(struct ftm_module))

static void update_iv_info(cmmb_t *cmmb, const char *str)
{
	strcpy(cmmb->info, str);
	cmmb->iv->text_col = 0;
	cmmb->iv->text_row = 0;
	cmmb->iv->text_top = 0;
	cmmb->iv->redraw(cmmb->iv);
}

cmmb_t *g_cmmb = NULL;

int AutoScanProc(int channel, int freq)
{
	LOGD(TAG "auto-scan, get a channel: %d, frequency: %d\n", channel, freq);

	if (channel_count < MAX_CMMB_CHANNEL)
	{
		int i = channel_count;
		channel_info[i].channel_id = channel;
		channel_info[i].frequency = freq;
		channel_count++;
	}

	if (g_cmmb != NULL
			&& g_cmmb->exit_thd)
	{
		return 0;	// break scanning
	}

	return 1;	// continue
}

static void *cmmb_update_iv_thread(void *priv)
{
	cmmb_t *cmmb = (cmmb_t *)priv;
	struct itemview *iv = cmmb->iv;
	CmmbResult errCode;

	LOGD(TAG "%s: Start\n", __FUNCTION__);

	while (1) {
		if (cmmb->exit_thd)
			break;

		usleep(100000);

		if (cmmb->exit_thd)
			break;

		if (cmmb->cmd_done)
			continue;

		switch (cmmb->cmmb_cmd)
		{
			case CMD_INIT:
				errCode = CmmbFtInit();

			#ifndef FTM_CMMB_AUTO_TEST

				if (errCode == CMMB_S_OK)
				{
					update_iv_info(cmmb, uistr_info_cmmb_init_ok);
					cmmb->cmmb_state = CMMB_IDLE;
				}
				else
					update_iv_info(cmmb, uistr_info_cmmb_init_fail);
					
				cmmb->cmd_done = true;
				break;


			#else
				if (errCode != CMMB_S_OK)
				{	
					update_iv_info(cmmb, uistr_info_cmmb_init_fail);
					cmmb->mod->test_result = FTM_TEST_FAIL;
					cmmb->cmd_done = true;
					cmmb->exit_thd = true;
					usleep(1000000);
					break;
				}
				
			
				channel_count = 0;
				update_iv_info(cmmb, uistr_info_cmmb_scanning);
				errCode =CmmbFtAutoScan(AutoScanProc);
			
				if (cmmb->exit_thd)
					break;
				
				if (errCode != CMMB_S_OK)
				{
					update_iv_info(cmmb, uistr_info_cmmb_scan_fail);
					cmmb->mod->test_result = FTM_TEST_FAIL;
					cmmb->cmd_done = true;
					cmmb->exit_thd = true;
					usleep(1000000);
					break;					
				}			

				cmmb->mod->test_result = FTM_TEST_PASS;
				update_iv_info(cmmb, uistr_pass);
				cmmb->cmd_done = true;
				cmmb->exit_thd = true;
				usleep(1000000);
			
				break;
			#endif	

			case CMD_AUTO_SCAN:
				channel_count = 0;
				g_cmmb = cmmb;
				update_iv_info(cmmb, uistr_info_cmmb_scanning);
				errCode =CmmbFtAutoScan(AutoScanProc);

				if (cmmb->exit_thd)
					break;

				// update channels information
				if (errCode == CMMB_S_OK)
				{
					update_iv_info(cmmb, uistr_info_cmmb_scan_ok);
					cmmb->cmmb_state = CMMB_SCAN_OK;
				}
				else
					update_iv_info(cmmb, uistr_info_cmmb_scan_fail);

				cmmb->cmd_done = true;
				break;

			case CMD_UPDATE_SIGNAL:
				{
					char szBuf_1[120];
					CmmbProps Props;
					errCode = CmmbFtGetProp(&Props);
					// initialize title and text
					FormatSignalInfo(szText, Props.SNR,Props.PRE_BER,Props.RSSI);
					sprintf(szBuf_1, "%s,fileSize:%d KB \n",szBuf,GetMfsSize());
					strcat(szText, szBuf_1);
					usleep(1000*1000);
					if(record_iv)
						record_iv->redraw(record_iv);
             
                                        break;
				}
			default:
				break;
		}
	}

	pthread_exit(NULL);

	LOGD(TAG "%s: Exit\n", __FUNCTION__);
	return NULL;
}

#define MAX_SRV_COUNT 40

int srv_list[MAX_SRV_COUNT];
int srv_count = 0;

int ServiceProc(int serviceId)
{
	LOGD(TAG "list-service, service ID: %d\n", serviceId);
	srv_list[srv_count] = serviceId;
	srv_count++;
	return 1;	// continue
}


static item_t list_srvs_items[MAX_SRV_COUNT];
static char srv_names[MAX_SRV_COUNT][MAX_SRV_LEN];

enum {
	ITEM_STOP,
};

static item_t record_srv_items[] = {
	item(ITEM_STOP,   uistr_info_cmmb_stop),
	item(-1, NULL),
};

void FormatSignalInfo(char* info, int val1, int val2, int val3)
{
	char *ptr;

	/* prepare info */
	ptr  = info;
	ptr += sprintf(ptr, "SNR: %d \n", val1);
	ptr += sprintf(ptr, "PRE_BER: %d \n", val2);
	ptr += sprintf(ptr, "RSSI: %d \n", val3);
}


int StartRecordService(int srvId,void *priv)
{
	static char szTitle[MAX_TITLE_LEN];
	cmmb_t *cmmb = (cmmb_t *)priv;
	LOGD(TAG "cmmb=0x%x",(int)cmmb);

	LOGD(TAG "%s\n", __FUNCTION__);

	record_iv = ui_new_itemview();

	if (!record_iv)
	{
		LOGD(TAG "No memory");
		return -1;
	}

	static int index = 1;

	text_t title;
	text_t text;
	char szFile[80];
	sprintf(szFile, "/data/%03d_ch%d_stm%d.mfs", index++, channel_info[channel_selected].channel_id, srvId);

	if (CmmbFtStartService(srvId, szFile) != CMMB_S_OK)
	{
		LOGD(TAG "CmmbFtStartService failed!");
		return -1;
	}
/*
	int propVal1 = 0;
	CmmbFtGetProp(CMMB_PROP_SNR, &propVal1);

	int propVal2 = 0;
	CmmbFtGetProp(CMMB_PROP_PRE_BER, &propVal2);

	int propVal3 = 0;
	CmmbFtGetProp(CMMB_PROP_RSSI, &propVal3);
*/
	// initialize title and text
	strcpy(szTitle, uistr_info_cmmb_recording);
	sprintf(szBuf, "%s%s \n",uistr_info_cmmb_recording_to, szFile);
	CmmbProps Props;
	CmmbFtGetProp(&Props);
	// initialize title and text
	FormatSignalInfo(szText, Props.SNR,Props.PRE_BER,Props.RSSI);
//	FormatSignalInfo(szText, propVal1, propVal2, propVal3);
	strcat(szText, szBuf);

	init_text(&title, szTitle, COLOR_YELLOW);
	init_text(&text, szText, COLOR_YELLOW);

	record_iv->set_title(record_iv, &title);
	record_iv->set_text(record_iv, &text);
	record_iv->set_items(record_iv, record_srv_items, 0);

	bool exit = false;
	bool recording = true;
	int chosen;

	if (cmmb->cmmb_state == CMMB_SCAN_OK
			&& cmmb->cmd_done == true)
	{
		LOGD(TAG "set cmb update signal\n");
		cmmb->cmmb_cmd = CMD_UPDATE_SIGNAL;
		cmmb->cmd_done = false;
	}

	do {
		chosen = record_iv->run(record_iv, &exit);

		if (chosen == ITEM_STOP
				&& recording)
		{
			CmmbFtStopService();
			sprintf(szBuf, "%s%s \n", uistr_info_cmmb_stop_to, szFile);
			FormatSignalInfo(szText, Props.SNR,Props.PRE_BER,Props.RSSI);
			strcat(szText, szBuf);
			recording = false;
		}

		if (exit) {
			cmmb->cmd_done = true;
			break;
		}
	} while (1);

	if (recording)
		CmmbFtStopService();

	ui_free_itemview(&record_iv);
        record_iv=NULL;
	return 0;
}

int ShowServiceListView(void *priv)
{
	static char szTitle[MAX_TITLE_LEN];
	static char szText[MAX_TEXT_LEN];
	struct cmmb_t *cmmb = (struct cmmb_t *)priv;
	struct itemview *iv;
	LOGD(TAG "%s\n", __FUNCTION__);

	iv = ui_new_itemview();

	if (!iv)
	{
		LOGD(TAG "No memory");
		return -1;
	}

	text_t title;
	text_t text;

	// initialize title and text
	strcpy(szTitle, uistr_info_cmmb_servicelist);
	strcpy(szText, uistr_info_cmmb_selectstream);

	if (CmmbFtSetChannel(channel_info[channel_selected].channel_id) != CMMB_S_OK)
	{
		LOGD(TAG "CmmbFtSetChannel failed!");
		return -1;
	}

	srv_count = 0;
	CmmbFtListServices(ServiceProc);
	LOGD(TAG "total %d services", srv_count);

	// initialize channel content
	int i = 0;
	for (; i < srv_count; i++)
	{
		list_srvs_items[i].id = i;
		sprintf(srv_names[i], "%d", srv_list[i]);
		list_srvs_items[i].name = srv_names[i];
	}

	list_srvs_items[i].id = -1;
	list_srvs_items[i].name = NULL;

	init_text(&title, szTitle, COLOR_YELLOW);
	init_text(&text, szText, COLOR_YELLOW);

	iv->set_title(iv, &title);
	iv->set_text(iv, &text);
	iv->set_items(iv, list_srvs_items, 0);

	bool exit = false;
	int chosen;

	do {
		chosen = iv->run(iv, &exit);

		if (chosen >=0 && chosen < MAX_SRV_COUNT)
		{
			StartRecordService(srv_list[chosen],priv);
		}

		if (exit) {
			break;
		}
	} while (1);

	ui_free_itemview(&iv);
	return 0;
}

#define MAX_CHANNEL_LEN		12

static item_t list_channels_items[MAX_CMMB_CHANNEL];
char channel_names[MAX_CMMB_CHANNEL][MAX_CHANNEL_LEN];

int ShowChannelListView(void *priv)
{
	static char szTitle[MAX_TITLE_LEN];
	static char szText[MAX_TEXT_LEN];
	struct cmmb_t *cmmb = (struct cmmb_t *)priv;
	struct itemview *iv;
	LOGD(TAG "%s\n", __FUNCTION__);

	iv = ui_new_itemview();

	if (!iv)
	{
		LOGD(TAG "No memory");
		return -1;
	}

	text_t title;
	text_t text;

	// initialize title and text
	strcpy(szTitle, uistr_info_cmmb_channellist);
	strcpy(szText, uistr_info_cmmb_tune_channel);

	// initialize channel content
	int i = 0;
	for (; i < channel_count; i++)
	{
		list_channels_items[i].id = i;
		sprintf(channel_names[i], "Channel %d", channel_info[i].channel_id);
		list_channels_items[i].name = channel_names[i];
	}

	list_channels_items[i].id = -1;
	list_channels_items[i].name = NULL;

	init_text(&title, szTitle, COLOR_YELLOW);
	init_text(&text, szText, COLOR_YELLOW);

	iv->set_title(iv, &title);
	iv->set_text(iv, &text);
	iv->set_items(iv, list_channels_items, 0);

	bool exit = false;
	int chosen;

	do {
		chosen = iv->run(iv, &exit);

		if (chosen < channel_count)
		{
			channel_selected = chosen;
			ShowServiceListView(priv);
		}

		if (exit) {
			break;
		}
	} while (1);

	ui_free_itemview(&iv);
	return 0;
}

int cmmb_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;
	cmmb_t *cmmb = (cmmb_t *)priv;
	struct itemview *iv;

	LOGD(TAG "%s\n", __FUNCTION__);

	init_text(&cmmb->title, param->name, COLOR_YELLOW);             //?//xingyu param->name from where?
	init_text(&cmmb->text, &cmmb->info[0], COLOR_YELLOW);

	cmmb->exit_thd = false;

	if (!cmmb->iv)
	{
		cmmb->iv = ui_new_itemview();

		if (!cmmb->iv)
		{
			LOGD(TAG "No memory");
			return -1;
		}
	}

	iv = cmmb->iv;
	iv->set_title(iv, &cmmb->title);
	iv->set_items(iv, cmmb_main_items, 0);
	iv->set_text(iv, &cmmb->text);

	cmmb->cmmb_state = CMMB_NONE;
	cmmb->cmmb_cmd = CMD_INIT;
	cmmb->cmd_done = false;

	pthread_create(&cmmb->update_thd, NULL, cmmb_update_iv_thread, priv);
#ifndef FTM_CMMB_AUTO_TEST
	do {
		LOGD(TAG "cmmb_entry, do\n");

		chosen = iv->run(iv, &exit);
		switch (chosen) {
			case ITEM_PASS:
			case ITEM_FAIL:
				if (chosen == ITEM_PASS) {
					cmmb->mod->test_result = FTM_TEST_PASS;
				}
				else if (chosen == ITEM_FAIL) {
					cmmb->mod->test_result = FTM_TEST_FAIL;
				}
				exit = true;
				break;

			case ITEM_AUTO_SCAN:
				if (cmmb->cmmb_state == CMMB_IDLE
						&& cmmb->cmd_done == true)
				{
					cmmb->cmmb_cmd = CMD_AUTO_SCAN;
					cmmb->cmd_done = false;
				}
				break;

			case ITEM_CHNN_LIST:
				if (cmmb->cmmb_state == CMMB_SCAN_OK)
					ShowChannelListView(priv);
				break;
		}
		if (exit) {
			cmmb->exit_thd = true;
			break;
		}	

	} while (1);
#endif

	pthread_join(cmmb->update_thd, NULL);
#ifndef FTM_CMMB_AUTO_TEST
	if (cmmb->iv)
		ui_free_itemview(&cmmb->iv);
#endif

	return 0;
}

int cmmb_init()
{
	int ret = 0;
	struct ftm_module *mod;
	cmmb_t *cmmb;

	LOGD(TAG "%s\n", __FUNCTION__);

	mod = ftm_alloc(ITEM_CMMB, sizeof(cmmb_t));           //?//xingyu ITEM_CMMB no defined
	cmmb  = mod_to_cmmb(mod);

	if (!mod)
		return -ENOMEM;

	cmmb->mod	= mod;

	LOGD(TAG "cmmb=0x%x",(int)cmmb);
	ret = ftm_register(mod, cmmb_entry, (void*)cmmb);

	return ret;
}

//#endif // FEATURE_FTM_CMMB
