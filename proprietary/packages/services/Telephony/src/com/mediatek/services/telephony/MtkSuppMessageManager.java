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
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

package com.mediatek.services.telephony;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import com.android.internal.telephony.Call;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.gsm.SuppServiceNotification;
import com.android.services.telephony.ImsConference;
import com.android.services.telephony.TelephonyConnection;
import com.android.services.telephony.TelephonyConnectionService;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.gsm.MtkSuppCrssNotification;
import com.mediatek.internal.telephony.gsm.MtkSuppServiceNotification;


import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.telecom.Conference;
import android.telecom.Connection;
import android.telephony.PhoneNumberUtils;
import com.android.services.telephony.Log;

public class MtkSuppMessageManager {
    private static final int EVENT_SUPP_SERVICE_FAILED            = 250;
    private static final int EVENT_CRSS_SUPP_SERVICE_NOTIFICATION = EVENT_SUPP_SERVICE_FAILED + 1;
    private static final int EVENT_SUPP_SERVICE_NOTIFICATION      = EVENT_SUPP_SERVICE_FAILED + 2;

    private TelephonyConnectionService mConnectionService;
    private List<SuppMessageHandler> mSuppMessageHandlerList
                                = new ArrayList<SuppMessageHandler>();

    private HashMap<Integer, ArrayList<SuppServiceNotification>> mCachedSsnsMap =
            new HashMap<Integer, ArrayList<SuppServiceNotification>>();
    private HashMap<Integer, ArrayList<MtkSuppCrssNotification>> mCachedCrssnsMap =
            new HashMap<Integer, ArrayList<MtkSuppCrssNotification>>();


    public MtkSuppMessageManager(TelephonyConnectionService connectionService) {
        mConnectionService = connectionService;
    }

    /**
     * register handlers to Phone when TelephonyConnectionService raise.
     */
    public void registerSuppMessageForPhones() {
        mSuppMessageHandlerList.clear();
        Phone[] phones = PhoneFactory.getPhones();
        for (Phone phone : phones) {
            if (phone != null) {
                Log.d(this, "registerSuppMessageForPhones()...for service" + mConnectionService
                        + " for phone " + phone.getPhoneId());
                SuppMessageHandler handler = new SuppMessageHandler(phone);
                mSuppMessageHandlerList.add(handler);
                phone.registerForSuppServiceFailed(handler, EVENT_SUPP_SERVICE_FAILED, null);
                if (phone instanceof MtkGsmCdmaPhone) {
                    ((MtkGsmCdmaPhone) phone).registerForCrssSuppServiceNotification(handler,
                            EVENT_CRSS_SUPP_SERVICE_NOTIFICATION, null);
                }
                phone.registerForSuppServiceNotification(handler, EVENT_SUPP_SERVICE_NOTIFICATION,
                        null);
            }
        }
    }

    /**
     * unregister handlers from Phone when TelephonyConnectionService down.
     */
    public void unregisterSuppMessageForPhones() {
        for (SuppMessageHandler handler : mSuppMessageHandlerList) {
            Phone phone = handler.getPhone();
            if (phone != null) {
                Log.d(this, "unregisterSuppMessageForPhones()..."
                        + " for phone " + phone.getPhoneId());
                phone.unregisterForSuppServiceFailed(handler);
                phone.unregisterForSuppServiceNotification(handler);
                if (phone instanceof MtkGsmCdmaPhone) {
                    ((MtkGsmCdmaPhone) phone).unregisterForCrssSuppServiceNotification(handler);
                }
            }
        }
        mSuppMessageHandlerList.clear();
        mCachedSsnsMap.clear();
        mCachedCrssnsMap.clear();
    }

    public void registerSuppMessageForPhone(Phone phone) {
        for (SuppMessageHandler handler : mSuppMessageHandlerList) {
            if (phone == handler.getPhone()) {
                return;
            }
        }
        SuppMessageHandler handler = new SuppMessageHandler(phone);
        mSuppMessageHandlerList.add(handler);
        phone.registerForSuppServiceFailed(handler, EVENT_SUPP_SERVICE_FAILED, null);
        if (phone instanceof MtkGsmCdmaPhone) {
            ((MtkGsmCdmaPhone) phone).registerForCrssSuppServiceNotification(handler,
                    EVENT_CRSS_SUPP_SERVICE_NOTIFICATION, null);
        }
        phone.registerForSuppServiceNotification(handler, EVENT_SUPP_SERVICE_NOTIFICATION,
                null);
    }

