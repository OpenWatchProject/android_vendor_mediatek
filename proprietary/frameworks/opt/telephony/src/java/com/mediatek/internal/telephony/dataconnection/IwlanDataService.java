/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony.dataconnection;

import android.net.LinkProperties;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.data.DataCallResponse;
import android.telephony.data.DataProfile;
import android.telephony.data.DataService;
import android.telephony.data.DataServiceCallback;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents IWLAN data service which handles telephony data requests and response
 * from the IWLAN.
 */
public class IwlanDataService extends DataService {
    private static final String TAG = IwlanDataService.class.getSimpleName();

    private static final boolean DBG = true;

    private static final int SETUP_DATA_CALL_COMPLETE               = 1;
    private static final int DEACTIVATE_DATA_ALL_COMPLETE           = 2;
    private static final int SET_INITIAL_ATTACH_APN_COMPLETE        = 3;
    private static final int SET_DATA_PROFILE_COMPLETE              = 4;
    private static final int GET_DATA_CALL_LIST_COMPLETE            = 5;
    private static final int DATA_CALL_LIST_CHANGED                 = 6;

    private class IwlanDataServiceProvider extends DataService.DataServiceProvider {

        private final Map<Message, DataServiceCallback> mCallbackMap = new HashMap<>();

        private final Looper mLooper;

        private final Handler mHandler;

        private final HandlerThread mHandlerThread;

        private final Phone mPhone;

        private IwlanDataServiceProvider(int slotId) {
            super(slotId);

            mPhone = PhoneFactory.getPhone(getSlotIndex());

            mHandlerThread = new HandlerThread(IwlanDataService.class.getSimpleName());
            mHandlerThread.start();
            mLooper = mHandlerThread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {
                    DataServiceCallback callback = mCallbackMap.remove(message);

                    AsyncResult ar = (AsyncResult) message.obj;
                    switch (message.what) {
                        case SETUP_DATA_CALL_COMPLETE:
                            DataCallResponse response = (DataCallResponse) ar.result;
                            callback.onSetupDataCallComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS,
                                    response);
                            break;
                        case DEACTIVATE_DATA_ALL_COMPLETE:
                            callback.onDeactivateDataCallComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS);
                            break;
                        case SET_INITIAL_ATTACH_APN_COMPLETE:
                            callback.onSetInitialAttachApnComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS);
                            break;
                        case SET_DATA_PROFILE_COMPLETE:
                            callback.onSetDataProfileComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS);
                            break;
                        case GET_DATA_CALL_LIST_COMPLETE:
                            callback.onRequestDataCallListComplete(
                                    ar.exception != null
                                            ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                            : DataServiceCallback.RESULT_SUCCESS,
                                    ar.exception != null
                                            ? null : (List<DataCallResponse>) ar.result
                                    );
                            break;
                        case DATA_CALL_LIST_CHANGED:
                            notifyDataCallListChanged((List<DataCallResponse>) ar.result);
                            break;
                        default:
                            loge("Unexpected event: " + message.what);
                            return;
                    }
                }
            };

            if (DBG) log("Register for data call list changed.");
            mPhone.mCi.registerForDataCallListChanged(mHandler, DATA_CALL_LIST_CHANGED, null);
        }

        @Override
        public void setupDataCall(int accessNetworkType, DataProfile dataProfile, boolean isRoaming,
                                  boolean allowRoaming, int reason, LinkProperties linkProperties,
                                  DataServiceCallback callback) {
            if (DBG) log("setupDataCall " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, SETUP_DATA_CALL_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            mPhone.mCi.setupDataCall(accessNetworkType, dataProfile, isRoaming, allowRoaming,
                    reason, linkProperties, message);
        }

        @Override
        public void deactivateDataCall(int cid, int reason, DataServiceCallback callback) {
            if (DBG) log("deactivateDataCall " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, DEACTIVATE_DATA_ALL_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            mPhone.mCi.deactivateDataCall(cid, reason, message);
        }

        @Override
        public void setInitialAttachApn(DataProfile dataProfile, boolean isRoaming,
                                        DataServiceCallback callback) {
            if (DBG) log("setInitialAttachApn " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, SET_INITIAL_ATTACH_APN_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            mPhone.mCi.setInitialAttachApn(dataProfile, isRoaming, message);
        }

        @Override
        public void setDataProfile(List<DataProfile> dps, boolean isRoaming,
                                   DataServiceCallback callback) {
            if (DBG) log("setDataProfile " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, SET_DATA_PROFILE_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            mPhone.mCi.setDataProfile(dps.toArray(new DataProfile[dps.size()]), isRoaming, message);
        }

        @Override
        public void requestDataCallList(DataServiceCallback callback) {
            if (DBG) log("requestDataCallList " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, GET_DATA_CALL_LIST_COMPLETE);
                mCallbackMap.put(message, callback);
            }
            mPhone.mCi.getDataCallList(message);
        }

        @Override
        public void close() {
            mPhone.mCi.unregisterForDataCallListChanged(mHandler);
            mHandlerThread.quit();
        }
    }

    @Override
    public DataServiceProvider onCreateDataServiceProvider(int slotIndex) {
        log("IWLAN data service created for slot " + slotIndex);
        if (!SubscriptionManager.isValidSlotIndex(slotIndex)) {
            loge("Tried to IWLAN data service with invalid slotId " + slotIndex);
            return null;
        }
        return new IwlanDataServiceProvider(slotIndex);
    }

    private void log(String s) {
        Rlog.d(TAG, s);
    }

    private void loge(String s) {
        Rlog.e(TAG, s);
    }
}
