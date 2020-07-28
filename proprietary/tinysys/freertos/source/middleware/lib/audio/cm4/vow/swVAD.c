/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
 */

#include <stdio.h>
#include <FreeRTOS.h>
#include <stdlib.h>
#include <string.h>
#include "swVAD.h"

struct VAD_INFO VadInfo;
struct DCREMOVAL_INFO DcRemovalPara;
struct DATABUFFER_INFO DataBuffer;

void dataInit()
{
    memset(DataBuffer.dataRing, 0, sizeof(int)*FRAMELEN);
    return;
}

void dcRemovalInit()
{
    DcRemovalPara.preX = 0;
    DcRemovalPara.preY = 0;
    return;
}

void vadInit()
{
    VadInfo.frameIndex = 0;
    VadInfo.speechCount = 0;
    VadInfo.noiseCount = 0;
    VadInfo.vadFlag = 0;
    VadInfo.currVadPos = 0;
    VadInfo.noiseRef = 0;
    VadInfo.snrValue=-100;
    memset(VadInfo.signalRing, 0, sizeof(float) * SIGNAL_EST_BUFFER_LEN);
    return;
}

void dcRemoval()
{
    int i;
    int temp;
    for(i = 0; i < FRAMELEN ; i++) {
        temp = DcRemovalPara.preY - (DcRemovalPara.preY >> 6) - DcRemovalPara.preX;    // y(n) = x(n)- x(n-1) + (1-1/64)*y(n-1); -3dB@39.5Hz
        DcRemovalPara.preX = DataBuffer.dataRing[i];
        DataBuffer.dataRing[i] = DataBuffer.dataRing[i] + temp;
        DcRemovalPara.preY = DataBuffer.dataRing[i];
    }
    return;
}


void noiseRefUpdate()
{
    long long temp64;
    int i;
    int noiseCurr;

    temp64 = 0;
    for (i = -NOISE_EST_LEN; i <= NOISE_EST_LEN; i++) {
        temp64 += (long long) VadInfo.signalRing[(VadInfo.frameIndex - SIGNAL_EST_LEN + i) % SIGNAL_EST_BUFFER_LEN];
    }
    noiseCurr= (int)(temp64 / NOISE_EST_BUFFER_LEN);

    VadInfo.noiseRef = VadInfo.noiseRef + ((noiseCurr - VadInfo.noiseRef) >> 5);  // N'= (1-1/32)N+ (1/32)Ncurr; 1/32=0.0313
    return;
}

void noiseRefMinimumTracking()
{
    int i;
    int noiseCurr;
    const int minimumTrackLen = 6;

    noiseCurr = 32767 << 15;
    for (i = -minimumTrackLen; i <= minimumTrackLen; i++) {
        if (VadInfo.signalRing[(VadInfo.frameIndex-SIGNAL_EST_LEN + i) % SIGNAL_EST_BUFFER_LEN] < noiseCurr) {
            noiseCurr = VadInfo.signalRing[(VadInfo.frameIndex - SIGNAL_EST_LEN + i) % SIGNAL_EST_BUFFER_LEN];
        }
    }
    VadInfo.noiseRef = VadInfo.noiseRef + ((noiseCurr - VadInfo.noiseRef) >> 5);  // N'= (1-1/32)N+ (1/32)Ncurr; 1/32=0.0313
    return;
}

