/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.internal.telephony;

import android.content.ComponentName;
import android.content.Context;
import android.net.NetworkRequest;
import android.os.Handler;
import android.os.Looper;
import android.os.SystemProperties;
import android.telephony.AccessNetworkConstants.TransportType;
import android.telephony.Rlog;
import android.util.Log;

import com.android.internal.telephony.CallManager;
import com.android.internal.telephony.cdma.CdmaSubscriptionSourceManager;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.DefaultPhoneNotifier;
import com.android.internal.telephony.GsmCdmaCallTracker;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.ims.ImsServiceController;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCallTracker;
import com.android.internal.telephony.ITelephonyRegistry;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneNotifier;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.ProxyController;
import com.android.internal.telephony.RIL;
import com.android.internal.telephony.ServiceStateTracker;
// MTK-START: SIM COMMON
import com.android.internal.telephony.uicc.IccCardStatus;
import com.android.internal.telephony.uicc.UiccController;
import com.mediatek.internal.telephony.uicc.MtkUiccController;
import com.mediatek.internal.telephony.uicc.MtkUiccProfile;
// MTK-END
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.SubscriptionInfoUpdater;
import com.android.internal.telephony.MultiSimSettingController;

import com.mediatek.internal.telephony.MtkPhoneNotifier;
import com.mediatek.internal.telephony.MtkProxyController;

import com.android.internal.telephony.ImsSmsDispatcher;
import com.android.internal.telephony.SmsStorageMonitor;
import com.android.internal.telephony.SmsUsageMonitor;
import com.android.internal.telephony.gsm.GsmSMSDispatcher;
import com.android.internal.telephony.gsm.GsmInboundSmsHandler;
import com.android.internal.telephony.cdma.CdmaInboundSmsHandler;
import com.android.internal.telephony.SmsBroadcastUndelivered;
import com.android.internal.telephony.IccSmsInterfaceManager;
import com.android.internal.telephony.SmsUsageMonitor;
import com.android.internal.telephony.SmsStorageMonitor;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.InboundSmsTracker;
import com.android.internal.telephony.SmsHeader;
import com.android.internal.telephony.WspTypeDecoder;
import com.android.internal.telephony.WapPushOverSms;
import com.android.internal.telephony.SmsDispatchersController;
import com.mediatek.internal.telephony.gsm.MtkGsmSMSDispatcher;
import com.mediatek.internal.telephony.MtkImsSmsDispatcher;
import com.mediatek.internal.telephony.MtkInboundSmsTracker;
import com.mediatek.internal.telephony.MtkSmsDispatchersController;
import android.database.Cursor;
import com.mediatek.internal.telephony.MtkWspTypeDecoder;
import com.mediatek.internal.telephony.MtkWapPushOverSms;
import com.mediatek.internal.telephony.MtkIccSmsInterfaceManager;
import com.mediatek.internal.telephony.MtkSmsUsageMonitor;
import com.mediatek.internal.telephony.MtkSmsStorageMonitor;
import com.mediatek.internal.telephony.MtkSmsBroadcastUndelivered;
import com.mediatek.internal.telephony.gsm.MtkGsmInboundSmsHandler;
import com.mediatek.internal.telephony.gsm.MtkGsmCellBroadcastHandler;
import com.mediatek.internal.telephony.imsphone.MtkImsPhoneCallTracker;
import com.mediatek.internal.telephony.imsphone.MtkImsPhone;
import com.mediatek.internal.telephony.MtkSmsHeader;
import com.mediatek.internal.telephony.RadioManager;
import com.mediatek.internal.telephony.MtkNetworkStatusUpdater;

// MTK-START
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.DeviceStateMonitor;
import com.android.internal.telephony.cat.CatService;
import com.android.internal.telephony.cat.RilMessageDecoder;
import com.android.internal.telephony.cat.CommandParamsFactory;
import com.android.internal.telephony.cat.IconLoader;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccProfile;
import com.android.internal.telephony.PhoneConstants;

