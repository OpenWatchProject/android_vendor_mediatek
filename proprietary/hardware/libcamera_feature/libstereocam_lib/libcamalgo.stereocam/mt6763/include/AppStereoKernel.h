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

#include "StereoKernel_Core.h"

#ifndef _APP_STEREO_KERNEL_H_
#define _APP_STEREO_KERNEK_H_
#define STEREO_KERNEL_LOG_BUFFER_SIZE 1024000

typedef enum
{
    STEREO_KERNEL_STATE_STANDBY,
    STEREO_KERNEL_STATE_INIT,
    STEREO_KERNEL_STATE_PROC,
    STEREO_KERNEL_STATE_READY,
    STEREO_KERNEL_STATE_IDLE,
    STEREO_KERNEL_STATE_MAX
}
STEREO_KERNEL_STATE_ENUM ;


class AppStereoKernel : public MTKStereoKernel {
public:
    static MTKStereoKernel* getInstance();
    virtual void destroyInstance();

    AppStereoKernel();
    virtual ~AppStereoKernel();
    // Process Control
    MRESULT StereoKernelInit(void* InitData, void* TuneData);
    MRESULT StereoKernelMain();
    MRESULT StereoKernelReset();   //Reset
    int StereoKernelAlignment(void* arg);
    // Feature Control
    MRESULT StereoKernelFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

	MTKStereoKernelCore* pStereoKernelCoreObj;
	MUINT32 input_stage;
	MUINT32 main_stage;
	MTKStereoKernel *pAppStereoKernel;
	STEREO_KERNEL_STATE_ENUM gStereoKernelState;

    STEREO_KERNEL_SET_ENV_INFO_STRUCT gStereoKernelInitInfo;
    MUINT32 gStereoKernelSensorConfig ;
    MUINT32 gStereoKernelEnableGPU ;
    MUINT32 gStereoKernelEnableCc  ;
    MUINT32 gStereoKernelEnableLDC  ;
    /*
    // no need
    STEREO_KERNEL_SET_WORK_BUF_INFO_STRUCT gStereoCoreBufInfo;
	STEREO_KERNEL_SET_ENV_INFO_STRUCT gStereoCoreInitInfo;
	STEREO_KERNEL_TUNING_PARA_STRUCT gStereoCoreTuningInfo;
    */

	char StereoKernelLogFileName[100];
	MUINT32* StereoKernelLogBuffer;

	EGLClientBuffer clientBuffer;

#ifndef PC_SIM
	TEXTURE_PARA_STRUCT InputTexStruct[MAX_FRAME_NUM];
	TEXTURE_PARA_STRUCT OutputTexStruct[MAX_FRAME_NUM];
#endif

	int dynamic_buffer_num;
	int frame_count; 		  //input buffer count
	int output_count;		  //output buffer count
	pthread_mutex_t frame_count_mutex;
	pthread_mutex_t output_count_mutex;

	STEREO_KERNEL_SET_PROC_INFO_STRUCT gStereoKernelProcInfo[MAX_FRAME_NUM];
	STEREO_KERNEL_RESULT_STRUCT gStereoKernelResult[MAX_FRAME_NUM];

	MUINT32 dumpCnt2;
	MUINT32 dumpCnt;
};


#endif
