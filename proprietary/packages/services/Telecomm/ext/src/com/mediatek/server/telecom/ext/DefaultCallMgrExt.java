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

package com.mediatek.server.telecom.ext;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.telecom.PhoneAccountHandle;

public class DefaultCallMgrExt implements ICallMgrExt {

    /**
     * should build call capabilities.
     *
     * @param smsCapability can response via sms.
     *
     * @return capalilities of the call.
     */
    @Override
    public int buildCallCapabilities(boolean smsCapability) {
        return 0;
    }

    /**
     * get call features
     *
     * @param callObj the call object
     *
     * @return int call features.
     */
    @Override
    public int getCallFeatures(Object callObj, int callFeatures) {
        return callFeatures;
    }

    /**
     * Should disconnect call calls when dial an ECC out.
     * Only for CMCC Volte PCT test
     *
     * @return whether to disconnect or not
     *
     */
    @Override
    public boolean shouldDisconnectCallsWhenEcc() {
        return true;
    }

    /**
     * should prevent video call based on battery status
     *
     * @param intent can response via sms.
     *
     * @return boolean true/false
     */
    @Override
    public boolean shouldPreventVideoCallIfLowBattery(Context context, Intent intent) {
        return false;
    }

    /**
     * should popup when emergency call tried on roaming
     *
     * @param handle uri for getting accounthandle.
     *
     * @param phoneAccountHandle phoneaccounthandle.
     *
     * @param extras Bundle.
     *
     * @return status of dialog shown or not
     */
    @Override
    public boolean blockOutgoingCall(Uri handle, PhoneAccountHandle phoneAccountHandle,
                                     Bundle extras){
        return false;
    }

    /**
     * should resume hold call.
     *
     * @return whether resume hold call or not.
     *
     */
    @Override
    public boolean shouldResumeHoldCall() {
        //default do not resume hold calls when disconnect remote
        return false;
    }

    /**
     * log Emergency number or not.
     *
     * @return whether log ECC or not.
     *
     */
    @Override
    public boolean shouldLogEmergencyNumber() {
        //default do not log Ecc
        return false;
    }
}
