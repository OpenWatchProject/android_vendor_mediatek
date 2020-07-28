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
package com.mediatek.internal.telephony.worldphone;

public interface IWorldPhone {
    public static final String LOG_TAG = "WORLDMODE";

    public static final int POLICY_OM       = 0;
    public static final int POLICY_OP01     = 1;

    static final int SELECTION_MODE_MANUAL = 0;
    static final int SELECTION_MODE_AUTO   = 1;

    static final int UNKNOWN_USER = 0;
    static final int TYPE1_USER   = 1;
    static final int TYPE2_USER   = 2;
    static final int TYPE3_USER   = 3;

    static final int REGION_UNKNOWN  = 0;
    static final int REGION_DOMESTIC = 1;
    static final int REGION_FOREIGN  = 2;

    static final int EVENT_RADIO_ON_1 = 0;
    static final int EVENT_RADIO_ON_2 = 1;
    static final int EVENT_RADIO_ON_3 = 2;
    static final int EVENT_RADIO_ON_4 = 3;
    static final int EVENT_REG_PLMN_CHANGED_1 = 10;
    static final int EVENT_REG_PLMN_CHANGED_2 = 11;
    static final int EVENT_REG_PLMN_CHANGED_3 = 12;
    static final int EVENT_REG_PLMN_CHANGED_4 = 13;
    static final int EVENT_REG_SUSPENDED_1    = 30;
    static final int EVENT_REG_SUSPENDED_2    = 31;
    static final int EVENT_REG_SUSPENDED_3    = 32;
    static final int EVENT_REG_SUSPENDED_4    = 33;
    static final int EVENT_STORE_MODEM_TYPE   = 40;
    static final int EVENT_QUERY_MODEM_TYPE   = 50;
    static final int EVENT_INVALID_SIM_NOTIFY_1 = 60;
    static final int EVENT_INVALID_SIM_NOTIFY_2 = 61;
    static final int EVENT_INVALID_SIM_NOTIFY_3 = 62;
    static final int EVENT_INVALID_SIM_NOTIFY_4 = 63;
    static final int EVENT_RESUME_CAMPING_1       = 70;   //[ALPS01974750]
    static final int EVENT_RESUME_CAMPING_2       = 71;
    static final int EVENT_RESUME_CAMPING_3       = 72;
    static final int EVENT_RESUME_CAMPING_4       = 73;
    static final int EVENT_SERVICE_STATE_CHANGED_1 = 80;
    static final int EVENT_SERVICE_STATE_CHANGED_2 = 81;
    static final int EVENT_SERVICE_STATE_CHANGED_3 = 82;
    static final int EVENT_SERVICE_STATE_CHANGED_4 = 83;
    static final int EVENT_WP_GMSS_RAT_CHANGED_1      = 1060;  //[ALPS02156732]
    static final int EVENT_WP_GMSS_RAT_CHANGED_2      = 1061;  //[ALPS02156732]
    static final int EVENT_WP_GMSS_RAT_CHANGED_3      = 1062;  //[ALPS02156732]
    static final int EVENT_WP_GMSS_RAT_CHANGED_4      = 1063;  //[ALPS02156732]

    static final int DEFAULT_MAJOR_SIM    = 0;
    static final int MAJOR_CAPABILITY_OFF = -1;
    static final int AUTO_SWITCH_OFF      = -98;
    static final int MAJOR_SIM_UNKNOWN    = -99;

    static final int CAMP_ON_NOT_DENIED                     = 0;
    static final int CAMP_ON_DENY_REASON_UNKNOWN            = 1;
    static final int CAMP_ON_DENY_REASON_NEED_SWITCH_TO_FDD = 2;
    static final int CAMP_ON_DENY_REASON_NEED_SWITCH_TO_TDD = 3;
    static final int CAMP_ON_DENY_REASON_DOMESTIC_FDD_MD    = 4;

    static final int ICC_CARD_TYPE_UNKNOWN = 0;
    static final int ICC_CARD_TYPE_SIM     = 1;
    static final int ICC_CARD_TYPE_USIM    = 2;

    //switch modem cause type
    static final int CAUSE_TYPE_PLMN_CHANGE = 0;
    static final int CAUSE_TYPE_OOS   = 1;
    static final int CAUSE_TYPE_OTHERS   = 255; //default value, EM, SIM switch,CU/CT card etc

    static final String NO_OP = "OM";
    static final String ACTION_SHUTDOWN_IPO = "mediatek.intent.action.ACTION_SHUTDOWN_IPO";
    static final String ACTION_ADB_SWITCH_MODEM = "mediatek.intent.action.ACTION_ADB_SWITCH_MODEM";
    static final String PROPERTY_SWITCH_MODEM_CAUSE_TYPE = "vendor.ril.switch.modem.cause.type";

    //[ALPS02302039]
    static final String ACTION_SAP_CONNECTION_STATE_CHANGED =
        "android.bluetooth.sap.profile.action.CONNECTION_STATE_CHANGED";

    //For Test
    static final String ACTION_TEST_WORLDPHONE = "mediatek.intent.action.ACTION_TEST_WORLDPHOE";
    static final String EXTRA_FAKE_USER_TYPE = "FAKE_USER_TYPE";
    static final String EXTRA_FAKE_REGION = "EXTRA_FAKE_REGION";

    public static final String PROPERTY_MAJOR_SIM = "persist.vendor.radio.simswitch";

    /**
    * World phone auto network select mode.
    */
    public static final String WORLD_PHONE_AUTO_SELECT_MODE = "persist.vendor.radio.wm_selectmode";

    /**
    * World phone fdd modem standby timer.
    */
    public static final String WORLD_PHONE_FDD_MODEM_TIMER = "persist.vendor.radio.wm_fddtimer";

    public void setModemSelectionMode(int mode, int modemType);
    public void notifyRadioCapabilityChange(int capailitySimId);
}

