package com.mediatek.presence.core.ims.rcsua;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Looper;
import android.os.Messenger;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.Bundle;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.support.v4.content.LocalBroadcastManager;

import com.android.ims.ImsConnectionStateListener;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;
import com.android.ims.ImsServiceClass;

import com.mediatek.ims.internal.MtkImsManager;
import com.mediatek.internal.telephony.MtkPhoneConstants;
import com.mediatek.presence.core.ims.network.sip.FeatureTags;
import com.mediatek.presence.core.ims.protocol.sip.SipException;
import com.mediatek.presence.platform.AndroidFactory;
import com.mediatek.presence.platform.network.NetworkFactory;
import com.mediatek.presence.provider.settings.RcsSettings;
import com.mediatek.presence.provider.settings.RcsSettingsData;
import com.mediatek.presence.provider.settings.RcsSettingsManager;
import com.mediatek.presence.provisioning.AcsProvisioningController;

import com.mediatek.ims.rcsua.RcsUaService;
import com.mediatek.ims.rcsua.Client;
import com.mediatek.ims.rcsua.ImsEventCallback;
import com.mediatek.ims.rcsua.AcsEventCallback;
import com.mediatek.ims.rcsua.SipChannel;
import com.mediatek.ims.rcsua.Configuration;
import com.mediatek.ims.rcsua.RegistrationInfo;
import com.mediatek.presence.service.api.PresenceServiceImpl;
import com.mediatek.presence.service.api.OptionsServiceImpl;
import com.mediatek.presence.service.StartService;
import com.mediatek.presence.core.ims.rcsua.RcsUaAdapterManager;

import java.lang.Thread.State;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicLong;
import java.util.List;
import java.util.Scanner;
import java.io.InterruptedIOException;
import java.io.IOException;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.io.DataInputStream;
import java.io.BufferedOutputStream;
import java.net.InetAddress;
import java.net.UnknownHostException;
import javax2.sip.ListeningPoint;
import com.android.ims.ImsConfig;
import com.mediatek.presence.utils.logger.Logger;

//import com.mediatek.ims.WfcReasonInfo;

/**
 * The Class RcsUaAdapter.
 */
public class RcsUaAdapter {

    private Logger logger = null;

    public static final String ACTION_IMS_DEREG_START = "android.intent.presence.IMS_DEREG_START";
    public static final String ACTION_IMS_DEREG_UNPUBLISH_DONE = "android.intent.presence.IMS_DEREG_UNPUBLISH_DONE";
    public static final String ACTION_IMS_RECOVER_REGISTER = "android.intent.presence.IMS_RECOVER_REGISTER";
    public static final String EXTRA_DEREG_ID = "android:imsDeregId";
    public static final String EXTRA_SLOT_ID = "slotId";
    public static final String VOLTE_SERVICE_NOTIFY_INTENT = "COM.MEDIATEK.PRESENCE.IMS.VOLTE_SERVICE_NOTIFICATION";
    private static final String KEY_DEREG_SUSPEND = "OPTION_DEREG_SUSPEND";

    private final static int OWNER_INVALID = -1;
    private static int RCSSIPStackPort = 0;
    private Object mListenerLock = new Object();

    private static Context mContext;

    private static boolean mIsImsRegistered = false;
    private static boolean mIsRegistered = false;
    private static boolean mIsRegistering = false;
    private static boolean misRATWFC = false;
    private Messenger mMessanger;

    private static boolean isServiceStarted = false;
    private boolean mIsWfc;

    private RegistrationInfo mRegistrationInfo = null;
    private RcsSipEventListener mSipEvtListener = null;
    private VopsStateListener mVopsStateListener = null;

    //RcsUaService
    private RcsUaService mRcsUaService = null;
    private Client mClient = null;
    private SipChannel mSipChannel = null;

    private int mSlotId = 0;

    private int RCS_CAPABILITY_IMS_SESSION_SUPPORT = 0x01;
    private int RCS_CAPABILITY_FILE_TRANSFER_SUPPORT = 0x02;
    private int RCS_CAPABILITY_FILE_TRANSFER_HTTP_SUPPORT = 0x200;
    private int RCS_CAPABILITY_IP_GEO_LOCATION_PUSH_SUPPORT = 0x10;
    private int RCS_CAPABILITY_IP_GEO_LOCATION_PULL_SUPPORT = 0x20;
    private int RCS_CAPABILITY_IP_GEO_LOCATION_PULL_USING_FILE_TRANSFER_SUPPORT = 0x40;
    private int RCS_CAPABILITY_STANDALONE_MESSAGING_SUPPORT = (0x04 | 0x08);
    private int RCS_CAPABILITY_CHATBOT_SUPPORT = 0x800;
    private int RCS_CAPABILITY_CHATBOT_SA_SUPPORT = 0x1000;
    private int RCS_CAPABILITY_BOTVERSION_SUPPORT = 0x2000;
    private int RCS_CAPABILITY_CALLCOMPOSER_SUPPORT = 0x400;


