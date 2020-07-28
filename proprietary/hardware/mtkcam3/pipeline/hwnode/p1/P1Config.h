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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONFIG_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONFIG_H_

/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_SUPPORT_3A
#define P1NODE_SUPPORT_3A (1)
#undef  P1NODE_SUPPORT_ISP
#define P1NODE_SUPPORT_ISP (1)
#undef  P1NODE_SUPPORT_PERFRAME_CTRL
#define P1NODE_SUPPORT_PERFRAME_CTRL (0)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_SUPPORT_LCS
#if (MTKCAM_HW_NODE_LCS_SUPPORT > 0)
#define P1NODE_SUPPORT_LCS (1)
#else
#define P1NODE_SUPPORT_LCS (0)
#endif

#undef  P1NODE_SUPPORT_RSS
#define P1NODE_SUPPORT_RSS (1)
#undef  P1NODE_SUPPORT_FSC
#define P1NODE_SUPPORT_FSC (1)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_SUPPORT_VHDR
#if (MTKCAM_HW_NODE_VHDR_SUPPORT > 0)
#define P1NODE_SUPPORT_VHDR (1)
#else
#define P1NODE_SUPPORT_VHDR (0)
#endif


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_SUPPORT_RRZ_DST_CTRL
#define P1NODE_SUPPORT_RRZ_DST_CTRL (1)
//
#undef  P1NODE_SUPPORT_CONFIRM_BUF_PA
#define P1NODE_SUPPORT_CONFIRM_BUF_PA (1)
#undef  P1NODE_SUPPORT_CONFIRM_BUF_PA_VA
#define P1NODE_SUPPORT_CONFIRM_BUF_PA_VA (P1NODE_SUPPORT_CONFIRM_BUF_PA && (0))


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_SUPPORT_BUFFER_TUNING_DUMP
#if (MTKCAM_HW_NODE_USING_TUNING_UTILS > 0)
#define P1NODE_SUPPORT_BUFFER_TUNING_DUMP (1)
#else
#define P1NODE_SUPPORT_BUFFER_TUNING_DUMP (0)
#endif


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_USING_MTK_LDVT
#ifdef USING_MTK_LDVT
#define P1NODE_USING_MTK_LDVT (1)
#else
#define P1NODE_USING_MTK_LDVT (0)
#endif


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_USING_CTRL_3A_LIST
#if (MTKCAM_HW_NODE_USING_3A_LIST > 0)
#define P1NODE_USING_CTRL_3A_LIST (1)
#else
#define P1NODE_USING_CTRL_3A_LIST (0)
#endif
#define P1NODE_USING_CTRL_3A_LIST_PREVIOUS (P1NODE_USING_CTRL_3A_LIST && (1))//USING_PREVIOUS_3A_LIST


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_USING_DRV_SET_RRZ_CBFP_EXP_SKIP
#define P1NODE_USING_DRV_SET_RRZ_CBFP_EXP_SKIP (1)
#undef  P1NODE_USING_DRV_SET_CRZO_R1_CBFP_EXP_SKIP
#define P1NODE_USING_DRV_SET_CRZO_R1_CBFP_EXP_SKIP (1)
#undef  P1NODE_USING_DRV_SET_CRZO_R2_CBFP_EXP_SKIP
#define P1NODE_USING_DRV_SET_CRZO_R2_CBFP_EXP_SKIP (1)
#undef  P1NODE_USING_DRV_QUERY_CAPABILITY_EXP_SKIP
#define P1NODE_USING_DRV_QUERY_CAPABILITY_EXP_SKIP (1)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_USING_DRV_CB_CRZO_SUBSAMPLE_WITH_FULLY_STUFF
#define P1NODE_USING_DRV_CB_CRZO_SUBSAMPLE_WITH_FULLY_STUFF (1)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_USING_DRV_IO_PIPE_EVENT
#define P1NODE_USING_DRV_IO_PIPE_EVENT (1)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_ENABLE_CHECK_CONFIG_COMMON_PORPERTY
#define P1NODE_ENABLE_CHECK_CONFIG_COMMON_PORPERTY (0)


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_HAVE_AEE_FEATURE
#if (HWNODE_HAVE_AEE_FEATURE > 0)
#define P1NODE_HAVE_AEE_FEATURE (1)
#else
#define P1NODE_HAVE_AEE_FEATURE (0)
#endif


/******************************************************************************
 *
 ******************************************************************************/
#undef  P1NODE_BUILD_LOG_LEVEL_DEFAULT
#if (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 3)
#define P1NODE_BUILD_LOG_LEVEL_DEFAULT (4)  // for ENG build
#elif   (MTKCAM_HW_NODE_LOG_LEVEL_DEFAULT > 2)
#define P1NODE_BUILD_LOG_LEVEL_DEFAULT (3)  // for USERDEBUG build
#else
#define P1NODE_BUILD_LOG_LEVEL_DEFAULT (2)  // for USER build
#endif


#endif//_MTK_HARDWARE_INCLUDE_MTKCAM_PIPELINE_HWNODE_P1_CONFIG_H_

