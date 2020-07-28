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
#include <fcntl.h>
#include <string.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#include "common.h"
#include "miniui.h"
#include "ftm.h"
#include <unistd.h>

#include "cust_fm.h"

#ifdef FEATURE_FTM_FM
#include <linux/fm.h>
#include "ftm_audio_Common.h"

#define TAG                 "[FM] "
#define FM_FMD_BAND         FM_BAND_WIDTH
#define FM_GET_HEADSET_STATE
#define UNUSED(x) (void)(x)

/*
Global variable
*/
static int g_freq_item = 0;
static int g_setfreq = 0;
static int g_fm_fd = -1;
static int g_rssi = 0;
static fm_bool g_check_done = fm_false;

#if (defined MT5192_FM || defined MT5193_FM) && defined FM_ANALOG_INPUT
static fm_bool FMAudioEnable = fm_false;
#endif

#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
static int Audio_enable = false;
static void *mI2Sdriver = NULL;
static int mI2Sid = 0;
static char* mAudioBuffer = NULL;
static void *fm_audio_thread(void *priv);
#endif
/*
Function description
*/
extern char *ftm_get_prop(const char *name);
static void read_preferred_freq();
static int read_rssi_th(void);
//static void fm_updateTextInfo(char* output_buf, int buf_len);
static void *fm_update_thread(void *priv);
static void * Audio_Record_thread(void *priv);



extern sp_ata_data return_data;

enum {
    ITEM_FREQ0,
    ITEM_FREQ1,
    ITEM_FREQ2,
    ITEM_FREQ3,
    ITEM_PASS,
    ITEM_FAIL,
//  ITEM_RETURN,
};

static item_t fm_items_man[] = {
    item(ITEM_FREQ0,  uistr_info_fmr_freq0),
    item(ITEM_FREQ1,  uistr_info_fmr_freq1),
    item(ITEM_FREQ2,  uistr_info_fmr_freq2),
    item(ITEM_FREQ3,  uistr_info_fmr_freq3),
    #ifndef FEATURE_FTM_TOUCH_MODE
    item(ITEM_PASS,   uistr_info_fmr_pass),
    item(ITEM_FAIL,   uistr_info_fmr_failed),
    #endif
    //item(ITEM_RETURN, "Return"),
    item(-1, NULL),
};

static item_t fm_items_auto[] = {
    //item(ITEM_FREQ0,  "Test Freq 0"),
    //item(ITEM_FREQ1,  "Test Freq 1"),
    //item(ITEM_FREQ2,  "Test Freq 2"),
    //item(ITEM_FREQ3,  "Test Freq 3"),
    //item(ITEM_PASS,   "Test Pass"),
    //item(ITEM_FAIL,   "Test Fail"),
    //item(ITEM_RETURN, "Return"),
    item(-1, NULL),
};

struct fm_factory {
    char  info[1024];
    fm_bool  exit_thd;
    int   result;

    /* for UI display */
    text_t    title;
    text_t    text;

    pthread_t update_thd;
    pthread_t record_thd;
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
    pthread_t     mAudioThread;
#endif
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
};


#define mod_to_fm(p)     (struct fm_factory*)((char*)(p) + sizeof(struct ftm_module))

#ifdef FM_GET_HEADSET_STATE
static int init_accdet(void)
{
    #define ACCDET_IOC_MAGIC 'A'
    #define ACCDET_INIT      _IO(ACCDET_IOC_MAGIC,0)
    #define ACCDET_PATH      "/dev/accdet"

    int fd = open(ACCDET_PATH, O_RDONLY);
    if(fd < 0){
        LOGD(TAG "open %s failed, fd = %d", ACCDET_PATH, fd);
        return -1;
    }

    if(ioctl(fd, ACCDET_INIT, 0) < 0){
        LOGE(TAG "ioctl ACCDET_INIT failed\n");
        goto out;
    }

out:
    if(fd){
        close(fd);
    }
    return 0;
}