int frameBasedVAD()
{
    int i = 0;
    int frameAbs;
    int bufferAbs;
    int snrValue = -123;
    long long temp64;

    temp64 = 0;
    for (i = 0; i < FRAMELEN; i++) {
        temp64 += (long long) (abs(DataBuffer.dataRing[i])); // 18.13
    }
    frameAbs = (int) (temp64 / FRAMELEN);

    if (VadInfo.frameIndex < INIT_NOISE_LEN) {
// VadInfo.noiseRef += (frameAbs/INIT_NOISE_LEN);
        VadInfo.noiseRef += (frameAbs >> 4); //16 frames --> shift 4 bits
    }

    VadInfo.signalRing[VadInfo.frameIndex%SIGNAL_EST_BUFFER_LEN] = frameAbs;

    if (VadInfo.frameIndex > SIGNAL_EST_BUFFER_LEN) {
        temp64 = 0;
        for (i = 0; i < SIGNAL_EST_BUFFER_LEN; i++) {
            temp64 += ((long long) VadInfo.signalRing[i]);
        }
        bufferAbs = (int)(temp64 / SIGNAL_EST_BUFFER_LEN);
        snrValue = bufferAbs / ((VadInfo.noiseRef >> 5) + 1);  // rescale SNR values to easy set THD

        if (snrValue >= VAD_THD) {
            VadInfo.vadFlag = 1;
            VadInfo.speechCount++;
            VadInfo.noiseCount = 0;
        } else {
            VadInfo.vadFlag = 0;
            VadInfo.speechCount = 0;
            VadInfo.noiseCount++;
            noiseRefUpdate();
        }
    }

    if (VadInfo.speechCount >= FORCE_NOISEUPDATE_THD1) {
        noiseRefUpdate();
    } else if (VadInfo.speechCount >= FORCE_NOISEUPDATE_THD2) {
        noiseRefMinimumTracking();
    }
    return snrValue;
}

int back2Phase1Decision(int noKeywordCount)
{
    int bk2Phase1Flag = 0;

    if ((VadInfo.noiseCount > BK2PHASE1_NOISE_THD) && (noKeywordCount > BK2PHASE1_KWS_THD)) {
        bk2Phase1Flag = 1;
    }
    return bk2Phase1Flag;
}

void initSwVad()
{
    dataInit();
    dcRemovalInit();
    vadInit();
    return;
}

void resetSwVadPara()   // call this function when phase 2--> phase 1 --> phase 2
{
    VadInfo.speechCount = 0;
    VadInfo.noiseCount = 0;
    VadInfo.vadFlag = 0;
    return;
}

int returnToPhase1BySwVad(short* inData, int noKeywordCount)
{
    int i;

    for (i=0; i< FRAMELEN; i++) {
        DataBuffer.dataRing[i] = ((int) inData[i]) << 13;
    }

    // remove low-frequency component. -3dB @39.5Hz
    dcRemoval();
    VadInfo.snrValue = frameBasedVAD();
    VadInfo.frameIndex++;
    return back2Phase1Decision(noKeywordCount);
}

/*
void getVadInfo(struct VAD_INFO* rtnInfo)
{
    (*rtnInfo) = VadInfo;
}
*/

int decidePhase1RGValue(void) //pass in VadInfo.snrValue;
{
    const int noise_level_6dBth = 350000;
    const int noise_level_0dBth = 5000000;
    const int th_6dB = 320;
    const int th_0dB = 260;
    const int rg_value_6dB = 1;
    const int rg_value_0dB = 0;
    //const int RG_207E_AFE_VOW_CFG6_bit10_to_bit8_rg_flr_ratio = 111;
    const int defaultvalue = -1;
    int RG_2076_AFE_VOW_CFG2 = defaultvalue;
    int snr = VadInfo.snrValue;
    int noise = VadInfo.noiseRef;

    if ((noise >= noise_level_6dBth) && (snr <= th_6dB)) {
        if ((noise >= noise_level_0dBth) && (snr <= th_0dB)) {
            RG_2076_AFE_VOW_CFG2 = rg_value_0dB;
        } else {
            RG_2076_AFE_VOW_CFG2 = rg_value_6dB;
        }
    }
    //else 2076_AFE_VOW_CFG2 left as default.
    //printf("snr:%d, noise:%d\n\r", VadInfo.snrValue, VadInfo.noiseRef);
    //TODO: then, actually set the RGs for 207E_AFE_VOW_CFG6_bit10_to_bit8_rg_flr_ratio & 2076_AFE_VOW_CFG2
    return RG_2076_AFE_VOW_CFG2;
}