    public void unregisterSuppMessageForPhone(Phone phone) {
        for (SuppMessageHandler handler : mSuppMessageHandlerList) {
            Phone p = handler.getPhone();
            if (phone == p) {
                p.unregisterForSuppServiceFailed(handler);
                p.unregisterForSuppServiceNotification(handler);
                if (p instanceof MtkGsmCdmaPhone) {
                    ((MtkGsmCdmaPhone) p).unregisterForCrssSuppServiceNotification(handler);
                }
                mSuppMessageHandlerList.remove(handler);
                return;
            }
        }
    }

    /**
     * Force Supplementary Message update once TelephonyConnection is created.
     * @param c The connection to update supplementary messages.
     * @param p The phone to update supplementary messages.
     */
    public void forceSuppMessageUpdate(TelephonyConnection c, Phone p) {
        Log.d(this, "forceSuppMessageUpdate for " + c + ", " + p + " phone " + p.getPhoneId());
        ArrayList<SuppServiceNotification> ssnList = mCachedSsnsMap.get(p.getPhoneId());
        if (ssnList != null) {
            Log.d(this, "forceSuppMessageUpdate()... for SuppServiceNotification for " + c
                    + " for phone " + p.getPhoneId());
            for (SuppServiceNotification ssn : ssnList) {
                onSuppServiceNotification(ssn, p, c);
            }
            mCachedSsnsMap.remove(p.getPhoneId());
         }

        ArrayList<MtkSuppCrssNotification> crssnList = mCachedCrssnsMap.get(p.getPhoneId());
        if (crssnList != null) {
            Log.d(this, "forceSuppMessageUpdate()... for SuppCrssNotification for " + c
                    + " for phone " + p.getPhoneId());
            for (MtkSuppCrssNotification crssn : crssnList) {
                onCrssSuppServiceNotification(crssn, p, c);
            }
            mCachedCrssnsMap.remove(p.getPhoneId());
         }
    }

    /**
     * Handler to handle SS messages, one phone one handler.
     */
    private class SuppMessageHandler extends Handler {
        private Phone mPhone;   // This is used for some operation when we receive message.

        public SuppMessageHandler(Phone phone) {
            mPhone = phone;
        }

        private Phone getPhone() {
            return mPhone;
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(this, "handleMessage()... for service " + mConnectionService
                    + " for phone " + mPhone.getPhoneId());
            AsyncResult ar = (AsyncResult) msg.obj;
            if (ar == null || ar.result == null || mConnectionService == null) {
                Log.w(this, "handleMessage()...Wrong condition: ar / mConnectionService = "
                                + ar + " / " + mConnectionService);
                return;
            }
            switch (msg.what) {
                case EVENT_SUPP_SERVICE_FAILED:
                    if (!(ar.result instanceof Phone.SuppService)) {
                        Log.w(this, "handleMessage()...Wrong data for Phone.SuppService");
                        return;
                    }
                    // M: fix CR:ALPS02781094,suppService is conference and switch,telecom
                    // will show toast,telephony will show dialog,so revert to goolge default,
                    // only show dialog,don't trigger onSuppServiceFailed in telecom
                    // when conference and switch.
                    if (ar.result == Phone.SuppService.CONFERENCE) {
                        Log.d(this, "Service is conference don't trigger"
                            + " onSuppServiceFailed");
                        return;
                    }
                    Phone.SuppService service = (Phone.SuppService) ar.result;
                    onSuppServiceFailed(service, mPhone);
                    break;
                case EVENT_SUPP_SERVICE_NOTIFICATION: {
                    if (!(ar.result instanceof SuppServiceNotification)) {
                        Log.w(this, "handleMessage()..Wrong data for SuppServiceNotification");
                        return;
                    }
                    SuppServiceNotification noti = (SuppServiceNotification) ar.result;
                    onSuppServiceNotification(noti, mPhone, null);
                    break;
                }
                case EVENT_CRSS_SUPP_SERVICE_NOTIFICATION: {
                    if (!(ar.result instanceof MtkSuppCrssNotification)) {
                        Log.w(this, "handleMessage()...Wrong data for SuppCrssNotification");
                        return;
                    }
                    MtkSuppCrssNotification noti = (MtkSuppCrssNotification) ar.result;
                    onCrssSuppServiceNotification(noti, mPhone, null);
                    break;
                }
                default:
                    break;
            }
        }
    };