import com.mediatek.internal.telephony.cat.MtkCatService;
import com.mediatek.internal.telephony.cat.MtkRilMessageDecoder;
import com.mediatek.internal.telephony.cat.MtkCommandParamsFactory;
import com.mediatek.internal.telephony.cat.MtkIconLoader;
import com.mediatek.internal.telephony.cat.MtkCatLog;
import com.mediatek.internal.telephony.cdma.MtkCdmaSubscriptionSourceManager;
import com.android.internal.telephony.SubscriptionMonitor;
import com.android.internal.telephony.TelephonyComponentFactory;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DataServiceManager;
import com.android.internal.telephony.dataconnection.DcController;
import com.android.internal.telephony.dataconnection.DcRequest;
import com.android.internal.telephony.dataconnection.DcTesterFailBringUpAll;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.dataconnection.TelephonyNetworkFactory;
import com.mediatek.internal.telephony.dataconnection.MtkDataConnection;
import com.mediatek.internal.telephony.dataconnection.MtkDcController;
import com.mediatek.internal.telephony.dataconnection.MtkDcTracker;
import com.mediatek.internal.telephony.dataconnection.MtkTelephonyNetworkFactory;
import com.mediatek.internal.telephony.dataconnection.MtkDcHelper;
import com.mediatek.internal.telephony.dataconnection.MtkDcRequest;
import com.mediatek.internal.telephony.datasub.DataSubSelector;
import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistant;
// M: Data Framework - Data Retry enhancement
import com.android.internal.telephony.RetryManager;
import com.mediatek.internal.telephony.MtkRetryManager;

import com.android.internal.telephony.cdma.CdmaSMSDispatcher;
import com.mediatek.internal.telephony.cdma.MtkCdmaInboundSmsHandler;
import com.mediatek.internal.telephony.cdma.MtkCdmaSMSDispatcher;
import com.mediatek.internal.telephony.worldphone.WorldPhoneUtil;
// PHB START
import com.android.internal.telephony.IccInternalInterface;
import com.android.internal.telephony.IccPhoneBookInterfaceManager;
import com.android.internal.telephony.UiccPhoneBookController;
import com.mediatek.internal.telephony.phb.MtkIccPhoneBookInterfaceManager;
import com.mediatek.internal.telephony.phb.MtkUiccPhoneBookController;
// PHB END

// Carrier Express
import com.mediatek.internal.telephony.carrierexpress.CarrierExpressFwkHandler;

// GWSD
import java.lang.reflect.Method;
import dalvik.system.PathClassLoader;

public class MtkTelephonyComponentFactory extends TelephonyComponentFactory {
    private static MtkTelephonyComponentFactory sInstance;

    public static MtkTelephonyComponentFactory getInstance() {
        if (sInstance == null) {
            sInstance = new MtkTelephonyComponentFactory();
        }
        return sInstance;
    }

    @Override
    public GsmCdmaPhone makePhone(Context context, CommandsInterface ci, PhoneNotifier notifier,
            int phoneId, int precisePhoneType,
            TelephonyComponentFactory telephonyComponentFactory) {
        return new MtkGsmCdmaPhone(context, ci, notifier, phoneId, precisePhoneType,
                telephonyComponentFactory);
    }

    @Override
    public RIL makeRil(Context context, int preferredNetworkType, int cdmaSubscription,
            Integer instanceId) {
        return new MtkRIL(context, preferredNetworkType, cdmaSubscription, instanceId);
    }

    @Override
    public ServiceStateTracker makeServiceStateTracker(GsmCdmaPhone phone, CommandsInterface ci) {
        return new MtkServiceStateTracker(phone, ci);
    }

    @Override
    public SubscriptionController makeSubscriptionController(Phone phone) {
        return MtkSubscriptionController.mtkInit(phone);
    }

