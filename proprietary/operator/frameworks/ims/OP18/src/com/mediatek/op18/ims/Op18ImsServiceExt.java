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
 * MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.op18.ims;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.Uri;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.ims.ImsManager;
import com.mediatek.ims.ext.ImsServiceExt;
import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.ims.ril.ImsRILAdapter;
import com.mediatek.wfo.WifiOffloadManager;

/**
 * Plugin implementation for ImsService.
 */
public class Op18ImsServiceExt extends ImsServiceExt {

    private static final String TAG = "Op18ImsServiceExt";
    private static final String IMS_SIM = "imssim";
    private static ContentObserver sImsSimChangeObserver = null;
    private static boolean sEnhanced4GLte = false;
    private static boolean sVtSetting = false;
    private static boolean sWfcSetting = false;
    private static int PHONEID_SIM1 = 0;
    private static int PHONEID_SIM2 = 1;
    private static boolean sIsImsServiceDown = true;
    private static final int DELAY_TIMER = 10000; // 10 SECONDS
    private static boolean sIsImsSimSwitchDone = true;
    private static final String IMS_INTENT = "com.mediatek.ims.notify_ims_switch_done";
    private static final String MULTI_IMS_SUPPORT = "persist.vendor.mims_support";
    private static final int EVENT_IMS_REGISTRATION_INFO = 1;
    private static final int IMS_SWITCHING_COMPLETE = 1;
    private static final int IMS_SWITCHING_IN_PROGRESS = 0;
    private static final String PRIMARY_SIM = "primary_sim";

    /// M: Event for IMS RTP Report @{
    protected static final int EVENT_IMS_RTP_INFO_URC = 19;
    protected static final int EVENT_SET_IMS_RTP_INFO_DONE = 100;
    private static final int WIFI_PDN_ID = 0;
    private static final int DEFAULT_RTP_REPORT_TIMER = 20000;
    /// @}

    private Context mContext;
    private boolean mImsServiceReady = false;

