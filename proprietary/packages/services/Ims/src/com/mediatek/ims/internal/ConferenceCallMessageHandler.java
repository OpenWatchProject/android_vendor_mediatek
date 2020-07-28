/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */


package com.mediatek.ims.internal;

import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.telephony.Rlog;

import com.mediatek.ims.ImsServiceCallTracker;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import java.util.ArrayList;
import java.util.List;

/**
* This class is used to parse IMS conference call user information.
*/
public class ConferenceCallMessageHandler extends DefaultHandler {
    private static final String TAG = "ConferenceCallMessageHandler";

    // Sensitive log task
    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    private static final boolean TELDBG = (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private List<User> mUsers;
    private User mUser;
    private int mMaxUserCount;
    private String mTag;
    private String mTempVal;
    private int mCallId = -1;
    private int mIndex = 0;
    private int mUserCount = -1;
    private String mHostInfo;
    private boolean mParsingHostInfo = false;
    private int mVersion = -1;
    private int mCEPState = CEP_STATE_UNKNOWN;
    /*
    * Define in RFC 4575
    *
    */
    public static final String STATUS_PENDING = "pending";
    public static final String STATUS_DIALING_OUT = "dialing-out";
    public static final String STATUS_DIALING_IN = "dialing-in";
    public static final String STATUS_ALERTING = "alerting";
    public static final String STATUS_ON_HOLD = "on-hold";
    public static final String STATUS_CONNECTED = "connected";
    public static final String STATUS_DISCONNECTING = "disconnecting";
    public static final String STATUS_DISCONNECTED = "disconnected";
    public static final String STATUS_MUTED_VIA_FOCUS = "muted-via-focus";
    public static final String STATUS_CONNECT_FAIL = "connect-fail";

    public static final int CEP_STATE_UNKNOWN = 0;
    public static final int CEP_STATE_FULL = 1;
    public static final int CEP_STATE_PARTIAL = 2;

    /**
    * This class is used to store IMS conference call user information.
    *
    */
    public class User {
        private String mEndPoint;
        private String mEntity;
        private String mDisplayText;
        private String mStatus = STATUS_DISCONNECTED;     //Default is "disconnected"
        private String mSipTelUri;
        private int    mIndex;
        private int    mConnectionIndex = -1;

        /**
        * To set endpoint entity value, it is usually phone number in tel uri or sip uri format.
        *
        * @param entity entity value
        */
        void setEndPoint(String endPoint) {
            mEndPoint = endPoint;
        }

        /**
        * To set user entity value, it is usually phone number in tel uri or sip uri format.
        *
        * @param entity entity value
        */
        void setEntity(String entity) {
            mEntity = entity;
        }

        /**
        * To set sip uri and/or tel uri value.
        *
        * @param uri sip uri and/or tel uri value
        */
        void setSipTelUri(String uri) {
            mSipTelUri = uri;
        }

        /**
        * To set the display text value.
        *
        * @param displayText displayText value
        */
        void setDisplayText(String displayText) {
            mDisplayText = displayText;
        }

        /**
        * To set status of conf. call participant
        * Refer to {@link #STATUS_CONNECTED}, {@link #STATUS_DISCONNECTED} or
        * {@link #STATUS_ON_HOLD}.
        *
        * @param displayText displayText value
        */
        void setStatus(String status) {
            mStatus = status;
        }

        /**
        * To set sequential index for each conf. call participant.
        * It will be used during SRVCC for call id reassignment.
        *
        * @param index index value
        */
        void setIndex(int index) {
            mIndex = index;
        }

        /**
        * To set connection index.
        *
        * @param index connection index value
        */
        public void setConnectionIndex(int index) {
            mConnectionIndex = index;
        }

        /**
        * To retrieve endpoint entity value, it is usually phone number
        * in tel uri or sip uri format.
        *
        * @return entity value
        */
        public String getEndPoint() {
            return mEndPoint;
        }

        /**
        * To retrieve user entity value, it is usually phone number in tel uri or
        * sip uri format.
        *
        * @return entity value
        */
        public String getEntity() {
            return mEntity;
        }

        /**
        * To retrieve sip uri and/or tel uri value.
        *
        * @return sip uri and/or tel uri value
        */
        public String getSipTelUri() {
            return mSipTelUri;
        }

        /**
        * To retrieve display text.
        *
        * @return display text value
        */
        public String getDisplayText() {
            return mDisplayText;
        }

        /**
        * To retrieve status of conf. call participant
        * Refer to {@link #STATUS_CONNECTED}, {@link #STATUS_DISCONNECTED} or
        * {@link #STATUS_ON_HOLD}
        *
        * @return participant status
        */
        public String getStatus() {
            return mStatus;
        }

        /**
        * To retrieve sequential index.
        *
        * @return sequential index
        */
        public int getIndex() {
            return mIndex;
        }

        /**
        * To retrieve connection index.
        *
        * @return connection index
        */
        public int getConnectionIndex() {
            return mConnectionIndex;
        }
    } /* End of User class */

    /**
    * To retrieve all conf. call participants.
    *
    * @return all conf. call participants
    */
    public List<User> getUsers() {
        return mUsers;
    }

    /**
    * To set the maximum count of participants.
    *
    * @param maxUserCount the maximum count of participants
    */
    private void setMaxUserCount(String maxUserCount) {
        mMaxUserCount = Integer.parseInt(maxUserCount);
    }

    /**
    * To retrive the maximum count of participants.
    *
    * @return the maximum count of participants
    */
    public int getMaxUserCount() {
        return mMaxUserCount;
    }

    /**
    * To set call id.
    *
    * @param callId The call id
    */
    public void setCallId(int callId) {
        mCallId = callId;
    }

    /**
    * To retrieve call id.
    *
    * @return call id
    */
    public int getCallId() {
        return mCallId;
    }

    /**
    * To start parse xml document.
    * @throws SAXException if something wrong
    */
    @Override
    public void startDocument() throws SAXException {
        mUsers = new ArrayList<User>();
    }

    /**
    * Notification for receiving character.
    *
    * @param ch character array
    * @param start the start index
    * @param length the length of character
    * @throws SAXException if something wrong
    */
    @Override
    public void characters(char[] ch, int start, int length) throws SAXException {
        if (mTag == null) {
            Rlog.d(TAG, "Parse val failed: tag is null");
            return;
        }
        String val = new String(ch, start, length);
        telLog("Current tag: " + mTag + " val: " + sensitiveEncode(val));
        // get normal element
        if (mTag.equalsIgnoreCase("maximum-user-count")) {
            setMaxUserCount(val);
        } else if (mTag.equalsIgnoreCase("user-count")) {
            mUserCount = Integer.parseInt(val);
        } else if (mParsingHostInfo && mTag.equalsIgnoreCase("uri")) {
            mHostInfo = val;
        }
        // get user related element value
        if (mUser == null) {
            Rlog.d(TAG, "Parse val failed: user is null");
            return;
        }
        if (mTag.equalsIgnoreCase("display-text")) {
            mUser.setDisplayText(val);
        } else if (mTag.equalsIgnoreCase("status")) {
            mUser.setStatus(val);
        }
    }

    /**
    * Notification for start element.
    *
    * @param uri URI value
    * @param localName local name
    * @param qName tag name
    * @param attributes attribute values
    * @throws SAXException if something wrong
    */
    @Override
    public void startElement(String uri, String localName, String qName,
            Attributes attributes) throws SAXException {
        if (qName.equalsIgnoreCase("user")) {
            mIndex++;
            mUser = new User();
            mUser.setIndex(mIndex);
            mUser.setEntity(attributes.getValue("", "entity"));
            telLog("user - entity: " + sensitiveEncode(mUser.getEntity()));
        } else if (qName.equalsIgnoreCase("endpoint")) {
            mUser.setEndPoint(attributes.getValue("", "entity"));
            telLog("endpoint - entity: " + sensitiveEncode(mUser.getEndPoint()));
        } else if (qName.equalsIgnoreCase("endpoint")) {
            mUser.setEndPoint(attributes.getValue("", "entity"));
        } else if (qName.equalsIgnoreCase("host-info")) {
            mParsingHostInfo = true;
        } else if (qName.equalsIgnoreCase("conference-info")) {
            String ver = attributes.getValue("version");
            try {
                mVersion = Integer.valueOf(ver).intValue();
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
            telLog("version:" + mVersion);

            String state = attributes.getValue("state");
            mCEPState = updateCEPState(state);
            telLog("get CEP state: " + state);
        }
        mTag = qName;
    }

    /**
    * Notification for end element.
    *
    * @param uri URI value
    * @param localName local name
    * @param qName tag name
    * @throws SAXException if something wrong
    */
    @Override
    public void endElement(String uri, String localName, String qName)
            throws SAXException {
        if (qName.equalsIgnoreCase("user") && mUsers != null) {
            mUsers.add(mUser);
            mUser = null;
        } else if (qName.equalsIgnoreCase("host-info")) {
            mParsingHostInfo = false;
        }
        mTag = null;
    }

    /**
    * To retrive the count of participants.
    *
    * @return the count of participants
    */
    public int getUserCount() {
        return mUserCount;
    }

    /**
    * To retrive the host information.
    *
    * @return the host information
    */
    public String getHostInfo() {
        return mHostInfo;
    }

    /**
     * To get the conference version.
     *
     * @return the conference version
     */
    public int getVersion() {
        return mVersion;
    }

    /**
    * To retrive the CEP state.
    *
    * @return the CEP state
    */
    public int getCEPState() {
        return mCEPState;
    }

    private int updateCEPState(String val) {
        if (val == null) {
            return CEP_STATE_UNKNOWN;
        }
        if (val.equalsIgnoreCase("full")) {
            return CEP_STATE_FULL;
        } else if (val.equalsIgnoreCase("partial")) {
            return CEP_STATE_PARTIAL;
        }

        return CEP_STATE_UNKNOWN;
    }

    private void telLog(String msg) {
        if(!TELDBG) return;
        Rlog.d(TAG, msg);
    }

    private String sensitiveEncode(String msg) {
        return ImsServiceCallTracker.sensitiveEncode(msg);
    }
}
