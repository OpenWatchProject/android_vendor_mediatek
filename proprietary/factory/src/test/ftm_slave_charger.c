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
#include <sys/ioctl.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_SLAVE_CHARGER

#define TAG "[CHG2] "

#define REGMAP_PATH "/d/rt-regmap/rt9465/slave_addr"

static item_t slave_chg_items[] = {
#if 1
    /* auto test */
    item(-1, NULL),
#else
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
#endif
};

extern sp_ata_data return_data;

struct slave_chgFTM {
    char   info[1024];
    bool   exit_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;

    text_t title;
    text_t text;
    text_t left_btn;
    text_t center_btn;
    text_t right_btn;
};

#define mod_to_slave_chgFTM(p)     (struct slave_chgFTM*)((char*)(p) + sizeof(struct ftm_module))

/*
static bool is_slave_chg_exist2(void)
{
    int fd = -1;
    int ret = 0;
    char buf[10] = {0};

    fd = open(REGMAP_PATH, O_RDONLY, 0);
    if (fd == -1) {
        LOGD(TAG "Can't open %s, ret=%d\n", REGMAP_PATH, errno);
        return false;
    }

    ret = read(fd, buf, sizeof(buf));
    if (ret < 0) {
        LOGD(TAG "read fail, ret=%d\n", errno);
        close(fd);
        return false;
    }
    close(fd);

    LOGD(TAG "[%s] result=%s\n", __func__, buf);

    return true;
}
*/

/* IOCTL */
#define CH_SLAVE_CHARGER             _IOW('k', 13, int)

static bool is_slave_chg_exist(void)
{
    int fd = -1;
    int ret = 0;
    int in_data = 0;

    fd = open("/dev/charger_ftm", O_RDONLY, 0);
    if (fd == -1) {
        LOGD(TAG "Can't open /dev/charger_ftm, ret=%d\n", errno);
        return false;
    }

    ret = ioctl(fd, CH_SLAVE_CHARGER, &in_data);
    if (ret == -1) {
        LOGD(TAG "ioctl fail, ret=%d\n", errno);
        close(fd);
        return false;
    }
    close(fd);

    LOGD(TAG "[%s] result=%d\n", __func__, in_data);

    if (in_data == 0)
        return false;
    return true;
}

static void slave_chg_update_info(char *info)
{
    char *ptr;

    /* prepare text view info */
    ptr = info;

    ptr += sprintf(ptr, "%s : %s\n", uistr_info_title_slave_charger,
        (is_slave_chg_exist()) ? uistr_info_title_slave_charger_connect
        : uistr_info_title_slave_charger_no_connect);

    return;
}

static int slave_chg_entry(struct ftm_param *param, void *priv)
{
    struct slave_chgFTM *slave_chg = (struct slave_chgFTM *)priv;
    /* struct textview *tv; */
    struct itemview *iv;
    /* auto test */

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&slave_chg->title, param->name, COLOR_YELLOW);
    init_text(&slave_chg->text, &slave_chg->info[0], COLOR_YELLOW);
    init_text(&slave_chg->left_btn, "Fail", COLOR_YELLOW);
    init_text(&slave_chg->center_btn, "Pass", COLOR_YELLOW);
    init_text(&slave_chg->right_btn, "Back", COLOR_YELLOW);

    slave_chg_update_info(slave_chg->info);

    /* show text view */
    slave_chg->exit_thd = false;

    if (!slave_chg->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        slave_chg->iv = iv;
    }

    iv = slave_chg->iv;
    iv->set_title(iv, &slave_chg->title);
    iv->set_items(iv, slave_chg_items, 0);
    iv->set_text(iv, &slave_chg->text);
    iv->start_menu(iv,0);

    iv->redraw(iv);

    /* auto test - check slave charger */
    if (is_slave_chg_exist()) {
        LOGD(TAG "is_slave_chg_exist : YES\n");
        slave_chg->mod->test_result = FTM_TEST_PASS;
        LOGD(TAG "Final : PASS\n");
    } else {
        LOGD(TAG "is_slave_chg_exist : NO\n");
        slave_chg->mod->test_result = FTM_TEST_FAIL;
    }

    return 0;
}

int slave_chg_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct slave_chgFTM *slave_chg;

    LOGD(TAG "%s\n", __func__);

    mod = ftm_alloc(ITEM_SLAVE_CHARGER, sizeof(struct slave_chgFTM));
    slave_chg = mod_to_slave_chgFTM(mod);

    /* init */
    slave_chg->mod = mod;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, slave_chg_entry, (void*)slave_chg);

    return ret;
}

#endif