    private BroadcastReceiver mImsIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "mImsIntentReceiver: action " + intent.getAction());
            if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_UP)) {
                mImsServiceReady = true;
                registerImsSimObserver(mContext);
            } else if (intent.getAction().equals(ImsManager.ACTION_IMS_SERVICE_DOWN)) {
                mImsServiceReady = false;
                unregisterImsSimObserver(mContext);
            }
        }
    };
   /**
    * Constructor
    */
    public Op18ImsServiceExt(Context context) {
        super(context);
        mContext = context;
        if (sIsImsSimSwitchDone) {
            setImsSwitchCompleteStatus(IMS_SWITCHING_COMPLETE);
        } else {
            setImsSwitchCompleteStatus(IMS_SWITCHING_IN_PROGRESS);
        }
        IntentFilter filter = new IntentFilter();
        filter.addAction(ImsManager.ACTION_IMS_SERVICE_UP);
        filter.addAction(ImsManager.ACTION_IMS_SERVICE_DOWN);
        mContext.registerReceiver(mImsIntentReceiver, filter);
    }

    /**
     * Register IMS SIM observer.
     * @param context hostapp context
     */
    private void registerImsSimObserver(Context context) {
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) != 1) {
            Log.d(TAG, "registerImsSimObserver");
            if (sImsSimChangeObserver == null) {
                registerImsSimChange(new Handler());
            }
        }
    }

    private void unregisterImsSimObserver(Context context) {
        if (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) != 1) {
            Log.d(TAG, "unregisterImsSimObserver");
            if (sImsSimChangeObserver != null) {
                mContext.getContentResolver().unregisterContentObserver(sImsSimChangeObserver);
            }
        }
    }

   /**
    * Observes primary SIM changes.
    */
    private void registerImsSimChange(Handler handler) {
        Log.d(TAG, "registerImsSimChange");
        sImsSimChangeObserver = new ContentObserver(handler) {
                @Override
                public void onChange(boolean selfChange) {
                    this.onChange(selfChange, Settings.Global
                            .getUriFor(PRIMARY_SIM));
                }

                @Override
                public void onChange(boolean selfChange, Uri uri) {
                    int subId = Settings.Global.getInt(mContext.getContentResolver(),
                            PRIMARY_SIM, -1);
                    Log.d(TAG, "onChange primary SIM: , subId = " + subId);
                    if (subId != -1) {
                        int phoneId = SubscriptionManager.getPhoneId(subId);
                        getCurrentSettings(phoneId);
                        int oldPhoneId = (phoneId == PHONEID_SIM1) ? PHONEID_SIM2 : PHONEID_SIM1;
                        Log.d(TAG, "OldPhoneId:" + oldPhoneId + " NewPhoneId:" + phoneId);
                        if (!sEnhanced4GLte && !sVtSetting && !sWfcSetting) {
                            Log.d(TAG, "do nothing:");
                        } else {
                            updateImsSettings(oldPhoneId, false);
                            sIsImsServiceDown = false;
                            sIsImsSimSwitchDone = false;
                            setImsSwitchCompleteStatus(IMS_SWITCHING_IN_PROGRESS);
                            mHandler.postDelayed(mRunnable, DELAY_TIMER);
                        }
                    } else {
                        Log.d(TAG, "onChange primary SIM: Invalid SubId");
                    }
                }
            };
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(PRIMARY_SIM),
                false, sImsSimChangeObserver);
    }

    private void handleImsDisableDone(int phoneId) {
        // disable IMS here for old primary SIM
        Log.d(TAG, "handleImsDisableDone for phoneId = " + phoneId);
        int respPhoneId = phoneId;
        int reqPhoneId = getPrimarySimPhoneId();
        Log.d(TAG, "respPhoneId: " + respPhoneId + ", reqPhoneId:" + reqPhoneId);
        if (respPhoneId != reqPhoneId && !sIsImsServiceDown) {
            Log.d(TAG, "IMS service down for old IMS sim received");
            sIsImsServiceDown = true;
            sIsImsSimSwitchDone = false;
            setImsSwitchCompleteStatus(IMS_SWITCHING_IN_PROGRESS);
            updateImsSettings(reqPhoneId, true);
        }
    }

    private void handleImsEnableDone(int phoneId) {
        Log.d(TAG, "handleImsEnableDone for phoneId = " + phoneId);
        int respPhoneId = phoneId;
        int reqPhoneId = getPrimarySimPhoneId();
        if (respPhoneId == reqPhoneId && !sIsImsSimSwitchDone) {
            Log.d(TAG, "IMS SimSwitch Done for phoneId " + reqPhoneId);
            sIsImsSimSwitchDone = true;
            setImsSwitchCompleteStatus(IMS_SWITCHING_COMPLETE);
            sendBroadCast();
        }
    }

    private Handler mHandler = new Handler();
    private Runnable mRunnable = new Runnable() {
        @Override
        public void run() {
            Log.d(TAG, "Runnable starts");
            if (!sIsImsServiceDown) {
                updateImsSettingWhenNoImsServiceDown();
            }
        }
    };

    private void updateImsSettingWhenNoImsServiceDown() {
        int phoneId = getPrimarySimPhoneId();
        Log.d(TAG, "Updating ImsSetting When No IMS de-registration Info received");
        sIsImsServiceDown = true;
        sIsImsSimSwitchDone = false;
        setImsSwitchCompleteStatus(IMS_SWITCHING_IN_PROGRESS);
        updateImsSettings(phoneId, true);
    }

    private void updateImsSettings(int phoneId, boolean enable) {
        Log.d(TAG, "updateImsSettings: , phoneId = " + phoneId + ", enable = " + enable);
        Thread t = new Thread() {
            @Override
            public void run() {
                try {
                    if (mImsServiceReady) {
                        Log.d(TAG, "updateImsServiceConfig: , phoneId: " + phoneId
                                        + ", enable = " + enable);
                        ImsManager.updateImsServiceConfig(mContext, phoneId, true);
                    } else {
                        Log.d(TAG, "ImsService not ready");
                    }
               } catch (Exception e) {
                    Log.d(TAG, "updateImsSettings Exception: " + e);
               }
            }
        };
        t.start();
    }

    private int getPrimarySimPhoneId() {
        int phoneId = 0;
        int subId = Settings.Global.getInt(mContext.getContentResolver(),
                PRIMARY_SIM, -1);
        if (subId != -1) {
            phoneId = SubscriptionManager.getPhoneId(subId);
        }
        Log.d(TAG, "getPrimarySimPhoneId: , phoneId: " + phoneId + " subId:" + subId);
        return phoneId;
    }

    private void getCurrentSettings(int phoneId) {
        ImsManager imsMgr = ImsManager.getInstance(mContext, phoneId);
        sEnhanced4GLte = imsMgr.isEnhanced4gLteModeSettingEnabledByUser();
        sVtSetting = imsMgr.isVtEnabledByUser();
        sWfcSetting = imsMgr.isWfcEnabledByUser();
        Log.d(TAG, "sEnhanced4GLte :" + sEnhanced4GLte + ", sVtSetting: " + sVtSetting +
                ", sWfcSetting: " + sWfcSetting);
    }

    private void sendBroadCast() {
        Log.d(TAG, "sendBroadCast IMS Switch completed Intent");
        Intent i = new Intent();
        i.setAction(IMS_INTENT);
        mContext.sendBroadcast(i);
    }

    /**
     * Api to set IMS SIM switching is complete or not.
     */
    private void setImsSwitchCompleteStatus(int status) {
        Log.d(TAG, "setImsSwitchCompleteStatus = " + status);
        android.provider.Settings.System.putInt(
                mContext.getContentResolver(), IMS_SIM, status);
    }

    /**
     * Notify about IMS service events.
     * @param phoneId phoneId
     * @param context hostContext
     * @param msg message received at ImsService
     */
    @Override
    public void notifyImsServiceEvent(int phoneId, Context context, Message msg) {
        Log.d(TAG, "ImsService event: " + msg.what);
        boolean isMultiImsSupported =
                (SystemProperties.getInt(MULTI_IMS_SUPPORT, 1) != 1);
        AsyncResult ar;

        switch(msg.what) {
            case EVENT_IMS_REGISTRATION_INFO:
                ar = (AsyncResult) msg.obj;
                Log.d(TAG, "EVENT_IMS_REGISTRATION_INFO with status:" + ((int[]) ar.result)[0]
                                + " phoneId:" + phoneId);
                if (isMultiImsSupported && ar.exception == null) {
                    if (((int[]) ar.result)[0] == 1) {
                       handleImsEnableDone(phoneId);
                    } else if (((int[]) ar.result)[0] == 0) {
                       handleImsDisableDone(phoneId);
                    } else {
                       Log.d(TAG, "Invalid IMS Registration Info");
                    }
                }
                break;
            case EVENT_IMS_RTP_INFO_URC:
                ar = (AsyncResult) msg.obj;
                Intent intent;
                String[] rtpInfo = (String[]) ar.result;
                if (rtpInfo == null) {
                   Log.d(TAG, "rtpInfo array is null");
                   return;
                }
                Log.d(TAG, "receive EVENT_IMS_RTP_INFO_URC, pdn id:" +
                        Integer.parseInt(rtpInfo[0]));
                if (WIFI_PDN_ID == Integer.parseInt(rtpInfo[0])) {
                    intent = new Intent(MtkImsManager.ACTION_IMS_RTP_INFO);
                   intent.putExtra(MtkImsManager.EXTRA_RTP_PDN_ID,
                           Integer.parseInt(rtpInfo[0]));
                   intent.putExtra(MtkImsManager.EXTRA_RTP_NETWORK_ID,
                           Integer.parseInt(rtpInfo[1]));
                   intent.putExtra(MtkImsManager.EXTRA_RTP_TIMER,
                           Integer.parseInt(rtpInfo[2]));
                   intent.putExtra(MtkImsManager.EXTRA_RTP_SEND_PKT_LOST,
                           Integer.parseInt(rtpInfo[3]));
                   intent.putExtra(MtkImsManager.EXTRA_RTP_RECV_PKT_LOST,
                           Integer.parseInt(rtpInfo[4]));
                    mContext.sendBroadcast(intent);
                }
                break;
           case EVENT_SET_IMS_RTP_INFO_DONE:
               // Only log for tracking
               Log.d(TAG, "receive EVENT_SET_IMS_RTP_INFO_DONE");
               break;
           default:
               break;
        }
    }

    /**
     * Notify registrationStateChange.
     *
     * @param ranType ims RAN type
     * @param handler ImsService main handler
     * @param imsRILAdapter ImsRILAdapter instance
     * @return
     */
    @Override
    public void notifyRegistrationStateChange(int ranType, Handler handler,
            Object imsRILAdapter) {
        if (ranType == WifiOffloadManager.RAN_TYPE_WIFI) {
            // set IMS RTP info only for RJIL WFC
            setImsRtpInfo(handler, (ImsRILAdapter) imsRILAdapter);
        }
    }

    /**
     * Notify MD to report RTP info. Currently this is for RJIL(OP18) and report
     * WFC RTP status only.
     *
     * @hide
     */
    private void setImsRtpInfo(Handler handler, ImsRILAdapter imsRILAdapter) {
        Log.d(TAG, "setImsRtpInfo");
        int networkId = -1;

        ConnectivityManager connectivityManager = (ConnectivityManager) mContext
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        Network[] allNetworks = connectivityManager.getAllNetworks();
        for (Network network : allNetworks) {
            Log.d(TAG, "Checking network:" + network);
            NetworkCapabilities nc = connectivityManager.getNetworkCapabilities(network);
            if (nc.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS)) {
                Log.d(TAG, "Found IMS capability, netId=" + network.netId);
                networkId = network.netId;
                break;
            }
        }
        if (networkId < 0) {
            Log.d(TAG, "IMS capability not found, skip");
            return;
        }

        imsRILAdapter.setImsRtpInfo(WIFI_PDN_ID, networkId, DEFAULT_RTP_REPORT_TIMER,
                handler.obtainMessage(EVENT_SET_IMS_RTP_INFO_DONE));
    }
}