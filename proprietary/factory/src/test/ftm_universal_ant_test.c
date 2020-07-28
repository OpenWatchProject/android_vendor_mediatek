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

#ifdef FACTORY_C2K_SUPPORT
#ifndef MTK_ECCCI_C2K
#include <c2kutils.h>
#endif
#endif

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include "hardware/ccci_intf.h"

extern int send_at(const int fd, const char *pCMD);
extern int wait4_ack(const int fd, char *pACK, int timeout);
extern int openDeviceWithDeviceName(char *deviceName);
extern bool is_support_modem(int modem);

#define MAX_MODEM_INDEX 4
#define DEVICE_NAME ccci_get_node_name(USR_FACTORY_RF, MD_SYS1)
#define HALT_INTERVAL 20000
#define BUF_SIZE_EXT 128

extern pthread_mutex_t ccci_mutex;

#if defined(FEATURE_FTM_UNIVERSAL_ANT)

#define TAG    "[UNIVERSAL ANT] "

static void *universal_ant_update_thread(void *priv);
static int universal_ant_test_entry(struct ftm_param *param, void *priv);
static int get_ccci_path(int modem_index,char * path);
#ifdef FACTORY_C2K_SUPPORT
#ifndef EVDO_FTM_DT_VIA_SUPPORT
static int wait4_ack_with_retry_error(const int fd, char *pACK, char *pRetryError, int timeout);
#endif // EVDO_FTM_DT_VIA_SUPPORT
#endif // FACTORY_C2K_SUPPORT

static void *RemoveWarning( void *a ) { return a; }
#define UNUSED_PARM(a) RemoveWarning((void *)&(a));

int gOneModem = -1;

extern sp_ata_data return_data;
static item_t universal_ant_items[] = {
    item(-1, NULL),
};

struct universal_ant_factory {
    char info[1024];
    text_t title;
    text_t text;
    struct ftm_module *mod;
    struct itemview *iv;
    pthread_t update_thread;
    bool exit_thread;
    bool test_done;
    bool test_result;
};

typedef enum {
    ANT_RAT_LTE = 0,
    ANT_RAT_WCDMA,
    ANT_RAT_TDSCDMA,
    ANT_RAT_GSM,
    ANT_RAT_C2K,
    ANT_RAT_NR
} ftm_universal_ant_rat;

static const char *RatToString(ftm_universal_ant_rat rat) {
    switch(rat) {
        case ANT_RAT_NR : return "RAT_NR";
        case ANT_RAT_LTE : return "RAT_LTE";
        case ANT_RAT_WCDMA: return "RAT_WCDMA";
        case ANT_RAT_TDSCDMA: return "RAT_TDSCDMA";
        case ANT_RAT_GSM: return "RAT_GSM";
        case ANT_RAT_C2K: return "RAT_C2K";
        default: return "<unknown Rat>";
    }
}

#define mod_to_ant(p)  (struct universal_ant_factory*)((char*)(p) + sizeof(struct ftm_module))

static int tok_start(char **p_cur) {
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

static void skipWhiteSpace(char **p_cur) {
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && isspace(**p_cur)) {
        (*p_cur)++;
    }
}

static void skipNextComma(char **p_cur) {
    if (*p_cur == NULL) return;

    while (**p_cur != '\0' && **p_cur != ',') {
        (*p_cur)++;
    }

    if (**p_cur == ',') {
        (*p_cur)++;
    }
}

static char * nextTok(char **p_cur) {
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

static int at_tok_nextint_base(char **p_cur, int *p_out, int base, int  uns) {
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

int at_tok_nextstr(char **p_cur, char **p_out)
{
    if (*p_cur == NULL) {
        return -1;
    }

    *p_out = nextTok(p_cur);

    return 0;
}

/**
 * Parses the next base 10 integer in the AT response line
 * and places it in *p_out
 * returns 0 on success and -1 on fail
 * updates *p_cur
 */
static int at_tok_nextint(char **p_cur, int *p_out) {
    return at_tok_nextint_base(p_cur, p_out, 10, 0);
}

/** returns 1 on "has more tokens" and 0 if no */
static int at_tok_hasmore(char **p_cur)
{
    return ! (*p_cur == NULL || **p_cur == '\0');
}

static int read_preferred_para(const char *pstr,int default_value) {
    int value = default_value;
    char *pVal = NULL;

    if(pstr== NULL) {
        LOGD(TAG "preferred_receiver_para error %s", pstr);
        return 0;
    }
    pVal = ftm_get_prop(pstr);
    if (pVal != NULL) {
        value = (int)atoi(pVal);
        LOGD(TAG "preferred_receiver_para %s- %d \n",pstr,value);
    }
    else {
        LOGD(TAG "preferred_receiver_para can't get %s , use default %d\n", pstr, value);
    }
    return value;
}

static int read_preferred_para_index(const char *pstr1, const char *pstr2, int index, int default_value) {
    int value = default_value;
    char *pVal = NULL;
    char path[128];
    memset(path, 0, sizeof(path));

    if(pstr1 == NULL || pstr2 == NULL) {
        LOGD(TAG "preferred_receiver_para error %s", pstr1);
        return 0;
    }
    if (index > 0) {
        sprintf(path,"%s%d.%s", pstr1, index, pstr2);
    } else {
        sprintf(path,"%s.%s", pstr1, pstr2);
    }
    pVal = ftm_get_prop(path);
    if (pVal != NULL) {
        value = (int)atoi(pVal);
        LOGD(TAG "preferred_receiver_para %s- %d \n", path, value);
    }
    else {
        LOGD(TAG "preferred_receiver_para can't get %s , use default %d\n", path, value);
    }
    return value;
}

static int format_test_verdict(char *info, int *infoLen, int rat, bool rssi1_needed,
        int band, int ch, int mRfRssi0dbm, int mRfRssi1dbm, int rssi0_dbm, int rssi1_dbm, bool is_pass, int index) {

    int ret = 0;
    int len = *infoLen;
    const char *verdict = is_pass ? uistr_info_pass : uistr_info_fail;

    if (rssi1_needed) {
        switch (rat) {
            case ANT_RAT_LTE:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d, \nReceived rssi0_dbm=%d, rssi1_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm);
                break;
            }
            case ANT_RAT_WCDMA:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nCH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d, \nReceived rssi0_dbm=%d, rssi1_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        ch,
                        mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm);
                break;
            }
            case ANT_RAT_TDSCDMA:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nCH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d, \nReceived rssi0_dbm=%d, rssi1_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        ch,
                        mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm);
                break;
            }
            case ANT_RAT_GSM:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d, \nReceived rssi0_dbm=%d, rssi1_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm);
                break;
            }
            case ANT_RAT_C2K:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d, \nReceived rssi0_dbm=%d, rssi1_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm);
                break;
            }
            default:
            {
                ret = -1;
                break;
            }
        }
    } else {
        switch (rat) {
            case ANT_RAT_LTE:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, \nReceived rssi0_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, rssi0_dbm);
                break;
            }
            case ANT_RAT_WCDMA:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nCH=%d\nVerdict rssi0_dbm=%d, \nReceived rssi0_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        ch,
                        mRfRssi0dbm, rssi0_dbm);
                break;
            }
            case ANT_RAT_TDSCDMA:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nCH=%d\nVerdict rssi0_dbm=%d, \nReceived rssi0_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        ch,
                        mRfRssi0dbm, rssi0_dbm);
                break;
            }
            case ANT_RAT_GSM:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, \nReceived rssi0_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, rssi0_dbm);
                break;
            }
            case ANT_RAT_C2K:
            {
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, \nReceived rssi0_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, verdict,
                        band, ch,
                        mRfRssi0dbm, rssi0_dbm);
                break;
            }
            default:
            {
                ret = -1;
                break;
            }
        }
    }

    *infoLen = len;
    return ret;
}

int checkOneModem() {
    int fd = -1;
    char atDevPath1[32] = {0};

    if (get_ccci_path(0, atDevPath1) == 0) {
        LOGD(TAG "Can't get CCCI path!");
        goto err;
    }
    fd = openDeviceWithDeviceName(atDevPath1);
    if (fd < 0) {
        LOGD(TAG "Fail to open fd\n");
        goto err;
    }
    LOGD(TAG "OK to open fd\n");
    for (int i = 0; i < 60; i++) usleep(50000); //sleep 3s wait for modem bootup

    LOGD(TAG "[AT]AT polling first:\n");
    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 300));

    LOGD(TAG "[AT]Check One Modem:\n");
    send_at(fd, "AT+EMDVER?\r\n");
    if (wait4_ack(fd, "\"LWCTG one MD\",\"1\"", 3000)) {
        gOneModem = 0;
    } else {
        gOneModem = 1;
    }

err:
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;
    for (int i = 0; i < 30; i++) usleep(50000); //sleep 1.5s
    LOGD(TAG "Check one Modem=%d\n", gOneModem);
    return gOneModem;
}