static int fm_get_headset_info(void)
{
    #define HEADSET_STATE_PATH "/sys/bus/platform/drivers/Accdet_Driver/state"
    #define ACCDET_STATE_PATH "/sys/bus/platform/drivers/Accdet_Driver/state"
    #define BUF_LEN 2

    int ret = 0;
    int fd = -1;
    char rbuf[BUF_LEN] = {'\0'};
    char wbuf[BUF_LEN] = {'1'};
    char wbuf1[BUF_LEN] = {'2'};

    fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
    if (fd < 0){
        LOGD(TAG "Can't open %s\n", HEADSET_STATE_PATH);
        ret = -1;
        goto out;
    }
    if (read(fd, rbuf, BUF_LEN-1) == -1){
        LOGD(TAG "Can't read %s\n", HEADSET_STATE_PATH);
        ret = -2;
        goto out;
    }

    if(!strncmp(wbuf, rbuf, BUF_LEN-1)){
        LOGD(TAG "state== 1");
        ret = 1;
        goto out;
    }else if(!strncmp(wbuf1, rbuf, BUF_LEN-1)) {
        LOGD(TAG "state== 2");
        ret = 2;
    }else{
        LOGW(TAG "else state== %s", rbuf);
        ret = 0;
    }

out:
    if(fd >= 0) close(fd);
    return ret;
}
#endif

int fm_entry(struct ftm_param *param, void *priv)
{
    int chosen;
    bool exit = false;
    struct fm_factory *fmr = (struct fm_factory *)priv;
    struct itemview *iv;
    int headset_state = 0;
    int rssi_th = 0;
    char *rawVal = NULL;
#ifdef FEATURE_FTM_TOUCH_MODE
    text_t* lbtn = NULL;
    text_t* cbtn = NULL;
    text_t* rbtn = NULL;
#endif

    LOGD(TAG "%s\n", __FUNCTION__);

    memset(&fmr->info[0], 0, sizeof(fmr->info));
    memset(&fmr->info[0], '\n', 10);

    init_text(&fmr->title, param->name, COLOR_YELLOW);
    init_text(&fmr->text, &fmr->info[0], COLOR_YELLOW);

    /* show text view */
    if (!fmr->iv) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        fmr->iv = iv;
    }

    iv = fmr->iv;
    iv->set_title(iv, &fmr->title);
    if (FTM_AUTO_ITEM == param->test_type) {
        iv->set_items(iv, fm_items_auto, 0);
    } else if (FTM_MANUAL_ITEM == param->test_type) {
        iv->set_items(iv, fm_items_man, 0);
#ifdef FEATURE_FTM_TOUCH_MODE
        lbtn = malloc(sizeof(struct itemview));
        cbtn = malloc(sizeof(struct itemview));
        rbtn = malloc(sizeof(struct itemview));

        memset(lbtn, 0, sizeof(text_t));
        memset(cbtn, 0, sizeof(text_t));
        memset(rbtn, 0, sizeof(text_t));

        init_text(lbtn, uistr_key_fail, COLOR_YELLOW);
        init_text(cbtn, uistr_key_back, COLOR_YELLOW);
        init_text(rbtn, uistr_key_pass, COLOR_YELLOW);
        iv->set_btn(iv, lbtn, cbtn, rbtn);
#endif
    }

    iv->set_text(iv, &fmr->text);
    iv->start_menu(iv,0);
    iv->redraw(iv);

    //check headset state
#ifdef FM_GET_HEADSET_STATE
    int i;
    for (i=0; i<8; i++) {
        LOGD("[FM]init headset device\n");
        init_accdet();
        LOGD("[FM]get headset state\n");
        headset_state = fm_get_headset_info();
        if (headset_state > 0) {
            break;
        } else {
            LOGD("[FM]waiting for insert headset device\n");
            sprintf(fmr->info, "%s", uistr_info_fmr_no_headset_warning);
            iv->redraw(iv);
            usleep(1000*1000);//sleep 1s
        }
    }
    if (headset_state <= 0) {
        LOGD("[FM]no headset device\n");
        fmr->mod->test_result = FTM_TEST_FAIL;
        return -1;
    }
#endif
    LOGD("[FM]init audio device\n");
    Common_Audio_init();

    /* initialize thread condition */
    read_preferred_freq();
    fmr->exit_thd = fm_false;
    fmr->result = fm_false;

    pthread_create(&fmr->update_thd, NULL, fm_update_thread, priv);
    pthread_create(&fmr->record_thd, NULL, Audio_Record_thread, priv);
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
    Audio_enable = fm_false;
    mI2Sdriver = I2SGetInstance();
    if(mI2Sdriver == NULL){
       LOGD("I2S driver doesn't exists\n");
       goto FM_EXIT;
    }
    mI2Sid = I2SOpen(mI2Sdriver);
    if(mI2Sid == 0){
       LOGD("I2S driver get ID fail\n");
       goto FM_EXIT;
    }
    pthread_create(&fmr->mAudioThread, NULL, fm_audio_thread, priv);
