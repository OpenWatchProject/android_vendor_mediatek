/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <isp_drv_cam.h> //for cfg the test pattern of seninf only
#include <isp_drv_camsv.h>
#include "main_DrvKernelCtrl.h"


int DrvKernelCtrl()
{
#define CAM_LOOP_NUM       (3)
#define CAMSV_LOOP_NUM   (6)

    int ret = 0, i = 0, data = 0;
    ISP_DRV_REG_IO_STRUCT* Regs_W;
    ISP_DRV_REG_IO_STRUCT* Regs_R;
    MUINT32 bFail = MFALSE;
    const char *drvCamName[CAM_LOOP_NUM] = {"Test_IspDrvCam_A", "Test_IspDrvCam_B", "Test_IspDrvCam_C"};
    ISP_DRV_CAM* drvCam[CAM_LOOP_NUM];
    ISP_DRV_CAM* ptr = NULL;

    const char *drvCamSVName[CAMSV_LOOP_NUM] = {"Test_IspDrvCamSV_0", "Test_IspDrvCamSV_1", "Test_IspDrvCamSV_2", "Test_IspDrvCamSV_3", "Test_IspDrvCamSV_4", "Test_IspDrvCamSV_5"};
    IspDrvCamsv *drvCamSV[CAMSV_LOOP_NUM];

    for (i = CAM_A; i < CAM_LOOP_NUM; i++) {
        drvCam[i] = (ISP_DRV_CAM*)ISP_DRV_CAM::createInstance((ISP_HW_MODULE)i,ISP_DRV_CQ_THRE0,0,drvCamName[i]);
    }

    for (i = 0; i < CAM_LOOP_NUM; i++) {
        if(drvCam[i] == NULL){
            printf("CAM_A create fail\n");
            return -1;
        }

        if(drvCam[i]->init(drvCamName[i]) == MFALSE){
            drvCam[i]->destroyInstance();
            printf("CAM_%c init failure\n", 'A'+i);
            return -1;
        }
    }

    for (i = CAMSV_0; i < (CAMSV_LOOP_NUM + CAMSV_0); i++) {
        drvCamSV[(ISP_HW_MODULE)i - CAMSV_0] = (IspDrvCamsv*)IspDrvCamsv::createInstance((ISP_HW_MODULE)i);
    }

    for (i = 0; i < CAMSV_LOOP_NUM; i++) {
        if(drvCamSV[i] == NULL){
            printf("CAMSV_%d create fail\n", i);
            return -1;
        }

        if(drvCamSV[i]->init(drvCamSVName[i]) == MFALSE){
            drvCamSV[i]->destroyInstance();
            printf("CAMSV_%d init failure\n", i);
            return -1;
        }
    }

    for (i = 0; i < CAM_LOOP_NUM; i++) {
        struct ISP_MULTI_RAW_CONFIG multiRAWConfig;

        if (i == CAM_A) {
            multiRAWConfig.master_module = CAM_A;
            multiRAWConfig.slave_cam_num = 1;
            multiRAWConfig.twin_module = CAM_B;
            multiRAWConfig.cq_base_pAddr = 0xaaaaaaaa;
        } else if (i == CAM_B) {
            multiRAWConfig.cq_base_pAddr = 0xbbbbbbbb;
        }

        drvCam[i]->setDeviceInfo(_SET_VF_ON, (MUINT8*)NULL);
        drvCam[i]->setDeviceInfo(_SET_VF_OFF, (MUINT8*)NULL);
        drvCam[i]->setDeviceInfo(_RESET_VSYNC_CNT, (MUINT8*)NULL);
        drvCam[i]->getDeviceInfo(_GET_DROP_FRAME_STATUS,(MUINT8 *)&data);
        drvCam[i]->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&data);
        drvCam[i]->getDeviceInfo(_GET_VSYNC_CNT,(MUINT8 *)&data);
        drvCam[i]->getDeviceInfo(ISP_CQ_SW_PATCH,(MUINT8 *)&multiRAWConfig);
    }

    for (i = 0; i < CAMSV_LOOP_NUM; i++) {
        drvCamSV[i]->uninit(drvCamSVName[i]);
        drvCamSV[i]->destroyInstance();
    }

    for (i = 0; i < CAM_LOOP_NUM; i++) {
        drvCam[i]->uninit(drvCamName[i]);
        drvCam[i]->destroyInstance();
    }

    return ret;
}


