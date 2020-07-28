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
package com.mediatek.internal.telephony;


public class MtkSuppServContants {
    /// For SS Easy config
    public static final String SYS_PROP_BOOL_VALUE         = "persist.vendor.ss.cfg.boolvalue";
    public static final String SYS_PROP_BOOL_CONFIG        = "persist.vendor.ss.cfg.boolconfig";

    // If there is a new type of config need to be added, please put it in the end
    public static enum CUSTOMIZATION_ITEM {
        GSM_UT_SUPPORT,
        TBCLIR,
        IMS_NW_CW,
        NOT_SUPPORT_XCAP,
        NOT_SUPPORT_OCB,
        ENABLE_XCAP_HTTP_RESPONSE_409,
        TRANSFER_XCAP_404,
        NOT_SUPPORT_WFC_UT,
        NOT_SUPPORT_CALL_IDENTITY,
        RE_REGISTER_FOR_CF,
        SUPPORT_SAVE_CF_NUMBER,
        QUERY_CFU_AGAIN_AFTER_SET,
        NEED_CHECK_DATA_ENABLE,
        NEED_CHECK_DATA_ROAMING,
        NEED_CHECK_IMS_WHEN_ROAMING
    }

    public static String toString(CUSTOMIZATION_ITEM item) {
        switch (item) {
            case GSM_UT_SUPPORT:
                return "GSM_UT_SUPPORT";
            case NOT_SUPPORT_XCAP:
                return "NOT_SUPPORT_XCAP";
            case TBCLIR:
                return "TBCLIR";
            case IMS_NW_CW:
                return "IMS_NW_CW";
            case ENABLE_XCAP_HTTP_RESPONSE_409:
                return "ENABLE_XCAP_HTTP_RESPONSE_409";
            case TRANSFER_XCAP_404:
                return "TRANSFER_XCAP_404";
            case NOT_SUPPORT_CALL_IDENTITY:
                return "NOT_SUPPORT_CALL_IDENTITY";
            case RE_REGISTER_FOR_CF:
                return "RE_REGISTER_FOR_CF";
            case SUPPORT_SAVE_CF_NUMBER:
                return "SUPPORT_SAVE_CF_NUMBER";
            case QUERY_CFU_AGAIN_AFTER_SET:
                return "QUERY_CFU_AGAIN_AFTER_SET";
            case NOT_SUPPORT_OCB:
                return "NOT_SUPPORT_OCB";
            case NOT_SUPPORT_WFC_UT:
                return "NOT_SUPPORT_WFC_UT";
            case NEED_CHECK_DATA_ENABLE:
                return "NEED_CHECK_DATA_ENABLE";
            case NEED_CHECK_DATA_ROAMING:
                return "NEED_CHECK_DATA_ROAMING";
          case NEED_CHECK_IMS_WHEN_ROAMING:
                return "NEED_CHECK_IMS_WHEN_ROAMING";
            default:
                return "UNKNOWN_ITEM";
        }
    }
}