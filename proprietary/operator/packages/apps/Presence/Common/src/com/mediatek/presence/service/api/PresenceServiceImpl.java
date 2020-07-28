package com.mediatek.presence.service.api;

import com.android.ims.internal.uce.presence.IPresenceService;
import com.android.ims.internal.uce.presence.IPresenceListener;
import com.android.ims.internal.uce.presence.PresCapInfo;
import com.android.ims.internal.uce.presence.PresServiceInfo;
import com.android.ims.internal.uce.presence.PresCmdId;
import com.android.ims.internal.uce.presence.PresCmdStatus;
import com.android.ims.internal.uce.presence.PresSipResponse;
import com.android.ims.internal.uce.presence.PresTupleInfo;
import com.android.ims.internal.uce.presence.PresRlmiInfo;
import com.android.ims.internal.uce.presence.PresResInfo;
import com.android.ims.internal.uce.presence.PresResInstanceInfo;
import com.android.ims.internal.uce.presence.PresPublishTriggerType;
import com.android.ims.internal.uce.common.UceLong;
import com.android.ims.internal.uce.common.StatusCode;
import com.android.ims.internal.uce.common.CapInfo;

import com.mediatek.presence.core.Core;
import com.mediatek.presence.core.ims.service.capability.CapabilityService;
import com.mediatek.presence.core.ims.service.presence.PresenceService;
import com.mediatek.presence.core.ims.service.capability.Capabilities;
import com.mediatek.presence.core.ims.service.presence.pidf.PidfDocument;
import com.mediatek.presence.core.ims.service.presence.pidf.Contact;
import com.mediatek.presence.core.ims.service.presence.pidf.Tuple;
import com.mediatek.presence.core.ims.service.presence.rlmi.ResourceInstance;
import com.mediatek.presence.core.ims.service.presence.rlmi.RlmiDocument;
import com.mediatek.presence.core.ims.service.presence.pidf.CapabilityDetails;
import com.mediatek.presence.core.ims.service.presence.PresenceUtils;
import com.mediatek.presence.service.ServiceUtils;
import com.mediatek.presence.utils.logger.Logger;
import com.mediatek.presence.utils.PstUtils;
import com.mediatek.presence.utils.CapTransferUtils;
import com.mediatek.presence.utils.SimUtils;
import com.mediatek.presence.service.MtkUceService;
import com.mediatek.presence.core.ims.protocol.sip.SipResponse;
import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.provider.settings.RcsSettings;

import java.util.HashMap;
import java.util.Vector;
import java.util.ArrayList;
import android.os.RemoteException;
import android.content.Context;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

public class PresenceServiceImpl extends IPresenceService.Stub {

    public static final int DEFAULT_SLOT_ID = 0;
    private CapabilityService mCapabilityService =
            Core.getInstance().getImsModule(DEFAULT_SLOT_ID).getCapabilityService();
    private PresenceService mPresenceService =
            Core.getInstance().getImsModule(DEFAULT_SLOT_ID).getPresenceService();
    private static Context mContext;
    private static long mListenerId = 0;
    private static volatile int mRequestId = 0;
    private static HashMap<Long,IPresenceListener> mPresenceServiceListeners =
        new HashMap<Long,IPresenceListener>();
    private static HashMap<Integer,Integer> mRequestIdActionMappingTable =
        new HashMap<Integer,Integer>();
    private static HashMap<String,Integer> mCallIdRequestIdMappingTable =
        new HashMap<String,Integer>();
    private static Logger logger = Logger.getLogger("PresenceServiceImpl");

    public static final int TYPE_REENABLESERVICE = 0;
    public static final int TYPE_PUBLISHMYCAP = 1;
    public static final int TYPE_GETCONTACTCAP = 2;
    public static final int TYPE_GETCONTACTLISTCAP = 3;
    public static final int TYPE_SETNEWFEATURETAG = 4;

    public static final String PRESENCE_SERVICE_VERSION = "version_1.0";
    public static final String ACTION_RESET_489_STATE =
            "android.intent.presence.RESET_489_STATE";

    public PresenceServiceImpl(Context context) {
        mContext = context;
    }

