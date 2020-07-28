package com.mediatek.ims.ril;


import static com.android.internal.telephony.RILConstants.RIL_UNSOL_SUPP_SVC_NOTIFICATION;

import java.util.ArrayList;

import vendor.mediatek.hardware.mtkradioex.V1_0.IncomingCallNotification;
import vendor.mediatek.hardware.mtkradioex.V1_0.ImsConfParticipant;
import vendor.mediatek.hardware.mtkradioex.V1_0.Dialog;
import vendor.mediatek.hardware.mtkradioex.V1_0.ImsRegStatusInfo;
import vendor.mediatek.hardware.mtkradioex.V1_0.CallInfoType;

import android.hardware.radio.V1_0.CdmaSmsMessage;
import android.hardware.radio.V1_0.SuppSvcNotification;
import android.os.AsyncResult;
import android.os.Build;
import android.os.RemoteException;
import android.telephony.Rlog;
import android.telephony.SmsMessage;

import com.android.internal.telephony.cdma.SmsMessageConverter;
import com.android.internal.telephony.gsm.SuppServiceNotification;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.RIL;
import com.mediatek.ims.ImsCallSessionProxy.User;
import com.mediatek.ims.ril.ImsCommandsInterface.RadioState;
import com.mediatek.ims.ImsRegInfo;
import com.mediatek.ims.ImsServiceCallTracker;

public class ImsRadioIndication extends ImsRadioIndicationBase {

    ImsRadioIndication(ImsRILAdapter ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
    }

    // IMS RIL Instance
    private ImsRILAdapter mRil;
    // Phone Id
    private int mPhoneId;

    // IMS Constants
    private static final int INVALID_CALL_MODE = 0xFF;

    /**
     * Indicates of no emergency callback mode
     * URC: ImsRILConstants.RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE
     * @param indicationType RadioIndicationType
     * @param radioState android.hardware.radio.V1_0.RadioState
     */
    @Override
    public void noEmergencyCallbackMode(int indicationType) {

        mRil.processIndication(indicationType);
        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLog(ImsRILConstants.RIL_UNSOL_NO_EMERGENCY_CALLBACK_MODE);

        if (mRil.mNoECBMRegistrants != null) {
            mRil.mNoECBMRegistrants.notifyRegistrants();
        }
    }

    /**
     * Indicates of Video Capabilities
     * URC: ImsRILConstants.RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR
     * @param type
     * @param callId
     * @param localVideoCap
     * @param remoteVideoCap
     */
    @Override
    public void videoCapabilityIndicator(int type, String callId,
                                         String localVideoCap,
                                         String remoteVideoCap) {

        mRil.processIndication(type);
        String [] ret = new String[] { callId, localVideoCap, remoteVideoCap };

        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_VIDEO_CAPABILITY_INDICATOR, ret);

