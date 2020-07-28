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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
 TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define TAG "[FTM_STROBE] "

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/statfs.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include "flashlight.h"

#define STROBE_DEVICE_FILE        "/dev/flashlight"
#define STROBE_DEVICE_FILE_LEGACY "/dev/kd_camera_flashlight"


/**********************************************************************
 * Strobe Basic Functions
 *********************************************************************/
enum {
    STROBE_TYPE_MAX = 10,
    STROBE_CT_MAX = 10,
};

int strobe_status[STROBE_TYPE_MAX][STROBE_CT_MAX];

int strobe_ioctl(int stb_fd, int cmd, int type_id, int ct_id, int arg)
{
    struct flashlight_user_arg stb_arg;

    stb_arg.type_id = type_id + 1;
    stb_arg.ct_id = ct_id + 1;
    stb_arg.arg = arg;

    return ioctl(stb_fd, cmd, &stb_arg);
}

int strobe_ioctl_r(int stb_fd, int cmd, int type_id, int ct_id, int *arg)
{
    struct flashlight_user_arg stb_arg;
    int ret;

    stb_arg.type_id = type_id + 1;
    stb_arg.ct_id = ct_id + 1;
    stb_arg.arg = 0;

    ret = ioctl(stb_fd, cmd, &stb_arg);
    *arg = stb_arg.arg;

    return ret;
}

int strobe_set_level(int stb_fd, int type_id, int ct_id, int level)
{
    LOGD(TAG "Strobe level (%d,%d) = (%d)\n", type_id, ct_id, level);

    if (strobe_ioctl(stb_fd, FLASH_IOC_SET_DUTY, type_id, ct_id, level)) {
        LOGD(TAG "Failed to set level.\n");
        return -1;
    }

    return 0;
}

int strobe_set_level_all(int stb_fd, int level)
{
    int i, j;
    int ret = 0;

    for (i = 0; i < STROBE_TYPE_MAX; i++)
        for (j = 0; j < STROBE_CT_MAX; j++)
            if (strobe_status[i][j])
                if (strobe_set_level(stb_fd, i, j, level))
                    ret = -1;

    return ret;
}

int strobe_enable(int stb_fd, int type_id, int ct_id, int enable)
{
    LOGD(TAG "Strobe enable (%d,%d) = (%d)\n", type_id, ct_id, enable);

    if (strobe_ioctl(stb_fd, FLASH_IOC_SET_ONOFF, type_id, ct_id, enable)) {
        LOGD(TAG "Failed to enable.\n");
        return -1;
    }

    return 0;
}

int strobe_enable_all(int stb_fd, int enable)
{
    int i, j;
    int ret = 0;

    for (i = 0; i < STROBE_TYPE_MAX; i++)
        for (j = 0; j < STROBE_CT_MAX; j++)
            if (strobe_status[i][j])
                if (strobe_enable(stb_fd, i, j, enable))
                    ret = -1;

    return ret;
}

int strobe_open()
{
    int stb_fd;
    int i, j;

    /* open flashlight driver */
    LOGD(TAG "Strobe open.\n");
    stb_fd = open(STROBE_DEVICE_FILE, O_RDWR);
    if (stb_fd < 0) {
        ALOGD("Failed to open \"%s\".\n", STROBE_DEVICE_FILE);

        /* try to open legacy driver */
        stb_fd = open(STROBE_DEVICE_FILE_LEGACY, O_RDWR);
        if (stb_fd < 0) {
            ALOGD("Failed to open \"%s\".\n", STROBE_DEVICE_FILE_LEGACY);
            return -1;
        }
    }

    /* set flashlight driver */
    LOGD(TAG "Strobe set driver.\n");
    for (i = 0; i < STROBE_TYPE_MAX; i++)
        for (j = 0; j < STROBE_CT_MAX; j++)
            if (!strobe_ioctl(stb_fd, FLASHLIGHTIOC_X_SET_DRIVER, i, j, 1))
                strobe_status[i][j] = 1;

    return stb_fd;
}

int strobe_close(int stb_fd)
{
    int i, j;

    /* disable flashlight */
    LOGD(TAG "Strobe close.\n");
    strobe_enable_all(stb_fd, 0);

    /* clear flashlight driver */
    for (i = 0; i < STROBE_TYPE_MAX; i++)
        for (j = 0; j < STROBE_CT_MAX; j++)
            if (strobe_status[i][j]) {
                strobe_ioctl(stb_fd, FLASHLIGHTIOC_X_SET_DRIVER, i, j, 0);
                strobe_status[i][j] = 0;
            }

    /* close flashlight driver */
    close(stb_fd);

    return 0;
}


/**********************************************************************
 * Strobe Test
 *********************************************************************/
enum {
    ITEM_NULL,
    ITEM_PASS,
    ITEM_FAIL,
    ITEM_STROBE_TEST,
};

struct strobe {
    char info[1024];
    int exit;
    int cmd;
    int is_test;
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};

#define mod_to_strobe(p) (struct strobe *)((char *)(p) + sizeof(struct ftm_module))

static item_t menu_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS, uistr_pass),
    item(ITEM_FAIL, uistr_fail),
#endif
    item(-1, NULL),
};

bool strobe_test_exit = false;
static int strobe_test_type = 0;

