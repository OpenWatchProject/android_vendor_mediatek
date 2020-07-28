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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.common.carrierexpress;


/**
 * CarrierExpressManager.
 */
public class CarrierExpressManager {
    private static final String TAG = "CarrierExpressManager";
    private static final boolean DBG = true;

    /**
    * For accessing the carrierexpress service.
    * Internal use only.
    * @hide
    */
    public static final String CARRIEREXPRESS_SERVICE = "carrierexpress";

    public static final String ACTION_USER_NOTIFICATION_INTENT =
            "com.mediatek.common.carrierexpress.action.user_notification_intent";
    public static final String ACTION_OPERATOR_CONFIG_CHANGED =
            "com.mediatek.common.carrierexpress.operator_config_changed";
    public static final String ACTION_CXP_RESET_MODEM =
            "com.mediatek.common.carrierexpress.cxp_reset_modem";
    public static final String ACTION_CXP_NOTIFY_FEATURE =
            "com.mediatek.common.carrierexpress.cxp_notify_feature";
    public static final String ACTION_CXP_SET_VENDOR_PROP =
            "com.mediatek.common.carrierexpress.cxp_set_vendor_prop";

    private static CarrierExpressManager sInstance = null;

    // current state of global device configuration
    // configuration can start only when state is idle
    public static final int SRV_CONFIG_STATE_INIT = 0;
    public static final int SRV_CONFIG_STATE_WAIT = 1;
    public static final int SRV_CONFIG_STATE_IDLE = 2;

    // Operator Orange's sub IDs
    public static final int OPERATOR_OP03_SUBID_DEFAULT = 0;
    public static final int OPERATOR_OP03_SUBID_1 = 1;
    public static final int OPERATOR_OP03_SUBID_2 = 2;
    public static final int OPERATOR_OP03_SUBID_3 = 3;
    public static final int OPERATOR_OP03_SUBID_4 = 4;
    public static final int OPERATOR_OP03_SUBID_5 = 5;

}
