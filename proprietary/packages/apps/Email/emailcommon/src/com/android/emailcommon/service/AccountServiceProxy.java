/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.emailcommon.service;

import android.content.Context;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;

import com.android.emailcommon.service.ServiceProxy.ProxyTask;
import com.google.common.annotations.VisibleForTesting;

public class AccountServiceProxy extends ServiceProxy implements IAccountService {

    public static final int DEFAULT_ACCOUNT_COLOR = 0xFF0000FF;

    private IAccountService mService = null;
    private Object mReturn;

    @VisibleForTesting
    public AccountServiceProxy(Context _context) {
        super(_context, getIntentForEmailPackage(_context, "ACCOUNT_INTENT"));
    }

    @Override
    public void onConnected(IBinder binder) {
        mService = IAccountService.Stub.asInterface(binder);
    }

    @Override
    public IBinder asBinder() {
        return null;
    }

    /// M: Notify that mails had been sent successfully
    @Override
    public void notifySendingSucceeded(final long accountId, final int messageCount) {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException {
                mService.notifySendingSucceeded(accountId, messageCount);
            }
        }, "notifySendingSucceeded");
    }

    /// M: Notify that mails had been sent failed
    @Override
    public void notifySendingFailed(final long accountId, final int messageCount) {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException {
                mService.notifySendingFailed(accountId, messageCount);
            }
        }, "notifySendingFailed");
    }

    /// M: Notify that mails are being sent
    @Override
    public void notifySendingStarted(final long accountId, final int messageCount) {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException {
                mService.notifySendingStarted(accountId, messageCount);
            }
        }, "notifySendingStarted");
    }

    // The following call is synchronous, and should not be made from the UI thread
    @Override
    public int getAccountColor(final long accountId) {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException{
                mReturn = mService.getAccountColor(accountId);
            }
        }, "getAccountColor");
        waitForCompletion();
        if (mReturn == null) {
            return DEFAULT_ACCOUNT_COLOR;
        } else {
            return (Integer)mReturn;
        }
    }

    // The following call is synchronous, and should not be made from the UI thread
    @Override
    public Bundle getConfigurationData(final String accountType) {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException{
                mReturn = mService.getConfigurationData(accountType);
            }
        }, "getConfigurationData");
        waitForCompletion();
        if (mReturn == null) {
            return null;
        } else {
            return (Bundle)mReturn;
        }
    }

    // The following call is synchronous, and should not be made from the UI thread
    @Override
    public String getDeviceId() {
        setTask(new ProxyTask() {
            @Override
            public void run() throws RemoteException{
                mReturn = mService.getDeviceId();
            }
        }, "getDeviceId");
        waitForCompletion();
        if (mReturn == null) {
            return null;
        } else {
            return (String)mReturn;
        }
    }
}

