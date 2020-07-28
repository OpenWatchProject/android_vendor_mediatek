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
import android.content.res.Resources;
import com.android.internal.telephony.*;
import com.android.internal.telephony.gsm.*;
import com.android.internal.telephony.gsm.GsmMmiCode;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;

import android.os.*;
import android.telephony.PhoneNumberUtils;
import android.text.SpannableStringBuilder;
import android.text.BidiFormatter;
import android.text.TextDirectionHeuristics;
import android.text.TextUtils;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

import static com.android.internal.telephony.CommandsInterface.*;

import java.util.Arrays;
import java.util.Random;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

import com.mediatek.internal.telephony.MtkGsmCdmaPhone;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.internal.telephony.MtkRIL;
import com.mediatek.internal.telephony.MtkSSRequestDecisionMaker;
import com.mediatek.internal.telephony.MtkSuppServManager;
import com.mediatek.internal.telephony.MtkSuppServHelper;

import static com.mediatek.internal.telephony.MtkTelephonyProperties.PROPERTY_TBCW_MODE;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_DISABLED;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_OFF;
import static com.mediatek.internal.telephony.MtkTelephonyProperties.TBCW_ON;

/**
 * {@hide}
 */
public final class MtkGsmMmiCode extends GsmMmiCode {
    static final String LOG_TAG = "MtkGsmMmiCode";
    private static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");

    /* Call Name Presentation (CNAP) */
    static final String SC_CNAP = "300";
    static final String CNAPMmi = "Calling Name Presentation";

    /* Connected Line Presentation */
    static final String SC_COLP = "76";
    static final String SC_COLR = "77";

    /* Event for COLP and COLR */
    static final int EVENT_GET_COLR_COMPLETE = 8;
    static final int EVENT_GET_COLP_COMPLETE = 9;

    MtkGsmCdmaPhone mPhone;

    private MtkSSRequestDecisionMaker mMtkSSReqDecisionMaker;
    /* MTK
     * Fix ALPS01471897
     */
    private boolean mUserInitiatedMMI = false;

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
    public static MtkGsmMmiCode newFromDialString(String dialString, MtkGsmCdmaPhone phone,
            UiccCardApplication app) {
        return newFromDialString(dialString, phone, app, null);
    }

    public static MtkGsmMmiCode newFromDialString(String dialString, MtkGsmCdmaPhone phone,
            UiccCardApplication app, ResultReceiver wrappedCallback) {
        Matcher m;
        MtkGsmMmiCode ret = null;
        Rlog.d(LOG_TAG, "newFromDialString, dialstring = " + MtkSuppServHelper.encryptString(dialString));

        /// To improve call setup performace @{
        String dialPart = PhoneNumberUtils.extractNetworkPortionAlt(PhoneNumberUtils.
                stripSeparators(dialString));
        boolean isMmi = dialPart.startsWith("*") || dialPart.startsWith("#") ||
                        dialPart.endsWith("#");
        if (!(isMmi || dialPart.length() <= 2)) {
            Rlog.d(LOG_TAG, "Not belong to MMI format.");
            return null;
        }
        /// @}

        if (phone.getServiceState().getVoiceRoaming()
                && phone.supportsConversionOfCdmaCallerIdMmiCodesWhileRoaming()) {
            /* The CDMA MMI coded dialString will be converted to a 3GPP MMI Coded dialString
             * so that it can be processed by the matcher and code below */
            dialString = convertCdmaMmiCodesTo3gppMmiCodes(dialString);
        }

        m = sPatternSuppService.matcher(dialString);

        // Is this formatted like a standard supplementary service code?
        if (m.matches()) {
            ret = new MtkGsmMmiCode(phone, app);
            ret.mPoundString = makeEmptyNull(m.group(MATCH_GROUP_POUND_STRING));
            ret.mAction = makeEmptyNull(m.group(MATCH_GROUP_ACTION));
            ret.mSc = makeEmptyNull(m.group(MATCH_GROUP_SERVICE_CODE));
            ret.mSia = makeEmptyNull(m.group(MATCH_GROUP_SIA));
            ret.mSib = makeEmptyNull(m.group(MATCH_GROUP_SIB));
            ret.mSic = makeEmptyNull(m.group(MATCH_GROUP_SIC));
            ret.mPwd = makeEmptyNull(m.group(MATCH_GROUP_PWD_CONFIRM));
            ret.mDialingNumber = makeEmptyNull(m.group(MATCH_GROUP_DIALING_NUMBER));

            if(ret.mDialingNumber != null &&
                    ret.mDialingNumber.endsWith("#") &&
                    dialString.endsWith("#")){
                // According to TS 22.030 6.5.2 "Structure of the MMI",
                // the dialing number should not ending with #.
                // The dialing number ending # is treated as unique USSD,
                // eg, *400#16 digit number# to recharge the prepaid card
                // in India operator(Mumbai MTNL)
                ret = new MtkGsmMmiCode(phone, app);
                ret.mPoundString = dialString;
            } else if (ret.isFacToDial()) {
                // This is a FAC (feature access code) to dial as a normal call.
                ret = null;
            }
        } else if (dialString.endsWith("#")) {
            // TS 22.030 sec 6.5.3.2
            // "Entry of any characters defined in the 3GPP TS 23.038 [8] Default Alphabet
            // (up to the maximum defined in 3GPP TS 24.080 [10]), followed by #SEND".
            ret = new MtkGsmMmiCode(phone, app);
            ret.mPoundString = dialString;
        } else if (isTwoDigitShortCode(phone.getContext(), dialString)) {
            //Is a country-specific exception to short codes as defined in TS 22.030, 6.5.3.2
            ret = null;
        } else if (isShortCode(dialString, phone)) {
            // this may be a short code, as defined in TS 22.030, 6.5.3.2
            ret = new MtkGsmMmiCode(phone, app);
            ret.mDialingNumber = dialString;
        }

        if (ret != null) {
            ret.mCallbackReceiver = wrappedCallback;
        }

        return ret;
    }

