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

package com.mediatek.internal.telephony.imsphone;

import android.content.Context;
import android.content.res.Resources;
import android.os.AsyncResult;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.ResultReceiver;
import android.os.SystemProperties;
import android.telephony.CarrierConfigManager;
import android.telephony.PhoneNumberUtils;
import android.telephony.TelephonyManager;
import android.text.SpannableStringBuilder;
import android.text.TextUtils;
import android.telephony.Rlog;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsInfo;

import com.android.ims.ImsException;
import com.android.ims.ImsUtInterface;
import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.util.ArrayUtils;

import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_NONE;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_VOICE;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_DATA;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_FAX;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_SMS;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_DATA_SYNC;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_DATA_ASYNC;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_PACKET;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_PAD;
import static com.android.internal.telephony.CommandsInterface.SERVICE_CLASS_MAX;

import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.IMS_DEREG_PROP;
import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.IMS_DEREG_ON;
import static com.mediatek.internal.telephony.MtkGsmCdmaPhone.IMS_DEREG_OFF;
import static com.mediatek.internal.telephony.MtkRIL.SERVICE_CLASS_VIDEO;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.PROPERTY_TBCW_MODE;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_DISABLED;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_OFF;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_ON;

import static android.telephony.ServiceState.STATE_IN_SERVICE;

import com.mediatek.ims.MtkImsUt;
import com.mediatek.ims.MtkImsReasonInfo;

import com.android.internal.telephony.MmiCode;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneMmiCode;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSuppServHelper;

import com.mediatek.internal.telephony.imsphone.MtkImsPhone;

import java.util.Arrays;
import java.util.Random;
import java.util.regex.Pattern;
import java.util.regex.Matcher;


/**
 * {@hide}
 */
public final class MtkImsPhoneMmiCode extends ImsPhoneMmiCode {
    static final String LOG_TAG = "MtkImsPhoneMmiCode";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");

    private static final String SC_CFNotRegister = "68";

    //***** Public Class methods

    /**
     * Some dial strings in GSM are defined to do non-call setup
     * things, such as modify or query supplementary service settings (eg, call
     * forwarding). These are generally referred to as "MMI codes".
     * We look to see if the dial string contains a valid MMI code (potentially
     * with a dial string at the end as well) and return info here.
     *
     * If the dial string contains no MMI code, we return an instance with
     * only "dialingNumber" set
     *
     * Please see flow chart in TS 22.030 6.5.3.2
     */

    public static MtkImsPhoneMmiCode newFromDialString(String dialString, ImsPhone phone) {
       return newFromDialString(dialString, phone, null);
    }

    public static MtkImsPhoneMmiCode newFromDialString(String dialString,
                                             ImsPhone phone, ResultReceiver wrappedCallback) {
        Matcher m;
        MtkImsPhoneMmiCode ret = null;

        Rlog.d(LOG_TAG, "newFromDialString, dialstring = " + MtkSuppServHelper.encryptString(dialString));

        /// To improve call setup performace @{
        boolean isMmi = dialString.startsWith("*") || dialString.startsWith("#") ||
                        dialString.endsWith("#");
        if (!(isMmi || dialString.length() <= 2)) {
            Rlog.d(LOG_TAG, "Not belong to MMI format.");
            return null;
        }
        /// @}

        m = sPatternSuppService.matcher(dialString);

        // Is this formatted like a standard supplementary service code?
        if (m.matches()) {
            ret = new MtkImsPhoneMmiCode(phone);
            ret.mPoundString = makeEmptyNull(m.group(MATCH_GROUP_POUND_STRING));
            ret.mAction = makeEmptyNull(m.group(MATCH_GROUP_ACTION));
            ret.mSc = makeEmptyNull(m.group(MATCH_GROUP_SERVICE_CODE));
            ret.mSia = makeEmptyNull(m.group(MATCH_GROUP_SIA));
            ret.mSib = makeEmptyNull(m.group(MATCH_GROUP_SIB));
            ret.mSic = makeEmptyNull(m.group(MATCH_GROUP_SIC));
            ret.mPwd = makeEmptyNull(m.group(MATCH_GROUP_PWD_CONFIRM));
            ret.mDialingNumber = makeEmptyNull(m.group(MATCH_GROUP_DIALING_NUMBER));
            ret.mCallbackReceiver = wrappedCallback;
            // According to TS 22.030 6.5.2 "Structure of the MMI",
            // the dialing number should not ending with #.
            // The dialing number ending # is treated as unique USSD,
            // eg, *400#16 digit number# to recharge the prepaid card
            // in India operator(Mumbai MTNL)
            if (ret.mDialingNumber != null &&
                    ret.mDialingNumber.endsWith("#") &&
                    dialString.endsWith("#")){
                ret = new MtkImsPhoneMmiCode(phone);
                ret.mPoundString = dialString;
            } else if (ret.isFacToDial()) {
                // This is a FAC (feature access code) to dial as a normal call.
                ret = null;
            }
        } else if (dialString.endsWith("#")) {
            // TS 22.030 sec 6.5.3.2
            // "Entry of any characters defined in the 3GPP TS 23.038 [8] Default Alphabet
            // (up to the maximum defined in 3GPP TS 24.080 [10]), followed by #SEND".

            ret = new MtkImsPhoneMmiCode(phone);
            ret.mPoundString = dialString;
        } else if (isTwoDigitShortCode(phone.getContext(), dialString)) {
            //Is a country-specific exception to short codes as defined in TS 22.030, 6.5.3.2
            ret = null;
        } else if (isShortCode(dialString, phone)) {
            // this may be a short code, as defined in TS 22.030, 6.5.3.2
            ret = new MtkImsPhoneMmiCode(phone);
            ret.mDialingNumber = dialString;
        }

        return ret;
    }

    public static MtkImsPhoneMmiCode
    newNetworkInitiatedUssd(String ussdMessage, boolean isUssdRequest, MtkImsPhone phone) {
        MtkImsPhoneMmiCode ret;

        ret = new MtkImsPhoneMmiCode(phone);

        ret.mMessage = ussdMessage;
        ret.mIsUssdRequest = isUssdRequest;

        // If it's a request, set to PENDING so that it's cancelable.
        if (isUssdRequest) {
            ret.mIsPendingUSSD = true;
            ret.mState = State.PENDING;
        } else {
            ret.mState = State.COMPLETE;
        }

        return ret;
    }