#endif

    iv->redraw(iv);

if(FTM_AUTO_ITEM == param->test_type)
{
    do {
        g_setfreq = 1;
        g_freq_item = 0;
        usleep(200 * 1000); // update every 200ms
        LOGD("para checking\n");
    } while (fm_false == g_check_done);
    g_setfreq = 0;

    rawVal = ftm_get_prop("FMRadio.PlayTime"); // Delay enough time for playing
	if (rawVal != NULL)
    {
        LOGD("FTMFMPlayTime = %s\n", rawVal);
        sleep(atoi(rawVal));
    }

	rssi_th = read_rssi_th();
    if (g_rssi >= rssi_th) {
        fmr->mod->test_result = FTM_TEST_PASS;
        LOGD("check pass\n");
		//sprintf(fmr->info+strlen(fmr->info), uistr_info_fmr_pass, rssi_th);
		snprintf(fmr->info+strlen(fmr->info), sizeof(fmr->info)-strlen(fmr->info)-1, uistr_info_fmr_pass, rssi_th);
		iv->redraw(iv);
    } else {
        fmr->mod->test_result = FTM_TEST_FAIL;
        LOGD("check fail\n");
		//sprintf(fmr->info+strlen(fmr->info), uistr_info_fmr_failed, rssi_th);
		snprintf(fmr->info+strlen(fmr->info), sizeof(fmr->info)-strlen(fmr->info)-1, uistr_info_fmr_failed, rssi_th);
		iv->redraw(iv);
    }
	fmr->exit_thd = fm_true;

    usleep(2000 * 1000); // delay 2000ms
    g_check_done = fm_false;
    exit = true;
}
else if(FTM_MANUAL_ITEM == param->test_type)
{
    /* process procedure */
    do {
        chosen = iv->run(iv, &exit);
        switch (chosen) {
        case ITEM_FREQ0:
            g_setfreq = 1;
            g_freq_item = 0;
            break;
        case ITEM_FREQ1:
            g_setfreq = 1;
            g_freq_item = 1;
            break;
        case ITEM_FREQ2:
            g_setfreq = 1;
            g_freq_item = 2;
            break;
        case ITEM_FREQ3:
            g_setfreq = 1;
            g_freq_item = 3;
            break;
#ifndef FEATURE_FTM_TOUCH_MODE
        case ITEM_PASS:
        case ITEM_FAIL:
//      case ITEM_RETURN:
            if (chosen == ITEM_PASS) {
                fmr->mod->test_result = FTM_TEST_PASS;
            } else if (chosen == ITEM_FAIL) {
                fmr->mod->test_result = FTM_TEST_FAIL;
            }
            g_setfreq = 0;
            exit = true;
            break;
#endif
#ifdef FEATURE_FTM_TOUCH_MODE
        case L_BTN_DOWN:
            fmr->mod->test_result = FTM_TEST_FAIL;
            g_setfreq = 0;
            exit = true;
            break;
        case C_BTN_DOWN:
            g_setfreq = 0;
            exit = true;
            break;
        case R_BTN_DOWN:
            fmr->mod->test_result = FTM_TEST_PASS;
            g_setfreq = 0;
            exit = true;
            break;
#endif
        }
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
    if(g_setfreq == 1)
    {
        if(!I2SStart(mI2Sdriver,mI2Sid,MATV)){
            LOGE("I2S start fialed");
        }
        usleep(500);
        Audio_enable = fm_true;
     }
#endif
     if (exit) {
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
     Audio_enable = fm_false;
     usleep(500);
     I2SStop(mI2Sdriver,mI2Sid);
#endif
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_ANALOG_INPUT
     FMAudioEnable = fm_false;
#endif
     fmr->exit_thd = fm_true;
     break;
     }
    } while (1);
}

#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
FM_EXIT:
#endif
    pthread_join(fmr->update_thd, NULL);
    pthread_join(fmr->record_thd, NULL);
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
    I2SClose(mI2Sdriver,mI2Sid);
    //free I2S instance
    I2SFreeInstance(mI2Sdriver);
    mI2Sdriver = NULL;
    pthread_join(fmr->mAudioThread, NULL);
