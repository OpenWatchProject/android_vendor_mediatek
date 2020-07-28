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
#include <unistd.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include "cust.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include <cutils/properties.h>



#define TAG   "[Vibrator] "
static uint32_t vibrator_time = 0;
extern int status;
bool vibrator_thread_exit = false;
#ifdef FEATURE_FTM_VIBRATOR

#define VIBRATOR_ENABLE "/sys/class/timed_output/vibrator/enable"
/* Add for kernel-4.9 vibrator change path */
#define VIBRATOR_DURATION_NEW "/sys/class/leds/vibrator/duration"
#define VIBRATOR_ENABLE_NEW "/sys/class/leds/vibrator/activate"
char vib_path[128];
static bool vib_new_p;

extern int item_testing;
bool vibrator_test_exit = false;
static pthread_t vibrator_thread;

enum {
    ITEM_PASS,
    ITEM_FAIL
};

static item_t items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
#endif
    item(-1, NULL),
};

#ifdef VIBRATOR_AUTOTEST
static item_t items_auto[] = {
    {-1, NULL, 0, 0},
};
#endif

static int
write_int(char const* path, int value)
{
    int fd;

    if (path == NULL)
        return -1;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);
        close(fd);
        return amt == -1 ? -errno : 0;
    }

    LOGE("write_int failed to open %s\n", path);
    return -errno;
}

int judge_vib_path(void)
{
    int ret;

    ret = access(VIBRATOR_ENABLE, F_OK);
    if (ret < 0)
        LOGD("vibrator not has path:%s\n", VIBRATOR_ENABLE);
    else {
        strlcpy(vib_path, VIBRATOR_ENABLE, sizeof(vib_path));
        vib_new_p = false;
    }

    ret = access(VIBRATOR_DURATION_NEW, F_OK);
    if (ret < 0) {
        LOGD("vibrator not has path:%s\n", VIBRATOR_DURATION_NEW);
        return ret;
    }

    ret = access(VIBRATOR_ENABLE_NEW, F_OK);
    if (ret < 0)
        LOGD("vibrator not has path:%s\n", VIBRATOR_ENABLE_NEW);

    if (ret >= 0) {
        strlcpy(vib_path, VIBRATOR_DURATION_NEW, sizeof(vib_path));
        vib_new_p = true;
    }

    return ret;
}

static void *update_vibrator_thread_default(__attribute__((unused)) void *priv)
{
    int ret;
    LOGD("%s: Start\n", __FUNCTION__);

    ret = judge_vib_path();
    if (ret < 0)
        LOGE("no vibrator path\n");

    if(vibrator_time == 0)
    {
        do {
            write_int(vib_path, 8000);
            if (vib_new_p == true)
                write_int(VIBRATOR_ENABLE_NEW, 1);
            if (vibrator_test_exit)
                break;
            sleep(1);
        } while (1);
        if (vib_new_p == true)
            write_int(VIBRATOR_ENABLE_NEW, 0);
        else
            write_int(vib_path, 0);

    }
    else
    {
        LOGD("%s: write vibrator_enable=%d\n", __FUNCTION__, vibrator_time);
        write_int(vib_path, vibrator_time);
        if (vib_new_p == true)
            write_int(VIBRATOR_ENABLE_NEW, 1);
        sleep(1);
        if (vib_new_p == true)
            write_int(VIBRATOR_ENABLE_NEW, 0);
        else
            write_int(vib_path, 0);
        LOGD("%s: write vibrator_enable=0\n", __FUNCTION__);
    }

    pthread_exit(NULL);
    LOGD("%s: Exit\n", __FUNCTION__);
    return NULL;
}

static void *update_vibrator_thread(void *priv)
{
    return update_vibrator_thread_default(priv);
}

