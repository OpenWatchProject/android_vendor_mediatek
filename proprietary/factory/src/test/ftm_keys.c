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
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <common.h>
#include <miniui.h>
#include <ftm.h>
#include <linux/kpd.h>
#include <utils.h>
#include <graphics.h>
#ifdef FEATURE_FTM_KEYS

#define TAG	"[KEYS] "

// #define KEYS_NUM_ROWS		((CUST_LCD_AVAIL_HEIGHT - 2 * CHAR_HEIGHT) / CHAR_HEIGHT) 
#define KEYS_NUM_COLS_NEW	1
#define KEYS_NUM_ROWS		((gr_fb_height() - 2 * CHAR_HEIGHT) / CHAR_HEIGHT)
#define KEYS_KEY_PER_PAGE	(KEYS_NUM_COLS_NEW * KEYS_NUM_ROWS)
#define KEYS_COL_WIDTH_NEW	(gr_fb_width() / KEYS_NUM_COLS_NEW)
#define KEYS_COL_LEN_NEW	(KEYS_COL_WIDTH_NEW / CHAR_WIDTH)
#define	KEYPAD_NAME "/dev/mtk-kpd"

static int kpd_fd = 0;

#ifdef CUST_KEY_CONFIRM
#ifdef MTK_WEARABLE_PLATFORM
#define CUST_KEY_CONFIRM KEY_ANGLE	//Some project would use homekey, so assign another used as confirm key on wearable.  
#else
#undef CUST_KEY_CONFIRM
#define CUST_KEY_CONFIRM KEY_HOME
#endif	//MTK_WEARABLE_PLATFORM
#endif

enum {
	ITEM_PASS,
	ITEM_FAIL
};

/*static item_t keys_item[] = {
	item(ITEM_PASS,   "Test Pass"),
	item(ITEM_FAIL,   "Test Fail"),
	item(-1, NULL),
};*/

struct keys {
	struct ftm_module *mod;

	int pstart;
	int pnum;
	int ptested;
	int untested;

	int cfm_i;
	int cfm_cnt;
	
	pthread_t update_thd;//new added yucong
	
	text_t title;
	struct textview tv;
	//struct itemview *iv;
};

#define mod_to_keys(p)	(struct keys *)((char *)(p) + sizeof(struct ftm_module))
extern int get_confirm_key(void);
extern struct key keys_keymap[10];
extern int keys_num;
extern ctext_t keys_ctext[10];
extern void ftm_get_key_map(void);

/***********************************************************/
static int kpd_open()
{
	kpd_fd = -1;
    if(kpd_fd < 0)
	{
		LOGD(TAG "ftm open kpd device!\n");
		kpd_fd = open(KEYPAD_NAME, O_RDONLY);
	}
	
	if(kpd_fd < 0)
	{
		LOGD(TAG "Couldn't open kpd device!\n");
		return -EINVAL;
	}

	return 0;	
}
/***********************************************************/

static int read_keyscan_type(void)
{
    int type = 0;
    char *ptype = NULL;
    char uName[64];

    memset(uName,0,sizeof(uName));
    sprintf(uName,"FtmKeyType");
    ptype = ftm_get_prop(uName);
    if (ptype != NULL){
        type = (int)atoi(ptype);
        ALOGD("read_keyscan_type: %d\n",type);
    }
    else{
        ALOGD("read_keyscan_type can't get\n");
    }
    return type;
}

static int keys_key_handler(int key, void *priv)
{
	int i, pend;
    char buffer[64];
	struct keys *keys = (struct keys *)priv;
	struct textview *tv = &keys->tv;
	
	if (key == BTN_TOUCH){
		return 0;
	}
	if ((key == get_confirm_key() && 
	    keys_ctext[keys->cfm_i].color == COLOR_BLACK)||key == CUST_KEY_CONFIRM) {
		keys->cfm_cnt++;
		if (keys->cfm_cnt >= 3)	{	/* force to exit */
			keys->cfm_cnt = 0;
			keys->mod->test_result = FTM_TEST_PASS;
			tv->exit(tv);
		}
	} else {
		keys->cfm_cnt = 0;
	}

    //return keycode ro ATA tool
	if((get_is_ata() != 0) && (read_keyscan_type() != 0))
	{	
		memset(buffer,0,sizeof(buffer));
		sprintf(buffer, "%d:%d\r\n", ITEM_KEYS, key);
		write_data_to_pc(buffer, strlen(buffer));
	}

	pend = keys->pstart + keys->pnum;
	for (i = keys->pstart; i < pend; i++) {
		if (key == keys_keymap[i].code) {
			if (keys_ctext[i].color == COLOR_BLACK)	/* tested */
				break;

			keys_ctext[i].color = COLOR_BLACK;
			tv->set_ctext(tv, &keys_ctext[keys->pstart], keys->pnum);
			keys->ptested++;
			keys->untested--;
			break;
		}
	}

	if (keys->untested == 0) {
		keys->mod->test_result = FTM_TEST_PASS;
		tv->exit(tv);
		return 0;
	}

	if (keys->ptested == keys->pnum) {	/* change to next page */
		keys->pstart += keys->pnum;
		keys->pnum = keys->untested;
		if (keys->pnum > KEYS_KEY_PER_PAGE)
			keys->pnum = KEYS_KEY_PER_PAGE;
		tv->set_ctext(tv, &keys_ctext[keys->pstart], keys->pnum);
		keys->ptested = 0;
	}

	return 0;
}