#endif
    Common_Audio_deinit();
#ifdef FEATURE_FTM_TOUCH_MODE
    if (lbtn)
        free(lbtn);
    if (cbtn)
        free(cbtn);
    if (rbtn)
        free(rbtn);
#endif

    LOGD("FM out\n");
    return 0;
}

int fm_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct fm_factory *fmr;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_FM, sizeof(struct fm_factory));
    if (!mod)
        return -ENOMEM;

    fmr  = mod_to_fm(mod);
    fmr->mod = mod;

    ret = ftm_register(mod, fm_entry, (void*)fmr);

    return ret;
}

/*
static void fm_updateTextInfo(char* output_buf, int buf_len)
{
    return;
}
*/

static void read_preferred_freq()
{
    unsigned int i = 0;
    char *pFreq = NULL;
    char channel_no[64]; //max path

    for (i = 0; i < sizeof(fm_freq_list)/sizeof(fm_freq_list[0]); i++) {
        memset(channel_no, 0, sizeof(channel_no));
        sprintf(channel_no, "FMRadio.CH%d", i+1);
        pFreq = ftm_get_prop(channel_no);

        if (pFreq != NULL) {
            fm_freq_list[i] = (uint16_t)atoi(pFreq);
            LOGD(TAG "preferred_freq: %d, %d\n", i, fm_freq_list[i]);
        } else {
            LOGD(TAG "preferred_freq %d can't get\n", i);
        }
    }
}


static int read_rssi_th(void)
{
    unsigned int rssi = 0;
    char *pRSSI_TH = NULL;

    pRSSI_TH = ftm_get_prop("FMRadio.RSSITH");

    if (pRSSI_TH) {
        rssi = atoi(pRSSI_TH);
        LOGD(TAG "rssi th: %d\n", rssi);
    } else {
        LOGE(TAG "rssi th: failed\n");
    }

    return rssi;
}


