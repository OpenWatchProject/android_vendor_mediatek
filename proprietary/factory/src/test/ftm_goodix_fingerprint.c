/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include "common.h"
#include "ftm.h"
#include "errno.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define TAG "[GOODIX_FINGERPRINT]"

/* define the dummy API for Goodix fingerprint not supported projects.
use the real API just when FP is supported and tested.
for fingerprint not supported projects, common source code also needed 
to be built to avoid "init API can't be find" error */

#if defined(FEATURE_FTM_GOODIX_FINGERPRINT) && defined(MTK_GOODIX_FINGERPRINT_ENABLED)
extern int gf_factory_test_entry(void) ;

#else
int gf_factory_test_entry()
{
	LOGE(TAG " [%s] enter.  this is dummy", __func__);
        return 1;
}
#endif

/* define old solution here  as backup */
#ifdef MT6775
#define FINGERPRINT_FACTORY_OLD_SOLUTION
#endif

#ifdef FINGERPRINT_FACTORY_OLD_SOLUTION
#define GOODIXFP_PATH "/dev/goodix_fp"
typedef unsigned char  u8;
#define GF_IOC_MAGIC	'g'
#define GF_IOC_FTM		_IOR(GF_IOC_MAGIC, 20, u8)
#endif

#define mod_to_fingerprint(p)     (struct fingerprint*)((char*)(p) + sizeof(struct ftm_module))

struct fingerprint {
    struct ftm_module *mod;
    text_t title;
    struct itemview* iv;
};

static struct itemview *iv = NULL;

static item_t finger_item[] = {
    item(0, uistr_info_test_pass),
    item(1, uistr_info_test_fail),
    item(-1, NULL)
};

#ifdef FINGERPRINT_FACTORY_OLD_SOLUTION
static int fingerprint_test_old_solution(char* chipID)
{
	int fd = 0;
	int err = 0;
	char buf[12];

	LOGD(TAG " [%s] enter", __func__);

	fd = open(GOODIXFP_PATH, O_RDONLY);
	if(fd < 0)
	{
		LOGD(TAG " [%s] open fingerprint fail.", __func__);
		return -1;
	}

	err = ioctl(fd, GF_IOC_FTM, buf);
	close(fd);
	LOGD(TAG " [%s] enter,close fd", __func__);
	if(err)
	{
		LOGD(TAG " [%s] read fingerprint data fail: %s(%d)\n", __func__, strerror(errno), err);
		return -1;
	}

	LOGD(TAG " [%s] line:%d", __func__, __LINE__);
	//if (strncmp(CONFIG_MTK_FINGERPRINT_SELECT, "GF318M", 6) == 0 || strncmp(CONFIG_MTK_FINGERPRINT_SELECT, "GF316M", 6) == 0)
	{
		if (memcmp(buf, "GFx16M", 6) == 0)
		{
			LOGD(TAG "GFx16M is ok\n");
			memcpy(chipID, buf, 6);
			return 0;
		}
		//LOGE(TAG "[%s] oswego get errror firmware info. firmware info:%s \n", __func__, buf);
		//return -1;
	}

	LOGD(TAG " [%s] line:%d", __func__, __LINE__);
	//if (strncmp(CONFIG_MTK_FINGERPRINT_SELECT, "GF5216", 6) == 0)
	{
		memcpy(chipID, buf, 2);
		LOGD(TAG "[%s] ChipID is: 0x%s 0x%x 0x%x\n",__func__, chipID, chipID[0], chipID[1]);
		if (chipID[0] == 0x12 && (chipID[1] == 0xa1 || chipID[1] == 0xa4))
			return 0;
	}

	LOGE(TAG " [%s] %d error", __func__, __LINE__);

	return -1;
}

#endif

int goodix_fingerprint_entry(struct ftm_param *param, void *priv) {
    uint8_t is_pass = 0;

    LOGD(TAG " [%s] enter", __func__);
#ifdef FINGERPRINT_FACTORY_OLD_SOLUTION
	char chipIDBuf[12];
	if (0 == fingerprint_test_old_solution(chipIDBuf)) {
		is_pass = 1;
	}
#else
    if (0 == gf_factory_test_entry()){
        is_pass = 1;
    }
#endif

    struct fingerprint* fp = (struct fingerprint*) priv;

    init_text(&fp->title, param->name, COLOR_YELLOW);

    if (NULL == fp->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG " [%s] No memory for item view", __func__);
            return -1;
        }
        fp->iv = iv;
    }
    iv->set_title(iv, &fp->title);

    if (is_pass) {
        LOGI(TAG " [%s] test pass", __func__);
        fp->mod->test_result = FTM_TEST_PASS;
        iv->set_items(iv, finger_item, 0);
        iv->start_menu(iv, 0);
    } else {
        LOGI(TAG " [%s] test fail", __func__);
        fp->mod->test_result = FTM_TEST_FAIL;
        iv->set_items(iv, finger_item, 1);
        iv->start_menu(iv, 1);
    }

    iv->redraw(iv);
    usleep(500000);
    return 0;
}

int goodix_fingerprint_init(void) {
    int ret = 0;
    struct fingerprint *fp = NULL;
    struct ftm_module *mod = NULL;

    LOGE(TAG " [%s] enter", __func__);
    LOGD(TAG " [%s] enter", __func__);

    mod = ftm_alloc(ITEM_FINGERPRINT_TEST, sizeof(struct fingerprint));
    fp = mod_to_fingerprint(mod);
    memset(fp, 0x00, sizeof(*fp));
    if (!mod) {
        return -ENOMEM;
    }
    fp->mod = mod;
    ret = ftm_register(mod, goodix_fingerprint_entry, (void *) fp);

    LOGD(TAG " [%s] exit, ret = %d", __func__, ret);
    return ret;
}
