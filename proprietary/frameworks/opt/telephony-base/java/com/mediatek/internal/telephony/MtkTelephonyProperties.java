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

public interface MtkTelephonyProperties
{
    /**
     * PROPERTY_TERMINAL_BASED_CALL_WAITING_MODE is the mode for the terminal-based call waiting
     * possible values: "disabled_tbcw", "enabled_tbcw_on" and "enabled_tbcw_off".
     */
    public static String PROPERTY_TBCW_MODE =
            "persist.vendor.radio.terminal-based.cw";
    public static String TBCW_DISABLED       = "disabled_tbcw";
    public static String TBCW_ON             = "enabled_tbcw_on";
    public static String TBCW_OFF            = "enabled_tbcw_off";
    public static String TBCW_ON_VOLTE_ONLY  = "tbcw_on_volte_only";
    public static String TBCW_OFF_VOLTE_ONLY = "tbcw_off_volte_only";

    /**
     * NITZ operator long name,short name, numeric (if ever received from MM information)
     */
    public static String PROPERTY_NITZ_OPER_CODE = "persist.vendor.radio.nitz_oper_code";
    public static String PROPERTY_NITZ_OPER_LNAME = "persist.vendor.radio.nitz_oper_lname";
    public static String PROPERTY_NITZ_OPER_SNAME = "persist.vendor.radio.nitz_oper_sname";

    /* For solving ALPS01023811
     * To determine if CFU query is for power-on query.
     */
    public static final String CFU_QUERY_ICCID_PROP       = "persist.vendor.radio.cfu.iccid.";
    public static final String CFU_QUERY_SIM_CHANGED_PROP = "persist.vendor.radio.cfu.change.";
    public static final String CFU_QUERY_OVER_IMS         = "persist.vendor.radio.cfu_over_ims";
    public static final String SS_SERVICE_CLASS_PROP      = "vendor.gsm.radio.ss.sc";
    public static final String CFU_STATUS_SYNC_FOR_OTA    = "persist.vendor.radio.cfu.sync_for_ota";

    /* CFU query type */
    public static final String CFU_QUERY_TYPE_PROP      = "persist.vendor.radio.cfu.querytype";
    public static final String CFU_QUERY_TYPE_DEF_VALUE = "0";

    /**
     * PROPERTY_UT_CFU_NOTIFICATION_MODE is the mode for the UT/XCAP CFU notification
     * possible values: "disabled_ut_cfu_notification", "enabled_ut_cfu_notification_on"
     * and "enabled_ut_cfu_notification_off".
     */
    public static String PROPERTY_UT_CFU_NOTIFICATION_MODE = "persist.vendor.radio.cfu.mode";
    public static String UT_CFU_NOTIFICATION_MODE_DISABLED = "disabled_cfu_mode";
    public static String UT_CFU_NOTIFICATION_MODE_ON       = "enabled_cfu_mode_on";
    public static String UT_CFU_NOTIFICATION_MODE_OFF      = "enabled_cfu_mode_off";

    /**
     * PROPERTY_ERROR_MESSAGE_FROM_XCAP would keep the XCAP error message from XCAP server.
     */
    public static String PROPERTY_ERROR_MESSAGE_FROM_XCAP = "vendor.gsm.radio.ss.errormsg";

    // MTK-START: MVNO
    /** PROPERTY_ICC_OPERATOR_DEFAULT_NAME is the operator name for plmn which origins the SIM.
     *  Availablity: SIM state must be "READY"
     */
    static String PROPERTY_ICC_OPERATOR_DEFAULT_NAME = "vendor.gsm.sim.operator.default-name";
    // MTK-END

    public static final String PROPERTY_ACTIVE_MD = "vendor.ril.active.md";

    // External SIM [START]
    /**
     * External SIM enabled properties.
     */
    static final String PROPERTY_EXTERNAL_SIM_ENABLED = "vendor.gsm.external.sim.enabled";

    /**
     * External SIM inserted properties.
     * 1: local SIM inserted, 2: remote sim inserted
     */
    static final String PROPERTY_EXTERNAL_SIM_INSERTED = "vendor.gsm.external.sim.inserted";

    /**
     * External SIM inserted persist properties.
     * 1: enabled persist, 0: disable persist vsim
     */
    static final String PROPERTY_PERSIST_EXTERNAL_SIM = "persist.vendor.radio.external.sim";

    /**
     * External SIM inserted persist timeout properties.
     */
    static final String PROPERTY_PERSIST_EXTERNAL_SIM_TIMEOUT = "persist.vendor.radio.vsim.timeout";

    /**
     * Prefered Remote SIM slot id.
     * 0: slot1, 1: slot2, 2: slot3
     */
    static final String PROPERTY_PREFERED_REMOTE_SIM = "vendor.gsm.prefered.rsim.slot";

    /**
     * Prefered AKA SIM slot id.
     * 0: slot1, 1: slot2, 2: slot3
     */
    static final String PROPERTY_PREFERED_AKA_SIM = "vendor.gsm.prefered.aka.sim.slot";

    /**
     * External SIM time out properties.
     */
    static final String PROPERTY_EXTERNAL_SIM_TIMEOUT = "vendor.gsm.external.sim.timeout";

    /**
     * External SIM enable/disable preferred sim dialog.
     * 0: Enable, 1: Disable
     */
    static final String PROPERTY_EXTERNAL_DISABLE_SIM_DIALOG = "vendor.gsm.disable.sim.dialog";
    // External SIM [END]

    /**
     * Indicate if each phone is in emergency callback mode splited by ','.
     */
    static final String PROPERTY_INECM_MODE_BY_SLOT = "vendor.ril.cdma.inecmmode_by_slot";
}