    /**
     * handle "EVENT_SUPP_SERVICE_FAILED" case.
     * TODO: maybe we should leave toast-part just here, like mr1.
     * @param service SuppService of Phone
     * @param phone Target Phone
     */
    private void onSuppServiceFailed(Phone.SuppService service, Phone phone) {
        Log.d(this, "onSuppServiceFailed()... " + service);

        int actionCode = (int) getSuppServiceActionCode(service);

        com.android.internal.telephony.Connection originalConnection =
                getProperOriginalConnection(phone);

        Log.d(this, "getProperOriginalConnection originalConnection = " + originalConnection);

        TelephonyConnection connection = (TelephonyConnection) findConnection(originalConnection);
        if (connection != null) {
            connection.notifyActionFailed(actionCode);
        } else {
            Log.d(this, "onSuppServiceFailed()...connection is null");
        }
    }

    /**
     * handle "EVENT_SUPP_SERVICE_NOTIFICATION" case.
     * TODO: maybe we should leave toast-part just here, like mr1.
     * @param noti SuppServiceNotification
     * @param phone Target Phone
     */
    private void onSuppServiceNotification(SuppServiceNotification noti, Phone phone,
            Connection forceConn) {
        Log.v(this, "onSuppServiceNotification()... " + noti);
        com.android.internal.telephony.Connection originalConnection = null;
        TelephonyConnection connection = null;
        boolean normalCase = false;
        if (noti.notificationType == 0) {       // for MO cases
            if (noti.code == MtkSuppServiceNotification.MO_CODE_CALL_IS_EMERGENCY) {
                Connection targetConnection = getConnectionWithState(Call.State.DIALING);
                if (targetConnection == null) {
                    targetConnection = getConnectionWithState(Call.State.ALERTING);
                }
                if (targetConnection instanceof TelephonyConnection) {
                    if (forceConn == null || forceConn == targetConnection) {
                        ((TelephonyConnection) targetConnection).notifyEcc();
                    }
                } else if (targetConnection == null && forceConn == null) {
                    Log.d(this, "onSuppServiceNotification()...MO connection is null");
                    addSsnList(noti, phone);
                }
            } else {
                normalCase = true;
            }
        } else if (noti.notificationType == 1) {    // for MT cases
            normalCase = true;
        }
        if (normalCase) {
            // the upper layer just show a toast, so here we just bypass it to Telecomm,
            // and do not check which connection it belongs to (get anyone).
            originalConnection = getProperOriginalConnection(phone);
            connection = (TelephonyConnection) findConnection(originalConnection);
            if (connection != null) {
                if (forceConn == null || forceConn == connection) {
                    connection.notifySSNotificationToast(noti.notificationType,
                            noti.type, noti.code, noti.number, noti.index);
                }
            } else {
                if (forceConn == null) {
                    Log.d(this, "onSuppServiceNotification()...MT connection is null");
                    addSsnList(noti, phone);
                }
            }
        }
    }

    private void addSsnList(SuppServiceNotification noti, Phone phone) {
        Log.d(this, "addSsnList for " + phone + " phone " + phone.getPhoneId());
        ArrayList<SuppServiceNotification> ssnList = mCachedSsnsMap.get(phone.getPhoneId());
        if (ssnList == null) {
            ssnList = new ArrayList<SuppServiceNotification>();
        } else {
            mCachedSsnsMap.remove(phone.getPhoneId());
        }
        ssnList.add(noti);
        mCachedSsnsMap.put(phone.getPhoneId(), ssnList);
    }