    public static MtkImsPhoneMmiCode newNetworkInitiatedUssdError(String ussdMessage,
            MtkImsPhone phone) {
        MtkImsPhoneMmiCode ret;
        ret = new MtkImsPhoneMmiCode(phone);
        if (ussdMessage != null && ussdMessage.length() > 0) {
            ret.mMessage = ussdMessage;
        } else {
            ret.mMessage = ret.mContext.getText(com.android.internal.R.string.mmiError);
        }
        ret.mIsUssdRequest = false;
        ret.mState = State.FAILED;

        return ret;
    }

    public static MtkImsPhoneMmiCode newFromUssdUserInput(String ussdMessge, MtkImsPhone phone) {
        MtkImsPhoneMmiCode ret = new MtkImsPhoneMmiCode(phone);

        ret.mMessage = ussdMessge;
        ret.mState = State.PENDING;
        ret.mIsPendingUSSD = true;

        return ret;
    }

    private static int
    siToServiceClass(String si) {
        if (si == null || si.length() == 0) {
                return  SERVICE_CLASS_NONE;
        } else {
            // NumberFormatException should cause MMI fail
            int serviceCode = Integer.parseInt(si, 10);

            switch (serviceCode) {
                case 10: return SERVICE_CLASS_SMS + SERVICE_CLASS_FAX  + SERVICE_CLASS_VOICE;
                case 11: return SERVICE_CLASS_VOICE;
                case 12: return SERVICE_CLASS_SMS + SERVICE_CLASS_FAX;
                case 13: return SERVICE_CLASS_FAX;

                case 16: return SERVICE_CLASS_SMS;

                case 19: return SERVICE_CLASS_FAX + SERVICE_CLASS_VOICE;

                case 20: return SERVICE_CLASS_DATA_ASYNC + SERVICE_CLASS_DATA_SYNC;

                case 21: return SERVICE_CLASS_PAD + SERVICE_CLASS_DATA_ASYNC;
                case 22: return SERVICE_CLASS_PACKET + SERVICE_CLASS_DATA_SYNC;
                case 24: return SERVICE_CLASS_DATA_SYNC + MtkRIL.SERVICE_CLASS_VIDEO;
                case 25: return SERVICE_CLASS_DATA_ASYNC;
                case 26: return SERVICE_CLASS_DATA_SYNC + SERVICE_CLASS_VOICE;
                case 99: return SERVICE_CLASS_PACKET;

                default:
                    throw new RuntimeException("unsupported MMI service code " + si);
            }
        }
    }

    //***** Constructor

    public MtkImsPhoneMmiCode(ImsPhone phone) {
        super(phone);
    }

