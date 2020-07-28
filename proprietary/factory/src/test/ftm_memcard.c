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
#include <errno.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cutils/properties.h>
#include "common.h"
#include "miniui.h"
#undef BLOCK_SIZE
#include "ftm.h"

#ifdef FEATURE_FTM_MEMCARD

#define TAG                 "[MCARD] "

/* should be moved to customized part */
#define MAX_NUM_SDCARDS     (3)
#define MIN_SDCARD_IDX      (0)
#define MAX_SDCARD_IDX      (MAX_NUM_SDCARDS + MIN_SDCARD_IDX - 1)

enum {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD)
    ITEM_FORMAT_EMMC_FAT,
#endif
    ITEM_PASS,
    ITEM_FAIL,
};

static item_t mcard_items[] = {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD)
    //item(ITEM_FORMAT_EMMC_FAT,    uistr_info_emmc_format_item),
#endif
    //item(ITEM_PASS,   uistr_pass),
    //item(ITEM_FAIL,   uistr_fail),
    item(-1, NULL),
};

static item_t mcard_items_manu[] = {
#if defined(MTK_EMMC_SUPPORT) && !defined(MTK_SHARED_SDCARD)
    //item(ITEM_FORMAT_EMMC_FAT,    uistr_info_emmc_format_item),
#endif
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
#endif
    item(-1, NULL),
};

int g_test_result = 0;

struct mcard {
    int          id;
    char         dev_path[512];
    char         sys_path[512];
    char         info[1024];
    char        *mntpnt;
    bool         mounted;
    bool         avail;
    int          blocknum;
    unsigned int checksum;
    const char  *format_stat;
};
struct mcard_array{
    struct mcard* mcard[1];
    char info[2048];
    char *ptr_step;
    int mcard_no;

    text_t title;
    text_t text;
    bool isFormatting;
    bool exit_thd;
    pthread_t update_thd;
    struct ftm_module *mod;
    struct itemview *iv;
};

#define mod_to_mcard_array(p)  (struct mcard_array*)((char*)(p) + sizeof(struct ftm_module))

#define FREEIF(p)   do { if(p) free(p); (p) = NULL; } while(0)

extern sp_ata_data return_data;

static bool mcard_avail(struct mcard *mc)
{
    char name[20];
    char *ptr;
    DIR *dp;
    struct dirent *dirp;

    if (mc->id < MIN_SDCARD_IDX || mc->id > MAX_SDCARD_IDX)
        return false;

    sprintf(name, "mmc%d", mc->id - MIN_SDCARD_IDX);

    ptr  = &mc->sys_path[0];
    ptr += sprintf(ptr, "/sys/class/mmc_host/%s", name);

    if (NULL == (dp = opendir(mc->sys_path)))
        goto error;

    while (NULL != (dirp = readdir(dp))) {
        if (strstr(dirp->d_name, name)) {
            ptr += sprintf(ptr, "/%s", dirp->d_name);
            break;
        }
    }

    closedir(dp);

    if (!dirp)
        goto error;

    return true;

error:
    return false;
}

/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)
static int get_boot_type(void) {
	int fd;
	ssize_t s;
	char boot_type[4] = {'0'};

	fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
	if (fd < 0) {
		LOGD(TAG "fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
		return -1;
	}

	s = read(fd, (void *)&boot_type, sizeof(boot_type) - 1);
	close(fd);

	if (s <= 0) {
		LOGD(TAG "could not read boot type sys file\n");
		return -1;
	}

	boot_type[s] = '\0';

	return atoi((char *)&boot_type);
}

char buf[512];
static void mcard_update_info(struct mcard *mc, char *info)
{
    char *ptr;
    int rc, fd;
    unsigned int nr_sec;
    unsigned int sd_total_size = 0;
    int mt_boot_type = get_boot_type();

    g_test_result = 0;
    return_data.memcard.sd1_total_size=0;
    return_data.memcard.sd1_free_size=0;

    mc->avail = mcard_avail(mc);
    mc->avail = 1;

    if(mt_boot_type == BOOTDEV_SDMMC) {
        if ((fd = open("/dev/block/mmcblk1", O_RDONLY, 0644)) < 0)
		mc->avail = 0;
    } else {
        if ((fd = open("/dev/block/mmcblk0", O_RDONLY, 0644)) < 0)
		mc->avail = 0;
    }

    if(mc->avail != 0) {
        rc = read(fd, buf, 512);
        if (rc <= 0) {
            mc->avail = 0;
            goto out;
        }

        if ((ioctl(fd, BLKGETSIZE, &nr_sec)) == -1) {
            LOGD("BLKGETSIZE fail \n");
        } else {
            sd_total_size = (nr_sec /2048);
        }
    }

out:
    /* prepare info */
    ptr  = info;

    ptr += sprintf(ptr, "%s: \n",uistr_info_sd);

    if (mc->avail)
        ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail,uistr_info_emmc_sd_yes);
    else
        ptr += sprintf(ptr, "%s: %s\n", uistr_info_emmc_sd_avail, uistr_info_emmc_sd_no);

    ptr += sprintf(ptr, "%s: %d MB\n",uistr_info_emmc_sd_total_size, sd_total_size);

    return_data.memcard.sd1_total_size=sd_total_size;
    return_data.memcard.sd1_free_size=0;

    if (mc->avail && sd_total_size > 0)
        g_test_result = 1;
    else
        g_test_result = 0;

    if (fd >=0)
        close(fd);

    return;
}


