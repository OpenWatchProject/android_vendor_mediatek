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

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.CellLocation;
import android.telephony.SmsCbLocation;
import android.telephony.SmsCbMessage;
import android.telephony.gsm.GsmCellLocation;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.gsm.GsmCellBroadcastHandler;
import com.android.internal.telephony.gsm.GsmSmsCbMessage;

import java.util.HashMap;
import java.util.Iterator;

// MTK-START
import android.telephony.SubscriptionManager;

import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.gsm.MtkGsmSmsCbMessage;
// ETWS features
import com.mediatek.internal.telephony.MtkEtwsNotification;
import com.mediatek.internal.telephony.MtkEtwsUtils;
import com.mediatek.internal.telephony.MtkRIL;
import android.content.IntentFilter;
import android.content.Intent;
import android.app.PendingIntent;
import android.os.SystemClock;
import android.app.AlarmManager;
import android.content.BroadcastReceiver;

import com.android.internal.telephony.WakeLockStateMachine;
// MTK-END

/**
 * Handler for 3GPP format Cell Broadcasts. Parent class can also handle CDMA Cell Broadcasts.
 */
public class MtkGsmCellBroadcastHandler extends GsmCellBroadcastHandler {
    private static final boolean VDBG = false;  // log CB PDU data

    protected static final int EVENT_NEW_ETWS_NOTIFICATION = 2000;

    public MtkGsmCellBroadcastHandler(Context context, Phone phone) {
        super("MtkGsmCellBroadcastHandler", context, phone, null);

        mDefaultState = new DefaultStateEx();
        mIdleState = new IdleStateEx();
        mWaitingState = new WaitingStateEx();

        addState(mDefaultState);
        addState(mIdleState, mDefaultState);
        addState(mWaitingState, mDefaultState);
        setInitialState(mIdleState);

        // MTK-START
        MtkRIL ci = (MtkRIL)phone.mCi;
        ci.setOnEtwsNotification(getHandler(), EVENT_NEW_ETWS_NOTIFICATION, null);
        // MTK-END
    }

    @Override
    protected void onQuitting() {
        // MTK-START
        MtkRIL ci = (MtkRIL)mPhone.mCi;
        ci.unSetOnEtwsNotification(getHandler());
        // MTK-END
        super.onQuitting();     // release wakelock
    }

    /**
     * This parent state throws an exception (for debug builds) or prints an error for unhandled
     * message types.
     */
    class DefaultStateEx extends WakeLockStateMachine.DefaultState {
        @Override
        public boolean processMessage(Message msg) {
            switch (msg.what) {
                default: {
                    return super.processMessage(msg);
                }
            }
        }
    }

    /**
     * Idle state delivers Cell Broadcasts to receivers. It acquires the wakelock, which is
     * released when the broadcast completes.
     */
    class IdleStateEx extends WakeLockStateMachine.IdleState {
        @Override
        public boolean processMessage(Message msg) {
            switch (msg.what) {
                // MTK-START
                case EVENT_NEW_ETWS_NOTIFICATION:
                    // transition to waiting state if we sent a broadcast
                    log("receive ETWS notification");
                    if (handleEtwsPrimaryNotification(msg)) {
                        transitionTo(mWaitingState);
                    }
                    return HANDLED;
                // MTK-END

                default:
                    return super.processMessage(msg);
            }
        }
    }

    /**
     * Waiting state waits for the result receiver to be called for the current cell broadcast.
     * In this state, any new cell broadcasts are deferred until we return to Idle state.
     */
    class WaitingStateEx extends WakeLockStateMachine.WaitingState {
        @Override
        public boolean processMessage(Message msg) {
            switch (msg.what) {
                // MTK-START
                case EVENT_NEW_ETWS_NOTIFICATION:
                // MTK-END
                    log("deferring message until return to idle");
                    deferMessage(msg);
                    return HANDLED;

                default:
                    return super.processMessage(msg);
            }
        }
    }