/***********************************************************/
static void *kpd_update_iv_thread(__attribute__((unused)) void *priv)
{
    int err = 0;
	static char buf[128];

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    if ((err = kpd_open())) {
        pthread_exit(NULL);
        return NULL;
    }
    sleep(2);
    err = ioctl(kpd_fd, SET_KPD_KCOL, buf);
	if(err != 0){
		LOGD(TAG "ioctl function err! err value = %d!\n",err);
	}

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    return NULL;
}

/***********************************************************/

static int keys_entry(struct ftm_param *param, void *priv)
{
	int i, err, num = 0;
	int x = 0, y = CHAR_HEIGHT;
	int fd;
	char *mrdump_rst_cmd = "1 0 0";
	char *mrdump_irq_cmd = "1 1 0";
	struct keys *keys = (struct keys *)priv;
	struct textview *tv = &keys->tv;
	//struct itemview *iv = keys->iv;

	err = 0;
	err = kpd_open();
	if(err != 0){
		LOGD(TAG "Couldn't open kpd device!\n");
	}

	/* Disable function in /proc/mrdump_rst */
	fd = open("/proc/mrdump_rst", O_WRONLY);
	if (fd < 0)
		LOGD(TAG "Couldn't open mrdump_rst. error=%d", errno);
	else {
		err = write(fd, mrdump_rst_cmd, strlen(mrdump_rst_cmd));
		if (err < 0)
			LOGD(TAG "Couldn't write mrdump_rst. error=%d", errno);

		err = write(fd, mrdump_irq_cmd, strlen(mrdump_irq_cmd));
		if (err < 0)
			LOGD(TAG "Couldn't write mrdump_rst. error=%d", errno);
	}

	keys->pstart = 0;
	keys->pnum = (keys_num <= KEYS_KEY_PER_PAGE ? keys_num
	                                                 : KEYS_KEY_PER_PAGE);
	keys->ptested = 0;
	keys->untested = keys_num;
	init_text(&keys->title, param->name, COLOR_YELLOW);
	for (i = 0; i < keys_num; i++) {
		if (keys_keymap[i].code == CUST_KEY_CONFIRM || keys_keymap[i].code == get_confirm_key()) {
			keys->cfm_i = i;
			keys->cfm_cnt = 0;
		}

		init_ctext(&keys_ctext[i], keys_keymap[i].name,
		           COLOR_YELLOW, x, y);
		num++;

		x += KEYS_COL_WIDTH_NEW;
		if (!(num % KEYS_NUM_COLS_NEW)) {
			x = 0;
			y += CHAR_HEIGHT;
			if (num == KEYS_KEY_PER_PAGE) {
				/* next page */
				y = CHAR_HEIGHT;
				num = 0;
			}
		}
	}

	ui_init_textview(tv, keys_key_handler, (void*)keys);
	tv->set_title(tv, &keys->title);
	tv->set_ctext(tv, keys_ctext, keys->pnum);

	/*******************auto test part***********************/
	if(get_is_ata() && (read_keyscan_type() == 0))
	    pthread_create(&keys->update_thd, NULL, kpd_update_iv_thread, priv);
	/*****************************************************/
	tv->run(tv);

	/* finish testing */
	/*iv->set_title(iv, &keys->title);
	iv->set_items(iv, keys_item, ITEM_PASS);

	while (1) {
		num = iv->run(iv, &exit);
		if (num == ITEM_PASS) {
			keys->mod->test_result = FTM_TEST_PASS;
			exit = true;
		} else if (num == ITEM_FAIL) {
			keys->mod->test_result = FTM_TEST_FAIL;
			exit = true;
		}

		if (exit)
			break;
	}*/

	if (fd >= 0)
		close(fd);

	return 0;
}

int keys_init(void)
{
	int r;
	struct ftm_module *mod;
	struct keys *keys;
	ftm_get_key_map();

	mod = ftm_alloc(ITEM_KEYS, sizeof(struct keys));
	if (!mod)
		return -ENOMEM;

	keys = mod_to_keys(mod);
	keys->mod = mod;
	/*keys->iv = ui_new_itemview();
	if (!keys->iv)
		return -ENOMEM;*/

	r = ftm_register(mod, keys_entry, (void*)keys);
	if (r) {
		LOGD(TAG "register KEYS failed (%d)\n", r);
		return r;
	}

	return 0;
}

#endif
