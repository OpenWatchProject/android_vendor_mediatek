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


package com.mediatek.internal.telephony.gsm;

import android.os.Build;
import android.content.Context;
import android.telephony.SmsCbCmasInfo;
import android.telephony.SmsCbEtwsInfo;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.gsm.SmsCbHeader;
import com.android.internal.telephony.gsm.SmsCbConstants;

import java.util.Arrays;

// MTK-START
// For loading special PWS emergency channel list
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import android.os.Environment;
import android.telephony.Rlog;
import android.util.Xml;

import com.android.internal.util.XmlUtils;

import android.os.Build;
import com.mediatek.internal.telephony.gsm.MtkSmsCbConstants;
import mediatek.telephony.MtkSmsCbCmasInfo;

// MTK-END


/**
 * Parses a 3GPP TS 23.041 cell broadcast message header. This class is public for use by
 * CellBroadcastReceiver test cases, but should not be used by applications.
 *
 * All relevant header information is now sent as a Parcelable
 * {@link android.telephony.SmsCbMessage} object in the "message" extra of the
 * {@link android.provider.Telephony.Sms.Intents#SMS_CB_RECEIVED_ACTION} or
 * {@link android.provider.Telephony.Sms.Intents#SMS_EMERGENCY_CB_RECEIVED_ACTION} intent.
 * The raw PDU is no longer sent to SMS CB applications.
 */
public class MtkSmsCbHeader extends SmsCbHeader {
    private static final String LOG_TAG = "MtkSmsCbHeader";
    private static final boolean ENG = "eng".equals(Build.TYPE);
    private boolean mIsEtwsPrimary = false;
    // For loading special PWS emergency channel list
    protected String mPlmn;

    public MtkSmsCbHeader(byte[] pdu,
            String plmn, boolean isEtwsPrimary) throws IllegalArgumentException {
        super();
        if (pdu == null || pdu.length < PDU_HEADER_LENGTH) {
            throw new IllegalArgumentException("Illegal PDU");
        }

        mPlmn = plmn;
        mIsEtwsPrimary = isEtwsPrimary;
        log("Create header!" + pdu.length);
        if (pdu.length <= PDU_LENGTH_GSM) {
            // can be ETWS or GSM format.
            // Per TS23.041 9.4.1.2 and 9.4.1.3.2, GSM and ETWS format both
            // contain serial number which contains GS, Message Code, and Update Number
            // per 9.4.1.2.1, and message identifier in same octets
            mGeographicalScope = (pdu[0] & 0xc0) >>> 6;
            mSerialNumber = ((pdu[0] & 0xff) << 8) | (pdu[1] & 0xff);
            mMessageIdentifier = ((pdu[2] & 0xff) << 8) | (pdu[3] & 0xff);
            if (isEtwsMessage() && pdu.length <= PDU_LENGTH_ETWS && mIsEtwsPrimary) {
                mFormat = FORMAT_ETWS_PRIMARY;
                mDataCodingScheme = -1;
                mPageIndex = -1;
                mNrOfPages = -1;
                boolean emergencyUserAlert = (pdu[4] & 0x1) != 0;
                boolean activatePopup = (pdu[5] & 0x80) != 0;
                int warningType = (pdu[4] & 0xfe) >>> 1;
                byte[] warningSecurityInfo;
                // copy the Warning-Security-Information, if present
                if (pdu.length > PDU_HEADER_LENGTH) {
                    warningSecurityInfo = Arrays.copyOfRange(pdu, 6, pdu.length);
                } else {
                    warningSecurityInfo = null;
                }
                mEtwsInfo = new SmsCbEtwsInfo(warningType, emergencyUserAlert, activatePopup,
                        true, warningSecurityInfo);
                if (ENG) {
                    log("Create primary ETWS Info!");
                }
                mCmasInfo = null;
                return;     // skip the ETWS/CMAS initialization code for regular notifications
            } else {
                // GSM pdus are no more than 88 bytes
                mFormat = FORMAT_GSM;
                mDataCodingScheme = pdu[4] & 0xff;

                // Check for invalid page parameter
                int pageIndex = (pdu[5] & 0xf0) >>> 4;
                int nrOfPages = pdu[5] & 0x0f;

                if (pageIndex == 0 || nrOfPages == 0 || pageIndex > nrOfPages) {
                    pageIndex = 1;
                    nrOfPages = 1;
                }

                mPageIndex = pageIndex;
                mNrOfPages = nrOfPages;
            }
        } else {
            // UMTS pdus are always at least 90 bytes since the payload includes
            // a number-of-pages octet and also one length octet per page
            mFormat = FORMAT_UMTS;

            int messageType = pdu[0];
            if (messageType != MESSAGE_TYPE_CBS_MESSAGE) {
                throw new IllegalArgumentException("Unsupported message type " + messageType);
            }

            mMessageIdentifier = ((pdu[1] & 0xff) << 8) | pdu[2] & 0xff;
            mGeographicalScope = (pdu[3] & 0xc0) >>> 6;
            mSerialNumber = ((pdu[3] & 0xff) << 8) | (pdu[4] & 0xff);
            mDataCodingScheme = pdu[5] & 0xff;

            // We will always consider a UMTS message as having one single page
            // since there's only one instance of the header, even though the
            // actual payload may contain several pages.
            mPageIndex = 1;
            mNrOfPages = 1;
        }

        if (isEtwsMessage()) {
            boolean emergencyUserAlert = isEtwsEmergencyUserAlert();
            boolean activatePopup = isEtwsPopupAlert();
            int warningType = getEtwsWarningType();
            mEtwsInfo = new SmsCbEtwsInfo(warningType, emergencyUserAlert, activatePopup,
                    false, null);
            if (ENG) {
                log("Create non-primary ETWS Info!");
            }
            mCmasInfo = null;
        } else if (isCmasMessage()) {
            int messageClass = getCmasMessageClass();
            int severity = getCmasSeverity();
            int urgency = getCmasUrgency();
            int certainty = getCmasCertainty();
            mEtwsInfo = null;
            mCmasInfo = new MtkSmsCbCmasInfo(messageClass, SmsCbCmasInfo.CMAS_CATEGORY_UNKNOWN,
                    SmsCbCmasInfo.CMAS_RESPONSE_TYPE_UNKNOWN, severity, urgency, certainty, 0L);
        } else {
            mEtwsInfo = null;
            mCmasInfo = null;
        }

        log("pdu length= " + pdu.length + ", " + this);
    }

