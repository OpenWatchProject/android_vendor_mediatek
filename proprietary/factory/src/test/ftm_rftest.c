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
#include <pthread.h>
#include <fcntl.h>
#include <cutils/properties.h>
#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include "hardware/ccci_intf.h"

extern int send_at (const int fd, const char *pCMD);
extern int wait4_ack (const int fd, char *pACK, int timeout);
#define DEVICE_NAME ccci_get_node_name(USR_FACTORY_RF, MD_SYS1)

#ifdef FEATURE_FTM_RF

/* Replaced by "Gsm.RF.rssi.dbm" defined in factory.ini */
////#ifndef RF_TEST_RSSI_LEVEL
////#define RF_TEST_RSSI_LEVEL 80 /* RF Test threshold might be different by projects or chips. Please customize it in custom\factory\inc\cust_rf.h if necessary.  */
////#endif

#define TAG    "[RF] "

static void *rf_update_thread(void *priv);
int rf_test_entry(struct ftm_param *param, void *priv);
int rf_test_init(void);

extern sp_ata_data return_data;
static item_t rf_items[] = {
    item(-1, NULL),
};

struct rf_factory {
    char info[1024];
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct itemview *iv;
    pthread_t update_thread;
    bool exit_thread;
    bool test_done;
};

#define mod_to_rf(p)  (struct rf_factory*)((char*)(p) + sizeof(struct ftm_module))

static int at_tok_start(char **p_cur)
{
    if (*p_cur == NULL) {
        return -1;
    }

    // skip prefix
    // consume "^[^:]:"

    *p_cur = strchr(*p_cur, ':');

    if (*p_cur == NULL) {
        return -1;
    }

    (*p_cur)++;

    return 0;
}

static void skipWhiteSpace(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && isspace(**p_cur)) {
        (*p_cur)++;
    }
}

static void skipNextComma(char **p_cur)
{
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && **p_cur != ',') {
        (*p_cur)++;
    }

    if (**p_cur == ',') {
        (*p_cur)++;
    }
}

static char * nextTok(char **p_cur)
{
    char *ret = NULL;

    skipWhiteSpace(p_cur);

    if (*p_cur == NULL) {
        ret = NULL;
    } else if (**p_cur == '"') {
        (*p_cur)++;
        ret = strsep(p_cur, "\"");
        skipNextComma(p_cur);
    } else {
        ret = strsep(p_cur, ",");
    }

    return ret;
}

/**
 * Parses the next integer in the AT response line and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 * "base" is the same as the base param in strtol
 */

static int at_tok_nextint_base(char **p_cur, int *p_out, int base, int  uns)
{
    char *ret;

    if (*p_cur == NULL) {
        return -1;
    }

    ret = nextTok(p_cur);

    if (ret == NULL) {
        return -1;
    } else {
        long l;
        char *end;

        if (uns)
            l = strtoul(ret, &end, base);
        else
            l = strtol(ret, &end, base);

        *p_out = (int)l;

        if (end == ret) {
            return -1;
        }
    }

    return 0;
}

/**
 * Parses the next base 10 integer in the AT response line
 * and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 */
static int at_tok_nextint(char **p_cur, int *p_out)
{
    return at_tok_nextint_base(p_cur, p_out, 10, 0);
}

/** returns 1 on "has more tokens" and 0 if no */
static int at_tok_hasmore(char **p_cur)
{
    return ! (*p_cur == NULL || **p_cur == '\0');
}

int read_preferred_para(const char *pstr,int default_value)
{
    int value = default_value;
    char *pVal = NULL;

    if(pstr== NULL)
    {
        LOGD(TAG "preferred_receiver_para error %s", pstr);
        return 0;
    }
    pVal = ftm_get_prop(pstr);
    if (pVal != NULL){
        value = (int)atoi(pVal);
        LOGD(TAG "preferred_receiver_para %s- %d \n",pstr,value);
    }
    else{
        LOGD(TAG "preferred_receiver_para can't get %s , use default %d\n", pstr, value);
    }
    return value;
}

