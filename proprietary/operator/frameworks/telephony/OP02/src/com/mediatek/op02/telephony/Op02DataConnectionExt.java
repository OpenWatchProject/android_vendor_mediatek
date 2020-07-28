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

package com.mediatek.op02.telephony;

import android.content.Context;
import android.telephony.Rlog;
import android.text.TextUtils;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.internal.telephony.dataconnection.DataConnectionExt;
import com.mediatek.internal.telephony.MtkPhoneConstants;

/**
 * Implement methods to support China Unicom requirements.
 *
 * @return
 */
public class Op02DataConnectionExt extends DataConnectionExt {
    static final String TAG = "Op02DataConnectionExt";

    public Op02DataConnectionExt(Context context) {
        super(context);
        log("constructor");
    }

    /**
     * To check metered apn type is decided by load type or not.
     *
     * @return true if metered apn type is decided by load type.
     */
    @Override
    public boolean isMeteredApnTypeByLoad() {
        return true;
    }


    /**
     * To check apn type is metered or not.
     *
     * @param type APN type.
     * @param isRoaming true if network in roaming state.
     * @return true if this APN type is metered.
     */
    @Override
    public boolean isMeteredApnType(String type, boolean isRoaming) {
        // Default metered apn type: [default, supl, dun, mms]
        if (TextUtils.equals(type, PhoneConstants.APN_TYPE_DEFAULT)
            || TextUtils.equals(type, PhoneConstants.APN_TYPE_SUPL)
            || TextUtils.equals(type, PhoneConstants.APN_TYPE_DUN)) {
            log("apnType=" + type + ",isRoaming=" + isRoaming);
            return true;
        }

        return false;
    }

    /**
     *Print radio log.
     *@param text The context needs to be printed.
     */
    @Override
    public void log(String text) {
        Rlog.d(TAG, text);
    }

    /**
    *Check if data allow when data enable is turned off.
    *@param apnType Type of request APN.
    *@return true if data is allowed when data enable is turned off, else return false.
    */
    @Override
    public boolean isDataAllowedAsOff(String apnType) {
        if (TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DEFAULT) ||
                TextUtils.equals(apnType, PhoneConstants.APN_TYPE_DUN)) {
            return false;
        }
        return true;
    }
}
