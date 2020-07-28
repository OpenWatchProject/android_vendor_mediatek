package com.mediatek.op.ims.ril;

import java.util.ArrayList;

import vendor.mediatek.hardware.radio_op.V2_0.IImsRadioResponseOp;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.RadioError;
import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;

public class OpImsRadioResponse extends IImsRadioResponseOp.Stub {

    OpImsRadioResponse(OpImsRIL ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("OpImsRadioResponse, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private OpImsRIL mRil;
    // Phone Id
    private int mPhoneId;

    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    /**
     * Response for request 'dialFrom'
     * @param info Response info struct containing response type, serial no. and error
     */
    public void dialFromResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'sendUssiFrom'
     * @param info Radio Response Info
     */
    @Override
    public void sendUssiFromResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'cancelUssiFrom'
     * @param info Radio Response Info
     */
    @Override
    public void cancelUssiFromResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setEmergencyCallConfig'
     * @param info Radio Response Info
     */
    @Override
    public void setEmergencyCallConfigResponse(RadioResponseInfo info) {
    }

    /**
     * Response for request 'deviceSwitch'
     * @param info Radio Response Info
     */
    @Override
    public void deviceSwitchResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'cancelDeviceSwitch'
     * @param info Radio Response Info
     */
    @Override
    public void cancelDeviceSwitchResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /// Private Methods =================================================================

    /**
     * Send a void response message
     * @param responseInfo
     */
    private void responseVoid(RadioResponseInfo responseInfo) {

        RILRequest rr = mRil.processResponse(responseInfo);
        if (rr != null) {
            Object ret = null;
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    /**
     * Send a string response message
     * @param responseInfo
     * @param str
     */
    private void responseString(RadioResponseInfo responseInfo, String str) {

        RILRequest rr = mRil.processResponse(responseInfo);
        if (rr != null) {
            String ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = str;
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }
}