static void *lte_rf_update_thread(void *priv)
{
    struct rf_factory *rf = (struct rf_factory*)priv;
    struct itemview *iv = rf->iv;
    int rssi0_dbm = 0;
    int rssi1_dbm = 0;
    int ret = 0;
    int len = 0;
    int retryCount = 0;
    int mLteRfBand = read_preferred_para("RF.band",1);;
    int mLteRfDlEarfcn = read_preferred_para("RF.dl.earfcn",300);
    int mLteRfRssi0dbm = read_preferred_para("RF.rssi0.dbm",-75);
    int mLteRfRssi1dbm = read_preferred_para("RF.rssi1.dbm",-80);
    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];
    const int HALT_TIME = 200000;//0.2s (200000ms)

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    int fd = -1;
    fd = open(DEVICE_NAME, O_RDWR);
    if(fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n",DEVICE_NAME, strerror(errno));
        goto err;
    }
    LOGD(TAG "%s has been opened...\n",DEVICE_NAME);

    memset(rf->info, 0, sizeof(rf->info) / sizeof(*(rf->info)));
    if(!rf->test_done) {
        rf->test_done = true;

        LOGD(TAG "[AT]AT polling first:\n");
        do {
            send_at (fd, "AT\r\n");
        } while (wait4_ack (fd, NULL, 300));

        LOGD(TAG "[AT]Disable Sleep Mode:\n");
        send_at (fd, "AT+ESLP=0\r\n");
        if (wait4_ack (fd, NULL, 3000))goto err;

        /* To start SIM*/
        LOGD(TAG "Send AT+CFUN=4 to startup modem \n");
        send_at (fd, "AT+CFUN=4\r\n");
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+CFUN=4 OK ,Start to set 4G RSSI test\n");

        /* To check whether device support the input band */
        LOGD(TAG "Check modem supported LTE band\n");

        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+EPBSE?\r\n", BUF_SIZE);
        write(fd, cmd_buf, strlen(cmd_buf));
        LOGD(TAG "Send AT+EPBSE?\n");
        while (1) {
            const char *tok_epbse = "+EPBSE: ";
            char *p_epbse = NULL;
            char *p_ok = NULL;

            memset(rsp_buf, 0, sizeof(rsp_buf));

            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------AT+EPBSE? start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+EPBSE? end------\n");

            p_epbse = strstr(rsp_buf, tok_epbse);

            if (p_epbse!= NULL) {
                LOGD(TAG "p_epbse=%s\n", p_epbse);
                //example: p_epbse now is pointed to "+EPBSE: 154,33,0,480"
                at_tok_start(&p_epbse);

                int gsm_band;
                int umts_band;
                int lte_band_1_32;
                int lte_band_33_64;
                int lte_band_65_96;
                int lte_band_97_128;
                int lte_band_129_160;
                int lte_band_161_192;
                int lte_band_193_224;
                int lte_band_225_256;
                int band_extension_support = 0;

                at_tok_nextint(&p_epbse, &gsm_band);
                at_tok_nextint(&p_epbse, &umts_band);
                at_tok_nextint(&p_epbse, &lte_band_1_32);
                at_tok_nextint(&p_epbse, &lte_band_33_64);
                // NEW MD support LTE band 65~256
                if (at_tok_hasmore(&p_epbse)) {
                    band_extension_support = 1;
                    at_tok_nextint(&p_epbse, &lte_band_65_96);
                    at_tok_nextint(&p_epbse, &lte_band_97_128);
                    at_tok_nextint(&p_epbse, &lte_band_129_160);
                    at_tok_nextint(&p_epbse, &lte_band_161_192);
                    at_tok_nextint(&p_epbse, &lte_band_193_224);
                    at_tok_nextint(&p_epbse, &lte_band_225_256);
                }

                // try to read out "OK" after +EPBSE: x,x in buffer if any.This is to prevent mis-parsing hereafter
                if (p_epbse != NULL) {
                    p_ok = strstr(p_epbse, "OK");
                }
                if (p_ok == NULL) {
                    LOGD(TAG "No OK found");
                    //wait4_ack(fd, NULL, 500);
                }

                if ((mLteRfBand >= 1) && (mLteRfBand <= 32)) {
                    if (!(lte_band_1_32 & (1 << (mLteRfBand - 1)))) {
                        LOGD(TAG "Band (%d) not support", mLteRfBand);
                        goto err;
                    }
                } else if ((mLteRfBand >= 33) && (mLteRfBand <= 64)) {
                    if (!(lte_band_33_64 & (1 << (mLteRfBand - 33)))) {
                        LOGD(TAG "Band (%d) not support", mLteRfBand);
                        goto err;
                    }
                }
                if (band_extension_support == 1) {
                    if ((mLteRfBand >= 65) && (mLteRfBand <= 96)) {
                        if (!(lte_band_65_96 & (1 << (mLteRfBand - 65)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    } else if ((mLteRfBand >= 97) && (mLteRfBand <= 128)) {
                        if (!(lte_band_97_128 & (1 << (mLteRfBand - 97)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    } else if ((mLteRfBand >= 129) && (mLteRfBand <= 160)) {
                        if (!(lte_band_129_160 & (1 << (mLteRfBand - 129)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    } else if ((mLteRfBand >= 161) && (mLteRfBand <= 192)) {
                        if (!(lte_band_161_192 & (1 << (mLteRfBand - 161)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    } else if ((mLteRfBand >= 193) && (mLteRfBand <= 224)) {
                        if (!(lte_band_193_224 & (1 << (mLteRfBand - 193)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    } else if ((mLteRfBand >= 225) && (mLteRfBand <= 256)) {
                        if (!(lte_band_225_256 & (1 << (mLteRfBand - 225)))) {
                            LOGD(TAG "Band (%d) not support", mLteRfBand);
                            goto err;
                        }
                    }
                }
                break;
            }
        }

        /* To start test 4G RSSI */
        LOGD(TAG "Send AT+ERFTX=7,%d,%d to set test 4G RSSI \n",mLteRfBand,mLteRfDlEarfcn);
        memset(cmd_buf, 0, sizeof(cmd_buf));
        sprintf(cmd_buf,"AT+ERFTX=7,%d,%d\r\n",mLteRfBand,mLteRfDlEarfcn);
        send_at (fd, cmd_buf);
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "%s OK ,Start to turn on RF\n",cmd_buf);

        /* Wait URC +ERXRSSI: <rssi0_dBm>, <rssi1_dBm> for verification */
        retryCount = 0;

        while(retryCount < 100){
            char *p = NULL;
            rssi0_dbm = 0;
            rssi1_dbm = 0;

            memset(rsp_buf, 0, sizeof(rsp_buf));
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------Wait ERXRSSI URC start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------Wait ERXRSSI URC end------\n");

            retryCount++;
            p = strstr(rsp_buf, "+ERXRSSI:");

            if(p!= NULL){
                //example: p now is pointed to "+ERXRSSI: -80,-96"
                at_tok_start(&p);

                //parse out "-80" and assigned to rssi0_dbm
                at_tok_nextint(&p, &rssi0_dbm);

                //eamplex: p now is pointed to "-96"
                at_tok_nextint(&p, &rssi1_dbm);

                //<rssi0_dbm>,<rssi1_dbm> shall be negative integer value and -60 dbm is a stronger signal than -80
                if(((rssi0_dbm <0) && (rssi0_dbm > mLteRfRssi0dbm)) &&
                   ((rssi1_dbm <0) && (rssi1_dbm > mLteRfRssi1dbm))){
                    ret = 1;
                    LOGD(TAG "rssi0_dbm=%d ,rssi1_dbm=%d pass in RF test",rssi0_dbm,rssi1_dbm);
                    break;
                } else {
                    ret = 0;
                    LOGD(TAG "rssi0_dbm=%d ,rssi1_dbm=%d fail in RF test",rssi0_dbm,rssi1_dbm);
                    break;
                }
            }
            usleep(HALT_TIME);
        }

        return_data.rf.rf_data = 0-rssi0_dbm;
        return_data.rf.rf_data_lte = 0-rssi1_dbm;

        if(ret) {
            LOGD(TAG "RF Test result pass\n");

            len = 0;
            len += sprintf(rf->info + len,
                "%s: %s. \n", uistr_rf_test, uistr_info_pass);
            len += sprintf(rf->info + len,
                "Verdict RSSI_0: %d dbm ,RSSI_1: %d dbm\n", mLteRfRssi0dbm,mLteRfRssi1dbm);
            len += sprintf(rf->info + len,
                "Received RSSI_0: %d dbm ,RSSI_1: %d dbm\n", rssi0_dbm,rssi1_dbm);
            len += sprintf(rf->info + len,
                "LTE Band: %d ,DL Channel Number: %d \n", mLteRfBand,mLteRfDlEarfcn);

            close(fd);
            fd = -1;

            LOGD(TAG "%s: Exit\n", __FUNCTION__);
            iv->redraw(iv);
            usleep(HALT_TIME * 30);

            return NULL;
        } else {
            LOGD(TAG "RF Test result fail\n");
            goto err;
        }
    } // end if(!sim->test_done)

err:
    LOGD(TAG "%s: FAIL\n",__FUNCTION__);

    len = 0;
    len += sprintf(rf->info + len,
            "%s: %s. \n", uistr_rf_test, uistr_info_fail);
    len += sprintf(rf->info + len,
            "Verdict RSSI_0: %d dbm ,RSSI_1: %d dbm\n", mLteRfRssi0dbm,mLteRfRssi1dbm);
    len += sprintf(rf->info + len,
            "Received RSSI_0: %d dbm ,RSSI_1: %d dbm\n", rssi0_dbm,rssi1_dbm);
    len += sprintf(rf->info + len,
            "LTE Band: %d ,DL Channel Number: %d \n", mLteRfBand,mLteRfDlEarfcn);

    close(fd);
    fd = -1;

    LOGD(TAG "redraw\n");
    iv->redraw(iv);
    usleep(HALT_TIME * 30);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return NULL;
}



static void *rf_update_thread(void *priv)
{
    struct rf_factory *rf = (struct rf_factory*)priv;
    struct itemview *iv = rf->iv;
    int rssi_level = 0;
    int mGsmRfRssidbm = read_preferred_para("Gsm.RF.rssi.dbm",-80);
    int len = 0;
    const int HALT_TIME = 200000;//0.2s (200000ms)

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];
    //[ALPS01679754]-start
    char bandSetting[BUF_SIZE];
    bool isBackup = false;
    //[ALPS01679754]-end

    int fd = -1;
    fd = open(DEVICE_NAME, O_RDWR);
    if(fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n",DEVICE_NAME, strerror(errno));
        goto err;
    }
    LOGD(TAG "%s has been opened...\n",DEVICE_NAME);

    int retryCount = 0;
    int dynamicBandSupport = 0;

    memset(rf->info, 0, sizeof(rf->info) / sizeof(*(rf->info)));
    if(!rf->test_done) {
        bool ret = false;
        rf->test_done = true;

        LOGD(TAG "[AT]AT polling first:\n");
        do
        {
            send_at (fd, "AT\r\n");
        } while (wait4_ack (fd, NULL, 300));

        LOGD(TAG "[AT]Disable Sleep Mode:\n");
        send_at (fd, "AT+ESLP=0\r\n");
        if (wait4_ack (fd, NULL, 3000))goto err;

        //[ALPS01679754]-start : backup default bend mode settings
        while(1){
            const char *tok_epbse = "+EPBSE:";
            const char *tok_split = " ";
            char *p_epbse = NULL;
            char *p_str = NULL;
            char *p_ok = NULL;

            memset(cmd_buf, 0, sizeof(cmd_buf));
            memset(rsp_buf, 0, sizeof(rsp_buf));
            memset(bandSetting, 0, sizeof(bandSetting));

            strncpy(cmd_buf, "AT+EPBSE?\r\n", BUF_SIZE-1);
            write(fd, cmd_buf, strlen(cmd_buf));
            LOGD(TAG "Send AT+EPBSE?\n");
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------AT+EPBSE? start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+EPBSE? end------\n");

            p_epbse = strstr(rsp_buf, tok_epbse);

            if(p_epbse!= NULL){
                p_str = strstr(p_epbse, tok_split);
                LOGD(TAG "p_str=%s\n", p_str+1);
                strncpy(bandSetting, p_str+1, BUF_SIZE-1);
                LOGD(TAG "bandSetting=%s\n", bandSetting);
                isBackup = true;
                //try to read out "OK" after +EPBSE: x,x in buffer if any.This is to prevent mis-parsing hereafter
                p_ok = strstr(p_epbse, "OK");
                if(p_ok == NULL) {
                    LOGD(TAG "No OK found, try to read it out from buffer");
                    wait4_ack (fd, NULL, 500);
                }

                break;
            }
        }
        //[ALPS01679754]-end

        while(1){
            /* Set Band to GSM 900 */
            LOGD(TAG "\n");
            send_at (fd, "AT+EPBSE=2\r\n");
            LOGD(TAG "Send AT+EPBASE=2 to set Band GSM 900, retryCount=%d \n",retryCount);
            if (wait4_ack (fd, NULL, 3000)){
                if(retryCount == 5){
                    LOGD(TAG "EPBSE fail too many times\n");
                    goto err;
                }else{
                    if(retryCount == 0){
                        /* ALPS01432749 : to backward compatilbe for old modem. we need to activate modem first */
                        LOGD(TAG "[AT]Try to activate modem first:\n");
                        retryCount++;
                        send_at (fd, "AT+CFUN=4\r\n");
                        if (wait4_ack (fd, NULL, 3000))goto err;
                    }

                    retryCount++;
                    usleep(HALT_TIME);
                }
            }else{
               LOGD(TAG "AT+EPBSE=2 got OK\n");
                break;
            }
        }

        /* Check if band setting can take effect without rebooting modem. Dynamic band setting is only supported in newer version modem */
        send_at (fd, "AT+CESQ=?\r\n");
        LOGD(TAG "Send AT+CESQ=? to check if it's newer version modem \n");
        if (wait4_ack (fd, NULL, 5000)!= -1){
            LOGD(TAG "AT+CESQ=? got OK, set dynamicBandSupport \n");
            dynamicBandSupport = 1;
        }

        if(dynamicBandSupport == 0){
        /* Reboot modem to make new band setting work */
        send_at (fd, "AT+EPON\r\n");
        LOGD(TAG "Send AT+EPON to reboot modem \n");
        if (wait4_ack (fd, NULL, 5000))goto err;


        /* Wait modem ready URC +EIND:128 */
        LOGD(TAG "AT+EPON OK ,Wait modem ready +EIND:128 \n");
        LOGD(TAG "[AT]Sleep:\n");
        usleep(10 * HALT_TIME);

        /* Reopen ccci FD. otherwise cannot access CCCI normally after rebooting modem */
        close(fd);
        fd = open(DEVICE_NAME, O_RDWR);
        if(fd < 0) {
            LOGD(TAG "Fail to open %s: %s\n",DEVICE_NAME, strerror(errno));
            goto err;
        }
        LOGD(TAG "%s has been reopened...\n",DEVICE_NAME);

        retryCount = 0;

        while(retryCount < 100){
            char *p = NULL;

            memset(rsp_buf, 0, sizeof(rsp_buf));
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------Wait EIND URC start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------Wait ENID URC end------\n");

            retryCount++;
            p = strstr(rsp_buf, "+EIND:");

            if(p!= NULL){
                LOGD(TAG "+EIND:128 detected\n");
                break;
            }
            usleep(HALT_TIME);
        }

        LOGD(TAG "[AT]Disable Sleep Mode:\n");
        send_at (fd, "AT+ESLP=0\r\n");
        if (wait4_ack (fd, NULL, 3000))goto err;

        }

        //[ALPS01723693]-start: avoide AT+ERAT return ERROR
        /* To start SIM*/
        LOGD(TAG "Send AT+CFUN=4 to start SIM \n");
        send_at (fd, "AT+CFUN=4\r\n");
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+CFUN=4 OK ,Start to turn on RF\n");
        //[ALPS01723693]-end

        /* To trun on RF . AT+CFUN=1 can be used for single or mulitple SIM project */
        LOGD(TAG "Send AT+CFUN=1 to turn on RF \n");
        send_at (fd, "AT+CFUN=1\r\n");
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+CFUN=1 OK ,Start to test RF \n");

        /* To set GSM only modem. To make sure GSM 900 work */
        LOGD(TAG "Send AT+ERAT=0 to set GSM only \n");
        send_at (fd, "AT+ERAT=0\r\n");
        if (wait4_ack (fd, NULL, 5000))goto err;
        LOGD(TAG "AT+ERAT=0 OK ,Start to test RF \n");

        /* Start RF test */
        retryCount = 0;

        while(retryCount < 100){
            const char *tok = "+ECSQ";
            const char *minus_tok = "-";
            char *p = NULL;
            char *minus_p = NULL;
            rssi_level = 0;

            /* Check RF RSSI level. The verdict of RF test is the RSSI level shall be greater than -80dbm  */
            LOGD(TAG "\n");
            usleep(HALT_TIME);

            memset(cmd_buf, 0, sizeof(cmd_buf));
            strncpy(cmd_buf, "AT+ECSQ\r\n", BUF_SIZE-1);
            write(fd, cmd_buf, strlen(cmd_buf));

            LOGD(TAG "Send AT+ECSQ to check RF, retryCount=%d \n",retryCount);

            memset(rsp_buf, 0, sizeof(rsp_buf));
            read(fd, rsp_buf, BUF_SIZE);
            LOGD(TAG "------AT+ECSQ start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+ECSQ end------\n");
            retryCount++;
            p = strstr(rsp_buf, tok);

            /* ex: +ECSQ: 26, 99, -241 */
            if(p!= NULL){
                LOGD(TAG "p=%s\n", p);
                minus_p = strstr(p, minus_tok);
                if(minus_p != NULL){
                    /* (qdbm value) /4 = dbm value */
                    LOGD(TAG "rssi_level str =%s\n", minus_p);
                    rssi_level = atoi(minus_p) / 4;
                }
            }
            return_data.rf.rf_data = rssi_level ;
            /* AT+ECSQ might got null immeidate response or rssi_in_qdbm=1 ,which means modem did not have any measurement result yet. keep retry polling */
            LOGD(TAG "mGsmRfRssidbm=%d, rssi_level=%d\n",mGsmRfRssidbm, rssi_level);
            if(rssi_level > mGsmRfRssidbm && rssi_level!=0){
                ret = 1;
                LOGD(TAG "rssi_level pass in RF test");
                break;
            }else if(rssi_level <= mGsmRfRssidbm){
                ret = 0;
                LOGD(TAG "rssi_level fail in RF test");
                break;
            }
        }

        //[ALPS01679754]-start:restore band mode settings
        while(isBackup){
            LOGD(TAG "restore band setting");
            const char tok_end[] = "\r\n";
            memset(cmd_buf, 0, sizeof(cmd_buf));
            strncpy(cmd_buf, "AT+EPBSE=", BUF_SIZE-1);
            strncat(cmd_buf, bandSetting, BUF_SIZE-strlen(cmd_buf)-1);
            strncat(cmd_buf, tok_end, BUF_SIZE-strlen(cmd_buf)-1);
            LOGD(TAG "Send %s\n",cmd_buf);
            send_at(fd, cmd_buf);
            if (wait4_ack (fd, NULL, 5000)) {
                isBackup = false;
                goto err;
            } else {
                isBackup = false;
                LOGD(TAG "restore OK");
                break;
            }
        }
        //[ALPS01679754]-end

        if(ret) {
            LOGD(TAG "RF Test result pass\n");
            len = 0;
            len += sprintf(rf->info + len,
                "%s: %s. \n", uistr_rf_test, uistr_info_pass);
            len += sprintf(rf->info + len,
                "Verdict RSSI: %d dbm\r\nReceived RSSI: %d dbm\n", mGsmRfRssidbm,rssi_level);
            close(fd);
            fd = -1;

            LOGD(TAG "%s: Exit\n", __FUNCTION__);
            iv->redraw(iv);
            usleep(HALT_TIME * 30);

            return NULL;
        } else {
            LOGD(TAG "RF Test result fail\n");
            goto err;
        }

        LOGD(TAG "redraw\n");
        iv->redraw(iv);
    } // end if(!sim->test_done)

err:
    LOGD(TAG "%s: FAIL\n",__FUNCTION__);

    //[ALPS01723693]-start:error handling
    while(isBackup){
        LOGD(TAG "err:restore band setting");
        const char tok_end[] = "\r\n";
        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+EPBSE=", BUF_SIZE-1);
        strncat(cmd_buf, bandSetting, BUF_SIZE-strlen(cmd_buf)-1);
        strncat(cmd_buf, tok_end, BUF_SIZE-strlen(cmd_buf)-1);
        LOGD(TAG "err:Send %s\n",cmd_buf);
        send_at(fd, cmd_buf);
        if (wait4_ack (fd, NULL, 5000)) {
            LOGD(TAG "err:restore fail");
        } else {
            LOGD(TAG "err:restore OK");
            break;
        }
    }
    //[ALPS01723693]-end

    len = 0;
    len += sprintf(rf->info + len,
            "%s: %s. \n", uistr_rf_test, uistr_info_fail);
    len += sprintf(rf->info + len,
            "Verdict RSSI: %d dbm\r\nReceived RSSI: %d dbm\n", mGsmRfRssidbm,rssi_level);

    close(fd);
    fd = -1;

    LOGD(TAG "redraw\n");
    iv->redraw(iv);
    usleep(HALT_TIME * 30);
    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return NULL;
}

int rf_test_entry(struct ftm_param *param, void *priv)
{
  int  passCount = 0;
  struct rf_factory *rf = (struct rf_factory*)priv;
  struct itemview *iv = NULL;
  char isLteSupport[PROPERTY_VALUE_MAX] = {0};

  LOGD(TAG "%s: Start\n", __FUNCTION__);
  strncpy(rf->info,"", 1024);
  init_text(&rf->title, param->name, COLOR_YELLOW);
  init_text(&rf->text, &rf->info[0], COLOR_YELLOW);

  if(NULL == rf->iv) {
    iv = ui_new_itemview();
    if(!iv) {
      LOGD(TAG "No memory for item view");
      return -1;
    }
    rf->iv = iv;
  }
  iv = rf->iv;
  iv->set_title(iv, &rf->title);
  iv->set_items(iv, rf_items, 0);
  iv->set_text(iv, &rf->text);
  iv->start_menu(iv,0);
  iv->redraw(iv);

  rf->exit_thread = false;

  property_get("ro.vendor.mtk_lte_support", isLteSupport, "");
  if (strcmp(isLteSupport, "1") == 0) {
      pthread_create(&rf->update_thread, NULL, lte_rf_update_thread, priv);
  } else {
      pthread_create(&rf->update_thread, NULL, rf_update_thread, priv);
  }

  strncpy(rf->info, "", 1024);
  rf->test_done = false;
  while (strlen(rf->info) == 0) {
      usleep(200000);
      if (strstr(rf->info, uistr_info_pass)) {
          passCount++;
      }
  }
  LOGD(TAG "passCount = %d\n", passCount);

  //Exit RF Test thread
  rf->exit_thread = true;
  rf->test_done = true;

  pthread_join(rf->update_thread, NULL);

  //Check test result
  if (passCount == 1) {
     rf->mod->test_result = FTM_TEST_PASS;
  } else {
     rf->mod->test_result = FTM_TEST_FAIL;
  }

  LOGD(TAG "%s: End\n", __FUNCTION__);

  return 0;
}

int rf_test_init(void)
{
  int ret = 0;
  struct ftm_module *mod;
  struct rf_factory *rf;

  LOGD(TAG "%s: Start\n", __FUNCTION__);

  mod = ftm_alloc(ITEM_RF_TEST, sizeof(struct rf_factory));
  if(!mod) {
    return -ENOMEM;
  }
  rf = mod_to_rf(mod);
  rf->mod = mod;
  rf->test_done = true;

  ret = ftm_register(mod, rf_test_entry, (void*)rf);
  if(ret) {
    LOGD(TAG "register rf_test_entry failed (%d)\n", ret);
  }

  return ret;
}


#endif