    @Override
    public SubscriptionController makeSubscriptionController(Context c, CommandsInterface[] ci) {
        return MtkSubscriptionController.mtkInit(c, ci);
    }

    @Override
    public GsmCdmaCallTracker makeGsmCdmaCallTracker(GsmCdmaPhone phone) {
        return new MtkGsmCdmaCallTracker(phone);
    }

    @Override
    public SubscriptionInfoUpdater makeSubscriptionInfoUpdater(Looper looper, Context context,
            Phone[] phone, CommandsInterface[] ci) {
        return new MtkSubscriptionInfoUpdater(looper, context, phone, ci);
    }

    @Override
    public MultiSimSettingController makeMultiSimSettingController(Context context,
            SubscriptionController sc) {
        Rlog.d(LOG_TAG , "makeMultiSimSettingController mtk");
        return new MtkMultiSimSettingController(context, sc);
    }

    @Override
    public CdmaSubscriptionSourceManager
    makeCdmaSubscriptionSourceManager(Context context, CommandsInterface ci, Handler h,
            int what, Object obj) {
        return new MtkCdmaSubscriptionSourceManager(context, ci);
    }

    @Override
    public DefaultPhoneNotifier makeDefaultPhoneNotifier() {
        Rlog.d(LOG_TAG , "makeDefaultPhoneNotifier mtk");
        return new MtkPhoneNotifier();
    }

    // MTK-START: SIM COMMON
    @Override
    public UiccController makeUiccController(Context c, CommandsInterface[] ci) {
        Rlog.d(LOG_TAG , "makeUiccController mtk");
        return new MtkUiccController(c, ci);
    }

    @Override
    public UiccProfile makeUiccProfile(Context context, CommandsInterface ci, IccCardStatus ics,
            int phoneId, UiccCard uiccCard, Object lock) {
        return new MtkUiccProfile(context, ci, ics, phoneId, uiccCard, lock);
    }
    // MTK-END

    @Override
    public void initRadioManager(Context context, int numPhones,
            CommandsInterface[] sCommandsInterfaces) {
        RadioManager.init(context, numPhones, sCommandsInterfaces);
    }

    public CatService makeCatService(CommandsInterface ci, UiccCardApplication ca, IccRecords ir,
            Context context, IccFileHandler fh, UiccProfile uiccProfile, int slotId) {
        /**
         * For CDMA dual mode SIM card,when the phone type is CDMA, need get
         * Uicc application of 3GPP2.
         */
        int phoneType = PhoneConstants.PHONE_TYPE_GSM;
        int subId[] = SubscriptionManager.getSubId(slotId);
        if (subId != null) {
            phoneType = TelephonyManager.getDefault().getCurrentPhoneType(subId[0]);
            MtkCatLog.d("MtkCatService", "makeCatService phoneType : " + phoneType
                    + " slotId: " + slotId + " subId[0]:" + subId[0]);
        }
        if (uiccProfile != null) {
            if (phoneType == PhoneConstants.PHONE_TYPE_CDMA) {
                ca = uiccProfile.getApplication(UiccController.APP_FAM_3GPP2);
            } else {
                ca = uiccProfile.getApplicationIndex(0);
            }
        }
        MtkCatLog.v("MtkCatService", "makeCatService  ca = " + ca);
        if (ci == null || ca == null || ir == null || context == null || fh == null
                || uiccProfile == null) {
            MtkCatLog.e("MtkCatService",
                    "makeCatService exception, will not create MtkCatservice!!!!");
            return null;
        }
        return new MtkCatService(ci, ca, ir, context, fh, uiccProfile, slotId);
    }

    @Override
    public RilMessageDecoder makeRilMessageDecoder(Handler caller, IccFileHandler fh,
            int slotId) {
        return new MtkRilMessageDecoder(caller, fh, slotId);
    }

