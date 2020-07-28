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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_BT

#define TAG                 "[BT] "

enum {
  ITEM_PASS,
  ITEM_FAIL,
};

static item_t bf_items_man[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
  item(ITEM_PASS,   uistr_pass),
  item(ITEM_FAIL,   uistr_fail),
#endif
  item(-1, NULL),
};

static item_t bf_items_auto[] = {
  item(-1, NULL),
};

struct bt_factory {
  char  info[1024];
  bool  exit_thd;
  int   result;

  /* for UI display */
  text_t title;
  text_t text;

  pthread_t update_thd;
  struct ftm_module *mod;
  struct textview tv;
  struct itemview *iv;
};

enum {
  TEST_RESULT_UNKNOWN = 0,
  TEST_RESULT_PASS,
  TEST_RESULT_FAIL,
};

typedef struct {
  int test_type;
  int test_result;
} TEST_INFO_T;


#define mod_to_bf(p) (struct bt_factory*)((char*)(p) + sizeof(struct ftm_module))

#ifndef BOOL
#define BOOL     bool
#endif

TEST_INFO_T bf_test;
/* For background test */
extern char result[3][16];

//===================================================================

extern void updateTextInfo(TEST_INFO_T *pTest, char *output_buf, int buf_len);
extern BOOL FM_BT_init(void);
extern BOOL FM_BT_inquiry(void);
extern void FM_BT_deinit(void);


static void *bt_update_thread(void *priv)
{
    struct bt_factory *bf = (struct bt_factory *)priv;
    struct itemview *iv = bf->iv;
    char bt_result[32] = {0};

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    snprintf(bf->info, sizeof(bf->info), "%s", uistr_info_bt_init);
    iv->redraw(iv);

    if (FM_BT_init() == false) {
        LOGD(TAG "%s: Exit\n", __FUNCTION__);
        snprintf(bf->info, sizeof(bf->info), "%s", uistr_info_bt_init_fail);
        iv->redraw(iv);
        pthread_exit(NULL);
        goto exit;
    }

    if (bf->exit_thd)
        goto exit;

    snprintf(bf->info, sizeof(bf->info), "%s", uistr_info_bt_init_ok);
    iv->redraw(iv);

    FM_BT_inquiry();

    while (1) {
        if (bf->exit_thd)
            break;

        updateTextInfo(&bf_test, bf->info, sizeof(bf->info));
        iv->redraw(iv);

        if (FTM_MANUAL_ITEM != bf_test.test_type) {
            if (bf_test.test_result != TEST_RESULT_UNKNOWN)
                break;
        }

        usleep(200000);
    }

exit:
    FM_BT_deinit();

    if (bf_test.test_result == TEST_RESULT_PASS)
        bf->mod->test_result = FTM_TEST_PASS;
    else
        bf->mod->test_result = FTM_TEST_FAIL;

    if (FTM_MANUAL_ITEM != bf_test.test_type &&
        FTM_AUTO_ITEM != bf_test.test_type) {
        /* Auto test background mode */
        sprintf(bt_result, "%d:%s", bf->mod->id, result[bf->mod->test_result]);
        write_data_to_pc(bt_result, strlen(bt_result));
    }

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    return NULL;
}

#if 0
static int bt_key_handler(int key, void *priv)
{
    int handled = 0, exit = 0;
    struct bt_factory *bf = (struct bt_factory *)priv;
    struct textview *tv = &bf->tv;
    struct ftm_module *fm = bf->mod;

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
      default:
        handled = -1;
        break;
    }
    if (exit) {
        LOGD(TAG "%s: Exit thead\n", __FUNCTION__);
        bf->exit_thd = true;
        tv->exit(tv);
    }
    return handled;
}
#endif

int bt_entry(struct ftm_param *param, void *priv)
{
    int    chosen;
    bool   exit = false;
    struct bt_factory *bf = (struct bt_factory *)priv;
    struct textview *tv;
    struct itemview *iv;
#ifdef FEATURE_FTM_TOUCH_MODE
    text_t lbtn;
    text_t cbtn;
    text_t rbtn;
#endif

    LOGD(TAG "%s\n", __FUNCTION__);

    /* Initialize test info struct */
    bf_test.test_type = param->test_type;
    bf_test.test_result = TEST_RESULT_UNKNOWN;

    /* Initialize item view */
    memset(&bf->info[0], 0, sizeof(bf->info));
    memset(&bf->info[0], '\n', 10);

    init_text(&bf->title, param->name, COLOR_YELLOW);
    init_text(&bf->text, &bf->info[0], COLOR_YELLOW);

    if (!bf->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        bf->iv = iv;
    }

    iv = bf->iv;
    iv->set_title(iv, &bf->title);
    if (FTM_MANUAL_ITEM == bf_test.test_type) {
        iv->set_items(iv, bf_items_man, 0);
#ifdef FEATURE_FTM_TOUCH_MODE
        init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
        init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
        init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);
        iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif

    } else {
        iv->set_items(iv, bf_items_auto, 0);
    }
    iv->set_text(iv, &bf->text);
    iv->start_menu(iv,0);
    iv->redraw(iv);

    /* Initialize thread condition */
    bf->exit_thd = false;
    bf->result = false;
    pthread_create(&bf->update_thd, NULL, bt_update_thread, priv);

    if (FTM_MANUAL_ITEM == bf_test.test_type ||
        FTM_AUTO_ITEM == bf_test.test_type) {
        if (FTM_MANUAL_ITEM == bf_test.test_type) {
            do {
                chosen = iv->run(iv, &exit);
                switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
                case ITEM_PASS:
                    bf_test.test_result = TEST_RESULT_PASS;
                    exit = true;
                    break;
                case ITEM_FAIL:
                    bf_test.test_result = TEST_RESULT_FAIL;
                    exit = true;
                    break;
#else /* FEATURE_FTM_TOUCH_MODE */
                case L_BTN_DOWN:
                    bf_test.test_result = TEST_RESULT_FAIL;
                    exit = true;
                    break;
                case C_BTN_DOWN:
                    exit = true;
                    break;
                case R_BTN_DOWN:
                    bf_test.test_result = TEST_RESULT_PASS;
                    exit = true;
                    break;
#endif /* FEATURE_FTM_TOUCH_MODE */
                default :
                    break;
                }

                if (exit) {
                    bf->exit_thd = true;
                    break;
                }
            } while (1);
        }

        pthread_join(bf->update_thd, NULL);
    }

    return 0;
}

int bluetooth_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct bt_factory *bf;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_BT, sizeof(struct bt_factory));
    if (!mod)
        return -ENOMEM;

    bf = mod_to_bf(mod);
    bf->mod  = mod;

    ret = ftm_register(mod, bt_entry, (void*)bf);

    return ret;
}
#endif