    public static MtkGsmMmiCode newFromUssdUserInput(String ussdMessge,
                                                  MtkGsmCdmaPhone phone,
                                                  UiccCardApplication app) {
        MtkGsmMmiCode ret = new MtkGsmMmiCode(phone, app);

        ret.mMessage = ussdMessge;
        ret.mState = State.PENDING;
        ret.mIsPendingUSSD = true;

        return ret;
    }

    private static int siToServiceClass(String si) {
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
/*
    Note for code 20:
     From TS 22.030 Annex C:
                "All GPRS bearer services" are not included in "All tele and bearer services"
                    and "All bearer services"."
....so SERVICE_CLASS_DATA, which (according to 27.007) includes GPRS
*/
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

    public MtkGsmMmiCode(MtkGsmCdmaPhone phone, UiccCardApplication app) {
        super(phone, app);
        mPhone = phone;  // Cast mPhone to a MtkGsmMmiCode object

        mMtkSSReqDecisionMaker = phone.getMtkSSRequestDecisionMaker();
    }

    /* For ALPS01471897 */
    public void setUserInitiatedMMI(boolean userinit) {
       mUserInitiatedMMI = userinit;
    }

    public boolean getUserInitiatedMMI() {
       return mUserInitiatedMMI;
    }

    /** Process a MMI code or short code...anything that isn't a dialing number */
    @Override
    public void processCode() throws CallStateException {
        try {
            if (isShortCode()) {
                Rlog.d(LOG_TAG, "isShortCode");
                // These just get treated as USSD.
                sendUssd(mDialingNumber);
            } else if (mDialingNumber != null) {
                Rlog.w(LOG_TAG, "Special USSD Support:" + mPoundString + mDialingNumber);
                sendUssd(mPoundString + mDialingNumber);
            } else if (mSc != null && mSc.equals(SC_CNAP) && isInterrogate()) {
                if (mPoundString != null) {
                    handleCNAP(mPoundString);
                }
            } else if (mSc != null && mSc.equals(SC_CLIP)) {
                handleCLIP();
            } else if (mSc != null && mSc.equals(SC_CLIR)) {
                handleCLIR();
            } else if (mSc != null && mSc.equals(SC_COLP)) {
                handleCOLP();
            } else if (mSc != null && mSc.equals(SC_COLR)) {
                handleCOLR();
            } else if (isServiceCodeCallForwarding(mSc)) {
                handleCallForward();
            } else if (isServiceCodeCallBarring(mSc)) {
                handleCallBarring();
            } else if (mSc != null && mSc.equals(SC_PWD)) {
                handleChangeBarringPassward();
            } else if (mSc != null && mSc.equals(SC_WAIT)) {
                handleCW();
            } else if (isPinPukCommand()) {
                // TODO: This is the same as the code in CmdaMmiCode.java,
                // MmiCode should be an abstract or base class and this and
                // other common variables and code should be promoted.

                // sia = old PIN or PUK
                // sib = new PIN
                // sic = new PIN
                String oldPinOrPuk = mSia;
                String newPinOrPuk = mSib;
                int pinLen = newPinOrPuk.length();
                if (isRegister()) {
                    if (!newPinOrPuk.equals(mSic)) {
                        // password mismatch; return error
                        handlePasswordError(com.android.internal.R.string.mismatchPin);
                    } else if (pinLen < 4 || pinLen > 8 ) {
                        // invalid length
                        handlePasswordError(com.android.internal.R.string.invalidPin);
                    } else if (mSc.equals(SC_PIN)
                            && mUiccApplication != null
                            && mUiccApplication.getState() == AppState.APPSTATE_PUK) {
                        // Sim is puk-locked
                        handlePasswordError(com.android.internal.R.string.needPuk);
                    } else if (mUiccApplication != null) {
                        Rlog.d(LOG_TAG, "process mmi service code using UiccApp sc=" + mSc);

                        // We have an app and the pre-checks are OK
                        if (mSc.equals(SC_PIN)) {
                            mUiccApplication.changeIccLockPassword(oldPinOrPuk, newPinOrPuk,
                                    obtainMessage(EVENT_SET_COMPLETE, this));
                        } else if (mSc.equals(SC_PIN2)) {
                            mUiccApplication.changeIccFdnPassword(oldPinOrPuk, newPinOrPuk,
                                    obtainMessage(EVENT_SET_COMPLETE, this));
                        } else if (mSc.equals(SC_PUK)) {
                            mUiccApplication.supplyPuk(oldPinOrPuk, newPinOrPuk,
                                    obtainMessage(EVENT_SET_COMPLETE, this));
                        } else if (mSc.equals(SC_PUK2)) {
                            mUiccApplication.supplyPuk2(oldPinOrPuk, newPinOrPuk,
                                    obtainMessage(EVENT_SET_COMPLETE, this));
                        } else {
                            throw new RuntimeException("uicc unsupported service code=" + mSc);
                        }
                    } else {
                        throw new RuntimeException("No application mUiccApplicaiton is null");
                    }
                } else {
                    throw new RuntimeException ("Ivalid register/action=" + mAction);
                }
            } else if (mPoundString != null) {
                if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                    mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                }
                sendUssd(mPoundString);
            } else {
                throw new RuntimeException ("Invalid or Unsupported MMI Code");
            }
        } catch (RuntimeException exc) {
            mState = State.FAILED;
            exc.printStackTrace();
            Rlog.d(LOG_TAG, "exc.toString() = " + exc.toString());
            Rlog.d(LOG_TAG, "procesCode: mState = FAILED");
            mMessage = mContext.getText(com.android.internal.R.string.mmiError);
            mPhone.onMMIDone(this);
        }
    }