    /**
     * Return whether this broadcast is a WHAM message .
     * @return true if this message is a WHAM message; false otherwise
     */
    public boolean isWHAMMessage() {
        boolean result = false;

        if (mMessageIdentifier == MtkSmsCbConstants.MESSAGE_ID_WHAM) {
            result = true;
        }

        return result;
    }

    /**
     * Return whether this broadcast is an emergency (PWS) message type.
     * @return true if this message is emergency type; false otherwise
     */
    // MTK-START
    // Modification for sub class
    public boolean isEmergencyMessage() {
    // MTK-END
        //return mMessageIdentifier >= SmsCbConstants.MESSAGE_ID_PWS_FIRST_IDENTIFIER
        //        && mMessageIdentifier <= SmsCbConstants.MESSAGE_ID_PWS_LAST_IDENTIFIER;
        boolean result = false;

        if (mMessageIdentifier >= SmsCbConstants.MESSAGE_ID_PWS_FIRST_IDENTIFIER &&
                mMessageIdentifier <= SmsCbConstants.MESSAGE_ID_PWS_LAST_IDENTIFIER) {
            result = true;
        } else if (checkNationalEmergencyChannels()){
            result = true;
        }

        return result;
    }

    /**
     * Return whether this message is a CMAS emergency message type.
     * @return true if this message is CMAS emergency type; false otherwise
     */
    protected boolean isCmasMessage() {
        boolean result = false;

        if (mMessageIdentifier >= SmsCbConstants.MESSAGE_ID_CMAS_FIRST_IDENTIFIER &&
                mMessageIdentifier <= SmsCbConstants.MESSAGE_ID_CMAS_LAST_IDENTIFIER) {
            result = true;
        } else if (checkNationalEmergencyChannels()){
            result = true;
        }

        return result;
    }

