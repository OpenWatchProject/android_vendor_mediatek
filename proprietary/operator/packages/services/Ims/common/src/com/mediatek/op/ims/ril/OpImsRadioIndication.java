package com.mediatek.op.ims.ril;

import vendor.mediatek.hardware.radio_op.V2_0.IImsRadioIndicationOp;
import android.os.AsyncResult;
import android.os.RemoteException;

public class OpImsRadioIndication extends IImsRadioIndicationOp.Stub {

    OpImsRadioIndication(OpImsRIL ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("OpImsRadioIndication, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private OpImsRIL mRil;
    // Phone Id
    private int mPhoneId;

}
