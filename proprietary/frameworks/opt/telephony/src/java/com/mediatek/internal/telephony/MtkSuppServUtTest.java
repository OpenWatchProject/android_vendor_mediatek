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

package com.mediatek.internal.telephony;


import android.content.Intent;
import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;
import android.telephony.TelephonyManager;
import android.telephony.ServiceState;

import com.android.ims.ImsException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.Phone;
import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

public class MtkSuppServUtTest {

    static final String LOG_TAG = "MtkSuppServUtTest";
    static final String ACTION_SUPPLEMENTARY_SERVICE_UT_TEST
                = "android.intent.action.ACTION_SUPPLEMENTARY_SERVICE_UT_TEST";
    static final String ACTION_SUPPLEMENTARY_SERVICE_ROAMING_TEST
                = "android.intent.action.ACTION_SUPPLEMENTARY_SERVICE_ROAMING_TEST";

    static final String SUPPLEMENTARY_SERVICE_PERMISSION =
            "com.mediatek.permission.SUPPLEMENTARY_SERVICE_UT_TEST";

    private static final String EXTRA_ACTION = "action";
    private static final String EXTRA_SERVICE_CODE = "serviceCode";
    private static final String EXTRA_SERVICE_INFO_A = "serviceInfoA";
    private static final String EXTRA_SERVICE_INFO_B = "serviceInfoB";
    private static final String EXTRA_SERVICE_INFO_C = "serviceInfoC";
    private static final String EXTRA_SERVICE_INFO_D = "serviceInfoD";
    private static final String EXTRA_PHONE_ID = "phoneId";

    private static final String SC_BAIC  = "35";
    private static final String SC_BAICr = "351";
    private static final String SC_CFU   = "21";
    private static final String SC_CFUR  = "22";
    private static final String SC_CFB   = "67";
    private static final String SC_CFNRy = "61";
    private static final String SC_CFNR  = "62";
    private static final String SC_CFNotRegister = "68";

    private static final String SC_WAIT = "43";
    private static final String SC_CLIR = "31";
    private static final String SC_CLIP = "30";
    private static final String SC_COLR = "77";
    private static final String SC_COLP = "76";

    private static final int ACTION_DEACTIVATE = 0;
    private static final int ACTION_ACTIVATE = 1;
    private static final int ACTION_INTERROGATE = 2;

    // Calling Line Presentation Constants
    private static final int NUM_PRESENTATION_ALLOWED     = 0;
    private static final int NUM_PRESENTATION_RESTRICTED  = 1;

    private int ssAction;
    private String serviceCode;
    private String serviceInfoA;
    private String serviceInfoB;
    private String serviceInfoC;
    private String serviceInfoD;
    private int phoneId;
    private MtkGsmCdmaPhone activePhone;
    private MtkImsPhone activeImsPhone;
    private Context mContext;

    public MtkSuppServUtTest(Context context, Intent intent, Phone phone) {
        ssAction     = intent.getIntExtra(EXTRA_ACTION, -1);
        serviceCode  = intent.getStringExtra(EXTRA_SERVICE_CODE);
        serviceInfoA = intent.getStringExtra(EXTRA_SERVICE_INFO_A);
        serviceInfoB = intent.getStringExtra(EXTRA_SERVICE_INFO_B);
        serviceInfoC = intent.getStringExtra(EXTRA_SERVICE_INFO_C);
        serviceInfoD = intent.getStringExtra(EXTRA_SERVICE_INFO_D);
        phoneId      = getValidPhoneId(intent.getIntExtra(EXTRA_PHONE_ID, -1));

        mContext    = context;
        activePhone = (MtkGsmCdmaPhone) phone;
        activeImsPhone = (MtkImsPhone) activePhone.getImsPhone();
    }

