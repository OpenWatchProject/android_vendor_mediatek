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

#ifndef _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_AF_DEFINE_H_
#define _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_AF_DEFINE_H_

#define DBG_MSG_BUF_SZ 1024 //byte

#define DEBUG_LEVEL_DEFAULT 0 // defautl log
#define DEBUG_LEVEL_FLOW    1 // + per frame log (essential flow)
#define DEBUG_LEVEL_VERBOSE 2 // + verbose log (all flow)
#define DEBUG_LEVEL_STT     3 // + statistic data

#define LEVEL_FLOW    (m_i4DgbLogLv>=DEBUG_LEVEL_FLOW)
#define LEVEL_VERBOSE (m_i4DgbLogLv>=DEBUG_LEVEL_VERBOSE)
#define LEVEL_STT     (m_i4DgbLogLv>=DEBUG_LEVEL_STT)


#define FUNC_START_DEV(X)  CAM_LOGD_IF(X, "%s Dev(%d) +", __FUNCTION__, m_i4CurrSensorDev);
#define FUNC_END_DEV(X)    CAM_LOGD_IF(X, "%s, Dev(%d) -", __FUNCTION__, m_i4CurrSensorDev);

#define FUNC_START(X)   CAM_LOGD_IF(X, "%s +", __FUNCTION__);
#define FUNC_END(X)     CAM_LOGD_IF(X, "%s -", __FUNCTION__);


enum EAFMgrCtrl_T
{
    EAFMgrCtrl_GetIsSMVR     = 0,
    EAFMgrCtrl_SetSMVRMode   = 1,
    EAFMgrCtrl_NotifyPreStop = 2,
    EAFMgrCtrl_SetHighFPSCCUSuspend = 3,
    EAFMgrCtrl_CheckFPSCCUResume    = 4,
    EAFMgrCtrl_SetFrameRateInfo     = 5,
    EAFMgrCtrl_GetFrameRateInfo     = 6,
    EAFMgrCtrl_GetFocusValue        = 7,
    EAFMgrCtrl_IsCCUSupport         = 8,
    EAFMgrCtrl_IsAFSuspend          = 9,
    EAFMgrCtrl_GetFSCInitInfo,
    EAFMgrCtrl_GetFSCInfo,
    EAFMgrCtrl_SetSecureFlag,
    EAFMgrCtrl_GetSecureFlag,

};

enum EAFConfigCtrl_T
{
	EAFConfigCtrl_GetAFConfigLatency    = 0,
    EAFConfigCtrl_SetTimeReadOutPerLine = 1,
    EAFConfigCtrl_IsZoomEffectChange    = 2,
    EAFConfigCtrl_SetHighFPSCCUSuspend  = 3,
    EAFConfigCtrl_CheckFPSCCUResume     = 4,
    EAFConfigCtrl_SetFrameRateInfo      = 5,
    EAFConfigCtrl_GetFrameRateInfo      = 6,

    EAFConfigCtrl_IsCCUSupport          = 8,
    EAFConfigCtrl_IsAFSuspend           = 9,
    EAFConfigCtrl_IsAF_DSSupport        = 10,
};

typedef enum
{
    AfCommand_Idle = 0,
    AfCommand_Start,
    AfCommand_Cancel,
} eAFControl;


#endif