    public class RcsUaServiceCallback implements RcsUaService.Callback {
        @Override
        public void serviceConnected(RcsUaService service) {
            logger.debug("RcsUaService connect");
            setServiceStatus(true);
            mRcsUaService = service;
            mClient = mRcsUaService.registerClient(new ImsEventCallbackExt());

            if (RcsUaService.isAcsAvailable(mContext)) {
                mRcsUaService.registerAcsEventCallback(new UceEventCallback());
            } else {
                logger.error("ACS is not available");
            }

            try {
                mSipChannel = mClient.openSipChannel(new SipCallback(), SipChannel.MODE_STANDALONE_PRESENCE);
            } catch (IOException e) {
                logger.error("OpenSipChannel fail", e);
            }
        }

        @Override
        public void serviceDisconnected(RcsUaService service) {
            logger.debug("RcsUaService disconnect");
            setServiceStatus(false);
            connectRcsUaService();
        }
    }

    public class ImsEventCallbackExt extends ImsEventCallback {
        @Override
        public void onRegistering(int mode) {
        }

        @Override
        public void onRegistered(int mode) {
            logger.debug("onRegistered");
            mRegistrationInfo = mClient.getRegistrationInfo();
            Configuration imsConfig = mRegistrationInfo.getImsConfig();
            logger.debug("Ims specific information is : " + imsConfig.toString());
            updateRcsCapabilities(imsConfig.getRegRcsFeatureTags());
            notifyServiceAvailable();
            if (!mIsImsRegistered) {
                mIsImsRegistered = true;
                logger.debug("broadcastVolteService for IMS registered");
                broadcastVolteService();
            }

        }

        @Override
        public void onDeregistering(int mode){
        }

        @Override
        public void onDeregistered(int mode){
            logger.debug("onDeregistered");
            cleanIMSProfileDetails();
            notifyServiceUnAvailable();
            if (mIsImsRegistered) {
                mIsImsRegistered = false;
                logger.debug("broadcastVolteService for IMS unregistered");
                broadcastVolteService();

            }
        }

        @Override
        public void onReregistered(int mode) {
        }

        @Override
        public void onDeregStart(int mode) {
            logger.debug("onDeregStart");
            for (int i = 0; i < mImsStatusListenerList.size(); i++) {
                mImsStatusListenerList.get(i).onImsDeregisterInd();
            }
        }

        @Override
        public void onVopsIndication(int vops) {
            logger.debug("onVopsIndication: " + vops);
            if (mVopsStateListener == null) {
                logger.debug("No need to notify Vops because mVopsStateListener is empty");
                return;
            }
            mVopsStateListener.onVopsStateChanged(vops);
        }
    }

    public class SipCallback extends SipChannel.EventCallback {
        @Override
        public void messageReceived(byte[] message) {
            try {
                logger.debug( "Receive SIP Message");
                String ipAddress = getHostAddress();
                InetAddress inetAddress = InetAddress.getByName(ipAddress);
                int sipPort = getRCSSipStackPort();
                if (mSipEvtListener == null) {
                    logger.error("ERROR : SIPEvtListener for SIP stack is null");
                } else {
                    logger.debug( "mSipEvtListener.notifySIPMessage");
                    mSipEvtListener.notifySipMessage(message, inetAddress, sipPort);
                }
            } catch (UnknownHostException e) {
                logger.error("InetAddress getByName fail", e);
            }
        }
    }

    public class UceEventCallback extends AcsEventCallback {
        private AcsProvisioningController acsCtrl = null;

        public UceEventCallback() {
            acsCtrl = StartService.getAcsProvisioningController();
        }

        public void onConfigurationStatusChanged(boolean valid, int version) {
            logger.debug("onConfigurationStatusChanged: valid: " + valid + " version: " + version);

            if (acsCtrl == null) {
                logger.error("AcsProvisioningController does not init");
                return;
            }

            if (valid == true) {
                 acsCtrl.updateProvisionValues(mSlotId, mRcsUaService);
            }
        }

        public void onAcsConnected() {
             logger.debug("onAcsConnected");

            if (acsCtrl == null) {
                logger.error("AcsProvisioningController does not init");
                return;
            }

            acsCtrl.updateProvisionValues(mSlotId, mRcsUaService);
        }

