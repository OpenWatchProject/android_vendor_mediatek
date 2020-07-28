/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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
package com.mediatek.internal.telephony;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;

import android.telephony.AccessNetworkConstants;
import android.telephony.CellIdentity;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsMmTelManager;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.LteVopsSupportInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.NetworkService;
import android.telephony.NetworkServiceCallback;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.wfo.IMwiService;
import com.mediatek.wfo.IWifiOffloadService;
import com.mediatek.wfo.MwisConstants;
import com.mediatek.wfo.WifiOffloadManager;
import com.mediatek.wfo.IWifiOffloadService;

import java.util.concurrent.ConcurrentHashMap;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class IWlanNetworkService extends NetworkService {
    private static final String TAG = "IWlanNetworkService";

    private final ConcurrentHashMap<Integer, IWlanNetworkServiceProvider> mIWlanNetSrvProviderMap =
            new ConcurrentHashMap<>();

    public static final int MWI_SERVICE_READY = 0;

    private IWifiOffloadService sWifiOffloadService = null;
    private IWifiOffloadServiceDeathRecipient mDeathRecipient =
            new IWifiOffloadServiceDeathRecipient();
    private IWifiOffloadListenerProxy mProxy = null;



    public IWlanNetworkService() {
        super();

        bindAndRegisterWifiOffloadService();
        log("IWlanNetworkService init.");
    }

    private class IWlanNetworkServiceProvider extends NetworkServiceProvider {

        private final ConcurrentHashMap<Message, NetworkServiceCallback> mCallbackMap =
                new ConcurrentHashMap<>();

        private final Looper mLooper;

        private final HandlerThread mHandlerThread;

        private final Handler mHandler;

        private final Phone mPhone;

        private int mWfcState = 0;

        private final Object mLock = new Object();

        public static final int IWLAN_REGISTRATION_STATE_CHANGED  = 1;
        public static final int GET_IWLAN_REGISTRATION_STATE_DONE = 2;

        IWlanNetworkServiceProvider(int slotId) {
            super(slotId);

            log("IWlanNetworkServiceProvider construct.");

            mPhone = PhoneFactory.getPhone(getSlotIndex());

            mHandlerThread = new HandlerThread(IWlanNetworkServiceProvider.class.getSimpleName());
            mHandlerThread.start();
            mLooper = mHandlerThread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {
                    NetworkServiceCallback callback = mCallbackMap.remove(message);

                    AsyncResult ar;
                    switch (message.what) {
                        case IWLAN_REGISTRATION_STATE_CHANGED: {
                            int state = message.arg1;
                            log("IWLAN_REGISTRATION_STATE_CHANGED, slotid: " + getSlotIndex() +
                                    ", state: " + state);
                            mWfcState = state;
                            notifyNetworkRegistrationInfoChanged();
                            break;
                        }
                        case GET_IWLAN_REGISTRATION_STATE_DONE: {
                            NetworkRegistrationInfo netState =
                                    createRegistrationState(mWfcState);
                            int resultCode = NetworkServiceCallback.RESULT_SUCCESS;

                            try {
                                log("Calling callback.onGetNetworkRegistrationStateComplete."
                                        + "resultCode = " + resultCode
                                        + ", netState = " + netState
                                        + ", slotid: " + getSlotIndex());

                                callback.onRequestNetworkRegistrationInfoComplete(
                                         resultCode, netState);
                            } catch (Exception e) {
                                loge("Exception: " + e);
                            }

                            break;
                        }
                        default:
                            return;
                    }
                }
            };
        }

        public Handler getHandler() {
            return mHandler;
        }

        @Override
        public void requestNetworkRegistrationInfo(int domain, NetworkServiceCallback callback) {
            log("getNetworkRegistrationState for domain " + domain
                    + ", slotid: " + getSlotIndex());
            Message message = null;

            if (domain == NetworkRegistrationInfo.DOMAIN_PS) {
                message = Message.obtain(mHandler, GET_IWLAN_REGISTRATION_STATE_DONE);
                mCallbackMap.put(message, callback);
                message.sendToTarget();
            } else {
                loge("getNetworkRegistrationState invalid domain " + domain
                    + ", slotid: " + getSlotIndex());
                callback.onRequestNetworkRegistrationInfoComplete(
                        NetworkServiceCallback.RESULT_ERROR_INVALID_ARG, null);
            }
        }

        @Override
        public void close() {
            log("close.");
            mCallbackMap.clear();
            mHandlerThread.quit();
        }

        private NetworkRegistrationInfo createRegistrationState(int state) {
            log("createRegistrationState.");
            int domain = NetworkRegistrationInfo.DOMAIN_PS;
            int transportType = AccessNetworkConstants.TRANSPORT_TYPE_WLAN;

            int regState = 0;
            int accessNetworkTechnology = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            if (state == 1) {
                regState = NetworkRegistrationInfo.REGISTRATION_STATE_HOME;
                accessNetworkTechnology = TelephonyManager.NETWORK_TYPE_IWLAN;
            } else {
                regState = NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
                accessNetworkTechnology = TelephonyManager.NETWORK_TYPE_UNKNOWN;
            }
            int reasonForDenial = 0;
            boolean emergencyOnly = false;
            int maxDataCalls = 0;
            ArrayList<Integer> availableServices = getAvailableServices(
                    regState, domain, emergencyOnly);
            CellIdentity cellIdentity = null;
            LteVopsSupportInfo lteVopsSupportInfo = new LteVopsSupportInfo(
                    LteVopsSupportInfo.LTE_STATUS_NOT_AVAILABLE,
                    LteVopsSupportInfo.LTE_STATUS_NOT_AVAILABLE);

            return new NetworkRegistrationInfo(domain, transportType, regState,
                        accessNetworkTechnology, reasonForDenial, emergencyOnly, availableServices,
                        cellIdentity, maxDataCalls, false /* isDcNrRestricted */,
                        false /* isNrAvailable */, false /* isEnDcAvailable */, lteVopsSupportInfo,
                        false /* defaultRoamingIndicator */);
        }
    }

    private ArrayList<Integer> getAvailableServices(int regState, int domain,
            boolean emergencyOnly) {
        log("getAvailableServices.");
        ArrayList<Integer> availableServices = null;

        // In emergency only states, only SERVICE_TYPE_EMERGENCY is available.
        // Otherwise, certain services are available only if it's registered on home or roaming
        // network.
        if (emergencyOnly) {
            availableServices = new ArrayList<>(
                    Arrays.asList(NetworkRegistrationInfo.SERVICE_TYPE_EMERGENCY));
        } else if (regState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING
                || regState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME) {
            if (domain == NetworkRegistrationInfo.DOMAIN_PS) {
                availableServices = new ArrayList<>(
                        Arrays.asList(NetworkRegistrationInfo.SERVICE_TYPE_DATA));
            }
        }


        return availableServices;
    }

    @Override
    public NetworkServiceProvider onCreateNetworkServiceProvider(int slotIndex) {
        log("IWlan network service created for slot " + slotIndex);
        if (!SubscriptionManager.isValidSlotIndex(slotIndex)) {
            loge("Tried to Iwlan network service with invalid slotId " + slotIndex);
            return null;
        }

        mIWlanNetSrvProviderMap.remove(slotIndex);
        mIWlanNetSrvProviderMap.put(slotIndex, new IWlanNetworkServiceProvider(slotIndex));

        return mIWlanNetSrvProviderMap.get(slotIndex);
    }

    private void log(String s) {
        Rlog.d(TAG, s);
    }

    private void loge(String s) {
        Rlog.e(TAG, s);
    }




    /**
     *create wifiOffloadListnerProxy.
     *@return return wifiOffloadLisetnerProxy
     *@hide
     */
    private IWifiOffloadListenerProxy createWifiOffloadListenerProxy() {
        if (mProxy == null) {
            log("create WifiOffloadListenerProxy");
            mProxy = new IWifiOffloadListenerProxy();
        }
        return mProxy;
    }

    /**
     * Adapter class for {@link IWifiOffloadListener}.
     */
    private class IWifiOffloadListenerProxy extends WifiOffloadManager.Listener {
        @Override
        public void onWfcStateChanged(int simId, int state) {
            log("onWfcStateChanged simIdx=" + simId + ", state=" + state);
            notifyWfcStateChanged(simId, state);
        }
    }

    private void notifyWfcStateChanged(int simId, int state) {
        // if (simId != getSlotIndex()) {
        //     return;
        // }
        if (!mIWlanNetSrvProviderMap.containsKey(simId)) {
            log("IWlanNetworkServiceProvider id " + simId + " did not exist.");
            return;
        }

        // mWfcState = state;

        log("notifyWfcStateChanged: " + state);
        Message msg = mIWlanNetSrvProviderMap.get(simId).getHandler().obtainMessage(
                IWlanNetworkServiceProvider.IWLAN_REGISTRATION_STATE_CHANGED,
                state,
                0);
        msg.sendToTarget();
    }

    /**
     * Binds the WifiOffload service.
     */
    private void checkAndBindWifiOffloadService() {
        IBinder b = ServiceManager.getService(MwisConstants.MWI_SERVICE);
        try {
            if (b != null) {
                b.linkToDeath(mDeathRecipient, 0);
                sWifiOffloadService = IMwiService.Stub.asInterface(b).getWfcHandlerInterface();
            } else {
                log("No MwiService exist");
            }
        } catch (RemoteException e) {
            loge("can't get MwiService:" + e);
        }

        log("checkAndBindWifiOffloadService: sWifiOffloadService = " +
                sWifiOffloadService);
    }

    /**
     * Death recipient class for monitoring WifiOffload service.
     */
    private class IWifiOffloadServiceDeathRecipient implements IBinder.DeathRecipient {
        @Override
        public void binderDied() {
            sWifiOffloadService = null;
            bindAndRegisterWifiOffloadService();
        }
    }


    /**
     * Try to bind WifiOffload service and register for handover event.
     */
    private void bindAndRegisterWifiOffloadService() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                while (sWifiOffloadService == null) {
                    checkAndBindWifiOffloadService();
                    if (sWifiOffloadService != null) {
                        try {
                            sWifiOffloadService.registerForHandoverEvent(
                                    createWifiOffloadListenerProxy());
                        } catch (RemoteException e) {
                            loge("can't register handover event");
                        }
                    } else {
                        if(SystemProperties.getInt("persist.vendor.mtk_wfc_support", 0) == 0){
                            loge("can't get WifiOffloadService");
                            break;
                        }
                    }

                    if (sWifiOffloadService != null) {
                        break;
                    }

                    loge("can't get WifiOffloadService, retry after 1s.");
                    try {
                        Thread.sleep(1000);
                    } catch (InterruptedException e) {

                    }
                }
            }
        }).start();
    }

}