#if (defined MT5192_FM || defined MT5193_FM) && defined FM_DIGITAL_INPUT
static void *fm_audio_thread(void *priv)
{
    struct fm_factory *fmr = (struct fm_factory *)priv;
    struct itemview *iv = fmr->iv;
    int bufSize = 0;
    int numread = 0;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    bufSize = I2SGetReadBufferSize(mI2Sdriver);
    LOGV("got buffer size = %d", bufSize);
    mAudioBuffer = (char *)malloc(bufSize * 3);

    while (1) {
        if (fmr->exit_thd)
           break;

        if (Audio_enable == fm_true) {
           memset(mAudioBuffer, 0x0,bufSize * 3);
           numread = I2SRead(mI2Sdriver, mI2Sid, mAudioBuffer, bufSize);
           ATV_AudioWrite(mAudioBuffer, numread);
        } else {
           usleep(500 * 1000);
        }
    }

    LOGD(TAG "%s: matv_audio_thread Exit\n", __FUNCTION__);

    if (mAudioBuffer) {
        free(mAudioBuffer);
        mAudioBuffer = NULL;
    }
    return NULL;
}
#endif
static void *fm_update_thread(void *priv)
{
    int ret = -1, rssi = 0;
    struct fm_factory *fmr = (struct fm_factory *)priv;
    struct itemview *iv = fmr->iv;
    struct fm_tune_parm parm_tune;
    float f_freq;
    uint32_t mute = 0;
    int type = -1;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    memset(fmr->info, 0x00, sizeof(fmr->info));

    //move here to avoid to close FM device node
#ifdef FM_ANALOG_INPUT
    FMLoopbackTest(fm_true);
#else
    Audio_I2S_Play(fm_true);
#endif

#if (defined MT5192_FM || defined MT5193_FM)
    g_fm_fd=0;
#else
    g_fm_fd = open(FM_DEVICE_NAME, O_RDWR);
    LOGD(TAG "FM_IOCTL_POWERUP, g_fm_fd=%d\n", g_fm_fd);
#endif
    if (g_fm_fd < 0) {
        LOGD(TAG "Factory Mode Open FM %s failed\n", FM_DEVICE_NAME);
        sprintf(fmr->info, "%s", uistr_info_fmr_open_fail);
        iv->redraw(iv);
        goto out;
    }

    parm_tune.band = FM_FMD_BAND;
#ifdef MTK_FM_50KHZ_SUPPORT
    parm_tune.freq = fm_freq_list[0]*10; //default value for FM power up.
#else
    parm_tune.freq = fm_freq_list[0]; //default value for FM power up.
#endif //MTK_FM_50KHZ_SUPPORT
    parm_tune.hilo = FM_AUTO_HILO_OFF;
    parm_tune.space = FM_SPACE_WIDTH;
#if (defined MT5192_FM || defined MT5193_FM)
    ret =fm_powerup(&parm_tune) ? 0 : 1;
#else
    ret = ioctl(g_fm_fd, FM_IOCTL_POWERUP, &parm_tune);
    LOGD(TAG "FM_IOCTL_POWERUP, ret=%d\n", ret);
#endif
    if(ret) {
        LOGD(TAG "FM_OP_POWER_ON failed\n");
#if (!defined MT5192_FM && !defined MT5193_FM)
        close(g_fm_fd);
 #endif
        g_fm_fd = -1;
        sprintf(fmr->info, "%s", uistr_info_fmr_poweron_fail);
        iv->redraw(iv);
        goto out;
    }

#if (defined MT5192_FM || defined MT5193_FM)
    ret = fm_mute(mute) ? 0 : 1;
#else
    ret = ioctl(g_fm_fd, FM_IOCTL_MUTE, &mute);
    LOGD(TAG "FM_IOCTL_MUTE, ret=%d\n", ret);
#endif
    if(ret) {
        LOGD(TAG "FM_IOCTL_MUTE failed\n");
#if (!defined MT5192_FM && !defined MT5193_FM)
        close(g_fm_fd);
 #endif
        g_fm_fd = -1;
        sprintf(fmr->info, "%s", uistr_info_fmr_mute_fail);
        iv->redraw(iv);
        goto out;
    }

    sprintf(fmr->info, "%s", uistr_info_fmr_poweron_ok);
    iv->redraw(iv);

    while (1) {
        usleep(200000); // update every 200ms
        if (fmr->exit_thd)
            break;

        if (g_setfreq) {
#if (defined MT5192_FM || defined MT5193_FM) && defined FM_ANALOG_INPUT
         if (!FMAudioEnable) {
           ret= matv_set_chipdep(190,3) ? 0 : 1;
           if(ret) {
            g_fm_fd = -1;
            sprintf(fmr->info, "%s", "matv_set_chipdep failed\n");
            iv->redraw(iv);
            return NULL;
            }
           FMAudioEnable = fm_true;
        }
#endif
            g_setfreq = 0;
            memset(fmr->info, 0x0, sizeof(fmr->info));

            f_freq = (float)fm_freq_list[g_freq_item]/10;
            sprintf(fmr->info, "%s %g%s\n", uistr_info_fmr_setfreq,  f_freq, uistr_info_fmr_mhz);
            iv->redraw(iv);
            LOGD(TAG "freq=%d\n", fm_freq_list[g_freq_item]);

            //set Freq and get RSSI
            parm_tune.band = FM_FMD_BAND;
#ifdef MTK_FM_50KHZ_SUPPORT
            parm_tune.freq = fm_freq_list[g_freq_item]*10;
#else
            parm_tune.freq = fm_freq_list[g_freq_item];
#endif //MTK_FM_50KHZ_SUPPORT
            parm_tune.hilo = FM_AUTO_HILO_OFF;
            parm_tune.space = FM_SPACE_100K;
#if (defined MT5192_FM || defined MT5193_FM)
           ret = fm_tune(&parm_tune) ? 0 : 1;
#else
            ret = ioctl(g_fm_fd, FM_IOCTL_TUNE, &parm_tune);
            LOGD(TAG "FM_IOCTL_TUNE, ret=%d\n", ret);
#endif
            if (ret) {
                //sprintf(fmr->info+strlen(fmr->info), uistr_info_fmr_fail);
                snprintf(fmr->info + strlen(fmr->info), sizeof(fmr->info) - strlen(fmr->info) - 1, uistr_info_fmr_fail);
                iv->redraw(iv);
                LOGD(TAG "FM_IOCTL_TUNE failed:%d:%d\n", ret, parm_tune.err);
            } else {
                //sprintf(fmr->info+strlen(fmr->info), uistr_info_fmr_success);
                //iv->redraw(iv);
#if (defined MT5192_FM || defined MT5193_FM)
               rssi = fm_getrssi();
               ret = 0;
#else
                ret = ioctl(g_fm_fd, FM_IOCTL_GETRSSI, &rssi);
                LOGD(TAG "FM_IOCTL_GETRSSI, ret=%d\n", ret);
#endif
                if (ret) {
                     LOGD(TAG "FM_IOCTL_GETRSSI failed\n");
                } else {
                     //sprintf(fmr->info+strlen(fmr->info), uistr_info_fmr_rssi, rssi);
                     snprintf(fmr->info + strlen(fmr->info), sizeof(fmr->info) - strlen(fmr->info) - 1, uistr_info_fmr_rssi, rssi);
                     LOGD(TAG "freq %d, rssi %d\n", parm_tune.freq, rssi);
                     iv->redraw(iv);
                     g_rssi = rssi;
                     g_check_done = fm_true;
                }
                return_data.fm.fm_rssi = rssi;
            }
        }
        //other control
    }
#ifdef FM_ANALOG_INPUT
    FMLoopbackTest(fm_false);
#else
    // add delay to make sure sound can be heard
    sleep(1);
    Audio_I2S_Play(fm_false);
#endif

#if (defined MT5192_FM || defined MT5193_FM)
    fm_powerdown(0);
#else
    type = FM_RX;
    ioctl(g_fm_fd, FM_IOCTL_POWERDOWN, &type);
    close(g_fm_fd);
#endif
    g_fm_fd = -1;

out:
    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    g_check_done = fm_true;
    pthread_exit(NULL);
    return NULL;
}