static void *strobe_test_thread(void *priv)
{
    struct strobe *stb = (struct strobe *)priv;
    int stb_fd;

    LOGD(TAG "Strobe test.\n");

    /* open strobe and set level */
    stb_fd = strobe_open();
    if (stb_fd < 0) {
        LOGD(TAG "Failed to open device file.\n");
        return NULL;
    }
    strobe_set_level_all(stb_fd, 0);

    /* enable strobe */
    while (!stb->exit) {
        if (stb->cmd == ITEM_STROBE_TEST) {
            stb->is_test = 1;

            if (get_is_ata() == 1 && strobe_test_type == FTM_AUTO_ITEM) {
                /* if modem is operating */
                strobe_enable_all(stb_fd, 1);
                usleep(100000);
            } else {
                strobe_enable_all(stb_fd, 1);
                usleep(30000);
                strobe_enable_all(stb_fd, 0);
                usleep(100000);
            }

        }
        usleep(30000);
    }

    /* close strobe */
    strobe_close(stb_fd);

    return NULL;
}

int strobe_entry(struct ftm_param *param, void *priv)
{
    struct strobe *stb = (struct strobe *)priv;
    struct itemview *iv = NULL;
    int chosen;
    bool exit = false;
    static int is_test = 0;

    LOGD(TAG "Strobe entry.\n");

    init_text(&stb->title, param->name, COLOR_YELLOW);
    init_text(&stb->text, &stb->info[0], COLOR_YELLOW);

    if (!stb->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGE(TAG "No memory");
            return -1;
        }
        stb->iv = iv;
    }
    iv = stb->iv;

    iv->set_title(iv, &stb->title);
    iv->set_items(iv, menu_items, 0);
    iv->set_text(iv, &stb->text);

#ifdef FEATURE_FTM_TOUCH_MODE
    text_t lbtn;
    text_t cbtn;
    text_t rbtn;
    init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
    init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
    init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
    iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif

    strobe_test_type = param->test_type;

    /* create strobe test thread */
    pthread_t strobe_thread;
    pthread_create(&strobe_thread, NULL, strobe_test_thread, stb);

    stb->cmd = ITEM_NULL;
    stb->exit = 0;
    stb->is_test = is_test;

    if (!is_test) {
        stb->cmd = ITEM_STROBE_TEST;
        is_test = 1;
    }

    /* modem is operating */
    if (get_is_ata() == 1 && strobe_test_type == FTM_AUTO_ITEM) {
        strobe_test_exit = false;
        while (!strobe_test_exit) {
            LOGD(TAG "Modem is operating. Sleep.\n");
            sleep(1);
        }
        stb->mod->test_result = FTM_TEST_PASS;
        exit = true;
    }

    /* modem is not operating */
    while (!exit) {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
        case ITEM_PASS:
            stb->cmd = ITEM_NULL;
            if (stb->is_test) {
                stb->mod->test_result = FTM_TEST_PASS;
                exit = true;
            } else {
                memset(stb->info, 0, 1024);
                sprintf(stb->info, "Not test done!\n");
                iv->set_text(iv, &stb->text);
                iv->redraw(iv);
            }
            break;

        case ITEM_FAIL:
            stb->cmd = ITEM_NULL;
            if (stb->is_test) {
                stb->mod->test_result = FTM_TEST_FAIL;
                exit = true;
            } else {
                memset(stb->info, 0, 1024);
                sprintf(stb->info, "Not test done!\n");
                iv->set_text(iv, &stb->text);
                iv->redraw(iv);
            }
            break;
#endif
#ifdef FEATURE_FTM_TOUCH_MODE
        case L_BTN_DOWN:
            stb->cmd = ITEM_NULL;
            if (stb->is_test) {
                stb->mod->test_result = FTM_TEST_FAIL;
                exit = true;
            } else {
                memset(stb->info, 0, 1024);
                sprintf(stb->info, "Not test done!\n");
                iv->set_text(iv, &stb->text);
                iv->redraw(iv);
            }
            break;

        case C_BTN_DOWN:
            stb->cmd = ITEM_NULL;
            exit = true;
            break;

        case R_BTN_DOWN:
            stb->cmd = ITEM_NULL;
            if (stb->is_test) {
                stb->mod->test_result = FTM_TEST_PASS;
                exit = true;
            } else {
                memset(stb->info, 0, 1024);
                sprintf(stb->info, "Not test done!\n");
                iv->set_text(iv, &stb->text);
                iv->redraw(iv);
            }
            break;
#endif
        }
        usleep(30000);
    }

    if (exit) {
        is_test = stb->is_test;
        stb->cmd = ITEM_NULL;
        stb->exit = 1;
        is_test = 0;
    }

    /* join strobe test thread */
    LOGD(TAG "Wait for joining thread.\n");
    pthread_join(strobe_thread, NULL);
    LOGD(TAG "Thread has been joint.\n");

    return 0;
}

int strobe_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct strobe *stb;

    LOGD(TAG "Strobe init.\n");

    mod = ftm_alloc(ITEM_STROBE, sizeof(struct strobe));
    if (!mod)
        return -ENOMEM;

    stb = mod_to_strobe(mod);
    stb->mod = mod;
    ret = ftm_register(mod, strobe_entry, (void *)stb);

    return ret;
}

