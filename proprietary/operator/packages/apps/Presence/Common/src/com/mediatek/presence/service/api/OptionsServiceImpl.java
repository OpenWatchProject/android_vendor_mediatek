package com.mediatek.presence.service.api;

import com.android.ims.internal.uce.options.IOptionsService;
import com.android.ims.internal.uce.options.IOptionsListener;
import com.android.ims.internal.uce.options.OptionsCapInfo;
import com.android.ims.internal.uce.options.OptionsCmdStatus;
import com.android.ims.internal.uce.options.OptionsSipResponse;
import com.android.ims.internal.uce.options.OptionsCmdId;
import com.android.ims.internal.uce.common.CapInfo;
import com.android.ims.internal.uce.common.StatusCode;
import com.android.ims.internal.uce.common.UceLong;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.ims.service.capability.OptionsManager;
import com.mediatek.presence.core.ims.service.capability.CapabilityUtils;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.core.ims.service.capability.NACapabilityService;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.protocol.sip.SipRequest;
import com.mediatek.presence.core.ims.protocol.sip.SipMessage;
import com.mediatek.presence.core.ims.network.sip.SipUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.CapTransferUtils;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.service.MtkUceService;
import com.mediatek.presence.provider.settings.RcsSettings;

import android.content.Context;
import android.os.RemoteException;

import java.util.HashMap;
import java.util.ArrayList;

public class OptionsServiceImpl extends IOptionsService.Stub {

    private Context mContext;
    private CapabilityService mCapabilityService =
            Core.getInstance().getImsModule(DEFAULT_SLOT_ID).getCapabilityService();
    private OptionsManager mOptionsManager = null;
    private static long mListenerId = 0;
    private static int mRequestId = 0;
    private static HashMap<Long,IOptionsListener> mOptionsServiceListeners =
        new HashMap<Long,IOptionsListener>();
    private static HashMap<Integer,Integer> mRequestIdActionMappingTable =
        new HashMap<Integer,Integer>();
    private static HashMap<Integer,SipRequest> mIncomingOptionRequestList =
        new HashMap<Integer,SipRequest>();
    public static final String OPTIONS_SERVICE_VERSION = "version_1.0";
    private static Logger logger = Logger.getLogger("OptionsServiceImpl");
    public static final int TYPE_SETMYCDINFO = 0;
    public static final int TYPE_GETMYCDINFO = 1;
    public static final int TYPE_GETCONTACTCAP = 2;
    public static final int TYPE_GETCONTACTLISTCAP = 3;
    public static final int DEFAULT_SLOT_ID = 0;

    public OptionsServiceImpl(Context context) {
        mContext = context;
        if (SimUtils.isAttSimCard(DEFAULT_SLOT_ID)) {
            mOptionsManager = ((NACapabilityService)mCapabilityService).getOptionsManager();
        }
    }

    public StatusCode getVersion(int optionsServiceHandle) {
        StatusCode ret = new StatusCode();
        if (logger.isActivated()) {
            logger.debug("Start getVersion and optionsServiceHdl = " +  optionsServiceHandle);
        }

        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        notifyServiceVersionToListener();
        ret.setStatusCode(StatusCode.UCE_SUCCESS);

        return ret;
    }

