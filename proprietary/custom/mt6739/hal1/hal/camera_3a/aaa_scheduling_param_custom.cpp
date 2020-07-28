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

#include "camera_custom_types.h"
#include "aaa_scheduling_custom.h"


const char* Job3AName(E_Job_3A eJob)
{
    static const char* Names[E_Job_NUM] = 
    {
        "AAO",
        "Awb",
        "Af",
        "Flicker",
        "Lsc",
        "AeFlare",
        "IspValidate" //IspValidate on/off is not listed in scheduling table entry
    };
    return Names[eJob];
}


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
WorkPerCycle getWorkPerCycle(int normalizeM, MUINT32 senDevId) // M = fps/30
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return getWorkPerCycle_Main(normalizeM);
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return getWorkPerCycle_Sub(normalizeM);
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return getWorkPerCycle_Main2(normalizeM);
    default:
        return getWorkPerCycle_Main(normalizeM);
    }
}

MVOID resetCycleCtr(MUINT32 senDevId) //M = fps/30
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return resetCycleCtr_Main();
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return resetCycleCtr_Sub();
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return resetCycleCtr_Main2();
    default:
        return resetCycleCtr_Main();
    }
}

int get3AThreadNiceValue(MUINT32 senDevId)
{
	switch(senDevId)
	{
	case 0x01: // SENSOR_DEV_MAIN = 0x01
		return -8;
	case 0x02: // SENSOR_DEV_SUB  = 0x02
		return -8;
	case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
		return -8;
	default:
		return -8;
	}
}


long long int getVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/)
{
    // you may define your own timeout limit for each fps and each M
    /*
    criterion: as long as possible, but it should satisfy: 
        1. impossible to catch next Update, 
        2. 3A thread has enough time to wait next Vsync
    tolerance: 
        33-6=27ms for 30fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        16-3=13ms for 60fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        11-3=8ms for 90fps, -> might have short-P1-dump issue
        8-3=5ms for 120fps, -> might have short-P1-dump issue
    */
    switch((int)(fps/30000))
    {
    case 0:
        return (long long int) 8000000;
    case 1:
        return (long long int) 8000000;
    case 2:
        return (long long int) 8000000;
    case 3: 
        return (long long int) 8000000;
    case 4:
        return (long long int) 5000000;
    default:
        return (long long int) 5000000; //shortest
    }
}

long long int getAEThreadVsTimeOutLimit_ns(int normalizeM, int fps/*x1000*/)
{
    // you may define your own timeout limit for each fps and each M
    /*
    criterion: as long as possible, but it should satisfy: 
        1. impossible to catch next Update, 
        2. 3A thread has enough time to wait next Vsync
    tolerance: 
        33-6=27ms for 30fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        16-3=13ms for 60fps, -> P1 dump might be short (ex: 10ms), choose 8ms
        11-3=8ms for 90fps, -> might have short-P1-dump issue
        8-3=5ms for 120fps, -> might have short-P1-dump issue
    */
    switch((int)(fps/30000))
    {
    case 0:
        return (long long int) 18000000;
    case 1:
        return (long long int) 18000000;
    case 2:
        return (long long int) 18000000;
    case 3:
        return (long long int) 18000000;
    case 4:
        return (long long int) 5000000;
    default:
        return (long long int) 5000000; //shortest
    }
}


int queryFramesPerCycle_custom(int fps/*x1000*/)
{
    //might be customized
    return (((fps/30000) >= 1) ? (fps/30000) : 1); 
}

int getResetFrameCount(MUINT32 senDevId)
{
    switch(senDevId)
    {
    case 0x01: // SENSOR_DEV_MAIN = 0x01
        return -3;
    case 0x02: // SENSOR_DEV_SUB  = 0x02
        return -3;
    case 0x04: // SENSOR_DEV_MAIN_2 = 0x04
        return -3;
    default:
        return -3; //conservative
    }
}