    void run() {
        Log.d(LOG_TAG, "onReceive, ssAction = " + ssAction +
                ", serviceCode = " + serviceCode + ", serviceInfoA = " + serviceInfoA +
                ", serviceInfoB = " + serviceInfoB + ", serviceInfoC = " + serviceInfoC +
                ", serviceInfoD = " + serviceInfoD + ", phoneId = " + phoneId);

        try {
            if (isServiceCodeCallForwarding(serviceCode)) {
                int cfAction = actionToCommandAction(ssAction);
                int reason = scToCallForwardReason(serviceCode);
                String dialingNumber = serviceInfoA;
                int serviceClass = siToServiceClass(serviceInfoB);
                int time = siToTime(serviceInfoC);
                long[] timeSlot = convertToLongTime(serviceInfoD);

                if (isInterrogate(ssAction)) {
                    if (reason == CommandsInterface.CF_REASON_UNCONDITIONAL) {
                        activePhone.getCallForwardInTimeSlot(reason, null);
                    } else {
                        activePhone.getCallForwardingOption(reason, null);
                    }
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    if ((reason == CommandsInterface.CF_REASON_UNCONDITIONAL)
                            && (timeSlot != null)) {
                        Log.d(LOG_TAG, "onReceive: setCallForwardInTimeSlot");
                        activePhone.setCallForwardInTimeSlot(cfAction, reason,
                               dialingNumber, time, timeSlot, null);
                    } else {
                        Log.d(LOG_TAG, "onReceive: setCallForwardingOption");
                        activePhone.setCallForwardingOption(cfAction, reason,
                                dialingNumber, time, null);
                    }
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (isServiceCodeCallBarring(serviceCode)) {
                boolean lockState = isActivate(ssAction);
                String facility = scToBarringFacility(serviceCode);
                int serviceClass = siToServiceClass(serviceInfoB);
                if (isInterrogate(ssAction)) {
                    activePhone.getCallBarring(facility, "1234", null);
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    activePhone.setCallBarring(facility, lockState,
                            "1234", null);
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_WAIT)) {
                boolean enable = isActivate(ssAction);
                int serviceClass = siToServiceClass(serviceInfoA);
                if (isInterrogate(ssAction)) {
                    activePhone.getCallWaiting(null);
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    activePhone.setCallWaiting(enable, null);
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_CFUR)){
                if(isActivate(ssAction) || isDeactivate(ssAction)){
                    Intent rIntent = new Intent(ACTION_SUPPLEMENTARY_SERVICE_ROAMING_TEST);
                    rIntent.putExtra(EXTRA_PHONE_ID, phoneId);
                    mContext.sendBroadcast(rIntent);
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported service code");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_CLIR)){
                checkIMSStatus(SC_CLIR);
                int clirMode = isActivate(ssAction) ?
                        CommandsInterface.CLIR_INVOCATION : CommandsInterface.CLIR_SUPPRESSION;
                if (isInterrogate(ssAction)) {
                    activePhone.getOutgoingCallerIdDisplay(null);
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    activePhone.setOutgoingCallerIdDisplay(clirMode, null);
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_CLIP)){
                checkIMSStatus(SC_CLIP);
                boolean clipMode = isActivate(ssAction);
                if (isInterrogate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().queryCLIP(null);
                    } catch (ImsException e) {
                        Log.d(LOG_TAG, "Could not get UT handle for queryCLIP.");
                    }
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().updateCLIP(clipMode, null);
                    } catch (ImsException e) {
                        Log.d(LOG_TAG, "Could not get UT handle for updateCLIP.");
                    }
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_COLR)){
                checkIMSStatus(SC_COLR);
                int colrMode = isActivate(ssAction) ?
                        NUM_PRESENTATION_RESTRICTED : NUM_PRESENTATION_ALLOWED;
                if (isInterrogate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().queryCOLR(null);
                    } catch (ImsException e) {
                        Log.d(LOG_TAG, "processCode: Could not get UT handle for queryCOLR.");
                    }
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().updateCOLR(colrMode, null);
                    } catch (ImsException e) {
                        Log.d(LOG_TAG, "processCode: Could not get UT handle for updateCOLR.");
                    }
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else if (serviceCode != null && serviceCode.equals(SC_COLP)){
                checkIMSStatus(SC_COLP);
                boolean colpMode = isActivate(ssAction);
                if (isInterrogate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().queryCOLP(null);
                    } catch (ImsException e) {
                        Log.d(LOG_TAG, "processCode: Could not get UT handle for queryCOLP.");
                    }
                } else if (isActivate(ssAction) || isDeactivate(ssAction)) {
                    try {
                        activeImsPhone.mCT.getUtInterface().updateCOLP(colpMode, null);
                     } catch (ImsException e) {
                         Log.d(LOG_TAG, "processCode: Could not get UT handle for updateCOLP.");
                     }
                } else {
                    Log.d(LOG_TAG, "onReceive: Not supported SS action");
                }
            } else {
                Log.d(LOG_TAG, "onReceive: Not supported service code");
            }
        } catch (RuntimeException e) {
            e.printStackTrace();
        }
    }