    public StatusCode addListener(int optionsServiceHandle, IOptionsListener optionsListener,
            UceLong optionsServiceListenerHdl) {
        if (logger.isActivated()) {
            logger.debug("Start addListener and optionsServiceHdl = " + optionsServiceHandle
                    + ", ucelong value return to client = " + mListenerId
                    + ", listener = " + optionsListener);
        }

        StatusCode ret = new StatusCode();
        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        mOptionsServiceListeners.put(mListenerId, optionsListener);
        optionsServiceListenerHdl.setUceLong(mListenerId);
        mListenerId++;
        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode removeListener(int optionsServiceHandle, UceLong optionsServiceListenerHdl) {
        if (logger.isActivated()) {
            logger.debug("Start removeListener and optionsServiceHdl = " + optionsServiceHandle
                    + ", ucelong value from client = " + optionsServiceListenerHdl.getUceLong());
        }

        StatusCode ret = new StatusCode();
        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        long listenerId = optionsServiceListenerHdl.getUceLong();
        if (!mOptionsServiceListeners.containsKey(listenerId)) {
            ret.setStatusCode(StatusCode.UCE_INVALID_LISTENER_HANDLE);
            return ret;
        }

        mOptionsServiceListeners.remove(listenerId);
        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode setMyInfo(int optionsServiceHandle, CapInfo capInfo, int reqUserData) {
        StatusCode ret = new StatusCode();
        if (logger.isActivated()) {
            logger.debug("Start setMyInfo and optionsServiceHdl = " + optionsServiceHandle
                    + ", userData = " + reqUserData);
            logger.debug("My Capability Info: " + CapTransferUtils.transferCapInfoToString(capInfo));
        }

        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        try {
            Thread t = new Thread() {
                public void run() {
                    OptionsCmdStatus cmdStatus = createCmdStatus(TYPE_SETMYCDINFO, reqUserData, null);
                    notifyCmdStatusToListener(cmdStatus);
                    Capabilities caps = CapTransferUtils.transferCapInfoToCapabilities(capInfo);
                    mOptionsManager.setMyInfo(caps);
                    mRequestId++;
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
        }

        ret.setStatusCode(StatusCode.UCE_SUCCESS);

        return ret;
    }

    public StatusCode getMyInfo(int optionsServiceHandle, int reqUserData) {
        StatusCode ret = new StatusCode();
        if (logger.isActivated()) {
            logger.debug("Start getMyInfo and optionsServiceHdl = " + optionsServiceHandle
                    + ", userData = " + reqUserData);
        }

        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        try {
            Thread t = new Thread() {
                public void run() {
                    Capabilities caps = mOptionsManager.getMyInfo();
                    OptionsCmdStatus cmdStatus = createCmdStatus(TYPE_GETMYCDINFO, reqUserData, caps);
                    notifyCmdStatusToListener(cmdStatus);
                    mRequestId++;
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
        }

        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode getContactCap(int optionsServiceHandle, String remoteUri, int reqUserData) {
        StatusCode ret = new StatusCode();
        if (logger.isActivated()) {
            logger.debug("Start getContactCap and optionsServiceHdl = " + optionsServiceHandle
                    + ", remoteUri = " + remoteUri
                    + ", userData = " + reqUserData);
        }

        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        // The format of remoteUri is "tel: + phoneNumber", we have to
        // to remove "tel:" as input string when calling requestContactCapabilities
        String contactPhoneNumber = remoteUri.substring(4);
        if (logger.isActivated()) {
            logger.info("The contact's phone number before trimimng is : " + remoteUri
                    + "The contact's phone number after trimimng is : " + contactPhoneNumber);
        }

        try {
            Thread t = new Thread() {
                public void run() {
                    OptionsCmdStatus cmdStatus = createCmdStatus(
                            TYPE_GETCONTACTCAP, reqUserData, null);
                    mRequestIdActionMappingTable.put(mRequestId, TYPE_GETCONTACTCAP);
                    notifyCmdStatusToListener(cmdStatus);
                    mOptionsManager.requestCapabilities(contactPhoneNumber, mRequestId);
                    mRequestId++;
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
        }

        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode getContactListCap(int optionsServiceHandle,
            String[] remoteUriList, int reqUserData) {
        if (logger.isActivated()) {
            for (int i = 0; i < remoteUriList.length; ++i) {
                logger.debug("Start getContactListCap and optionsServiceHdl = "
                        + optionsServiceHandle
                        + " remoteUri = " + remoteUriList[i]
                        + " userData = " + reqUserData);
            }
        }

        StatusCode ret = new StatusCode();
        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        // The format of remoteUri is "tel: + phoneNumber", we have to
        // to remove "tel:" as input string when calling requestContactCapabilities
        ArrayList<String> contactPhoneNumberList = new ArrayList<String>();
        for (int i = 0;i < remoteUriList.length; ++i) {
            contactPhoneNumberList.add(remoteUriList[i].substring(4));
            if (logger.isActivated()) {
                logger.info("The contact's phone number before trimimng is : " + remoteUriList[i]
                        + "The contact's phone number after trimimng is : "
                        + contactPhoneNumberList.get(i));
            }
        }

        try {
            Thread t = new Thread() {
                public void run() {
                    OptionsCmdStatus cmdStatus = createCmdStatus(
                            TYPE_GETCONTACTLISTCAP, reqUserData, null);
                    mRequestIdActionMappingTable.put(mRequestId, TYPE_GETCONTACTLISTCAP);
                    notifyCmdStatusToListener(cmdStatus);
                    mOptionsManager.requestCapabilities(contactPhoneNumberList, mRequestId);
                    mRequestId++;
                }
            };
            t.start();
        } catch(Exception e) {
            if (logger.isActivated()) {
                logger.error("Unexpected error", e);
            }
        }

        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode responseIncomingOptions(int optionsServiceHandle, int tId, int sipResponseCode,
            String reasonPhrase, OptionsCapInfo optionsCapInfo, boolean bContactInBL) {
        if (logger.isActivated()) {
            logger.debug("Start responseIncomingOptions and optionsServiceHdl = " + optionsServiceHandle
                    + ", tId = " + tId
                    + ", sipResponseCode = " + sipResponseCode
                    + ", reasonPhrase = " + reasonPhrase
                    + ", bContactInBL = " + bContactInBL);
            logger.debug("My Capability Info: sdp = " + optionsCapInfo.getSdp() + ", "
                    + CapTransferUtils.transferCapInfoToString(optionsCapInfo.getCapInfo()));
        }

        StatusCode ret = new StatusCode();
        if (mOptionsManager == null) {
            if (logger.isActivated()) {
                logger.error("The operator does not support options procedure");
            }
            ret.setStatusCode(StatusCode.UCE_SERVICE_UNKNOWN);
            return ret;
        }

        if (optionsServiceHandle != MtkUceService.OPTIONS_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }
        SipRequest options = mIncomingOptionRequestList.get(tId);
        if (options == null) {
            if (logger.isActivated()) {
                logger.error("Cannot find corresponding SipRequest to response incoming options");
            }
            ret.setStatusCode(StatusCode.UCE_FAILURE);
            return ret;
        }
        Capabilities myCaps = CapTransferUtils.transferCapInfoToCapabilities(
                optionsCapInfo.getCapInfo());
        mOptionsManager.responseIncomingOptions(mIncomingOptionRequestList.get(tId),
                sipResponseCode, reasonPhrase, myCaps);
        mIncomingOptionRequestList.remove(tId);

        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;

    }

    public static void receiveSipResponse(int requestId, SipResponse sipResponse) {
        String contactUri = sipResponse.getContactURI();
        String sdp = sipResponse.getSdpContent();
        Capabilities caps = CapabilityUtils.extractCapabilities(sipResponse);
        OptionsSipResponse optionsSipResponse = createSipResonse(requestId, sipResponse);
        OptionsCapInfo optionsCapInfo = createOptionsCapInfo(sdp, caps);
        if (optionsSipResponse == null) {
            return;
        }

        if (logger.isActivated()) {
            logger.debug("Receive Sip Response and requestId = " + requestId);
            logger.debug("Contact Capability Info: sdp = " + optionsCapInfo.getSdp() + ", "
                    + CapTransferUtils.transferCapInfoToString(optionsCapInfo.getCapInfo()));
        }

        notifySipResponseToListener(contactUri, optionsSipResponse, optionsCapInfo);
    }

    public static void receiveIncomingOptions(SipRequest sipRequest, Capabilities contactCaps) {
        String uri = SipUtils.getAssertedIdentity(sipRequest);
        String sdp = sipRequest.getSdpContent();
        int tId = (int)sipRequest.getCSeq();
        mIncomingOptionRequestList.put(tId, sipRequest);
        OptionsCapInfo optionsCapInfo = createOptionsCapInfo(sdp, contactCaps);

        if (logger.isActivated()) {
            logger.debug("Receive Incoming Options and uri = " + uri
                    + "tId = " + tId);
            logger.debug("Contact Capability Info: sdp = " + optionsCapInfo.getSdp() + ", "
                    + CapTransferUtils.transferCapInfoToString(optionsCapInfo.getCapInfo()));
        }

        notifyIncomingOptionsToListener(uri, optionsCapInfo, tId);

    }

    public static void serviceAvailable() {
        if (logger.isActivated()) {
            logger.info("Options Service is available now");
        }

        notifyServiceAvailableToListener();
    }

    public static void serviceUnAvailable() {
        if (logger.isActivated()) {
            logger.info("Options Service is Unavailable now");
        }

        notifyServiceUnAvailableToListener();
    }

    private void notifyServiceVersionToListener() {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifyServiceVersionToListener, listener =  " + listener);
                listener.getVersionCb(OPTIONS_SERVICE_VERSION);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send options service version to client because connection lost");
            }
        }
    }

    private void notifyCmdStatusToListener(OptionsCmdStatus optionsCmdStatus) {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifyCmdStatusToListener, listener =  " + listener);
                listener.cmdStatus(optionsCmdStatus);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send cmd status to client because connection lost");
            }
        }
    }

    private static void notifySipResponseToListener(String uri,
            OptionsSipResponse optionsSipResponse, OptionsCapInfo optionsCapInfo) {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifySipResponseToListener, listener =  " + listener);
                listener.sipResponseReceived(uri, optionsSipResponse, optionsCapInfo);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send sip response to client because connection lost");
            }
        }
    }

    private static void notifyIncomingOptionsToListener(String uri, OptionsCapInfo optionsCapInfo,
            int tId) {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifyIncomingOptionsToListener, listener =  " + listener);
                listener.incomingOptions(uri, optionsCapInfo, tId);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send sip response to client because connection lost");
            }
        }
    }

    private static void notifyServiceAvailableToListener() {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifyServiceAvailableToListener, listener =  " + listener);
                StatusCode statusCode = new StatusCode();
                statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
                listener.serviceAvailable(statusCode);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send presence service available information"
                        + " to client because connection lost");
            }
        }
    }