    @Override
    public boolean
    isSupportedOverImsPhone() {
        if (isShortCode()) return true;
        else if (mDialingNumber != null) return false;
        else if (mSc != null && mSc.equals(SC_CNAP)) return false;
        else if (isServiceCodeCallForwarding(mSc)
                || isServiceCodeCallBarring(mSc)
                || (mSc != null && mSc.equals(SC_WAIT))
                || (mSc != null && mSc.equals(SC_CLIR))
                || (mSc != null && mSc.equals(SC_CLIP))
                || (mSc != null && mSc.equals(SC_COLR))
                || (mSc != null && mSc.equals(SC_COLP))
                || (mSc != null && mSc.equals(SC_BS_MT))
                || (mSc != null && mSc.equals(SC_BAICa))) {
            if (supportMdAutoSetupIms()) {
                try {
                    int serviceClass = siToServiceClass(mSib);
                    if (((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) ||
                        ((serviceClass & MtkRIL.SERVICE_CLASS_VIDEO) != 0) ||
                        (serviceClass == CommandsInterface.SERVICE_CLASS_NONE)) {
                        Rlog.d(LOG_TAG, "isSupportedOverImsPhone(), return true!");
                        return true;
                    }
                    /// @}
                    return false;
                } catch (RuntimeException exc) {
                    Rlog.d(LOG_TAG, "Invalid service class " + exc);
                }
            } else {
                if (mPhone.isVolteEnabled()
                    || (mPhone.isWifiCallingEnabled()
                    && ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isWFCUtSupport())) {
                    try {
                        int serviceClass = siToServiceClass(mSib);
                        /// M:  ALPS03012236,handle with voice, video and NONE in Ut. @{
                        if (((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) ||
                            ((serviceClass & MtkRIL.SERVICE_CLASS_VIDEO) != 0) ||
                             (serviceClass == CommandsInterface.SERVICE_CLASS_NONE)) {
                            Rlog.d(LOG_TAG, "isSupportedOverImsPhone(), return true!");
                            return true;
                        }
                        /// @}
                    } catch (RuntimeException exc) {
                        Rlog.d(LOG_TAG, "exc.toString() = " + exc.toString());
                    }
                }
            }
            return false;
        } else if (isPinPukCommand()
                || (mSc != null
                    && (mSc.equals(SC_PWD) || mSc.equals(SC_CLIP) || mSc.equals(SC_CLIR)))) {
            return false;
        } else if (mPoundString != null) return true;

        return false;
    }

    /** Process a MMI code or short code...anything that isn't a dialing number */
    @Override
    public void
    processCode () throws CallStateException {
        try {
            if ((supportMdAutoSetupIms() == false) &&
                    ((MtkGsmCdmaPhone)mPhone.mDefaultPhone).getCsFallbackStatus()
                    != MtkPhoneConstants.UT_CSFB_PS_PREFERRED) {
                Rlog.d(LOG_TAG, "processCode(): getCsFallbackStatus(): CS Fallback!");
                ((MtkImsPhone)mPhone).removeMmi(this);
                throw new CallStateException(Phone.CS_FALLBACK);
            }

            if (isShortCode()) {
                Rlog.d(LOG_TAG, "processCode: isShortCode");

                // These just get treated as USSD.
                Rlog.d(LOG_TAG, "processCode: Sending short code '"
                       + mDialingNumber + "' over CS pipe.");
                throw new CallStateException(Phone.CS_FALLBACK);
            } else if (isServiceCodeCallForwarding(mSc)) {
                handleCallForward();
            } else if (isServiceCodeCallBarring(mSc)) {
                handleCallBarring();
            } else if (mSc != null && mSc.equals(SC_CLIR)) {
                handleCLIR();
            } else if (mSc != null && mSc.equals(SC_CLIP)) {
                handleCLIP();
            } else if (mSc != null && mSc.equals(SC_COLP)) {
                handleCOLP();
            } else if (mSc != null && mSc.equals(SC_COLR)) {
                handleCOLR();
            } else if (mSc != null && (mSc.equals(SC_BS_MT))) {
                handleCallBarringSpecificMT();
            } else if (mSc != null && mSc.equals(SC_BAICa)) {
                handleCallBarringACR();
            } else if (mSc != null && mSc.equals(SC_WAIT)) {
                handleCW();
            } else if (mPoundString != null) {
                // We'll normally send USSD over the CS pipe, but if it happens that the CS phone
                // is out of service, we'll just try over IMS instead.
                if (mPhone.getDefaultPhone().getServiceStateTracker().mSS.getState()
                        == STATE_IN_SERVICE) {
                    Rlog.i(LOG_TAG, "processCode: Sending ussd string '"
                            + Rlog.pii(LOG_TAG, mPoundString) + "' over CS pipe.");
                    throw new CallStateException(Phone.CS_FALLBACK);
                } else {
                    Rlog.i(LOG_TAG, "processCode: CS is out of service, sending ussd string '"
                            + Rlog.pii(LOG_TAG, mPoundString) + "' over IMS pipe.");
                    sendUssd(mPoundString);
                }
            } else {
                throw new RuntimeException ("Invalid or Unsupported MMI Code");
            }
        } catch (RuntimeException exc) {
            exc.printStackTrace();
            Rlog.d(LOG_TAG, "procesCode: mState = FAILED");
            mState = State.FAILED;
            mMessage = mContext.getText(com.android.internal.R.string.mmiError);
            mPhone.onMMIDone(this);
        }
    }

    /* Query Call Forward & Set Call Forward*/
    void handleCallForward() {
        Rlog.d(LOG_TAG, "processCode: is CF");

        String dialingNumber = mSia;
        int reason = scToCallForwardReason(mSc);
        int serviceClass = siToServiceClass(mSib);
        int time = siToTime(mSic);

        if (isInterrogate()) {
            if (serviceClass != SERVICE_CLASS_NONE) {
                if ((MtkGsmCdmaPhone)mPhone.mDefaultPhone instanceof MtkGsmCdmaPhone) {
                    ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).setServiceClass(serviceClass);
                }
            }
            mPhone.getCallForwardingOption(reason,
                    obtainMessage(EVENT_QUERY_CF_COMPLETE, this));
        } else {
            int cfAction;

            if (isActivate()) {
                // 3GPP TS 22.030 6.5.2
                // a call forwarding request with a single * would be
                // interpreted as registration if containing a forwarded-to
                // number, or an activation if not
                if (isEmptyOrNull(dialingNumber)) {
                    cfAction = CommandsInterface.CF_ACTION_ENABLE;
                    mIsCallFwdReg = false;
                } else {
                    cfAction = CommandsInterface.CF_ACTION_REGISTRATION;
                    mIsCallFwdReg = true;
                }
            } else if (isDeactivate()) {
                cfAction = CommandsInterface.CF_ACTION_DISABLE;
            } else if (isRegister()) {
                cfAction = CommandsInterface.CF_ACTION_REGISTRATION;
            } else if (isErasure()) {
                cfAction = CommandsInterface.CF_ACTION_ERASURE;
            } else {
                throw new RuntimeException ("invalid action");
            }

            int isSettingUnconditional =
                    ((reason == CommandsInterface.CF_REASON_UNCONDITIONAL) ||
                     (reason == CommandsInterface.CF_REASON_ALL)) &&
                    (((serviceClass & CommandsInterface.SERVICE_CLASS_VOICE) != 0) ||
                    (serviceClass == CommandsInterface.SERVICE_CLASS_NONE)) ? 1 : 0;

            int isEnableDesired =
                ((cfAction == CommandsInterface.CF_ACTION_ENABLE) ||
                        (cfAction == CommandsInterface.CF_ACTION_REGISTRATION)) ? 1 : 0;

            Rlog.d(LOG_TAG, "is CF setCallForward");

            if (((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpReregisterForCF()) {
                Rlog.i(LOG_TAG, "Set ims dereg to ON.");
                SystemProperties.set(IMS_DEREG_PROP, IMS_DEREG_ON);
            }

            mPhone.setCallForwardingOption(cfAction, reason,
                    dialingNumber, serviceClass, time, obtainMessage(
                            EVENT_SET_CFF_COMPLETE,
                            isSettingUnconditional,
                            isEnableDesired, this));
        }
    }

    /* Query Call Barring & Set Call Barring */
    void handleCallBarring() {
        Rlog.d(LOG_TAG, "processCode: is CB");
        // sia = password
        // sib = basic service group
        // service group is not supported

        String password = mSia;
        String facility = scToBarringFacility(mSc);
        int serviceClass = siToServiceClass(mSib);

        if (isInterrogate()) {
            mPhone.getCallBarring(facility, password,
                    obtainMessage(EVENT_SUPP_SVC_QUERY_COMPLETE, this), serviceClass);
        } else if (isActivate() || isDeactivate()) {
            mPhone.setCallBarring(facility, isActivate(), password,
                    obtainMessage(EVENT_SET_COMPLETE, this), serviceClass);
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Query Call Waiting & Set Call Waiting */
    void handleCW() {
        Rlog.d(LOG_TAG, "processCode: is CW");
        // sia = basic service group
        int serviceClass = siToServiceClass(mSia);

        if (supportMdAutoSetupIms()) {
            // 93 AOSP logic
            if (isActivate() || isDeactivate()) {
                mPhone.setCallWaiting(isActivate(), serviceClass,
                        obtainMessage(EVENT_SET_COMPLETE, this));
            } else if (isInterrogate()) {
                mPhone.getCallWaiting(obtainMessage(EVENT_QUERY_COMPLETE, this));
            } else {
                throw new RuntimeException ("Invalid or Unsupported MMI Code");
            }
        } else {
            // non 93 logic
            int tbcwMode = MtkGsmCdmaPhone.TBCW_UNKNOWN;
            tbcwMode = ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).getTbcwMode();
            if (isActivate() || isDeactivate()) {
                /* Terminal-based Call Waiting */
                if (((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpNwCW()) {
                    Rlog.d(LOG_TAG, "setCallWaiting() by Ut interface.");
                    mPhone.setCallWaiting(isActivate(), serviceClass,
                        obtainMessage(EVENT_SET_COMPLETE, this));
                } else if (tbcwMode == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                    ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).mCi.setCallWaiting(
                            isActivate(), serviceClass, obtainMessage(EVENT_SET_COMPLETE,
                            isActivate() ? 1 : 0, -1, this));
                } else {
                    /* Terminal-based Call Waiting */
                    String tbcwStatus = TelephonyManager.getTelephonyProperty(
                            mPhone.mDefaultPhone.getPhoneId(),
                            PROPERTY_TBCW_MODE,
                            TBCW_DISABLED);
                    Rlog.d(LOG_TAG, "setTerminal-based CallWaiting(): tbcwStatus = " + tbcwStatus
                            + ", enable = " + isActivate());
                    if (!tbcwStatus.equals(TBCW_DISABLED)) {
                        ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).setTerminalBasedCallWaiting(
                                isActivate(), obtainMessage(EVENT_SET_COMPLETE, this));
                    } else {
                        Rlog.d(LOG_TAG, "setCallWaiting() by Ut interface.");
                        mPhone.setCallWaiting(isActivate(), serviceClass,
                            obtainMessage(EVENT_SET_COMPLETE, this));
                    }
                }
            } else if (isInterrogate()) {
                tbcwMode = ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).getTbcwMode();
                if (((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpNwCW()) {
                    Rlog.d(LOG_TAG, "getCallWaiting() by Ut interface.");
                    mPhone.getCallWaiting(obtainMessage(EVENT_QUERY_COMPLETE, this));
                } else if (tbcwMode == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                    ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).mCi.queryCallWaiting(
                            serviceClass, obtainMessage(EVENT_QUERY_COMPLETE, this));
                } else {
                    /* Terminal-based Call Waiting */
                    String tbcwStatus = TelephonyManager.getTelephonyProperty(
                                            mPhone.mDefaultPhone.getPhoneId(),
                                            PROPERTY_TBCW_MODE,
                                            TBCW_DISABLED);
                    Rlog.d(LOG_TAG, "SC_WAIT isInterrogate() tbcwStatus = " + tbcwStatus);
                    if (TBCW_ON.equals(tbcwStatus)) {
                        int[] cwInfos = new int[2];
                        cwInfos[0] = 1;
                        cwInfos[1] = SERVICE_CLASS_VOICE;

                        Message msg = obtainMessage(EVENT_QUERY_COMPLETE, null);
                        AsyncResult.forMessage(msg, cwInfos, null);
                        sendMessage(msg);
                    } else if (TBCW_OFF.equals(tbcwStatus)) {
                        int[] cwInfos = new int[2];
                        cwInfos[0] = 0;

                        Message msg = obtainMessage(EVENT_QUERY_COMPLETE, null);
                        AsyncResult.forMessage(msg, cwInfos, null);
                        sendMessage(msg);
                    } else {
                        Rlog.d(LOG_TAG, "getCallWaiting() by Ut interface.");
                        mPhone.getCallWaiting(obtainMessage(EVENT_QUERY_COMPLETE, this));
                    }
                }
            } else {
                throw new RuntimeException ("Invalid or Unsupported MMI Code");
            }
        }
    }

    /* Set CLIP & Query CLIP*/
    void handleCLIP() {
        Rlog.d(LOG_TAG, "processCode: is CLIP");
        // NOTE: Refer to the note above.
        if (!checkIfOPSupportCallerID()) {
            return;
        }
        if (isInterrogate()) {
            try {
                mPhone.mCT.getUtInterface()
                    .queryCLIP(obtainMessage(EVENT_SUPP_SVC_QUERY_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for queryCLIP.");
            }
        } else if (isActivate() || isDeactivate()) {
            try {
                mPhone.mCT.getUtInterface().updateCLIP(isActivate(),
                        obtainMessage(EVENT_SET_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for updateCLIP.");
            }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Set CLIR & Query CLIR */
    void handleCLIR() {
        Rlog.d(LOG_TAG, "processCode: is CLIR");
        // NOTE: Since these supplementary services are accessed only
        //       via MMI codes, methods have not been added to ImsPhone.
        //       Only the UT interface handle is used.
        if (!checkIfOPSupportCallerID()) {
            return;
        }
        if (isActivate()) {
            if ((supportMdAutoSetupIms() == false) &&
                ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpTbClir()) {
                ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).mCi.setCLIR(
                        CommandsInterface.CLIR_INVOCATION,
                        obtainMessage(EVENT_SET_COMPLETE,
                        CommandsInterface.CLIR_INVOCATION, 0, this));
                return;
            }

            try {
                mPhone.mCT.getUtInterface().updateCLIR(CommandsInterface.CLIR_INVOCATION,
                    obtainMessage(EVENT_SET_COMPLETE,
                        CommandsInterface.CLIR_INVOCATION, 0, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for updateCLIR.");
            }
        } else if (isDeactivate()) {
            if ((supportMdAutoSetupIms() == false) &&
                    ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpTbClir()) {
                ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).mCi.setCLIR(
                    CommandsInterface.CLIR_SUPPRESSION,
                    obtainMessage(EVENT_SET_COMPLETE,
                        CommandsInterface.CLIR_SUPPRESSION, 0, this));
                return;
            }

            try {
                mPhone.mCT.getUtInterface().updateCLIR(CommandsInterface.CLIR_SUPPRESSION,
                    obtainMessage(EVENT_SET_COMPLETE,
                        CommandsInterface.CLIR_SUPPRESSION, 0, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for updateCLIR.");
            }
        } else if (isInterrogate()) {
            if ((supportMdAutoSetupIms() == false) &&
                    ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpTbClir()) {
                Message msg = obtainMessage(EVENT_GET_CLIR_COMPLETE, this);
                if (msg != null) {
                    int[] result =
                        ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).getSavedClirSetting();
                    Bundle info = new Bundle();
                    info.putIntArray(UT_BUNDLE_KEY_CLIR, result);

                    AsyncResult.forMessage(msg, info, null);
                    msg.sendToTarget();
                }
                return;
            }

            try {
                mPhone.mCT.getUtInterface()
                    .queryCLIR(obtainMessage(EVENT_GET_CLIR_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for queryCLIR.");
            }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Query COLP, SET COLP as *76# or #76# is not supported */
    void handleCOLP() {
        Rlog.d(LOG_TAG, "processCode: is COLP");
        // NOTE: Refer to the note above.
        if (!checkIfOPSupportCallerID()) {
            return;
        }
        if (isInterrogate()) {
            try {
                mPhone.mCT.getUtInterface()
                    .queryCOLP(obtainMessage(EVENT_SUPP_SVC_QUERY_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for queryCOLP.");
            }
        } else if (isActivate() || isDeactivate()) {
            try {
                mPhone.mCT.getUtInterface().updateCOLP(isActivate(),
                            obtainMessage(EVENT_SET_COMPLETE, this));
                } catch (ImsException e) {
                    Rlog.d(LOG_TAG, "Could not get UT handle for updateCOLP.");
                }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Query COLR, SET COLR as *76# or #76# is not supported  */
    void handleCOLR() {
        Rlog.d(LOG_TAG, "processCode: is COLR");
        // NOTE: Refer to the note above.
        if (!checkIfOPSupportCallerID()) {
            return;
        }
        if (isActivate()) {
            try {
                mPhone.mCT.getUtInterface().updateCOLR(NUM_PRESENTATION_RESTRICTED,
                        obtainMessage(EVENT_SET_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for updateCOLR.");
            }
        } else if (isDeactivate()) {
            try {
                mPhone.mCT.getUtInterface().updateCOLR(NUM_PRESENTATION_ALLOWED,
                        obtainMessage(EVENT_SET_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for updateCOLR.");
            }
        } else if (isInterrogate()) {
            try {
                mPhone.mCT.getUtInterface()
                    .queryCOLR(obtainMessage(EVENT_SUPP_SVC_QUERY_COMPLETE, this));
            } catch (ImsException e) {
                Rlog.d(LOG_TAG, "Could not get UT handle for queryCOLR.");
            }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Handle Barring of Specific Incoming calls */
    void handleCallBarringSpecificMT() {
        Rlog.d(LOG_TAG, "processCode: is CB (specifc MT)");
        try {
            if (isInterrogate()) {
                mPhone.mCT.getUtInterface().queryCallBarring(ImsUtInterface.CB_BS_MT,
                        obtainMessage(EVENT_QUERY_ICB_COMPLETE,this));
            } else {
                processIcbMmiCodeForUpdate();
            }
         // TODO: isRegister() case needs to be handled.
        } catch (ImsException e) {
            Rlog.d(LOG_TAG, "Could not get UT handle for ICB.");
        }
    }

    /* Handle Barring of Anonymous Communication Rejection (ACR) */
    void handleCallBarringACR() {
        Rlog.d(LOG_TAG, "processCode: is CB (ACR)");
        int callAction =0;
        String password = mSia;
        int serviceClass = siToServiceClass(mSib);

        // TODO: Should we route through queryCallBarring() here?
        try {
            if (isInterrogate()) {
                mPhone.mCT.getUtInterface()
                .queryCallBarring(ImsUtInterface.CB_BIC_ACR,
                                  obtainMessage(EVENT_QUERY_ICB_COMPLETE,this));
            } else {
                if (isActivate()) {
                    callAction = CommandsInterface.CF_ACTION_ENABLE;
                } else if (isDeactivate()) {
                    callAction = CommandsInterface.CF_ACTION_DISABLE;
                }
                ((MtkImsUt) mPhone.mCT.getUtInterface()).updateCallBarring(
                        password,
                        ImsUtInterface.CB_BIC_ACR,
                        callAction,
                        obtainMessage(EVENT_SET_COMPLETE,this),
                        null,
                        serviceClass);
            }
        } catch (ImsException e) {
            Rlog.d(LOG_TAG, "Could not get UT handle for ICBa.");
        }
    }

    /** Called from ImsPhone.handleMessage; not a Handler subclass */
    @Override
    public void
    handleMessage (Message msg) {
        AsyncResult ar;

        if (triggerMmiCodeCsfb(msg)) {
            return;
        }

        switch (msg.what) {
            case EVENT_SET_COMPLETE:
                ar = (AsyncResult) (msg.obj);

                if (!supportMdAutoSetupIms()) {
                    if (mSc.equals(SC_WAIT) &&
                        ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).getTbcwMode() ==
                          MtkGsmCdmaPhone.TBCW_WITH_CS) {
                        if (ar.exception == null) {
                            boolean enable = (msg.arg1 == 1) ? true : false;
                            ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).setTerminalBasedCallWaiting(
                                    enable, null);
                        }
                    }
                }

                onSetComplete(msg, ar);
                break;

            case EVENT_SET_CFF_COMPLETE:
                ar = (AsyncResult) (msg.obj);

                /*
                * msg.arg1 = 1 means to set unconditional voice call forwarding
                * msg.arg2 = 1 means to enable voice call forwarding
                */
                /* M: SS part */
                if ((ar.exception == null) && (msg.arg1 == 1)) {
                    boolean cffEnabled = (msg.arg2 == 1);  //The action value from user.
                    if (((MtkGsmCdmaPhone) mPhone.mDefaultPhone).queryCFUAgainAfterSet()) {
                        if (ar.result != null) {
                            CallForwardInfo[] cfInfos = (CallForwardInfo[]) ar.result;
                            if (cfInfos == null || cfInfos.length == 0) {
                                Rlog.i(LOG_TAG, "cfInfo is null or length is 0.");
                            } else {
                                for (int i = 0; i < cfInfos.length; i++) {
                                    if ((cfInfos[i].serviceClass & SERVICE_CLASS_VOICE) != 0) {
                                        // reset the value which from query.
                                        if (cfInfos[i].status == 1) {
                                            Rlog.i(LOG_TAG, "Set CF_ENABLE, serviceClass: "
                                                + cfInfos[i].serviceClass);
                                            cffEnabled = true;
                                        } else {
                                            Rlog.i(LOG_TAG, "Set CF_DISABLE, serviceClass: "
                                                + cfInfos[i].serviceClass);
                                            cffEnabled = false;
                                        }
                                        break;
                                    }
                                }
                            }
                        } else {
                            Rlog.i(LOG_TAG, "ar.result is null.");
                        }
                    }
                    Rlog.i(LOG_TAG, "EVENT_SET_CFF_COMPLETE: cffEnabled:" + cffEnabled
                            + ", mDialingNumber=" + mDialingNumber
                            + ", mIccRecords=" + mIccRecords);
                    if (mIccRecords != null) {
                        ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).setVoiceCallForwardingFlag(1,
                                cffEnabled, mDialingNumber);
                        /// M: SS OP01 Ut
                        ((MtkImsPhone)mPhone).saveTimeSlot(null);
                    }
                }
                /* M: SS part end */

                onSetComplete(msg, ar);
                break;

            default:
                // If the msg.what is not in above cases, then go to AOSP's switch case
                super.handleMessage(msg);
                break;
        }
    }

    public static boolean isUtMmiCode(String dialString, ImsPhone dialPhone) {

        MtkImsPhoneMmiCode mmi = MtkImsPhoneMmiCode.newFromDialString(dialString, dialPhone);
        if (mmi == null || mmi.isTemporaryModeCLIR()) {
            return false;
        }

        if (mmi.isShortCode() || mmi.mDialingNumber != null) {
            return false;
        } else if (mmi.mSc != null
                && (mmi.mSc.equals(SC_CLIP)
                || mmi.mSc.equals(SC_CLIR)
                || mmi.mSc.equals(SC_COLP)
                || mmi.mSc.equals(SC_COLR)
                || isServiceCodeCallForwarding(mmi.mSc)
                || isServiceCodeCallBarring(mmi.mSc)
                || mmi.mSc.equals(SC_WAIT)
                || mmi.mSc.equals(SC_BS_MT)
                || mmi.mSc.equals(SC_BAICa)
                )) {
            return true;
        }
        return false;
    }
    /// @}

    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    @Override
    protected void onQueryCfComplete(AsyncResult ar) {
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception != null) {
            super.onQueryCfComplete(ar);
            return;
        } else {
            CallForwardInfo infos[];

            infos = (CallForwardInfo[]) ar.result;

            if (infos == null || infos.length == 0) {
                // Assume the default is not active
                sb.append(mContext.getText(com.android.internal.R.string.serviceDisabled));

                // Set unconditional CFF in SIM to false
                if (mIccRecords != null) {
                    mPhone.setVoiceCallForwardingFlag(1, false, null);
                }
            } else {

                SpannableStringBuilder tb = new SpannableStringBuilder();

                // Each bit in the service class gets its own result line
                // The service classes may be split up over multiple
                // CallForwardInfos. So, for each service class, find out
                // which CallForwardInfo represents it and then build
                // the response text based on that

                for (int serviceClassMask = 1;
                        serviceClassMask <= MtkRIL.SERVICE_CLASS_MTK_MAX;
                        serviceClassMask <<= 1) {
                    if (serviceClassMask == MtkRIL.SERVICE_CLASS_LINE2) continue;

                    for (int i = 0, s = infos.length; i < s ; i++) {
                        if ((serviceClassMask & infos[i].serviceClass) != 0) {
                            tb.append(makeCFQueryResultMessage(infos[i],
                                            serviceClassMask));
                            tb.append("\n");
                        }
                    }
                }
                sb.append(tb);
            }

            mState = State.COMPLETE;
        }

        mMessage = sb;
        Rlog.d(LOG_TAG, "onQueryCfComplete: mmi=" + this);
        mPhone.onMMIDone(this);

    }

    @Override
    protected void onSuppSvcQueryComplete(AsyncResult ar) {
        /* The response of call barring may contain other service classes (e.g. video),
         * but it is not considered in AOSP's onSuppSvcQueryComplete().
         * So override this function to handle these cases */
        if (isServiceCodeCallBarring(mSc) && ar.exception == null
                && !(ar.result instanceof Bundle)) {
            StringBuilder sb = new StringBuilder(getScString());
            sb.append("\n");

            Rlog.d(LOG_TAG, "onSuppSvcQueryComplete: Received Call Barring Response.");
            // Response for Call Barring queries.
            int[] cbInfos = (int[]) ar.result;
            if (cbInfos[0] == 0) {
                sb.append(mContext.getText(com.android.internal.R.string.serviceDisabled));
            } else {
                // ints[0] for Call Barring is a bit vector of services
                sb.append(createQueryCallBarringResultMessage(cbInfos[0]));
            }

            mState = State.COMPLETE;
            mMessage = sb;
            Rlog.d(LOG_TAG, "onSuppSvcQueryComplete mmi=" + this);
            mPhone.onMMIDone(this);
            return;
        }
        super.onSuppSvcQueryComplete(ar);
    }

    private CharSequence createQueryCallBarringResultMessage(int serviceClass)
    {
        StringBuilder sb = new StringBuilder(mContext.
                getText(com.android.internal.R.string.serviceEnabledFor));

        for (int classMask = 1
                    ; classMask <= MtkRIL.SERVICE_CLASS_MTK_MAX
                    ; classMask <<= 1
        ) {
            if ((classMask & serviceClass) != 0) {
                sb.append("\n");
                sb.append(serviceClassToCFString(classMask & serviceClass));
            }
        }
        return sb;
    }

    /**
     * @param serviceClass 1 bit of the service class bit vectory
     * @return String to be used for call forward query MMI response text.
     *        Returns null if unrecognized
     */
    @Override
    protected CharSequence serviceClassToCFString (int serviceClass) {
        Rlog.d(LOG_TAG, "serviceClassToCFString, serviceClass = " + serviceClass);
        switch (serviceClass) {
            case MtkRIL.SERVICE_CLASS_LINE2:
            case MtkRIL.SERVICE_CLASS_VIDEO:
                return mContext.getText(com.mediatek.R.string.serviceClassVideo);
            default:
                return super.serviceClassToCFString(serviceClass);
        }
    }

    @Override
    /** one CallForwardInfo + serviceClassMask -> one line of text */
    protected CharSequence
    makeCFQueryResultMessage(CallForwardInfo info, int serviceClassMask) {
        CharSequence template;
        String sources[] = {"{0}", "{1}", "{2}"};
        CharSequence destinations[] = new CharSequence[3];
        boolean needTimeTemplate;

        // CF_REASON_NO_REPLY also has a time value associated with
        // it. All others don't.

        needTimeTemplate = (info.reason == CommandsInterface.CF_REASON_NO_REPLY
                && info.timeSeconds >=0);

        if (info.status == 1) {
            if (needTimeTemplate) {
                template = mContext.getText(
                        com.android.internal.R.string.cfTemplateForwardedTime);
            } else {
                template = mContext.getText(
                        com.android.internal.R.string.cfTemplateForwarded);
            }
        } else if (info.status == 0 && isEmptyOrNull(info.number)) {
            template = mContext.getText(
                        com.android.internal.R.string.cfTemplateNotForwarded);
        } else { /* (info.status == 0) && !isEmptyOrNull(info.number) */
            // A call forward record that is not active but contains
            // a phone number is considered "registered"

            if (needTimeTemplate) {
                template = mContext.getText(
                        com.android.internal.R.string.cfTemplateRegisteredTime);
            } else {
                template = mContext.getText(
                        com.android.internal.R.string.cfTemplateRegistered);
            }
        }

        // In the template (from strings.xmls)
        //         {0} is one of "bearerServiceCode*"
        //        {1} is dialing number
        //      {2} is time in seconds

        destinations[0] = serviceClassToCFString(info.serviceClass & serviceClassMask);
        destinations[1] = PhoneNumberUtils.stringFromStringAndTOA(info.number, info.toa);
        destinations[2] = Integer.toString(info.timeSeconds);

        if (info.reason == CommandsInterface.CF_REASON_UNCONDITIONAL &&
                (info.serviceClass & serviceClassMask)
                        == CommandsInterface.SERVICE_CLASS_VOICE) {
            boolean cffEnabled = (info.status == 1);
            if (mIccRecords != null) {
                mPhone.setVoiceCallForwardingFlag(1, cffEnabled, info.number);
            }
        }

        return TextUtils.replace(template, sources, destinations);
    }

    @Override
    protected CharSequence getMmiErrorMessage(AsyncResult ar) {
        if (ar.exception instanceof ImsException) {
            Rlog.d(LOG_TAG, "getMmiErrorMessage, ims error code = " +
                ((ImsException) ar.exception).getCode());
            switch (((ImsException) ar.exception).getCode()) {
                case ImsReasonInfo.CODE_FDN_BLOCKED:
                    return mContext.getText(com.android.internal.R.string.mmiFdnError);
                case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL:
                    return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_dial);
                case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_USSD:
                    return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_ussd);
                case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_SS:
                    return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_ss);
                case ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL_VIDEO:
                    return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_dial_video);
                case MtkImsReasonInfo.CODE_UT_XCAP_409_CONFLICT:
                    ImsException error = (ImsException) ar.exception;
                    String errorMsg = error.getMessage();
                    if (errorMsg != null && !errorMsg.isEmpty()) {
                        // Remove meaningless error code from ImsException message
                        // e.g Connection problem or invalid MMI code.(61449)
                        errorMsg = removeLastErrorCode(errorMsg);
                        Rlog.d(LOG_TAG, "Ims errorMessage = " + errorMsg);
                        return errorMsg;
                    } else {
                        return mContext.getText(com.android.internal.R.string.mmiError);
                    }
                default:
                    return mContext.getText(com.android.internal.R.string.mmiError);
            }
        } else if (ar.exception instanceof CommandException) {
            CommandException err = (CommandException) ar.exception;
            Rlog.d(LOG_TAG, "getMmiErrorMessage, error code = " + err.getCommandError());
            if (err.getCommandError() == CommandException.Error.FDN_CHECK_FAILURE) {
                return mContext.getText(com.android.internal.R.string.mmiFdnError);
            } else if (err.getCommandError() == CommandException.Error.SS_MODIFIED_TO_DIAL) {
                return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_dial);
            } else if (err.getCommandError() == CommandException.Error.SS_MODIFIED_TO_USSD) {
                return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_ussd);
            } else if (err.getCommandError() == CommandException.Error.SS_MODIFIED_TO_SS) {
                return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_ss);
            } else if (err.getCommandError() == CommandException.Error.SS_MODIFIED_TO_DIAL_VIDEO) {
                return mContext.getText(com.android.internal.R.string.stk_cc_ss_to_dial_video);
            } else if (err.getCommandError() == CommandException.Error.OEM_ERROR_25) {
                String errorMsg = err.getMessage();
                if (errorMsg != null && !errorMsg.isEmpty()) {
                    // Remove meaningless error code from ImsException message
                    // e.g Connection problem or invalid MMI code.(61449)
                    errorMsg = removeLastErrorCode(errorMsg);
                    Rlog.d(LOG_TAG, "errorMessage = " + errorMsg);
                    return errorMsg;
                } else {
                    return mContext.getText(com.android.internal.R.string.mmiError);
                }
            }
        }
        return mContext.getText(com.android.internal.R.string.mmiError);
    }

    @Override
    protected CharSequence getImsErrorMessage(AsyncResult ar) {
        ImsException error = (ImsException) ar.exception;
        CharSequence errorMessage;
        if ((errorMessage = getMmiErrorMessage(ar)) != null) {
            return errorMessage;
        } else if (error.getMessage() != null) {
            // Remove meaningless error code from ImsException message
            // e.g Connection problem or invalid MMI code.(61449)
            String errorMsg = removeLastErrorCode(error.getMessage());
            Rlog.d(LOG_TAG, "getImsErrorMessage, errorMsg = " + errorMsg);
            return errorMsg;
        } else {
            return getErrorMessage(ar);
        }
    }

    @Override
    protected CharSequence getErrorMessage(AsyncResult ar) {
        if (ar.exception instanceof CommandException) {
            CommandException err = (CommandException) ar.exception;
            String errorMsg = err.getMessage();
            if (err.getCommandError() == CommandException.Error.OEM_ERROR_1) {
                if (errorMsg != null && !errorMsg.isEmpty()) {
                    // Remove meaningless error code from ImsException message
                    // e.g Connection problem or invalid MMI code.(61449)
                    errorMsg = removeLastErrorCode(errorMsg);
                    Rlog.d(LOG_TAG, "getErrorMessage, errorMsg = " + errorMsg);
                    return errorMsg;
                }
            }
        }
        return super.getErrorMessage(ar);
    }

    private String removeLastErrorCode(CharSequence str) {
        String result = "";
        String reverse = new StringBuilder(str).reverse().toString();
        String pattern = "\\)[0-9]{5}\\("; // Number in brackets (Reverse)
        result = new StringBuilder(reverse.replaceFirst(pattern, "")).reverse().toString();
        return result;
    }

    protected static boolean
    isServiceCodeCallForwarding(String sc) {
        return sc != null &&
                (sc.equals(SC_CFU)
                || sc.equals(SC_CFB) || sc.equals(SC_CFNRy)
                || sc.equals(SC_CFNR) || sc.equals(SC_CF_All)
                || sc.equals(SC_CF_All_Conditional)
                || sc.equals(SC_CFNotRegister));
    }

    protected static int
    scToCallForwardReason(String sc) {
        if (sc == null) {
            throw new RuntimeException ("invalid call forward sc");
        }

        if (sc.equals(SC_CF_All)) {
           return CommandsInterface.CF_REASON_ALL;
        } else if (sc.equals(SC_CFU)) {
            return CommandsInterface.CF_REASON_UNCONDITIONAL;
        } else if (sc.equals(SC_CFB)) {
            return CommandsInterface.CF_REASON_BUSY;
        } else if (sc.equals(SC_CFNR)) {
            return CommandsInterface.CF_REASON_NOT_REACHABLE;
        } else if (sc.equals(SC_CFNRy)) {
            return CommandsInterface.CF_REASON_NO_REPLY;
        } else if (sc.equals(SC_CF_All_Conditional)) {
            return CommandsInterface.CF_REASON_ALL_CONDITIONAL;
        } else if (sc.equals(SC_CFNotRegister)) {
            return MtkRIL.CF_REASON_NOT_REGISTERED;
        } else {
            throw new RuntimeException ("invalid call forward sc");
        }
    }

    private boolean triggerMmiCodeCsfb(Message msg) {
        AsyncResult ar;
        if (supportMdAutoSetupIms() == false
                && !((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isNotSupportUtToCS()) {
            // for non 93 logic
            ar = (AsyncResult) msg.obj;
            if (ar != null && ar.exception != null) {
                if (ar.exception instanceof CommandException) {
                    CommandException cmdException = (CommandException) ar.exception;
                    if (cmdException.getCommandError()
                            == CommandException.Error.OPERATION_NOT_ALLOWED) {
                        Rlog.d(LOG_TAG, "handleMessage():"
                                + " CommandException.Error.UT_XCAP_403_FORBIDDEN");
                        ((MtkImsPhone)mPhone).handleMmiCodeCsfb(
                                MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN, this);
                        return true;
                    } else if (cmdException.getCommandError()
                            == CommandException.Error.OEM_ERROR_3) {
                        Rlog.d(LOG_TAG, "handleMessage(): CommandException.Error.UT_UNKNOWN_HOST");
                        ((MtkImsPhone)mPhone).handleMmiCodeCsfb(
                                MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, this);
                        return true;
                    }
                } else if (ar.exception instanceof ImsException) {
                    ImsException imsException = (ImsException) ar.exception;
                    if (imsException.getCode() == MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN) {
                        Rlog.d(LOG_TAG, "handleMessage():"
                                + " ImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN");
                        ((MtkImsPhone)mPhone).handleMmiCodeCsfb(
                                MtkImsReasonInfo.CODE_UT_XCAP_403_FORBIDDEN, this);
                        return true;
                    } else if (imsException.getCode() == MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST) {
                        Rlog.d(LOG_TAG, "handleMessage(): ImsReasonInfo.CODE_UT_UNKNOWN_HOST");
                        ((MtkImsPhone)mPhone).handleMmiCodeCsfb(
                                MtkImsReasonInfo.CODE_UT_UNKNOWN_HOST, this);
                        return true;
                    }
                }
            }
        }
        return false;
    }

    private boolean checkIfOPSupportCallerID() {
        if ((supportMdAutoSetupIms() == false) &&
                ((MtkGsmCdmaPhone) mPhone.mDefaultPhone).isOpNotSupportCallIdentity()) {
            handleGeneralError();
            return false;
        }
        return true;
    }

    private void handleGeneralError() {
        mState = State.FAILED;
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");
        sb.append(mContext.getText(
                com.android.internal.R.string.mmiError));
        mMessage = sb;
        mPhone.onMMIDone(this);
    }

    /**
     * Returns true if the Service Code is FAC to dial as a normal call.
     *
     * FAC stands for feature access code and it is special patterns of characters
     * to invoke certain features.
     */
    private boolean isFacToDial() {
        CarrierConfigManager configManager = (CarrierConfigManager)
                mPhone.getContext().getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = configManager.getConfigForSubId(mPhone.getSubId());
        if (b != null) {
            String[] dialFacList = b.getStringArray(CarrierConfigManager
                    .KEY_FEATURE_ACCESS_CODES_STRING_ARRAY);
            if (!ArrayUtils.isEmpty(dialFacList)) {
                for (String fac : dialFacList) {
                    if (fac.equals(mSc)) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder("MtkImsPhoneMmiCode {");

        sb.append("State=" + getState());
        if (mAction != null) sb.append(" action=" + mAction);
        if (mSc != null) sb.append(" sc=" + mSc);
        if (mSia != null) sb.append(" sia=" + MtkSuppServHelper.encryptString(mSia));
        if (mSib != null) sb.append(" sib=" + MtkSuppServHelper.encryptString(mSib));
        if (mSic != null) sb.append(" sic=" + MtkSuppServHelper.encryptString(mSic));
        if (mPoundString != null) sb.append(" poundString=" + Rlog.pii(LOG_TAG, mPoundString));
        if (mDialingNumber != null) sb.append(" dialingNumber="
                + Rlog.pii(LOG_TAG, mDialingNumber));
        if (mPwd != null) sb.append(" pwd=" + Rlog.pii(LOG_TAG, mPwd));
        if (mCallbackReceiver != null) sb.append(" hasReceiver");
        sb.append("}");
        return sb.toString();
    }
}
