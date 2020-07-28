package com.mediatek.ims.ril;

import java.util.ArrayList;

import android.hardware.radio.V1_0.LastCallFailCauseInfo;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;
import android.os.AsyncResult;
import android.os.Message;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.LastCallFailCause;
import com.android.internal.telephony.SmsResponse;

public class RadioResponseImpl extends RadioResponseBase {

    RadioResponseImpl(ImsRILAdapter ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("ImsRadioResponse, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private ImsRILAdapter mRil;
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
     * Response for request 'getLastCallFailCause'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void getLastCallFailCauseResponse(RadioResponseInfo responseInfo,
                LastCallFailCauseInfo failCauseInfo) {

        responseFailCause(responseInfo, failCauseInfo);
    }

    /**
     * Response for request 'acceptCall'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void acceptCallResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'conference'
     * @param info Radio Response Info
     */
    @Override
    public void conferenceResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'dial'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void dialResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void emergencyDialResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'exitEmergencyCallbackModeResponse'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void exitEmergencyCallbackModeResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'explicitCallTransfer'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void explicitCallTransferResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'hangupConnection'
     * AOSP code
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void hangupConnectionResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'sendDtmf'
     * AOSP code
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void sendDtmfResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'setMute'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void setMuteResponse(RadioResponseInfo response) {
        responseVoid(response);
    }

    /**
     * Response for request 'startDtmf'
     * AOSP code
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void startDtmfResponse(RadioResponseInfo responseInfo) {
        mRil.handleDtmfQueueNext(responseInfo.serial);
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'stopDtmf'
     * AOSP code
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void stopDtmfResponse(RadioResponseInfo info) {
        mRil.handleDtmfQueueNext(info.serial);
        responseVoid(info);
    }

    /**
     * Response for request 'switchWaitingOrHoldingAndActiveResponse'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void switchWaitingOrHoldingAndActiveResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /// MTK Proprietary Interfaces are as below =========================================

    /**
     * Response as StringArrayList
     * @param ril
     * @param responseInfo
     * @param strings
     */
    static void responseStringArrayList(ImsRILAdapter ril,
                                        RadioResponseInfo responseInfo,
                                        ArrayList<String> strings) {

        RILRequest rr = ril.processResponse(responseInfo, false);
        if (rr != null) {
            String[] ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = new String[strings.size()];
                for (int i = 0; i < strings.size(); i++) {
                    ret[i] = strings.get(i);
                }
                sendMessageResponse(rr.mResult, ret);
            }
            ril.processResponseDone(rr, responseInfo, ret);
        }
    }

    /// IRadio Extension APIs Below =====================================================

    /**
     *
     * @param info Response info struct containing response type, serial no. and error
     * @param response 0 is the TS 27.007 service class bit vector of
     *        services for which the specified barring facility
     *        is active. "0" means "disabled for all"
     */
    @Override
    public void getFacilityLockForAppResponse(RadioResponseInfo info, int resp) {
        responseInts(info, resp);
    }

    /**
     *
     * @param info Response info struct containing response type, serial no. and error
     * @param retry 0 is the number of retries remaining, or -1 if unknown
     */
    @Override
    public void setFacilityLockForAppResponse(RadioResponseInfo info, int retry) {
        responseInts(info, retry);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setCallForwardResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param callForwardInfos points to a vector of CallForwardInfo, one for
     *        each distinct registered phone number.
     */
    @Override
    public void getCallForwardStatusResponse(RadioResponseInfo info,
            ArrayList<android.hardware.radio.V1_0.CallForwardInfo> callForwardInfos) {
        responseCallForwardInfo(info, callForwardInfos);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param enable If current call waiting state is disabled, enable = false else true
     * @param serviceClass If enable, then callWaitingResp[1]
     *        must follow, with the TS 27.007 service class bit vector of services
     *        for which call waiting is enabled.
     *        For example, if callWaitingResp[0] is 1 and
     *        callWaitingResp[1] is 3, then call waiting is enabled for data
     *        and voice and disabled for everything else.
     */
    @Override
    public void getCallWaitingResponse(RadioResponseInfo info, boolean enable, int serviceClass) {
        responseInts(info, enable ? 1 : 0, serviceClass);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setCallWaitingResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param n is "n" parameter from TS 27.007 7.7
     * @param m is "m" parameter from TS 27.007 7.7
     */
    @Override
    public void getClirResponse(RadioResponseInfo info, int n, int m) {
        responseInts(info, n, m);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setClirResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     *
     * @param info Response info struct containing response type, serial no. and error
     * @param status indicates CLIP status
     */
    @Override
    public void getClipResponse(RadioResponseInfo info, int status) {
        responseInts(info, status);
    }

    /// Protected Methods ===============================================================

    @Override
    protected void riljLoge(String msg) {
        mRil.riljLoge(msg);
    }

    /// Private Methods =================================================================

    private void responseCallForwardInfo(RadioResponseInfo responseInfo,
            ArrayList<android.hardware.radio.V1_0.CallForwardInfo> callForwardInfos) {

        RILRequest rr = mRil.processResponse(responseInfo, false);
        if (rr != null) {
            CallForwardInfo[] ret = new CallForwardInfo[callForwardInfos.size()];
            for (int i = 0; i < callForwardInfos.size(); i++) {
                ret[i] = new CallForwardInfo();
                ret[i].status = callForwardInfos.get(i).status;
                ret[i].reason = callForwardInfos.get(i).reason;
                ret[i].serviceClass = callForwardInfos.get(i).serviceClass;
                ret[i].toa = callForwardInfos.get(i).toa;
                ret[i].number = callForwardInfos.get(i).number;
                ret[i].timeSeconds = callForwardInfos.get(i).timeSeconds;
            }
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }



    /**
     * Send a void response message
     * @param responseInfo
     */
    private void responseVoid(RadioResponseInfo responseInfo) {

        RILRequest rr = mRil.processResponse(responseInfo, false);
        if (rr != null) {
            Object ret = null;
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    /**
     * Send a ints response message
     * @param responseInfo
     * @param ...var
     */
    public void responseInts(RadioResponseInfo responseInfo, int ...var) {
        final ArrayList<Integer> ints = new ArrayList<Integer>();
        for (int i = 0; i < var.length; i++) {
            ints.add(var[i]);
        }
        responseIntArrayList(responseInfo, ints);
    }

    public void responseIntArrayList(RadioResponseInfo responseInfo, ArrayList<Integer> var) {
        RILRequest rr = mRil.processResponse(responseInfo, false);

        if (rr != null) {
            int[] ret = new int[var.size()];
            for (int i = 0; i < var.size(); i++) {
                ret[i] = var.get(i);
            }
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    /**
     * Send a last call fail cause response
     * @param responseInfo
     * @param info
     */
    private void responseFailCause(RadioResponseInfo responseInfo,
                                   LastCallFailCauseInfo info) {

        RILRequest rr = mRil.processResponse(responseInfo, false);
        if (rr != null) {
            LastCallFailCause failCause = null;
            if (responseInfo.error == RadioError.NONE) {
                failCause = new LastCallFailCause();
                failCause.causeCode = info.causeCode;
                failCause.vendorCause = info.vendorCause;
                sendMessageResponse(rr.mResult, failCause);
            }
            mRil.processResponseDone(rr, responseInfo, failCause);
        }
    }
}