    /**
     * handle "EVENT_CRSS_SUPP_SERVICE_NOTIFICATION" case.
     * TODO: maybe we can merge notifyNumberUpdate() and notifyIncomingInfoUpdate() to one.
     * just as IConnectionServiceAdapter.setAddress()
     * @param noti MtkSuppCrssNotification
     * @param phone Target Phone
     */
    private void  onCrssSuppServiceNotification(MtkSuppCrssNotification noti, Phone phone,
            Connection forceConn) {
        Log.v(this, "onCrssSuppServiceNotification... " + noti);
        com.android.internal.telephony.Connection originalConnection = null;
        TelephonyConnection connection = null;
        switch (noti.code) {
            case MtkSuppCrssNotification.CRSS_CALL_WAITING:
                break;
            case MtkSuppCrssNotification.CRSS_CALLED_LINE_ID_PREST:
                originalConnection = getOriginalConnectionWithState(
                        phone, Call.State.ACTIVE);
                connection = (TelephonyConnection) findConnection(originalConnection);
                if (connection != null) {
                    if (forceConn == null || forceConn == connection) {
                        connection.notifyNumberUpdate(noti.number);
                    }
                } else {
                    if (forceConn == null) {
                        Log.d(this, "onCrssSuppServiceNotification()...connection is null");
                        addCrssnList(noti, phone);
                    }
                }
                break;
            case MtkSuppCrssNotification.CRSS_CALLING_LINE_ID_PREST:
                originalConnection = getOriginalConnectionWithState(
                        phone, Call.State.INCOMING);
                connection = (TelephonyConnection) findConnection(originalConnection);
                if (connection != null) {
                    if (forceConn == null || forceConn == connection) {
                        connection.notifyIncomingInfoUpdate(
                                noti.type, noti.alphaid, noti.cli_validity);
                    }
                } else {
                    if (forceConn == null) {
                        Log.d(this, "onCrssSuppServiceNotification()...connection is null");
                        addCrssnList(noti, phone);
                    }
                }
                break;
            case MtkSuppCrssNotification.CRSS_CONNECTED_LINE_ID_PREST:
                originalConnection = getOriginalConnectionWithState(
                        phone, Call.State.DIALING);
                connection = (TelephonyConnection) findConnection(originalConnection);
                if (connection == null) {
                    originalConnection = getOriginalConnectionWithState(
                            phone, Call.State.ALERTING);
                    connection = (TelephonyConnection) findConnection(originalConnection);
                    if (connection == null) {
                        originalConnection = getOriginalConnectionWithState(
                                phone, Call.State.ACTIVE);
                        connection = (TelephonyConnection) findConnection(originalConnection);
                    }
                }
                if (connection != null) {
                    if (forceConn == null || forceConn == connection) {
                        connection.notifyNumberUpdate(PhoneNumberUtils.stringFromStringAndTOA(
                                noti.number, noti.type));
                    }
                } else {
                    if (forceConn == null) {
                        Log.d(this, "onCrssSuppServiceNotification()...connection is null");
                        addCrssnList(noti, phone);
                    }
                }
                break;
            default:
                break;
        }
    }

    private void addCrssnList(MtkSuppCrssNotification noti, Phone phone) {
        Log.d(this, "addCrssnList for " + phone + " phone " + phone.getPhoneId());
        ArrayList<MtkSuppCrssNotification> crssnList = mCachedCrssnsMap.get(phone.getPhoneId());
        if (crssnList == null) {
            crssnList = new ArrayList<MtkSuppCrssNotification>();
        } else {
            mCachedCrssnsMap.remove(phone.getPhoneId());
        }
        crssnList.add(noti);
        mCachedCrssnsMap.put(phone.getPhoneId(), crssnList);
    }

    /**
     * For some SS message, we can tell which connection it belongs to,
     * use this function to find it.
     * Eg, "SuppServiceNotification.MO_CODE_CALL_IS_WAITING"
     * is for connection with state of DIALING or ALERTING.
     * @param phone
     * @param state
     * @return
     */
    private com.android.internal.telephony.Connection getOriginalConnectionWithState(Phone phone,
            Call.State state) {
        com.android.internal.telephony.Connection originalConnection = null;
        Call call = null;
        if (state == Call.State.INCOMING) {
            call = phone.getRingingCall();
        } else if (state == Call.State.HOLDING) {
            call = phone.getBackgroundCall();
        } else if (state == Call.State.DIALING || state == Call.State.ALERTING
                || state == Call.State.ACTIVE) {
            call = phone.getForegroundCall();
        }
        if (call != null && call.getState() == state) {
            originalConnection = call.getLatestConnection();
        }
        return originalConnection;
    }

