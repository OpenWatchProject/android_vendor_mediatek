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

package com.mediatek.ims.rcsua;

import android.os.RemoteException;
import android.support.annotation.Nullable;
import android.support.annotation.NonNull;
import android.util.Log;

import com.mediatek.ims.rcsua.service.IRcsUaClient;
import com.mediatek.ims.rcsua.service.ISipEventCallback;
import com.mediatek.ims.rcsua.service.ISipChannel;
import com.mediatek.ims.rcsua.service.RcsUaException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashSet;

/**
 * API class represent for UA client.
 */
public final class Client {

    /**
     * Open SIP channel to start SIP sending/receiving.
     *
     * @param sipCallback callback used to receive SIP message.
     * @return SipChannel instance.
     * @throws IOException
     */
    public SipChannel openSipChannel(@NonNull SipChannel.EventCallback sipCallback) throws IOException {
        /* Currently only support TCP and UDP transport */
        int transport = Configuration.TCP | Configuration.UDP;
        return openSipChannel(transport, sipCallback, 0);
    }

    /**
     * Open SIP channel to start SIP sending/receiving.
     *
     * @param sipCallback callback used to receive SIP message.
     * @return SipChannel instance.
     * @throws IOException
     */
    public SipChannel openSipChannel(@NonNull SipChannel.EventCallback sipCallback, int mode) throws IOException {
        /* Currently only support TCP and UDP transport */
        int transport = Configuration.TCP | Configuration.UDP;
        return openSipChannel(transport, sipCallback, mode);
    }

    /**
     * Open SIP channel to start SIP sending/receiving.
     *
     * @param sipCallback callback used to receive SIP message.
     * @return SipChannel instance.
     * @throws IOException
     */
    public SipChannel openSipChannel(int transport, @NonNull SipChannel.EventCallback sipCallback, int  mode) throws IOException {
        Log.d(TAG, "Client openSipChannel with active:" + activeChannels);
        if (!service.isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        if (channelIntf == null) {
            synchronized (lock) {
                if (channelIntf == null) {
                    try {
                        RcsUaException ex = new RcsUaException();
                        channelIntf = clientIntf.openSipChannel(sipEventCallback, mode, ex);
                        if (ex.isSet()) {
                            ex.throwException();
                        }
                    } catch (RemoteException e) {
                        channelIntf = null;
                        return null;
                    }
                }
            }
        } else {
            for (SipChannel channel : activeChannels) {
                if (channel.isTransportSupported(transport))
                    return channel;
            }
        }

        SipChannel channel = new SipChannel(service, this, channelIntf, transport, sipCallback);
        activeChannels.add(channel);

        return channel;
    }

    /**
     * Register IMS event callback, through which IMS event can be received.
     *
     * @param callback to be registered.
     */
    public void registerImsEventCallback(@NonNull ImsEventCallback callback) {
        synchronized (lock) {
            callbacks.add(callback);
        }
    }

    /**
     * Unregister IMS event callback.
     * @param callback to be unregistered.
     */
    public void unregisterImsEventCallback(@NonNull ImsEventCallback callback) {
        synchronized (lock) {
            callbacks.remove(callback);
        }
    }

    /**
     * Retrieve current IMS registration information.
     *
     * @return IMS registration information.
     */
    public RegistrationInfo getRegistrationInfo() {
        if (!service.isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        RegistrationInfo regInfo = null;
        try {
            regInfo = clientIntf.getRegistrationInfo();
        } catch (RemoteException e) {

        }

        if (regInfo == null) {
            regInfo = new RegistrationInfo();
        }
        return regInfo;
    }


    /**
     * Resume IMS deregistration.
     * Only available when OPTION_DEREG_SUSPEND > 0
     */
    public void resumeImsDeregistration() {
        if (!service.isConnected()) {
            throw new IllegalStateException("RCS UA service disconnected");
        }

        try {
            clientIntf.resumeImsDeregistration();
        } catch (RemoteException e) {

        }
    }

    IRcsUaClient getInterface() {
        return clientIntf;
    }

    void handleImsEventCallback(RegistrationInfo regInfo) {
        Log.d(TAG, "handleImsEventCallback");

        int state = regInfo.getRegState();
        int mode = regInfo.getRegMode();

        for (ImsEventCallback callback : callbacks) {
            callback.run(callback.new Runner(state, mode));
        }
    }

    void handleImsReregistered(RegistrationInfo regInfo) {
        Log.d(TAG, "handleImsReregistered");
        // TODO: hack to reuse appCallback
        int state = 256;
        int mode = regInfo.getRegMode();

        for (ImsEventCallback callback : callbacks) {
            callback.run(callback.new Runner(state, mode));
        }
    }

    void handleImsDeregInd(RegistrationInfo regInfo) {
        Log.d(TAG, "handleImsDeregInd");
        // TODO: hack to reuse appCallback
        int state = 128;
        int mode = regInfo.getRegMode();

        for (ImsEventCallback callback : callbacks) {
            callback.run(callback.new Runner(state, mode));
        }
    }

    void handleVopsInd(int vops) {
        Log.d(TAG, "handleVopsInd:" + vops);
        // TODO: hack to reuse appCallback
        int state = 512;
        int mode = 0;
        for (ImsEventCallback callback : callbacks) {
            callback.run(callback.new Runner(state, mode, vops));
        }
    }

    Client(RcsUaService service, IRcsUaClient clientIntf) {
        this.service = service;
        this.clientIntf = clientIntf;
    }

    int channelClosed(SipChannel channel) {
        activeChannels.remove(channel);
        int activeChannelNum = activeChannels.size();
        Log.d(TAG, "channelClosed, activeChannelNum = " + activeChannelNum);
        if (activeChannelNum == 0) {
            channelIntf = null;
        }
        return activeChannelNum;
    }

    ISipEventCallback sipEventCallback = new ISipEventCallback.Stub() {
        @Override
        public void messageReceived(int transport, byte[] message) throws RemoteException {
            for (SipChannel channel : activeChannels) {
                if (channel.isTransportSupported(transport)) {
                    channel.handleSipMessageReceived(message);
                    break;
                }
            }
        }
    };

    private RcsUaService service;
    private IRcsUaClient clientIntf;
    private ISipChannel channelIntf;
    private HashSet<ImsEventCallback> callbacks = new HashSet<ImsEventCallback>();
    private volatile ArrayList<SipChannel> activeChannels = new ArrayList<>();

    private Object lock = new Object();
    private static final String TAG = "[RcsUaService][API]";
}
