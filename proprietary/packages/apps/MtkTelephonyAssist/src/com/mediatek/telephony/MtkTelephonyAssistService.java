/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

package com.mediatek.telephony;

import java.util.ArrayList;
import java.util.List;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;


public class MtkTelephonyAssistService extends Service {

    public static final int BASE = 0;
    public static final int EVENT_SERVICE_CREATED = BASE + 1;
    public static final int EVENT_SERVICE_DESTROYED = BASE + 2;
    public static final int EVENT_PHONE_CREATE_DONE = BASE + 3;

    private Context mContext;
    private boolean mPhoneCreated = false;
    private boolean mRadiosAvailable = false;
    private List<BaseController> mControllers;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action != null && action.equals(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED)
                    && !mPhoneCreated) {
                MtkTelephonyAssistGlobal.logd(this, "Receive ACTION_SIM_CARD_STATE_CHANGED");
                mHandler.obtainMessage(EVENT_PHONE_CREATE_DONE).sendToTarget();
            }
        }
    };

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_SERVICE_CREATED:
                    initControllers();
                    updatePhoneCreatedState();
                    break;
                case EVENT_PHONE_CREATE_DONE:
                    updatePhoneCreatedState();
                    break;
                case EVENT_SERVICE_DESTROYED:
                    onServiceDestroyed();
                    break;
                default:
                    MtkTelephonyAssistGlobal.loge(this, "Unhandled message with number: " + msg.what);
                    break;
            }
        }
    };

    @Override
    public void onCreate() {
        mContext = getBaseContext();
        MtkTelephonyAssistGlobal.sServiceAlve = true;
        MtkTelephonyAssistGlobal.logd(this, "MtkTelephonyService created");
        IntentFilter intentFilter = new IntentFilter(TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED);
        registerReceiver(mReceiver, intentFilter);
        mHandler.obtainMessage(EVENT_SERVICE_CREATED).sendToTarget();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return Service.START_STICKY;
    }

    @Override
    public void onDestroy() {
        MtkTelephonyAssistGlobal.sServiceAlve = false;
        mPhoneCreated = false;
        MtkTelephonyAssistGlobal.logd(this, "MtkTelephonyService destroyed");
        unregisterReceiver(mReceiver);
        mHandler.obtainMessage(EVENT_SERVICE_DESTROYED).sendToTarget();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void initControllers() {
        mControllers = new ArrayList<BaseController>();
        mControllers.add(new CommonStateController(mContext));
        mControllers.add(new DataStateController(mContext));
    }

    private boolean isPhoneCreated() {
        return mPhoneCreated;
    }

    private void onServiceDestroyed() {
        for (BaseController controler : mControllers) {
            controler.onDestroy();
        }
    }

    private void updatePhoneCreatedState() {
        boolean phoneCreated = mPhoneCreated;
        Phone[] phones = null;
        try {
            phones = PhoneFactory.getPhones() ;
            mPhoneCreated = (phones != null);
        } catch (IllegalStateException e) {
            MtkTelephonyAssistGlobal.logd(this, "Get IllegalStateException, phone not inited");
            mPhoneCreated = false;
        }

        MtkTelephonyAssistGlobal.logd(this, "phoneCreated = " + phoneCreated
                + " mPhoneCreated = " + mPhoneCreated);
        if (!phoneCreated && mPhoneCreated && phones != null) {
            AssistRIL[] assistRIL = initAssistRIL(phones);
            for (BaseController controler : mControllers) {
                controler.onPhoneCreated(phones, assistRIL);
            }
        }
    }

    private AssistRIL[] initAssistRIL(Phone[] phones) {
        AssistRIL[] assistRIL = new AssistRIL[phones.length];
        for (int i = 0; i < phones.length; i++) {
            assistRIL[i] = new AssistRIL(mContext, i);
        }

        return assistRIL;
    }
}