        public void onAcsDisconnected() {
            logger.debug("onAcsDisconnected");
        }
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            logger.debug("Receive: " + action);

            if (action.equalsIgnoreCase(RcsUaAdapter.ACTION_IMS_DEREG_UNPUBLISH_DONE)) {
                int deregId = intent.getIntExtra(EXTRA_DEREG_ID, 1);
                int slotId = intent.getIntExtra(EXTRA_SLOT_ID, 0);
                logger.debug("Receive IMS_DEREG_UNPUBLISH_DONE intent, deregId: " + deregId
                        + ", slotId: " + slotId);
                if (slotId != mSlotId) {
                    return;
                }
                if (mClient == null) {
                    logger.error("Cannot send UNPUBLISH_DONE because mClient is null");
                } else {
                    mClient.resumeImsDeregistration();
                }
            } else if (action.equalsIgnoreCase(RcsUaAdapter.ACTION_IMS_RECOVER_REGISTER)) {
                int slotId = intent.getIntExtra(EXTRA_SLOT_ID, 0);
                logger.debug("Receive IMS_RECOVER_REGISTER intent, slotId: " + slotId);
                if (slotId != mSlotId) {
                    return;
                }
                if (mRcsUaService == null) {
                    logger.error("Cannot send IMS_RECOVER_REGISTER because mRcsUaService is null");
                } else {
                    mRcsUaService.triggerRestoration();
                }
            }
        }
    };

    /**
     * Instantiates a new rcs ua adapter.
     *
     * @param slotId slot index
     * @param context the context
     */
    public RcsUaAdapter(int slotId, Context context) {
        mContext = context;
        mSlotId = slotId;
        logger = Logger.getLogger(mSlotId, "PresenceUaAdapter");

        IntentFilter filter = new IntentFilter();
        filter.addAction(RcsUaAdapter.ACTION_IMS_DEREG_UNPUBLISH_DONE);
        filter.addAction(RcsUaAdapter.ACTION_IMS_RECOVER_REGISTER);
        LocalBroadcastManager.getInstance(mContext).registerReceiver(mBroadcastReceiver, filter);

    }

    /**
     * Initialize.
     */
    public void initialize() {

        logger.debug("initialize");

        if (!isServiceStarted()) {
            connectRcsUaService();
        } else {
            logger.debug("already initialized, skip it");
        }
    }

    /**
     * terminate.
     */
    public void terminate() {
        if(!isServiceStarted()) {
            logger.debug("isServiceStarted = false, no need terminate");
            return;
        }

        cleanIMSProfileDetails();
        setServiceStatus(false);
    }

    public void unregisterIMSStateUpdates() {
        if(volteServiceUpListener != null) {
            synchronized (mListenerLock) {
                if(volteServiceUpListener !=null) {
                    AndroidFactory.getApplicationContext().unregisterReceiver(volteServiceUpListener);
                    volteServiceUpListener = null;
                }

            }
        }
    }


    /**
     * Battery level listener
     */
    private volatile BroadcastReceiver volteServiceUpListener = null;


    public boolean isServiceStarted() {
        return isServiceStarted;
    }

    public void setServiceStatus(boolean status) {
        logger.debug("setServiceStatus old = " + isServiceStarted + " new = " + status);
        isServiceStarted  = status;
    }

    public void connectRcsUaService() {
        logger.debug("connectRcsUaService");
        if (!RcsUaService.isAvailable(mContext)) {
            logger.debug("RCS UA Daemon does not support");
            return;
        }
        setServiceStatus(true);
        Bundle bundle = new Bundle();
        bundle.putInt(KEY_DEREG_SUSPEND, 1);
        RcsUaService.startService(mContext, mSlotId, new RcsUaServiceCallback(), bundle);
    }

    /**
     * Checks if is single registration supported.
     *
     * @return true, if is single registration supported
     */
    public boolean isImsRegistered() {
        logger.debug("mIsImsRegistered : " + mIsImsRegistered);
        return mIsImsRegistered;
    }


    public void sendSipMessage(byte[] message) {
        if (mSipChannel == null) {
            logger.debug("Cannot send SIP message through RcsUaService"
                    + " because mSipChannel is null");
            return;
        }

        if (!mSipChannel.isConnected()) {
            logger.debug("Cannot send SIP message through RcsUaService"
                    + " because SipChannel disconnected");
            return;
        }

        try {
            mSipChannel.sendMessage(message);
        } catch (IOException e) {
            logger.error("Cannnot send SIP message", e);
        }
    }

    public static interface ImsStatusListener {
        public void onImsDeregisterInd();
    }

    private List<ImsStatusListener> mImsStatusListenerList = new ArrayList();
    public void registerImsStatusListener(ImsStatusListener listener) {
        mImsStatusListenerList.add(listener);
    }

    public void unRegisterImsStatusListener(ImsStatusListener listener) {
        mImsStatusListenerList.remove(listener);
    }

    private void broadcastVolteService() {
        Intent intent = new Intent(RcsUaAdapter.VOLTE_SERVICE_NOTIFY_INTENT);
        intent.putExtra(EXTRA_SLOT_ID, mSlotId);
        LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
    }

    public String[] getAssociatedUri() {
        String[] uri = {""};
        return mRegistrationInfo == null ?
                uri : mRegistrationInfo.getImsConfig().getPAssociatedUri();
    }

    public String getHomeDomain() {
        return mRegistrationInfo == null ?
                "" : mRegistrationInfo.getImsConfig().getHomeDomain();
    }

    /**
     * Gets the RCS feature tag.
     *
     * @return the RCS feature tag
     */
    protected String getRCSFeatureTag() {
        String data = "";
        //data = FeatureTags.FEATURE_RCSE;
        data = FeatureTags.FEATURE_CPM_SESSION +","+FeatureTags.FEATURE_CPM_FT + "," +FeatureTags.FEATURE_CPM_MSG;
        return data;
    }

    /**
     * Notify ims connection manager connect event.
     */
    public void notifyIMSConnectionManagerConnectEvent() {

    }

    /**
     * Gets the ims proxy addr for vo lte.
     *
     * @return the ims proxy addr for vo lte
     */
    public String[] getImsProxyAddrForVoLTE() {
        if (mRegistrationInfo == null) {
            String[] addr = {""};
            return addr;
        }
        String imsProxyAddr = mRegistrationInfo.getImsConfig().getProxyAddress();
        String data[] = { imsProxyAddr };
        return data;
    }

    /**
     * Gets the ims proxy port for vo lte.
     *
     * @return the ims proxy port for vo lte
     */
    public int[] getImsProxyPortForVoLTE() {
        if (mRegistrationInfo == null) {
            int[] port = {0};
            return port;
        }
        int imsProxyPort = mRegistrationInfo.getImsConfig().getProxyPort();
        int data[] = { imsProxyPort };
        return data;
    }

    /**
     * Gets the vo lte stack ip address.
     *
     * @return the vo lte stack ip address
     */
    public String getVoLTEStackIPAddress() {
        return mRegistrationInfo == null ?
                "" : mRegistrationInfo.getImsConfig().getLocalAddress();
    }


    /**
     * Gets the host address.
     *
     * @return the host address
     */
    public String getHostAddress() {
        return getVoLTEStackIPAddress();
    }

    public String getUserAgent() {
        return mRegistrationInfo == null ?
                "" : mRegistrationInfo.getImsConfig().getUserAgent();
    }

    /**
     * Gets the SIP default protocol for vo lte.
     *
     * @return the SIP default protocol for vo lte
     */
    public String getSIPDefaultProtocolForVoLTE() {
        //TODO FIX THIS
        String data = "TCP";//String data = SIPDefaultProtocolForVoLTE;
        return data;
    }

    /**
     * Gets the IMS private id.
     *
     * @return the IMS private id
     */
    public String getIMSPrivateID() {
        String data = "";
        return data;
    }

    /**
     * Sets the RCS sip stack port.
     *
     * @param port the new RCS sip stack port
     */
    public void setRCSSipStackPort(int port) {
        RCSSIPStackPort = port;
    }

    /**
     * Gets the RCS sip stack port.
     *
     * @return the RCS sip stack port
     */
    public int getRCSSipStackPort() {
        return RCSSIPStackPort;
    }

    /**
     * Gets Digits virtual line count.
     *
     * @return Digits virtual line count
     */
    public int getVirtualLineCount() {
        return mRegistrationInfo.getImsConfig().getDigitVlineNumber();
    }

    /**
     * Gets Digits PIDENTIFIER header info.
     *
     * @return Digits PIDENTIFIER header info
     */
    public String getPidentifier() {
        return mRegistrationInfo.getImsConfig().getPIdentifier();
    }

    /**
     * Gets Digits P-Preferred-Association header info.
     *
     * @return Digits P-Preferred-Association header info
     */
    public String getPpa() {
        return mRegistrationInfo.getImsConfig().getPPreferredAssociation();
    }

    /**
     * Checks if is IMS via wfc.
     *
     * @param slotId slot index
     * @return true, if is IMS via wfc
     */
    public boolean isIMSViaWFC() {
        boolean isWfcRegistered = false;
        int[] subIds = SubscriptionManager.getSubId(mSlotId);
        if (subIds == null) {
            logger.debug("isIMSViaWFC = false because subIds is null");
            return false;
        }
        TelephonyManager tm = TelephonyManager.from(mContext).createForSubscriptionId(subIds[0]);
        isWfcRegistered = tm.isWifiCallingAvailable();

        return isWfcRegistered;
    }

    boolean isRATWFC() {
        return misRATWFC;
    }

    void setWFCRATStatus(boolean status) {
        misRATWFC = status;
    }

    /*
     * Clean IMS profile details
     */
    void cleanIMSProfileDetails() {
        logger.debug("cleanIMSProfileDetails");
        RCSSIPStackPort = 0;
        Configuration config = new Configuration();
        if (mRegistrationInfo != null) {
            mRegistrationInfo.setRegMode(OWNER_INVALID);
            mRegistrationInfo.setImsConfiguration(config);
        }
    }

    public boolean isRegisteredbyRoI() {
        int owner = mRegistrationInfo.getRegMode();
        if (owner == RcsUaService.REG_MODE_INTERNET && isImsRegistered()) {
            return true;
        }
        return false;
    }

    private ArrayList<Byte> byteToArrayList(int length, byte [] value) {
        ArrayList<Byte> al = new ArrayList<Byte>();
        logger.debug("byteToArrayList, value.length = " + value.length + ", length = " + length);
        for(int i = 0; i < length; i++) {
            al.add(value[i]);
        }
        return al;
    }

    public String byte2Hex(byte[] b) {
        String result = "";
        for (int i=0 ; i<b.length ; i++)
            result += Integer.toString( ( b[i] & 0xff ) + 0x100, 16).substring( 1 );
        return result;
    }

    /**
     * RcsSIPEventListener
     */
    public static abstract class RcsSipEventListener {

        /**
         * Notify sip message.
         *
         * @param sipMsgResponse the sip msg response
         * @param address the address
         * @param port the port
         */
        public abstract void notifySipMessage(byte[] sipMsgResponse,
                InetAddress address, int port);
    }

    /**
     * Adds the rcs sip event listener.
     *
     * @param listener the listener
     */
    public void addRcsSipEventListener(RcsSipEventListener listener) {
        logger.debug( "addRcsSipEventListener");
        mSipEvtListener = listener;
    }

    public static interface VopsStateListener {
        public void onVopsStateChanged(int vops);
    }

    public void registerVopsStateListener(VopsStateListener listener) {
        logger.debug("registerVopsStateListener");
        mVopsStateListener = listener;
    }

    //AOSP procedure
    private void notifyServiceAvailable() {
        if (mSlotId == 0) {
            PresenceServiceImpl.serviceAvailable();
            OptionsServiceImpl.serviceAvailable();
        }
    }

    //AOSP procedure
    private void notifyServiceUnAvailable() {
        if (mSlotId == 0) {
            PresenceServiceImpl.serviceUnAvailable();
            OptionsServiceImpl.serviceUnAvailable();
        }
    }

    private void updateRcsCapabilities(int rcsFeatureTag) {
        logger.debug("[updateRcsCapabilities] rcsFeatureTag = "
                + Integer.toBinaryString(rcsFeatureTag));
        if ((rcsFeatureTag & RCS_CAPABILITY_IMS_SESSION_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IM_SESSION, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_IM_SESSION, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_FILE_TRANSFER_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_FILE_TRANSFER_HTTP_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_FILE_TRANSFER_HTTP, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_IP_GEO_LOCATION_PUSH_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PUSH, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_IP_GEO_LOCATION_PULL_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PULL, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PULL, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_IP_GEO_LOCATION_PULL_USING_FILE_TRANSFER_SUPPORT)
                != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PULL_FT, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CAPABILITY_GEOLOCATION_PULL_FT, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_STANDALONE_MESSAGING_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.STANDALONE_MSG_SUPPORT, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.STANDALONE_MSG_SUPPORT, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_CHATBOT_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CHATBOT_SUPPORT, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CHATBOT_SUPPORT, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_CHATBOT_SA_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CHATBOT_SA_SUPPORT, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CHATBOT_SA_SUPPORT, "false");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_BOTVERSION_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.BOT_VERSION, "1.0");
        }

        if ((rcsFeatureTag & RCS_CAPABILITY_CALLCOMPOSER_SUPPORT) != 0) {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CALLCOMPOSER_SUPPORT, "true");
        } else {
            RcsSettingsManager.getRcsSettingsInstance(mSlotId).writeParameter(
                    RcsSettingsData.CALLCOMPOSER_SUPPORT, "false");
        }

    }
}