static void *mcard_update_iv_thread(void *priv)
{
    struct mcard_array *ma = (struct mcard_array *)priv;
    struct itemview *iv = ma->iv;
    int chkcnt = 10;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    while (1) {
        usleep(100000);
        chkcnt--;

        if (ma->exit_thd)
            break;

        if (chkcnt > 0)
            continue;

        if (ma->isFormatting)
            continue;

        chkcnt = 10;

        mcard_update_info(ma->mcard[0],ma->info);

        iv->redraw(iv);
        ma->exit_thd = true;
    }
    pthread_exit(NULL);

    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return NULL;
}

int mcard_entry(struct ftm_param *param, void *priv)
{
    int chosen;
    bool exit_val = false;
    struct mcard_array *ma = (struct mcard_array *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&ma->title, param->name, COLOR_YELLOW);
    init_text(&ma->text, &ma->info[0], COLOR_YELLOW);

    mcard_update_info(ma->mcard[0],ma->info);

    ma->isFormatting = false;
    ma->exit_thd = false;

    if (!ma->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        ma->iv = iv;
    }

    iv = ma->iv;
    iv->set_title(iv, &ma->title);
    if (FTM_MANUAL_ITEM == param->test_type) {
#ifdef FEATURE_FTM_TOUCH_MODE
        text_t lbtn ;
        text_t cbtn ;
        text_t rbtn ;
        init_text(&lbtn, uistr_key_fail, COLOR_YELLOW);
        init_text(&cbtn, uistr_key_back, COLOR_YELLOW);
        init_text(&rbtn, uistr_key_pass, COLOR_YELLOW);
        iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
#endif
        iv->set_items(iv, mcard_items_manu, 0);
    } else {
        iv->set_items(iv, mcard_items, 0);
    }
    iv->set_text(iv, &ma->text);
    iv->redraw(iv);

    if (FTM_MANUAL_ITEM == param->test_type) {
        do {
            chosen = iv->run(iv, &exit_val);
            switch (chosen) {
#ifndef FEATURE_FTM_TOUCH_MODE
            case ITEM_PASS:
            case ITEM_FAIL:
                if (chosen == ITEM_PASS)
                    ma->mod->test_result = FTM_TEST_PASS;
                else if (chosen == ITEM_FAIL)
                    ma->mod->test_result = FTM_TEST_FAIL;

                ma->mcard[0]->format_stat = "Default";

                exit_val = true;
                break;
#endif
#ifdef FEATURE_FTM_TOUCH_MODE
            case L_BTN_DOWN:
            case C_BTN_DOWN:
            case R_BTN_DOWN:
                if (chosen == L_BTN_DOWN)
                    ma->mod->test_result = FTM_TEST_FAIL;
                else if (chosen == R_BTN_DOWN)
                    ma->mod->test_result = FTM_TEST_PASS;
                ma->mcard[0]->format_stat = "Default";
                exit_val = true;
                break;
#endif
            }

            if (exit_val) {
                ma->exit_thd = true;
                break;
            }
        } while (1);

    } else if(FTM_AUTO_ITEM == param->test_type){
        pthread_create(&ma->update_thd, NULL, mcard_update_iv_thread, priv);
        pthread_join(ma->update_thd, NULL);
    }

    if (g_test_result > 0)
        ma->mod->test_result = FTM_TEST_PASS;
    else
        ma->mod->test_result = FTM_TEST_FAIL;

    return 0;
}

int mcard_init(void)
{
    int ret = 0;
    struct ftm_module *mod = NULL;
    struct mcard *mc = NULL;
    struct mcard_array *ma = NULL;
    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_MEMCARD, sizeof(struct mcard_array));
    if (!mod)
        return -ENOMEM;
    mc = (struct mcard*)malloc(sizeof(struct mcard));
    if(!mc){
        ftm_free(mod);
        return -ENOMEM;
    }
    /*
    ma = (struct mcard_array*)malloc(sizeof(struct mcard_array));
    if(!ma) {
        ftm_free(mod);
        free(mc);
        return -ENOMEM;
    }
    */

    ma  = mod_to_mcard_array(mod);
    memset(ma, 0, sizeof(struct mcard_array));
    ma->mod      = mod;
    mc->id       = 0;
    LOGD(TAG "mc->id=%d \n", mc->id);

    mc->mounted  = false;
    mc->avail    = false;
    mc->mntpnt   = NULL;
    mc->checksum = -1;
    mc->blocknum = -1;

    mc->format_stat = "Default";

    ma->mcard[0] = mc;
    ma->mcard_no += 1;

    LOGD(TAG "ma->mcard_no(%d)\n",ma->mcard_no);
    mcard_update_info(ma->mcard[0],ma->info);
    ret = ftm_register(mod, mcard_entry, (void*)ma);
    return ret;
}

#endif
