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



#define MTK_LOG_ENABLE 1
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <linux/ioctl.h>
#include <unistd.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include <cutils/log.h>
#undef LOG_TAG
#define LOG_TAG "FTM_HEADSET"

#ifdef FEATURE_FTM_HEADSET
#include "ftm_audio_Common.h"
//#include <AudioYusuHeadsetMessage.h>

#ifdef __cplusplus
extern "C" {
#include "DIF_FFT.h"
#include "Audio_FFT_Types.h"
#include "Audio_FFT.h"
#endif


#define TAG                 "[HEADSET] "
#define HEADSET_STATE_PATH "/sys/bus/platform/drivers/Accdet_Driver/state"

//#ifdef RECEIVER_HEADSET_AUTOTEST
//#undef RECEIVER_HEADSET_AUTOTEST
//#endif
/*
 * NO_DEVICE =0,
 * HEADSET_MIC = 1,
 * HEADSET_NO_MIC = 2,
 * HEADSET_FIVE_POLE = 3,
*/
#define BUF_LEN 1
char rbuf[BUF_LEN] = {'\0'};
char wbuf[BUF_LEN] = {'1'};
char wbuf_2[BUF_LEN] = {'2'};
char hs5pole_id[BUF_LEN] = {'3'};

enum
{
    ITEM_RINGTONE,
    ITEM_MIC1,
    ITEM_MIC2,
    ITEM_HEADSET_MIC,
    ITEM_PASS,
    ITEM_FAIL,
};

enum HEADSET_POLE_TYPE
{
    HEADSET_POLE_UNKNOWN = -1,
    HEADSET_POLE_3 = 3,
    HEADSET_POLE_4 = 4,
    HEADSET_POLE_5 = 5
};

#ifdef RECEIVER_HEADSET_AUTOTEST
static item_t headset_items_auto[] =
{
    { -1, NULL, 0, 0},
};
#endif
static item_t headset_items[] =
{
    {ITEM_RINGTONE, uistr_info_audio_ringtone, COLOR_BLACK, 0},
    {ITEM_HEADSET_MIC, uistr_info_audio_loopback_headset_mic, COLOR_BLACK, 0},
#ifndef FEATURE_FTM_TOUCH_MODE
    {ITEM_PASS, uistr_pass, COLOR_BLACK, 0},
    {ITEM_FAIL, uistr_fail, COLOR_BLACK, 0},
#endif
    { -1, NULL, 0, 0},
};

static item_t headset_items_ex[] =
{
    {ITEM_RINGTONE, uistr_info_audio_ringtone, COLOR_BLACK, 0},
#ifdef MTK_DUAL_MIC_SUPPORT
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mi1, 0, 0},
    {ITEM_MIC2, uistr_info_audio_loopback_dualmic_mi2, 0, 0},
#else
    {ITEM_MIC1, uistr_info_audio_loopback_dualmic_mic, 0, 0},
#endif
    {ITEM_HEADSET_MIC, uistr_info_audio_loopback_headset_mic, COLOR_BLACK, 0},
#ifndef FEATURE_FTM_TOUCH_MODE
    {ITEM_PASS, uistr_pass, COLOR_BLACK, 0},
    {ITEM_FAIL, uistr_fail, COLOR_BLACK, 0},
#endif
    { -1, NULL, 0, 0},
};
struct headset
{
    char  info[1024];
    bool  avail;
    bool  Headset_mic;
    enum HEADSET_POLE_TYPE num_hs_pole;
    bool  exit_thd;
    pthread_t hRecordThread;
    pthread_t headset_update_thd;
    struct ftm_module *mod;
    struct textview tv;
    struct itemview *iv;
    int  recordDevice;
    text_t    title;
    text_t    text;
    text_t    left_btn;
    text_t    center_btn;
    text_t    right_btn;
    bool  isPhoneTest;
};

#define mod_to_headset(p)     (struct headset*)((char*)(p) + sizeof(struct ftm_module))

#define ACCDET_IOC_MAGIC 'A'
#define ACCDET_INIT       _IO(ACCDET_IOC_MAGIC,0)  // call wehn first time
#define SET_CALL_STATE    _IO(ACCDET_IOC_MAGIC,1)  // when state is changing , tell headset driver.
#define GET_BUTTON_STATUS _IO(ACCDET_IOC_MAGIC,2)  // ioctl to get hook button state.
static const char *HEADSET_PATH = "/dev/accdet";

#define FFT_SIZE 4096
#define FFT_DAT_MAXNUM 200

// use for ioctl for headset driver
static int HeadsetFd = 0;
extern sp_ata_data return_data;
extern float CalculateTHD(unsigned int sampleRate, short *pData, kal_uint32 signalFrequency, float frequencyMargin);
extern void CalculateStatistics(float *pData, int length, int start, int end, audio_data_statistic *data_stat);

int headset_phonetest_state = 0;

