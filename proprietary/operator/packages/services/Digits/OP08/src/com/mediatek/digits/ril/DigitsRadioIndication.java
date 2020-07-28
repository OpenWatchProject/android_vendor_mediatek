package com.mediatek.digits.ril;

import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_UNSOL_DIGITS_LINE_INDICATION;
import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.RIL_UNSOL_GET_TRN_INDICATION;

import vendor.mediatek.hardware.radio_op.V2_0.IDigitsRadioIndication;
import android.os.AsyncResult;
import android.os.RemoteException;

import java.util.ArrayList;

public class DigitsRadioIndication extends IDigitsRadioIndication.Stub {

    DigitsRadioIndication(DigitsRIL ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("DigitsRadioIndication, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private DigitsRIL mRil;
    // Phone Id
    private int mPhoneId;

    /**
     * Digits line indication
     * URC: RIL_UNSOL_DIGITS_LINE_INDICATION
     * @param indicationType
     * @param accountId
     * @param serial
     * @param msisdnNum
     * @param msisdn
     * @param isActive
     */
    @Override
    public void digitsLineIndication(int indicationType, int accountId, int serial, int msisdnNum,
        ArrayList<String> msisdn, ArrayList<Boolean> isActive) {

        mRil.processIndication(indicationType);

        String[] ret = new String[23];
        ret[0] = Integer.toString(accountId);
        ret[1] = Integer.toString(serial);
        ret[2] = Integer.toString(msisdnNum);

        int index = 3;
        for (String eachMsisdn : msisdn) {
            ret[index ++] = eachMsisdn;
        }
        for (Boolean eachActive : isActive) {
            ret[index ++] = Integer.toString(eachActive ? 1 : 0);
        }

        if (DigitsRIL.DIGITS_RILA_LOGD) {
            mRil.unsljLogRet(RIL_UNSOL_DIGITS_LINE_INDICATION, ret);
        }

        if (mRil.mDigitsLineIndicationRegistrants != null) {
            mRil.mDigitsLineIndicationRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                                                                                       null));
        }

    }

    /**
     * Get Trn indication
     * URC: RIL_UNSOL_GET_TRN_INDICATION
     * @param indicationType
     * @param accountId
     * @param serial
     * @param msisdnNum
     * @param msisdn
     * @param isActive
     */
    @Override
    public void getTrnIndication(int indicationType, String fromMsisdn, String toMsisdn) {

        mRil.processIndication(indicationType);

        String[] ret = new String[2];
        ret[0] = fromMsisdn;
        ret[1] = toMsisdn;

        if (DigitsRIL.DIGITS_RILA_LOGD) {
            mRil.unsljLogRet(RIL_UNSOL_GET_TRN_INDICATION, ret);
        }

        if (mRil.mGetTrnIndicationRegistrants != null) {
            mRil.mGetTrnIndicationRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                                                                                       null));
        }

    }

}
