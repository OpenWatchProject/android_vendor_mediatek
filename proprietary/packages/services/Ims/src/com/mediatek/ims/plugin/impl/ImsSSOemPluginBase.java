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

package com.mediatek.ims.plugin.impl;

import android.util.Log;
import android.content.Context;
import android.os.SystemProperties;
import android.telephony.ims.ImsReasonInfo;

import com.android.internal.telephony.CommandException;

import com.mediatek.ims.ImsUtImpl;
import com.mediatek.ims.MtkImsReasonInfo;
import com.mediatek.ims.plugin.ImsSSOemPlugin;

import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_VOICE;

/**
 * Default Implementation of interface ImsCallPlugin
 */
public class ImsSSOemPluginBase implements ImsSSOemPlugin {

    private static final String TAG = "ImsSSOemPluginBase";
    private static final boolean DBG = true;

    private Context mContext;

    public static final String ERROR_MSG_PROP_PREFIX = "vendor.gsm.radio.ss.errormsg.";

    public ImsSSOemPluginBase(Context context) {
        mContext = context;
    }

    public ImsReasonInfo commandExceptionToReason(CommandException commandException, int phoneId) {
        CommandException.Error err = null;
        ImsReasonInfo reason = null;

        err = commandException.getCommandError();

        if (DBG) {
            Log.d(TAG, "commandException: " + err);
        }

        /**
         * CME_409_CONFLICT                 CommandException.Error.OEM_ERROR_25
         * CME_403_FORBIDDEN                CommandException.Error.OEM_ERROR_2
         * CME_NETWORK_TIMEOUT              CommandException.Error.OEM_ERROR_3
         * CME_404_NOT_FOUND                CommandException.Error.OEM_ERROR_4
         * CME_832_TERMINAL_BASE_SOLUTION   CommandException.Error.OEM_ERROR_7
         */
        if (err == CommandException.Error.OEM_ERROR_2) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN, 0);
        } else if (err == CommandException.Error.OEM_ERROR_3) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, 0);
        } else if (err == CommandException.Error.OEM_ERROR_4) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_404_NOT_FOUND, 0);
        } else if (err == CommandException.Error.OEM_ERROR_25) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_409_CONFLICT, 0,
                    getXCAPErrorMessageFromSysProp(CommandException.Error.OEM_ERROR_25, phoneId));
        } else if (err == CommandException.Error.OEM_ERROR_7) {
            reason = new ImsReasonInfo(MtkImsReasonInfo.CODE_UT_XCAP_832_TERMINAL_BASE_SOLUTION, 0);
        } else if (err == CommandException.Error.REQUEST_NOT_SUPPORTED) {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NOT_SUPPORTED, 0);
        } else if (err == CommandException.Error.RADIO_NOT_AVAILABLE) {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0);
        } else if (err == CommandException.Error.PASSWORD_INCORRECT) {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_CB_PASSWORD_MISMATCH, 0);
        } else if (err == CommandException.Error.FDN_CHECK_FAILURE) {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_FDN_BLOCKED, 0);
        } else {
            reason = new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR, 0);
        }
        return reason;
    }

    public String getXcapQueryCarrierConfigKey() {
        return "mtk_carrier_ss_xcap_query";
    }

    public String getVolteSubscriptionKey() {
        return "volte_subscription";
    }

    public int getVolteSubUnknownConstant() {
        return 0;
    }

    public int getVolteSubEnableConstant() {
        return 1;
    }

    public int getVolteSubDisableConstant() {
        return 2;
    }

    public String getXCAPErrorMessageFromSysProp(CommandException.Error error, int phondId) {
        String propNamePrefix = ERROR_MSG_PROP_PREFIX + phondId;

        String fullErrorMsg = "";
        String errorMsg = null;

        int idx = 0;
        String propName = propNamePrefix +"." + idx;
        String propValue = "";

        propValue = SystemProperties.get(propName, "");


        while (!propValue.equals("")) {
            // SystemProperties.set(propName, "");
            fullErrorMsg += propValue;

            idx++;
            propName = propNamePrefix +"." + idx;
            propValue = SystemProperties.get(propName, "");
        }

        Log.d(TAG, "fullErrorMsg: " + fullErrorMsg);

        String errorCode = "";
        switch (error) {
            case OEM_ERROR_25:
                errorCode = "409";
                break;
            default:
                Log.d(TAG, "errorMsg: " + errorMsg);
                return errorMsg;
        }

        if (!fullErrorMsg.startsWith(errorCode)) {
            Log.d(TAG, "errorMsg: " + errorMsg);
            return errorMsg;
        }

        errorMsg = fullErrorMsg.substring(errorCode.length() + 1);
        Log.d(TAG, "errorMsg: " + errorMsg);

        return errorMsg;
    }

}