//display info & info length
//return 0/1 indicate test fail or success
static int gsm_ant_test(char *info, int *infoLen, ftm_universal_ant_rat rat, int index) {
    //Verdict RSSI
    int mRfRssi0dbm=0;
    int mRfRssi1dbm=0;

    //Received RSSI
    int rssi0_dbm = 0;
    int rssi1_dbm = 0;

    int fd = -1;

    int ret = 0;
    int tryCount = 0;
    int tryCountMax = read_preferred_para("RF.rssi.read.count", 100);
    if (tryCountMax < 10) {
        tryCountMax = 10;
    } else if (tryCountMax > 500) {
        tryCountMax = 500;
    }
    const int HALT_TIME = 200000;//0.2s

    LOGD(TAG "%s(%s): Start\n", __FUNCTION__, RatToString(rat));

    // band
    int mLteRfBand = 38;
    int mGsmRfBand = 1;
    // channel
    int mLteRfDlEarfcn = 0;
    int mWcdmaRfUarfcn = 0;
    int mTdscdmaRfUarfcn = 0;
    int mGsmRfArfcn = 0;

    char path[128];
    memset(path, 0, sizeof(path));

    if (ANT_RAT_LTE == rat) {
        mLteRfBand = read_preferred_para_index("LTE", "RF.band", index, 38);
    }

    if (ANT_RAT_GSM == rat) {
        mGsmRfBand = read_preferred_para_index("GSM", "RF.band", index, 0);
    }

    //at command and response buf
    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];

    char *pVal = NULL;
    bool rssi1_needed = false;
    bool needLeaveMetaMode = false;//wcdma used
    bool needLeaveLtgMode = false;//tdscdma used

    if (!((rat >= ANT_RAT_LTE) && (rat <= ANT_RAT_GSM))) {
        LOGD(TAG "%s: not support", RatToString(rat));
        goto err;
    }

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n", DEVICE_NAME, strerror(errno));
        goto err;
    }
    LOGD(TAG "%s has been opened...\n", DEVICE_NAME);

    LOGD(TAG "[AT]AT polling first:\n");
    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 300));

    LOGD(TAG "[AT]Disable Sleep Mode:\n");
    send_at(fd, "AT+ESLP=0\r\n");
    if (wait4_ack(fd, NULL, 3000)) goto err;

    /* For tdd 3G*/
    if (ANT_RAT_TDSCDMA == rat || ANT_RAT_GSM == rat || ANT_RAT_WCDMA == rat) {
        LOGD(TAG "Send AT+CFUN=1\n");
        send_at(fd, "AT+CFUN=1\r\n");
        if (wait4_ack(fd, NULL, 5000)) goto err;
        LOGD(TAG "AT+CFUN=1 OK\n");
    }

    //set rat mode
    switch (rat) {
        case ANT_RAT_LTE:
        {
            LOGD(TAG "Send AT+ERAT=6,4\n");
            send_at(fd, "AT+ERAT=6,4\r\n");
            if (wait4_ack(fd, NULL, 5000)) goto err;
            LOGD(TAG "AT+ERAT=6,4 OK\n");
            break;
        }
        case ANT_RAT_WCDMA:
        {
            // for Wcdma, switch to FDD first, such that ERAT can be set to UL1
            /* Assuming lwg before each RAT test, so skip switching to FDD again
            LOGD(TAG "Send AT+ECSRA=2,1,0,1,1,0\n");
            send_at(fd, "AT+ECSRA=2,1,0,1,1,0\r\n");
            if (wait4_ack(fd, NULL, 5000)) {
                goto err;
            }
            */

            LOGD(TAG "Send AT+ERAT=1\n");
            send_at(fd, "AT+ERAT=1\r\n");
            if (wait4_ack(fd, NULL, 5000)) goto err;
            LOGD(TAG "AT+ERAT=1 OK\n");
            break;
        }
        case ANT_RAT_TDSCDMA:
        {
            // for Tds, switch to TDD first, such that ERAT can be set to TL1
            LOGD(TAG "Send AT+ECSRA=2,0,1,1,1,0\n");
            send_at(fd, "AT+ECSRA=2,0,1,1,1,0\r\n");
            if (wait4_ack(fd, NULL, 5000)) {
                goto err;
            } else {
                needLeaveLtgMode = true;
            }

            LOGD(TAG "Send AT+ERAT=1\n");
            send_at(fd, "AT+ERAT=1\r\n");
            if (wait4_ack(fd, NULL, 5000)) goto err;
            LOGD(TAG "AT+ERAT=1 OK\n");
            break;
        }
        case ANT_RAT_GSM:
        {
            LOGD(TAG "Send AT+ERAT=0\n");
            send_at(fd, "AT+ERAT=0\r\n");
            if (wait4_ack(fd, NULL, 5000)) goto err;
            LOGD(TAG "AT+ERAT=0 OK\n");
            break;
        }
        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            goto err;
            break;
        }
    }

    /* To start SIM*/
    LOGD(TAG "Send AT+CFUN=4 to startup modem \n");
    send_at(fd, "AT+CFUN=4\r\n");
    if (wait4_ack(fd, NULL, 5000)) goto err;
    LOGD(TAG "AT+CFUN=4 OK ,Start to set RSSI test\n");

    /* To check whether device support the input band */
    LOGD(TAG "Check modem supported RAT band\n");
    if (rat == ANT_RAT_LTE || rat == ANT_RAT_GSM) {
        tryCount = 0;
        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+EPBSE=?\r\n", (BUF_SIZE-1));
        send_at(fd, cmd_buf);
        LOGD(TAG "Send AT+EPBSE=?\n");

        while(tryCount++ < tryCountMax) {
            const char *tok_epbse = "+EPBSE: ";
            char *p_epbse = NULL;
            char *p_ok = NULL;

            memset(rsp_buf, 0, sizeof(rsp_buf));

            read(fd, rsp_buf, BUF_SIZE-1);
            LOGD(TAG "------AT+EPBSE=? start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+EPBSE=? end------\n");

            p_epbse = strstr(rsp_buf, tok_epbse);

            if(p_epbse!= NULL) {
                LOGD(TAG "p_epbse=%s\n", p_epbse);
                //example: p_epbse now is pointed to "+EPBSE: 154,33,0,480"
                tok_start(&p_epbse);

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

                //try to read out "OK" after +EPBSE: x,x in buffer if any.This is to prevent mis-parsing hereafter
                if (p_epbse != NULL) {
                    p_ok = strstr(p_epbse, "OK");
                }
                /*
                if(p_ok == NULL) {
                    LOGD(TAG "No OK found, try to read it out from buffer");
                    wait4_ack(fd, NULL, 500);
                }
                */
                if (rat == ANT_RAT_LTE) {
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
                } else { //gsm
                    if ((mGsmRfBand >= 1) && (mGsmRfBand <= 8)) {
                        if (!(gsm_band & (1 << (mGsmRfBand - 1)))) {
                            LOGD(TAG "Band (%d) not support", mGsmRfBand);
                            goto err;
                        }
                    }
                }
                break;
            }
        }
        if (tryCount > tryCountMax) goto err;
    } else if (rat == ANT_RAT_TDSCDMA || rat == ANT_RAT_WCDMA) {
        //check modem mode whether support lwg or ltg
        tryCount = 0;
        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+CSRA?\r\n", (BUF_SIZE-1));
        send_at(fd, cmd_buf);
        LOGD(TAG "Send AT+CSRA?\n");

        while (tryCount++ < tryCountMax) {
            const char *tok_csraa = "+CSRAA: ";
            char *p_csraa = NULL;
            char *p_ok = NULL;

            memset(rsp_buf, 0, sizeof(rsp_buf));

            read(fd, rsp_buf, BUF_SIZE-1);
            LOGD(TAG "------AT+CSRA? start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+CSRA? end------\n");

            p_csraa = strstr(rsp_buf, tok_csraa);

            if (p_csraa != NULL) {
                LOGD(TAG "p_csraa=%s\n", p_csraa);
                //example: p_csraa now is pointed to "+CSRAA: 1,1,0,0,0,1,1"
                tok_start(&p_csraa);

                int skip;
                int utranFdd;
                int utranTddLcr;
                at_tok_nextint(&p_csraa, &skip);
                at_tok_nextint(&p_csraa, &utranFdd);
                at_tok_nextint(&p_csraa, &utranTddLcr);
                //try to read out "OK" after +CSRAA: x,x in buffer if any.This is to prevent mis-parsing hereafter
                p_ok = strstr(p_csraa, "OK");
                if (p_ok == NULL) {
                    LOGD(TAG "No OK found, try to read it out from buffer");
                    wait4_ack(fd, NULL, 500);
                }
                if (!(((rat == ANT_RAT_WCDMA) && utranFdd) ||
                    ((rat == ANT_RAT_TDSCDMA) && utranTddLcr))) {
                    LOGD(TAG "modem mode not support, rat = %s, utranFdd=%d, utranTddLcr=%d", RatToString(rat), utranFdd, utranTddLcr);
                    goto err;
                }
                break;
            }
        }
        if (tryCount > tryCountMax) goto err;
    } else {
        LOGD(TAG "%s: not support", RatToString(rat));
        goto err;
    }

    /* To start test RSSI */
    switch (rat) {
        case ANT_RAT_LTE:
        {
            mLteRfDlEarfcn = read_preferred_para_index("LTE", "RF.dl.earfcn", index, 300);
            mRfRssi0dbm = read_preferred_para_index("LTE", "RF.rssi0.dbm", index, -75);
            pVal = NULL;
            if (index > 0) {
                sprintf(path, "LTE%d.RF.rssi1.dbm", index);
            } else {
                sprintf(path, "LTE.RF.rssi1.dbm");
            }
            pVal = ftm_get_prop(path);
            if (pVal != NULL) {
                mRfRssi1dbm = (int)atoi(pVal);
                rssi1_needed = true;
                LOGD(TAG "preferred_receiver_para LTE.RF.rssi1.dbm- %d \n", mRfRssi1dbm);
            } else {
                LOGD(TAG "preferred_receiver_para can't get LTE.RF.rssi1.dbm\n");
            }
            LOGD(TAG "Send AT+ERFTX=7,%d,%d to set test LTE RSSI \n", mLteRfBand, mLteRfDlEarfcn);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf,"AT+ERFTX=7,%d,%d\r\n", mLteRfBand, mLteRfDlEarfcn);
            send_at(fd, cmd_buf);
            if (wait4_ack(fd, NULL, 5000)) goto err;
            break;
        }

        case ANT_RAT_WCDMA:
        {
            mWcdmaRfUarfcn = read_preferred_para_index("WCDMA", "RF.uarfcn", index, 300);
            mRfRssi0dbm = read_preferred_para_index("WCDMA", "RF.rssi0.dbm", index, -75);
            pVal = NULL;
            if (index > 0) {
                sprintf(path, "WCDMA%d.RF.rssi1.dbm", index);
            } else {
                sprintf(path, "WCDMA.RF.rssi1.dbm");
            }
            pVal = ftm_get_prop(path);
            if (pVal != NULL) {
                mRfRssi1dbm = (int)atoi(pVal);
                rssi1_needed = true;
                LOGD(TAG "preferred_receiver_para WCDMA.RF.rssi1.dbm- %d \n", mRfRssi1dbm);
            } else {
                LOGD(TAG "preferred_receiver_para can't get WCDMA.RF.rssi1.dbm\n");
            }
            LOGD(TAG "Send AT+ERFTX=0,2,%d to set test WCDMA RSSI \n", mWcdmaRfUarfcn);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf,"AT+ERFTX=0,2,%d\r\n",mWcdmaRfUarfcn);
            send_at(fd, cmd_buf);
            needLeaveMetaMode = true;
            if (wait4_ack(fd, NULL, 5000)) goto err;
            break;
        }

        case ANT_RAT_TDSCDMA:
        {
            mTdscdmaRfUarfcn = read_preferred_para_index("TDSCDMA", "RF.uarfcn", index, 300);
            mRfRssi0dbm = read_preferred_para_index("TDSCDMA", "RF.rssi0.dbm", index, -75);
            pVal = NULL;
            if (index > 0) {
                sprintf(path, "TDSCDMA%d.RF.rssi1.dbm", index);
            } else {
                sprintf(path, "TDSCDMA.RF.rssi1.dbm");
            }
            pVal = ftm_get_prop(path);
            if (pVal != NULL) {
                mRfRssi1dbm = (int)atoi(pVal);
                rssi1_needed = true;
                LOGD(TAG "preferred_receiver_para TDSCDMA.RF.rssi1.dbm- %d \n", mRfRssi1dbm);
            } else {
                LOGD(TAG "preferred_receiver_para can't get TDSCDMA.RF.rssi1.dbm\n");
            }
            LOGD(TAG "Send AT+ERFTX=0,2,%d to set test TDSCDMA RSSI \n", mTdscdmaRfUarfcn);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf,"AT+ERFTX=0,2,%d\r\n",mTdscdmaRfUarfcn);
            send_at(fd, cmd_buf);
            if (wait4_ack(fd, NULL, 5000)) goto err;
            break;
        }

        case ANT_RAT_GSM:
        {
            mGsmRfArfcn = read_preferred_para_index("GSM", "RF.arfcn", index, 300);
            int mGsmRfGain = read_preferred_para_index("GSM", "RF.gain", index, 300);
            int mGsmRfPattern = read_preferred_para_index("GSM", "RF.pattern", index, 300);
            mRfRssi0dbm = read_preferred_para_index("GSM", "RF.rssi0.dbm", index, -75);
            pVal = NULL;
            if (index > 0) {
                sprintf(path, "GSM%d.RF.rssi1.dbm", index);
            } else {
                sprintf(path, "GSM.RF.rssi1.dbm");
            }
            pVal = ftm_get_prop(path);
            if (pVal != NULL) {
                mRfRssi1dbm = (int)atoi(pVal);
                rssi1_needed = true;
                LOGD(TAG "preferred_receiver_para GSM.RF.rssi1.dbm- %d \n", mRfRssi1dbm);
            } else {
                LOGD(TAG "preferred_receiver_para can't get GSM.RF.rssi1.dbm\n");
            }
            LOGD(TAG "Send AT+ERFTX=2,2,%d,%d,%d,%d to set test GSM RSSI \n", mGsmRfArfcn, mGsmRfGain, mGsmRfBand, mGsmRfPattern);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf,"AT+ERFTX=2,2,%d,%d,%d,%d\r\n", mGsmRfArfcn, mGsmRfGain, mGsmRfBand, mGsmRfPattern);
            send_at(fd, cmd_buf);
            if (wait4_ack(fd, NULL, 5000)) goto err;
            break;
        }

        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            goto err;
            break;
        }
    }
    LOGD(TAG "%s OK ,Start to turn on RF\n",cmd_buf);

    /* Wait URC +ERSSI: <rssi0_dBm>, <rssi1_dBm> for verification */
    tryCount = 0;

    while (tryCount++ < tryCountMax) {
        char *p = NULL;
        rssi0_dbm = 0;
        rssi1_dbm = 0;

        memset(rsp_buf, 0, sizeof(rsp_buf));
        read(fd, rsp_buf, BUF_SIZE-1);
        LOGD(TAG "------Wait ERSSI URC start------\n");
        LOGD(TAG "%s\n", rsp_buf);
        LOGD(TAG "------Wait ERSSI URC end------\n");

        p = strstr(rsp_buf, "+ERSSI:");

        if (p != NULL) {
            //example: p now is pointed to "+ERSSI: 7,-80,-96"
            tok_start(&p);

            //act
            int skip;
            at_tok_nextint(&p, &skip);

            //parse out "-80" and assigned to rssi0_dbm
            at_tok_nextint(&p, &rssi0_dbm);
            rssi0_dbm /= 8;

            //eamplex: p now is pointed to "-96"
            at_tok_nextint(&p, &rssi1_dbm);
            rssi1_dbm /= 8;

            //<rssi0_dbm>,<rssi1_dbm> shall be negative integer value and -60 dbm is a stronger signal than -80


            if ((rssi0_dbm < 0) && (rssi0_dbm > mRfRssi0dbm)) {
                ret = 1;
                if (rssi1_needed) {
                    if ((rssi1_dbm < 0) && (rssi1_dbm > mRfRssi1dbm)) {
                        ret = 1;
                    } else {
                        ret = 0;
                    }
                }
            } else {
                ret = 0;
            }

            if(ret == 1) {
                LOGD(TAG "recv rssi0_dbm=%d ,rssi1_dbm=%d, rssi1_needed=%d, pass in RF test",rssi0_dbm,rssi1_dbm,rssi1_needed);
                break;
            } else {
                LOGD(TAG "recv rssi0_dbm=%d ,rssi1_dbm=%d, rssi1_needed=%d, fail in RF test",rssi0_dbm,rssi1_dbm,rssi1_needed);
                break;
            }
        }
        LOGD(TAG "------sleep %d us------\n", HALT_TIME);
        usleep(HALT_TIME);
        LOGD(TAG "------wake up ------\n");
    }
    if (tryCount > tryCountMax) goto err;

    //return_data.rf.rf_data = 0-rssi0_dbm;
    //return_data.rf.rf_data_lte = 0-rssi1_dbm;

    if (ret) {
        LOGD(TAG "RF Test(%s) result pass\n", RatToString(rat));

        bool is_pass = true;
        switch (rat) {
            case ANT_RAT_LTE:
            {
                format_test_verdict(info, infoLen, rat, rssi1_needed,
                        mLteRfBand, mLteRfDlEarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
                break;
            }
            case ANT_RAT_WCDMA:
            {
                format_test_verdict(info, infoLen, rat, rssi1_needed,
                        -1, mWcdmaRfUarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
                break;
            }
            case ANT_RAT_TDSCDMA:
            {
                format_test_verdict(info, infoLen, rat, rssi1_needed,
                        -1, mTdscdmaRfUarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
                break;
            }
            case ANT_RAT_GSM:
            {
                format_test_verdict(info, infoLen, rat, rssi1_needed,
                        mGsmRfBand, mGsmRfArfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
                break;
            }
            default:
            {
                LOGD(TAG "%s: not support", RatToString(rat));
                goto err;
                break;
            }
        }

        //reset ltg to lwg mode
        if (needLeaveLtgMode) {
            LOGD(TAG "Switch back to LWG mode, send AT+ECSRA=2,1,0,1,1,0\n");
            send_at(fd, "AT+ECSRA=2,1,0,1,1,0\r\n");
            wait4_ack(fd, NULL, 5000);
        }

        //reset modem to enter normal mode
        if (needLeaveMetaMode) {
            needLeaveMetaMode = false;
            LOGD(TAG "Send AT+CFUN=1,1\n");
            send_at(fd, "AT+CFUN=1,1\r\n");
            if (gOneModem == 1) {
                usleep(5000000 * 2);//wait for modem reset done
            }
            //wait4_ack(fd, NULL, 5000);
            LOGD(TAG "AT+CFUN=1,1 OK\n");
            if (fd >= 0) {
                close(fd);
            }
            fd = -1;
            usleep(5000000 * 2);//wait for modem reset done
        }

        if (fd >= 0) {
            close(fd);
        }
        fd = -1;

        LOGD(TAG "%s(%s): Exit\n", __FUNCTION__, RatToString(rat));

        return ret;
    } else {
        LOGD(TAG "RF Test(%s) result fail\n", RatToString(rat));
        goto err;
    }

err:
    LOGD(TAG "%s(%s): FAIL\n",__FUNCTION__, RatToString(rat));

    bool is_pass = false;
    switch (rat) {
        case ANT_RAT_LTE:
        {
            format_test_verdict(info, infoLen, rat, rssi1_needed,
                    mLteRfBand, mLteRfDlEarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
            break;
        }
        case ANT_RAT_WCDMA:
        {
            format_test_verdict(info, infoLen, rat, rssi1_needed,
                    -1, mWcdmaRfUarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
            break;
        }
        case ANT_RAT_TDSCDMA:
        {
            format_test_verdict(info, infoLen, rat, rssi1_needed,
                    -1, mTdscdmaRfUarfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
            break;
        }
        case ANT_RAT_GSM:
        {
            format_test_verdict(info, infoLen, rat, rssi1_needed,
                    mGsmRfBand, mGsmRfArfcn, mRfRssi0dbm, mRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
            break;
        }
        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            break;
        }
    }

    //reset ltg to lwg mode
    if (needLeaveLtgMode) {
        LOGD(TAG "Switch back to LWG mode, send AT+ECSRA=2,1,0,1,1,0\n");
        send_at(fd, "AT+ECSRA=2,1,0,1,1,0\r\n");
        wait4_ack(fd, NULL, 5000);
    }

    //reset modem to enter normal mode
    if (needLeaveMetaMode) {
        needLeaveMetaMode = false;
        LOGD(TAG "Send AT+CFUN=1,1\n");
        send_at(fd, "AT+CFUN=1,1\r\n");
        if (gOneModem == 1) {
            usleep(5000000 * 2);//wait for modem reset done
        }
        //wait4_ack(fd, NULL, 5000);
        LOGD(TAG "AT+CFUN=1,1 OK\n");
        if (fd >= 0) {
            close(fd);
        }
        fd = -1;
        usleep(5000000 * 2);//wait for modem reset done
    }
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;

    LOGD(TAG "%s(%s): Exit\n", __FUNCTION__, RatToString(rat));
    return ret;
}

static int nr_ant_test(char *info, int *infoLen, ftm_universal_ant_rat rat, int index) {
    //Verdict RSSI
    int mRfRssi0dbm=0;
    int mRfRssi1dbm=0;
    int mRfRssi2dbm=0;
    int mRfRssi3dbm=0;

    //Received RSSI
    int rssi0_dbm = 0;
    int rssi1_dbm = 0;
    int rssi2_dbm = 0;
    int rssi3_dbm = 0;

    int fd = -1;

    int ret = 0;
    int tryCount = 0;
    int tryCountMax = read_preferred_para("RF.rssi.read.count", 100);
    if (tryCountMax < 10) {
        tryCountMax = 10;
    } else if (tryCountMax > 500) {
        tryCountMax = 500;
    }
    const int HALT_TIME = 200000;//0.2s

    LOGD(TAG "%s(%s): Start\n", __FUNCTION__, RatToString(rat));

    // band
    int mNrRfBand = read_preferred_para_index("NR", "RF.band", index, 0);
    // channel
	int mNrRfDlEarfcn = read_preferred_para_index("NR", "RF.dl.earfcn", index, 300);

    char path[128];
    memset(path, 0, sizeof(path));

    //at command and response buf
    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];

    bool rssi1_needed = false;
    if (rat != ANT_RAT_NR || mNrRfBand < 0) {
        LOGD(TAG "rat:%s or band::%d not support", RatToString(rat), mNrRfBand);
        goto err;
    }

    fd = open(DEVICE_NAME, O_RDWR);
    if (fd < 0) {
        LOGD(TAG "Fail to open %s: %s\n", DEVICE_NAME, strerror(errno));
        goto err;
    }
    LOGD(TAG "%s has been opened...\n", DEVICE_NAME);

    LOGD(TAG "[AT]AT polling first:\n");
    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 300));

    LOGD(TAG "[AT]Disable Sleep Mode:\n");
    send_at(fd, "AT+ESLP=0\r\n");
    if (wait4_ack(fd, NULL, 3000)) goto err;

    //set rat mode
    switch (rat) {
        case ANT_RAT_NR:
        {
            LOGD(TAG "Send AT+ERAT=15\n");
            send_at(fd, "AT+ERAT=15\r\n");
            if (wait4_ack(fd, NULL, 5000)) goto err;
            LOGD(TAG "AT+ERAT=15 OK\n");
            break;
        }
        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            goto err;
            break;
        }
    }

    /* To start SIM*/
    LOGD(TAG "Send AT+CFUN=4 to startup modem \n");
    send_at(fd, "AT+CFUN=4\r\n");
    if (wait4_ack(fd, NULL, 5000)) goto err;
    LOGD(TAG "AT+CFUN=4 OK ,Start to set RSSI test\n");

    /* To check whether device support the input band */
    LOGD(TAG "Check modem supported RAT band\n");
#if 0
    if (rat == ANT_RAT_NR) {
        tryCount = 0;
        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+EPBSEH=?\r\n", (BUF_SIZE-1));
        send_at(fd, cmd_buf);
        LOGD(TAG "Send AT+EPBSEH=?\n");

        while(tryCount++ < tryCountMax) {
            const char *tok_epbse = "+EPBSEH: ";
            char *p_epbse = NULL;
            char *p_ok = NULL;

            memset(rsp_buf, 0, sizeof(rsp_buf));
            read(fd, rsp_buf, BUF_SIZE-1);
            LOGD(TAG "------AT+EPBSEH=? start------\n");
            LOGD(TAG "%s\n", rsp_buf);
            LOGD(TAG "------AT+EPBSEH=? end------\n");

            p_epbse = strstr(rsp_buf, tok_epbse);

            if(p_epbse!= NULL) {
                LOGD(TAG "p_epbseh=%s\n", p_epbse);
                //example: p_epbse now is pointed to "+EPBSEH: "<gsm_band>","<umts_band>","<lte_band_64>","<nr_band_128>""
                tok_start(&p_epbse);
                char *gsm_band = NULL;
                char *umts_band = NULL;
                char *lte_band = NULL;
                char *nr_band = NULL;

                at_tok_nextstr(&p_epbse, &gsm_band);
                at_tok_nextstr(&p_epbse, &umts_band);
                at_tok_nextstr(&p_epbse, &lte_band);
                at_tok_nextstr(&p_epbse, &nr_band);

                //try to read out "OK" after +EPBSEH: x,x in buffer if any.This is to prevent mis-parsing hereafter
                if (p_epbse != NULL) {
                    p_ok = strstr(p_epbse, "OK");
                }
                if (rat == ANT_RAT_NR) {
                    int band = 0;
                    char p;
                    p = nr_band[(mNrRfDlEarfcn/4)];
                    band = atoi(&p);
                    LOGD(TAG "p=%c, band=%d, mNrRfDlEarfcn%4=%d, mNrRfBand=%d\n", p, band, (mNrRfDlEarfcn%4), mNrRfBand);
                    if (band & (mNrRfDlEarfcn%4) == 0) {
                        goto err;
                    }
                }
                break;
            }
        }
        if (tryCount > tryCountMax) goto err;
    } else {
        LOGD(TAG "%s: not support", RatToString(rat));
        goto err;
    }
#endif
    /* To start test RSSI */
    switch (rat) {
        case ANT_RAT_NR:
        {
            mRfRssi0dbm = read_preferred_para_index("NR", "RF.rssi0.dbm", index, -75);
            mRfRssi1dbm = read_preferred_para_index("NR", "RF.rssi1.dbm", index, -75);
            mRfRssi2dbm = read_preferred_para_index("NR", "RF.rssi2.dbm", index, -75);
            mRfRssi3dbm = read_preferred_para_index("NR", "RF.rssi3.dbm", index, -75);

            LOGD(TAG "preferred NR:: rssi0: %d rssi1: %d rssi2: %d rssi3: %d\n",
                    mRfRssi0dbm, mRfRssi1dbm, mRfRssi2dbm, mRfRssi3dbm);
            LOGD(TAG "Send AT+EGMC=1,\"NrRssi\",%d,%d to set test NR RSSI \n",
                    mNrRfBand, mNrRfDlEarfcn);
            memset(cmd_buf, 0, sizeof(cmd_buf));
            sprintf(cmd_buf,"AT+EGMC=1,\"NrRssi\",%d,%d\r\n", mNrRfBand, mNrRfDlEarfcn);
            send_at(fd, cmd_buf);
            break;
        }
        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            goto err;
            break;
        }
    }
    LOGD(TAG "%s OK ,Start to turn on RF\n",cmd_buf);

    /* Wait URC +ERSSI: <rssi0_dBm>, <rssi1_dBm> for verification */
    tryCount = 0;

    while (tryCount++ < tryCountMax) {
        char *p = NULL;
        rssi0_dbm = 0;
        rssi1_dbm = 0;
        rssi2_dbm = 0;
        rssi3_dbm = 0;

        memset(rsp_buf, 0, sizeof(rsp_buf));
        read(fd, rsp_buf, BUF_SIZE-1);
        LOGD(TAG "------Wait EGMC RESPONSE start------\n");
        LOGD(TAG "%s\n", rsp_buf);
        LOGD(TAG "------Wait EGMC RESPONSE end------\n");

        p = strstr(rsp_buf, "+EGMC:");

        if (p != NULL) {
            char *skipstr = NULL;
            //example: p now is pointed to "+EGMC: "NrRssi",[RXM RSSI],[RXD RSSI],[Slave RXM RSSI],[Slave RXD RSSI]
            tok_start(&p);
            //act
            at_tok_nextstr(&p, &skipstr);
            //parse out "-80" and assigned to rssi0_dbm
            at_tok_nextint(&p, &rssi0_dbm);
            //eamplex: p now is pointed to "-96"
            at_tok_nextint(&p, &rssi1_dbm);
            //eamplex: p now is pointed to "-96"
            at_tok_nextint(&p, &rssi2_dbm);
            //eamplex: p now is pointed to "-96"
            at_tok_nextint(&p, &rssi3_dbm);

            //<rssi0_dbm>,<rssi1_dbm> shall be negative integer value and -60 dbm is a stronger signal than -80
            if ((rssi0_dbm < 0) && (rssi0_dbm > mRfRssi0dbm) &&
                    (rssi1_dbm < 0) && (rssi1_dbm > mRfRssi1dbm) &&
                    (rssi2_dbm < 0) && (rssi2_dbm > mRfRssi2dbm) &&
                    (rssi3_dbm < 0) && (rssi3_dbm > mRfRssi3dbm)) {
                ret = 1;
            } else {
                ret = 0;
            }

            if(ret == 1) {
                LOGD(TAG "recv rssi0_dbm=%d ,rssi1_dbm=%d ,rssi2_dbm=%d ,rssi3_dbm=%dm rssi1_needed=%d, pass in RF test",
                        rssi0_dbm,rssi1_dbm,rssi2_dbm,rssi3_dbm,rssi1_needed);
                break;
            } else {
                LOGD(TAG "recv rssi0_dbm=%d ,rssi1_dbm=%d ,rssi2_dbm=%d ,rssi3_dbm=%d, rssi1_needed=%d, fail in RF test",
                        rssi0_dbm,rssi1_dbm,rssi2_dbm,rssi3_dbm,rssi1_needed);
                break;
            }
        }
        LOGD(TAG "------sleep %d us------\n", HALT_TIME);
        usleep(HALT_TIME);
        LOGD(TAG "------wake up ------\n");
    }
    if (tryCount > tryCountMax) goto err;

    if (ret) {
        LOGD(TAG "RF Test(%s) result pass\n", RatToString(rat));
        switch (rat) {
            case ANT_RAT_NR:
            {
                int len = *infoLen;
                len += sprintf(info + len,
                        "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d,rssi2_dbm=%d, rssi3_dbm=%d,\nReceived rssi0_dbm=%d, rssi1_dbm=%d, rssi2_dbm=%d, rssi3_dbm=%d\n",
                        uistr_ant_test, RatToString(rat), index, uistr_info_pass,
                        mNrRfBand, mNrRfDlEarfcn, mRfRssi0dbm, mRfRssi1dbm, mRfRssi2dbm, mRfRssi3dbm,
                        rssi0_dbm, rssi1_dbm, rssi2_dbm, rssi3_dbm);
                *infoLen = len;
                break;
            }
            default:
            {
                LOGD(TAG "%s: not support", RatToString(rat));
                goto err;
                break;
            }
        }

        if (fd >= 0) {
            close(fd);
        }
        fd = -1;

        LOGD(TAG "%s(%s): Exit\n", __FUNCTION__, RatToString(rat));
        return ret;
    } else {
        LOGD(TAG "RF Test(%s) result fail\n", RatToString(rat));
        goto err;
    }

err:
    LOGD(TAG "%s(%s): FAIL\n",__FUNCTION__, RatToString(rat));
    switch (rat) {
        case ANT_RAT_NR:
        {
            int len = *infoLen;
            len += sprintf(info + len,
                    "%s(%s-%d): %s. \nBAND=%d,CH=%d\nVerdict rssi0_dbm=%d, rssi1_dbm=%d,rssi2_dbm=%d, rssi3_dbm=%d,\nReceived rssi0_dbm=%d, rssi1_dbm=%d, rssi2_dbm=%d, rssi3_dbm=%d\n",
                    uistr_ant_test, RatToString(rat), index, uistr_info_fail,
                    mNrRfBand, mNrRfDlEarfcn, mRfRssi0dbm, mRfRssi1dbm, mRfRssi2dbm, mRfRssi3dbm,
                    rssi0_dbm, rssi1_dbm, rssi2_dbm, rssi3_dbm);
            *infoLen = len;
            break;
        }
        default:
        {
            LOGD(TAG "%s: not support", RatToString(rat));
            break;
        }
    }
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;

    LOGD(TAG "%s(%s): Exit\n", __FUNCTION__, RatToString(rat));
    return ret;
}

//display info & info length
//return 0/1 indicate test fail or success
static int c2k_ant_test(char *info, int *infoLen, int index) {
if (isC2kSupport() == 1) {
#ifdef FACTORY_C2K_SUPPORT
#ifndef EVDO_FTM_DT_VIA_SUPPORT
    //Verdict RSSI
    int mC2kRfRssi0dbm = 0;
    int mC2kRfRssi1dbm = 0;

    //Received RSSI
    int rssi0_dbm = 0;
    int rssi1_dbm = 0;

    // Received asu value & related subsystem
    int subSystem = 0;
    int rssi0 = 0;
    int rssi1 = 0xFF;

    int mC2kRfSubsystem = 0;
    int mC2kRfBand = 0;
    int mC2kRfChannel = 0;

    int fd1 = -1;
    int fd3 = -1;

    int ret = 0;
    int len = *infoLen;
    int tryCount = 0;
    int tryCountMax = read_preferred_para("RF.rssi.read.count", 100);
    if (tryCountMax < 10) {
        tryCountMax = 10;
    } else if (tryCountMax > 500) {
        tryCountMax = 500;
    }
    int i = 0;
    const int HALT_TIME = 200000;//0.2s

    //at command and response buf
    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];

    char atDevPath1[32] = {0};
#ifdef MTK_ECCCI_C2K
    char atDevPath3[32] = {0} ;
#else
    char *atDevPath3 = NULL;
#endif


    char *pVal = NULL;
    bool rssi1_needed = false;

    LOGD(TAG "%s(%s): Start\n", __FUNCTION__, RatToString(ANT_RAT_C2K));

    if (is_support_modem(4)) {
        LOGD(TAG "Get CCCI path of modem1");
        if (get_ccci_path(0, atDevPath1) == 0) {
            LOGD(TAG "Can't get CCCI path!");
            goto err;
        }
        LOGD(TAG "Go to open modem1 fd1 atDevPath1 = %s", atDevPath1);
        fd1 = openDeviceWithDeviceName(atDevPath1);
        if (fd1 < 0) {
             LOGD(TAG "Fail to open fd1\n");
             goto err;
        }
        LOGD(TAG "OK to open fd1\n");

        for (i = 0; i < 30; i++) usleep(50000); //sleep 1.5s wait for modem bootup

        LOGD(TAG "Go to open C2K modem fd3!");

#ifdef MTK_ECCCI_C2K
        if (get_ccci_path(3, atDevPath3) == 0) {
            LOGD(TAG "Can't get CCCI path!");
            goto err;
        }
#else
         atDevPath3 = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_AT);
#endif

        fd3 = openDeviceWithDeviceName(atDevPath3);
        if (fd3 < 0) {
            LOGD(TAG "Fail to open atDevPath3\n");
            goto err;
        }
        LOGD(TAG "OK to open atDevPath3\n");
        for (i = 0; i < 30; i++) usleep(50000); //sleep 1.5s wait for modem bootup
    } else {
        LOGD(TAG "Not support C2K modem\n");
        goto err;
    }

    LOGD(TAG "[MD1] AT polling first:\n");
    do {
        send_at(fd1, "AT\r\n");
    } while (wait4_ack(fd1, NULL, 300));
    LOGD(TAG "[MD1]Send AT+ESLP=0 to disable sleep mode:\n");
    if (send_at(fd1, "AT+ESLP=0\r\n")) goto err;
    if (wait4_ack(fd1, NULL, 5000)) goto err;

    LOGD(TAG "[MD1]Send AT+ESIMS=1 to reset SIM1:\n");
    if (send_at(fd1, "AT+ESIMS=1\r\n")) goto err;
    if (wait4_ack(fd1, NULL, 5000)) goto err;

    LOGD(TAG "[MD1]Send AT+EFUN=0\n");
    if (send_at(fd1, "AT+EFUN=0\r\n")) goto err;
    if (wait4_ack(fd1, NULL, 5000)) goto err;

    LOGD(TAG "[MD1]Send AT+EMDSTATUS=1,1\n");
    if (send_at(fd1, "AT+EMDSTATUS=1,1\r\n")) goto err;
    if (wait4_ack(fd1, NULL, 5000)) goto err;

    LOGD(TAG "[MD1]send AT+EFUN=1\n");
    if (send_at(fd1, "AT+EFUN=1\r\n")) goto err;
    if (wait4_ack(fd1, NULL, 3000)) goto err;

    LOGD(TAG "[MD3]AT polling first:\n");
    send_at(fd3, "ate0q0v1\r\n");
    do {
        send_at(fd3, "AT\r\n");
    } while (wait4_ack(fd3, NULL, 3000));

    LOGD(TAG "[MD3]Send AT+CPOF to reboot modem \n");
    if (send_at(fd3, "AT+CPOF\r\n")) goto err;
    wait4_ack(fd3, NULL, 5000);

    LOGD(TAG "[MD3]Wait for +VPON:0, C2K modem turn off:\n");
    wait4_ack(fd3, "+VPON:0", 3000);

    LOGD(TAG "[MD3]Send AT+EMDSTATUS=1,1\n");
    if (send_at(fd3, "AT+EMDSTATUS=1,1\r\n")) goto err;
    wait4_ack(fd3, NULL, 5000);

    /* Reboot modem to make new band setting work */
    LOGD(TAG "[MD3]Send AT+CPON to reboot modem \n");
    int retErr = -1;
    do
    {
        send_at(fd3, "AT+CPON\r\n");
        retErr = wait4_ack_with_retry_error(fd3, NULL, "+CME ERROR:13", 5000);
    } while (retErr > 0);
    if (retErr < 0) {
        LOGD(TAG "[MD3] CPON error");
        goto err;
    }

    //read property from factory.ini
    mC2kRfSubsystem = read_preferred_para("C2K.RF.subsystem", 0);
    mC2kRfBand = read_preferred_para("C2K.RF.band", 0);
    mC2kRfChannel = read_preferred_para("C2K.RF.channel", 283);
    mC2kRfRssi0dbm = read_preferred_para("C2K.RF.rssi0.dbm", -95);

    pVal = ftm_get_prop("C2K.RF.rssi1.dbm");
    if (pVal != NULL) {
        mC2kRfRssi1dbm = (int)atoi(pVal);
        rssi1_needed = true;
        LOGD(TAG "preferred_receiver_para C2K.RF.rssi1.dbm- %d \n", mC2kRfRssi1dbm);
    } else {
        LOGD(TAG "preferred_receiver_para can't get C2K.RF.rssi1.dbm\n");
    }

    //set band & channel
    LOGD(TAG "[MD3]Send AT+ECBAND=1,%d,%d,%d to set band & channel\n", mC2kRfSubsystem, mC2kRfBand, mC2kRfChannel);
    memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf,"AT+ECBAND=1,%d,%d,%d\r\n", mC2kRfSubsystem, mC2kRfBand, mC2kRfChannel);
    if (send_at(fd3, cmd_buf)) goto err;
    wait4_ack(fd3, NULL, 5000);

    LOGD(TAG "[MD3]Wait for +VSER:0, found network\n");
    wait4_ack(fd3, "+VSER:0", 15000);

    /* Start RF test */
    tryCount = 0;

    while (tryCount++ < tryCountMax) {
        char *p = NULL;
        char *value = NULL;

        /* Check RF SQM level. The verdict of RF test is the SQM level shall be greater than 18 */
        LOGD(TAG "\n");
        usleep(HALT_TIME);

        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+ECSQ\r\n", (BUF_SIZE-1));
        send_at(fd3, cmd_buf);

        LOGD(TAG "Send AT+ECSQ to check RF, tryCount=%d \n", tryCount);

        memset(rsp_buf, 0, sizeof(rsp_buf));
        read(fd3, rsp_buf, BUF_SIZE-1);
        LOGD(TAG "------AT+ECSQ start------\n");
        LOGD(TAG "%s\n", rsp_buf);
        LOGD(TAG "------AT+ECSQ end------\n");

        p = strstr(rsp_buf, "+ECSQ:");
        if (p != NULL) {
            LOGD(TAG "p=%s\n", p);
            //example: p now is pointed to "+ECSQ: 0,-80,-96,1,-80,-90"
            tok_start(&p);

            if (0 != at_tok_nextint(&p, &subSystem)) continue; //continue because of incompleted response
            if (0 != at_tok_nextint(&p, &rssi0)) continue;
            if (0 != at_tok_nextint(&p, &rssi1)) continue;

            if (1 == mC2kRfSubsystem) //evdo
            {
                if (0 != at_tok_nextint(&p, &subSystem)) continue;
                if (0 != at_tok_nextint(&p, &rssi0)) continue;
                if (0 != at_tok_nextint(&p, &rssi1)) continue;
            }

            LOGD(TAG "subSystem=%d, rssi0=%d, rssi1=%d\n", subSystem, rssi0, rssi1);

            if (rssi0 == 0) {
                //rssi = -113;
                //Don't update rssi, and goto retry to get valid value
                LOGD(TAG "rssi0 = 0 ignore.\n");
                continue;
            }

            if (rssi1_needed) {
                if (rssi1 == 0) {
                    //rssi = -113;
                    //Don't update rssi, and goto retry to get valid value
                    LOGD(TAG "rssi1 = 0 ignore.\n");
                    continue;
                }
            }

            //rssi1 need confirm with modem

            if (rssi0 > 0 && rssi0 <= 31) {
                rssi0_dbm = rssi0 * 2 - 113;
            } else {
                LOGD(TAG "invalid rssi0 value.\n");
            }
            LOGD(TAG "rssi0_dbm=%d\n", rssi0_dbm);

            if (rssi1_needed) {
                if (rssi1 > 0 && rssi1 <= 31) {
                    rssi1_dbm = rssi1 * 2 - 113;
                } else {
                    LOGD(TAG "invalid rssi1 value.\n");
                }
                LOGD(TAG "rssi1_dbm=%d\n", rssi1_dbm);
            }

        } else {
            continue; //try again
        }

        /* AT+ECSQ might got null immeidate response or modem did not have any measurement result yet. keep retry polling */
        if ((rssi0_dbm < 0) && (rssi0_dbm > mC2kRfRssi0dbm)) {
            ret = 1;
            if (rssi1_needed) {
                if (((rssi1_dbm < 0) && (rssi1_dbm > mC2kRfRssi1dbm)) || (rssi1 == 0xFF)) {
                    ret = 1;
                } else {
                    ret = 0;
                }
            }
        } else {
            ret = 0;
        }
        break;
    }
    if (tryCount > tryCountMax) goto err;

    if (ret) {
        LOGD(TAG "C2K RF Test result pass\n");

        bool is_pass = true;
        format_test_verdict(info, infoLen, ANT_RAT_C2K, rssi1_needed,
                mC2kRfBand, mC2kRfChannel, mC2kRfRssi0dbm, mC2kRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);

        close(fd3);
        fd3 = -1;
        close(fd1);
        fd1 = -1;

        LOGD(TAG "%s: Exit\n", __FUNCTION__);

        return ret;
    } else {
        LOGD(TAG "C2K RF Test result fail\n");
        goto err;
    }

