/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.ims.rcsua.service;

import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import com.mediatek.ims.rcsua.RegistrationInfo;
import com.mediatek.ims.rcsua.service.utils.Logger;

public class RuaClient {

    RuaClient(RuaService service, IImsEventCallback callback) {
        this.service = service;
        this.callback = callback;
        callbacks.register(callback);
        this.adapter = RuaAdapter.getInstance();
        this.intrf = new RcsUaClientImpl();
    }

    ISipChannel openSipChannel(ISipEventCallback callback, int mode, RcsUaException exception) {
        logger.debug("openSipChannel:" + channel);
        if (channel != null) {
            exception.set(new IllegalStateException("Channel already opened"));
            return null;
        }
        channel = new RuaChannel(this, callback, mode);

        return channel.getBinder();
    }

    void resumeImsDeregistration() {
        logger.debug("resumeImsDeregistration");

        adapter.resumeImsDeregistration();
    }

    RegistrationInfo getRegistrationInfo() {
        return adapter.getRegistrationInfo();
    }

    IRcsUaClient getInterface() {
        return intrf;
    }

    void channelClosed(RuaChannel channel) {
        if (this.channel == channel) {
            this.channel = null;
        }
    }

    void activate() {
        if (channel != null) {
            channel.setEnabled(true);
        }
    }

    void deactivate() {
        if (channel != null) {
            channel.setEnabled(false);
        }
    }

    void unregister() {
        if (channel != null) {
            channel.close();
            channel = null;
        }

        callbacks.unregister(callback);
        callback = null;
    }

    void notifyRegistrationChanged(RegistrationInfo info) {
        if (callbacks != null) {
            synchronized (callbacks) {
                final int N = callbacks.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        callbacks.getBroadcastItem(i).onStatusChanged(info);
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object for us.
                    }
                }
                callbacks.finishBroadcast();
            }
        }
    }

    void notifyReRegistration(RegistrationInfo info) {
        if (callbacks != null) {
            synchronized (callbacks) {
                final int N = callbacks.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        callbacks.getBroadcastItem(i).onReregistered(info);
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object for us.
                    }
                }
                callbacks.finishBroadcast();
            }
        }
    }

    void notifyDeregStart(RegistrationInfo info) {
        if (callbacks != null) {
            synchronized (callbacks) {
                final int N = callbacks.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        callbacks.getBroadcastItem(i).onDeregStarted(info);
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object for us.
                    }
                }
                callbacks.finishBroadcast();
            }
        }
    }

    void notifyVopsInfication(int vops) {
        if (callbacks != null) {
            synchronized (callbacks) {
                final int N = callbacks.beginBroadcast();
                for (int i = 0; i < N; i++) {
                    try {
                        callbacks.getBroadcastItem(i).onVopsIndication(vops);
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object for us.
                    }
                }
                callbacks.finishBroadcast();
            }
        }
    }

    private class RcsUaClientImpl extends IRcsUaClient.Stub {

        @Override
        public RegistrationInfo getRegistrationInfo() throws RemoteException {
            return RuaClient.this.getRegistrationInfo();
        }

        @Override
        public void resumeImsDeregistration() throws RemoteException {
            RuaClient.this.resumeImsDeregistration();
        }

        @Override
        public ISipChannel openSipChannel(ISipEventCallback callback, int mode, RcsUaException exception) throws RemoteException {
            return RuaClient.this.openSipChannel(callback, mode, exception);
        }
    }

    private RemoteCallbackList<IImsEventCallback> callbacks = new RemoteCallbackList<IImsEventCallback>() {
        @Override
        public void onCallbackDied(IImsEventCallback callback) {

        }
    };

    private RuaAdapter adapter;
    private RuaService service;
    private RuaChannel channel;
    private IImsEventCallback callback;
    private RcsUaClientImpl intrf;
    private Logger logger = Logger.getLogger(RuaClient.class.getName());

}
