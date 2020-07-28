package com.mediatek.ims.ril;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.TimeZone;

import com.android.internal.telephony.CallForwardInfo;
import com.android.internal.telephony.LastCallFailCause;

import vendor.mediatek.hardware.mtkradioex.V1_0.CallForwardInfoEx;

import android.hardware.radio.V1_0.LastCallFailCauseInfo;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.os.AsyncResult;
import android.os.Message;
import android.os.RemoteException;

import com.mediatek.internal.telephony.MtkCallForwardInfo;

// SMS-START
import android.hardware.radio.V1_0.SendSmsResult;
import com.mediatek.ims.MtkSmsResponse;
// SMS-END

public class ImsRadioResponse extends ImsRadioResponseBase {

    private ImsRILAdapter mRil;
    // Phone Id
    private int mPhoneId;

    ImsRadioResponse(ImsRILAdapter ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("ImsRadioResponse, phone = " + mPhoneId);
    }

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
     * Response for request 'pullCall'
     * @param responseInfo Response info containing response type, serial no. and error
     */
    @Override
    public void pullCallResponse(RadioResponseInfo responseInfo) {
       responseVoid(responseInfo);
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

        RILRequest rr = ril.processResponse(responseInfo, true);
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
     * Response for request 'videoCallAccept'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void videoCallAcceptResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'eccRedialApprove'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void eccRedialApproveResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'imsEctCommand'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void imsEctCommandResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    /**
     * Response for request 'holdCall' and 'resumeCall'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void controlCallResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setCallIndication'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setCallIndicationResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'imsDeregNotification'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void imsDeregNotificationResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setImsEnable'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setImsEnableResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setImsVideoEnable'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setImsVideoEnableResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setImscfg'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setImscfgResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setModemImsCfg'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setModemImsCfgResponse(RadioResponseInfo info, String results) {
        responseString(info, results);
    }

    /**
     * Response for request 'getProvisionValue'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void getProvisionValueResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setProvisionValue'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setProvisionValueResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void setImsCfgFeatureValueResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void getImsCfgFeatureValueResponse(RadioResponseInfo info, int value) {
        responseInts(info, value);
    }

    @Override
    public void setImsCfgProvisionValueResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void getImsCfgProvisionValueResponse(RadioResponseInfo info, String value) {
        responseString(info, value);
    }

    @Override
    public void getImsCfgResourceCapValueResponse(RadioResponseInfo info, int value) {
        responseInts(info, value);
    }

    /**
     * Response for request 'removeImsConferenceCallMemberResponse' and 'addImsConferenceCallMember'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void controlImsConferenceCallMemberResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'hangupAll'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void hangupAllResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setWfcProfile'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setWfcProfileResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'conferenceDial'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void conferenceDialResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'vtDial'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void vtDialResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'vtDialWithSipUri'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void vtDialWithSipUriResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'dialWithSipUri'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void dialWithSipUriResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'sendUssi'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void sendUssiResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'cancelUssi'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void cancelUssiResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'getXcapStatusResponse'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void getXcapStatusResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'resetSuppServResponse'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void resetSuppServResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setupXcapUserAgentStringResponse'
     * @param info Response info containing response type, serial no. and error
     */
    @Override
    public void setupXcapUserAgentStringResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'forceReleaseCall'
     * @param info Radio Response Info
     */
    @Override
    public void forceReleaseCallResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'imsBearerStateConfirm'
     * @param info Radio Response Info
     */
    @Override
    public void imsBearerStateConfirmResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'setImsBearerNotificationResponse'
     * @param info Radio Response Info
     */
    @Override
    public void setImsBearerNotificationResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * Response for request 'imsRtpReport'
     * @param info Radio Response Info
     */
    @Override
    public void setImsRtpReportResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void setImsRegistrationReportResponse(RadioResponseInfo info) {
       responseVoid(info);
    }

    /* MTK SS Feature : Start */
    @Override
    public void setVoiceDomainPreferenceResponse(RadioResponseInfo info) {

       responseVoid(info);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setClipResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param status indicates COLR status. "0" means not provisioned, "1" means provisioned,
     *        "2" means unknown
     */
    @Override
    public void getColrResponse(RadioResponseInfo info, int status) {
        responseInts(info, status);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setColrResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param n Colp status in network, "0" means disabled, "1" means enabled
     * @param m Service status, "0" means not provisioned, "1" means provisioned in permanent mode
     */
    @Override
    public void getColpResponse(RadioResponseInfo info, int n, int m) {
        responseInts(info, n, m);
    }

    /**
     * @param info Response info struct containing response type, serial no. and error
     */
    @Override
    public void setColpResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void queryCallForwardInTimeSlotStatusResponse(RadioResponseInfo responseInfo,
                                                         ArrayList<CallForwardInfoEx>
                                                         callForwardInfoExs) {
        responseCallForwardInfoEx(responseInfo, callForwardInfoExs);
    }

    @Override
    public void setCallForwardInTimeSlotResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    @Override
    public void runGbaAuthenticationResponse(RadioResponseInfo responseInfo,
                                             ArrayList<String> resList) {

        responseStringArrayList(mRil, responseInfo, resList);
    }
    /* MTK SS Feature : End */

    @Override
    public void hangupWithReasonResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    // Client API start
    @Override
    public void setSipHeaderResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void setSipHeaderReportResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void setImsCallModeResponse(RadioResponseInfo info) {
        responseVoid(info);
    }

    @Override
    public void toggleRttAudioIndicationResponse(RadioResponseInfo info) {
        responseVoid(info);
    }
    // Client API end

    /// Protected Methods ===============================================================

    @Override
    protected void riljLoge(String msg) {
        mRil.riljLoge(msg);
    }

    /// Private Methods =================================================================
    private void responseCallForwardInfoEx(RadioResponseInfo responseInfo,
                                           ArrayList<CallForwardInfoEx> callForwardInfoExs) {

        long[] timeSlot;
        String[] timeSlotStr;
        // process response in Ims RIL
        RILRequest rr = mRil.processResponse(responseInfo, true);
        if (rr != null) {
            MtkCallForwardInfo[] ret = new MtkCallForwardInfo[callForwardInfoExs.size()];
            for (int i = 0; i < callForwardInfoExs.size(); i++) {
                timeSlot = new long[2];
                timeSlotStr = new String[2];

                ret[i] = new MtkCallForwardInfo();
                ret[i].status = callForwardInfoExs.get(i).status;
                ret[i].reason = callForwardInfoExs.get(i).reason;
                ret[i].serviceClass = callForwardInfoExs.get(i).serviceClass;
                ret[i].toa = callForwardInfoExs.get(i).toa;
                ret[i].number = callForwardInfoExs.get(i).number;
                ret[i].timeSeconds = callForwardInfoExs.get(i).timeSeconds;
                timeSlotStr[0] = callForwardInfoExs.get(i).timeSlotBegin;
                timeSlotStr[1] = callForwardInfoExs.get(i).timeSlotEnd;

                if (timeSlotStr[0] == null || timeSlotStr[1] == null) {
                    ret[i].timeSlot = null;
                } else {
                    // convert to local time
                    for (int j = 0; j < 2; j++) {
                        SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm");
                        dateFormat.setTimeZone(TimeZone.getTimeZone("GMT+8"));
                        try {
                            Date date = dateFormat.parse(timeSlotStr[j]);
                            timeSlot[j] = date.getTime();
                        } catch (ParseException e) {
                            e.printStackTrace();
                            timeSlot = null;
                            break;
                        }
                    }
                    ret[i].timeSlot = timeSlot;
                }
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

        RILRequest rr = mRil.processResponse(responseInfo, true);
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

        RILRequest rr = mRil.processResponse(responseInfo, true);
        if (rr != null) {
            String ret = null;
            if (responseInfo.error == RadioError.NONE) {
                ret = str;
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
        final ArrayList<Integer> ints = new ArrayList<>();
        for (int i = 0; i < var.length; i++) {
            ints.add(var[i]);
        }
        responseIntArrayList(responseInfo, ints);
    }

    public void responseIntArrayList(RadioResponseInfo responseInfo, ArrayList<Integer> var) {
        RILRequest rr = mRil.processResponse(responseInfo, true);

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

        RILRequest rr = mRil.processResponse(responseInfo, true);
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

    // SMS-START
    /**
     *
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param sms Response to sms sent as defined by SendSmsResult in types.hal
     */
    public void sendImsSmsExResponse(RadioResponseInfo responseInfo, SendSmsResult sms) {
        RILRequest rr = mRil.processResponse(responseInfo, true);

        if (rr != null) {
            MtkSmsResponse ret = new MtkSmsResponse(sms.messageRef, sms.ackPDU, sms.errorCode);
            if (responseInfo.error == RadioError.NONE) {
                sendMessageResponse(rr.mResult, ret);
            }
            mRil.processResponseDone(rr, responseInfo, ret);
        }
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void acknowledgeLastIncomingGsmSmsExResponse (
            RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    public void acknowledgeLastIncomingCdmaSmsExResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }
    // SMS-END


    public void queryVopsStatusResponse(RadioResponseInfo responseInfo, int vops) {
        responseInts(responseInfo, vops);
    }

    public void setVendorSettingResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }

    public void querySsacStatusResponse(RadioResponseInfo responseInfo, ArrayList<Integer> status) {
        responseIntArrayList(responseInfo, status);
    }

    @Override
    public void setCallAdditionalInfoResponse(RadioResponseInfo responseInfo) {
        responseVoid(responseInfo);
    }
}
