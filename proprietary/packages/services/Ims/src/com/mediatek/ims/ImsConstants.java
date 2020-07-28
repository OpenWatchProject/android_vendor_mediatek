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

package com.mediatek.ims;

/**
 * Contains IMS constants.
 *
 */
public class ImsConstants {
    public static final String ACTION_LTE_MESSAGE_WAITING_INDICATION =
            "mediatek.intent.action.lte.mwi";
    public static final String ACTION_IMS_CONFERENCE_CALL_INDICATION =
            "android.intent.action.ims.conference";
    public static final String ACTION_IMS_DIALOG_EVENT_PACKAGE =
            "com.mediatek.intent.action.ims.dialogEventPackage";
    public static final String PERMISSION_READ_LTE_MESSAGE_WAITING_INDICATION =
            "com.mediatek.permission.READ_LTE_MESSAGE_WAITING_INDICATION";
    public static final String PACKAGE_NAME_PHONE = "com.android.phone";

    public static final String EXTRA_LTE_MWI_BODY = "lte_mwi_body";
    public static final String EXTRA_MESSAGE_CONTENT = "message.content";
    public static final String EXTRA_CALL_ID = "call.id";
    public static final String EXTRA_PHONE_ID = "phone.id";

    public static final String EXTRA_DEP_CONTENT = "com.mediatek.intent.extra.DEP_CONTENT";

    public static final String SYS_PROP_MD_AUTO_SETUP_IMS = "ro.vendor.md_auto_setup_ims";
    public static final String SELF_IDENTIFY_UPDATE =
            "com.mediatek.ims.action.self_identify_update";
    /// md multi ims
    public static final String PROPERTY_MD_MULTI_IMS_SUPPORT = "ro.vendor.md_mims_support";

    public static final String PROPERTY_CAPABILITY_SWITCH = "persist.vendor.radio.simswitch";

    public static String PROPERTY_TBCW_MODE = "persist.vendor.radio.terminal-based.cw";
    public static String TBCW_DISABLED      = "disabled_tbcw";
    public static String TBCW_OFF           = "enabled_tbcw_off";

    //VOLTE IMS STATE
    public static final int IMS_STATE_DISABLED = 0;
    public static final int IMS_STATE_ENABLE = 1;
    public static final int IMS_STATE_ENABLING = 2;
    public static final int IMS_STATE_DISABLING = 3;
}
