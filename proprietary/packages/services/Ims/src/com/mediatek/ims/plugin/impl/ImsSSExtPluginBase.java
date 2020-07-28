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
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;

import com.mediatek.ims.ImsUtImpl;
import com.mediatek.ims.plugin.ImsSSExtPlugin;

import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_VOICE;

public class ImsSSExtPluginBase implements ImsSSExtPlugin {

    private static final String TAG = "ImsSSExtPluginBase";

    private Context mContext;

    public ImsSSExtPluginBase(Context context) {
        mContext = context;
    }

    @Override
    public ImsCallForwardInfo[] getImsCallForwardInfo(CallForwardInfo[] info) {
        ImsCallForwardInfo[] imsCfInfo = new ImsCallForwardInfo[1];
        if (info != null) {
            for (int i = 0; i < info.length; i++) {
                Log.d(TAG, "getImsCallForwardInfo: info[" + i + "] = " + info[i]);
                // AOSP ImsPhone only accepts VOICE class CF info
                if (info[i].serviceClass == SERVICE_CLASS_VOICE) {
                    imsCfInfo[0] = new ImsCallForwardInfo();
                    imsCfInfo[0].mCondition = ImsUtImpl.getConditionFromCFReason(info[i].reason);
                    imsCfInfo[0].mStatus = info[i].status;
                    imsCfInfo[0].mServiceClass = info[i].serviceClass;
                    imsCfInfo[0].mToA = info[i].toa;
                    imsCfInfo[0].mNumber = info[i].number;
                    imsCfInfo[0].mTimeSeconds = info[i].timeSeconds;
                    return imsCfInfo;
                }
            }
            /**
             * We need to make a default voice class call forwarding info when modem didn't return one.
             * Modem will only return the enable part.
             * e.x AT< +CCFC: 1, 16,"phone number", 145
             */
            imsCfInfo[0] = new ImsCallForwardInfo();
            imsCfInfo[0].mCondition = ImsUtImpl.getConditionFromCFReason(info[0].reason);
            imsCfInfo[0].mStatus = 0;
            imsCfInfo[0].mServiceClass = SERVICE_CLASS_VOICE;
            imsCfInfo[0].mToA = 0;
            imsCfInfo[0].mNumber = "";
            imsCfInfo[0].mTimeSeconds = 0;
        }
        return imsCfInfo;
    }
}