/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.mediatek.op18.settings;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Handler;

import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionInfo;
import android.telephony.TelephonyManager;
import android.util.Log;
import java.util.List;
/**
 * "SmartCallFwdService" is a service to send and receive command to server
 * and communicate the result to app.
 */
public class SmartCallFwdService extends Service {

    private static final String TAG = "Op18SmartCallFwdService";
    private static final boolean DBG = true;
    private MyHandler mHandler = new MyHandler();
    //private IncomingHandler mIHandler = new IncomingHandler();
    private Phone mPhone;
    private Phone[] mPhoneObjArr = new Phone[2];
    CallForwardInfo callForwardInfo;
    private int mCurrSim;
    private String mPhnum = null;
    private Bundle mBundle = new Bundle();

    /** Command to the service to display a message */
    private final int MSG_TYPE_DATA = 0;
    private final int MSG_TYPE_REGISTER = 1;
    private final int MSG_GET_CF_REQ = 2;
    private final int MSG_SET_CF_REQ = 3;
    private final int MSG_GET_CF_RES = 4;
    private final int MSG_SET_CF_RES = 5;
    private final int MSG_TYPE_DEREGISTER = 6;

    private final int READ = 0;
    private final int WRITE = 1;
    private final int CF_ACTION_DISABLE = 0;
    private final int CF_ACTION_ENABLE = 1;
    private final int CF_REASON_NOT_REACHABLE = 3;
    /**
     * Target we publish for clients to send messages to IncomingHandler.
     */
    final Messenger mMessenger = new Messenger(new IncomingHandler());
    Messenger mResponseMessenger = null;
    /**
     * Handler of incoming messages from clients.
     */
    class IncomingHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage:" + msg.what);
            switch (msg.what) {
                case MSG_GET_CF_REQ:
                    Log.d(TAG, "MSG_GET_CF_REQ");
                    handleReqFromActivity(msg);
                    break;
                case MSG_SET_CF_REQ:
                    Log.d(TAG, "MSG_SET_CF_REQ");
                    handleReqFromActivity(msg);
                    break;
                case MSG_TYPE_REGISTER:
                    Log.d(TAG, "MSG_TYPE_REGISTER");
                    mResponseMessenger = msg.replyTo;
                    sendToActivity(null, MSG_TYPE_REGISTER);
                    break;
                case MSG_TYPE_DEREGISTER:
                    mResponseMessenger = null;
                    break;
                default:
                    super.handleMessage(msg);
            }
        }
    }

    private void handleReqFromActivity(Message msg) {
        Bundle b = msg.getData();
        int simId = b.getInt("simId");
        int action = b.getInt("action");
        int act = b.getInt("act");
        Log.d(TAG, "handleReqFromActivity : simId: " + simId);
        mCurrSim = simId;
        mPhone = mPhoneObjArr[simId]; //getPhoneUsingSlotId(simId);
        if (mPhone == null) {
            Log.d(TAG, "mPhone:" + mPhone);
            return;
        }
        if (act == READ) {
            readSmartCallSettings();
        } else {
            Log.d(TAG, "handleReqFromActivity : action: " + action);
            writeSmartCallSettings(b);
        }
    }

    private void readSmartCallSettings() {
        Log.d(TAG, "readSmartCallSettings");
        if (mPhone != null) {
            mPhone.getCallForwardingOption(CommandsInterface.CF_REASON_NOT_REACHABLE,
                    mHandler.obtainMessage(MyHandler.MESSAGE_GET_CF,
                    CommandsInterface.CF_ACTION_DISABLE, MyHandler.MESSAGE_GET_CF, null));
        }
    }

    private void writeSmartCallSettings(Bundle b) {
        int action = b.getInt("action");
        int timeSlot = 0;
        Log.d(TAG, "action: " + action);
        mPhnum = b.getString("phnum");
        Log.d(TAG, "writeSmartCallSettings: " + mPhnum);
        if (mPhone != null) {
            mPhone.setCallForwardingOption(action,
            CF_REASON_NOT_REACHABLE,
            mPhnum,
            timeSlot,
            mHandler.obtainMessage(MyHandler.MESSAGE_SET_CF, action, MyHandler.MESSAGE_SET_CF));
        }
    }

    /**
     * When binding to the service, we return an interface to our messenger
     * for sending messages to the service.
     */
    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        initializePhone();
        return mMessenger.getBinder();
    }

    private void initializePhone() {
        mPhoneObjArr[0] = getPhoneUsingSlotId(0);
        mPhoneObjArr[1] = getPhoneUsingSlotId(1);
        Log.d(TAG, "mPhoneObjArr[0]: " + mPhoneObjArr[0]);
        Log.d(TAG, "mPhoneObjArr[1]: " + mPhoneObjArr[1]);
    }

    /**
     * Get phone by sub id.
     * @param subId the sub id
     * @return phone according to the sub id
     */
    private Phone getPhoneUsingSlotId(int slotId) {
        int []subIdArray = null;
        subIdArray = SubscriptionManager.getSubId(slotId);
        Log.d(TAG, "subId:" + subIdArray[0]);
        try {
            int phoneId = SubscriptionManager.getPhoneId(subIdArray[0]);
            if (phoneId < 0 || phoneId >= TelephonyManager.getDefault().getPhoneCount()) {
                return PhoneFactory.getPhone(0);
            }
            return PhoneFactory.getPhone(phoneId);
        } catch (Exception e) {
             e.printStackTrace();
             return null;
        }
    }

    /**
     * Check whether there has active SubInfo indicated by given subId on the device.
     * @param context the current context
     * @param subId the sub id
     * @return true if the sub id is valid, else return false
     */
    private boolean isValidSubId(Context context, int subId) {
        Log.d(TAG, "isValidSubId subId:" + subId);
        boolean isValid = false;
        List<SubscriptionInfo> activeSubInfoList = SubscriptionManager
                .from(context).getActiveSubscriptionInfoList();
        if (activeSubInfoList != null) {
            for (SubscriptionInfo subscriptionInfo : activeSubInfoList) {
                if (subscriptionInfo.getSubscriptionId() == subId) {
                    isValid = true;
                    break;
                }
            }
        }
        return isValid;
    }


    private void sendToActivity(Bundle b, int msgId) {
        if (mResponseMessenger == null) {
            Log.d(TAG, "Cannot send message to activity - no activity registered to this service.");
        } else {
            Log.d(TAG, "Sending message to activity: ");
            Message msg = Message.obtain(null, msgId);
            if (b != null) {
                msg.setData(b);
            }
            try {
                mResponseMessenger.send(msg);
            } catch (RemoteException e) {
                e.printStackTrace();
            }
        }
    }

    // Message protocol:
    // what: get vs. set
    // arg1: action -- register vs. disable
    // arg2: get vs. set for the preceding request
    private class MyHandler extends Handler {
        static final int MESSAGE_GET_CF = 0;
        static final int MESSAGE_SET_CF = 1;
        static final int MESSAGE_GET_CALL_WAIT_RES = 3;
        static final int MESSAGE_SET_CALL_WAIT_RES = 4;
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_GET_CF:
                    handleGetCFResponse(msg);
                    break;
                case MESSAGE_SET_CF:
                    handleSetCFResponse(msg);
                    break;
                case MESSAGE_GET_CALL_WAIT_RES:
                    handleGetCallWaitResponse(msg);
                    break;
                case MESSAGE_SET_CALL_WAIT_RES:
                    handleSetCallWaitResponse(msg);
                    break;
            }
        }

        private void handleGetCallWaitResponse(Message msg) {
            AsyncResult ar = (AsyncResult) msg.obj;
            int callWaitStatus = 0;
            Bundle b = (Bundle) mBundle.clone();
            if (ar.exception != null) {
                Log.d(TAG, "handleGetCallWaitResponse: ar.exception=" + ar.exception);
                b.putInt("err", -1);
                if (msg.arg1 == MESSAGE_SET_CALL_WAIT_RES) {
                    sendToActivity(b, MSG_SET_CF_RES);
                } else {
                    sendToActivity(b, MSG_GET_CF_RES);
                }
            } else {
                Log.d(TAG, "handleGetCallWaitingResponse: CW state successfully queried.");
                int[] cwArray = (int[]) ar.result;
                // If cwArray[0] is = 1, then cwArray[1] must follow,
                // with the TS 27.007 service class bit vector of services
                // for which call waiting is enabled.
                try {
                    Log.d(TAG, "handleGetCallWaitingResponse cwArray[0]:cwArray[1] = "
                            + cwArray[0] + ":" + cwArray[1]);
                    if (((cwArray[0] == 1) && ((cwArray[1] & 0x01) == 0x01))) {
                        callWaitStatus = 1;
                        Log.d(TAG, "callWaitStatus: " + callWaitStatus);
                    }
                    b.putInt("callwait", callWaitStatus);

                    if (msg.arg1 == MESSAGE_SET_CALL_WAIT_RES) {
                        sendToActivity(b, MSG_SET_CF_RES);
                    } else {
                        sendToActivity(b, MSG_GET_CF_RES);
                    }
                } catch (ArrayIndexOutOfBoundsException e) {
                    Log.e(TAG, "handleGetCallWaitingResponse: improper result: err ="
                            + e.getMessage());
                }
            }
        }

        private void handleSetCallWaitResponse(Message msg) {
            final AsyncResult ar = (AsyncResult) msg.obj;

            final Phone phone = mPhoneObjArr[(mCurrSim + 1) % 2];
            Log.d(TAG, "phone:" + phone);
            if (ar.exception != null) {
                if (DBG) {
                    Log.d(TAG, "handleSetCallWaitingResponse: ar.exception=" + ar.exception);
                }
            }
            if (DBG) {
                Log.d(TAG, "handleSetCallWaitingResponse: re get start");
            }
            /// M: modem has limitation that if query result immediately set, will
            //  not get the right result, so we need wait 1s to query. just AP workaround @{
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    if (DBG) {
                        Log.d(TAG, "handleSetCallWaitingResponse: re get");
                    }
                    phone.getCallWaiting(obtainMessage(MESSAGE_GET_CALL_WAIT_RES,
                            MESSAGE_SET_CALL_WAIT_RES, MESSAGE_SET_CALL_WAIT_RES, ar.exception));
                }
            };
            postDelayed(runnable, 1000);
            // @}
        }

        private void handleGetCFResponse(Message msg) {
            Log.d(TAG, "handleGetCFResponse: done");
            String err;
            boolean callWaitAction = false;

            AsyncResult ar = (AsyncResult) msg.obj;
            Bundle b = new Bundle();
            b.putInt("simId", mCurrSim);
            final Phone phone = mPhoneObjArr[(mCurrSim + 1) % 2];
            Log.d(TAG, "phone:" + phone);
            callForwardInfo = null;
            if (ar.exception != null) {
                Log.d(TAG, "handleGetCFResponse: ar.exception=" + ar.exception);
                //err = "Exception: " + ar.exception;
                b.putInt("err", -1);
                if (msg.arg2 == MESSAGE_SET_CF) {
                    sendToActivity(b, MSG_SET_CF_RES);
                } else {
                    sendToActivity(b, MSG_GET_CF_RES);
                }
            } else {
                CallForwardInfo cfInfoArray[] = (CallForwardInfo[]) ar.result;
                if (cfInfoArray == null || cfInfoArray.length == 0) {
                    Log.d(TAG, "handleGetCFResponse: cfInfoArray.length==0");
                } else {
                    boolean classFound = false;
                    for (int i = 0, length = cfInfoArray.length; i < length; i++) {
                        Log.d(TAG, "handleGetCFResponse, cfInfoArray[" + i + "]="
                                + cfInfoArray[i]);
                        if ((CommandsInterface.SERVICE_CLASS_VOICE &
                        cfInfoArray[i].serviceClass) != 0) {
                            // corresponding class
                            classFound =  true;
                            CallForwardInfo info = cfInfoArray[i];
                            //b.putInt("simId", mCurrSim);
                            b.putInt("status", info.status);
                            b.putInt("reason", info.reason);
                            b.putString("phnum", info.number);

                            if (msg.arg2 == MESSAGE_SET_CF) {
                                b.putInt("action", msg.arg1);
                                Log.d(TAG, "set num: " + info.number + ", to be set = " + mPhnum);
                                if (msg.arg1 == CF_ACTION_ENABLE &&
                                        !PhoneNumberUtils.compareLoosely(info.number, mPhnum)) {
                                    b.putInt("err", 1);
                                } else if (msg.arg1 == CF_ACTION_ENABLE && info.status == 0) {
                                    //callWaitAction = false;
                                    b.putInt("err", 2); // fail to enable call forwarding
                                } else if (msg.arg1 == CF_ACTION_DISABLE && info.status == 1 &&
                                        PhoneNumberUtils.compareLoosely(info.number, mPhnum)) {
                                    //callWaitAction = true;
                                    b.putInt("err", 3); // fail to disable call forwarding
                                } else {
                                    b.putInt("err", 0); // call forward set action success
                                    callWaitAction = (msg.arg1 == CF_ACTION_ENABLE) ? true : false;
                                    Log.d(TAG, "Set call waiting, callWaitAction: " +
                                    callWaitAction);
                                }
                                mBundle.clear();
                                mBundle = b;
                                if (b.getInt("err") > 0) {
                                    sendToActivity(b, MSG_SET_CF_RES);
                                } else {
                                    phone.setCallWaiting(callWaitAction,
                                            mHandler.obtainMessage(MESSAGE_SET_CALL_WAIT_RES));
                                }
                            } else {
                                phone.getCallWaiting(mHandler.obtainMessage
                                (MESSAGE_GET_CALL_WAIT_RES));
                                mBundle.clear();
                                mBundle = b;
                            }
                        }
                    }
                    if (!classFound) {
                        b.putInt("err", 4); // Service class voice not found
                        Log.d(TAG, "Service class voice not found");
                        if (msg.arg2 == MESSAGE_SET_CF) {
                            sendToActivity(b, MSG_SET_CF_RES);
                        } else {
                            sendToActivity(b, MSG_GET_CF_RES);
                        }
                    }
                }
            }
        }

        private void handleSetCFResponse(Message msg) {
            final AsyncResult ar = (AsyncResult) msg.obj;
            Bundle b = new Bundle();
            b.putInt("simId", mCurrSim);
            if (ar.exception != null) {
                Log.d(TAG, "handleSetCFResponse: ar.exception=" + ar.exception);
                b.putInt("err", -1);
                if (msg.arg2 == MESSAGE_SET_CF) {
                    sendToActivity(b, MSG_SET_CF_RES);
                }
                return;
            }

            /// M: modem has limitation that if query result immediately set, will
            //  not get the right result, so we need wait 1s to query. just AP workaround @{
            final int arg1 = msg.arg1;
            Runnable runnable = new Runnable() {
                @Override
                public void run() {
                    if (DBG) {
                        Log.d(TAG, "handleSetCFResponse: re get");
                    }
                    mPhone.getCallForwardingOption(CF_REASON_NOT_REACHABLE,
                            obtainMessage(MESSAGE_GET_CF, arg1, MESSAGE_SET_CF, ar.exception));
                }
            };
            postDelayed(runnable, 1000);
            // @}
        }
    }
}