static void headset_update_info(struct headset *hds, char *info)
{

    char *ptr;
    int fd = -1;
    int hb_status = 0;
    int ret = 0;
    hds->Headset_mic = false;

    fd = open(HEADSET_STATE_PATH, O_RDONLY, 0);
    if (fd == -1)
    {
        LOGD(TAG "Can't open %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        goto EXIT;
    }
    if (read(fd, rbuf, BUF_LEN) == -1)
    {
        LOGD(TAG "Can't read %s\n", HEADSET_STATE_PATH);
        hds->avail = false;
        close(fd);
        goto EXIT;
    }

    if (!strncmp(wbuf, rbuf, BUF_LEN))   /*the same*/
    {
        ALOGD(TAG "state == %s", wbuf);
        hds->avail = true;
        hds->Headset_mic = true;
        hds->num_hs_pole = HEADSET_POLE_4;
    }
    else
    {
        ALOGD(TAG "state != %s", wbuf);
        hds->avail = false;
        hds->num_hs_pole = HEADSET_POLE_UNKNOWN;
    }
    if (!strncmp(wbuf_2, rbuf, BUF_LEN))   /*the same*/
    {
        ALOGD(TAG "state == %s", wbuf_2);
        hds->avail = true;
        hds->Headset_mic = false;
        hds->num_hs_pole = HEADSET_POLE_3;
    }
    if (!strncmp(hs5pole_id, rbuf, BUF_LEN))   /*the same*/
    {
        ALOGD(TAG "state == %s", hs5pole_id);
        hds->avail = true;
        hds->Headset_mic = true;
        hds->num_hs_pole = HEADSET_POLE_5;
    }
    close(fd);

EXIT:
    return_data.headset.hds_state = hds->avail ? 1 : 0;
    return_data.headset.hds_mic_state = hds->Headset_mic ? 1 : 0;

    //if (hds->avail) {
    if (!strncmp(wbuf, rbuf, BUF_LEN))
    {
#ifdef HEADSET_BUTTON_DETECTION
        // open headset device
        HeadsetFd = open(HEADSET_PATH, O_RDONLY);
        if (HeadsetFd < 0)
        {
            LOGD(TAG "FTM:HEADSET open %s error fd = %d", HEADSET_PATH, HeadsetFd);
            goto EXIT_HEADSET;
        }

        // enable button detection
        LOGD(TAG "enable button detection \n");
        ret = ::ioctl(HeadsetFd, SET_CALL_STATE, 1);

        // read button status
        LOGD(TAG "read button status \n");
        ret = ::ioctl(HeadsetFd, GET_BUTTON_STATUS, 0);
        if (ret == 0)
        {
            hb_status = 0;
        }
        else
        {
            hb_status = 1;
        }

        // disable button detection
        //LOGD(TAG "disable button detection \n");
        //ret = ::ioctl(HeadsetFd,HEADSET_SET_STATE,0);
        close(HeadsetFd);
#endif
    }

EXIT_HEADSET:
    /* preare text view info */
    ptr  = info;
    if (!hds->avail)
    {
        ptr += sprintf(ptr, "%s", uistr_info_audio_headset_note);
    }
    ptr += sprintf(ptr, "%s : %s\n\n", uistr_info_avail , hds->avail ? "Yes" : "No");
#if 0
    if (hds->avail)
    {
        ptr += sprintf(ptr, "%s", "Please listen the sound from the headset\n\n");
    }
    else
    {
        ptr += sprintf(ptr, "%s", "Please insert the headset to test this item\n\n");
    }
#endif

#ifdef HEADSET_BUTTON_DETECTION
    if (hds->avail)
    {
        ptr += sprintf(ptr, "%s: %s\n\n", uistr_info_button, hb_status ? uistr_info_press : uistr_info_release);
    }
#endif
    if (hds->avail)
    {
        ptr += sprintf(ptr, "%s %s\n\n", uistr_info_audio_headset_mic_avail, hds->Headset_mic ? uistr_info_audio_yes : uistr_info_audio_no);
        if (!hds->Headset_mic)
        {
            ptr += sprintf(ptr, "%s:N/A\n", uistr_info_audio_loopback_headset_mic);
        }
    }

    return;
}


static void *headset_update_iv_thread(void *priv)
{
    struct headset *hds = (struct headset *)priv;
    struct itemview *iv = hds->iv;
    int chkcnt = 5;

    LOGD(TAG "%s: Start\n", __FUNCTION__);

    while (1)
    {
        //usleep(200000);
        usleep(20000);
        chkcnt--;

        if (hds->exit_thd)
        {
            break;
        }

        if (chkcnt > 0)
        {
            continue;
        }

        headset_update_info(hds, hds->info);

        iv->set_text(iv, &hds->text);
        iv->redraw(iv);
        chkcnt = 5;
    }

    LOGD(TAG "%s: Exit\n", __FUNCTION__);
    pthread_exit(NULL);

    return NULL;
}
static bool read_preferred_recorddump(void)
{
    char *pDump = NULL;
    char uName[64];

    memset(uName, 0, sizeof(uName));
    sprintf(uName, "%s", "Audio.Record.Dump");
    pDump = ftm_get_prop(uName);
    ALOGD("pDump:%s", pDump);

    if (pDump != NULL)
    {
        if (!strcmp(pDump, "1"))
        {
            ALOGD("Dump record data");
            return true;
        }
        else
        {
            ALOGD("No need to dump record data");
            return false;
        }
    }
    else
    {
        ALOGD("Dump record prop can't get");
        return false;
    }
}

static void read_preferred_magnitude(int *pUpper, int *pLower)
{
    char *pMagLower = NULL, *pMagUpper = NULL;
    char uMagLower[64], uMagUpper[64];

    *pUpper = 0;
    *pLower = 0;
    memset(uMagLower, 0, sizeof(uMagLower));
    memset(uMagUpper, 0, sizeof(uMagUpper));

    sprintf(uMagLower, "%s", "Lower.Magnitude.Headset");
    sprintf(uMagUpper, "%s", "Upper.Magnitude.Headset");

    pMagLower = ftm_get_prop(uMagLower);
    pMagUpper = ftm_get_prop(uMagUpper);
    if (pMagLower != NULL && pMagUpper != NULL)
    {
        *pLower = (int)atoi(pMagLower);
        *pUpper = (int)atoi(pMagUpper);
        ALOGD("Lower.Magnitude:%d,Upper.Magnitude:%d\n", *pLower, *pUpper);
    }
    else
    {
        ALOGD("Lower/Upper.Magnitude can not get\n");
    }
}
static void *Audio_Record_thread(void *mPtr)
{
    struct headset *hds  = (struct headset *)mPtr;
    ALOGD(TAG "%s: Start", __FUNCTION__);
    usleep(100000);
    bool dumpFlag = read_preferred_recorddump();
    //    bool dumpFlag = true;//for test
    int magLower = 0, magUpper = 0;
    read_preferred_magnitude(&magUpper, &magLower);
    int lowFreq = 1000 * (1 - 0.1); //1k
    int highFreq = 1000 * (1 + 0.1);
    short *pbuffer, *pbufferL, *pbufferR;
    unsigned int freqDataL[3] = {0}, magDataL[3] = {0};
    unsigned int freqDataR[3] = {0}, magDataR[3] = {0};
    int checkCnt = 0;
    uint32_t samplerate = 0;

    pbuffer = (short *)malloc(8192 * sizeof(short));
    if (pbuffer == NULL) {
        ALOGE(TAG "%s: pbuffer allocate fail !!", __FUNCTION__);
        return NULL;
    }

    pbufferL = (short *)malloc(4096 * sizeof(short));
    if (pbufferL == NULL) {
        ALOGE(TAG "%s: pbufferL allocate fail !!", __FUNCTION__);
        free(pbuffer);
        return NULL;
    }

    pbufferR = (short *)malloc(4096 * sizeof(short));
    if (pbufferR == NULL) {
        ALOGE(TAG "%s: pbufferR allocate fail !!", __FUNCTION__);
        free(pbuffer);
        free(pbufferL);
        return NULL;
    }

    // headsetLR: Phone level test; headset: Non-Phone level test.
    return_data.headsetL.freqL = return_data.headsetR.freqL = freqDataL[0];
    return_data.headsetL.freqR = return_data.headsetR.freqR = freqDataR[0];
    return_data.headsetL.amplL = return_data.headsetR.amplL = magDataL[0];
    return_data.headsetL.amplR = return_data.headsetR.amplR = magDataR[0];

    recordInit(hds->recordDevice, &samplerate);
    while (1)
    {
        memset(pbuffer, 0, 8192 * sizeof(short));
        memset(pbufferL, 0, 4096 * sizeof(short));
        memset(pbufferR, 0, 4096 * sizeof(short));

        int readSize  = readRecordData(pbuffer, 8192 * 2);
        for (int i = 0 ; i < 4096 ; i++)
        {
            pbufferL[i] = pbuffer[2 * i];
            pbufferR[i] = pbuffer[2 * i + 1];
        }

        if (dumpFlag)
        {
            char filenameL[] = "/data/record_headset_dataL.pcm";
            char filenameR[] = "/data/record_headset_dataR.pcm";
            FILE *fpL = fopen(filenameL, "wb+");
            FILE *fpR = fopen(filenameR, "wb+");

            if (fpL != NULL)
            {
                fwrite(pbufferL, readSize / 2, 1, fpL);
                fclose(fpL);
            }

            if (fpR != NULL)
            {
                fwrite(pbufferR, readSize / 2, 1, fpR);
                fclose(fpR);
            }
        }

        memset(freqDataL, 0, sizeof(freqDataL));
        memset(freqDataR, 0, sizeof(freqDataR));
        memset(magDataL, 0, sizeof(magDataL));
        memset(magDataR, 0, sizeof(magDataR));
        ApplyFFT256(samplerate, pbufferL, 0, freqDataL, magDataL);
        ApplyFFT256(samplerate, pbufferR, 0, freqDataR, magDataR);
        for (int i = 0; i < 3 ; i ++)
        {
            SLOGV("%d.freqDataL[%d]:%d,magDataL[%d]:%d", i, i, freqDataL[i], i, magDataL[i]);
            SLOGV("%d.freqDataR[%d]:%d,magDataR[%d]:%d", i, i, freqDataR[i], i, magDataR[i]);
        }

        if (hds->isPhoneTest)
        {
            if (headset_phonetest_state == 0) // CH1
            {
                //CH1 Log
                return_data.headsetL.freqL = freqDataL[0];
                return_data.headsetL.amplL = magDataL[0];
                return_data.headsetL.freqR = freqDataR[0];
                return_data.headsetL.amplR = magDataR[0];

                if ((freqDataL[0] <= (unsigned int)highFreq && freqDataL[0] >= (unsigned int)lowFreq) && (magDataL[0] <= (unsigned int)magUpper && magDataL[0] >= (unsigned int)magLower))
                {
                    checkCnt ++;
                    if (checkCnt >= 5)
                    {
                        ALOGD("[Headset-L] freqDataL:%d,magDataL:%d,freqDataR:%d,magDataR:%d", freqDataL[0], magDataL[0], freqDataR[0], magDataR[0]);
                        checkCnt = 0;
                    }
                }
                else
                {
                    checkCnt = 0;
                }
            }
            else if (headset_phonetest_state == 1) // CH2
            {
                if ((freqDataR[0] <= (unsigned int)highFreq && freqDataR[0] >= (unsigned int)lowFreq) && (magDataR[0] <= (unsigned int)magUpper && magDataR[0] >= (unsigned int)magLower))
                {
                    checkCnt ++;
                    if (checkCnt >= 5)
                    {
                        ALOGD("[Headset-R] freqDataL:%d,magDataL:%d,freqDataR:%d,magDataR:%d", freqDataL[0], magDataL[0], freqDataR[0], magDataR[0]);
                        snprintf(hds->info, sizeof(hds->info), "Check freq pass.\n");
                        ALOGD(" @ info : %s", hds->info);
                        break;
                    }
                }
                else
                {
                    checkCnt = 0;
                }
            }
            else
            {
                break;
            }
        }
        else // Non Phone level test
        {
            if ((hds->num_hs_pole != HEADSET_POLE_5 &&
                (freqDataL[0] <= (unsigned int)highFreq && freqDataL[0] >= (unsigned int)lowFreq) &&
                (magDataL[0] <= (unsigned int)magUpper && magDataL[0] >= (unsigned int)magLower))
                ||
                (hds->num_hs_pole == HEADSET_POLE_5 &&
                (freqDataL[0] <= (unsigned int)highFreq && freqDataL[0] >= (unsigned int)lowFreq) &&
                (magDataL[0] <= (unsigned int)magUpper && magDataL[0] >= (unsigned int)magLower) &&
                (freqDataR[0] <= (unsigned int)highFreq && freqDataR[0] >= (unsigned int)lowFreq) &&
                (magDataR[0] <= (unsigned int)magUpper && magDataR[0] >= (unsigned int)magLower)))
            {
                checkCnt ++;
                ALOGD("[HS mic] checkCnt[%d], freqDataL:%d,magDataL:%d,freqDataR:%d,magDataR:%d", checkCnt, freqDataL[0], magDataL[0], freqDataR[0], magDataR[0]);
                if (checkCnt >= 5)
                {
                    snprintf(hds->info, sizeof(hds->info), "Check freq pass.\n");
                    ALOGD(" @ info : %s", hds->info);
                    break;
                }
            }
            else
            {
                checkCnt = 0;
                ALOGD("[HS mic] FAIL, checkCnt reset [%d], freqDataL:%d,magDataL:%d,freqDataR:%d,magDataR:%d", checkCnt, freqDataL[0], magDataL[0], freqDataR[0], magDataR[0]);
            }

            if (hds->exit_thd)
            {
                break;
            }
        }
    }

    // Log and ATA Return
    if (hds->isPhoneTest)
    {
        //CH2 Log
        return_data.headsetR.freqL = freqDataL[0];
        return_data.headsetR.freqR = freqDataR[0];
        return_data.headsetR.amplL = magDataL[0];
        return_data.headsetR.amplR = magDataR[0];
        ALOGD(TAG "ATA Return Data[Headset-L]: [Mic1]Freq = %d, Amp = %d, [Mic2]Freq = %d, Amp = %d", return_data.headsetL.freqL, return_data.headsetL.amplL, return_data.headsetL.freqR, return_data.headsetL.amplR);
        ALOGD(TAG "ATA Return Data[Headset-R]: [Mic1]Freq = %d, Amp = %d, [Mic2]Freq = %d, Amp = %d", return_data.headsetR.freqL, return_data.headsetR.amplL, return_data.headsetR.freqR, return_data.headsetR.amplR);
    }
    else
    {
        if (headset_phonetest_state == 0)   // L ch
        {
            return_data.headset.freqL = freqDataL[0];   // HS mic record in single channel format, data is the same in L && R channel
            return_data.headset.amplL = magDataL[0];
            ALOGD(TAG "ATA Return Data[Headset-L]: Freq = %d, Amp = %d", return_data.headset.freqL, return_data.headset.amplL);
        }
        else if (headset_phonetest_state == 1)  // R ch
        {
            if (hds->num_hs_pole == HEADSET_POLE_5)
            {
                return_data.headset.freqR = freqDataR[0];
                return_data.headset.amplR = magDataR[0];
            }
            else // hds->num_hs_pole == HEADSET_POLE_4
            {
                return_data.headset.freqR = freqDataL[0];
                return_data.headset.amplR = magDataL[0];
            }
            ALOGD(TAG "ATA Return Data[Headset-R]: Freq = %d, Amp = %d", return_data.headset.freqR, return_data.headset.amplR);
        }
    }

    free(pbuffer);
    free(pbufferL);
    free(pbufferR);

    ALOGD(TAG "%s: Stop", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}

static void *Headset_THD_Record_thread(void *mPtr)
{
    struct headset *hds  = (struct headset *)mPtr;
    ALOGD(TAG "%s: Start", __FUNCTION__);
    usleep(100000);
    int magLower = 0, magUpper = 0;
    read_preferred_magnitude(&magUpper, &magLower);
    int freqOfRingtone = 1000;
    //int lowFreq = freqOfRingtone * (1-0.05);
    //int highFreq = freqOfRingtone * (1+0.05);
    short *pbuffer, *pbufferL, *pbufferR;
    //unsigned int freqDataL[3]={0},magDataL[3]={0};
    //unsigned int freqDataR[3]={0},magDataR[3]={0};
    float thdPercentage = 0;
    int lenL = 0, lenR = 0;
    float thdData[2][FFT_DAT_MAXNUM];
    audio_data_statistic headsetL_thd_sta, headsetR_thd_sta;

    uint32_t sampleRate;

    pbuffer = (short *)malloc(8192 * sizeof(short));
    pbufferL = (short *)malloc(4096 * sizeof(short));
    pbufferR = (short *)malloc(4096 * sizeof(short));

    recordInit(hds->recordDevice, &sampleRate);
    while (1)
    {
        memset(pbuffer, 0, 8192 * sizeof(short));
        memset(pbufferL, 0, 4096 * sizeof(short));
        memset(pbufferR, 0, 4096 * sizeof(short));

        for (int i = 0 ; i < 4096 ; i++)
        {
            pbufferL[i] = pbuffer[2 * i];
            pbufferR[i] = pbuffer[2 * i + 1];
        }

        if (headset_phonetest_state == 0) // L ch
        {
            thdPercentage = CalculateTHD(48000, pbufferL, freqOfRingtone, 0.0);
            ALOGD("HeadsetL THD: %f", thdPercentage);
            thdData[0][lenL] = thdPercentage;
            lenL++;
        }
        else if (headset_phonetest_state == 1)
        {
            thdPercentage = CalculateTHD(48000, pbufferR, freqOfRingtone, 0.0);
            ALOGD("HeadsetR THD: %f", thdPercentage);
            thdData[1][lenR] = thdPercentage;
            lenR++;
        }

        if (hds->exit_thd)
        {
            break;
        }
    }

    CalculateStatistics(&thdData[0][0], lenL, 5, 1, &headsetL_thd_sta);
    CalculateStatistics(&thdData[1][0], lenR, 5, 1, &headsetR_thd_sta);

    if (headsetL_thd_sta.deviation < 0.5 && headsetR_thd_sta.deviation < 0.5)
    {
        snprintf(hds->info + strlen(hds->info), sizeof(hds->info) - strlen(hds->info), "Check THD pass.\n");
        ALOGD(" @ info : %s", hds->info);
    }

    {
        return_data.headsetL_thd.thd.mean      = headsetL_thd_sta.mean;
        return_data.headsetL_thd.thd.deviation = headsetL_thd_sta.deviation;
        return_data.headsetL_thd.thd.max       = headsetL_thd_sta.max;
        return_data.headsetL_thd.thd.min       = headsetL_thd_sta.min;

        return_data.headsetR_thd.thd.mean      = headsetR_thd_sta.mean;
        return_data.headsetR_thd.thd.deviation = headsetR_thd_sta.deviation;
        return_data.headsetR_thd.thd.max       = headsetR_thd_sta.max;
        return_data.headsetR_thd.thd.min       = headsetR_thd_sta.min;

        ALOGD(TAG "ATA Return THD(Headset-L): Mean = %f, Deviation = %f, Max = %f, Min = %f", return_data.headsetL_thd.thd.mean, return_data.headsetL_thd.thd.deviation, return_data.headsetL_thd.thd.max, return_data.headsetL_thd.thd.min);
        ALOGD(TAG "ATA Return THD(Headset-R): Mean = %f, Deviation = %f, Max = %f, Min = %f", return_data.headsetR_thd.thd.mean, return_data.headsetR_thd.thd.deviation, return_data.headsetR_thd.thd.max, return_data.headsetR_thd.thd.min);
    }

    free(pbuffer);
    free(pbufferL);
    free(pbufferR);

    ALOGD(TAG "%s: Stop", __FUNCTION__);
    pthread_exit(NULL); // thread exit
    return NULL;
}

int headset_thd_entry(struct ftm_param *param, void *priv)
{

    struct headset *hds = (struct headset *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    hds->exit_thd = false;

    if (!hds->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    iv->set_items(iv, headset_items_auto, 0);
    iv->set_text(iv, &hds->text);
    iv->start_menu(iv, 0);
    iv->redraw(iv);

    Common_Audio_init();
    headset_phonetest_state = 0;

    memset(hds->info, 0, sizeof(hds->info) / sizeof(*(hds->info)));
    hds->recordDevice = BUILTIN_MIC;

    pthread_create(&hds->hRecordThread, NULL, Headset_THD_Record_thread, priv);

    int play_time = 2000;// 2s
    hds->mod->test_result = FTM_TEST_FAIL;
    ALOGD("start play and freq check");
    //----- Test CH1
    EarphoneTest(1);
    EarphoneTestLR(0);
    for (int i = 0; i < 100 ; i ++)
    {
        usleep(play_time * 10);
    }
    //----- Delay 100ms
    headset_phonetest_state = 1;
    EarphoneTest(0);
    for (int i = 0; i < 20 ; i ++)
    {
        usleep(500 * 10);
    }
    //----- Test CH2
    EarphoneTest(1);
    EarphoneTestLR(1);
    for (int i = 0; i < 100 ; i ++)
    {
        //if (strstr(hds->info, "Check freq pass"))
        //{
        //    hds->mod->test_result = FTM_TEST_PASS;
        //    ALOGD("Check freq pass");
        //    //break;
        //}
        usleep(play_time * 10);
    }
    EarphoneTest(0);
    ALOGD("stop play and freq check");
    //if(hds->mod->test_result == FTM_TEST_FAIL)
    //   ALOGD("Check freq fail");

    hds->exit_thd = true;
    headset_phonetest_state = 2;

    pthread_join(hds->hRecordThread, NULL);
    Common_Audio_deinit();

    if (strstr(hds->info, "Check THD pass"))
    {
        hds->mod->test_result = FTM_TEST_PASS;
        ALOGD("Check THD pass");
    }
    if (hds->mod->test_result == FTM_TEST_FAIL)
    {
        ALOGD("Check THD fail");
    }

    LOGD(TAG "%s: End\n", __FUNCTION__);

    return 0;
}



//#ifdef RECEIVER_HEADSET_AUTOTEST
int mAudio_headset_auto_entry(struct ftm_param *param, void *priv)
{
    struct headset *hds = (struct headset *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, hds->info, COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    hds->exit_thd = false;

    if (!hds->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    iv->set_items(iv, headset_items_auto, 0);
    iv->set_text(iv, &hds->text);
    iv->start_menu(iv, 0);
    iv->redraw(iv);

    // initialize parameters
    return_data.headset.freqL = 0;  // return_data is for ATA tool, the tool we get this value after running this test
    return_data.headset.amplL = 0;  // return_data.headset will be set in Audio_Record_thread()
    return_data.headset.freqR = 0;
    return_data.headset.amplR = 0;

    int    play_time = 3000;//ms

    hds->mod->test_result = FTM_TEST_FAIL;

    hds->exit_thd = false;

    // check hs state
    headset_update_info(hds, hds->info);

    memset(hds->info, 0, sizeof(hds->info));
    char *ptr = hds->info;
    ptr += sprintf(ptr, "Headset Pole = %d\n", hds->num_hs_pole);
    iv->redraw(iv);

    if (hds->num_hs_pole == HEADSET_POLE_4)
    {
    }
    else if (hds->num_hs_pole == HEADSET_POLE_5)
    {
    }
    else    // HEADSET_POLE_3 or HEADSET_POLE_UNKNOWN
    {
        ptr += sprintf(ptr, "%s", "Insert HeadsetMic\n");
        iv->redraw(iv);
        usleep(2000000);
        return 0;
    }

    ALOGD("start play and freq check");

    // start test
    Common_Audio_init();

    memset(hds->info, 0, sizeof(hds->info) / sizeof(*(hds->info)));
    hds->recordDevice = WIRED_HEADSET;
    pthread_create(&hds->hRecordThread, NULL, Audio_Record_thread, priv);

    bool leftChPass = false;
    bool rightChPass = false;

    //----- Test CH1
    EarphoneTest(1);
    EarphoneTestLR(0);
    headset_phonetest_state = 0;    // for passing which HS channel is in use to Audio_Record_thread(), 0: L ch, 0: R ch
    for (int i = 0; i < 500 ; i ++)
    {
        if (strstr(hds->info, "Check freq pass"))
        {
            leftChPass = true;
            ALOGD("L channel check freq pass");
            break;
        }
        usleep(play_time * 20);
    }
    EarphoneTest(0);
    hds->exit_thd = true;
    pthread_join(hds->hRecordThread, NULL);
    Common_Audio_deinit();

    // create new record thread to check right channel
    Common_Audio_init();

    memset(hds->info, 0, sizeof(hds->info) / sizeof(*(hds->info)));
    hds->exit_thd = false;
    pthread_create(&hds->hRecordThread, NULL, Audio_Record_thread, priv);

    //----- Test CH2
    EarphoneTest(1);
    EarphoneTestLR(1);
    headset_phonetest_state = 1;    // for passing which HS channel is in use to Audio_Record_thread(), 0: L ch, 0: R ch
    for (int i = 0; i < 500 ; i ++)
    {
        if (strstr(hds->info, "Check freq pass"))
        {
            rightChPass = true;
            ALOGD("R channel check freq pass");
            break;
        }
        usleep(play_time * 20);
    }
    EarphoneTest(0);
    hds->exit_thd = true;
    pthread_join(hds->hRecordThread, NULL);

    ALOGD("stop play and freq check");
    if (leftChPass && rightChPass)
    {
        hds->mod->test_result = FTM_TEST_PASS;
        ALOGD("Headset Loopback Test PASS");
    }
    else
    {
        ALOGD("Headset Loopback Test FAIL!!!!!");
    }

    // deinit
    Common_Audio_deinit();

    LOGD(TAG "%s: End\n", __FUNCTION__);

    return 0;
}
//#else
int mAudio_headset_manual_entry(struct ftm_param *param, void *priv)
{
    int chosen;
    bool exit = false;
    char *inputType = NULL;
    struct headset *hds = (struct headset *)priv;
    struct itemview *iv;
    int privChosen = -1;
    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    headset_update_info(hds, hds->info);

    hds->exit_thd = false;

    if (!hds->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    if (get_is_ata() == 0)
    {
        iv->set_items(iv, headset_items, 0);
    }
    else
    {
        iv->set_items(iv, headset_items_ex, 0);
    }
    iv->set_text(iv, &hds->text);

    Common_Audio_init();

    pthread_create(&hds->headset_update_thd, NULL, headset_update_iv_thread, priv);

    inputType = ftm_get_prop("Audio.Manual.InputType");
    if (inputType != NULL && (atoi(inputType) == 0 || atoi(inputType) == 1 || atoi(inputType) == 2 || atoi(inputType) == 3))
    {
        // @ input != NULL, PC command control mode
        ALOGD("Audio.Manual.InputType = %s", inputType);

        iv->redraw(iv);

        PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
        usleep(20000);
        if (atoi(inputType) == 1)
        {
            ALOGD("Set Mic1 on");
            PhoneMic_EarphoneLR_Loopback(MIC1_ON);
        }
        else if (atoi(inputType) == 2)
        {
            ALOGD("Set Mic2 on");
            PhoneMic_EarphoneLR_Loopback(MIC2_ON);
        }
        else if (atoi(inputType) == 3)
        {
            ALOGD("Set headset Mic on");
            HeadsetMic_EarphoneLR_Loopback(1, 1);
        }

        hds->exit_thd = true;
        pthread_join(hds->headset_update_thd, NULL);
        hds->mod->test_result = FTM_TEST_PASS;

        if (atoi(inputType) == 0)
        {
            ALOGD("Audio Deinit");
            Common_Audio_deinit();
        }

    }
    else
    {
        // Original manual operating mode
        do
        {
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
            switch (chosen)
            {
                case ITEM_RINGTONE:
                    if (!hds->avail || privChosen == ITEM_RINGTONE)
                    {
                        break;
                    }
                    if (privChosen == ITEM_HEADSET_MIC && hds->Headset_mic)
                    {
                        HeadsetMic_EarphoneLR_Loopback(0, 1);
                    }
                    usleep(20000);
                    EarphoneTest(1);
                    privChosen = ITEM_RINGTONE;
                    break;
                case ITEM_MIC1:
                    if (!hds->avail || privChosen == ITEM_MIC1)
                    {
                        break;
                    }
                    if (privChosen == ITEM_RINGTONE)
                    {
                        EarphoneTest(0);
                    }
                    else if (privChosen == ITEM_MIC2)
                    {
                        PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                    }
                    else if (privChosen == ITEM_HEADSET_MIC)
                    {
                        HeadsetMic_EarphoneLR_Loopback(0, 1);
                    }

                    usleep(20000);
                    PhoneMic_EarphoneLR_Loopback(MIC1_ON);
                    privChosen = ITEM_MIC1;
                    break;
                case ITEM_MIC2:
                    if (!hds->avail || privChosen == ITEM_MIC2)
                    {
                        break;
                    }
                    if (privChosen == ITEM_RINGTONE)
                    {
                        EarphoneTest(0);
                    }
                    else if (privChosen == ITEM_MIC1)
                    {
                        PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                    }
                    else if (privChosen == ITEM_HEADSET_MIC)
                    {
                        HeadsetMic_EarphoneLR_Loopback(0, 1);
                    }

                    usleep(20000);
                    PhoneMic_EarphoneLR_Loopback(MIC2_ON);
                    privChosen = ITEM_MIC2;
                    break;
                case ITEM_HEADSET_MIC:
                    if (!hds->avail || privChosen == ITEM_HEADSET_MIC || !hds->Headset_mic)
                    {
                        break;
                    }
                    if (privChosen == ITEM_RINGTONE)
                    {
                        EarphoneTest(0);
                    }
                    else if (privChosen == ITEM_MIC1)
                    {
                        PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
                    }
                    else if (privChosen == ITEM_MIC2)
                    {
                        PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
                    }
                    usleep(20000);
                    HeadsetMic_EarphoneLR_Loopback(1, 1);
                    privChosen = ITEM_HEADSET_MIC;
                    break;
#ifndef FEATURE_FTM_TOUCH_MODE
                case ITEM_PASS:
                case ITEM_FAIL:
                    if (chosen == ITEM_PASS)
                    {
                        hds->mod->test_result = FTM_TEST_PASS;
                    }
                    else if (chosen == ITEM_FAIL)
                    {
                        hds->mod->test_result = FTM_TEST_FAIL;
                    }
                    exit = true;
                    break;
#else /* FEATURE_FTM_TOUCH_MODE */
                case L_BTN_DOWN:
                    hds->mod->test_result = FTM_TEST_FAIL;
                    exit = true;
                    break;
                case C_BTN_DOWN:
                    exit = true;
                    break;
                case R_BTN_DOWN:
                    hds->mod->test_result = FTM_TEST_PASS;
                    exit = true;
                    break;
#endif /* FEATURE_FTM_TOUCH_MODE */

            }

            if (exit)
            {
                hds->exit_thd = true;
                break;
            }
        }
        while (1);

        if (privChosen == ITEM_RINGTONE)
        {
            EarphoneTest(0);
        }
        if (privChosen == ITEM_MIC1)
        {
            PhoneMic_EarphoneLR_Loopback(MIC1_OFF);
        }
        if (privChosen == ITEM_MIC2)
        {
            PhoneMic_EarphoneLR_Loopback(MIC2_OFF);
        }
        if (privChosen == ITEM_HEADSET_MIC)
        {
            HeadsetMic_EarphoneLR_Loopback(0, 1);
        }

        pthread_join(hds->headset_update_thd, NULL);

        Common_Audio_deinit();
    }

    return 0;
}
//#endif

#ifdef RECEIVER_HEADSET_AUTOTEST
int headset_PhoneTest_entry(struct ftm_param *param, void *priv)
{
    struct headset *hds = (struct headset *)priv;
    struct itemview *iv;

    LOGD(TAG "%s\n", __FUNCTION__);

    init_text(&hds->title, param->name, COLOR_YELLOW);
    init_text(&hds->text, &hds->info[0], COLOR_YELLOW);
    init_text(&hds->left_btn, "Fail", COLOR_YELLOW);
    init_text(&hds->center_btn, "Pass", COLOR_YELLOW);
    init_text(&hds->right_btn, "Back", COLOR_YELLOW);

    headset_phonetest_state = 0;

    if (!hds->iv)
    {
        iv = ui_new_itemview();
        if (!iv)
        {
            LOGD(TAG "No memory");
            return -1;
        }
        hds->iv = iv;
    }

    iv = hds->iv;
    iv->set_title(iv, &hds->title);
    iv->set_items(iv, headset_items_auto, 0);
    iv->set_text(iv, &hds->text);
    iv->start_menu(iv, 0);
    iv->redraw(iv);

    Common_Audio_init();

    memset(hds->info, 0, sizeof(hds->info) / sizeof(*(hds->info)));
    hds->recordDevice = BUILTIN_MIC;

    pthread_create(&hds->hRecordThread, NULL, Audio_Record_thread, priv);

    int    play_time = 3000;// 3s
    hds->mod->test_result = FTM_TEST_FAIL;
    ALOGD("start play and freq check");
    //----- Test CH1
    EarphoneTest(1);
    EarphoneTestLR(0);
    for (int i = 0; i < 100 ; i ++)
    {
        usleep(play_time * 10);
    }
    //----- Delay 0.5s
    headset_phonetest_state = 1;
    EarphoneTest(0);
    for (int i = 0; i < 100 ; i ++)
    {
        usleep(500 * 10);
    }
    //----- Test CH2
    EarphoneTest(1);
    EarphoneTestLR(1);
    for (int i = 0; i < 100 ; i ++)
    {
        if (strstr(hds->info, "Check freq pass"))
        {
            hds->mod->test_result = FTM_TEST_PASS;
            ALOGD("Check freq pass");
            //break;
        }
        usleep(play_time * 10);
    }
    EarphoneTest(0);
    ALOGD("stop play and freq check");
    if (hds->mod->test_result == FTM_TEST_FAIL)
    {
        ALOGD("Check freq fail");
    }

    headset_phonetest_state = 2;

    pthread_join(hds->hRecordThread, NULL);
    Common_Audio_deinit();

    LOGD(TAG "%s: End\n", __FUNCTION__);

    return 0;
}
#endif

int headset_entry(struct ftm_param *param, void *priv)
{
    struct headset *mc = (struct headset *)priv;
    char *outputType = NULL;

    if (FTM_AUTO_ITEM == param->test_type)
    {
        outputType = ftm_get_prop("Audio.Auto.OutputType");
        ALOGD("Audio.Auto.OutputType = %s\n", outputType);

        if (outputType == NULL)
        {
            // @ default PCBA level, Headset receiver -> headset mic
            mc->isPhoneTest = false;
            mAudio_headset_auto_entry(param, priv);
        }
        else
        {
            if (atoi(outputType) == 0) // @ PCBA level, Headset receiver -> headset mic
            {
                mc->isPhoneTest = false;
                mAudio_headset_auto_entry(param, priv);
            }
            else if (atoi(outputType) == 1) //@ Phone level,  Headset receiver -> phone mic
            {
                mc->isPhoneTest = true;
                headset_PhoneTest_entry(param, priv);
            }
            else if (atoi(outputType) == 2) // @ Phone level, THD, Headset receiver-->phone mic
            {
                mc->isPhoneTest = true;
                headset_thd_entry(param, priv);
            }
        }
    }
    else if (FTM_MANUAL_ITEM == param->test_type)
    {
        mAudio_headset_manual_entry(param, priv);
    }

    return 0;
}

int headset_init(void)
{
    int ret = 0;
    struct ftm_module *mod, *mod_phone;
    struct headset *hds, *hds_phone;

    LOGD(TAG "%s\n", __FUNCTION__);

    mod = ftm_alloc(ITEM_HEADSET, sizeof(struct headset));
    hds = mod_to_headset(mod);

    hds->mod    = mod;
    hds->avail    = false;
    hds->isPhoneTest = false;

    if (!mod)
    {
        return -ENOMEM;
    }

    ret = ftm_register(mod, headset_entry, (void *)hds);

#ifdef RECEIVER_HEADSET_AUTOTEST
    //---- Phone Level Test
    mod_phone = ftm_alloc(ITEM_HEADSET_PHONE, sizeof(struct headset));
    hds_phone = mod_to_headset(mod_phone);
    hds_phone->mod    = mod_phone;
    hds_phone->avail    = false;
    hds_phone->isPhoneTest = true;
    if (!mod_phone)
    {
        return -ENOMEM;
    }
    ret = ftm_register(mod_phone, headset_PhoneTest_entry, (void *)hds_phone);
    //----
#endif

#if 0
    //Headset THD Test
    mod_thd = ftm_alloc(ITEM_HEADSET_THD, sizeof(struct headset));
    hds_thd = mod_to_headset(mod_thd);
    hds_thd->mod = mod_thd;
    hds_thd->recordDevice = WIRED_HEADSET;
    if (!mod_thd)
    {
        return -ENOMEM;
    }
    ret = ftm_register(mod_thd, headset_thd_entry, (void *)hds_thd);
#endif
    return ret;
}


#ifdef __cplusplus
}
#endif

#endif
