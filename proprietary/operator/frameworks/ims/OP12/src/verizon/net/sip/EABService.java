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

package verizon.net.sip;

import com.gsma.services.rcs.capability.Capabilities;
import com.gsma.services.rcs.capability.CapabilitiesListener;
import com.gsma.services.rcs.capability.CapabilityService;
import com.gsma.services.rcs.contact.ContactId;
import com.gsma.services.rcs.contact.ContactUtil;
import com.gsma.services.rcs.RcsServiceListener;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Hashtable;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

public class EABService {
    private static final String TAG = "EABService";
    private static final int CAPABILITY_VOICE_OVER_LTE = 0x00000001;
    private static final int CAPABILITY_VIDEO_OVER_LTE = 0x00000002;
    private static HashMap<String, ContactTuple> mContactMap =
                            new HashMap<String, ContactTuple>();
    private static CapabilityService mCapabilitiesApi = null;
    private static MyCapabilitiesListener mMyCapabilitiesListener = null;
    private static MyRcsServiceListener myRcsServiceListener = null;
    private static Context mContext = null;
    private static Handler mHandler = new MyHandler();
    private static Listener mListener = null;
    private static boolean mCapabilitiesApiConnected = false;

    private static class MyHandler extends Handler {
        static final int MSG_START_CALLBACK = 0;
        static final int MSG_GET_CAPABILITIES = 1;
        static final int MSG_GET_AVAILABILITY = 2;
        static final int MSG_START_ADVERTISE = 3;

        MyHandler() {
            super(Looper.getMainLooper());
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_START_CALLBACK:
                    handleContactsListenerCallback();
                    break;
                case MSG_GET_CAPABILITIES:{
                    EabRequest request = (EabRequest)msg.obj;
                    handleGetCapabilities(request.mId,
                                        request.mServiceId,
                                        request.mListener);
                    }break;
                case MSG_GET_AVAILABILITY:{
                    EabRequest request = (EabRequest)msg.obj;
                    handleGetAvailability(request.mId,
                                        request.mServiceId,
                                        request.mListener);
                    }break;
                case MSG_START_ADVERTISE:
                    Log.d(TAG, "MSG_START_ADVERTISE");
                    mListener.onCapabilityAndAvailabilityPublished(0);
                    break;
            }
        }

        private void handleContactsListenerCallback() {
            Collection<ContactTuple> values = mContactMap.values();

            ArrayList<SipProfile> list = new ArrayList<SipProfile>();
            ContactTuple lastContact = null;
            for (Map.Entry<String, ContactTuple> entry : mContactMap.entrySet()) {
                ContactTuple ct = entry.getValue();
                if (ct.isSupportService()) {
                    list.add(ct.getSipProfile());
                    lastContact = ct;
                }
            }
            if (list != null && lastContact != null) {
                SipProfile[] array = new SipProfile[list.size()];
                array = list.toArray(array);
                //SipProfile[] array = (SipProfile[])list.toArray();
                lastContact.getListener().onServiceCapabilityAndAvailabilityReceived(
                            0, lastContact.getService(), array, null);
                Log.d(TAG, "handleContactsListenerCallback, notify "
                            + array.length + " listener(s)");
            }
            ContactUpdateChecker.getInstance().resetTargetCount();
            ContactUpdateChecker.getInstance().resetCurrentCount();
        }

        private static ContactId createContactIdFromNumber(String number) {
            try {
                return ContactUtil.getInstance(mContext).formatContact(number);
            } catch (Exception e) {
                Log.e(TAG, Log.getStackTraceString(e));
                return new ContactId(number);
            }
        }

        private void handleGetCapabilities(
                    String[] id, String serviceId, Listener listener) {

            Log.d(TAG, "internalGetCapabilities, for");
            for (String number : id) {
                Log.d(TAG, number);
            }
            if (id == null || id.length == 0) {
                // request all contacts
            } else {
                if (mCapabilitiesApi != null && mCapabilitiesApiConnected) {
                    try {
                        // 0. Store contacts/service/listener
                        // 1. request contacts by id
                        for (String number : id) {
                            ContactTuple ct = new ContactTuple(listener, serviceId);
                            ContactUpdateChecker.getInstance().increaseTargetCount();
                            mContactMap.put(number, ct);
                            mCapabilitiesApi.requestContactCapabilities(
                                    createContactIdFromNumber(number));
                        }
                    } catch (Exception e) {
                        Log.e(TAG, Log.getStackTraceString(e));
                    }
                }
            }
            listener.onServiceCapabilityAndAvailabilityReceived(
                            0, serviceId, null, null);
        }

