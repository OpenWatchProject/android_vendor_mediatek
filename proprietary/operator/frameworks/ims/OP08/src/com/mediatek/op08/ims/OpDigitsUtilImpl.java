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

package com.mediatek.op08.ims;

import android.os.Message;
import android.telephony.Rlog;
import android.telephony.ims.ImsCallProfile;

import com.mediatek.ims.MtkImsCallProfile;
import com.mediatek.ims.SipMessage;
import com.mediatek.ims.ext.DigitsUtilBase;
import com.mediatek.ims.ril.ImsCommandsInterface;
import com.mediatek.ims.ril.OpImsCommandsInterface;
import com.mediatek.telephony.MtkTelephonyManagerEx;

import mediatek.telecom.MtkTelecomManager;

public class OpDigitsUtilImpl extends DigitsUtilBase {
    private static final String TAG = "OpDigitsUtilImpl";

    private final boolean mIsDigitsSupported = MtkTelephonyManagerEx.getDefault()
            .isDigitsSupported();

    @Override
    public boolean hasDialFrom(ImsCallProfile profile) {
        Rlog.d(TAG, "hasDialFrom: " + (profile == null ? "empty profile"
                : profile.mCallExtras.getString(MtkImsCallProfile.EXTRA_DIAL_FROM)));
        if (profile == null) {
            return false;
        }
        if (profile.mCallExtras.containsKey(MtkImsCallProfile.EXTRA_DIAL_FROM)) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public void startFrom(String callee, ImsCallProfile profile, int clirMode, boolean
            isVideoCall, Object imsRILAdapter, Message response) {
        if (profile == null || profile.mCallExtras == null || imsRILAdapter == null) {
            Rlog.d(TAG, "startFrom: profile = " + profile + ", mCallExtras = " + profile
                    .mCallExtras + ", imsRILAdapter = " + imsRILAdapter);
            return;
        }
        String fromAddress = profile.mCallExtras.getString(MtkImsCallProfile.EXTRA_DIAL_FROM);
        OpImsCommandsInterface opUtil = ((ImsCommandsInterface) imsRILAdapter)
                .getOpCommandsInterface();
        opUtil.dialFrom(callee, fromAddress, clirMode, isVideoCall, null);
    }

    @Override
    public void putMtToNumber(String toNumber, ImsCallProfile profile){
        if (toNumber == null || profile == null) {
            return;
        }
        profile.mCallExtras.putString(MtkTelecomManager.EXTRA_VIRTUAL_LINE_NUMBER, toNumber);
    }

    @Override
    public void sendUssiFrom(Object imsRILAdapter, ImsCallProfile profile, int action,
            String ussi, Message response) {
        OpImsCommandsInterface opUtil = ((ImsCommandsInterface) imsRILAdapter)
                .getOpCommandsInterface();
        String from = (profile == null ? ""
                : profile.mCallExtras.getString(MtkImsCallProfile.EXTRA_DIAL_FROM, ""));
        opUtil.sendUssiFrom(from, action, ussi, response);
    }

    @Override
    public void cancelUssiFrom(Object imsRILAdapter, ImsCallProfile profile, Message response) {
        OpImsCommandsInterface opUtil = ((ImsCommandsInterface) imsRILAdapter)
                .getOpCommandsInterface();
        String from = (profile == null ? ""
                : profile.mCallExtras.getString(MtkImsCallProfile.EXTRA_DIAL_FROM, ""));
        opUtil.cancelUssiFrom(from, response);
    }

    @Override
    public void updateCallExtras(ImsCallProfile destCallProfile, ImsCallProfile srcCallProfile) {
        if (mIsDigitsSupported) {
            destCallProfile.updateCallExtras(srcCallProfile);
        }
    }

    @Override
    public boolean isRejectedByOthers(SipMessage msg) {
        if (msg == null) {
            return false;
        }
        return msg.isRejectedByOthers();
    }
}