    /**
     * Returns the message class for a CMAS warning notification.
     * This method assumes that the message ID has already been checked for CMAS type.
     * @return the CMAS message class as defined in {@link SmsCbCmasInfo}
     */
    protected int getCmasMessageClass() {
        switch (mMessageIdentifier) {
            // AOSP can't show emerygency message in several country
            // In order to support several national requirements about PWS, we have to modify
            // here directly.
            case MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_911:
            case MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_919:
            case MtkSmsCbConstants.MESSAGE_ID_GSMA_ALLOCATED_CHANNEL_921:
                return SmsCbCmasInfo.CMAS_CLASS_PRESIDENTIAL_LEVEL_ALERT;
            case MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT:
            case MtkSmsCbConstants.MESSAGE_ID_CMAS_PUBLIC_SAFETY_ALERT_LANGUAGE:
                return MtkSmsCbCmasInfo.CMAS_CLASS_PUBLIC_SAFETY;
            case MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST:
            case MtkSmsCbConstants.MESSAGE_ID_CMAS_WEA_TEST_LANGUAGE:
                return MtkSmsCbCmasInfo.CMAS_CLASS_WEA_TEST;
            default:
                return super.getCmasMessageClass();
        }
    }

    @Override
    public String toString() {
        return "MtkSmsCbHeader{GS=" + mGeographicalScope + ", serialNumber=0x" +
                Integer.toHexString(mSerialNumber) +
                ", messageIdentifier=0x" + Integer.toHexString(mMessageIdentifier) +
                ", DCS=0x" + Integer.toHexString(mDataCodingScheme) +
                ", page " + mPageIndex + " of " + mNrOfPages + ", isEtwsPrimary=" + mIsEtwsPrimary +
                ", plmn " + mPlmn + '}';
    }


    // For loading special PWS emergency channel list
    private static HashMap<String,String> mSpecialChannelList = null;

    private static final String SPECIAL_PWS_CHANNEL_PATH ="/vendor/etc/special_pws_channel.xml";

    private static final Object mListLock = new Object();

    private static void loadSpecialChannelList() {
        synchronized(mListLock) {
            if (mSpecialChannelList == null) {
                log("load special_pws_channel.xml...");
                mSpecialChannelList = new HashMap<String,String>();

                FileReader dbReader;

                File dbFile = new File(SPECIAL_PWS_CHANNEL_PATH);

                try {
                    dbReader = new FileReader(dbFile);
                } catch (FileNotFoundException e) {
                    Rlog.w(LOG_TAG, "Can not open " + dbFile.getAbsolutePath());
                    return;
                }

                try {
                    XmlPullParser parser = Xml.newPullParser();
                    parser.setInput(dbReader);

                    XmlUtils.beginDocument(parser, "SpecialPwsChannel");

                    while (true) {
                        XmlUtils.nextElement(parser);

                        String name = parser.getName();
                        if (!"SpecialPwsChannel".equals(name)) {
                            break;
                        }

                        String mcc = parser.getAttributeValue(null, "mcc");
                        String channels = parser.getAttributeValue(null, "channels");

                        mSpecialChannelList.put(mcc, channels);
                    }
                    dbReader.close();
                    log("Special channels list size=" + mSpecialChannelList.size());
                } catch (XmlPullParserException e) {
                    loge("Exception in parser " + e);
                } catch (IOException e) {
                    loge("Exception in parser " + e);
                }
            } else {
                log("Special PWS channel list is already loaded");
            }
        }
    }

    private boolean checkNationalEmergencyChannels() {
        loadSpecialChannelList();

        if (mSpecialChannelList != null) {
            String mcc = (mPlmn != null) ? (mPlmn.length() >= 3 ? mPlmn.substring(0, 3) : "" ): "";
            String channels = mSpecialChannelList.get(mcc);
            log("checkNationalEmergencyChannels, mPlmn " + mPlmn + ",mcc "
                    + mcc + ", channels list " + channels
                    + ", header's channel " + mMessageIdentifier);
            if ((channels != null) && (channels.length() > 0)) {
                String values[] = channels.split(",");
                for (int i = 0; i < values.length; i++) {
                    if (values[i].equals(Integer.toString(mMessageIdentifier))) {
                        return true;
                    }
                }
            }
        } else {
            log("checkNationalEmergencyChannels, mSpecialChannelList is null!");
        }
        return false;
    }

    private static void log(String msg) {
        if (ENG) {
            Rlog.d(LOG_TAG, msg);
        }
    }

    private static void loge(String msg) {
        if (ENG) {
            Rlog.e(LOG_TAG, msg);
        }
    }
    // MTK-END
}