        private void handleGetAvailability(
                    String[] id, String serviceId, Listener listener) {
            Log.d(TAG, "internalGetAvailability, for");
            for (String number : id) {
                Log.d(TAG, number);
            }

            if (mCapabilitiesApi != null && mCapabilitiesApiConnected) {
                try {
                    for (String number : id) {
                        mCapabilitiesApi.requestContactAvailability(
                                createContactIdFromNumber(number));
                    }
                    listener.onServiceCapabilityAndAvailabilityReceived(
                            0, serviceId, null, null);
                } catch (Exception e) {
                    Log.e(TAG, Log.getStackTraceString(e));
                }
            }
        }
    }

    private static class ContactUpdateChecker {
        private static ContactUpdateChecker sInstance = new ContactUpdateChecker();
        private int mTargetUpdatedCounter;
        private int mCurrentUpdatedCounter;

        public static ContactUpdateChecker getInstance() {
            return sInstance;
        }

        public ContactUpdateChecker() {
            mTargetUpdatedCounter = 0;
            mCurrentUpdatedCounter = 0;
        }

        public void increaseTargetCount() {
            mTargetUpdatedCounter++;
        }

        public void resetTargetCount() {
            mTargetUpdatedCounter = 0;
        }

        public void increaseCurrentCount() {
            mCurrentUpdatedCounter++;
        }

        public void resetCurrentCount() {
            mCurrentUpdatedCounter = 0;
        }

        public boolean isUpdateComplete() {
            return mCurrentUpdatedCounter == mTargetUpdatedCounter;
        }

        public boolean isForbiddenUpdate() {
            return mCurrentUpdatedCounter >= mTargetUpdatedCounter;
        }
    }

    public class MyCapabilitiesListener extends CapabilitiesListener {
        @Override
        public void onCapabilitiesReceived(final String contact,
                Capabilities capabilities) {

            SipProfile profile = new SipProfile();
            profile.mdn = contact;

            Log.d(TAG, "onCapabilitiesReceived, contact:" + contact
                + ", capabilities = " + capabilities);
            ContactTuple ct = mContactMap.get(contact);
            if (ct == null ||
                ContactUpdateChecker.getInstance().isForbiddenUpdate()) {
                Log.d(TAG, "EABService has not completed all listener callback");
                return;
            }

            int queryCap = getCapabilitiesFromServiceId(ct.getService());
            boolean support = false;
            if ((queryCap & CAPABILITY_VOICE_OVER_LTE) > 0) {
                support |= capabilities.isIR94_VoiceCallSupported();
                profile.capabilities.putString(
                    SipProfile.CAPABILITY_HD_AUDIO,
                    SipProfile.CAPABILITY_STATE_FULL);
            }
            if ((queryCap & CAPABILITY_VIDEO_OVER_LTE) > 0) {
                support |= capabilities.isIR94_VideoCallSupported();
                profile.capabilities.putString(
                    SipProfile.CAPABILITY_VIDEO_CALL,
                    SipProfile.CAPABILITY_STATE_FULL);
            }

            ct.mIsSupportService = support;
            ct.mIsUpdate = true;
            ct.mProfile = profile;
            ContactUpdateChecker.getInstance().increaseCurrentCount();

            if (ContactUpdateChecker.getInstance().isUpdateComplete()) {
                mHandler.obtainMessage(MyHandler.MSG_START_CALLBACK).sendToTarget();
            }
        }

        @Override
        public void onErrorReceived(final String contact, int type,
                                       int status, String reason) {
        }
    }

    public class MyRcsServiceListener implements RcsServiceListener {
        @Override
        public void onServiceConnected() {
            try {
                Log.d(TAG, "onServiceConnected, add capability listener");
                mCapabilitiesApiConnected = true;
                EABService.this.mCapabilitiesApi
                    .addCapabilitiesListener(mMyCapabilitiesListener);
            } catch (Exception e) {
                Log.e(TAG, Log.getStackTraceString(e));
            }
        }

        @Override
        public void onServiceDisconnected(ReasonCode reasonCode) {
            try {
                Log.d(TAG, "onServiceDisconnected, set mCapabilitiesApi to null");
                if(EABService.this.mCapabilitiesApi != null) {
                    EABService.this.mCapabilitiesApi
                        .removeCapabilitiesListener(mMyCapabilitiesListener);
                    mCapabilitiesApiConnected = false;
                }
            } catch (Exception e) {
                Log.e(TAG, Log.getStackTraceString(e));
            }
        }
    }

    private int getCapabilitiesFromServiceId(String service) {
        int cap = 0;
        if (service.equals(
              "org.3gpp.urn:urn-7:3gpp-service.ims.icsi.mmtel")){
            cap |= CAPABILITY_VOICE_OVER_LTE;
            cap |= CAPABILITY_VIDEO_OVER_LTE;
        }
        return cap;
    }

    public EABService(Context context) {
        Log.d(TAG, "new EABService");
        mContext = context;
        mMyCapabilitiesListener = new MyCapabilitiesListener();
        myRcsServiceListener = new MyRcsServiceListener();
        mCapabilitiesApi = new CapabilityService(context, myRcsServiceListener);
        mCapabilitiesApi.connect();
    }

    public static boolean getCapabilities(
                String[] id, String serviceId, Listener listener) {
        EabRequest request = new EabRequest(
                                        id, serviceId, listener);
        mHandler.obtainMessage(MyHandler.MSG_GET_CAPABILITIES,
                                        request).sendToTarget();
        return true;
    }

    public static boolean advertiseCapabilitiesAndAvailability(
                Listener listener, String serviceId, String contact,
                Hashtable<String,Boolean> availabilityPolicy) {
        Log.d(TAG, "advertiseCapabilitiesAndAvailability, serviceId:" + serviceId
                                                        + ", contact:" + contact);
        mListener = listener;
        mHandler.obtainMessage(MyHandler.MSG_START_ADVERTISE).sendToTarget();
        return true;
    }

    public class Listener {
        public void onCapabilityAndAvailabilityPublished(int errorCode){}
        public void onServiceCapabilityAndAvailabilityReceived(
            int errorCode, String serviceId,
            SipProfile[] result, String[] presenceDocument){}
    }

    public static boolean getAvailability(String[] id,
                           String serviceId, Listener listener) {
        if (id == null || id.length == 0) {
            return false;
        }
        EabRequest request = new EabRequest(
                                        id, serviceId, listener);
        mHandler.obtainMessage(MyHandler.MSG_GET_AVAILABILITY,
                                        request).sendToTarget();
        return true;
    }

    public static void setFeatureTags(String serviceId, String[] featureTags) {
        Log.d(TAG, "setFeatureTags, serviceId:" + serviceId);
    }
}
