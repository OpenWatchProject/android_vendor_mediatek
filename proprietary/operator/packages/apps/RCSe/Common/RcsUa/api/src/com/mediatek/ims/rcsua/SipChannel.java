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
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.util.Log;
import com.mediatek.ims.rcsua.service.ISipChannel;
import com.mediatek.ims.rcsua.service.RcsUaException;
import java.io.IOException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * Wrapping class used to sending/receiving SIP message.
 */
public final class SipChannel {

    /**
     * SIP channel mode
     */
    @IntDef({
            MODE_UNIFIED,
            MODE_STANDALONE_PRESENCE
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface ChannelMode {}

    /**
     * Normal SIP channel, no filter applied
     */
    public static final int MODE_UNIFIED = 0;

    /**
     * Restricted mode, non-IM message filter applied.
     */
    public static final int MODE_STANDALONE_PRESENCE = 1;

    /**
     * send SIP message to remote.
     *
     * @param message SIP raw data refer RFC3261 for SIP format.
     * @throws IOException
     */
    public void sendMessage(@NonNull byte[] message) throws IOException {
        if (!isConnected()) {
            throw new IOException("SIP channel not available");
        }

        try {
            RcsUaException ex = new RcsUaException();
            channelIntf.sendMessage(message, ex);
            if (ex.isSet()) {
                ex.throwException();
            }
        } catch (RemoteException e) {

        }
    };

    /**
     * close SIP channel. After channel closed, no more SIP message can be received.
     */
    public void close() {
        if (channelIntf == null)
            return;
        int restActiveChannelNum = client.channelClosed(this);

        try {
            if (restActiveChannelNum == 0) {
                RcsUaException ex = new RcsUaException();
                channelIntf.close(ex);
                channelIntf = null;
            }
        } catch (RemoteException e) {

        }
    }

    /**
     * To check whether SIP channel is avoilable to send/receive SIP messages.
     * @return ture for available to be used.
     */
    public boolean isConnected() {
        boolean connected = false;

        if (channelIntf != null) {
            if (service.isActivated()) {
                try {
                    connected = channelIntf.isAvailable();
                } catch (RemoteException e) {
                }
            }
        }

        Log.d(TAG, "isConnected[" + connected + "]: channelIntf[" + channelIntf + "]");

        return connected;
    }

    /**
     * Event callback used for SIP message receiving.
     */
    public static abstract class EventCallback extends AppCallback {

        /**
         * called when SIP message received from peer.
         *
         * @param message SIP raw data received. refer RFC3621 for SIP format.
         */
        public void messageReceived(byte[] message) {
        }

        class Runner extends BaseRunner<byte[]> {

            Runner(byte[]... params) {
                super(params);
            }

            @Override
            void exec(byte[]... params) {
                messageReceived(params[0]);
            }
        }
    }

    SipChannel(RcsUaService service, Client client, ISipChannel channel, int transport, EventCallback callback) {
        this.channelIntf = channel;
        this.callback = callback;
        this.service = service;
        this.client = client;
        this.transport = transport;
    }

    boolean isTransportSupported(int transport) {
        return ((this.transport & transport) > 0);
    }

    void handleSipMessageReceived(byte[] message) {
        callback.run(callback.new Runner(message));
    }

    private ISipChannel channelIntf;
    private EventCallback callback;
    private RcsUaService service;
    private Client client;
    private int transport;
    private static final String TAG = "[RcsUaService][API]";
}