    @Override
    public CommandParamsFactory makeCommandParamsFactory(RilMessageDecoder caller,
            IccFileHandler fh) {
        return new MtkCommandParamsFactory(caller, fh);
    }

    @Override
    public IconLoader makeIconLoader(Looper looper , IccFileHandler fh) {
        return new MtkIconLoader(looper, fh);
    }

    @Override
    public DcTracker makeDcTracker(Phone phone, int transportType) {
        return new MtkDcTracker(phone, transportType);
    }

    @Override
    public TelephonyNetworkFactory makeTelephonyNetworkFactories(
            SubscriptionMonitor subscriptionMonitor, Looper looper, Phone phone) {
        return new MtkTelephonyNetworkFactory(subscriptionMonitor, looper, phone);
    }

    @Override
    public void makeDcHelper(Context context, Phone[] phones) {
        MtkDcHelper.makeMtkDcHelper(context, phones);
    }

    @Override
    public void makeDataSubSelector(Context context, int numPhones) {
        DataSubSelector.makeDataSubSelector(context, numPhones);
    }

    @Override
    public void makeSmartDataSwitchAssistant(Context context, Phone[] phones) {
        SmartDataSwitchAssistant.makeSmartDataSwitchAssistant(context, phones);
    }

    @Override
    public void makeSuppServManager(Context context, Phone[] phones) {
        MtkSuppServManager ssManager = MtkSuppServManager.makeSuppServManager(context, phones);
        ssManager.init();
    }

    @Override
    public PhoneSwitcher makePhoneSwitcher(int maxActivePhones, int numPhones,
            Context context, SubscriptionController subscriptionController, Looper looper,
            ITelephonyRegistry tr, CommandsInterface[] cis, Phone[] phones) {
        PhoneSwitcher phoneSwitcher = new MtkPhoneSwitcher(maxActivePhones, numPhones, context,
                subscriptionController, looper, tr, cis, phones);
        return phoneSwitcher;
    }

    // SMS [Start]
    public SmsStorageMonitor makeSmsStorageMonitor(Phone phone) {
        return new MtkSmsStorageMonitor(phone);
    }

    public SmsUsageMonitor makeSmsUsageMonitor(Context context) {
        return new MtkSmsUsageMonitor(context);
    }

    public IccSmsInterfaceManager makeIccSmsInterfaceManager(Phone phone) {
        return new MtkIccSmsInterfaceManager(phone);
    }

    /**
     * Create ImsSMSDispatcher
     */
    public ImsSmsDispatcher makeImsSmsDispatcher(Phone phone,
            SmsDispatchersController smsDispatchersController) {
        return new MtkImsSmsDispatcher(phone, smsDispatchersController);
    }
    /**
     * Create a dispatcher for GSM SMS.
     */
    public GsmSMSDispatcher makeGsmSMSDispatcher(Phone phone,
            SmsDispatchersController smsDispatchersController,
            GsmInboundSmsHandler gsmInboundSmsHandler) {
        return new MtkGsmSMSDispatcher(phone, smsDispatchersController, gsmInboundSmsHandler);
    }

    /**
     * Create a tracker for a single-part SMS.
     */
    public InboundSmsTracker makeInboundSmsTracker(byte[] pdu, long timestamp, int destPort,
            boolean is3gpp2, boolean is3gpp2WapPdu, String address, String displayAddr,
            String messageBody, boolean isClass0) {
        return new MtkInboundSmsTracker(pdu, timestamp, destPort, is3gpp2, is3gpp2WapPdu, address,
                displayAddr, messageBody, isClass0);
    }

    /**
     * Create a tracker for a multi-part SMS.
     */
    public InboundSmsTracker makeInboundSmsTracker(byte[] pdu, long timestamp, int destPort,
            boolean is3gpp2, String address, String displayAddr, int referenceNumber,
            int sequenceNumber, int messageCount, boolean is3gpp2WapPdu, String messageBody,
            boolean isClass0) {
        return new MtkInboundSmsTracker(pdu, timestamp, destPort, is3gpp2, address, displayAddr,
                referenceNumber, sequenceNumber, messageCount, is3gpp2WapPdu, messageBody,
                isClass0);
    }

