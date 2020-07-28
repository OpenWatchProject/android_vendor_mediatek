/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
package com.mediatek.rcse.api;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;

import com.mediatek.rcse.service.ApiService;
import com.mediatek.rcse.service.IApiServiceWrapper;
import com.mediatek.rcse.service.IRegistrationStatus;

import com.mediatek.rcse.settings.RcsSettings;
import com.orangelabs.rcs.service.RcsCoreService;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

//This class provides an access to the RCS-e Registration status.
/**
 * The Class RegistrationApi.
 */
public class RegistrationApi extends
        IRegistrationStatusRemoteListener.Stub implements
        RemoteRcseApi {
    /**
     * RCS service name.
     */
    public static final String SERVICE_NAME = "com.orangelabs.rcs.SERVICE";
    /**
     * The Constant TAG.
     */
    public static final String TAG = "RegistrationApi";

    /**
     * The m remote window binder.
     */
    private IApiServiceWrapper mApiServiceWrapperBinder;

    /**
     * Start RCS-e registration process. This method only work when the
     * registration status is STOPED.
     *
     * @param context
     *            The application that wants to start the registration process.
     * @return true if succeeded to start the RCS-e core service. false if
     *         unable to find the RCS-e core service. Note that if the RCS-e
     *         core service has already been started, this method will also
     *         return true.
     */
    public static boolean start(Context context) {
        if (null == context) {
            Logger.e(TAG, "start() the context is null");
            return false;
        }
        String packageName = context.getPackageName();
        Logger.v(TAG, "start() entry, the calling package name is "
                + packageName);
        boolean result = false;
        ComponentName componentName = null;
        try {
            componentName = context.startService(new Intent(
                    context, RcsCoreService.class));
            if (null != componentName) {
                result = true;
            }
        } catch (SecurityException e) {
            e.printStackTrace();
            Logger.e(TAG,
                    "Start service failed, please check your permission.");
            result = false;
        }
        Logger.v(TAG, "start() leave, the calling package name is "
                + packageName + ", the result is " + result);
        return result;
    }
    /**
     * Unregister current RCS-e account from MNO server. Note that if RCS-e core
     * service is still bound by other applications, this method may do nothing.
     *
     * @param context
     *            The application that wants to unregister current RCS-e
     *            account.
     * @return true if succeeded to stop the RCS-e core service. false if unable
     *         to find the RCS-e core service. Note that if the RCS-e core
     *         service has already been stop, this method will also return true.
     */
    public static boolean stop(Context context) {
        if (null == context) {
            Logger.e(TAG, "stop() the context is null");
            return false;
        }
        String packageName = context.getPackageName();
        Logger.v(TAG, "start() entry, the calling package name is "
                + packageName);
        boolean result = false;
        try {
            result = context.stopService(new Intent(context, RcsCoreService.class));
        } catch (SecurityException e) {
            e.printStackTrace();
            Logger.e(TAG,
                    "stop() Stop service failed, please check your permission.");
        }
        Logger.v(TAG, "start() exit, the calling package name is "
                + packageName);
        return result;
    }
    /**
     * Constructor of RegistrationApi. It's recommended that each application
     * reserves no more than one RegistrationApi instance.
     *
     * @param context
     *            This Context should not be null, otherwise the constructor
     *            will threw a RuntimeException.
     */
    public RegistrationApi(Context context) {
        if (null == context) {
            Logger.e(TAG, "Constructor: context is null");
            throw new RuntimeException(
                    "Illigal input context, it's null!!");
        }
        mContext = context;
    }

    /**
     * Listener interface that used to observe the status of RCS-e account.
     *
     * @see IRegistrationStatusEvent
     */
    public interface IRegistrationStatusListener {
        /**
         * Called when the status of RCS-e account is registered.
         *
         * @param status
         *            Current status of RCS-e account.
         */
        void onStatusChanged(boolean status);
    }

    /**
     * The m context.
     */
    private Context mContext = null;
    /**
     * The m registration status.
     */
    private IRegistrationStatus mRegistrationStatus = null;
    /**
     * The m listeners.
     */
    private CopyOnWriteArrayList<IRegistrationStatusListener> mListeners =
            new CopyOnWriteArrayList<IRegistrationStatusListener>();

    /**
     * Add a RegistrationStatusListener.
     *
     * @param listener
     *            The listener you want to add.
     */
    public void addRegistrationStatusListener(
            IRegistrationStatusListener listener) {
        mListeners.add(listener);
    }
    /**
     * Remove a RegistrationStatusListener that once added.
     *
     * @param listener
     *            The listener you want to remove.
     * @return true If the input listener was added and now has been removed.
     *         false If the input listener wasn't added and no need to remove.
     */
    public boolean removeRegistrationStatusListener(
            IRegistrationStatusListener listener) {
        boolean result = mListeners.remove(listener);
        return result;
    }
    /*
     * (non-Javadoc)
     *
     * @see com.mediatek.rcse.api.RemoteRCSeApi#connect()
     */
    @Override
    public void connect() {
        /*Logger.v(TAG, "connect registrationApi() entry");
        Intent intent = new Intent(mContext, ApiService.class);
        boolean result = mContext.bindService(intent,
                    mRegistrationStatusConnection,
                    Context.BIND_AUTO_CREATE);*/

        Intent intent = new Intent();
        ComponentName cmp = new ComponentName("com.mediatek.rcs", "com.mediatek.rcse.service.ApiService");
        intent.setComponent(cmp);
        boolean connected = mContext.bindService(intent, mRegistrationStatusConnection, Context.BIND_AUTO_CREATE);

        Logger.v(TAG, "connect() exit, the result is " + connected);
    }
    /*
     * (non-Javadoc)
     *
     * @see com.mediatek.rcse.api.RemoteRCSeApi#disconnect()
     */
    @Override
    public void disconnect() {
        mContext.unbindService(mRegistrationStatusConnection);
    }

    /**
     * The m registration status connection.
     */
    protected ServiceConnection mRegistrationStatusConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName className,
                IBinder service) {

            mApiServiceWrapperBinder = IApiServiceWrapper.Stub.asInterface(service);
            IBinder IRegistrationBinder = null;
            try {
                IRegistrationBinder = mApiServiceWrapperBinder.getRegistrationStatusBinder();
            } catch (RemoteException e1) {
                e1.printStackTrace();
            }
            mRegistrationStatus = IRegistrationStatus.Stub.asInterface(IRegistrationBinder);

            if (null != mRegistrationStatus) {
                try {
                    mRegistrationStatus
                            .addRegistrationStatusListener(RegistrationApi.this);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
            handleConnected();
            Logger.v(TAG,
                    "onServiceConnected() exit, mRegistrationStatus: "
                            + mRegistrationStatus);
        }
        @Override
        public void onServiceDisconnected(ComponentName className) {
            Logger.v(TAG, "onServiceDisconnected() entry");
            mRegistrationStatus = null;
            handleDisconnected();
            Logger.v(TAG, "onServiceDisconnected() exit");
        }
    };

    /*
     * (non-Javadoc)
     *
     * @see
     * com.mediatek.rcse.api.IRegistrationStatusRemoteListener#onStatusChanged
     * (boolean)
     */
    /**
     * On status changed.
     *
     * @param status the status
     * @throws RemoteException the remote exception
     */
    @Override
    public void onStatusChanged(boolean status)
            throws RemoteException {
        for (IRegistrationStatusListener listener : mListeners) {
            listener.onStatusChanged(status);
        }
    }
    /**
     * Check whether the RCS-e account is registered.
     *
     * @return true If RCS-e account is registered and RCS-e services are
     *         enabled. false If RCS-e account is not registered and RCS-e
     *         services are disabled.
     */
    public boolean isRegistered() {
        Logger.v(TAG, "isRegistered() entry");
        if (null != mRegistrationStatus) {
            try {
                boolean result = mRegistrationStatus.isRegistered();
                Logger.i(TAG, "isRegistered() exit, the result is "
                        + result);
                return result;
            } catch (RemoteException e) {
                e.printStackTrace();
                return false;
            }
        } else {
            Logger.i(TAG, "isRegistered(), mRegistrationStatus is null ");
            return false;
        }
    }
    /*
     * (non-Javadoc)
     *
     * @see com.mediatek.rcse.api.RemoteRCSeApi#handleConnected()
     */
    @Override
    public void handleConnected() {
        Logger.v(TAG, "handleConnected()");
    }
    /*
     * (non-Javadoc)
     *
     * @see com.mediatek.rcse.api.RemoteRCSeApi#handleDisconnected()
     */
    @Override
    public void handleDisconnected() {
        Logger.v(TAG, "hanldeDisconnected()");
    }
    /*
     * (non-Javadoc)
     *
     * @see
     * com.mediatek.rcse.settings.RcsSettings#setServiceActivationState
     * ()
     */
    /**
     * Sets the service activation state.
     *
     * @param state the new service activation state
     */
    public static void setServiceActivationState(boolean state) {
        Logger.v(TAG,
                "setServiceActivationState() entry, the state is "
                        + state);
        RcsSettings.getInstance().setServiceActivationState(state);
        Logger.v(TAG, "setServiceActivationState() exit ");
    }
    /*
     * (non-Javadoc)
     *
     * @see
     * com.mediatek.rcse.settings.RcsSettings#isServiceActivated()
     */
    /**
     * Checks if is service activated.
     *
     * @return true, if is service activated
     */
    public static boolean isServiceActivated() {
        return RcsSettings.getInstance().isServiceActivated();
    }
}
