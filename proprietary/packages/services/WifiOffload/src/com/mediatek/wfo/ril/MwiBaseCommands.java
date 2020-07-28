/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.wfo.ril;

import android.content.Context;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import com.mediatek.wfo.ril.MwiCommandsInterface;

/**
 * {@hide}
 */
public abstract class MwiBaseCommands implements MwiCommandsInterface {

    // ***** Instance Variables

    // Context
    protected Context mContext;

    // Current Phone Id
    protected int mPhoneId;


    protected RegistrantList mRssiThresholdChangedRegistrants = new RegistrantList();
    protected RegistrantList mWifiPdnActivatedRegistrants = new RegistrantList();
    protected RegistrantList mWifiPdnErrorRegistrants = new RegistrantList();
    protected RegistrantList mWifiPdnHandoverRegistrants = new RegistrantList();
    protected RegistrantList mWifiPdnRoveOutRegistrants = new RegistrantList();
    protected RegistrantList mRequestGeoLocationRegistrants = new RegistrantList();
    protected RegistrantList mWfcPdnStateChangedRegistrants = new RegistrantList();
    protected RegistrantList mWifiPingRequestRegistrants = new RegistrantList();
    protected RegistrantList mWifiPdnOosRegistrants = new RegistrantList();
    protected RegistrantList mNattKeepAliveChangedRegistrants = new RegistrantList();
    protected RegistrantList mWifiLockRegistrants = new RegistrantList();

    public MwiBaseCommands(Context context, int instanceId) {
        mContext = context;
        mPhoneId = instanceId;
    }

    @Override
    public void registerRssiThresholdChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRssiThresholdChangedRegistrants.add(r);
    }

    @Override
    public void unregisterRssiThresholdChanged(Handler h) {
        mRssiThresholdChangedRegistrants.remove(h);
    }

    @Override
    public void registerWifiPdnActivated(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPdnActivatedRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPdnActivate(Handler h) {
        mWifiPdnActivatedRegistrants.remove(h);
    }

    @Override
    public void registerWifiPdnError(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPdnErrorRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPdnError(Handler h) {
        mWifiPdnErrorRegistrants.remove(h);
    }

    @Override
    public void registerWifiPdnHandover(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPdnHandoverRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPdnHandover(Handler h) {
        mWifiPdnHandoverRegistrants.remove(h);
    }

    @Override
    public void registerWifiPdnRoveOut(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPdnRoveOutRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPdnRoveOut(Handler h) {
        mWifiPdnRoveOutRegistrants.remove(h);
    }

    @Override
    public void registerRequestGeoLocation(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRequestGeoLocationRegistrants.add(r);
    }

    @Override
    public void unregisterRequestGeoLocation(Handler h) {
        mRequestGeoLocationRegistrants.remove(h);
    }

    @Override
    public void registerWfcPdnStateChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWfcPdnStateChangedRegistrants.add(r);
    }

    @Override
    public void unregisterWfcPdnStateChanged(Handler h) {
        mWfcPdnStateChangedRegistrants.remove(h);
    }

    @Override
    public void registerWifiPingRequest(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPingRequestRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPingRequest(Handler h) {
        mWifiPingRequestRegistrants.remove(h);
    }

    @Override
    public void registerWifiPdnOos(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiPdnOosRegistrants.add(r);
    }

    @Override
    public void unregisterWifiPdnOos(Handler h) {
        mWifiPdnOosRegistrants.remove(h);
    }

    @Override
    public void registerWifiLock(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mWifiLockRegistrants.add(r);
    }

    @Override
    public void unregisterWifiLock(Handler h) {
        mWifiLockRegistrants.remove(h);
    }

    @Override
    public void registerNattKeepAliveChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mNattKeepAliveChangedRegistrants.add(r);
    }

    @Override
    public void unrgisterNattKeepAliveChanged(Handler h) {
        mNattKeepAliveChangedRegistrants.remove(h);
    }

}
