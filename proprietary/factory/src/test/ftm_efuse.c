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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <common.h>
#include <miniui.h>
#include <ftm.h>

#ifdef FEATURE_FTM_EFUSE

#define TAG	"[EFUSE] "
#define EFUSE_STATUS_IDX   49

struct efuse_factory {
	unsigned int blow_result;
	char	info[1024];

   /* for UI display */
	text_t	  title;
	text_t	  text;

	bool exit_thd;
	pthread_t update_thd;

	struct ftm_module *mod;
	struct itemview *iv;
};

enum {
	EFUSE_UNKNOWN = 0,
	EFUSE_REBLOW = 0x55,
	EFUSE_BROKEN = 0x5A,
	EFUSE_SUCCESS = 0xAA,
	EFUSE_STATUS = 0xFF,
};

enum {
	ITEM_PASS,
	ITEM_FAIL,
};

static item_t efuse_items_auto[] = {
	item(-1, NULL),
};

static item_t efuse_items_manual[] = {
	item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1, NULL),
};

#define mod_to_efuse(p)  (struct efuse_factory *)((char *)(p) + sizeof(struct ftm_module))

typedef struct {
	unsigned int entry_num;
	unsigned int data[100];
} DEVINFO_S;

static int efuse_update_info(struct efuse_factory *efuse, char *info)
{
	int fd = 0;
	int ret = 0;
	char *str = "";
	DEVINFO_S devinfo;

	fd = open("/sys/bus/platform/drivers/dev_info/dev_info", O_RDONLY); /* v1 device node */
	if (fd < 0) { /* Use v2 device node if v1 device node is removed */
		fd = open("/proc/device-tree/chosen/atag,devinfo", O_RDONLY); /* v2 device node */
		if (fd < 0) {
			info += sprintf(info, "Error, can't get result\n");
			return -1;
		}
	}

	ret = read(fd, (void *)&devinfo, sizeof(DEVINFO_S));
	if (ret < 0) {
		info += sprintf(info, "Error, can't get result\n");

		close(fd);
		return -1;
	}

	efuse->blow_result = devinfo.data[EFUSE_STATUS_IDX];

	if (efuse->blow_result == EFUSE_SUCCESS)
		str = uistr_info_efuse_success;
	else if (efuse->blow_result == EFUSE_BROKEN)
		str = uistr_info_efuse_broken;
	else if (efuse->blow_result == EFUSE_REBLOW)
		str = uistr_info_efuse_reblow;
	else
		str = uistr_info_efuse_unknown;

	info += sprintf(info, "%s : %s (%x)\n", uistr_info_efuse_result, str, efuse->blow_result);

	close(fd);

	return 0;
}


static void *efuse_update_iv_thread(void *priv)
{
	struct efuse_factory *efuse = (struct efuse_factory *)priv;
	struct itemview *iv = efuse->iv;

	LOGD(TAG "%s: Start\n", __func__);

	efuse_update_info(efuse, efuse->info);
	iv->start_menu(iv, 0);
	iv->redraw(iv);

	LOGD(TAG "%s: Exit\n", __func__);

	return NULL;
}


int efuse_entry(struct ftm_param *param, void *priv)
{
	int chosen;
	bool exit = false;

	struct efuse_factory *efuse = (struct efuse_factory *)priv;
	struct itemview *iv;

	init_text(&efuse->title, param->name, COLOR_YELLOW);
	init_text(&efuse->text, &efuse->info[0], COLOR_YELLOW);

	efuse_update_info(efuse, efuse->info);

	efuse->exit_thd = false;

	/* show text view */
	if (!efuse->iv) {
		iv = ui_new_itemview();
		if (!iv) {
			LOGD(TAG "No memory");
			return -1;
		}
		efuse->iv = iv;
	}
	iv = efuse->iv;
	iv->set_title(iv, &efuse->title);

	if (FTM_AUTO_ITEM == param->test_type)
		iv->set_items(iv, efuse_items_auto, 0);
	else
		iv->set_items(iv, efuse_items_manual, 0);

	iv->set_text(iv, &efuse->text);
	iv->start_menu(iv, 0);
	iv->redraw(iv);

	if (FTM_AUTO_ITEM == param->test_type) {
		efuse_update_iv_thread(priv);
	} else if (FTM_MANUAL_ITEM == param->test_type) {
		pthread_create(&efuse->update_thd, NULL, efuse_update_iv_thread, priv);
		do {
			chosen = iv->run(iv, &exit);

			switch (chosen) {
			case ITEM_PASS:
			case ITEM_FAIL:
				if (chosen == ITEM_PASS)
					efuse->mod->test_result = FTM_TEST_PASS;
				else if (chosen == ITEM_FAIL)
					efuse->mod->test_result = FTM_TEST_FAIL;

				exit = true;
			break;
			}

			if (exit) {
				efuse->exit_thd = true;
				break;
			}
		} while (1);
		pthread_join(efuse->update_thd, NULL);
	}
	return 0;
}

int efuse_init(void)
{
	int ret = 0;
	struct ftm_module *mod;
	struct efuse_factory *efuse;

	LOGD("%s\n", __func__);

	mod = ftm_alloc(ITEM_EFUSE, sizeof(struct efuse_factory));
	if (!mod)
		return -ENOMEM;

	efuse = mod_to_efuse(mod);
	efuse->mod = mod;

	efuse->blow_result = EFUSE_UNKNOWN;

	ret = ftm_register(mod, efuse_entry, (void *)efuse);

	if (ret)
		LOGD(TAG "register EFUSE failed (%d)\n", ret);

	return ret;
}

#endif /* FEATURE_FTM_EFUSE */