static void * Audio_Record_thread(void *priv)
{
    struct fm_factory *fmr = (struct fm_factory *)priv;
    ALOGD(TAG "%s: Start", __FUNCTION__);
    usleep(100000);
    fm_bool dumpFlag = 0;

    short pbuffer[8192]={0};
    short pbufferL[4096]={0};
    short pbufferR[4096]={0};
    unsigned int freqDataL[3]={0}, magDataL[3]={0};
    unsigned int freqDataR[3]={0}, magDataR[3]={0};

    uint32_t samplerate = 0;

    return_data.fm.freq = 0;
    return_data.fm.ampl = 0;

    recordInit(BUILTIN_MIC, &samplerate);
    while (1) {
       memset(pbuffer,0,sizeof(pbuffer));
       memset(pbufferL,0,sizeof(pbufferL));
       memset(pbufferR,0,sizeof(pbufferR));

       int readSize  = readRecordData(pbuffer,8192*2);
       int i;
        for(i = 0 ; i < 4096 ; i++) {
            pbufferL[i] = pbuffer[2 * i];
            pbufferR[i] = pbuffer[2 * i + 1];
        }

        if (dumpFlag) {
            char filenameL[]="/data/record_dataL.pcm";
            char filenameR[]="/data/record_dataR.pcm";
            FILE * fpL= fopen(filenameL, "wb+");
            FILE * fpR= fopen(filenameR, "wb+");

            if (fpL!=NULL) {
                fwrite(pbufferL,readSize/2,1,fpL);
                fclose(fpL);
            }

            if(fpR!=NULL) {
                fwrite(pbufferR,readSize/2,1,fpR);
                fclose(fpR);
            }
        }
        memset(freqDataL,0,sizeof(freqDataL));
        memset(freqDataR,0,sizeof(freqDataR));
        memset(magDataL,0,sizeof(magDataL));
        memset(magDataR,0,sizeof(magDataR));
        ApplyFFT256(samplerate,pbufferL,0,freqDataL,magDataL);
        ApplyFFT256(samplerate,pbufferR,0,freqDataR,magDataR);

        ALOGD(TAG "FM FFT: FreqL = %d, FreqR = %d, AmpL = %d, AmpR = %d", freqDataL[0], freqDataR[0], magDataL[0], magDataR[0]);

        if (fmr->exit_thd) {
            break;
        }
      }

      ALOGD(TAG "FM FFT: FreqL = %d, FreqR = %d, AmpL = %d, AmpR = %d", freqDataL[0], freqDataR[0], magDataL[0], magDataR[0]);

      return_data.fm.freq = freqDataL[0];
      return_data.fm.ampl = magDataL[0];

      ALOGD(TAG "%s: Stop", __FUNCTION__);
      pthread_exit(NULL); // thread exit
      return NULL;
}

#endif