    /**
     * Handle 3GPP format SMS-CB message.
     * @param ar the AsyncResult containing the received PDUs
     */
    protected SmsCbMessage handleGsmBroadcastSms(AsyncResult ar) {
        try {
            byte[] receivedPdu = (byte[]) ar.result;

            if (VDBG) {
                int pduLength = receivedPdu.length;
                for (int i = 0; i < pduLength; i += 8) {
                    StringBuilder sb = new StringBuilder("SMS CB pdu data: ");
                    for (int j = i; j < i + 8 && j < pduLength; j++) {
                        int b = receivedPdu[j] & 0xff;
                        if (b < 0x10) {
                            sb.append('0');
                        }
                        sb.append(Integer.toHexString(b)).append(' ');
                    }
                    log(sb.toString());
                }
            }

            String plmn = TelephonyManager.from(mContext).getNetworkOperatorForPhone(
                    mPhone.getPhoneId());
            MtkSmsCbHeader header = new MtkSmsCbHeader(receivedPdu, plmn, false);
            int lac = -1;
            int cid = -1;
            CellLocation cl = mPhone.getCellLocation();
            // Check if cell location is GsmCellLocation.  This is required to support
            // dual-mode devices such as CDMA/LTE devices that require support for
            // both 3GPP and 3GPP2 format messages
            if (cl instanceof GsmCellLocation) {
                GsmCellLocation cellLocation = (GsmCellLocation)cl;
                lac = cellLocation.getLac();
                cid = cellLocation.getCid();
            }

            SmsCbLocation location;
            switch (header.getGeographicalScope()) {
                case SmsCbMessage.GEOGRAPHICAL_SCOPE_LA_WIDE:
                    location = new SmsCbLocation(plmn, lac, -1);
                    break;

                case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE:
                case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE_IMMEDIATE:
                    location = new SmsCbLocation(plmn, lac, cid);
                    break;

                case SmsCbMessage.GEOGRAPHICAL_SCOPE_PLMN_WIDE:
                default:
                    location = new SmsCbLocation(plmn);
                    break;
            }

            byte[][] pdus;
            int pageCount = header.getNumberOfPages();
            if (pageCount > 1) {
                // Multi-page message
                SmsCbConcatInfo concatInfo = new SmsCbConcatInfo(header, location);

                // Try to find other pages of the same message
                pdus = mSmsCbPageMap.get(concatInfo);

                if (pdus == null) {
                    // This is the first page of this message, make room for all
                    // pages and keep until complete
                    pdus = new byte[pageCount][];

                    mSmsCbPageMap.put(concatInfo, pdus);
                }

                // Page parameter is one-based
                pdus[header.getPageIndex() - 1] = receivedPdu;

                for (byte[] pdu : pdus) {
                    if (pdu == null) {
                        // Still missing pages, exit
                        return null;
                    }
                }

                // Message complete, remove and dispatch
                mSmsCbPageMap.remove(concatInfo);
            } else {
                // Single page message
                pdus = new byte[1][];
                pdus[0] = receivedPdu;
            }

            // Remove messages that are out of scope to prevent the map from
            // growing indefinitely, containing incomplete messages that were
            // never assembled
            Iterator<SmsCbConcatInfo> iter = mSmsCbPageMap.keySet().iterator();

            while (iter.hasNext()) {
                SmsCbConcatInfo info = iter.next();

                if (!info.matchesLocation(plmn, lac, cid)) {
                    iter.remove();
                }
            }

            boolean isGeoFencing = SystemProperties.get("ro.vendor.enable.geo.fencing").equals("1");
            if (isGeoFencing) {
                return MtkGsmSmsCbMessage.createSmsCbMessage(mContext, header, location, pdus);
            } else {
                return GsmSmsCbMessage.createSmsCbMessage(mContext, header, location, pdus);
            }
        } catch (RuntimeException e) {
            loge("Error in decoding SMS CB pdu", e);
            return null;
        }
    }

    // MTK-START, start ETWS alarm and open the necessary channels
    /**
     * Implemented by subclass to handle messages in {@link IdleState}.
     * It is used to handle the ETWS primary notification. The different
     * domain should handle by itself. Default will not handle this message.
     * @param message the message to process
     * @return true to transition to {@link WaitingState}; false to stay in {@link IdleState}
     */
    private boolean handleEtwsPrimaryNotification(Message message) {
        if (message.obj instanceof AsyncResult)
        {
            AsyncResult ar = (AsyncResult) message.obj;
            MtkEtwsNotification noti = (MtkEtwsNotification) ar.result;
            log(noti.toString());

            // create SmsCbMessage from MtkEtwsNotification, then
            // broadcast it to app
            SmsCbMessage etwsPrimary = handleEtwsPdu(noti.getEtwsPdu(), noti.plmnId);
            if (etwsPrimary != null) {
                log("ETWS Primary dispatch to App");
                handleBroadcastSms(etwsPrimary);
                return true;
            }
        }
        return false;
    }

    private SmsCbMessage handleEtwsPdu(byte[] pdu, String plmn) {
        if (pdu == null || pdu.length != MtkEtwsUtils.ETWS_PDU_LENGTH) {
            log("invalid ETWS PDU");
            return null;
        }
        // Set as ETWS primary message
        MtkSmsCbHeader header = new MtkSmsCbHeader(pdu, plmn, true);
        int lac = -1;
        int cid = -1;
        CellLocation cl = mPhone.getCellLocation();
        // Check if cell location is GsmCellLocation.  This is required to support
        // dual-mode devices such as CDMA/LTE devices that require support for
        // both 3GPP and 3GPP2 format messages
        if (cl instanceof GsmCellLocation) {
            GsmCellLocation cellLocation = (GsmCellLocation)cl;
            lac = cellLocation.getLac();
            cid = cellLocation.getCid();
        }

        SmsCbLocation location;
        switch (header.getGeographicalScope()) {
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_LA_WIDE:
                location = new SmsCbLocation(plmn, lac, -1);
                break;

            case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE:
            case SmsCbMessage.GEOGRAPHICAL_SCOPE_CELL_WIDE_IMMEDIATE:
                location = new SmsCbLocation(plmn, lac, cid);
                break;

            case SmsCbMessage.GEOGRAPHICAL_SCOPE_PLMN_WIDE:
            default:
                location = new SmsCbLocation(plmn);
                break;
        }

        byte[][] pdus = new byte[1][];
        pdus[0] = pdu;

        return GsmSmsCbMessage.createSmsCbMessage(mContext, header, location, pdus);
    }

    /**
     * Create a new CellBroadcastHandler.
     * @param context the context to use for dispatching Intents
     * @return the new handler
     */
    public static MtkGsmCellBroadcastHandler makeGsmCellBroadcastHandler(Context context,
            Phone phone) {
        MtkGsmCellBroadcastHandler handler = new MtkGsmCellBroadcastHandler(context, phone);
        handler.start();
        return handler;
    }
    // MTK-END
}