    public StatusCode getVersion(int presenceServiceHdl) {
        if (logger.isActivated()) {
            logger.debug("Start getVersion and presenceServiceHdl = " +  presenceServiceHdl);
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
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

    public StatusCode addListener(int presenceServiceHdl, IPresenceListener presenceServiceListener,
            UceLong presenceServiceListenerHdl) {
        if (logger.isActivated()) {
            logger.debug("Start addListener and presenceServiceHdl = " + presenceServiceHdl
                    + ", ucelong value return to client = " + mListenerId
                    + ", listener = " + presenceServiceListener);
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        mPresenceServiceListeners.put(mListenerId, presenceServiceListener);
        presenceServiceListenerHdl.setUceLong(mListenerId);
        mListenerId++;
        if (ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            notifyServiceAvailableToListener();
        }
        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode removeListener(int presenceServiceHdl, UceLong presenceServiceListenerHdl) {
        if (logger.isActivated()) {
            logger.debug("Start removeListener and presenceServiceHdl = " + presenceServiceHdl
                    + ", ucelong value from client = " + presenceServiceListenerHdl.getUceLong());
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        long listenerId = presenceServiceListenerHdl.getUceLong();
        if (!mPresenceServiceListeners.containsKey(listenerId)) {
            ret.setStatusCode(StatusCode.UCE_INVALID_LISTENER_HANDLE);
            return ret;
        }

        mPresenceServiceListeners.remove(listenerId);
        ret.setStatusCode(StatusCode.UCE_SUCCESS);
        return ret;
    }

    public StatusCode reenableService(int presenceServiceHdl, int userData) {
        if (logger.isActivated()) {
            logger.debug("Start reenableService and presenceServiceHdl = " + presenceServiceHdl
                    + ", userData = " + userData);
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
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
                        PresCmdStatus cmdStatus = createCmdStatus(TYPE_REENABLESERVICE, userData);
                        notifyCmdStatusToListener(cmdStatus);
                        mRequestId++;
                        Intent intent = new Intent();
                        intent.setAction(ACTION_RESET_489_STATE);
                        intent.putExtra("slotId", DEFAULT_SLOT_ID);
                        if (mContext != null) {
                            LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
                        }
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

    public StatusCode publishMyCap(int presenceServiceHdl, PresCapInfo myCapInfo, int userData) {
        if (logger.isActivated()) {
            logger.debug("Start publishMyCap and presenceServiceHdl = " + presenceServiceHdl
                    + ", userData = " + userData);
            logger.debug("My Capability Info: " + "contactUri = " + myCapInfo.getContactUri()
                    + ", " + CapTransferUtils.transferCapInfoToString(myCapInfo.getCapInfo()));
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
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
                        PresCmdStatus cmdStatus = createCmdStatus(TYPE_PUBLISHMYCAP, userData);
                        mRequestIdActionMappingTable.put(mRequestId, TYPE_PUBLISHMYCAP);
                        notifyCmdStatusToListener(cmdStatus);
                        Capabilities capabilities =
                                CapTransferUtils.transferCapInfoToCapabilities(myCapInfo.getCapInfo());
                        mPresenceService.updateMyCapability(mRequestId, capabilities);
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

    public StatusCode getContactCap(int presenceServiceHdl, String remoteUri, int userData) {
        if (logger.isActivated()) {
            logger.debug("Start getContactCap and presenceServiceHdl = " + presenceServiceHdl
                    + ", remoteUri = " + remoteUri
                    + ", userData = " + userData);
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
            ret.setStatusCode(StatusCode.UCE_INVALID_SERVICE_HANDLE);
            return ret;
        }

        if (!ServerApiUtils.isImsConnected(DEFAULT_SLOT_ID)) {
            ret.setStatusCode(StatusCode.UCE_LOST_NET);
            return ret;
        }

        // The format of remoteUri is "tel: + phoneNumber" or "sip: + phoneNumber", we have to
        // (1) Remove "tel:" as input string when calling requestContactCapabilities or
        // (2) Remove "sip:" as input string when calling requestContactAvailability
        String contactPhoneNumber = remoteUri.substring(4);
        String contactPhoneNumberPrefix = remoteUri.substring(0, 3);
        if (logger.isActivated()) {
            logger.debug("The contact's phone number before trimimng = " + remoteUri
                    + ", The contact's phone number after trimimng = " + contactPhoneNumber);
        }

        try {
            Thread t = new Thread() {
                public void run() {
                        PresCmdStatus cmdStatus = createCmdStatus(TYPE_GETCONTACTCAP, userData);
                        mRequestIdActionMappingTable.put(mRequestId, TYPE_GETCONTACTCAP);
                        notifyCmdStatusToListener(cmdStatus);
                        if (contactPhoneNumberPrefix.equalsIgnoreCase("sip")) {
                            mCapabilityService.requestContactAvailability(
                                    contactPhoneNumber, mRequestId);
                        } else if (contactPhoneNumberPrefix.equalsIgnoreCase("tel")) {
                            mCapabilityService.requestContactCapabilities(
                                    contactPhoneNumber, mRequestId);
                        }
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

    public StatusCode getContactListCap(int presenceServiceHdl, String[] remoteUriList, int userData) {
        if (logger.isActivated()) {
            for (int i = 0; i < remoteUriList.length; ++i) {
                logger.debug("Start getContactListCap and presenceServiceHdl = " + presenceServiceHdl
                        + " remoteUri = " + remoteUriList[i]
                        + " userData = " + userData);
            }
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
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
                logger.debug(" The contact's phone number before trimimng is : " + remoteUriList[i]
                        + " The contact's phone number after trimimng is : "
                        + contactPhoneNumberList.get(i));
            }
        }

        try {
            Thread t = new Thread() {
                public void run() {
                    PresCmdStatus cmdStatus = createCmdStatus(TYPE_GETCONTACTLISTCAP, userData);
                    mRequestIdActionMappingTable.put(mRequestId, TYPE_GETCONTACTLISTCAP);
                    notifyCmdStatusToListener(cmdStatus);
                    mCapabilityService.requestContactsCapabilities(
                            contactPhoneNumberList, mRequestId);
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

    public StatusCode setNewFeatureTag(int presenceServiceHdl, String featureTag,
            PresServiceInfo serviceInfo, int userData) {
        if (logger.isActivated()) {
            logger.debug("Start setNewFeatureTag and presenceServiceHdl = " + presenceServiceHdl
                    + " featureTag = " + featureTag
                    + " userData = " + userData);
        }

        StatusCode ret = new StatusCode();
        if (presenceServiceHdl != MtkUceService.PRESENCE_SERVICE_HDL) {
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
                    PresCmdStatus cmdStatus = createCmdStatus(TYPE_SETNEWFEATURETAG, userData);
                    notifyCmdStatusToListener(cmdStatus);
                    mRequestId++;
                    //TODO: Need to confirm tagId
                    //mPresenceService.setNewFeatureTagInfo(featureTag, serviceInfo);
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

    public static void serviceAvailable() {
        if (logger.isActivated()) {
            logger.debug("Presence Service is available now");
        }

        notifyServiceAvailableToListener();
    }

    public static void serviceUnAvailable() {
        if (logger.isActivated()) {
            logger.debug("Presence Service is Unavailable now");
        }

        notifyServiceUnAvailableToListener();
    }

    public static void unPublishMessageSent() {
        if (logger.isActivated()) {
            logger.debug("UnPublish message is sent to network");
        }

        notifyUnPublishMessageSentToListener();
    }

    public static void publishTriggering(int type) {
        if (logger.isActivated()) {
            logger.debug("Notify client to do publish if necessary");
        }

        notifyPublishTriggeringToListener(type);
    }

    public static void receiveSipResponse(int requestId, SipResponse response) {
        if (logger.isActivated()) {
            logger.debug("Receive Sip Response and requestId = " + requestId);
        }

        PresSipResponse presSipResponse = createSipResonse(requestId, response);
        if (presSipResponse == null) {
            return;
        }

        notifySipResponseToListener(presSipResponse);
    }

    public static void receiveContactCapability(PidfDocument pidfDocument) {
        String contactUri = pidfDocument.getEntity();

        if (logger.isActivated()) {
            logger.debug("Receive Contact Capability and contactUri = " + contactUri);
        }

        PresTupleInfo[] presTupleInfoList = createTupleInfoList(pidfDocument);
        if (presTupleInfoList == null) {
            return;
        }

        notifyContactCapabilityToListener(contactUri, presTupleInfoList);
    }

    public static void receiveContactListCapability(String callId, RlmiDocument rlmiDocument,
        HashMap<String,PidfDocument> pidfDocumentList) {
        if (logger.isActivated()) {
            logger.debug("Receive Contact List Capability and callId = " + callId);
        }

        if (!mCallIdRequestIdMappingTable.containsKey(callId)) {
            if (logger.isActivated()) {
                logger.debug("Cannot find the requestId based on the callId");
                return;
            }
        }

        PresRlmiInfo presRlmiInfo = createRlmiInfo(callId, rlmiDocument);
        PresResInfo[] presResInfoList = createResInfoList(rlmiDocument.getResourceList(),
            pidfDocumentList);
        if (presRlmiInfo == null
                || presResInfoList == null) {
            return;
        }
        notifyContactListCapabilityToListener(presRlmiInfo, presResInfoList);
    }

    private void notifyCmdStatusToListener(PresCmdStatus presCmdStatus) {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyCmdStatusToListener, listener =  " + listener);
                listener.cmdStatus(presCmdStatus);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send cmd status to client because connection lost");
            }
        }
    }

    private static void notifySipResponseToListener(PresSipResponse presSipResponse) {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifySipResponseToListener, listener =  " + listener);
                listener.sipResponseReceived(presSipResponse);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send sip response to client because connection lost");
            }
        }
    }

    private static void notifyContactCapabilityToListener(String contactUri,
            PresTupleInfo[] presTupleInfoList) {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyContactCapabilityToListener, listener =  " + listener);
                listener.capInfoReceived(contactUri, presTupleInfoList);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send contact capability to client because connection lost");
            }
        }
    }

    private static void notifyContactListCapabilityToListener(PresRlmiInfo presRlmiInfo,
            PresResInfo[] presResInfoList) {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyContactListCapabilityToListener, listener =  " + listener);
                listener.listCapInfoReceived(presRlmiInfo, presResInfoList);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send contact list capability to client because connection lost");
            }
        }
    }

    private static void notifyServiceVersionToListener() {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyServiceVersionToListener, listener =  " + listener);
                listener.getVersionCb(PRESENCE_SERVICE_VERSION);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send presence service version to client"
                        + " because connection lost");
            }
        }
    }

    private static void notifyServiceAvailableToListener() {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyServiceAvailableToListener, listener =  " + listener);
                StatusCode statusCode = new StatusCode();
                statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
                listener.serviceAvailable(statusCode);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send presence service available information to client"
                        + " because connection lost");
            }
        }
    }

    private static void notifyServiceUnAvailableToListener() {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                logger.debug("notifyServiceUnAvailableToListener, listener =  " + listener);
                StatusCode statusCode = new StatusCode();
                statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
                listener.serviceUnAvailable(statusCode);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot send presence service unavailable information to client"
                        + " because connection lost");
            }
        }
    }

    private static void notifyUnPublishMessageSentToListener() {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                listener.unpublishMessageSent();
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot notify unpublish message is sent to client"
                        + " because connection lost");
            }
        }
    }

    private static void notifyPublishTriggeringToListener(int type) {
        if (mPresenceServiceListeners == null
                || mPresenceServiceListeners.size() == 0) {
            if (logger.isActivated()) {
                logger.error("mPresenceServiceListeners is empty");
                return;
            }
        }

        try {
            for (IPresenceListener listener : mPresenceServiceListeners.values()) {
                PresPublishTriggerType typeObject = new PresPublishTriggerType();
                typeObject.setPublishTrigeerType(type);
                listener.publishTriggering(typeObject);
            }
        } catch (RemoteException e) {
            if (logger.isActivated()) {
                logger.debug("Cannot notify client to do is sent to client because connection lost");
            }
        }
    }

    private PresCmdStatus createCmdStatus(int type, int userData) {
        PresCmdStatus presCmdStatus = new PresCmdStatus();
        PresCmdId presCmdId = new PresCmdId();
        StatusCode statusCode = new StatusCode();
        String typeString = "";
        switch (type) {
            case TYPE_PUBLISHMYCAP:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_PUBLISHMYCAP);
                presCmdStatus.setCmdId(presCmdId);
                typeString = "TYPE_PUBLISHMYCAP";
                break;
            case TYPE_GETCONTACTCAP:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_GETCONTACTCAP);
                presCmdStatus.setCmdId(presCmdId);
                typeString = "TYPE_GETCONTACTCAP";
                break;
            case TYPE_GETCONTACTLISTCAP:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_GETCONTACTLISTCAP);
                presCmdStatus.setCmdId(presCmdId);
                typeString = "TYPE_GETCONTACTLISTCAP";
                break;
            case TYPE_SETNEWFEATURETAG:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_SETNEWFEATURETAG);
                presCmdStatus.setCmdId(presCmdId);
                typeString = "TYPE_SETNEWFEATURETAG";
                break;
            case TYPE_REENABLESERVICE:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_REENABLE_SERVICE);
                presCmdStatus.setCmdId(presCmdId);
                typeString = "TYPE_REENABLESERVICE";
                break;
            default:
                return null;
        }

        if (logger.isActivated()) {
            logger.debug("Action Type = " + typeString + ", mRequestId = " + mRequestId);
        }

        statusCode.setStatusCode(StatusCode.UCE_SUCCESS);
        presCmdStatus.setStatus(statusCode);
        presCmdStatus.setUserData(userData);
        presCmdStatus.setRequestId(mRequestId);

        return presCmdStatus;
    }

    private static PresSipResponse createSipResonse(int requestId, SipResponse sipResponse) {
        PresSipResponse presSipResponse = new PresSipResponse();
        PresCmdId presCmdId = new PresCmdId();
        if (!mRequestIdActionMappingTable.containsKey(requestId)) {
            logger.debug("Cannnot find the action in the mapping table for requestId: "
                    + String.valueOf(requestId));
            return null;
        }
        if (logger.isActivated()) {
            logger.debug("RequestId = " + requestId
                    + ", SipResponse statusCode = " + sipResponse.getStatusCode()
                    + ", SipResponse reasonPhrase = " + sipResponse.getReasonPhrase());
        }
        int type = mRequestIdActionMappingTable.get(requestId);
        String callId = sipResponse.getCallId();
        mRequestIdActionMappingTable.remove(requestId);
        switch (type) {
            case TYPE_PUBLISHMYCAP:
                break;
            case TYPE_GETCONTACTCAP:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_GETCONTACTCAP);
                mCallIdRequestIdMappingTable.put(callId, requestId);
                break;
            case TYPE_GETCONTACTLISTCAP:
                presCmdId.setCmdId(PresCmdId.UCE_PRES_CMD_GETCONTACTLISTCAP);
                mCallIdRequestIdMappingTable.put(callId, requestId);
                break;
            default:
                return null;
        }
        presSipResponse.setCmdId(presCmdId);
        presSipResponse.setRequestId(requestId);
        presSipResponse.setSipResponseCode(sipResponse.getStatusCode());
        presSipResponse.setReasonPhrase(sipResponse.getReasonPhrase());
        return presSipResponse;
    }

    private static PresTupleInfo[] createTupleInfoList(PidfDocument pidfDocument) {
        Vector<Tuple> tuples = pidfDocument.getTuplesList();
        if (tuples.size() == 0) {
            logger.debug("Does not receive any tuple");
            return null;
        }
        PresTupleInfo[] presTupleInfoList = new PresTupleInfo[tuples.size()];
        int tupleIndex = 0;
        for (Tuple tuple : tuples) {
            String contactUri = "";
            String featureTag = "";
            String timestamp = "";
            String serviceId = "";
            Vector<Contact> contactList = tuple.getContactList();
            if (contactList.size() == 0) {
                logger.debug("Does not receive <contact> information");
                contactUri = pidfDocument.getEntity();
            } else {
                contactUri = contactList.elementAt(0).getUri();
            }

            timestamp = String.valueOf(tuple.getTimestamp());
            if (logger.isActivated()) {
                logger.debug("ContactUri is : " + contactUri
                        + ", ServiceId is : " + serviceId
                        + ", Timestamp is : " + timestamp);
            }
            // If the stae is "open" meaning contact support this tuple capability,
            // then device shall pass featureTag to client
            String state = tuple.getStatus().getBasic().getValue();
            if (state.equals("open")) {
                serviceId = tuple.getService().getId();
                featureTag = getFeatureTagFromServiceId(serviceId, tuple);
                if (featureTag.equals("")) {
                    logger.debug("Cannot get feature tag because of unknown service id");
                    continue;
                }
            } else {
                logger.debug("Contact does not support: " + tuple.getId() + ", state is : "
                        + state);
                continue;
            }


            PresTupleInfo presTupleInfo = new PresTupleInfo();
            presTupleInfo.setContactUri(contactUri);
            presTupleInfo.setFeatureTag(featureTag);
            presTupleInfo.setTimestamp(timestamp);
            presTupleInfoList[tupleIndex] = presTupleInfo;
            tupleIndex++;
        }

        return presTupleInfoList;
    }

    private static PresRlmiInfo createRlmiInfo(String callId, RlmiDocument rlmiDocument) {
        PresRlmiInfo presRlmiInfo = new PresRlmiInfo();
        String uri = rlmiDocument.getUri();
        int version = rlmiDocument.getVersion();
        int requestId = mCallIdRequestIdMappingTable.get(callId);
        boolean fullState = rlmiDocument.isFullState();
        String listName = rlmiDocument.getName();
        int subscriptionExpireTime = getSubscriptionExpireTime();
        //TODO: mPresSubscriptionState and mSubscriptionTerminatedReason have not setup yet
        // because we cannot find related tag in RFC 4662
        mCallIdRequestIdMappingTable.remove(callId);

        presRlmiInfo.setUri(uri);
        presRlmiInfo.setVersion(version);
        presRlmiInfo.setRequestId(requestId);
        presRlmiInfo.setFullState(fullState);
        presRlmiInfo.setListName(listName);
        presRlmiInfo.setSubscriptionExpireTime(subscriptionExpireTime);

        if (logger.isActivated()) {
            logger.debug("Rlmi Information is : Uri = " + uri
                    + ", Version = " + String.valueOf(version)
                    + ", RequestId = " + String.valueOf(requestId)
                    + ", Fullstate = " + String.valueOf(fullState)
                    + ", Listname = " + listName
                    + ", subscriptionExpireTime = " + subscriptionExpireTime);
        }

        return presRlmiInfo;

    }

    private static PresResInfo[] createResInfoList(Vector<ResourceInstance> resourceInstanceList,
            HashMap<String,PidfDocument> pidfDocumentList) {
        int index = 0;
        PresResInfo[] presResInfoList = new PresResInfo[resourceInstanceList.size()];
        for (ResourceInstance resourceInstance : resourceInstanceList) {
            String uri = resourceInstance.getUri();
            String displayName = resourceInstance.getName();
            PresResInfo presResInfo = new PresResInfo();
            presResInfo.setResUri(uri);
            presResInfo.setDisplayName(displayName);
            PresResInstanceInfo resInstanceInfo = createPresResInstanceInfo(resourceInstance,
                    pidfDocumentList.get(uri));
            presResInfo.setInstanceInfo(resInstanceInfo);
            presResInfoList[index] = presResInfo;
            index++;
        }

        return presResInfoList;

    }

    private static PresResInstanceInfo createPresResInstanceInfo(ResourceInstance resourceInstance,
            PidfDocument pidfDocument) {
        PresResInstanceInfo presResInstanceInfo = new PresResInstanceInfo();
        int state = PresResInstanceInfo.UCE_PRES_RES_INSTANCE_STATE_UNKNOWN;
        switch (resourceInstance.getState()) {
            case ResourceInstance.STATUS_ACTIVE:
                state = PresResInstanceInfo.UCE_PRES_RES_INSTANCE_STATE_ACTIVE;
                break;
            case ResourceInstance.STATUS_PENDING:
                state = PresResInstanceInfo.UCE_PRES_RES_INSTANCE_STATE_PENDING;
                break;
            case ResourceInstance.STATUS_TERMINATED:
                state = PresResInstanceInfo.UCE_PRES_RES_INSTANCE_STATE_TERMINATED;
                break;
            default:
                state = PresResInstanceInfo.UCE_PRES_RES_INSTANCE_STATE_UNKNOWN;
                break;
        }
        String id = resourceInstance.getId();
        String reason = resourceInstance.getReason();
        String uri = resourceInstance.getUri();
        PresTupleInfo[] tupleInfoList = createTupleInfoList(pidfDocument);

        presResInstanceInfo.setResInstanceState(state);
        presResInstanceInfo.setResId(id);
        presResInstanceInfo.setReason(reason);
        presResInstanceInfo.setPresentityUri(uri);
        presResInstanceInfo.setTupleInfo(tupleInfoList);

        return presResInstanceInfo;
    }

    private static int getSubscriptionExpireTime() {
        int expireTime = 0;
        if (SimUtils.isVzwSimCard(DEFAULT_SLOT_ID)) {
            expireTime = PstUtils.getCapabilityPollListSubExp(DEFAULT_SLOT_ID, mContext);
        }
        return expireTime;
    }

    private static String getFeatureTagFromServiceId(String serviceId, Tuple tuple) {
        if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IP_VOICE_CALL)) {
            boolean isAudioSupported = false;
            boolean isVideoSupported = false;
            for(CapabilityDetails c : tuple.getServiceCapability().getCapabilityList()) {
                if(c.getName().equalsIgnoreCase("audio")) {
                    if(c.getValue().equalsIgnoreCase("true")) {
                        isAudioSupported = true;
                        continue;
                    }
                }

                if(c.getName().equalsIgnoreCase("video")) {
                    if(c.getValue().equalsIgnoreCase("true")) {
                        isVideoSupported = true;
                        continue;
                    }
                }
            }
            if (isAudioSupported && isVideoSupported) {
                return FeatureTags.FEATURE_3GPP_IP_VIDEO_CALL;
            } else if (isAudioSupported) {
                return FeatureTags.FEATURE_3GPP_IP_VOICE_CALL;
            } else if (isVideoSupported) {
                return FeatureTags.FEATURE_RCSE_IP_VIDEO_CALL_2;
            }
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_STANDALONEMSG)) {
            return FeatureTags.FEATURE_3GPP_STANDALONE_MSG;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT)
                || serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_CHAT_2)) {
            return FeatureTags.FEATURE_3GPP_CHAT;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_ST_GROUP_CHACT)) {
            return FeatureTags.FEATURE_3GPP_ST_GROUP_CHAT;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT)) {
            return FeatureTags.FEATURE_3GPP_FT;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT_THUMBNAIL)) {
            return FeatureTags.FEATURE_3GPP_FT_THUMB;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_FT_HTTP)) {
            return FeatureTags.FEATURE_3GPP_FT_HTTP;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_IMAGE_SHARE)) {
            return FeatureTags.FEATURE_3GPP_IMAGE_SHARE;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_VIDEO_SHARE)) {
            return FeatureTags.FEATURE_3GPP_VIDEO_SHARE;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_SOCIAL_PRESENCE_INFO)) {
            return FeatureTags.FEATURE_3GPP_SOCIAL_PRESENCE_INFO;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_RCS2_DISCOVERY_VIA_PRESENCE)) {
            return FeatureTags.FEATURE_3GPP_DISCOVERY_VIA_PRESENCE;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_GEOLOCATION_PULL)) {
            return FeatureTags.FEATURE_3GPP_GEOLOCATION_PULL;
        } else if (serviceId.equalsIgnoreCase(PresenceUtils.FEATURE_GEOLOCATION_PUSH)) {
            return FeatureTags.FEATURE_3GPP_GEOLOCATION_PUSH;
        }

        return "";
    }

}
