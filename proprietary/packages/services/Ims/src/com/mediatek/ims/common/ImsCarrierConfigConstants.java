/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.ims.common;

/******************************************************************
  * Constants of IMS CarrierConfigManager's key
  * Copy from MtkImsCarrierConfigManager
  * You need to add/update keys in both
  * ImsCarrierConfigConstants.java & MtkCarrierConfigManager.java
  *****************************************************************/
public interface ImsCarrierConfigConstants {
    /**
     * Determine whether to remove WFC preference mode or not.
     */
    String MTK_KEY_WFC_REMOVE_PREFERENCE_MODE_BOOL =
           "mtk_wfc_remove_preference_mode_bool";

    /**
     * @M: Operator ID
     * AP requires this information to know which operator SIM card is inserted.
     * Default value is 0(OM or no SIM inserted).
     */
    String KEY_OPERATOR_ID_INT = "operator_id";

    /*
     * For Call blokcing enhancement feature, reject call with cause.
     */
    String MTK_KEY_SUPPORT_ENHANCED_CALL_BLOCKING_BOOL = "mtk_support_enhanced_call_blocking_bool";

    /*
     * Indicates if the carrier is without precondition when MT RTT call CRING time, ex. VzW
     */
    String MTK_KEY_MT_RTT_WITHOUT_PRECONDITION_BOOL = "mtk_mt_rtt_without_precondition_bool";

    /*
     * Indicates if carrier supports RTT and Video call switch.
     */
    String MTK_KEY_RTT_VIDEO_SWITCH_SUPPORTED_BOOL = "mtk_rtt_video_switch_supported_bool";
}
