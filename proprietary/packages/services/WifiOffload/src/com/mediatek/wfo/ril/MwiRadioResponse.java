package com.mediatek.wfo.ril;

import java.util.ArrayList;


import vendor.mediatek.hardware.mtkradioex.V1_0.IMwiRadioResponse;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.RadioError;
import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;

public class MwiRadioResponse extends IMwiRadioResponse.Stub {

    MwiRadioResponse(MwiRIL ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("MwiRadioResponse, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private MwiRIL mRil;
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

    @Override
    public void setWifiEnabledResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setWifiAssociatedResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setWfcConfigResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setWifiSignalLevelResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setWifiIpAddressResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setLocationInfoResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setEmergencyAddressIdResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setNattKeepAliveStatusResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void setWifiPingResultResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void notifyEPDGScreenStateResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
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
