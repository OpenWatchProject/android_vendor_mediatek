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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _MTK_TNR_H
#define _MTK_TNR_H

#include "MTKTnrType.h"
#include "MTKTnrErrCode.h"

//#define TNR_DEBUG

typedef enum
{
    TNR_STATE_STANDBY,
    TNR_STATE_INIT,
    TNR_STATE_PROC,
    TNR_STATE_FINISH,
    TNR_STATE_IDLE
}TNR_STATE_ENUM;


typedef enum
{
    TNR_FEATURE_BEGIN = 0,
    TNR_FEATURE_SET_PROC_INFO,
    TNR_FEATURE_GET_PROC_INFO,
    TNR_FEATURE_SET_WORK_BUF_INFO,
    TNR_FEATURE_GET_RESULT_INFO,
    TNR_FEATURE_SET_DEBUG_INFO,
    TNR_FEATURE_GET_TNR_STATE,
    TNR_FEATURE_SAVE_LOG,
    TNR_FEATURE_MAX
} TNR_FEATURE_ENUM;


typedef struct
{
    void* extMemStartAddr; //working buffer start address
    MUINT32 extMemSize;
} TNR_SET_WORKING_BUFFER_STRUCT, *P_TNR_SET_WORKING_BUFFER_STRUCT;


typedef struct
{
    MBOOL GyroValid;
    MBOOL en_gyro_fusion;
    MUINT8 GMV_mode;
    MBOOL debug;
    MINT32 gyroMVWidth;
    MINT32 gyroMVHeight;
    MINT32 confThrL;
    MINT32 confThrH;
} TNR_SET_ENV_INFO_STRUCT, *P_TNR_SET_ENV_INFO_STRUCT;

typedef struct
{
    MINT32 lmv_gmv_conf[2];
    MFLOAT lmv_gmv[2];
    MINT32 lmv_max_gmv[2];
}LMV_GMV_INFO_STRUCT, *P_LMV_GMV_INFO_STRUCT;

typedef struct
{
    MUINT8 *gyro_in_mv;
    MUINT8 *valid_gyro_num;
}GYRO_INFO_STRUCT, *P_GYRO_INFO_STRUCT;

typedef struct
{
    MUINT8 *fbuf_in_rsc_mv;
    MUINT8 *fbuf_in_rsc_var;
}RSC_INFO_STRUCT, *P_RSC_INFO_STRUCT;

typedef struct
{
    MINT32 tnr_gmv_int[2];
    MFLOAT tnr_gmv_float[2];
    MINT32 tnr_gmv_conf[2];
    MINT32 tnr_max_gmv[2];
}TNR_RESULT_INFO_STRUCT, *P_TNR_RESULT_INFO_STRUCT;

typedef struct
{
    LMV_GMV_INFO_STRUCT lmv_info;
    GYRO_INFO_STRUCT gyro_info;
    RSC_INFO_STRUCT rsc_info;
    TNR_RESULT_INFO_STRUCT gmv_result_info;
    MINT32 RSSoWidth;
    MINT32 RSSoHeight;
    MINT32 imgiWidth;
    MINT32 imgiHeight;
    MINT32 CRZoWidth;
    MINT32 CRZoHeight;
    MINT32 AfCropWidth;
    MINT32 AfCropHeight;
    MINT32 AfResizeWidth;
    MINT32 AfResizeHeight;
    MUINT64 frame_t;
    MINT32 FrameUniqueKey;
    MINT32 FrameNumber;
    MINT32 FrameRequestNumber;
}TNR_SET_PROC_INFO_STRUCT, *P_TNR_SET_PROC_INFO_STRUCT;

typedef struct
{
    MUINT32 ext_mem_size;
}TNR_GET_PROC_INFO_STRUCT, *P_TNR_GET_PROC_INFO_STRUCT;


class MTKTnr
{
public:
    static MTKTnr* createInstance();
    virtual void   destroyInstance(MTKTnr* obj) = 0;

    virtual ~MTKTnr(){};
    // Process Control
    virtual MRESULT TnrInit(void* InitInData);
    virtual MRESULT TnrMain(TNR_RESULT_INFO_STRUCT *TnrResult);
    virtual MRESULT TnrReset();

    // Feature Control
    virtual MRESULT TnrFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);

private:
};

#endif