    private static int getValidPhoneId(int phoneId) {
        if (phoneId >= 0 && phoneId < TelephonyManager.getDefault().getPhoneCount()) {
            return phoneId;
        }
        return 0;
    }

    private static boolean isServiceCodeCallForwarding(String sc) {
        return sc != null &&
                (sc.equals(SC_CFU)
                || sc.equals(SC_CFB) || sc.equals(SC_CFNRy)
                || sc.equals(SC_CFNR) || sc.equals(SC_CFNotRegister));
    }

    private static boolean isServiceCodeCallBarring(String sc) {
        return sc != null &&
                (sc.equals(SC_BAIC)
                || sc.equals(SC_BAICr));
    }

    private static boolean isActivate(int action) {
        return action == ACTION_ACTIVATE;
    }

    private static boolean isDeactivate(int action) {
        return action == ACTION_DEACTIVATE;
    }

    private static boolean isInterrogate(int action) {
        return action == ACTION_INTERROGATE;
    }

    /* Only for the purpose of debugging, IMS status should already be ready
     * if we want to do XCAP PCT test case for CLIR/CLIP/COLR/COLP
     */
    private void checkIMSStatus(String reason) {
        Phone imsPhone = activeImsPhone;
        if ((imsPhone != null) &&
            ((imsPhone.getServiceState().getState() == ServiceState.STATE_IN_SERVICE)
                    || imsPhone.isUtEnabled())) {
            Log.d(LOG_TAG, "checkIMSStatus: ready, code: " + reason);
            return;
        }
        Log.d(LOG_TAG, "checkIMSStatus: IMS is not registered or not Ut enabled, code: " +
                reason);
    }

    private static int actionToCommandAction(int action) {
        switch (action) {
            case ACTION_DEACTIVATE: return CommandsInterface.CF_ACTION_DISABLE;
            case ACTION_ACTIVATE: return CommandsInterface.CF_ACTION_REGISTRATION;
            case ACTION_INTERROGATE: return 2;
            default:
                throw new RuntimeException("invalid action command");
        }
    }

    private static int scToCallForwardReason(String sc) {
        if (sc == null) {
            throw new RuntimeException("invalid call forward sc");
        }

        if (sc.equals(SC_CFU)) {
            return CommandsInterface.CF_REASON_UNCONDITIONAL;
        } else if (sc.equals(SC_CFB)) {
            return CommandsInterface.CF_REASON_BUSY;
        } else if (sc.equals(SC_CFNR)) {
            return CommandsInterface.CF_REASON_NOT_REACHABLE;
        } else if (sc.equals(SC_CFNRy)) {
            return CommandsInterface.CF_REASON_NO_REPLY;
        } else if (sc.equals(SC_CFNotRegister)) {
            return MtkRIL.CF_REASON_NOT_REGISTERED;
        } else {
            throw new RuntimeException("invalid call forward sc");
        }
    }

    private static String scToBarringFacility(String sc) {
        if (sc == null) {
            throw new RuntimeException("invalid call barring sc");
        }

        if (sc.equals(SC_BAIC)) {
            return CommandsInterface.CB_FACILITY_BAIC;
        } else if (sc.equals(SC_BAICr)) {
            return CommandsInterface.CB_FACILITY_BAICr;
        } else {
            throw new RuntimeException("invalid call barring sc");
        }
    }

    private static int siToServiceClass(String si) {
        if (si == null || si.length() == 0) {
                return  CommandsInterface.SERVICE_CLASS_NONE;
        } else {
            int serviceCode = Integer.parseInt(si, 10);

            switch (serviceCode) {
                case 1: return MtkRIL.SERVICE_CLASS_VOICE;
                case 2: return MtkRIL.SERVICE_CLASS_VIDEO;
                default:
                    throw new RuntimeException("unsupported service class " + si);
            }
        }
    }

    private static int siToTime(String si) {
        if (si == null || si.length() == 0) {
            return 0;
        } else {
            return Integer.parseInt(si, 10);
        }
    }

    private static long[] convertToLongTime(String timeSlotString) {
        long[] timeSlot = null;
        if (timeSlotString != null) {
            String[] timeArray = timeSlotString.split(",", 2);
            if (timeArray.length == 2) {
                timeSlot = new long[2];
                for (int i = 0; i < 2; i++) {
                    SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                    dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                    try {
                        Date date = dateFormat.parse(timeArray[i]);
                        timeSlot[i] = date.getTime();
                    } catch (ParseException e) {
                        e.printStackTrace();
                        return null;
                    }
                }
            }
        }
        return timeSlot;
    }
}