    /* Query CNAP */
    void handleCNAP(String cnapssMessage) {
        Rlog.d(LOG_TAG, "processCode: is CNAP");
        /* Note that unlike most everything else, the USSD complete
         * response does not complete this MMI code...we wait for
         * an unsolicited USSD "Notify" or "Request".
         * The matching up of this is done in GsmCdmaPhone.
         */
         if (isInterrogate()) {
            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            /* For current design, the "cnapssMessage"" is not used. But we keep the same
             * RIL interface for CNAP as before. So if we pass the cnap string on demand
             * in the future, we don't need to change the HIDL interface'
             */
            mPhone.mMtkCi.sendCNAP(cnapssMessage, obtainMessage(EVENT_QUERY_COMPLETE, this));
         } else {
             throw new RuntimeException ("Invalid or Unsupported MMI Code");
         }
    }

    /* Set CLIP & Query CLIP*/
    void handleCLIP() {
        Rlog.d(LOG_TAG, "processCode: is CLIP");
        if (isActivate() || isDeactivate()) {
            int clipEnable = isActivate() ? 1 : 0;

            if (supportMdAutoSetupIms()) {
                mPhone.mMtkCi.setCLIP(clipEnable, obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.setCLIP(clipEnable,
                        obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mMtkCi.setCLIP(clipEnable, obtainMessage(EVENT_SET_COMPLETE, this));
        } else if (isInterrogate()) {

            if (supportMdAutoSetupIms()) {
                mPhone.mMtkCi.queryCLIP(obtainMessage(EVENT_QUERY_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.getCLIP(obtainMessage(EVENT_QUERY_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mCi.queryCLIP(obtainMessage(EVENT_QUERY_COMPLETE, this));
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Set CLIR & Query CLIR */
    void handleCLIR() {
        Rlog.d(LOG_TAG, "processCode: is CLIR");
        if (isActivate() || isDeactivate()) {
            int clirAction = isActivate() ?
                    CommandsInterface.CLIR_INVOCATION : CommandsInterface.CLIR_SUPPRESSION;

            if (supportMdAutoSetupIms()) {
                mPhone.mCi.setCLIR(clirAction, obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if (mPhone.isOpTbClir()) {
                mPhone.mCi.setCLIR(clirAction, obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.setCLIR(CommandsInterface.CLIR_INVOCATION,
                        obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mCi.setCLIR(clirAction, obtainMessage(EVENT_SET_COMPLETE, this));
        } else if (isInterrogate()) {

            if (supportMdAutoSetupIms()) {
                mPhone.mCi.getCLIR(obtainMessage(EVENT_GET_CLIR_COMPLETE, this));
                return;
            }

            if (mPhone.isOpTbClir()) {
                mPhone.mCi.getCLIR(obtainMessage(EVENT_GET_CLIR_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.getCLIR(obtainMessage(EVENT_GET_CLIR_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mCi.getCLIR(obtainMessage(EVENT_GET_CLIR_COMPLETE, this));
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Query COLP, SET COLP as *76# or #76# is not supported */
    void handleCOLP() {
        Rlog.d(LOG_TAG, "processCode: is COLP");
        if (isInterrogate()) {
            if (supportMdAutoSetupIms()) {
                mPhone.mMtkCi.getCOLP(obtainMessage(EVENT_GET_COLP_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.getCOLP(obtainMessage(EVENT_GET_COLP_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mMtkCi.getCOLP(obtainMessage(EVENT_GET_COLP_COMPLETE, this));
        } else {
            throw new RuntimeException("Invalid or Unsupported MMI Code");
        }
    }

    /* Query COLR, SET COLR as *76# or #76# is not supported  */
    void handleCOLR() {
        Rlog.d(LOG_TAG, "processCode: is COLR");
        if (isInterrogate()) {
            if (supportMdAutoSetupIms()) {
                mPhone.mMtkCi.getCOLR(obtainMessage(EVENT_GET_COLR_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.getCOLR(obtainMessage(EVENT_GET_COLR_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mMtkCi.getCOLR(obtainMessage(EVENT_GET_COLR_COMPLETE, this));
        } else {
            throw new RuntimeException("Invalid or Unsupported MMI Code");
        }
    }

    /* Query Call Forward & Set Call Forward*/
    void handleCallForward() {
        Rlog.d(LOG_TAG, "processCode: is CF");
        String dialingNumber = mSia;
        int serviceClass     = siToServiceClass(mSib);
        int reason           = scToCallForwardReason(mSc);
        int time             = siToTime(mSic);

        if (isInterrogate()) {

            if (supportMdAutoSetupIms()) {
                mPhone.mCi.queryCallForwardStatus(reason, serviceClass, dialingNumber,
                        obtainMessage(EVENT_QUERY_CF_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.queryCallForwardStatus(reason, serviceClass,
                        dialingNumber, obtainMessage(EVENT_QUERY_CF_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mCi.queryCallForwardStatus(reason, serviceClass, dialingNumber,
                    obtainMessage(EVENT_QUERY_CF_COMPLETE, this));
        } else {
            int cfAction;
            if (isActivate()) {
                /* 3GPP TS 22.030 6.5.2
                 * a call forwarding request with a single * would be
                 * interpreted as registration if containing a forwarded-to
                 * number, or an activation if not
                 */
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

            int includeCFU = isVoiceUnconditionalForwarding(reason, serviceClass) ? 1 : 0;
            int isEnableDesired = ((cfAction == CommandsInterface.CF_ACTION_ENABLE) ||
                    (cfAction == CommandsInterface.CF_ACTION_REGISTRATION)) ? 1 : 0;

            Rlog.d(LOG_TAG, "is CF setCallForward");
            if (supportMdAutoSetupIms()) {
                mPhone.mCi.setCallForward(cfAction, reason, serviceClass,
                        dialingNumber, time, obtainMessage(
                                EVENT_SET_CFF_COMPLETE,
                                includeCFU,
                                isEnableDesired, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.setCallForward(cfAction, reason, serviceClass,
                    dialingNumber, time, obtainMessage(
                            EVENT_SET_CFF_COMPLETE,
                            includeCFU,
                            isEnableDesired, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }
            mPhone.mCi.setCallForward(cfAction, reason, serviceClass,
                    dialingNumber, time, obtainMessage(
                            EVENT_SET_CFF_COMPLETE,
                            includeCFU,
                            isEnableDesired, this));
        }
    }

    /* Query Call Barring & Set Call Barring */
    void handleCallBarring() {
        Rlog.d(LOG_TAG, "processCode: is CB");
        String password  = mSia;
        int serviceClass = siToServiceClass(mSib);
        String facility  = scToBarringFacility(mSc);

        if (isInterrogate()) {
            if (supportMdAutoSetupIms()) {
                mPhone.mCi.queryFacilityLock(facility, password, serviceClass,
                        obtainMessage(EVENT_QUERY_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.queryFacilityLock(facility, password,
                        serviceClass, obtainMessage(EVENT_QUERY_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /* If UiccApplication is not ready, for example, quickly switch flight mode.
            * We should not execute call barring. Just return GENERIC FAILURE.
            */
            if (mPhone.getUiccCardApplication() == null) {
                Rlog.d(LOG_TAG, "handleCallBarring: getUiccCardApplication() == null");

                Message msg = obtainMessage(EVENT_QUERY_COMPLETE, this);
                CommandException ce = new CommandException(
                        CommandException.Error.GENERIC_FAILURE);
                AsyncResult.forMessage(msg, null, ce);
                msg.sendToTarget();
                return;
            }

            mPhone.mCi.queryFacilityLockForApp(facility, password, serviceClass,
                    mPhone.getUiccCardApplication().getAid(),
                    obtainMessage(EVENT_QUERY_COMPLETE, this));
        } else if (isActivate() || isDeactivate()) {
            if (supportMdAutoSetupIms()) {
                mPhone.mCi.setFacilityLock(facility, isActivate(), password,
                        serviceClass, obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED) &&
                    mPhone.isGsmUtSupport()) {
                mMtkSSReqDecisionMaker.setFacilityLock(facility, isActivate(), password,
                        serviceClass, obtainMessage(EVENT_SET_COMPLETE, this));
                return;
            }

            if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
            }

            /* If UiccApplication is not ready, for example, quickly switch flight mode.
            * We should not execute call barring. Just return GENERIC FAILURE.
            */
            if (mPhone.getUiccCardApplication() == null) {
                Rlog.d(LOG_TAG, "handleCallBarring: getUiccCardApplication() == null");

                Message msg = obtainMessage(EVENT_SET_COMPLETE, this);
                CommandException ce = new CommandException(
                        CommandException.Error.GENERIC_FAILURE);
                AsyncResult.forMessage(msg, null, ce);
                msg.sendToTarget();
                return;
            }

            mPhone.mCi.setFacilityLockForApp(facility, isActivate(), password,
                    serviceClass, mPhone.getUiccCardApplication().getAid(),
                    obtainMessage(EVENT_SET_COMPLETE, this));
        } else {
            throw new RuntimeException("Invalid or Unsupported MMI Code");
        }
    }

    /* Change Call Barring Password */
    void handleChangeBarringPassward() {
        Rlog.d(LOG_TAG, "processCode: is Change PWD");
        String facility;
        String oldPwd = mSib;
        String newPwd = mSic;
        if (isActivate() || isRegister()) {
            /* Even though ACTIVATE is acceptable, this is really termed a REGISTER */
            mAction = ACTION_REGISTER;

            if (mSia == null) {
                /* If sc was not specified, treat it as BA_ALL. */
                facility = CommandsInterface.CB_FACILITY_BA_ALL;
            } else {
                facility = scToBarringFacility(mSia);
            }

            /* Check password in network side */
            if ((oldPwd != null) && (newPwd != null) && (mPwd != null)) {
                if ((mPwd.length() != newPwd.length()) ||
                    (oldPwd.length() != 4) || (mPwd.length() != 4)) {
                    handlePasswordError(com.android.internal.R.string.passwordIncorrect);
                } else {
                    if (mPhone.isDuringImsCall()) {
                        Message msg = obtainMessage(EVENT_SET_COMPLETE, this);
                        CommandException ce = new CommandException(
                                CommandException.Error.GENERIC_FAILURE);
                        AsyncResult.forMessage(msg, null, ce);
                        msg.sendToTarget();
                    } else {
                        /* From test spec 51.010-1 31.8.1.2.3,
                         * we shall not compare pwd here. Let pwd check in NW side.
                         */
                        mPhone.mMtkCi.changeBarringPassword(facility, oldPwd, newPwd,
                                mPwd, obtainMessage(EVENT_SET_COMPLETE, this));
                    }
                }
            } else {
                /* password mismatch; return error */
                handlePasswordError(com.android.internal.R.string.passwordIncorrect);
            }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /* Query Call Waiting & Set Call Waiting */
    void handleCW() {
        Rlog.d(LOG_TAG, "processCode: is CW");
        int serviceClass = siToServiceClass(mSia);
        Rlog.d(LOG_TAG, "CW serviceClass = " + serviceClass);

        if (supportMdAutoSetupIms()) {
            if (isActivate() || isDeactivate()) {
                mPhone.mCi.setCallWaiting(isActivate(), serviceClass,
                        obtainMessage(EVENT_SET_COMPLETE, this));
            } else if (isInterrogate()) {
                mPhone.mCi.queryCallWaiting(serviceClass,
                        obtainMessage(EVENT_QUERY_COMPLETE, this));
            } else {
                throw new RuntimeException ("Invalid or Unsupported MMI Code");
            }
            return;
        }

        int tbcwMode = mPhone.getTbcwMode();

        if (isActivate() || isDeactivate()) {
            if ((tbcwMode == MtkGsmCdmaPhone.TBCW_VOLTE_USER) && !mPhone.isOpNwCW()) {
                /* Terminal-based Call Waiting */
                String tbcwStatus = TelephonyManager.getTelephonyProperty(
                        mPhone.getPhoneId(),
                        PROPERTY_TBCW_MODE,
                        TBCW_DISABLED);
                Rlog.d(LOG_TAG, "setTerminal-based CallWaiting(): tbcwStatus = " + tbcwStatus
                        + ", enable = " + isActivate());
                if (!tbcwStatus.equals(TBCW_DISABLED)) {
                    mPhone.setTerminalBasedCallWaiting(isActivate(),
                            obtainMessage(EVENT_SET_COMPLETE, this));
                } else {
                    Rlog.d(LOG_TAG, "setCallWaiting() by NW.");
                    mPhone.mCi.setCallWaiting(isActivate(), serviceClass,
                                obtainMessage(EVENT_SET_COMPLETE, this));
                }
            } else if (tbcwMode == MtkGsmCdmaPhone.TBCW_NOT_VOLTE_USER
                    || tbcwMode == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                    mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                }
                mPhone.mCi.setCallWaiting(isActivate(), serviceClass,
                        obtainMessage(EVENT_SET_COMPLETE, isActivate() ? 1 : 0, -1, this));
            } else {
                Rlog.d(LOG_TAG, "processCode setCallWaiting");

                if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && mPhone.isGsmUtSupport()) {
                    mMtkSSReqDecisionMaker.setCallWaiting(isActivate(), serviceClass,
                            obtainMessage(EVENT_SET_COMPLETE, this));
                } else {
                    mPhone.mCi.setCallWaiting(isActivate(), serviceClass,
                            obtainMessage(EVENT_SET_COMPLETE, this));
                }
            }
        } else if (isInterrogate()) {
            if ((tbcwMode == MtkGsmCdmaPhone.TBCW_VOLTE_USER) &&
                !mPhone.isOpNwCW()) {
                    mPhone.getTerminalBasedCallWaiting(obtainMessage(EVENT_QUERY_COMPLETE, this));
            } else if (tbcwMode == MtkGsmCdmaPhone.TBCW_NOT_VOLTE_USER ||
                       tbcwMode == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                if (mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_ONCE) {
                    mPhone.setCsFallbackStatus(MtkPhoneConstants.UT_CSFB_PS_PREFERRED);
                }
                mPhone.mCi.queryCallWaiting(serviceClass,
                        obtainMessage(EVENT_QUERY_COMPLETE, this));
            } else {
                Rlog.d(LOG_TAG, "processCode getCallWaiting");

                if ((mPhone.getCsFallbackStatus() == MtkPhoneConstants.UT_CSFB_PS_PREFERRED)
                        && mPhone.isGsmUtSupport()) {
                    mMtkSSReqDecisionMaker.queryCallWaiting(serviceClass,
                            obtainMessage(EVENT_QUERY_COMPLETE, this));
                } else {
                    mPhone.mCi.queryCallWaiting(serviceClass, obtainMessage(
                            EVENT_QUERY_COMPLETE, this));
                }
            }
        } else {
            throw new RuntimeException ("Invalid or Unsupported MMI Code");
        }
    }

    /** Called from GsmCdmaPhone.handleMessage; not a Handler subclass */
    @Override
    public void handleMessage (Message msg) {
        AsyncResult ar;

        switch (msg.what) {
            case EVENT_SET_COMPLETE:
                ar = (AsyncResult) (msg.obj);

                if (!supportMdAutoSetupIms() && mSc.equals(SC_WAIT)
                    && mPhone.getTbcwMode() == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                    if (ar.exception == null) {
                        boolean enable = (msg.arg1 == 1) ? true : false;
                        mPhone.setTerminalBasedCallWaiting(enable, null);
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
                if ((ar.exception == null) && (msg.arg1 == 1)) {
                    boolean cffEnabled = (msg.arg2 == 1);
                    if (mIccRecords != null) {
                        mPhone.setVoiceCallForwardingFlag(1, cffEnabled, mDialingNumber);
                        mPhone.saveTimeSlot(null);
                    }
                }

                onSetComplete(msg, ar);
                break;

            case EVENT_GET_COLP_COMPLETE:
                ar = (AsyncResult) (msg.obj);
                onGetColpComplete(ar);
                break;

            case EVENT_GET_COLR_COMPLETE:
                ar = (AsyncResult) (msg.obj);
                onGetColrComplete(ar);
                break;

            case EVENT_QUERY_COMPLETE:
                ar = (AsyncResult) (msg.obj);
                if (!supportMdAutoSetupIms() && mSc.equals(SC_WAIT) &&
                    mPhone.getTbcwMode() == MtkGsmCdmaPhone.TBCW_WITH_CS) {
                    Rlog.d(LOG_TAG, "TBCW_WITH_CS");
                    if (ar.exception == null) {
                        int[] cwArray = (int[]) ar.result;
                        // If cwArray[0] is = 1, then cwArray[1] must follow,
                        // with the TS 27.007 service class bit vector of services
                        // for which call waiting is enabled.
                        try {
                            Rlog.d(LOG_TAG, "EVENT_GET_CALL_WAITING_FOR_CS_TB"
                                    + " cwArray[0]:cwArray[1] = "
                                    + cwArray[0] + ":" + cwArray[1]);

                            boolean csEnable = ((cwArray[0] == 1) &&
                                    ((cwArray[1] & 0x01) == SERVICE_CLASS_VOICE));
                            mPhone.setTerminalBasedCallWaiting(csEnable, null);
                        } catch (ArrayIndexOutOfBoundsException e) {
                            Rlog.e(LOG_TAG, "EVENT_GET_CALL_WAITING_FOR_CS_TB:"
                                    + " improper result: err ="
                                    + e.getMessage());
                        }
                    }
                }
                onQueryComplete(ar);
                break;

            default:
                // If the msg.what is not in above cases, then go to AOSP's switch case
                super.handleMessage(msg);
                break;
        }
    }

    @Override
    protected void onSetComplete(Message msg, AsyncResult ar){
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception != null) {
            mState = State.FAILED;
            if (ar.exception instanceof CommandException) {
                CommandException.Error err = ((CommandException)(ar.exception)).getCommandError();
                if (err == CommandException.Error.REQUEST_NOT_SUPPORTED) {
                    if (mSc.equals(SC_CLIR) || mSc.equals(SC_CLIP)) {
                        /* If VoLTE is registered, show "IMS UT exception， code=XXX".
                         * If VoLTE is not registered, show "Connection error or invalid MMI".
                         */
                        sb.append(mContext.getText(com.android.internal.R.string.mmiError));
                        mMessage = sb;
                        mPhone.onMMIDone(this);
                        return;
                    }
                }
            }
        }
        super.onSetComplete(msg, ar);
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

    /** one CallForwardInfo + serviceClassMask -> one line of text */
    @Override
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
    protected void onQueryCfComplete(AsyncResult ar) {
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception != null) {
            super.onQueryCfComplete(ar);
            return;
        } else {
            CallForwardInfo infos[];

            infos = (CallForwardInfo[]) ar.result;

            if (infos.length == 0) {
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
                            tb.append(makeCFQueryResultMessage(infos[i], serviceClassMask));
                            tb.append("\n");

                            /* Fix ALPS03481099, Update CFU Icon and System Property */
                            if (infos[i].reason == CommandsInterface.CF_REASON_UNCONDITIONAL &&
                                    (infos[i].serviceClass & serviceClassMask) ==
                                    CommandsInterface.SERVICE_CLASS_VOICE) {
                                if (mIccRecords != null) {
                                    mPhone.setVoiceCallForwardingFlag(1,
                                            infos[i].status == 1, null);
                                }
                            }
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
    protected void onQueryComplete(AsyncResult ar) {
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception == null) {
            int[] ints = (int[])ar.result;
            if (ints.length != 0) {
                if (ints[0] != 0 && mSc.equals(SC_CNAP)) {
                    // M: For query CNAP
                    Rlog.d(LOG_TAG, "onQueryComplete_CNAP");
                    sb.append(createQueryCnapResultMessage(ints[1]));
                    mMessage = sb;
                    mState = State.COMPLETE;
                    mPhone.onMMIDone(this);
                    return;
                }
            }
        }
        super.onQueryComplete(ar);
    }

    @Override
    protected CharSequence createQueryCallWaitingResultMessage(int serviceClass) {
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

    @Override
    protected CharSequence createQueryCallBarringResultMessage(int serviceClass)
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

    private CharSequence createQueryCnapResultMessage(int serviceProvisioned) {
        Rlog.d(LOG_TAG, "createQueryCnapResultMessage");
        StringBuilder sb = new StringBuilder(
                mContext.getText(com.android.internal.R.string.serviceEnabledFor));
        sb.append("\n");

        switch (serviceProvisioned) {
            case 0: // CNAP not provisioned
                sb.append(mContext.getText(
                            com.android.internal.R.string.serviceNotProvisioned));
                break;
            case 1: // CNAP provisioned
                sb.append(mContext.getText(
                            com.mediatek.R.string.serviceProvisioned));
                break;
            default:
                sb.append(mContext.getText(
                            com.android.internal.R.string.serviceNotProvisioned));
                break;
        }
        Rlog.d(LOG_TAG, "CNAP_sb = " + sb);
        return sb;
    }

    public static MtkGsmMmiCode
    newNetworkInitiatedUssdError(String ussdMessage, boolean isUssdRequest,
            MtkGsmCdmaPhone phone, UiccCardApplication app) {
        MtkGsmMmiCode ret;
        ret = new MtkGsmMmiCode(phone, app);
        if (ussdMessage != null && ussdMessage.length() > 0) {
            ret.mMessage = ussdMessage;
        } else {
            ret.mMessage = ret.mContext.getText(com.android.internal.R.string.mmiError);
        }
        ret.mIsUssdRequest = isUssdRequest;
        ret.mState = State.FAILED;

        return ret;
    }

    private void onGetColpComplete(AsyncResult ar) {
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception != null) {
            mState = State.FAILED;
            sb.append(getErrorMessage(ar));
        } else {
            int colpArgs[];
            colpArgs = (int[]) ar.result;

            // the 'm' parameter from TS 27.007 7.8
            switch (colpArgs[1]) {
                case 0: // COLP not provisioned
                    sb.append(mContext.getText(
                                com.android.internal.R.string.serviceNotProvisioned));
                    mState = State.COMPLETE;
                    break;
                case 1: // COLP provisioned
                    sb.append(mContext.getText(
                                com.mediatek.R.string.serviceProvisioned));
                    mState = State.COMPLETE;
                    break;
                case 2: // unknown (e.g. no network, etc.)
                    sb.append(mContext.getText(
                                com.mediatek.R.string.serviceUnknown));
                    mState = State.COMPLETE;
                    break;
            }
        }
        mMessage = sb;
        mPhone.onMMIDone(this);
    }

    private void onGetColrComplete(AsyncResult ar) {
        StringBuilder sb = new StringBuilder(getScString());
        sb.append("\n");

        if (ar.exception != null) {
            mState = State.FAILED;
            sb.append(getErrorMessage(ar));
        } else {
            int colrArgs[];
            colrArgs = (int[]) ar.result;

            // the 'm' parameter from mtk proprietary
            switch (colrArgs[0]) {
                case 0: // COLR not provisioned
                    sb.append(mContext.getText(
                                com.android.internal.R.string.serviceNotProvisioned));
                    mState = State.COMPLETE;
                break;

                case 1: // COLR provisioned
                    sb.append(mContext.getText(
                                com.mediatek.R.string.serviceProvisioned));
                    mState = State.COMPLETE;
                break;

                case 2: // unknown (e.g. no network, etc.)
                    sb.append(mContext.getText(
                                com.android.internal.R.string.mmiError));
                    mState = State.FAILED;
                break;

            }
        }

        mMessage = sb;
        mPhone.onMMIDone(this);
    }

    public static boolean
    isUtMmiCode(String dialString, MtkGsmCdmaPhone dialPhone, UiccCardApplication iccApp) {

        MtkGsmMmiCode mmi = MtkGsmMmiCode.newFromDialString(dialString, dialPhone, iccApp);
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
                )) {
            return true;
        }
        return false;
    }

    private boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }

    @Override
    protected CharSequence getScString() {
        if (mSc != null && mSc.equals(SC_CNAP)) {
            return CNAPMmi;
        } else {
            return super.getScString();
        }
    }

    @Override
    protected CharSequence getErrorMessage(AsyncResult ar) {
        if (ar.exception instanceof CommandException) {
            CommandException.Error err = ((CommandException)(ar.exception)).getCommandError();
            /* MTK has reserved OEM_ERROR_25 for HTTP 409 conflict error,
               so change AOSP error code */
            if (err == CommandException.Error.OEM_ERROR_25) {
                if (supportMdAutoSetupIms()) {
                    Rlog.i(LOG_TAG, "getErrorMessage, OEM_ERROR_25 409_CONFLICT");
                    MtkSuppServHelper ssHelper =
                            MtkSuppServManager.getSuppServHelper(mPhone.getPhoneId());
                    String errorMsg = null;
                    if (ssHelper != null) {
                        errorMsg = ssHelper.getXCAPErrorMessageFromSysProp(
                                CommandException.Error.OEM_ERROR_25);
                        if (errorMsg != null && !errorMsg.isEmpty()) {
                            return errorMsg;
                        } else {
                            return mContext.getText(com.android.internal.R.string.mmiError);
                        }
                    }
                } else {
                    return mContext.getText(com.android.internal.R.string.mmiError);
                }
            } else if (err == CommandException.Error.OEM_ERROR_5) {
                Rlog.i(LOG_TAG, "getErrorMessage, OEM_ERROR_5 CALL_BARRED");
                return mContext.getText(com.mediatek.R.string.callBarringFailMmi);
            } else if (err == CommandException.Error.FDN_CHECK_FAILURE) {
                Rlog.i(LOG_TAG, "getErrorMessage, FDN_CHECK_FAILURE");
                // Using MTK's string, instead of AOSP's string
                return mContext.getText(com.mediatek.R.string.fdnFailMmi);
            }
        }
        return super.getErrorMessage(ar);
    }
}