    /**
     * For some SS message, we can not judge which connection it belongs to,
     * so we scan all calls, and find the most-likely connection.
     * ringing > foreground > background
     * @param phone
     * @param state
     * @return
     */
    private com.android.internal.telephony.Connection getProperOriginalConnection(Phone phone) {
        if (phone == null) {
            Log.d(this, "getProperOriginalConnection: phone is null");
            return null;
        }

        com.android.internal.telephony.Connection originalConnection = null;
        // phone.getRingingCall() can get GsmCall or ImsPhoneCall.
        originalConnection = phone.getRingingCall().getLatestConnection();
        Log.d(this, "getRingingCall originalConnection = " + originalConnection);
        // FIXME: Need to clarify all scenario of SuppMsg.

        if (originalConnection == null) {
            List<com.android.internal.telephony.Connection> l =
                    phone.getForegroundCall().getConnections();
            for (int i = 0; i < l.size(); i++) {
                com.android.internal.telephony.Connection conn = l.get(i);
                Log.d(this, "getForegroundCall iterate conn = " + conn);
                // FIXME: Need to clarify all scenario of SuppMsg.
                // Here we assume no SuppMsg is received for disconnected/disconnecting calls.
                if (conn != null && conn.isAlive()) {
                    originalConnection = conn;
                    break;
                }
            }
        }

        if (originalConnection == null) {
            List<com.android.internal.telephony.Connection> l =
                    phone.getBackgroundCall().getConnections();
            for (int i = 0; i < l.size(); i++) {
                com.android.internal.telephony.Connection conn = l.get(i);
                Log.d(this, "getBackgroundCall iterate conn = " + conn);
                // FIXME: Need to clarify all scenario of SuppMsg.
                // Here we assume no SuppMsg is received for disconnected/disconnecting calls.
                if (conn != null && conn.isAlive()) {
                    originalConnection = conn;
                    break;
                }
            }
        }

        // for foreground and background, need to use ImsPhone to get ImsPhoneCall.
        Phone imsPhone = phone.getImsPhone();
        if (imsPhone != null) {
            if (originalConnection == null) {
                List<com.android.internal.telephony.Connection> l =
                        imsPhone.getForegroundCall().getConnections();
                for (int i = 0; i < l.size(); i++) {
                    com.android.internal.telephony.Connection conn;
                    try {
                        conn = l.get(i);
                    } catch (IndexOutOfBoundsException e) {
                        Log.e(this, e, "getForegroundCall (IMS) Exception in getting element: ");
                        break;
                    }

                    Log.d(this, "getForegroundCall (IMS) iterate conn = " + conn);
                    // FIXME: Need to clarify all scenario of SuppMsg.
                    // Here we assume no SuppMsg is received for disconnected/disconnecting calls.
                    if (conn != null && conn.isAlive()) {
                        originalConnection = conn;
                        break;
                    }
                }
            }

            if (originalConnection == null) {
                List<com.android.internal.telephony.Connection> l =
                        imsPhone.getBackgroundCall().getConnections();
                for (int i = 0; i < l.size(); i++) {
                    com.android.internal.telephony.Connection conn;
                    try {
                        conn = l.get(i);
                    } catch (IndexOutOfBoundsException e) {
                        Log.e(this, e, "getBackgroundCall (IMS) Exception in getting element: ");
                        break;
                    }

                    Log.d(this, "getBackgroundCall (IMS)  iterate conn = " + conn);
                    // FIXME: Need to clarify all scenario of SuppMsg.
                    // Here we assume no SuppMsg is received for disconnected/disconnecting calls.
                    if (conn != null && conn.isAlive()) {
                        originalConnection = conn;
                        break;
                    }
                }
            }
        }
        return originalConnection;
    }

    /**
     * get all connections exist in ConnectionService
     * @return
     */
    private ArrayList<TelephonyConnection> getAllTelephonyConnectionsFromService() {
        ArrayList<TelephonyConnection> connectionList = new ArrayList<TelephonyConnection>();
        if (mConnectionService != null) {
            for (Connection connection : mConnectionService.getAllConnections()) {
                if (connection instanceof TelephonyConnection) {
                    connectionList.add((TelephonyConnection) connection);
                }
            }
        }
        return connectionList;
    }