    private static void notifyServiceUnAvailableToListener() {
        if (mOptionsServiceListeners == null
                || mOptionsServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mOptionsServiceListeners is empty");
                return;
            }
        }

        try {
            for (IOptionsListener listener : mOptionsServiceListeners.values()) {
                logger.debug("notifyServiceUnAvailableToListener, listener =  " + listener);
                StatusCode statusCode = new StatusCode();
                statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
                listener.serviceUnavailable(statusCode);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send presence service unavailable information"
                        + " to client because connection lost");
            }
        }
    }

    private OptionsCmdStatus createCmdStatus(int type, int userData, Capabilities myCaps) {
        OptionsCmdStatus optionsCmdStatus = new OptionsCmdStatus();
        OptionsCmdId optionsCmdId = new OptionsCmdId();
        StatusCode statusCode = new StatusCode();
        CapInfo capInfo = null;

        switch (type) {
            case TYPE_GETMYCDINFO:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_GETMYCDINFO);
                capInfo = CapTransferUtils.transferCapabilitiesToCapInfo(myCaps);
                optionsCmdStatus.setCmdId(optionsCmdId);
                break;
            case TYPE_SETMYCDINFO:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_SETMYCDINFO);
                optionsCmdStatus.setCmdId(optionsCmdId);
                break;
            case TYPE_GETCONTACTCAP:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_GETCONTACTCAP);
                optionsCmdStatus.setCmdId(optionsCmdId);
                break;
            case TYPE_GETCONTACTLISTCAP:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_GETCONTACTLISTCAP);
                optionsCmdStatus.setCmdId(optionsCmdId);
                break;
            default:
                return null;
        }
        statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
        optionsCmdStatus.setStatus(statusCode);
        optionsCmdStatus.setUserData(userData);
        if (capInfo != null) {
            optionsCmdStatus.setCapInfo(capInfo);
        }
        //TODO: Ask Google why there is requestID in presCmdStatus but there is no requestId
        // in OptionsCmdStatus
        //optionsCmdStatus.setRequestId(mRequestId);

        return optionsCmdStatus;
    }

    private static OptionsSipResponse createSipResonse(int requestId, SipResponse sipResponse) {
        OptionsSipResponse optionsSipResponse = new OptionsSipResponse();
        OptionsCmdId optionsCmdId = new OptionsCmdId();
        if (!mRequestIdActionMappingTable.containsKey(requestId)) {
            logger.debug("Cannnot find the action in the mapping table for requestId: "
                    + String.valueOf(requestId));
            return null;
        }
        int type = mRequestIdActionMappingTable.get(requestId);
        String callId = sipResponse.getCallId();
        mRequestIdActionMappingTable.remove(requestId);
        switch (type) {
            case TYPE_GETCONTACTCAP:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_GETCONTACTCAP);
                break;
            case TYPE_GETCONTACTLISTCAP:
                optionsCmdId.setCmdId(OptionsCmdId.UCE_OPTIONS_CMD_GETCONTACTLISTCAP);
                break;
            default:
                return null;
        }
        optionsSipResponse.setCmdId(optionsCmdId);
        optionsSipResponse.setRequestId(requestId);
        optionsSipResponse.setSipResponseCode(sipResponse.getStatusCode());
        optionsSipResponse.setReasonPhrase(sipResponse.getReasonPhrase());
        return optionsSipResponse;
    }

    private static OptionsCapInfo createOptionsCapInfo(String sdp, Capabilities caps) {
        OptionsCapInfo optionsCapInfo = new OptionsCapInfo();
        CapInfo capInfo = CapTransferUtils.transferCapabilitiesToCapInfo(caps);
        optionsCapInfo.setCapInfo(capInfo);
        optionsCapInfo.setSdp(sdp);
        return optionsCapInfo;
    }

}
