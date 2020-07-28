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

#ifndef _SWVAD_H
#define _SWVAD_H

int  returnToPhase1BySwVad(short* inData, int kwsResult);
void initSwVad();
void resetSwVadPara();

#define FRAMELEN                  160
#define INIT_NOISE_LEN            16  // 16 frames --> shift 4 bits
#define NOISE_EST_LEN             3
#define SIGNAL_EST_LEN            9
#define SIGNAL_EST_BUFFER_LEN    ((SIGNAL_EST_LEN << 1) + 1)
#define NOISE_EST_BUFFER_LEN     ((NOISE_EST_LEN << 1) + 1)

#define VAD_THD                  45  // floor(1.414*32=45.2) --> set 45 as THD;  (3dB=1.414)
#define FORCE_NOISEUPDATE_THD1   400
#define FORCE_NOISEUPDATE_THD2   250
#define BK2PHASE1_NOISE_THD      100
#define BK2PHASE1_KWS_THD        100

#define abs(a)  (a > 0)? a : -a

struct DATABUFFER_INFO {
    int dataRing[FRAMELEN];
};

struct DCREMOVAL_INFO {
    int preX;
    int preY;
};

struct VAD_INFO {
    int frameIndex;
    int noiseCount;
    int speechCount;
    int vadFlag;
    int currVadPos;
    int noiseRef;
    int snrValue;
    int signalRing[SIGNAL_EST_BUFFER_LEN];
};
int decidePhase1RGValue(void);

#endif
