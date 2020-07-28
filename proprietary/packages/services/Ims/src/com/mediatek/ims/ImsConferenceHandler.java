/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ims;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.telephony.ims.ImsConferenceState;
import android.telephony.PhoneNumberUtils;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.support.v4.content.LocalBroadcastManager;

import com.mediatek.ims.common.SubscriptionManagerHelper;
import com.mediatek.ims.internal.ImsXuiManager;
import com.mediatek.ims.internal.CallControlDispatcher;
import com.mediatek.ims.internal.ConferenceCallMessageHandler;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.lang.Math;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;

/**
 * ImsConferenceHandler, singleton object to handle CEP and local participants
 */
public class ImsConferenceHandler extends DefaultConferenceHandler {

    private static final String LOG_TAG = "ImsConferenceHandler";
    private static final boolean DBG = true;
    private static final boolean VDBG = false; // STOPSHIP if true

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private static final String USER_ENTITY = "user-entity";
    private static final String CONF_HOST = "host";

    private static DefaultConferenceHandler mConfHdler;

    private static Listener mListener;

    private Context mContext;

    // Keep the conference participants
    private LinkedHashMap mConfParticipants = new LinkedHashMap<String, Bundle>();
    private List<Bundle> mUnknowParticipants = new ArrayList<Bundle>();

    // Local participant addr - CEP addr maps
    private ArrayList<String> mLocalParticipants = new ArrayList<String>();
    private String mAddingParticipant;
    private String mRemovingParticipant;
    private String mLatestRemovedParticipant;

    private int mConfCallId = -1;
    private int mCepVersion = -1;
    private int mPhoneId = -1;
    private int mConfState = CONFERENCE_STATE_CLOSED;
    private boolean mIsFirstCep = true;
    private String mHostAddr = null;
    private String mCachedConferenceData = null;

    private boolean mIsCepNotified = false;
    private boolean mRestoreParticipantsAddr = true;
    private boolean mRemoveParticipantsByUserEntity = false;
    private boolean mHaveUpdateConferenceWithMember = false;
    private boolean mSupportConferenceManagement = true;

    private static final int CONFERENCE_STATE_CLOSED = 0;
    private static final int CONFERENCE_STATE_ACTIVE = 1;

