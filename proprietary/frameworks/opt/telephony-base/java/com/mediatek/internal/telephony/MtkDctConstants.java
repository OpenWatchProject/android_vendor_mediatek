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

import com.android.internal.telephony.DctConstants;

public class MtkDctConstants extends DctConstants {
    /** M: start */
    public static final int MTK_BASE = BASE + 500;
    public static final int EVENT_DATA_ROAMING_CHANGED = MTK_BASE + 1;
    public static final int EVENT_INIT_EMERGENCY_APN_SETTINGS = MTK_BASE + 2;
    public static final int EVENT_APN_CHANGED_DONE = MTK_BASE + 3;
    public static final int EVENT_FDN_CHANGED = MTK_BASE + 4;
    public static final int EVENT_RESET_PDP_DONE = MTK_BASE + 5;
    public static final int EVENT_SETUP_PENDING_DATA = MTK_BASE + 6;
    public static final int EVENT_ATTACH_APN_CHANGED = MTK_BASE + 7;
    public static final int EVENT_REG_PLMN_CHANGED = MTK_BASE + 8;
    public static final int EVENT_REG_SUSPENDED = MTK_BASE + 9;
    public static final int EVENT_SET_RESUME = MTK_BASE + 10;
    public static final int EVENT_RESET_ATTACH_APN = MTK_BASE + 11;
    // M: [LTE][Low Power][UL traffic shaping] @{
    public static final int EVENT_DEFAULT_APN_REFERENCE_COUNT_CHANGED = MTK_BASE + 12;
    public static final int EVENT_LTE_ACCESS_STRATUM_STATE = MTK_BASE + 13;
    // M: [LTE][Low Power][UL traffic shaping] @}
    public static final int EVENT_ROAMING_TYPE_CHANGED = MTK_BASE + 14;
    // M: Data Framework - Data Retry enhancement
    public static final int EVENT_MD_DATA_RETRY_COUNT_RESET = MTK_BASE + 15;
    // M: Data Framework - CC 33
    public static final int EVENT_REMOVE_RESTRICT_EUTRAN = MTK_BASE + 16;
    // M: Multi-PS Attach
    public static final int EVENT_DATA_ALLOWED = MTK_BASE + 17;
    // M: For CarrierConfigLoader timing issue.
    public static final int EVENT_CARRIER_CONFIG_LOADED = MTK_BASE + 18;
    // M: Data Enabled Settings
    public static final int EVENT_DATA_ENABLED_SETTINGS = MTK_BASE + 19;

    public static final int EVENT_DATA_ATTACHED_PCO_STATUS = MTK_BASE + 20;

    /// Ims Data Framework @{
    public static final int EVENT_DEDICATED_BEARER_ACTIVATED = MTK_BASE + 21;
    public static final int EVENT_DEDICATED_BEARER_MODIFIED = MTK_BASE + 22;
    public static final int EVENT_DEDICATED_BEARER_DEACTIVATED = MTK_BASE + 23;
    /// @}

    public static final int EVENT_CHECK_FDN_LIST = MTK_BASE + 24;
    public static final int EVENT_RIL_CONNECTED = MTK_BASE + 25;
    public static final int EVENT_NETWORK_REJECT = MTK_BASE + 26;

    public static final int CMD_TEAR_DOWN_PDN_BY_TYPE = MTK_BASE + 27;

    /// M: SSC Mode 3 @{
    public static final int EVENT_DATA_SETUP_SSC_MODE3 = MTK_BASE + 28;
    /// @}
    public static final int EVENT_MODEM_RESET = MTK_BASE + 29;
    public static final int EVENT_IMSI_QUERY_DONE = MTK_BASE + 30;

    public static final int EVENT_MOBILE_DATA_USAGE = MTK_BASE + 31;
    public static final int EVENT_RECORDS_OVERRIDE = MTK_BASE + 32;
    public static final int EVENT_NW_LIMIT_STATE = MTK_BASE + 33;
}
