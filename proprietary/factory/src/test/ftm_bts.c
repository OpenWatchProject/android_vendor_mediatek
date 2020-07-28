/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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
#include <dirent.h>
#include <linux/input.h>
#include <math.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#ifdef FEATURE_FTM_BTS

// TODO: remove the following definitions to custom header file
#define BTS_TEMP_LOWER_BOUND        (5000)
#define BTS_TEMP_UPPER_BOUND        (50000)
#define BTS_PUP_R                   (39000)
#define BTS_PUP_VOLT                (1800)
#define BTS_OVER_CRITICAL_L         (68237)
#define BTS_NTC_TABLE               (4)
#define BTS_AUX_ADC_CHANNEL         (1)

extern sp_ata_data return_data;

/******************************************************************************
 * MACRO
 *****************************************************************************/
#define TAG "[BTS]"
#define mod_to_bts_data(p) (struct bts_data*)((char*)(p) + sizeof(struct ftm_module))
#define FLPLOGD(fmt, arg ...) LOGD(TAG fmt, ##arg)
#define FLPLOGE(fmt, arg ...) LOGE(TAG "%s [%5d]: " fmt, __func__, __LINE__, ##arg)
/******************************************************************************
 * Structure
 *****************************************************************************/
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
/*---------------------------------------------------------------------------*/
static item_t bts_items[] = {
#ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_pass),
    item(ITEM_FAIL,   uistr_fail),
#endif
    item(-1, NULL),
};
/*---------------------------------------------------------------------------*/
struct bts_priv
{
    // TODO: define what are needed here
    /*specific data field*/
    char            *dev;
    int             fd;
    char            tz0[32];
    char            tz1[32];
    char            tz2[32];
    char            tz3[32];
    char            tz4[32];
    char            tz5[32];
    char            tz6[32];
    char            tz7[32];
    char            tz8[32];
    char            tz9[32];
    char            tz10[32];
    char            tz11[32];
    char            tz12[32];
    char            tz13[32];
    char            tz14[32];
    char            tz15[32];
    char            tz16[32];
    char            tz17[32];
    char            tz18[32];
    char            tz19[32];
    char            tz20[32];
    char            tz21[32];
    char            tz22[32];
    char            tz23[32];

    int             tz0_temp;
    int             tz1_temp;
    int             tz2_temp;
    int             tz3_temp;
    int             tz4_temp;
    int             tz5_temp;
    int             tz6_temp;
    int             tz7_temp;
    int             tz8_temp;
    int             tz9_temp;
    int             tz10_temp;
    int             tz11_temp;
    int             tz12_temp;
    int             tz13_temp;
    int             tz14_temp;
    int             tz15_temp;
    int             tz16_temp;
    int             tz17_temp;
    int             tz18_temp;
    int             tz19_temp;
    int             tz20_temp;
    int             tz21_temp;
    int             tz22_temp;
    int             tz23_temp;
};
/*---------------------------------------------------------------------------*/
struct bts_data
{
    struct bts_priv     bts; // TODO:

    /*common for each factory mode*/
    char                info[1024];
    //bool              avail;
    bool                exit_thd;

    text_t              title;
    text_t              text;
    text_t              left_btn;
    text_t              center_btn;
    text_t              right_btn;

    pthread_t           update_thd;
    struct ftm_module   *mod;
    //struct textview   tv;
    struct itemview     *iv;
};

static bool thread_exit = false;
bool bts_test_exit = false;

/******************************************************************************
 * Functions
 *****************************************************************************/
#if 0
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
#endif

static int
read_int(char const* path)
{
    int fd;
    int ret = -1;

    if (path == NULL)
    	return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
    	char buffer[20];
    	int len = read(fd, buffer, 19);
		if (len < 0) {
            close(fd);
            return -1;
		}

    	if (sscanf(buffer, "%d", &ret))
    	{
    	    close(fd);
    	    // success
    	    return ret;
    	}
    	close(fd);
    	return -1;
    }
    return -errno;
}

static int
read_str(char const* path, char *buf, int size)
{
    int fd;

    if (path == NULL)
    	return -1;

    if (buf == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
		char buffer[32] = {0};

		if (size > 32)
			size = 32;

    	int len = read(fd, buffer, size-1);
		if (len < 0) {
            close(fd);
            return -1;
		}
        buffer[size-1] = '\0';
    	//FLPLOGD("%d [%s] [%s] \n", (int) buf, buf, buffer);

        sprintf(buf, "%s", buffer);

    	//read(fd, *buf, size);

    	//FLPLOGD("%d [%s] [%s] \n", (int) buf, buf, buffer);

    	close(fd);
    	return -1;
    }
    else
    {
        sprintf(buf, "not exist\n");
    }
    return -errno;
}

