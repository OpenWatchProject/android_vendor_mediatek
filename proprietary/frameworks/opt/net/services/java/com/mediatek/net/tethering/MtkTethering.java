/*
 * Copyright (C) 2017 MediaTek Inc.
 *
 * Modification based on code covered by the mentioned copyright
 * and/or permission notice(s).
 */
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.mediatek.net.tethering;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

import com.android.server.connectivity.Tethering;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;


/**
 *
 * @hide
 */
public class MtkTethering {
    private static final boolean DBG = false;
    private static final String TAG = "MtkTethering";

    static final int EVENT_BOOTUP = 1;

    private final Context mContext;
    private static Tethering sTethering;

    protected final HandlerThread mHandlerThread;
    final private InternalHandler mHandler;

    public MtkTethering(Context context, Tethering tethering) {
        mContext = context;
        sTethering = tethering;

        mHandlerThread = new HandlerThread("TetheringInternalHandler");
        mHandlerThread.start();
        mHandler = new InternalHandler(mHandlerThread.getLooper());

        mHandler.sendEmptyMessage(EVENT_BOOTUP);
    }

    // must be stateless - things change under us.
    private class InternalHandler extends Handler {
        public InternalHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_BOOTUP:
                    checkEmSetting();
                    break;
                default:
                    Log.e(TAG, "Invalid message: " + msg.what);
                    break;
            }
        }
    }

    private void checkEmSetting() {
        boolean isBgdataDisabled =
                SystemProperties.getBoolean("persist.vendor.radio.bgdata.disabled", false);
        if (isBgdataDisabled) {
            try {
                INetdagent netagent = INetdagent.getService();
                if (netagent == null) {
                    Log.e(TAG, "netagent is null");
                    return;
                }
                Log.d(TAG, "setIotFirewall");
                netagent.dispatchNetdagentCmd("netdagent firewall set_nsiot_firewall");
            } catch (Exception e) {
                Log.d(TAG, "Exception:" + e);
            }
        }
    }
}