    private static final int EVENT_TRY_UPDATE_WITH_LOCAL_CACHE = 0;
    private static final int EVENT_CLOSE_CONFERENCE = 1;
    private static final int EVENT_HANDLE_CACHED_CONFERENCE_DATA = 2;
    // Wait CEP for 5 secs
    private static final int CEP_TIMEOUT = 5000;
    private static final String ANONYMOUS_URI = "sip:anonymous@anonymous.invalid";
    private LinkedHashMap mConfParticipantsAddr = new LinkedHashMap<String, String>();
    private LinkedHashMap mFirstMergeParticipants = new LinkedHashMap<String, String>();

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Rlog.d(LOG_TAG, "handleMessage: " + msg.what);
            switch(msg.what) {
                case EVENT_TRY_UPDATE_WITH_LOCAL_CACHE:
                    if (mIsCepNotified) {
                        Rlog.d(LOG_TAG, "CEP is notified, no need to update with local cache");
                        break;
                    }
                    updateConferenceStateWithLocalCache();
                    break;
                case EVENT_CLOSE_CONFERENCE:
                    closeConferenceInternal(msg.arg1);
                    break;
                case EVENT_HANDLE_CACHED_CONFERENCE_DATA:
                    if (mCachedConferenceData != null) {
                        handleImsConfCallMessage(mCachedConferenceData.length(), mCachedConferenceData);
                        mCachedConferenceData = null;
                    }
                    break;
                default:
                    break;
            }
        }
    };

    public static DefaultConferenceHandler getInstance() {
        if (mConfHdler == null) {
            if (ImsCommonUtil.supportMdAutoSetupIms()) {
                mConfHdler = new DefaultConferenceHandler();
            } else {
                mConfHdler = new ImsConferenceHandler();
            }
        }
        return mConfHdler;
    }

    private ImsConferenceHandler() {
        Rlog.d(LOG_TAG, "ImsConferenceHandler()");
        /* Register for receiving conference call xml message */
    }

    public void startConference(Context ctx, Listener listener, String callId, int phoneId) {
        if (mContext != null) {
            Rlog.d(LOG_TAG, "startConference() failed, a conference is ongoing");
            return;
        }
        Rlog.d(LOG_TAG, "startConference()");
        mListener = listener;
        mContext = ctx;
        final IntentFilter filter = new IntentFilter();
        filter.addAction(ImsConstants.ACTION_IMS_CONFERENCE_CALL_INDICATION);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(mBroadcastReceiver, filter);
        mConfCallId = Integer.parseInt(callId);
        mPhoneId = phoneId;
        mIsFirstCep = true;
        mConfState = CONFERENCE_STATE_ACTIVE;
        mRemoveParticipantsByUserEntity =
                OperatorUtils.isMatched(OperatorUtils.OPID.OP132_Peru, mPhoneId);
        mSupportConferenceManagement =
                OperatorUtils.isMatched(OperatorUtils.OPID.OP151, mPhoneId);
    }

    public void closeConference(String callId) {
        Rlog.d(LOG_TAG, "closeConference() " + callId);
        if (callId == null || mConfCallId != Integer.parseInt(callId)) {
            return;
        }
        mHandler.sendMessage(
                mHandler.obtainMessage(EVENT_CLOSE_CONFERENCE, Integer.parseInt(callId), 0));
    }

    private void closeConferenceInternal(int callId) {
        Rlog.d(LOG_TAG, "closeConferenceInternal()");
        mConfState = CONFERENCE_STATE_CLOSED;
        mListener = null;
        if (mContext != null) {
            LocalBroadcastManager.getInstance(mContext).unregisterReceiver(mBroadcastReceiver);
            mContext = null;
        }
        // clean the member variable
        mLocalParticipants.clear();
        mAddingParticipant = null;
        mRemovingParticipant = null;
        mConfCallId = -1;
        mCepVersion = -1;
        mPhoneId = -1;
        mHostAddr = null;
        mConfParticipants.clear();
        mConfParticipantsAddr.clear();
        mFirstMergeParticipants.clear();
        mUnknowParticipants.clear();
        mIsCepNotified = false;
        mHandler.removeMessages(EVENT_TRY_UPDATE_WITH_LOCAL_CACHE);
        mLatestRemovedParticipant = null;
        mHaveUpdateConferenceWithMember = false;
        mCachedConferenceData = null;
    }

    public boolean isConferenceActive() {
        return mConfState == CONFERENCE_STATE_ACTIVE;
    }

    private String normalizeNumberFromCLIR(String number) {
        return number.replace("*31#","").replace("#31#","");
    }

    public void firstMerge(String callId_1, String callId_2, String num_1, String num_2) {
        mLocalParticipants.clear();
        mFirstMergeParticipants.clear();
        mFirstMergeParticipants.put(callId_1, normalizeNumberFromCLIR(num_1));
        mFirstMergeParticipants.put(callId_2, normalizeNumberFromCLIR(num_2));
    }

    public void addFirstMergeParticipant(String callId) {
        String num = (String) mFirstMergeParticipants.get(callId);
        if (num != null) {
            Rlog.d(LOG_TAG, "addFirstMergeParticipant() callId: " + callId
                    + ", num: " + sensitiveEncode(num));
            mLocalParticipants.add(num);
        }
    }

    // For One-key conference used
    public void addLocalCache(String[] participants) {
        if (participants == null) {
            return;
        }
        mLocalParticipants.clear();
        for (String participant : participants) {
            mLocalParticipants.add(normalizeNumberFromCLIR(participant));
        }
    }

    public void tryAddParticipant(String addr) {
        mAddingParticipant = normalizeNumberFromCLIR(addr);
    }

    public void tryRemoveParticipant(String addr) {
        mRemovingParticipant = normalizeNumberFromCLIR(addr);
    }

    public void modifyParticipantComplete() {
        boolean isFirstMerge = (mAddingParticipant == null && mRemovingParticipant == null);
        if (mAddingParticipant != null &&
            (!mLocalParticipants.contains(mAddingParticipant) || mAddingParticipant.isEmpty())) {
            mLocalParticipants.add(mAddingParticipant);
        }
        if (mRemovingParticipant != null) {
            mLocalParticipants.remove(mRemovingParticipant);
            mLatestRemovedParticipant = mRemovingParticipant;
        }
        mAddingParticipant = null;
        mRemovingParticipant = null;
        for (String addr : mLocalParticipants) {
            Rlog.d(LOG_TAG, "modifyParticipantComplete: " + sensitiveEncode(addr));
        }
        if (mSupportConferenceManagement) {
            mHandler.sendEmptyMessageDelayed(EVENT_TRY_UPDATE_WITH_LOCAL_CACHE, CEP_TIMEOUT);
        }

        if (mCachedConferenceData != null) {
            mHandler.sendEmptyMessage(EVENT_HANDLE_CACHED_CONFERENCE_DATA);
        }

        if (mIsCepNotified == true && isFirstMerge == true) {
            Rlog.d(LOG_TAG, "CEP is notify before merge complete, notify again");
            notifyConfStateUpdate();
        }
    }

    public void modifyParticipantFailed() {
        mAddingParticipant = null;
        mRemovingParticipant = null;
        mLatestRemovedParticipant = null;
        for (String addr : mLocalParticipants) {
            Rlog.d(LOG_TAG, "modifyParticipantFailed: " + sensitiveEncode(addr));
        }
        if (mCachedConferenceData != null) {
            mHandler.sendEmptyMessage(EVENT_HANDLE_CACHED_CONFERENCE_DATA);
        }
    }

    public String getConfParticipantUri(String addr, boolean isRetry) {
        if (mRestoreParticipantsAddr && (mRemoveParticipantsByUserEntity != isRetry)) {
            String confParticipantUri = (String) mConfParticipantsAddr.get(addr);
            if (confParticipantUri != null) {
                Rlog.d(LOG_TAG, "removeParticipants confParticipantUri: "
                    + sensitiveEncode(confParticipantUri) +
                    " addr: " + sensitiveEncode(addr));
                addr = confParticipantUri;
            }
        }
        Bundle confInfo = (Bundle)mConfParticipants.get(addr);
        if (confInfo == null) {
            if (addr == null || addr.isEmpty()) {
                return ANONYMOUS_URI;
            }
            return addr;
        }
        String participantUri = confInfo.getString(USER_ENTITY);
        if (participantUri == null || !participantUri.startsWith("sip:")) {
            participantUri = addr;
        }

        Rlog.d(LOG_TAG, "removeParticipants uri: " + sensitiveEncode(participantUri) +
                " addr: " + sensitiveEncode(addr));

        return participantUri;
    }

    private void updateConferenceStateWithLocalCache() {
        Rlog.d(LOG_TAG, "updateConferenceStateWithLocalCache()");

        if (mLocalParticipants.size() == 0 && shouldAutoTerminateConf()) {
            if (mListener != null) {
                mListener.onAutoTerminate();
            }
            Rlog.d(LOG_TAG, "no participants");
            return;
        }

        ImsConferenceState confState = new ImsConferenceState();

        for (String addr : mLocalParticipants) {
            Bundle userInfo = createFakeInfo(addr);
            confState.mParticipants.put(addr, userInfo);
            Rlog.d(LOG_TAG, "submit participants:  uri: " + sensitiveEncode(addr));
        }

        if (mListener != null) {
            mListener.onParticipantsUpdate(confState);
        }
    }

    private Bundle createFakeInfo(String addr) {
        Bundle userInfo = new Bundle();
        userInfo.putString(ImsConferenceState.USER, addr);
        userInfo.putString(ImsConferenceState.DISPLAY_TEXT, addr);
        userInfo.putString(ImsConferenceState.ENDPOINT, addr);
        userInfo.putString(ImsConferenceState.STATUS, ImsConferenceState.STATUS_CONNECTED);
        return userInfo;
    }

    private void notifyConfStateUpdate() {
        Rlog.d(LOG_TAG, "notifyConfStateUpdate()");

        ImsConferenceState confState = new ImsConferenceState();

        Iterator<Entry<String, Bundle>> iterator = mConfParticipants.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, Bundle> entry = iterator.next();
            confState.mParticipants.put(entry.getKey(), entry.getValue());
            Rlog.d(LOG_TAG, "submit participants: " + sensitiveEncode(entry.getKey()));
        }

        int key = 0;
        for (Bundle userInfo: mUnknowParticipants) {
            confState.mParticipants.put(Integer.toString(key), userInfo);
            Rlog.d(LOG_TAG, "submit unknow participants: " +
                    sensitiveEncode(Integer.toString(key)));
            ++key;
        }

        if (mListener != null) {
            mListener.onParticipantsUpdate(confState);
        }
    }

    private final BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Rlog.d(LOG_TAG, "received broadcast " + action);
            /* Handle IMS conference call xml message */
            if (ImsConstants.ACTION_IMS_CONFERENCE_CALL_INDICATION.equals(action)) {
                String data = intent.getStringExtra(ImsConstants.EXTRA_MESSAGE_CONTENT);
                int callId = intent.getIntExtra(ImsConstants.EXTRA_CALL_ID, 0);
                if (callId != 255 && (data != null) && (!data.equals(""))) {
                    mIsCepNotified = true;
                    if (mAddingParticipant != null || mRemovingParticipant != null) {
                        mCachedConferenceData = data;
                    } else {
                        handleImsConfCallMessage(data.length(), data);
                    }
                }
            } else {
                Rlog.e(LOG_TAG, "can't handle conference message since no call ID. Abnormal Case");
            }
        }
    };

    private ConferenceCallMessageHandler parseXmlPackage(int len, String data) {
        try {
            // Read conference data and parse it
            InputStream inStream =
                new ByteArrayInputStream(data.getBytes(StandardCharsets.UTF_8));
            SAXParserFactory factory = SAXParserFactory.newInstance();
            SAXParser saxParse = factory.newSAXParser();
            ConferenceCallMessageHandler xmlData = new ConferenceCallMessageHandler();
            if (xmlData == null) {
                return null;
            }
            saxParse.parse(inStream, xmlData);
            return xmlData;
        } catch (Exception ex) {
            Rlog.d(LOG_TAG, "Parsing exception: " + ex.toString());
            updateConferenceStateWithLocalCache();
            return null;
        }
    }

    private Bundle packUserInfo(ConferenceCallMessageHandler.User user) {
        String entity = user.getEntity();
        String userAddr = getUserNameFromSipTelUriString(entity);
        Bundle userInfo = new Bundle();
        userInfo.putString(ImsConferenceState.USER, userAddr);
        userInfo.putString(ImsConferenceState.DISPLAY_TEXT, user.getDisplayText());
        userInfo.putString(ImsConferenceState.ENDPOINT, user.getEndPoint());
        userInfo.putString(ImsConferenceState.STATUS, user.getStatus());
        userInfo.putString(USER_ENTITY, entity);
        return userInfo;
    }

    private void fullUpdateParticipants(List<ConferenceCallMessageHandler.User> users) {
        Rlog.d(LOG_TAG, "reset all users as participants");
        mUnknowParticipants.clear();
        mConfParticipants.clear();

        for (ConferenceCallMessageHandler.User user : users) {
            String entity = user.getEntity();
            String userAddr = getUserNameFromSipTelUriString(entity);
            Bundle userInfo = packUserInfo(user);
            Rlog.d(LOG_TAG, "handle user: " +  sensitiveEncode(entity) +
                    " addr: " + sensitiveEncode(userAddr));

            if (userAddr == null || userAddr.trim().length() == 0) {
                mUnknowParticipants.add(userInfo);
                Rlog.d(LOG_TAG, "add unknow participants");
            } else {
                mConfParticipants.put(userAddr, userInfo);
            }
        }
    }

    private void partialUpdateParticipants(List<ConferenceCallMessageHandler.User> users) {
        Rlog.d(LOG_TAG, "partial update participants");
        for (ConferenceCallMessageHandler.User user : users) {
            String entity = user.getEntity();
            String userAddr = getUserNameFromSipTelUriString(entity);
            if (mRestoreParticipantsAddr) {
                userAddr = getPairedAddressFromCache(userAddr);
            }
            Bundle userInfo = packUserInfo(user);
            Rlog.d(LOG_TAG, "handle user: " +  sensitiveEncode(entity) +
                    " addr: " + sensitiveEncode(userAddr));

            String status = user.getStatus();
            // update participants
            if(userAddr == null || userAddr.trim().length() == 0) {
                if (status.equals(ConferenceCallMessageHandler.STATUS_CONNECTED)) {
                    mUnknowParticipants.add(userInfo);
                    Rlog.d(LOG_TAG, "add unknow participants");
                } else if (status.equals(ConferenceCallMessageHandler.STATUS_DISCONNECTED)){
                    // remove last unknow participants
                    if (mUnknowParticipants.size() > 0) {
                        mUnknowParticipants.remove(mUnknowParticipants.size() - 1);
                        Rlog.d(LOG_TAG, "remove unknow participants");
                    }
                }
            } else {
                if (!status.equals(ConferenceCallMessageHandler.STATUS_DIALING_OUT)) {
                    mConfParticipants.put(userAddr, userInfo);
                }
            }
        }
    }

    private boolean isEmptyConference() {
        int userCount = mUnknowParticipants.size();

        Iterator<Entry<String, Bundle>> iterator = mConfParticipants.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, Bundle> entry = iterator.next();
            String userHandle = entry.getKey();
            Bundle confInfo = entry.getValue();
            String status = confInfo.getString(ImsConferenceState.STATUS);
            if (!status.equals(ConferenceCallMessageHandler.STATUS_DISCONNECTED)) {
                if (isSelfAddress(userHandle)) continue;
                ++userCount;
            }
        }

        if (userCount == 0) {
            return true;
        }
        if (!mHaveUpdateConferenceWithMember) {
            Rlog.d(LOG_TAG, "Set mHaveUpdateConferenceWithMember = true");
            mHaveUpdateConferenceWithMember = true;
        }
        return false;
    }

    /**
    * To handle IMS conference call message
    *
    * @param len    The length of data
    * @param data   Conference call message
    */
    private void handleImsConfCallMessage(int len, String data) {
        if (mConfCallId == -1) {
            Rlog.e(LOG_TAG, "ImsConference is closed");
            return;
        }

        if ((data == null) || (data.equals(""))) {
            Rlog.e(LOG_TAG, "Failed to handleImsConfCallMessage due to data is empty");
            return;
        }

        Rlog.d(LOG_TAG, "handleVoLteConfCallMessage, data length = " + data.length());

        ConferenceCallMessageHandler xmlData = parseXmlPackage(len, data);
        if (xmlData == null) {
            Rlog.e(LOG_TAG, "can't create xmlData object, update conf state with local cache");
            updateConferenceStateWithLocalCache();
            return;
        }

        // get host address from the optional xml element <host-info>
        if (mHostAddr == null) {
            mHostAddr = getUserNameFromSipTelUriString(xmlData.getHostInfo());
        }

        // get CEP state
        int cepState = xmlData.getCEPState();
        boolean isPartialCEP = cepState == ConferenceCallMessageHandler.CEP_STATE_PARTIAL;
        Rlog.d(LOG_TAG, "isPartialCEP: " + isPartialCEP);

        int version = xmlData.getVersion();
        // only refer to the cep version for full cep, particial cep will not notify in order.
        if (!isPartialCEP) {
            if (mCepVersion > version && mCepVersion != -1) {
                Rlog.e(LOG_TAG, "version is less than local version" +
                        mCepVersion + "," + version);
                return;
            }
            mCepVersion = version;
        }

        // get user data from xml and fill them into ImsConferenceState data structure.
        List<ConferenceCallMessageHandler.User> users = xmlData.getUsers();

        // get optional xml element: user count
        int userCount = xmlData.getUserCount();

        // no optional user count element,
        // remove the participants who is not included in the xml.
        switch (cepState) {
            case ConferenceCallMessageHandler.CEP_STATE_FULL:
                fullUpdateParticipants(users);
                break;
            case ConferenceCallMessageHandler.CEP_STATE_PARTIAL:
                partialUpdateParticipants(users);
                break;
            default:
                if (userCount == -1 || userCount == users.size()) {
                    fullUpdateParticipants(users);
                } else {
                    partialUpdateParticipants(users);
                    break;
                }
        }

        // Terminate the empty conference for specific operator. If it is first cep, never
        // auto termiate cause that might be an empty one, such as one key conference.
        if (isEmptyConference() && shouldAutoTerminateConf() && !mIsFirstCep
                && mHaveUpdateConferenceWithMember) {
            Rlog.d(LOG_TAG, "no participants, terminate the conference");
            if (mListener != null) {
                mListener.onAutoTerminate();
            }
        }

        if (mRestoreParticipantsAddr) {
            restoreParticipantsAddressByLocalCache();
        }

        notifyConfStateUpdate();
        updateLocalCache();
        mIsFirstCep = false;
    }

    private String getPairedAddressFromCache(String addr) {
        for (String cache : mLocalParticipants) {
            if (PhoneNumberUtils.compareLoosely(addr, cache)) {
                Rlog.d(LOG_TAG, "getPairedAddressFromCache: " + sensitiveEncode(cache));
                mConfParticipantsAddr.put(cache, addr);
                return cache;
            }
        }
        // Due to the latest removed participant will be removed from local cached,
        // But it will be notified in the latest xml, so get the paired number individually.
        if (mLatestRemovedParticipant != null &&
                PhoneNumberUtils.compareLoosely(addr, mLatestRemovedParticipant)) {
            Rlog.d(LOG_TAG, "getPairedAddressFromLatestRemoved: "
                    + sensitiveEncode(mLatestRemovedParticipant));
            return mLatestRemovedParticipant;
        }
        return addr;
    }

    private void updateLocalCache() {
        Iterator<Entry<String, Bundle>> iterator = mConfParticipants.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, Bundle> entry = iterator.next();
            Bundle confInfo = entry.getValue();
            String status = confInfo.getString(ImsConferenceState.STATUS);
            String addr = confInfo.getString(ImsConferenceState.USER);
            if (status.equals(ConferenceCallMessageHandler.STATUS_DISCONNECTED)) {
                mLocalParticipants.remove(addr);
            }
        }
    }

    private void restoreParticipantsAddressByLocalCache() {
        ArrayList<String> restoreCandidate = new ArrayList<String>(mLocalParticipants);
        LinkedHashMap restoreList = new LinkedHashMap<String, Bundle>();

        // to avoid concurrent access
        LinkedHashMap participants = new LinkedHashMap<String, Bundle>(mConfParticipants);

        // start restore, figure out the special user entity which can not be restored
        Iterator<Entry<String, Bundle>> iterator = participants.entrySet().iterator();
        while (iterator.hasNext()) {
            Entry<String, Bundle> entry = iterator.next();
            String userHandle = entry.getKey();
            Bundle confInfo = entry.getValue();
            String restoreAddr = getPairedAddressFromCache(userHandle);
            if (isSelfAddress(userHandle) == false
                    && restoreCandidate.remove(restoreAddr) == false) {
                // Not self and match failed, keep addr and wait for restore.
                restoreList.put(userHandle, confInfo);
                Rlog.d(LOG_TAG, "wait for restore: " + sensitiveEncode(restoreAddr));
            } else {
                confInfo.putString(ImsConferenceState.USER, restoreAddr);
                // update mConfParticipants
                mConfParticipants.put(userHandle, confInfo);
                Rlog.d(LOG_TAG, "restore participant: "
                        + sensitiveEncode(userHandle) + " to: " + sensitiveEncode(restoreAddr));
            }
        }

        // use the "not paired" local address to restored the special user entity
        Iterator<Entry<String, Bundle>> resIterator = restoreList.entrySet().iterator();
        ArrayList<String> restoreUnknowCandidates = new ArrayList<String>(restoreCandidate);
        int restoreIndex = 0;
        while (resIterator.hasNext()) {
            if (restoreCandidate.size() <= restoreIndex) {
                Rlog.d(LOG_TAG, "No candidate to restore, size: " + restoreCandidate.size()
                        + ", index: " + restoreIndex);
                break;
            }
            Entry<String, Bundle> entry = resIterator.next();
            String userHandle = entry.getKey();
            Bundle confInfo = entry.getValue();
            String restoreAddr = restoreCandidate.get(restoreIndex);
            // remove the used candidate
            if (restoreUnknowCandidates.size() > 0) {
                restoreUnknowCandidates.remove(0);
            }
            String status = confInfo.getString(ImsConferenceState.STATUS);
            if (status.equals(ConferenceCallMessageHandler.STATUS_DISCONNECTED)) {
                // do not restore the disconnected user, the disconnected user does not contain
                // in the cached
                continue;
            }
            mConfParticipantsAddr.put(restoreAddr, userHandle);
            confInfo.putString(ImsConferenceState.USER, restoreAddr);
            mConfParticipants.put(userHandle, confInfo);
            Rlog.d(LOG_TAG, "restore participant: "
                        + sensitiveEncode(userHandle) + " to: " + sensitiveEncode(restoreAddr));
            ++restoreIndex;
        }

        // Restore the unknown participants
        restoreUnknowParticipants(restoreUnknowCandidates);
    }

    private void restoreUnknowParticipants(ArrayList<String> restoreUnknowCandidates) {
        List<Bundle> restoredUnknowParticipants = new ArrayList<Bundle>(mUnknowParticipants);
        int restoreIndex = 0;
        for (Bundle userInfo: mUnknowParticipants) {
            if (restoreUnknowCandidates.size() <= restoreIndex) {
                restoredUnknowParticipants.add(userInfo);
                continue;
            }
            String restoreAddr = restoreUnknowCandidates.get(restoreIndex);
            userInfo.putString(ImsConferenceState.USER, restoreAddr);
            mConfParticipants.put(restoreAddr, userInfo);
            // remove the unknow participants in index 0 (current unknow participant)
            if (restoredUnknowParticipants.size() > 0) {
                restoredUnknowParticipants.remove(0);
            }
            Rlog.d(LOG_TAG,
                    "restore unknow participants(" + restoreIndex + ") to: "
                    + sensitiveEncode(restoreAddr));
            ++restoreIndex;
        }

        mUnknowParticipants = restoredUnknowParticipants;
    }

    // Customize for specific operator and location.
    private boolean shouldAutoTerminateConf() {
        boolean shouldTerminate = true;
        Rlog.d(LOG_TAG, "shouldTerminate:" + shouldTerminate);
        return shouldTerminate;
    }

    private String getUserNameFromSipTelUriString(String uriString) {
        if (uriString == null) {
            return null;
        }

        Uri uri = Uri.parse(uriString);

        // Gets the address part, i.e. everything between 'sip:' and the fragment separator '#'.
        // ex: '+8618407404132@10.185.184.137:5087;transport=UDP'
        // or '1234;phone-context=munich.example.com;isub=@1134'
        String address = uri.getSchemeSpecificPart();
        if (address == null) {
            return null;
        }

        // Gets user name, i.e. everything before '@'.
        // ex: '+8618407404132' or '1234;phone-context=munich.example.com;isub='
        String userName = PhoneNumberUtils.getUsernameFromUriNumber(address);
        if (userName == null) {
            return null;
        }

        // Gets pure user name part, i.e. everything before ';' or ','.
        // ex: '+8618407404132' or '1234'
        int pIndex = userName.indexOf(';');    // WAIT
        int wIndex = userName.indexOf(',');    // PAUSE

        if (pIndex >= 0 && wIndex >= 0) {
            return userName.substring(0, Math.min(pIndex, wIndex));
        } else if (pIndex >= 0) {
            return userName.substring(0, pIndex);
        } else if (wIndex >= 0) {
            return userName.substring(0, wIndex);
        } else {
            return userName;
        }
    }

    private String sensitiveEncode(String msg) {
        return ImsServiceCallTracker.sensitiveEncode(msg);
    }

    private boolean isSelfAddress(String addr) {
        if (PhoneNumberUtils.compareLoosely(mHostAddr, addr)) {
            Rlog.d(LOG_TAG, "isSelfAddress(): true, meet host info in xml");
            return true;
        }
        return ImsServiceCallTracker.getInstance(mPhoneId).isSelfAddress(addr);
    }
}
