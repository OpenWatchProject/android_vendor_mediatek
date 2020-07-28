/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

//package com.mediatek.ims.internal;
package com.mediatek.atci.service;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.Rlog;
import android.telecom.VideoProfile;
import android.util.Log;

import com.android.internal.telecom.IVideoProvider;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCall;
import com.android.internal.telephony.imsphone.ImsPhoneConnection;

public class AtciImsService extends Service {

    private static final String LOG_TAG = "AtciImsService";
    //same to AtciService
    private static final String ACTION_ATCI_COMMAND =
            "mediatek.intent.action.ATCTION_ATCI_IMS_COMMAND";
    //private Context mContext = null;
    private HandlerThread mAtciHandlerThread;
    private Handler mHandler = null;

    //same to AtciService
    private static final String NEXT_OP = "NEXT_OPERATION";
    private static final String OP_MERGE_CONFERENCE = "merge_conference";
    private static final String OP_VILTE_UPGRADE = "vilte_upgrade";
    private static final String OP_VILTE_DOWNGRADE = "vilte_downgrade";
    private static final String OP_VILTE_ACCEPT_UPGRADE = "vilte_accept_upgrade";
    private static final String OP_VILTE_REJECT_UPGRADE = "vilte_reject_upgrade";
    private static final String OP_VILTE_PAUSE_VIDEO = "vilte_pause_video";
    private static final String OP_VILTE_REUSME_VIDEO = "vilte_resume_video";


    private static final int EVENT_NONE = 0;
    private static final int EVENT_MERGE_CONFERENCE = 1;
    private static final int EVENT_VILTE_UPGRADE = 2;
    private static final int EVENT_VILTE_DOWNGRADE = 3;
    private static final int EVENT_VILTE_ACCEPT_UPGRADE = 4;
    private static final int EVENT_VILTE_REJECT_UPGRADE = 5;
    private static final int EVENT_VILTE_PAUSE_VIDEO = 6;
    private static final int EVENT_VILTE_RESUME_VIDEO = 7;

