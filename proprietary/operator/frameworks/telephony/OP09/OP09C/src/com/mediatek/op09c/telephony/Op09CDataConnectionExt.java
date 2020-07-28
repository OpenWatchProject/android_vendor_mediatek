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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.op09c.telephony;

import android.content.Context;
import android.text.TextUtils;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.dataconnection.DataConnectionExt;
import com.mediatek.telephony.RoamingDataDialogCustomization;

/**
 * Op09CDataConnectionExt is requested by China Telecom spec.
 * Start listen roaming status change and popup dialog if needed.
 */
public class Op09CDataConnectionExt extends DataConnectionExt {
    private static final String TAG = "Op09CDataConnectionExt";

    private RoamingDataDialogCustomization mRoamingDataCustomization;

    public Op09CDataConnectionExt(Context context){
        super(context);
    }

    @Override
    public void startDataRoamingStrategy(Phone phone) {
        mRoamingDataCustomization = new RoamingDataDialogCustomization(phone);
    }

    @Override
    public void stopDataRoamingStrategy() {
        if (mRoamingDataCustomization != null) {
            mRoamingDataCustomization.dispose();
            mRoamingDataCustomization = null;
        }
    }

    @Override
    public boolean isMeteredApnTypeByLoad() {
        return true;
    }

    @Override
    public boolean isMeteredApnType(String type, boolean isRoaming) {
        if (TextUtils.equals(type, PhoneConstants.APN_TYPE_DEFAULT)
                || TextUtils.equals(type, PhoneConstants.APN_TYPE_DUN)
                || TextUtils.equals(type, PhoneConstants.APN_TYPE_SUPL)) {
            return true;
        }

        return false;
    }
}