err:
    LOGD(TAG "%s: FAIL\n", __FUNCTION__);

    bool is_pass = false;
    format_test_verdict(info, infoLen, ANT_RAT_C2K, rssi1_needed,
            mC2kRfBand, mC2kRfChannel, mC2kRfRssi0dbm, mC2kRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);

    if (fd1 >= 0)
    {
        close(fd1);
    }
    fd1 = -1;
    if (fd3 >= 0)
    {
        close(fd3);
    }
    fd3 = -1;

    LOGD(TAG "%s: Exit\n", __FUNCTION__);

    return ret;
#else
    LOGD(TAG "EVDO_FTM_DT_VIA_SUPPORT support!!!");
    return 0;
#endif
#else
    LOGD(TAG "FACTORY_C2K_SUPPORT not support!!!");
    return 0;
#endif
} else {
    UNUSED_PARM(info);
    UNUSED_PARM(infoLen);
    UNUSED_PARM(index);
    LOGD(TAG "C2K rat Not support!!!");
    return 0;
}
}

static int c2k_ant_test_one_modem(char *info, int *infoLen, int index) {
    // C2K RF default value
    int mC2kRfSubsystem = 0;
    int mC2kRfBand = 0;
    int mC2kRfChannel = 0;
    int mC2kRfRssi0dbm = 0;
    int mC2kRfRssi1dbm = 0;

    // Received asu value & related subsystem
    int subSystem = 0;
    int rssi0 = 0;
    int rssi1 = 0xFF;

    //Received RSSI
    int rssi0_dbm = 0;
    int rssi1_dbm = 0;

    int fd = -1;
    int ret = 0;
    int tryCount = 0;
    int tryCountMax = read_preferred_para("RF.rssi.read.count", 100);

    if (tryCountMax < 10) {
        tryCountMax = 10;
    } else if (tryCountMax > 500) {
        tryCountMax = 500;
    }
    const int HALT_TIME = 200000;//0.2s

    //at command and response buf
    const int BUF_SIZE = 256;
    char cmd_buf[BUF_SIZE];
    char rsp_buf[BUF_SIZE];

    char atDevPath1[32] = {0};

    char *pVal = NULL;
    bool rssi1_needed = false;

    LOGD(TAG "%s(%s): Start\n", __FUNCTION__, RatToString(ANT_RAT_C2K));

    if (is_support_modem(4)) {
        LOGD(TAG "Get CCCI path of modem1");
        if (get_ccci_path(0, atDevPath1) == 0) {
            LOGD(TAG "Can't get CCCI path!");
            goto err;
        }
        LOGD(TAG "Go to open modem1 fd atDevPath1 = %s", atDevPath1);
        fd = openDeviceWithDeviceName(atDevPath1);
        if (fd < 0) {
             LOGD(TAG "Fail to open fd\n");
             goto err;
        }
        LOGD(TAG "OK to open fd\n");
        for (int i = 0; i < 30; i++) usleep(50000); //sleep 1.5s wait for modem bootup
    }

    LOGD(TAG "[AT]AT polling first:\n");
    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 300));

    LOGD(TAG "Send AT+ESLP=0 to disable sleep mode:\n");
    if (send_at(fd, "AT+ESLP=0\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+ESIMS=1 to reset SIM1:\n");
    if (send_at(fd, "AT+ESIMS=1\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+EFUN=0\n");
    if (send_at(fd, "AT+EFUN=0\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+EREG=3\n");
    if (send_at(fd, "AT+EREG=3\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+CREG=3\n");
    if (send_at(fd, "AT+CREG=3\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+ERAT=7\n");
    if (send_at(fd, "AT+ERAT=7\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+EFUN=1\n");
    if (send_at(fd, "AT+EFUN=1\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "AT polling first:\n");
    if (send_at(fd, "ate0q0v1\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    LOGD(TAG "Send AT+ERAT=7,0\n");
    if (send_at(fd, "AT+ERAT=7,0\r\n")) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    do {
        send_at(fd, "AT\r\n");
    } while (wait4_ack(fd, NULL, 5000));

    LOGD(TAG "[MD]Wait for +EREG\n");
    wait4_ack(fd, "+EREG:", 5000);

    //read property from factory.ini
    mC2kRfSubsystem = read_preferred_para("C2K.RF.subsystem", 0);
    mC2kRfBand = read_preferred_para("C2K.RF.band", 0);
    mC2kRfChannel = read_preferred_para("C2K.RF.channel", 283);
    mC2kRfRssi0dbm = read_preferred_para("C2K.RF.rssi0.dbm", -95);

    pVal = ftm_get_prop("C2K.RF.rssi1.dbm");
    if (pVal != NULL) {
        mC2kRfRssi1dbm = (int)atoi(pVal);
        rssi1_needed = true;
        LOGD(TAG "preferred_receiver_para C2K.RF.rssi1.dbm- %d \n", mC2kRfRssi1dbm);
    } else {
        LOGD(TAG "preferred_receiver_para can't get C2K.RF.rssi1.dbm\n");
    }

    //set band & channel
    LOGD(TAG "Send AT+ECBAND=1,%d,%d,%d to set band & channel\n", mC2kRfSubsystem, mC2kRfBand, mC2kRfChannel);
    memset(cmd_buf, 0, sizeof(cmd_buf));
    sprintf(cmd_buf,"AT+ECBAND=1,%d,%d,%d\r\n", mC2kRfSubsystem, mC2kRfBand, mC2kRfChannel);
    if (send_at(fd, cmd_buf)) goto err;
    if (wait4_ack(fd, NULL, 5000)) goto err;

    /* Start RF test */
    tryCount = 0;
    while (tryCount++ < tryCountMax) {
        char *p = NULL;

        /* Check RF SQM level. The verdict of RF test is the SQM level shall be greater than 18 */
        usleep(HALT_TIME);
        LOGD(TAG "Send AT+C2KECSQ to check RF, tryCount=%d \n", tryCount);

        memset(cmd_buf, 0, sizeof(cmd_buf));
        strncpy(cmd_buf, "AT+C2KECSQ\r\n", (BUF_SIZE-1));
        send_at(fd, cmd_buf);

        memset(rsp_buf, 0, sizeof(rsp_buf));
        read(fd, rsp_buf, BUF_SIZE-1);
        LOGD(TAG "------AT+C2KECSQ start------\n");
        LOGD(TAG "%s\n", rsp_buf);
        LOGD(TAG "------AT+C2KECSQ end------\n");

        if(strstr(rsp_buf, "ERROR")) {
            tryCount = tryCountMax+1;
        }
        p = strstr(rsp_buf, "+C2KECSQ:");
        if (p != NULL) {
            LOGD(TAG "p=%s\n", p);
            //example: p now is pointed to "+C2KECSQ: 0,-80,-96,1,-80,-90"
            tok_start(&p);

            if (0 != at_tok_nextint(&p, &subSystem)) continue; //continue because of incompleted response
            if (0 != at_tok_nextint(&p, &rssi0)) continue;
            if (0 != at_tok_nextint(&p, &rssi1)) continue;

            if (1 == mC2kRfSubsystem) {  //evdo
                if (0 != at_tok_nextint(&p, &subSystem)) continue;
                if (0 != at_tok_nextint(&p, &rssi0)) continue;
                if (0 != at_tok_nextint(&p, &rssi1)) continue;
            }
            LOGD(TAG "subSystem=%d, rssi0=%d, rssi1=%d\n", subSystem, rssi0, rssi1);

            if (rssi0 == 0) {
                //rssi = -113;
                //Don't update rssi, and goto retry to get valid value
                LOGD(TAG "rssi0 = 0 ignore.\n");
                continue;
            }

            //rssi1 need confirm with modem
            if (rssi1_needed) {
                if (rssi1 == 0) {
                    //rssi = -113;
                    //Don't update rssi, and goto retry to get valid value
                    LOGD(TAG "rssi1 = 0 ignore.\n");
                    continue;
                }
            }

            if (rssi0 > 0 && rssi0 <= 31) {
                rssi0_dbm = rssi0 * 2 - 113;
            } else {
                LOGD(TAG "invalid rssi0 value.\n");
            }
            LOGD(TAG "rssi0_dbm=%d\n", rssi0_dbm);

            if (rssi1_needed) {
                if (rssi1 > 0 && rssi1 <= 31) {
                    rssi1_dbm = rssi1 * 2 - 113;
                } else {
                    LOGD(TAG "invalid rssi1 value.\n");
                }
                LOGD(TAG "rssi1_dbm=%d\n", rssi1_dbm);
            }

        } else {
            continue; //try again
        }

        /* AT+C2KECSQ might got null immeidate response or modem did not have any measurement result yet. keep retry polling */
        if ((rssi0_dbm < 0) && (rssi0_dbm > mC2kRfRssi0dbm)) {
            ret = 1;
            if (rssi1_needed) {
                if (((rssi1_dbm < 0) && (rssi1_dbm > mC2kRfRssi1dbm)) || (rssi1 == 0xFF)) {
                    ret = 1;
                } else {
                    ret = 0;
                }
            }
        } else {
            ret = 0;
        }
        break;
    }
    if (tryCount > tryCountMax) goto err;


    if (ret) {
        LOGD(TAG "C2K RF Test result pass\n");
        bool is_pass = true;
        format_test_verdict(info, infoLen, ANT_RAT_C2K, rssi1_needed,
                mC2kRfBand, mC2kRfChannel, mC2kRfRssi0dbm, mC2kRfRssi1dbm, rssi0_dbm, rssi1_dbm, is_pass, index);
    } else {
        LOGD(TAG "C2K RF Test result fail\n");
        goto err;
    }

    //reset modem to enter normal mode
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;
    return ret;

err:
    LOGD(TAG "%s(%s): FAIL\n",__FUNCTION__, RatToString(ANT_RAT_C2K));

    format_test_verdict(info, infoLen, ANT_RAT_C2K, rssi1_needed,
            mC2kRfBand, mC2kRfChannel, mC2kRfRssi0dbm, mC2kRfRssi1dbm, rssi0_dbm, rssi1_dbm, false, index);

    //reset modem to enter normal mode
    if (fd >= 0) {
        close(fd);
    }
    fd = -1;
    return ret;
}

static void *universal_ant_update_thread(void *priv)
{
    struct universal_ant_factory *rf = (struct universal_ant_factory*)priv;
    struct itemview *iv = rf->iv;
    const int HALT_TIME = 1000000;//1s

    int report_duration = read_preferred_para("RF.report.duration", 15);//default 15sec
    if (report_duration < 1) {
        report_duration = 1;
    } else if (report_duration > 120) {
        report_duration = 120;
    }

    int run_count = 0;
    int run_count_max = read_preferred_para("RF.test.count", 1);//default 1
    if (run_count_max < 1) {
        run_count_max = 1;
    } else if (run_count_max > 1000) {
        run_count_max = 1000;
    }

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    int ret = 0;
    rf->test_result = true;

    if(!rf->test_done) {
        const int NUM_RAT = 6;
        int rat_priority[NUM_RAT];
        memset(rat_priority, 0, sizeof(rat_priority));

        while (run_count++ < run_count_max) {
            int i = 0;
            int j = 0;
            int len = 0;
            memset(rf->info, 0, sizeof(rf->info) / sizeof(*(rf->info)));

            len += sprintf(rf->info + len, "--- #%d ---\n", run_count);
            iv->redraw(iv);
            LOGD(TAG "--- #%d ---\n", run_count);

            // read testing priority from ini
            rat_priority[0] = read_preferred_para("GSM.RF.priority", 0);
            rat_priority[1] = read_preferred_para("WCDMA.RF.priority", 0);
            rat_priority[2] = read_preferred_para("LTE.RF.priority", 0);
            rat_priority[3] = read_preferred_para("TDSCDMA.RF.priority", 0);
            rat_priority[4] = read_preferred_para("C2K.RF.priority", 0);
            rat_priority[5] = read_preferred_para("NR.RF.priority", 0);
            LOGD(TAG "preferred_receiver_para GSM.RF.priority: %d \n", rat_priority[0]);
            LOGD(TAG "preferred_receiver_para WCMDA.RF.priority: %d \n", rat_priority[1]);
            LOGD(TAG "preferred_receiver_para LTE.RF.priority: %d \n", rat_priority[2]);
            LOGD(TAG "preferred_receiver_para TDSCDMA.RF.priority: %d \n", rat_priority[3]);
            LOGD(TAG "preferred_receiver_para C2K.RF.priority: %d \n", rat_priority[4]);
            LOGD(TAG "preferred_receiver_para NR.RF.priority: %d \n", rat_priority[5]);
            // set default priority if priorities not defined
            if ((rat_priority[0] == rat_priority[1]) &&
                    (rat_priority[1] == rat_priority[2]) &&
                    (rat_priority[2] == rat_priority[3]) &&
                    (rat_priority[3] == rat_priority[4])) {
                rat_priority[0] = 6;
                rat_priority[1] = 5;
                rat_priority[2] = 4;
                rat_priority[3] = 3;
                rat_priority[4] = 2;
                rat_priority[5] = 1;
            }
            // prevent negative priority from ini
            for (i = 0; i < NUM_RAT; i++) {
                if (rat_priority[i] < 0) {
                    rat_priority[i] = 0;
                }
            }

            for (i = 0; i < NUM_RAT; i++) {
                int largest = -1;
                int rat_index = -1;
                for (j = 0; j < NUM_RAT; j++) {
                    if (rat_priority[j] > largest) {
                        largest = rat_priority[j];
                        rat_index = j;
                    }
                }
                if (rat_index > -1 && rat_index < NUM_RAT) {
                    rat_priority[rat_index] = -1;
                }

                switch (rat_index) {
                    case 0://gsm
                    {
                        int mGsmRfTest = read_preferred_para("GSM.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking GSM.RF.test\n");
                        if (mGsmRfTest) {
                            do {
                                ret = gsm_ant_test(rf->info, &len, ANT_RAT_GSM, index);
                                index++;
                                mGsmRfTest = read_preferred_para_index("GSM", "RF.test", index, 0);
                                if (!ret) {
                                    rf->test_result = false;
                                }
                                LOGD(TAG "redraw\n");
                                iv->redraw(iv);
                            } while (mGsmRfTest != 0);
                        }
                        break;
                    }
                    case 1://wcdma
                    {
                        int mWcdmaRfTest = read_preferred_para("WCDMA.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking WCDMA.RF.test\n");
                        if (mWcdmaRfTest) {
                            do {
                                ret = gsm_ant_test(rf->info, &len, ANT_RAT_WCDMA, index);
                                index++;
                                mWcdmaRfTest = read_preferred_para_index("WCDMA", "RF.test", index, 0);
                                if (!ret) {
                                    rf->test_result = false;
                                }
                                LOGD(TAG "redraw\n");
                                iv->redraw(iv);
                            } while (mWcdmaRfTest != 0);
                        }
                        break;
                    }
                    case 2://lte
                    {
                        int mLteRfTest = read_preferred_para("LTE.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking LTE.RF.test\n");
                        if (mLteRfTest) {
                            do {
                                ret = gsm_ant_test(rf->info, &len, ANT_RAT_LTE, index);
                                index++;
                                mLteRfTest = read_preferred_para_index("LTE", "RF.test", index, 0);
                                if (!ret) {
                                    rf->test_result = false;
                                }
                                LOGD(TAG "redraw\n");
                                iv->redraw(iv);
                            } while (mLteRfTest != 0);
                        }
                        break;
                    }
                    case 3://tdscdma
                    {
                        int mTdsRfTest = read_preferred_para("TDSCDMA.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking TDSCDMA.RF.test\n");
                        if (mTdsRfTest) {
                            do {
                                ret = gsm_ant_test(rf->info, &len, ANT_RAT_TDSCDMA, index);
                                index++;
                                mTdsRfTest = read_preferred_para_index("TDSCDMA", "RF.test", index, 0);
                                if (!ret) {
                                    rf->test_result = false;
                                }
                                LOGD(TAG "redraw\n");
                                iv->redraw(iv);
                            } while (mTdsRfTest != 0);
                        }
                        break;
                    }
                    case 4://c2k
                    {
                        int mC2kRfTest = read_preferred_para("C2K.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking C2K.RF.test\n");
                        if (mC2kRfTest) {
                            ret = 0;
                            if (isC2kSupport() == 1) {
                                if (gOneModem == 1) {
                                    ret = c2k_ant_test_one_modem(rf->info, &len, index);
                                } else {
                                    ret = c2k_ant_test(rf->info, &len, index);
                                }
                            }
                            if (!ret) {
                                rf->test_result = false;
                            }
                            LOGD(TAG "redraw\n");
                            iv->redraw(iv);
                        }
                        break;
                    }
                    case 5://NR
                    {
                        int mNrRfTest = read_preferred_para("NR.RF.test", 0);
                        int index = 0;
                        LOGD(TAG "checking NR.RF.test\n");
                        if (mNrRfTest) {
                            do {
                                ret = nr_ant_test(rf->info, &len, ANT_RAT_NR, index);
                                index++;
                                mNrRfTest = read_preferred_para_index("NR", "RF.test", index, 0);
                                if (!ret) {
                                    rf->test_result = false;
                                }
                                LOGD(TAG "redraw\n");
                                iv->redraw(iv);
                            } while (mNrRfTest != 0);
                        }
                        break;
                    }
                    default:
                    {
                        rf->test_result = false;
                        break;
                    }
                }
            }
            usleep(HALT_TIME * report_duration);
        }
        rf->test_done = true;
    } // end if(!sim->test_done)

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    return NULL;
}

static int universal_ant_test_entry(struct ftm_param *param, void *priv)
{
    int  passCount = 0;
    struct universal_ant_factory *rf = (struct universal_ant_factory*)priv;
    struct itemview *iv = NULL;

    LOGD(TAG "%s: Start\n", __FUNCTION__);
    memset(rf->info, 0, sizeof(rf->info));
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
    iv->set_items(iv, universal_ant_items, 0);
    iv->set_text(iv, &rf->text);
    iv->start_menu(iv,0);
    iv->redraw(iv);

    rf->exit_thread = false;
    rf->test_done = false;
    rf->test_result = true;

    checkOneModem();

    pthread_create(&rf->update_thread, NULL, universal_ant_update_thread, priv);

    //strcpy(rf->info, "");
    //rf->test_done = false;
    //while (strlen(rf->info) == 0) {
    //usleep(200000);
    //  if (strstr(rf->info, uistr_info_pass)) {
    //   passCount++;
    //}
    //}
    while (rf->test_done == false) {
        usleep(200000);
    }
    if (rf->test_result) {
        passCount++;
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

int universal_ant_test_init(void)
{
    int ret = 0;
    struct ftm_module *mod;
    struct universal_ant_factory *rf;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_UNIVERSAL_ANT_TEST, sizeof(struct universal_ant_factory));
    if(!mod) {
        return -ENOMEM;
    }
    rf = mod_to_ant(mod);
    rf->mod = mod;
    rf->test_done = true;

    ret = ftm_register(mod, universal_ant_test_entry, (void*)rf);
    if(ret) {
        LOGD(TAG "register universal_ant_test_entry failed (%d)\n", ret);
    }
    return ret;
}

static int get_ccci_path(int modem_index,char * path)
{
#ifdef MTK_ECCCI_C2K
    int idx[MAX_MODEM_INDEX] = {1,2,5,4};
    int md_sys[MAX_MODEM_INDEX] = {MD_SYS1, MD_SYS2, MD_SYS5, MD_SYS3};
#else
    int idx[MAX_MODEM_INDEX] = {1,2,5};
    int md_sys[MAX_MODEM_INDEX] = {MD_SYS1, MD_SYS2, MD_SYS5};
#endif
    LOGD(TAG "modem_index:%d , %d",modem_index, idx[modem_index]);

    if(is_support_modem(idx[modem_index]))
    {
#ifdef MTK_ECCCI_C2K
       if(modem_index == 3){
            LOGD(TAG "prepare to get md3 node");
            snprintf(path, 32, "%s", ccci_get_node_name(USR_C2K_AT, (CCCI_MD)md_sys[modem_index]));
            LOGD(TAG "got md3 node %s", path);
        } else {
            snprintf(path, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, (CCCI_MD)md_sys[modem_index]));
        }
#else
       snprintf(path, 32, "%s", ccci_get_node_name(USR_FACTORY_DATA, (CCCI_MD)md_sys[modem_index]));
#endif
       LOGD(TAG "CCCI Path:%s",path);
       return 1 ;
    }
    else
    {
       LOGD(TAG "get_ccci_path fail due to not support modem");
       return 0 ;
    }
}

#ifdef FACTORY_C2K_SUPPORT
#ifndef EVDO_FTM_DT_VIA_SUPPORT
int wait4_ack_with_retry_error (const int fd, char *pACK, char *pRetryError, int timeout)
{
    char buf[BUF_SIZE_EXT] = {0};
    char *  p = NULL;
    int rdCount = 0, LOOP_MAX;
    int ret = -1;

    LOOP_MAX = timeout*1000/HALT_INTERVAL;

    LOGD(TAG "Wait for AT ACK...: %s; Special Pattern: %s\n", buf, (pACK==NULL)?"NULL":pACK);

    for(rdCount = 0; rdCount < LOOP_MAX; ++rdCount)
    {
        memset(buf,'\0',BUF_SIZE_EXT);
        if (pthread_mutex_lock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_lock ERROR!\n");
        }
        ret = read(fd, buf, BUF_SIZE_EXT - 1);
        if (pthread_mutex_unlock (&ccci_mutex))
        {
            LOGE( "read_ack pthread_mutex_unlock ERROR!\n");
        }

        LOGD(TAG "AT CMD ACK: %s.rdCount=%d\n", buf,rdCount);
        p = NULL;


        if (pACK != NULL)
        {
              p = strstr(buf, pACK);
              if(p) {
                ret = 0; break;
              }
              p = strstr(buf, pRetryError);
              if(p) {
                  LOGD(TAG "AT CMD retryError got");
                  ret = 1; break;
              }
              p = strstr(buf, "ERROR");
              if(p) {
                ret = -1; break;
              }
              p = strstr(buf, "NO CARRIER");
              if(p) {
                ret = -1; break;
              }

        }
        else
        {
            p = strstr(buf, "OK");
            if (p) {
                LOGD(TAG "Char before OK are %c,%c.\n", *(p - 2), *(p - 1));
                if (*(p - 2) == 'E' && *(p - 1) == 'P') {
                    char * ptr = NULL;
                    ptr = strstr(p + 1, "OK");
                    if (ptr) {
                        LOGD(TAG "EPOK detected and OK followed\n");
                        ret = 0;
                        break;
                    } else {
                        LOGD(TAG "EPOK detected and no further OK\n");
                    }
                } else {
                    LOGD(TAG "OK response detected\n");
                    ret = 0;
                    break;
                }
            }
            p = strstr(buf, pRetryError);
            if (p) {
                LOGD(TAG "AT CMD retryError got");
                ret = 1;
                break;
            }
            p = strstr(buf, "ERROR");
            if (p) {
                ret = -1;
                break;
            }
            p = strstr(buf, "NO CARRIER");
            if (p) {
                ret = -1;
                break;
            }
        }
        usleep(HALT_INTERVAL);

    }
    LOGD("ret = %d",ret);
    return ret;
}
#endif // EVDO_FTM_DT_VIA_SUPPORT
#endif // FACTORY_C2K_SUPPORT

#endif