    private static final int EVENT_SERVICE_CREATED = 20;

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate () {
        Rlog.d(LOG_TAG, "onCreate+");
        mAtciHandlerThread = new HandlerThread("ImsAtciService");
        mAtciHandlerThread.start();
        mHandler = new MyHandler(mAtciHandlerThread.getLooper());
        final IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_ATCI_COMMAND);
        registerReceiver(mBroadcastReceiver, filter);
        mHandler.obtainMessage(EVENT_SERVICE_CREATED).sendToTarget();
        Rlog.d(LOG_TAG, "onCreate-");
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            Rlog.d(LOG_TAG, "[onReceive] action=" + intent.getAction());
            if (ACTION_ATCI_COMMAND.equals(intent.getAction())) {
                String operation = intent.getStringExtra(NEXT_OP);
                Rlog.d(LOG_TAG, "[onReceive] operation=" + operation);
                int event = EVENT_NONE;
                if (operation.equals(OP_MERGE_CONFERENCE)) {
                    event = EVENT_MERGE_CONFERENCE;
                } else if (operation.equals(OP_VILTE_UPGRADE)) {
                    event = EVENT_VILTE_UPGRADE;
                } else if (operation.equals(OP_VILTE_DOWNGRADE)) {
                    event = EVENT_VILTE_DOWNGRADE;
                } else if (operation.equals(OP_VILTE_ACCEPT_UPGRADE)) {
                    event = EVENT_VILTE_ACCEPT_UPGRADE;
                } else if (operation.equals(OP_VILTE_REJECT_UPGRADE)) {
                    event = EVENT_VILTE_REJECT_UPGRADE;
                } else if (operation.equals(OP_VILTE_PAUSE_VIDEO)) {
                    event = EVENT_VILTE_PAUSE_VIDEO;
                } else if (operation.equals(OP_VILTE_REUSME_VIDEO)) {
                    event = EVENT_VILTE_RESUME_VIDEO;
                } else {
                    Rlog.d(LOG_TAG, "[onReceive] Unknown operation");
                    return;
                }
                mHandler.sendMessage(mHandler.obtainMessage(event));
            }
        }
    };

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Rlog.d(LOG_TAG, "onStartCommand");
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        Rlog.d(LOG_TAG, "onDestroy+");
        mAtciHandlerThread.quitSafely();
        unregisterReceiver(mBroadcastReceiver);
        Rlog.d(LOG_TAG, "onDestroy-");
    }

    private class MyHandler extends Handler {

        MyHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_MERGE_CONFERENCE:
                    Rlog.d(LOG_TAG, "EVENT_MERGE_CONFERENCE");
                    mergeConference();
                    break;

                case EVENT_VILTE_UPGRADE:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_UPGRADE");
                    handleDowngradeUpgrade(true);
                    break;

                case EVENT_VILTE_DOWNGRADE:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_DOWNGRADE");
                    handleDowngradeUpgrade(false);
                    break;

                case EVENT_VILTE_ACCEPT_UPGRADE:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_ACCEPT_UPGRADE");
                    handleAcceptRejectUpgrade(true);
                    break;

                case EVENT_VILTE_REJECT_UPGRADE:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_REJECT_UPGRADE");
                    handleAcceptRejectUpgrade(false);
                    break;

                case EVENT_VILTE_PAUSE_VIDEO:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_PAUSE_VIDEO");
                    processPauseResumeVideo(false);
                    break;

                case EVENT_VILTE_RESUME_VIDEO:
                    Rlog.d(LOG_TAG, "EVENT_VILTE_RESUME_VIDEO");
                    processPauseResumeVideo(true);
                    break;
                case EVENT_SERVICE_CREATED:
                    Rlog.d(LOG_TAG, "EVENT_SERVICE_CREATED");
                    break;
                default:
                    Rlog.d(LOG_TAG, "Known event");
                    break;
            }
        }
    }

    private void mergeConference() {
        Rlog.d(LOG_TAG, "mergeConference");
        Phone[] phones = PhoneFactory.getPhones();
        GsmCdmaPhone targetPhone = null;
        for (Phone phone : phones) {
            if (phone != null && phone instanceof GsmCdmaPhone) {
                GsmCdmaPhone gsmCdmaPhone = (GsmCdmaPhone) phone;
                ImsPhone imsPhone = (ImsPhone) phone.getImsPhone();
                if (gsmCdmaPhone.isInCall() || (imsPhone != null && imsPhone.isInCall())) {
                    targetPhone = gsmCdmaPhone;
                    break;
                }
            }
        }
        if (targetPhone != null) {
            try {
                if (targetPhone.isPhoneTypeCdmaLte() || targetPhone.isPhoneTypeCdma()) {
                    targetPhone.switchHoldingAndActive();
                } else {
                    targetPhone.conference();
                }
            } catch (CallStateException e) {
                Rlog.d(LOG_TAG, "Merge conference fail: " + e);
            }
        } else {
            Rlog.d(LOG_TAG, "Merge conference fail, no call found");
        }
    }

    private void handleDowngradeUpgrade(boolean upgrade) {
        IVideoProvider provider = getVideoProvider();
        if (provider == null) {
            Rlog.d(LOG_TAG, "handleDowngradeUpgrade, failed");
            return;
        }
        VideoProfile fromProfile;
        VideoProfile toProfile;
        if (upgrade) {
            fromProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
            toProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        } else {
            fromProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
            toProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        }

        try {
            provider.sendSessionModifyRequest(fromProfile, toProfile);
        } catch (RemoteException e) {
            Rlog.d(LOG_TAG, "handleDowngradeUpgrade exception: " + e);
            return;
        }
        Rlog.d(LOG_TAG, "handleDowngradeUpgrade, success");
    }

    private void handleAcceptRejectUpgrade(boolean accept) {
        IVideoProvider provider = getVideoProvider();
        if (provider == null) {
            Rlog.d(LOG_TAG, "handleAcceptRejctUpgrade, failed");
            return;
        }
        VideoProfile videoProfile = null;
        if (accept) {
            videoProfile = new VideoProfile(VideoProfile.STATE_BIDIRECTIONAL);
        } else {
            videoProfile = new VideoProfile(VideoProfile.STATE_AUDIO_ONLY);
        }
        try {
            provider.sendSessionModifyResponse(videoProfile);
        } catch (RemoteException e) {
            Rlog.d(LOG_TAG, "handleAcceptRejctUpgrade exception: " + e);
            return;
        }
        Rlog.d(LOG_TAG, "handleAcceptRejctUpgrade, success");
    }

    private void processPauseResumeVideo(boolean resume) {
        IVideoProvider provider = getVideoProvider();
        if (provider == null) {
            Rlog.d(LOG_TAG, "processPauseResumeVideo, failed");
            return;
        }
        VideoProfile fromVideoProfile = null;
        VideoProfile toVideoProfile = null;
        if (resume) {
            fromVideoProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED);
            toVideoProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED |
                    VideoProfile.STATE_TX_ENABLED);
        } else {
            fromVideoProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED |
                    VideoProfile.STATE_TX_ENABLED);
            toVideoProfile = new VideoProfile(VideoProfile.STATE_RX_ENABLED);
        }
        try {
            provider.sendSessionModifyRequest(fromVideoProfile, toVideoProfile);
        } catch (RemoteException e) {
            Rlog.d(LOG_TAG, "processPauseResumeVideo exception: " + e);
            return;
        }
        Rlog.d(LOG_TAG, "processPauseResumeVideo, success");
    }

    private IVideoProvider getVideoProvider() {
        Rlog.d(LOG_TAG, "getVideoProvider");
        Phone[] phones = PhoneFactory.getPhones();
        GsmCdmaPhone gsmPhone = null;
        for (Phone p : phones) {
            if (p.getPhoneType() == PhoneConstants.PHONE_TYPE_GSM
                    || p.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA
                    || p.getPhoneType() == PhoneConstants.PHONE_TYPE_IMS
                    || p.getPhoneType() == PhoneConstants.PHONE_TYPE_CDMA_LTE) {
                if (p instanceof GsmCdmaPhone) {
                    Rlog.d(LOG_TAG, "getVideoProvider, phone type is " + p.getPhoneType());
                    gsmPhone = (GsmCdmaPhone) p;
                }
            }
            if (gsmPhone == null) {
                continue;
            }
            Rlog.d(LOG_TAG, "gsmPhoneId = " + gsmPhone.getPhoneId());
            if (gsmPhone.getImsPhone() == null) {
                Rlog.d(LOG_TAG, "getVideoProvider, the imsphone is null");
                continue;
            }
            ImsPhoneCall call = (ImsPhoneCall) gsmPhone.getImsPhone().getForegroundCall();
            ImsPhoneConnection imsphoneConnection = (ImsPhoneConnection) call.getFirstConnection();
            if (imsphoneConnection == null
                    || imsphoneConnection.getState() != ImsPhoneCall.State.ACTIVE) {
                Rlog.d(LOG_TAG, "getVideoProvider, not an active call");
                continue;
            }

            IVideoProvider provider = imsphoneConnection.getVideoProvider().getInterface();
            if (provider == null) {
                Rlog.d(LOG_TAG, "getVideoProvider, not exist a provider");
                continue;
            } else {
                return provider;
            }
        }
        return null;
    }
}