#ifdef VIBRATOR_AUTOTEST
int vibrator_autotest_entry(struct ftm_param *param, void *priv)
{
    struct ftm_module *mod = (struct ftm_module *)priv;

    LOGD("%s\n", __FUNCTION__);

    if (write_int(VIBRATOR_ENABLE, 2000) == 0)
        mod->test_result = FTM_TEST_PASS;
    else if (write_int(VIBRATOR_DURATION_NEW, 2000) == 0 &&
	write_int(VIBRATOR_ENABLE_NEW, 1) == 0)
        mod->test_result = FTM_TEST_PASS;
	else
        mod->test_result = FTM_TEST_FAIL;

    LOGD(TAG "%s: End\n", __FUNCTION__);
    return 0;
}
#endif

int vibrator_manual_entry(struct ftm_param *param, void *priv)
{
    int chosen;
    bool exit = false;
    struct itemview *iv;
    text_t    title;
    char* vibr_time = NULL;
    struct ftm_module *mod = (struct ftm_module *)priv;

    LOGD("%s\n", __FUNCTION__);

    vibrator_test_exit = false;

    iv = ui_new_itemview();
    if (!iv) {
        LOGD("No memory");
        return -1;
    }
        init_text(&title, param->name, COLOR_YELLOW);

    iv->set_title(iv, &title);
    iv->set_items(iv, items, 0);

    vibr_time = ftm_get_prop("Vibrator_Last_Time");
    LOGD("%s: get vibrator last time=%s!\n", __FUNCTION__, vibr_time);
    if(vibr_time != NULL)
    {
        vibrator_time = (uint32_t)atoi(vibr_time);
        LOGD("%s: get vibrator last time=%d!\n", __FUNCTION__, vibrator_time);
    }
    else
    {
        LOGD("%s: get vibrator last time fail!\n", __FUNCTION__);
    }
    pthread_create(&vibrator_thread, NULL, update_vibrator_thread, priv);

    iv->start_menu(iv, 0);
    iv->redraw(iv);
    do {
        if (param->test_type == FTM_MANUAL_ITEM) {
#ifdef FEATURE_FTM_TOUCH_MODE
            text_t lbtn;
            text_t cbtn;
            text_t rbtn;

            init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
            init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
            init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);

            iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif
            chosen = iv->run(iv, &exit);
            switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
            case ITEM_PASS:
            case ITEM_FAIL:
                if (chosen == ITEM_PASS) {
                    mod->test_result = FTM_TEST_PASS;
                } else if (chosen == ITEM_FAIL) {
                    mod->test_result = FTM_TEST_FAIL;
                }
                exit = true;
                break;
#else /* FEATURE_FTM_TOUCH_MODE */
            case L_BTN_DOWN:
                mod->test_result = FTM_TEST_FAIL;
                exit = true;
                break;
            case C_BTN_DOWN:
                exit = true;
                break;
            case R_BTN_DOWN:
                mod->test_result = FTM_TEST_PASS;
                exit = true;
                break;
#endif /* FEATURE_FTM_TOUCH_MODE */
            }
            if (exit) {
                vibrator_test_exit = true;
                break;
            }
        } else {
            if(vibrator_test_exit)
                break;
            sleep(1);
        }
    } while (1);

    return 0;
}

int vibrator_entry(struct ftm_param *param, __attribute__((unused)) void *priv)
{

#ifdef VIBRATOR_AUTOTEST
    char *bPhone = NULL;
    bPhone = ftm_get_prop("Vibrator.PhoneAutoTest");
    if ((bPhone != NULL) && (atoi(bPhone) == 1)) // Auto detect
    {
        vibrator_autotest_entry(param, priv);
        item_testing = 0; // for Auto test, no need to send AT+VIBRATOR=STOP
    }
    else
#endif
    {
        vibrator_manual_entry(param, priv);
    }

    return 0;
}


int vibrator_init(void)
{
    int ret = 0;
    struct ftm_module *mod;

    LOGD("%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_VIBRATOR, sizeof(struct ftm_module));
    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, vibrator_entry, (void*)mod);

    return ret;
}

#endif // FEATURE_FTM_VIBRATOR