static int bts_init_priv(struct bts_priv *bts)
{
    memset(bts, 0x00, sizeof(*bts));
    bts->fd = -1;
    bts->dev = "/proc/mtktsAP/mtkts_AP_param";
    return 0;
}
/*---------------------------------------------------------------------------*/
static int bts_open(struct bts_priv *bts)
{
    (void) bts;
/* unnecessary anymore
    int err = 0, max_retry = 3, retry_period = 100, retry = 0;
    unsigned int flags = 1;
    if (bts->fd == -1) {
        bts->fd = open(bts->dev, O_RDWR); // r/w
        if (bts->fd < 0) {
            FLPLOGE("Couldn't open '%s' (%s)", bts->dev, strerror(errno));
            return -1;
        }
        retry = 0;

        {
            char buffer[128];
            int bytes = sprintf(buffer, "PUP_R %d PUP_VOLT %d OVER_CRITICAL_L %d NTC_TABLE %d %d\n",
                                BTS_PUP_R, BTS_PUP_VOLT, BTS_OVER_CRITICAL_L, BTS_NTC_TABLE, BTS_AUX_ADC_CHANNEL);
            int amt = write(bts->fd, buffer, bytes);
            close(bts->fd);

            {
                int fd = open("/proc/mtktsAP/mtktsAP", O_RDWR);
                if (fd >= 0)
                {
                    write(fd, "1 120000 0 mtk-cl-kshutdown00 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 0 0 no 1000", 104);
                    close(fd);
                }
            }
            return amt == -1 ? -errno : 0;
        }

    }
*/
    // already opened and set, do nothing...
    //FLPLOGD("%s() %d\n", __func__, bts->fd);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int bts_close(struct bts_priv *bts)
{
    //unsigned int flags = 0;
    //int err = 0;
    if (bts->fd != -1) {
        bts->fd = -1;
#if 0
        if ((err = ioctl(lps->fd, ALSPS_SET_PS_MODE, &flags))) {
            FLPLOGE("disable ps fail: %s", strerror(errno));
            return -1;
        } else if ((err = ioctl(lps->fd, ALSPS_SET_ALS_MODE, &flags))) {
            FLPLOGE("disable als fail: %s", strerror(errno));
            return -1;
        }
        close(lps->fd);
#endif
    }
    bts_init_priv(bts);
    return 0;
}
/*---------------------------------------------------------------------------*/
static int bts_update_info(struct bts_priv *bts)
{

    bts->tz0_temp = read_int("/sys/class/thermal/thermal_zone0/temp");
    bts->tz1_temp = read_int("/sys/class/thermal/thermal_zone1/temp");
    bts->tz2_temp = read_int("/sys/class/thermal/thermal_zone2/temp");
    bts->tz3_temp = read_int("/sys/class/thermal/thermal_zone3/temp");
    bts->tz4_temp = read_int("/sys/class/thermal/thermal_zone4/temp");
    bts->tz5_temp = read_int("/sys/class/thermal/thermal_zone5/temp");
    bts->tz6_temp = read_int("/sys/class/thermal/thermal_zone6/temp");
    bts->tz7_temp = read_int("/sys/class/thermal/thermal_zone7/temp");
    bts->tz8_temp = read_int("/sys/class/thermal/thermal_zone8/temp");
    bts->tz9_temp = read_int("/sys/class/thermal/thermal_zone9/temp");
    bts->tz10_temp = read_int("/sys/class/thermal/thermal_zone10/temp");
    bts->tz11_temp = read_int("/sys/class/thermal/thermal_zone11/temp");
    bts->tz12_temp = read_int("/sys/class/thermal/thermal_zone12/temp");
    bts->tz13_temp = read_int("/sys/class/thermal/thermal_zone13/temp");
    bts->tz14_temp = read_int("/sys/class/thermal/thermal_zone14/temp");
    bts->tz15_temp = read_int("/sys/class/thermal/thermal_zone15/temp");
    bts->tz16_temp = read_int("/sys/class/thermal/thermal_zone16/temp");
    bts->tz17_temp = read_int("/sys/class/thermal/thermal_zone17/temp");
    bts->tz18_temp = read_int("/sys/class/thermal/thermal_zone18/temp");
    bts->tz19_temp = read_int("/sys/class/thermal/thermal_zone19/temp");
    bts->tz20_temp = read_int("/sys/class/thermal/thermal_zone20/temp");
    bts->tz21_temp = read_int("/sys/class/thermal/thermal_zone21/temp");
    bts->tz22_temp = read_int("/sys/class/thermal/thermal_zone22/temp");
    bts->tz23_temp = read_int("/sys/class/thermal/thermal_zone23/temp");

    return 0;
}

int findKeyIndex(struct bts_priv *bts, char *key)
{
	int len = strlen(key);
	LOGD(TAG "s: %s, l: %d\n", key, len);

	if(strncmp(bts->tz0, key, len) == 0)
		return 0;
	if(strncmp(bts->tz1, key, len) == 0)
		return 1;
	if(strncmp(bts->tz2, key, len) == 0)
		return 2;
	if(strncmp(bts->tz3, key, len) == 0)
		return 3;
	if(strncmp(bts->tz4, key, len) == 0)
		return 4;
	if(strncmp(bts->tz5, key, len) == 0)
		return 5;
	if(strncmp(bts->tz6, key, len) == 0)
		return 6;
	if(strncmp(bts->tz7, key, len) == 0)
		return 7;
	if(strncmp(bts->tz8, key, len) == 0)
		return 8;
	if(strncmp(bts->tz9, key, len) == 0)
		return 9;
	if(strncmp(bts->tz10, key, len) == 0)
		return 10;
	if(strncmp(bts->tz11, key, len) == 0)
		return 11;
	if(strncmp(bts->tz12, key, len) == 0)
		return 12;
	if(strncmp(bts->tz13, key, len) == 0)
		return 13;
	if(strncmp(bts->tz14, key, len) == 0)
		return 14;
	if(strncmp(bts->tz15, key, len) == 0)
		return 15;
	if(strncmp(bts->tz16, key, len) == 0)
		return 15;
	if(strncmp(bts->tz16, key, len) == 0)
		return 16;
	if(strncmp(bts->tz17, key, len) == 0)
		return 17;
	if(strncmp(bts->tz18, key, len) == 0)
		return 18;
	if(strncmp(bts->tz19, key, len) == 0)
		return 19;
	if(strncmp(bts->tz20, key, len) == 0)
		return 20;
	if(strncmp(bts->tz21, key, len) == 0)
		return 21;
	if(strncmp(bts->tz22, key, len) == 0)
		return 22;
	if(strncmp(bts->tz23, key, len) == 0)
		return 23;

	return -1;
}

float getIndexTemp(struct bts_priv *bts, int index)
{
	switch(index){
		case 0:
			return bts->tz0_temp / 1000;
			break;
		case 1:
			return bts->tz1_temp / 1000;
			break;
		case 2:
			return bts->tz2_temp / 1000;
			break;
		case 3:
			return bts->tz3_temp / 1000;
			break;
		case 4:
			return bts->tz4_temp / 1000;
			break;
		case 5:
			return bts->tz5_temp / 1000;
			break;
		case 6:
			return bts->tz6_temp / 1000;
			break;
		case 7:
			return bts->tz7_temp / 1000;
			break;
		case 8:
			return bts->tz8_temp / 1000;
			break;
		case 9:
			return bts->tz9_temp / 1000;
			break;
		case 10:
			return bts->tz10_temp / 1000;
			break;
		case 11:
			return bts->tz11_temp / 1000;
			break;
		case 12:
			return bts->tz12_temp / 1000;
			break;
		case 13:
			return bts->tz13_temp / 1000;
			break;
		case 14:
			return bts->tz14_temp / 1000;
			break;
		case 15:
			return bts->tz15_temp / 1000;
			break;
		case 16:
			return bts->tz16_temp / 1000;
			break;
		case 17:
			return bts->tz17_temp / 1000;
			break;
		case 18:
			return bts->tz18_temp / 1000;
			break;
		case 19:
			return bts->tz19_temp / 1000;
			break;
		case 20:
			return bts->tz20_temp / 1000;
			break;
		case 21:
			return bts->tz21_temp / 1000;
			break;
		case 22:
			return bts->tz22_temp / 1000;
			break;
		case 23:
			return bts->tz23_temp / 1000;
			break;
		default:
			return -1;
	}
}
/*---------------------------------------------------------------------------*/
// TODO: fix this
static void *bts_update_iv_thread(void *priv)
{
    struct bts_data *dat = (struct bts_data *)priv;
    struct bts_priv *bts = &dat->bts;
    struct itemview *iv = dat->iv;
    int err = 0, len = 0;
    char i=0;
	int ataKey[5] = {0};
	bts_test_exit = false;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    if ((err = bts_open(bts))) {
    	memset(dat->info, 0x00, sizeof(dat->info));
        sprintf(dat->info, uistr_info_sensor_init_fail);
        iv->redraw(iv);
        FLPLOGE("bts_open() err = %d(%s)\n", err, dat->info);
        pthread_exit(NULL);
        return NULL;
    }

    read_str("/sys/class/thermal/thermal_zone0/type", &(bts->tz0[0]), 20);
    read_str("/sys/class/thermal/thermal_zone1/type", &(bts->tz1[0]), 20);
    read_str("/sys/class/thermal/thermal_zone2/type", &(bts->tz2[0]), 20);
    read_str("/sys/class/thermal/thermal_zone3/type", &(bts->tz3[0]), 20);
    read_str("/sys/class/thermal/thermal_zone4/type", &(bts->tz4[0]), 20);
    read_str("/sys/class/thermal/thermal_zone5/type", &(bts->tz5[0]), 20);
    read_str("/sys/class/thermal/thermal_zone6/type", &(bts->tz6[0]), 20);
    read_str("/sys/class/thermal/thermal_zone7/type", &(bts->tz7[0]), 20);
    read_str("/sys/class/thermal/thermal_zone8/type", &(bts->tz8[0]), 20);
    read_str("/sys/class/thermal/thermal_zone9/type", &(bts->tz9[0]), 20);
    read_str("/sys/class/thermal/thermal_zone10/type", &(bts->tz10[0]), 20);
    read_str("/sys/class/thermal/thermal_zone11/type", &(bts->tz11[0]), 20);
    read_str("/sys/class/thermal/thermal_zone12/type", &(bts->tz12[0]), 20);
    read_str("/sys/class/thermal/thermal_zone13/type", &(bts->tz13[0]), 20);
    read_str("/sys/class/thermal/thermal_zone14/type", &(bts->tz14[0]), 20);
    read_str("/sys/class/thermal/thermal_zone15/type", &(bts->tz15[0]), 20);
    read_str("/sys/class/thermal/thermal_zone16/type", &(bts->tz16[0]), 20);
    read_str("/sys/class/thermal/thermal_zone17/type", &(bts->tz17[0]), 20);
    read_str("/sys/class/thermal/thermal_zone18/type", &(bts->tz18[0]), 20);
    read_str("/sys/class/thermal/thermal_zone19/type", &(bts->tz19[0]), 20);
    read_str("/sys/class/thermal/thermal_zone20/type", &(bts->tz20[0]), 20);
    read_str("/sys/class/thermal/thermal_zone21/type", &(bts->tz21[0]), 20);
    read_str("/sys/class/thermal/thermal_zone22/type", &(bts->tz22[0]), 20);
    read_str("/sys/class/thermal/thermal_zone23/type", &(bts->tz23[0]), 20);

	ataKey[0] = findKeyIndex(bts, "mtktsAP");
	ataKey[1] = findKeyIndex(bts, "mtktsbtsmdpa");

	return_data.nbts.num = 2;
        sprintf(return_data.nbts.bts[0].bts_sensor_name, "%8s", "mtktsAP");
        sprintf(return_data.nbts.bts[1].bts_sensor_name, "%13s", "mtktsbtsmdpa");

    while (1) {

        if (dat->exit_thd)
            break;

        if ((err = bts_update_info(bts)))
            continue;

	if(bts_test_exit)
		break;

        len = 0;


		if(i==0)
        {
        	i=1;
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz0[0]), bts->tz0_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz1[0]), bts->tz1_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz2[0]), bts->tz2_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz3[0]), bts->tz3_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz4[0]), bts->tz4_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz5[0]), bts->tz5_temp);
		}
        else if (i==1)
		{
	        i=2;
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz6[0]), bts->tz6_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz7[0]), bts->tz7_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz8[0]), bts->tz8_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz9[0]), bts->tz9_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz10[0]), bts->tz10_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz11[0]), bts->tz11_temp);
		}

        else if (i==2)
		{
	        i=3;
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz12[0]), bts->tz12_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz13[0]), bts->tz13_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz14[0]), bts->tz14_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz15[0]), bts->tz15_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz16[0]), bts->tz16_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz17[0]), bts->tz17_temp);
		}
        else if (i==3)
		{
	        i=0;
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz18[0]), bts->tz18_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz19[0]), bts->tz19_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz20[0]), bts->tz20_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz21[0]), bts->tz21_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz22[0]), bts->tz22_temp);
	        len += snprintf(dat->info+len, sizeof(dat->info)-len, "%s  %d  \n", &(bts->tz23[0]), bts->tz23_temp);
		}

        return_data.nbts.bts[0].bts_temperature = getIndexTemp(bts, ataKey[0]);
        return_data.nbts.bts[1].bts_temperature = getIndexTemp(bts, ataKey[1]);
	LOGD(TAG "%s: %d %f, %s: %d %f\n", return_data.nbts.bts[0].bts_sensor_name, ataKey[0], return_data.nbts.bts[0].bts_temperature,
	return_data.nbts.bts[1].bts_sensor_name, ataKey[1], return_data.nbts.bts[1].bts_temperature);

        iv->set_text(iv, &dat->text);
        iv->redraw(iv);
		int status = get_is_ata();
        if(status == 1)
        {
            thread_exit = true;
            break;
        }
		sleep(3);
    }
    bts_close(bts);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);
    return NULL;
}
/*---------------------------------------------------------------------------*/
int bts_entry(struct ftm_param *param, void *priv)
{
    int chosen;
    struct bts_data *dat = (struct bts_data *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&dat->title, param->name, COLOR_YELLOW);
    init_text(&dat->text, &dat->info[0], COLOR_YELLOW);
    //init_text(&dat->left_btn, uistr_info_sensor_fail, COLOR_YELLOW);
    //init_text(&dat->center_btn, uistr_info_sensor_pass, COLOR_YELLOW);
    //init_text(&dat->right_btn, uistr_info_sensor_back, COLOR_YELLOW);


    snprintf(dat->info, sizeof(dat->info), uistr_info_sensor_initializing);
    dat->exit_thd = false;


    if (!dat->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory");
            return -1;
        }
        dat->iv = iv;
    }
  iv = dat->iv;
  #ifdef FEATURE_FTM_TOUCH_MODE
    	text_t lbtn;
    	text_t cbtn;
    	text_t rbtn;
    	init_text(&lbtn,uistr_key_fail, COLOR_YELLOW);
    	init_text(&cbtn,uistr_key_back, COLOR_YELLOW);
    	init_text(&rbtn,uistr_key_pass, COLOR_YELLOW);
    	iv->set_btn(iv, &lbtn, &cbtn, &rbtn);
  #endif


    iv->set_title(iv, &dat->title);
    iv->set_items(iv, bts_items, 0);
    iv->set_text(iv, &dat->text);

    pthread_create(&dat->update_thd, NULL, bts_update_iv_thread, priv);
    do {
        if(get_is_ata() != 1)
	    {
	        chosen = iv->run(iv, &thread_exit);
	        switch (chosen)
	        {
#ifndef FEATURE_FTM_TOUCH_MODE
		        case ITEM_PASS:
		        case ITEM_FAIL:
		            if (chosen == ITEM_PASS)
                    {
		                dat->mod->test_result = FTM_TEST_PASS;
		            }
                    else if (chosen == ITEM_FAIL)
		            {
		                dat->mod->test_result = FTM_TEST_FAIL;
		            }
		            thread_exit = true;
		            break;
#else
            case L_BTN_DOWN:
            	dat->mod->test_result = FTM_TEST_FAIL;
            	thread_exit = true;
            	break;
            case C_BTN_DOWN:
            	thread_exit = true;
            	break;
            case R_BTN_DOWN:
            	dat->mod->test_result = FTM_TEST_PASS;
            	thread_exit = true;
            	break;
#endif

	        }

	    }
        iv->redraw(iv);
        if (thread_exit)
        {
            dat->exit_thd = true;
            break;
        }
    } while (1);
    pthread_join(dat->update_thd, NULL);

    return 0;
}
/*---------------------------------------------------------------------------*/
int bts_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct bts_data *dat;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_BTS, sizeof(struct bts_data));
    dat  = mod_to_bts_data(mod);

    memset(dat, 0x00, sizeof(*dat));
    //alsps_init_priv(&dat->lps); // TODO: init BTS driver

    /*NOTE: the assignment MUST be done, or exception happens when tester press Test Pass/Test Fail*/
    dat->mod = mod;

    if (!mod)
        return -ENOMEM;

    ret = ftm_register(mod, bts_entry, (void*)dat);

    return ret;
}
#endif // FEATURE_FTM_BTS