        if (mRil.mVideoCapabilityIndicatorRegistrants != null) {
            mRil.mVideoCapabilityIndicatorRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indicates of Call Mode Change
     * URC: ImsRILConstants.RIL_UNSOL_CALLMOD_CHANGE_INDICATOR
     * @param type Indication type
     * @param callId Call id
     * @param callMode Call mode
     * @param videoState Video state
     * @param audioDirection auto direction
     * @param pau PAU
     */
    @Override
    public void callmodChangeIndicator(int type, String callId, String callMode,
                                       String videoState, String audioDirection,
                                       String pau) {

        mRil.processIndication(type);
        String [] ret = new String[] { callId, callMode, videoState,
                                       audioDirection, pau };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_CALLMOD_CHANGE_INDICATOR,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mCallModeChangeIndicatorRegistrants != null) {
            mRil.mCallModeChangeIndicatorRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indication for VoLTE Setting
     * URC: ImsRILConstants.RIL_UNSOL_VOLTE_SETTING
     * @param type Indication type
     * @param isEnable is VoLTE enable
     */
    @Override
    public void volteSetting(int type, boolean isEnable) {

        mRil.processIndication(type);
        int [] ret = new int[] { isEnable ? 1:0, mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_VOLTE_SETTING, ret);

        // Store it for sticky notification
        mRil.mVolteSettingValue = ret;
        if (mRil.mVolteSettingRegistrants != null) {
            mRil.mVolteSettingRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indication for XUI
     * URC: ImsRILConstants.RIL_UNSOL_ON_XUI
     * @param type Type
     * @param accountId Account Id
     * @param broadcastFlag Broadcast flag
     * @param xuiInfo XUI Information
     */
    @Override
    public void onXui(int type, String accountId, String broadcastFlag,
                      String xuiInfo) {

        mRil.processIndication(type);
        String [] ret = new String[] { accountId, broadcastFlag,
                                       xuiInfo, Integer.toString(mPhoneId) };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_ON_XUI,
                Rlog.pii(ImsRILAdapter.IMSRIL_LOG_TAG, ret));
        }

        if (mRil.mXuiRegistrants != null) {
            mRil.mXuiRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indication for XUI
     * URC: ImsRILConstants.RIL_UNSOL_ON_VOLTE_SUBSCRIPTION
     * @param type Type
     * @param status VoLTE Subscription status (VolTE Card, non VoLTE card, Unknown)
     */
    @Override
    public void onVolteSubscription(int type, int status) {

        mRil.processIndication(type);

        int[] ret = new int[] { status, mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_ON_VOLTE_SUBSCRIPTION, ret);
        }

        if (mRil.mVolteSubscriptionRegistrants != null) {
            mRil.mVolteSubscriptionRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void suppSvcNotify(int type, SuppSvcNotification data) {
        mRil.processIndication(type);

        SuppServiceNotification notification = new SuppServiceNotification();
        notification.notificationType = data.isMT ? 1 : 0;
        notification.code = data.code;
        notification.index = data.index;
        notification.type = data.type;
        notification.number = data.number;

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(RIL_UNSOL_SUPP_SVC_NOTIFICATION, data);
        }

        if (mRil.mSuppServiceNotificationRegistrants != null) {
            mRil.mSuppServiceNotificationRegistrants.notifyRegistrants(
                    new AsyncResult (null, notification, null));
        }
    }

    /**
     * Indication for ECT
     * URC: ImsRILConstants.RIL_UNSOL_ECT_INDICATION
     * @param type Type
     * @param callId Call id
     * @param ectResult ECT result
     * @param cause Cause
     */
    @Override
    public void ectIndication(int type, int callId, int ectResult,
                              int cause) {

        mRil.processIndication(type);
        int [] ret = new int[] { callId, ectResult, cause };

        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_ECT_INDICATION, ret);

        if (mRil.mEctResultRegistrants != null) {
            mRil.mEctResultRegistrants.notifyRegistrants(
                                             new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indication for USSI
     * URC: ImsRILConstants.RIL_UNSOL_ON_USSI
     * @param type Type
     * @param clazz Class
     * @param status Status
     * @param str String
     * @param lang Cause
     * @param errorCode Cause
     * @param alertingPattern Cause
     * @param sipCause Cause
     */
    @Override
    public void onUssi(int type, int ussdModeType, String msg) {

        mRil.processIndication(type);
        String [] ret = new String[] { Integer.toString(ussdModeType), msg,
                                       Integer.toString(mPhoneId) };

        if (ImsRILAdapter.IMS_RILA_LOGD)
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_ON_USSI, ret);

        if (mRil.mUSSIRegistrants != null) {
            mRil.mUSSIRegistrants.notifyRegistrants(
                                             new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indicates for SIP call progress indicator
     * URC: ImsRILConstants.RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR
     * @param type Indication type
     * @param callId Call Id
     * @param dir Directory
     * @param sipMsgType SIP message type
     * @param method Method
     * @param responseCode Reason code
     * @param reasonText Reason text
     */
    @Override
    public void sipCallProgressIndicator(int type, String callId,
                                         String dir, String sipMsgType,
                                         String method, String responseCode,
                                         String reasonText) {

        mRil.processIndication(type);
        String [] ret = {callId, dir, sipMsgType, method, responseCode, reasonText};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_SIP_CALL_PROGRESS_INDICATOR, ret);
        }

        if (mRil.mCallProgressIndicatorRegistrants != null) {
            mRil.mCallProgressIndicatorRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indicates for ECONF result indication
     * URC: ImsRILConstants.RIL_UNSOL_ECONF_RESULT_INDICATION
     * @param type Indication type
     * @param confCallId Conference call id
     * @param op Operator
     * @param num Number
     * @param result Result
     * @param cause Cause
     * @param joinedCallId Joined call id
     */
    @Override
    public void econfResultIndication(int type, String confCallId,
                                      String op, String num,
                                      String result, String cause,
                                      String joinedCallId) {

        mRil.processIndication(type);
        String [] ret = {confCallId, op, num, result, cause, joinedCallId};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_ECONF_RESULT_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mEconfResultRegistrants != null) {
            mRil.riljLog("ECONF result = " + ret[3]);
            mRil.mEconfResultRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Call Information Indication
     * URC: ImsRILConstants.RIL_UNSOL_CALL_INFO_INDICATION
     * @param type Indication Type
     * @param result Call indication data
     */
    @Override
    public void callInfoIndication(int indicationType, ArrayList<String> result)
    {

        mRil.processIndication(indicationType);
        String [] callInfo = null;
        if(result == null || result.size() == 0) {
            return;
        }
        else {
            callInfo = result.toArray(new String[result.size()]);
        }

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_CALL_INFO_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + callInfo));
        }

        if (mRil.mCallInfoRegistrants != null) {
            mRil.mCallInfoRegistrants
                    .notifyRegistrants(new AsyncResult(null, callInfo, null));
        }
    }

    /**
     * Incoming Call Indication
     * URC: ImsRILConstants.RIL_UNSOL_INCOMING_CALL_INDICATION
     * @param type Indication Type
     * @param inCallNotify Call Notification object
     */
    @Override
    public void incomingCallIndication(int type,
            IncomingCallNotification inCallNotify) {

        mRil.processIndication(type);
        String[] ret = new String[7];
        ret[0] = inCallNotify.callId;
        ret[1] = inCallNotify.number;
        ret[2] = inCallNotify.type;
        ret[3] = inCallNotify.callMode;
        ret[4] = inCallNotify.seqNo;
        ret[5] = inCallNotify.redirectNumber;
        ret[6] = inCallNotify.toNumber;

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_INCOMING_CALL_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mIncomingCallIndicationRegistrants != null) {
            mRil.mIncomingCallIndicationRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Get Provision Down
     * URC: ImsRILConstants.RIL_UNSOL_GET_PROVISION_DONE
     * @param type Indication Type
     * @param result1 Provision Data 1
     * @param result2 Provision Data 2
     */
    @Override
    public void getProvisionDone(int type, String result1, String result2)
    {

        mRil.processIndication(type);
        String[] ret = new String[] {result1, result2};
        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_GET_PROVISION_DONE, ret);
        }

        if (mRil.mImsGetProvisionDoneRegistrants != null) {
            mRil.mImsGetProvisionDoneRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * IMS RTP Information
     * URC: ImsRILConstants.RIL_UNSOL_IMS_RTP_INFO
     * @param type Indication Type
     * @param pdnId PDN Id
     * @param networkId Network Id
     * @param timer Timer
     * @param sendPktLost Send packet lost
     * @param recvPktLost Receive package lost
     * @param jitter Jitter in ms
     * @param delay Delay in ms
     */
    @Override
    public void imsRtpInfo(int type, String pdnId, String networkId, String timer,
                           String sendPktLost, String recvPktLost, String jitter, String delay)
   {

        mRil.processIndication(type);
        String[] ret = new String[] {pdnId, networkId, timer, sendPktLost, recvPktLost,
                jitter, delay};
        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_RTP_INFO, ret);
        }

        if (mRil.mRTPInfoRegistrants != null) {
            mRil.mRTPInfoRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

   /**
     * IMS RTP Information
     * URC: ImsRILConstants.RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION
     * @param type Indication Type
     * @param callid Call Id
     * @param pType P Type
     * @param urcIdx URC Index
     * @param totalUrcCount Total URC count
     * @param rawData Raw Data
     */
    @Override
    public void imsEventPackageIndication(int type,
                                          String callId, String pType, String urcIdx,
                                          String totalUrcCount, String rawData) {
        mRil.processIndication(type);
        String [] ret = new String[] {callId, pType, urcIdx, totalUrcCount,
                                      rawData, Integer.toString(mPhoneId) };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_EVENT_PACKAGE_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mImsEvtPkgRegistrants != null) {
            mRil.mImsEvtPkgRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * IMS Registeration Information Updated
     * URC: ImsRILConstants.RIL_UNSOL_IMS_REGISTRATION_INFO
     * @param type Indication Type
     * @param status IMS registeration status
     * @param capability IMS capabilities
     */
    @Override
    public void imsRegistrationInfo(int type, int status, int capability)
    {

        mRil.processIndication(type);
        int [] ret = new int[] {status, capability, mPhoneId};
        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_REGISTRATION_INFO, ret);
        }

        if (mRil.mImsRegistrationInfoRegistrants != null) {
            mRil.mImsRegistrationInfoRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * IMS Enabled
     * URC: ImsRILConstants.RIL_UNSOL_IMS_ENABLE_DONE
     * @param type Indication Type
     */
    @Override
    public void imsEnableDone(int type)
    {

        mRil.processIndication(type);
        int [] ret = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_ENABLE_DONE, ret);
        }

        if (mRil.mImsEnableDoneRegistrants != null) {
            mRil.mImsEnableDoneRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    /**
     * IMS Disabled
     * URC: ImsRILConstants.RIL_UNSOL_IMS_DISABLE_DONE
     * @param type Indication Type
     */
    @Override
    public void imsDisableDone(int type)
    {

        mRil.processIndication(type);
        int [] ret = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_DISABLE_DONE, ret);
        }

        if (mRil.mImsDisableDoneRegistrants != null) {
            mRil.mImsDisableDoneRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    /**
     * Start IMS Enabling
     * URC: ImsRILConstants.RIL_UNSOL_IMS_ENABLE_START
     * @param type Indication Type
     */
    @Override
    public void imsEnableStart(int type)
    {

        mRil.processIndication(type);
        int [] ret = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_ENABLE_START, ret);
        }

        if (mRil.mImsEnableStartRegistrants != null) {
            mRil.mImsEnableStartRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    /**
     * Start IMS Disabling
     * URC: ImsRILConstants.RIL_UNSOL_IMS_DISABLE_START
     * @param type Indication Type
     */
    @Override
    public void imsDisableStart(int type)
    {

        mRil.processIndication(type);
        int [] ret = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_DISABLE_START, ret);
        }

        if (mRil.mImsDisableStartRegistrants != null) {
            mRil.mImsDisableStartRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    /**
     * IMS Bearer Activation
     * URC: ImsRILConstants.RIL_UNSOL_IMS_BEARER_ACTIVATION
     * @param type Indication Type
     * @param aid AID
     * @param capability Capability
     */
    @Override
    public void imsBearerStateNotify(int type, int aid, int action, String capability)
    {

        mRil.processIndication(type);

        String phoneId = String.valueOf(mPhoneId);
        String strAid = String.valueOf(aid);
        String strAction = String.valueOf(action);
        String [] ret = new String[] { phoneId, strAid, strAction, capability };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_BEARER_STATE_NOTIFY, ret);
        }

        if (mRil.mBearerStateRegistrants != null) {
            mRil.mBearerStateRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    /**
     * Indication for IMS Bearer Deactivation
     * URC: ImsRILConstants.RIL_UNSOL_IMS_BEARER_INIT
     * @param type Indication Type
     * @param aid AID
     * @param capability Capability
     */
    @Override
    public void imsBearerInit(int type)
    {

        mRil.processIndication(type);

        int ret [] = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_BEARER_INIT, ret);
        }

        if (mRil.mBearerInitRegistrants != null) {
            mRil.mBearerInitRegistrants.notifyRegistrants(new AsyncResult(null, ret,
                    null));
        }
    }

    @Override
    public void imsDataInfoNotify(int type, String capability,
              String event, String extra) {

        mRil.processIndication(type);

        String phoneId = String.valueOf(mPhoneId);
        String [] ret = new String[] { phoneId, capability, event, extra };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_DATA_INFO_NOTIFY, ret);
        }

        if (mRil.mImsDataInfoNotifyRegistrants != null) {
            mRil.mImsDataInfoNotifyRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsDeregDone(int type) {

        mRil.processIndication(type);

        int ret [] = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_DEREG_DONE, ret);
        }

        if (mRil.mImsDeregistrationDoneRegistrants != null) {
            mRil.mImsDeregistrationDoneRegistrants.notifyRegistrants(
                   new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void multiImsCount(int type, int count) {

        mRil.processIndication(type);

        int [] ret = new int[] {count, mPhoneId};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_MULTIIMS_COUNT, ret);
        }

        if (mRil.mMultiImsCountRegistrants != null) {
            mRil.mMultiImsCountRegistrants.
                    notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsSupportEcc(int type, int supportLteEcc) {

        mRil.processIndication(type);

        int[] ret = new int[] { supportLteEcc, mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.riljLog(" ImsRILConstants.RIL_UNSOL_IMS_ECC_SUPPORT, " + supportLteEcc +
                    " phoneId = " + mPhoneId);
        }

        if (mRil.mImsEccSupportRegistrants != null) {
            mRil.mImsEccSupportRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsRadioInfoChange(int type, String iid, String info) {

    }

    @Override
    public void speechCodecInfoIndication(int type, int info) {
        mRil.processIndication(type);
        int[] ret = new int[] { info };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.riljLog(" ImsRILConstants.RIL_UNSOL_SPEECH_CODEC_INFO, " + info +
                    " phoneId = " + mPhoneId);
        }

        if (mRil.mSpeechCodecInfoRegistrant != null) {
            mRil.mSpeechCodecInfoRegistrant.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * Indication for IMS Conference participants info
     * URC: ImsRILConstants.RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION
     * @param type Indication type
     * @param arrays of IMS conference participant info
     */
    @Override
    public void imsConferenceInfoIndication(int type,
            ArrayList<ImsConfParticipant> participants) {

        mRil.processIndication(type);
        ArrayList<User> ret = new ArrayList<User>();
        for (int i = 0; i < participants.size(); i++) {
            User user = new User();
            user.mUserAddr = participants.get(i).user_addr;
            user.mEndPoint = participants.get(i).end_point;
            user.mEntity   = participants.get(i).entity;
            user.mDisplayText = participants.get(i).display_text;
            user.mStatus = participants.get(i).status;
            ret.add(user);
        }

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_CONFERENCE_INFO_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mImsConfInfoRegistrants != null) {
            mRil.mImsConfInfoRegistrants.
            notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * LTE Message Waiting Information
     * URC: ImsRILConstants.RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION
     * @param type Indication Type
     * @param callid Call Id
     * @param pType P Type
     * @param urcIdx URC Index
     * @param totalUrcCount Total URC count
     * @param rawData Raw Data
     */
    @Override
    public void lteMessageWaitingIndication(int type,
                                          String callId, String pType, String urcIdx,
                                          String totalUrcCount, String rawData) {

        mRil.processIndication(type);
        String [] ret = new String[] {callId, pType, urcIdx, totalUrcCount,
                rawData, Integer.toString(mPhoneId) };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_LTE_MESSAGE_WAITING_INDICATION,
                    ImsServiceCallTracker.sensitiveEncode("" + ret));
        }

        if (mRil.mLteMsgWaitingRegistrants != null) {
            mRil.mLteMsgWaitingRegistrants.
            notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    /**
     * [IMS] IMS Dialog Event Package Indiciation
     * @param type Type of radio indication
     * @param dialogList the dialog info list
     */
    @Override
    public void imsDialogIndication(int type, ArrayList<Dialog> dialogList) {
        mRil.processIndication(type);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLog(ImsRILConstants.RIL_UNSOL_IMS_DIALOG_INDICATION);
            for (Dialog d : dialogList) {
                mRil.riljLog("RIL_UNSOL_IMS_DIALOG_INDICATION " + "dialogId = " + d.dialogId
                        + ", address:" + ImsServiceCallTracker.sensitiveEncode(d.address));
            }
        }
        if (mRil.mImsDialogRegistrant != null) {
            mRil.mImsDialogRegistrant.notifyRegistrants(new AsyncResult(null, dialogList, null));
        }
    }

    @Override
    public void imsCfgDynamicImsSwitchComplete(int type) {
        mRil.processIndication(type);

        int ret [] = new int[] { mPhoneId };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_CONFIG_DYNAMIC_IMS_SWITCH_COMPLETE, ret);
        }

        if (mRil.mImsCfgDynamicImsSwitchCompleteRegistrants != null) {
            mRil.mImsCfgDynamicImsSwitchCompleteRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsCfgFeatureChanged(int type, int phoneId, int featureId, int value) {
        mRil.processIndication(type);

        int ret [] = new int[] { mPhoneId, featureId, value };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_CONFIG_FEATURE_CHANGED, ret);
        }

        if (mRil.mImsCfgFeatureChangedRegistrants != null) {
            mRil.mImsCfgFeatureChangedRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsCfgConfigChanged(int type, int phoneId, String configId, String value) {
        mRil.processIndication(type);

        String ret [] = new String[] { Integer.toString(mPhoneId), configId, value };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_CONFIG_CONFIG_CHANGED, ret);
        }

        if (mRil.mImsCfgConfigChangedRegistrants != null) {
            mRil.mImsCfgConfigChangedRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void imsCfgConfigLoaded(int type) {
        mRil.processIndication(type);

        String ret [] = new String[] { Integer.toString(mPhoneId)};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_IMS_CONFIG_CONFIG_LOADED, ret);
        }

        if (mRil.mImsCfgConfigLoadedRegistrants != null) {
            mRil.mImsCfgConfigLoadedRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void newSmsStatusReportEx(int indicationType, ArrayList<Byte> pdu) {
        mRil.processIndication(indicationType);

        String ret [] = new String[] { Integer.toString(mPhoneId)};

        byte[] pduArray = RIL.arrayListToPrimitiveArray(pdu);
        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT_EX, ret);
        }

        if (mRil.mSmsStatusRegistrant != null) {
            mRil.mSmsStatusRegistrant.notifyRegistrant(new AsyncResult(null, pduArray, null));
        }
    }

    @Override
    public void newSmsEx(int indicationType, ArrayList<Byte> pdu) {
        mRil.processIndication(indicationType);

        String ret [] = new String[] { Integer.toString(mPhoneId)};

        byte[] pduArray = RIL.arrayListToPrimitiveArray(pdu);
        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RESPONSE_NEW_SMS_EX, ret);
        }

        if (mRil.mNewSmsRegistrant != null) {
            mRil.mNewSmsRegistrant.notifyRegistrant(new AsyncResult(null, pduArray, null));
        }
    }

    @Override
    public void cdmaNewSmsEx(int indicationType, CdmaSmsMessage msg) {
        mRil.processIndication(indicationType);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLog(ImsRILConstants.RIL_UNSOL_RESPONSE_CDMA_NEW_SMS_EX);
        }

        SmsMessage sms = SmsMessageConverter.newSmsMessageFromCdmaSmsMessage(msg);
        if (mRil.mCdmaSmsRegistrant != null) {
            mRil.mCdmaSmsRegistrant.notifyRegistrant(new AsyncResult(null, sms, null));
        }
    }

    public void imsRedialEmergencyIndication(int type, String callId) {

        mRil.processIndication(type);

        String [] ret = new String[] {callId, Integer.toString(mPhoneId) };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.riljLog(" ImsRILConstants.RIL_UNSOL_REDIAL_EMERGENCY_INDICATION, " + callId +
                    " phoneId = " + mPhoneId);
        }

        if (mRil.mImsRedialEccIndRegistrants != null) {
            mRil.mImsRedialEccIndRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void rttModifyResponse(int indicationType, int callid, int result) {

        mRil.processIndication(indicationType);

        int ret[] = {callid, result};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RTT_MODIFY_RESPONSE, ret);
        }

        if (mRil.mRttModifyResponseRegistrants != null) {
            mRil.mRttModifyResponseRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void rttTextReceive(int indicationType, int callid, int length, String text) {

        mRil.processIndication(indicationType);

        String strCallId = Integer.toString(callid);
        String strLength = Integer.toString(length);

        String ret[] = {strCallId, strLength, text};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RTT_TEXT_RECEIVE, ret);
        }

        if (mRil.mRttTextReceiveRegistrants != null) {
            mRil.mRttTextReceiveRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void rttModifyRequestReceive(int indicationType, int callid, int rttType) {

        mRil.processIndication(indicationType);

        int ret[] = {callid, rttType};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RTT_MODIFY_REQUEST_RECEIVE, ret);
        }

        if (mRil.mRttModifyRequestReceiveRegistrants != null) {
            mRil.mRttModifyRequestReceiveRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void rttCapabilityIndication(int indicationType, int callid, int localCapability,
            int remoteCapability, int localStatus, int remoteStatus) {

        mRil.processIndication(indicationType);

        int ret[] = {callid, localCapability, remoteCapability, localStatus, remoteStatus};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_RTT_CAPABILITY_INDICATION, ret);
        }

        if (mRil.mRttCapabilityIndicatorRegistrants != null) {
            mRil.mRttCapabilityIndicatorRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void audioIndication(int indicationType, int callid, int audio) {

        mRil.processIndication(indicationType);

        int ret[] = {callid, audio};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_AUDIO_INDICATION, ret);
        }

        if (mRil.mRttAudioIndicatorRegistrants != null) {
            mRil.mRttAudioIndicatorRegistrants.notifyRegistrants(
                    new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void callAdditionalInfoInd(int indicationType,
            int ciType,
            ArrayList<String> info) {
        mRil.processIndication(indicationType);

        String[] notification = new String[info.size() + 1];
        notification[0] = Integer.toString(ciType);
        for (int i = 0; i < info.size(); i++) {
            notification[i + 1] = info.get(i);
        }

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_CALL_ADDITIONAL_INFO,
                    ImsServiceCallTracker.sensitiveEncode("" + notification));
        }

        if (mRil.mCallAdditionalInfoRegistrants !=  null) {
            mRil.mCallAdditionalInfoRegistrants
                    .notifyRegistrants(new AsyncResult(null, notification, null));
        }
    }

    @Override
    public void callRatIndication(int indicationType, int domain, int rat) {
        mRil.processIndication(indicationType);

        int [] ret = new int[] {domain, rat};

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_CALL_RAT_INDICATION, ret);
        }

        if (mRil.mCallRatIndicationRegistrants !=  null) {
            mRil.mCallRatIndicationRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void sipHeaderReport(int indicationType, ArrayList<String> data) {
        mRil.processIndication(indicationType);

        String [] sipHeaderInfo = null;
        if (data == null || data.size() == 0) {
            return;
        } else {
            sipHeaderInfo = data.toArray(new String[data.size()]);
        }

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.unsljLogRet(ImsRILConstants.RIL_UNSOL_SIP_HEADER, sipHeaderInfo);
        }

        if (mRil.mImsSipHeaderRegistrants != null) {
            mRil.mImsSipHeaderRegistrants.notifyRegistrants(
                    new AsyncResult(null, sipHeaderInfo, null));
        }
    }

    @Override
    protected void riljLoge(String msg) {
        mRil.riljLoge(msg);
    }

    /*
    * Indication for IMS VoPS
    * RIL_UNSOL_VOPS_INDICATION
    *
    * @param vops IMS VoPS value
    */
    public void sendVopsIndication(int indicationType, int vops) {
        int[] ret = new int[] { vops };

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.riljLog("ImsRILConstants.RIL_UNSOL_VOPS_INDICATION, " + vops +
                    " phoneId = " + mPhoneId);
        }

        if (mRil.mVopsStatusIndRegistrants != null) {
            mRil.mVopsStatusIndRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    public void sipRegInfoInd(int indicationType, int account_id, int response_code,
            ArrayList<String>  info) {

            StringBuilder b = new StringBuilder();

            b.append("sipRegInfoInd: ").append(account_id).append(",").append(response_code);

            for (String s : info) {
                b.append(",").append(s);
            }

            mRil.riljLog(b.toString());
    }

    /**
     * Report IMS registration status
     * @param report IMS registration status
     */
    @Override
    public void imsRegStatusReport(int type, ImsRegStatusInfo report) {
        mRil.processIndication(type);

        ImsRegInfo info = new ImsRegInfo(report.report_type, report.account_id, report.expire_time,
                report.error_code, report.uri, report.error_msg);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            mRil.riljLogv(info.toString());
        }

        if (mRil.mImsRegStatusIndRistrants != null) {
            mRil.mImsRegStatusIndRistrants.notifyRegistrants(
                    new AsyncResult(null, info, null));
        }
    }

    public void imsRegInfoInd(int indicationType, ArrayList<Integer>  info) {
        mRil.processIndication(indicationType);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            StringBuilder b = new StringBuilder();

            b.append("imsRegInfoInd: ");

            for (Integer i : info) {
                b.append(i).append(", ");
            }

            b.deleteCharAt(b.length()-1);

            mRil.riljLog(b.toString());
        }

        if (mRil.mEiregIndRegistrants != null) {
            mRil.mEiregIndRegistrants.notifyRegistrants(
                    new AsyncResult(null, info, null));
        }
    }


    public void onSsacStatus(int indicationType, ArrayList<Integer> status) {
        mRil.processIndication(indicationType);

        if (ImsRILAdapter.IMS_RILA_LOGD) {
            StringBuilder b = new StringBuilder();

            b.append("onSsacStatus: ");

            for (Integer i : status) {
                b.append(i).append(", ");
            }

            b.deleteCharAt(b.length()-1);

            mRil.riljLog(b.toString());
        }

        if (mRil.mSsacIndRegistrants != null) {
            mRil.mSsacIndRegistrants.notifyRegistrants(
                    new AsyncResult(null, status, null));
        }
    }
}