    /**
     * Create a tracker from a row of raw table
     */
    public InboundSmsTracker makeInboundSmsTracker(Cursor cursor, boolean isCurrentFormat3gpp2) {
        return new MtkInboundSmsTracker(cursor, isCurrentFormat3gpp2);
    }
    /**
     * Create an object of SmsBroadcastUndelivered.
     */
    public void makeSmsBroadcastUndelivered(Context context,
            GsmInboundSmsHandler gsmInboundSmsHandler,
            CdmaInboundSmsHandler cdmaInboundSmsHandler) {
        MtkSmsBroadcastUndelivered.initialize(context, gsmInboundSmsHandler,
                cdmaInboundSmsHandler);
    }
    public WspTypeDecoder makeWspTypeDecoder(byte[] pdu) {
        return new MtkWspTypeDecoder(pdu);
    }

    public WapPushOverSms makeWapPushOverSms(Context context) {
        return new MtkWapPushOverSms(context);
    }

    // Create GsmInboudSmsHandler
    public GsmInboundSmsHandler makeGsmInboundSmsHandler(Context context,
            SmsStorageMonitor storageMonitor, Phone phone) {
        return (GsmInboundSmsHandler)MtkGsmInboundSmsHandler.makeInboundSmsHandler(
                context, storageMonitor, phone);
    }

    public MtkSmsHeader makeSmsHeader() {
        return new MtkSmsHeader();
    }

    // Create MtkGsmCellBroadcastHandler
    public MtkGsmCellBroadcastHandler makeGsmCellBroadcastHandler(Context context,
            Phone phone) {
        return MtkGsmCellBroadcastHandler.makeGsmCellBroadcastHandler(context, phone);
    }

    public SmsDispatchersController makeSmsDispatchersController(Phone phone,
            SmsStorageMonitor storageMonitor,
            SmsUsageMonitor usageMonitor) {
        return new MtkSmsDispatchersController(
                phone, phone.mSmsStorageMonitor, phone.mSmsUsageMonitor);
    }
    // SMS [End]

    @Override
    public ProxyController makeProxyController(Context context, Phone[] phone,
            UiccController uiccController, CommandsInterface[] ci, PhoneSwitcher ps) {
        return new MtkProxyController(context, phone, uiccController, ci, ps);
    }

    @Override
    public CdmaInboundSmsHandler makeCdmaInboundSmsHandler(Context context,
            SmsStorageMonitor storageMonitor, Phone phone, CdmaSMSDispatcher smsDispatcher) {
        return new MtkCdmaInboundSmsHandler(context, storageMonitor, phone, smsDispatcher);
    }

    @Override
    public CdmaSMSDispatcher makeCdmaSMSDispatcher(Phone phone,
            SmsDispatchersController smsDispatchersController) {
        return new MtkCdmaSMSDispatcher(phone, smsDispatchersController);
    }
    /// M: eMBMS feature
    /*
    * Create EmbmsAdaptor
    */
    public void initEmbmsAdaptor(Context context, CommandsInterface[] sCommandsInterfaces) {
        MtkEmbmsAdaptor.getDefault(context, sCommandsInterfaces);
    }
    /// M: eMBMS end

    @Override
    public void makeWorldPhoneManager() {
        WorldPhoneUtil.makeWorldPhoneManager();
    }

    // PHB START
    @Override
    public IccPhoneBookInterfaceManager makeIccPhoneBookInterfaceManager(Phone phone) {
        Rlog.d(LOG_TAG , "makeIccPhoneBookInterfaceManager mtk");
        return new MtkIccPhoneBookInterfaceManager(phone);
    }
    // PHB END