    /**
     * get all conference exist in ConnectionService
     * @return
     */
    private ArrayList<ImsConference> getAllImsConferenceFromService() {
        ArrayList<ImsConference> conferenceList = new ArrayList<ImsConference>();
        if (mConnectionService != null) {
            for (Conference conference : mConnectionService.getAllConferences()) {
                if (conference instanceof ImsConference) {
                    conferenceList.add((ImsConference) conference);
                }
            }
        }
        return conferenceList;
    }

    /**
     * find Connection in ConnectionService, which
     * corresponding to certain original Connection.
     * @param originalConnection
     * @return
     */
    private Connection findConnection(com.android.internal.telephony.Connection
                                                         originalConnection) {
        Connection connection = null;
        ArrayList<TelephonyConnection> telephonyConnections
                           = getAllTelephonyConnectionsFromService();
        ArrayList<ImsConference> imsConferences = getAllImsConferenceFromService();

        Log.d(this, "findConnection originalConnection = " + originalConnection);
        Log.d(this, "findConnection telephonyConnections.size = " + telephonyConnections.size()
                +", imsConferences.size = "+ imsConferences.size());

        if (originalConnection != null && telephonyConnections != null) {
            for (TelephonyConnection telephonyConnection : telephonyConnections) {
                Log.d(this, "findConnection telephonyConnection = " + telephonyConnection);
                Log.d(this, "findConnection telephonyConnection.getOriginalConnection = "
                        + telephonyConnection.getOriginalConnection());
                if (originalConnection == telephonyConnection.getOriginalConnection()) {
                    connection = telephonyConnection;
                }
            }
        }

        // When call become member of peer IMS conference, can't get any connection from
        // TelephonyConnectionService. We should also check the connection in the ImsConference.
        if (originalConnection != null && imsConferences != null) {
            for (ImsConference imsConference : imsConferences) {
                TelephonyConnection telephonyConnection = imsConference.getHostConnection();
                Log.d(this, "findConnection telephonyConnection = " + telephonyConnection);
                Log.d(this, "findConnection telephonyConnection.getOriginalConnection = "
                        + telephonyConnection.getOriginalConnection());
                if (originalConnection == telephonyConnection.getOriginalConnection()) {
                    connection = telephonyConnection;
                }
            }
        }
        return connection;
    }

    /**
     * Get TelephonyConnection with special state.
     * @param state
     * @return
     */
    private Connection getConnectionWithState(Call.State state) {
        Connection connection = null;
        ArrayList<TelephonyConnection> telephonyConnections =
            getAllTelephonyConnectionsFromService();
        if (telephonyConnections != null) {
            for (TelephonyConnection telephonyConnection : telephonyConnections) {
                com.android.internal.telephony.Connection originalConnection =
                    telephonyConnection.getOriginalConnection();
                if (originalConnection != null) {
                    Call call = originalConnection.getCall();
                    if (call != null && call.getState() == state) {
                        connection = telephonyConnection;
                        break;
                    }
                }
            }
        }
        return connection;
    }

    private int getSuppServiceActionCode(Phone.SuppService service) {
        int actionCode = 0;
        switch (service) {
            case SWITCH:
                actionCode = 1;
                break;
            case SEPARATE:
                actionCode = 2;
                break;
            case TRANSFER:
                actionCode = 3;
                break;
            case REJECT:
                actionCode = 4;
                break;
            case HANGUP:
                actionCode = 5;
                break;
            case UNKNOWN:
            default:
                break;
        }
        return actionCode;
    }

    // Based on current ConnectionService-based design,
    // maybe should consider below items later.
    // TODO: 1/ maybe we can change path from "SuppMessageManager -> Connection
    //                       -> ConnectionService.mConnectionLisnter -> Adapter"
    //          to "SuppMessageManager -> ConnectionService -> Adapter"
    //      2/ do not scan connections in ConnectionService,
    //          instead, manage a list, like GsmConferenceController.
    //      3/ divide those messages into two parts: toast only + logic-related.
    //          for toast only, two ways to handle:
    //              1) show toast here (no interface needed);
    //              2) show toast in Telecomm or InCallUI; (current way)
    //                  modify interface with Telecomm (remove parameter of callId),
    //                  then here we can just bypass those messages directly,
    //                  no need try to find corresponding connection.
    //    for logic-related, maybe we can check interfaces with Telecomm again.
    // Eg,merge notifyNumberUpdate() and notifyIncomingInfoUpdate() to be setAddress().
}
