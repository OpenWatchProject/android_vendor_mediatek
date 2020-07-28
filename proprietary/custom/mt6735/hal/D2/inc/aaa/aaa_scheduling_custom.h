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

#ifndef AAA_SCHEDULING_CUSTOM_H_
#define AAA_SCHEDULING_CUSTOM_H_


#define MAX_FRAME_PER_CYCLE (6) //max number of M


enum // E_AE_JOB
{
    E_AE_IDLE      = 0,
    E_AE_AE_CALC   = (1<<0),
    E_AE_AE_APPLY  = (1<<1),
    E_AE_FLARE     = (1<<2)
};

enum E_Job_3A
{
    E_Job_AAO      = 0,
    E_Job_Awb,
    E_Job_Af,
    E_Job_Flicker,
    E_Job_Lsc,
    E_Job_AeFlare,
    E_Job_IspValidate, //IspValidate on/off  is not listed in scheduling table entry
    E_Job_NUM
};


struct WorkPerFrame
{
    MUINT32 AAOJobs;
    MUINT32 AwbJobs;
    MUINT32 AfJobs;
    MUINT32 FlickerJobs;
    MUINT32 LscJobs;
    
    MUINT32 AeFlareJobs;
};

struct WorkPerCycle
{
    WorkPerFrame mWorkPerFrame[MAX_FRAME_PER_CYCLE];
    MUINT32 mValidFrameIdx;
};

const char* Job3AName(E_Job_3A eJob);
WorkPerCycle getWorkPerCycle(int normalizeM, MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/); //M = fps/30
WorkPerCycle getWorkPerCycle_Main(int normalizeM);
WorkPerCycle getWorkPerCycle_Main2(int normalizeM);
WorkPerCycle getWorkPerCycle_Sub(int normalizeM);

MVOID resetCycleCtr(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);
MVOID resetCycleCtr_Main();
MVOID resetCycleCtr_Main2();
MVOID resetCycleCtr_Sub();

/*
definition of senDevId:
in IHalSensor.h (\alps_sw\trunk\kk\alps\mediatek\hardware\include\mtkcam\hal)
enum 
{
    SENSOR_DEV_NONE = 0x00,
    SENSOR_DEV_MAIN = 0x01,
    SENSOR_DEV_SUB  = 0x02,
    SENSOR_DEV_PIP = 0x03,
    SENSOR_DEV_MAIN_2 = 0x04,
    SENSOR_DEV_MAIN_3D = 0x05,
};
*/

int get3AThreadNiceValue(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);

long long int getVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/);
long long int getAEThreadVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/);

int queryFramesPerCycle_custom(int fps/*x1000*/);
int getResetFrameCount(MUINT32 senDevId = 1/*default: SENSOR_DEV_MAIN*/);

#endif /* AAA_SCHEDULING_CUSTOM_H_ */