    public ImsPhoneCallTracker makeImsPhoneCallTracker(ImsPhone imsPhone) {
        return new MtkImsPhoneCallTracker(imsPhone);
    }

    public ImsPhone makeImsPhone(Context context, PhoneNotifier phoneNotifier, Phone defaultPhone) {
        try {
            return new MtkImsPhone(context, phoneNotifier, defaultPhone);
        } catch (Exception e) {
            Rlog.e("TelephonyComponentFactoryEx", "makeImsPhoneExt", e);
            return null;
        }
    }

    public CallManager makeCallManager() {
        return new MtkCallManager();
    }

    /**
    * M: Data Framework - Data Retry enhancement
    * Create a retry manager instance
    */
    @Override
    public RetryManager makeRetryManager(Phone phone, String apnType) {
        return new MtkRetryManager(phone, apnType);
    }

    /**
     * Create a data connection instance
     */
    @Override
    public DataConnection makeDataConnection(Phone phone, String name, int id,
            DcTracker dct, DataServiceManager dataServiceManager,
            DcTesterFailBringUpAll failBringUpAll, DcController dcc) {
        DataConnection dc = new MtkDataConnection(phone, name, id, dct,
                dataServiceManager, failBringUpAll, dcc);
        dc.TCP_BUFFER_SIZES_LTE = "2097152,4194304,8388608,262144,524288,1048576";
        return dc;
    }

    /**
     * Create a dc controller instance
     */
    @Override
    public DcController makeDcController(String name, Phone phone, DcTracker dct,
            DataServiceManager dataServiceManager, Handler handler) {
        return new MtkDcController(name + "-Mtk", phone, dct, dataServiceManager, handler);
    }

    /**
     * Create a dc request instance.
     * @param nr {@link NetworkRequest} describing this request.
     * @param context  the context of the phone process
     *
     * @return the object of MtkDcRequest
     */
    public DcRequest makeDcRequest(NetworkRequest nr, Context context) {
        return new MtkDcRequest(nr, context);
    }

    public ComponentName makeConnectionServiceName() {
        Rlog.d(LOG_TAG , "makeConnectionServiceName mtk");
        return new ComponentName("com.android.phone",
                    "com.mediatek.services.telephony.MtkTelephonyConnectionService");
    }

    @Override
    public void makeNetworkStatusUpdater(Phone[] phones, int numPhones) {
        Rlog.d(LOG_TAG, "Creating NetworkStatusUpdater");
        MtkNetworkStatusUpdater.init(phones, numPhones);
    }

    @Override
    public DeviceStateMonitor makeDeviceStateMonitor(Phone phone) {
        return new MtkDeviceStateMonitor(phone);
    }

    /**
     * initialize carrier express instance .
     */
    @Override
    public void initCarrierExpress() {
        Rlog.d(LOG_TAG, "Creating CarrierExpress");
        CarrierExpressFwkHandler.init();
    }

    @Override
    public void initGwsdService(Context context) {
        // Check telephony add on support property
        if (SystemProperties.get("ro.vendor.mtk_telephony_add_on_policy", "0").equals("0") &&
                SystemProperties.get("ro.vendor.mtk_gwsd_support", "0").equals("1")) {
            String className = "com.mediatek.gwsd.service.GwsdService";
            String classPackage = "/system/framework/mediatek-gwsd.jar";
            Class<?> clazz = null;
            try{
                PathClassLoader classLoader = new PathClassLoader(classPackage,
                        ClassLoader.getSystemClassLoader());
                clazz = Class.forName(className, false, classLoader);
                Rlog.d(LOG_TAG, "class = " + clazz);
                Method method = clazz.getMethod("getInstance", Context.class);
                method.invoke(clazz, context);
            } catch(Exception e) {
                Rlog.e(LOG_TAG, Log.getStackTraceString(e));
            }
        }
    }
}
