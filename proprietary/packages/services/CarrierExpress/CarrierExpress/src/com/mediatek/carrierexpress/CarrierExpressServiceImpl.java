package com.mediatek.carrierexpress;

import android.app.Activity;
import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.AlertDialog;
import android.app.AppGlobals;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.IPackageManager;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telecom.TelecomManager;
import android.telephony.RadioAccessFamily;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.accessibility.AccessibilityEvent;
import android.widget.Toast;

import java.util.Arrays;

//import com.android.internal.notification.SystemNotificationChannels;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import static com.android.internal.util.Preconditions.checkNotNull;
import com.mediatek.carrierexpress.R;
import com.mediatek.common.carrierexpress.ICarrierExpressService;
import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.internal.telephony.IMtkTelephonyEx;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * CarrierExpress Service.
 */
public class CarrierExpressServiceImpl extends ICarrierExpressService.Stub {

    // TO DO: Minimize class elements
    private final String TAG = "CarrierExpressServiceImpl";
    private final boolean DEBUG = !Build.TYPE.equals("user");
    private final boolean TESTING_PURPOSE = true; // remove or set false when given to customer
    private Context mContext;
    private int mConfigState = CarrierExpressManager.SRV_CONFIG_STATE_INIT;
    private PackageManager mPm;
    private ActivityManager mAm;
    private AlertDialog mDialog;
    private ProgressDialog mWaitingDialog;
    private static final String SYSTEM_PATH = "/system/usp";
    private static final String CUSTOM_PATH = "/custom/usp";
    private static final String CARRIEREXPRESS_INFO_FILE = "usp-info.txt";
    private static Map<String, String> sOperatorMapInfo;
    private static Map<String, String> mOperatorResData;
    // This is for previously configured operator code, that is sent with broadcast
    // after configuration is done.
    private static String mPrevOptr;
    private int mSystemUserId;
    private int mCheckAppStateCount;
    private final IPackageManager mIPackageManager;

    static {
        sOperatorMapInfo = new HashMap<String, String>();
        sOperatorMapInfo.put("OP00", "Open market");
        sOperatorMapInfo.put("la_OP15", "Movistar");
        sOperatorMapInfo.put("OP01", "CMCC");
        sOperatorMapInfo.put("OP02", "CU");
        sOperatorMapInfo.put("OP03", "Orange");
        sOperatorMapInfo.put("OP05", "TMO EU");
        sOperatorMapInfo.put("OP06", "Vodafone");
        sOperatorMapInfo.put("OP07", "AT&T");
        sOperatorMapInfo.put("OP08", "TMO US");
        sOperatorMapInfo.put("OP09", "CT");
        sOperatorMapInfo.put("OP11", "H3G");
        sOperatorMapInfo.put("OP12", "Verizon");
        sOperatorMapInfo.put("OP15", "Telefonica");
        sOperatorMapInfo.put("OP16", "EE");
        sOperatorMapInfo.put("OP17", "DoCoMo");
        sOperatorMapInfo.put("OP18", "Reliance");
        sOperatorMapInfo.put("OP19", "Telstra");
        sOperatorMapInfo.put("OP20", "Sprint");
        sOperatorMapInfo.put("OP50", "Softbank");
        sOperatorMapInfo.put("OP100", "CSL");
        sOperatorMapInfo.put("OP101", "PCCW");
        sOperatorMapInfo.put("OP102", "SmarTone");
        sOperatorMapInfo.put("OP103", "SingTel");
        sOperatorMapInfo.put("OP104", "Starhub");
        sOperatorMapInfo.put("OP105", "AMX");
        sOperatorMapInfo.put("OP106", "3HK");
        sOperatorMapInfo.put("OP107", "SFR");
        sOperatorMapInfo.put("OP108", "TWN");
        sOperatorMapInfo.put("OP109", "CHT");
        sOperatorMapInfo.put("OP110", "FET");
        sOperatorMapInfo.put("OP112", "TelCel");
        sOperatorMapInfo.put("OP113", "Beeline");
        sOperatorMapInfo.put("OP114", "KT");
        sOperatorMapInfo.put("OP115", "SKT");
        sOperatorMapInfo.put("OP116", "U+");
        sOperatorMapInfo.put("OP117", "Smartfren");
        sOperatorMapInfo.put("OP118", "YTL");
        sOperatorMapInfo.put("OP119", "Natcom");
        sOperatorMapInfo.put("OP120", "Claro");
        sOperatorMapInfo.put("OP121", "Bell");
        sOperatorMapInfo.put("OP122", "AIS");
        sOperatorMapInfo.put("OP124", "APTG");
        sOperatorMapInfo.put("OP125", "DTAC");
        sOperatorMapInfo.put("OP126", "Avea");
        sOperatorMapInfo.put("OP127", "Megafon");
        sOperatorMapInfo.put("OP128", "DNA");
        sOperatorMapInfo.put("OP129", "KDDI");
        sOperatorMapInfo.put("OP130", "TIM");
        sOperatorMapInfo.put("OP131", "TrueMove");
        sOperatorMapInfo.put("OP132", "Movistar");
        sOperatorMapInfo.put("OP133", "DU");
        sOperatorMapInfo.put("OP134", "Elisa");
        sOperatorMapInfo.put("OP135", "MTS");
        sOperatorMapInfo.put("OP136", "Entel");
        sOperatorMapInfo.put("OP137", "TELE2");
        sOperatorMapInfo.put("OP139", "Bouygues");
        sOperatorMapInfo.put("OP140", "MTN");
        sOperatorMapInfo.put("OP141", "Cell C");
        sOperatorMapInfo.put("OP142", "Vodacom");
        sOperatorMapInfo.put("OP143", "Turkcell");
        sOperatorMapInfo.put("OP144", "Smile");
        sOperatorMapInfo.put("OP145", "Cricket");
        sOperatorMapInfo.put("OP146", "Etisalat");
        sOperatorMapInfo.put("OP147", "Airtel");
        sOperatorMapInfo.put("OP148", "Seatel");
        sOperatorMapInfo.put("OP149", "CMHK");
        sOperatorMapInfo.put("OP150", "Swisscom AG");
        sOperatorMapInfo.put("OP151", "M1");
        sOperatorMapInfo.put("OP152", "Optus");
        sOperatorMapInfo.put("OP153", "VHA");
        sOperatorMapInfo.put("OP154", "Telia");
        sOperatorMapInfo.put("OP155", "Digi");
        sOperatorMapInfo.put("OP156", "Telenor");
        sOperatorMapInfo.put("OP157", "Telus");
        sOperatorMapInfo.put("OP158", "Zain");
        sOperatorMapInfo.put("OP159", "STC");
        sOperatorMapInfo.put("OP160", "Webe");
        sOperatorMapInfo.put("OP161", "Play");
        sOperatorMapInfo.put("OP162", "Freedom");
        sOperatorMapInfo.put("OP163", "Dialog");
        sOperatorMapInfo.put("OP164", "Telenet");
        sOperatorMapInfo.put("OP165", "Sunrise");
        sOperatorMapInfo.put("OP166", "Salt");
        sOperatorMapInfo.put("OP168", "Meteor");
        sOperatorMapInfo.put("OP169", "VipNet");
        sOperatorMapInfo.put("OP170", "Partner");
        sOperatorMapInfo.put("OP171", "WOM");
        sOperatorMapInfo.put("OP172", "Altice");
        sOperatorMapInfo.put("OP173", "ALIV");
        sOperatorMapInfo.put("OP175", "TDC");
        sOperatorMapInfo.put("OP176", "Tstar");
        sOperatorMapInfo.put("OP177", "TelecomEgypt");
        sOperatorMapInfo.put("OP178", "Smart");
        sOperatorMapInfo.put("OP179", "Allai Newroz Telecom");
        sOperatorMapInfo.put("OP180", "Swazi Mobile");
        sOperatorMapInfo.put("OP181", "Telkom South Africa");
        sOperatorMapInfo.put("OP182", "Proximus");
        sOperatorMapInfo.put("OP183", "Personal");
        sOperatorMapInfo.put("OP184", "Viettel");
        sOperatorMapInfo.put("OP185", "2degrees");
        sOperatorMapInfo.put("OP186", "Idea");
        sOperatorMapInfo.put("OP187", "Safaricom");
        sOperatorMapInfo.put("OP188", "A1");
        sOperatorMapInfo.put("OP189", "UMOBILE");
        sOperatorMapInfo.put("OP190", "Telkom Kenya");
        sOperatorMapInfo.put("OP191", "Grameenphone");
        sOperatorMapInfo.put("OP192", "Robi");
        sOperatorMapInfo.put("OP193", "Banglalink");
        sOperatorMapInfo.put("OP194", "TeleTalk");
        sOperatorMapInfo.put("OP195", "O2");
        sOperatorMapInfo.put("OP196", "FirstNet");
        sOperatorMapInfo.put("OP197", "AT&T Mexico");
        sOperatorMapInfo.put("OP198", "Sberbank");
        sOperatorMapInfo.put("OP199", "Vivacom");
        sOperatorMapInfo.put("OP200", "JTL");
        sOperatorMapInfo.put("OP201", "KPN");
        sOperatorMapInfo.put("OP202", "Spark");
        sOperatorMapInfo.put("OP203", "NOS");
        sOperatorMapInfo.put("OP204", "Rogers");
        sOperatorMapInfo.put("OP205", "Altan");
        sOperatorMapInfo.put("OP206", "Sky");
        sOperatorMapInfo.put("OP207", "CTM");
        sOperatorMapInfo.put("OP208", "3 Macau");
        sOperatorMapInfo.put("OP209", "Avantel");
        sOperatorMapInfo.put("OP210", "BSNL");
        sOperatorMapInfo.put("OP211", "C&W");
        sOperatorMapInfo.put("OP212", "Open Mobile");
        sOperatorMapInfo.put("OP213", "Motiv");
        sOperatorMapInfo.put("OP214", "IDC");
        sOperatorMapInfo.put("OP215", "Cellcard");
        sOperatorMapInfo.put("OP216", "Zong");
        sOperatorMapInfo.put("OP217", "Telekom");
        sOperatorMapInfo.put("OP218", "Rakuten");
        sOperatorMapInfo.put("OP219", "Mytel");
        sOperatorMapInfo.put("OP220", "Tigo");
        sOperatorMapInfo.put("OP221", "TPG");
        sOperatorMapInfo.put("OP222", "ACG");
        sOperatorMapInfo.put("OP223", "Indosat Ooredoo");
        sOperatorMapInfo.put("OP224", "Telkomsel");
        sOperatorMapInfo.put("OP225", "XL");
        sOperatorMapInfo.put("OP226", "GrameenPhone");
        sOperatorMapInfo.put("OP227", "Wind");
        sOperatorMapInfo.put("OP228", "Celcom");
        sOperatorMapInfo.put("OP229", "Claro");
        sOperatorMapInfo.put("OP230", "Entel");
        sOperatorMapInfo.put("OP231", "Maxis");
        sOperatorMapInfo.put("OP232", "Mobitel");
        sOperatorMapInfo.put("OP233", "Pelephone");
        sOperatorMapInfo.put("OP234", "Plus");
        sOperatorMapInfo.put("OP235", "VIP");
        sOperatorMapInfo.put("OP236", "USCC");
        sOperatorMapInfo.put("OP1001", "OP1001 Lab");
        sOperatorMapInfo.put("OP1002", "OP1002 Lab");
        sOperatorMapInfo.put("OP1003", "OP1003 Lab");
        sOperatorMapInfo.put("OP1004", "OP1004 Lab");
    }

    // Do not reset below process when switch OP confiuration
    private static final String[] IGNORE_PACKAGE_LIST = {
        "system",
        "com.mediatek.carrierexpress",
        "com.google.android.configupdater",
        "com.google.android.backuptransport",
        "com.google.android.ext.services",
        "com.google.android.ext.shared",
        "com.google.android.feedback",
        "com.google.android.onetimeinitializer",
        "com.google.android.packageinstaller",
        "com.google.android.partnersetup",
        "com.google.android.printservice.recommendation",
        "com.google.android.gsf",
        "com.google.android.syncadapters.calendar",
        "com.google.android.syncadapters.contacts",
        "com.google.android.tts",
        "com.google.android.gms",
        "com.android.vending",
        "com.google.android.setupwizard",
        "com.google.android.webview",
        "com.android.chrome",
        "com.google.android.apps.docs",
        "com.google.android.gm",
        "com.google.android.talk",
        "com.google.android.apps.maps",
        "com.google.android.music",
        "com.google.android.apps.photos",
        "com.google.android.googlequicksearchbox",
        "com.google.android.videos",
        "com.google.android.youtube",
        "com.google.android.inputmethod.latin",
        "com.google.android.calendar",
        "com.android.phone",
        "com.android.stk",
        "com.android.providers.telephony",
        "com.mediatek.ims",
        "com.mediatek.wfo.impl",
        "com.android.systemui"
    };

    // The processes which cannot handle runtime OP switch scenario by themselves
    // They need CXP help to stop them.
    private static final String[] NEED_STOP_PROCESS = {
        "com.android.mms",
        "com.android.dialer"
    };

    // The properties which only SET/GET in SYSTEM, need CXP to set
    private static final String[] PROPERTY_NEED_CXP_SET_SYSTEM_ONLY = {
        "persist.vendor.mtk_rtt_support",
        "persist.vendor.vzw_device_type",
        "persist.vendor.mtk_rcs_support",
        "persist.vendor.mtk_uce_support"
    };

    private List<String> mPendingEnableDisableReq = new ArrayList<String>();
    // mOpList shall contain list of operators for carrier express pack.
    private List<String> mOpList = new ArrayList<String>();
    // Old method: SIM application state changed, check "gsm.sim.operator.numeric"
    // New method: SIM state changed, check "vendor.gsm.ril.uicc.mccmnc"
    // or "vendor.cdma.ril.uicc.mccmnc"
    private static final String PROP_GSM_SIM_OPERATOR_NUMERIC = "vendor.gsm.ril.uicc.mccmnc";
    private static final String PROP_CDMA_SIM_OPERATOR_NUMERIC = "vendor.cdma.ril.uicc.mccmnc";
    private static final String PROP_PERSIST_BOOTANIM_MNC = "persist.vendor.bootanim.mnc";
    private static final String PROP_PERSIST_OPERATOR_OPTR = "persist.vendor.operator.optr";
    private static final String PROP_PERSIST_OPERATOR_SPEC = "persist.vendor.operator.spec";
    private static final String PROP_PERSIST_OPERATOR_SEG = "persist.vendor.operator.seg";
    // This PROP_CXP_CONFIG_CTRL is used internally by service to keep some control flags
    // one for first boot, second for first valid sim switch
    private static final String PROP_CXP_CONFIG_CTRL = "persist.vendor.mtk_usp_cfg_ctrl";
    private static final int MAX_AT_CMD_RESPONSE = 2048;

    private static final int PROP_CFG_CTRL_FLAG_NOT_FIRST_BOOT = 1;
    private static final int PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE = 2;
    private static final int PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID = 3;
    private static final int PROP_CFG_CTRL_FLAG_POPUP_HANDLED = 4;
    private static final int PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT = 5;
    private static final int CARRIEREXPRESS_NOTIFICATION = 0x10;
    private static final String CARRIEREXPRESS_CHANNEL_ID = "cxp_channel";

    private MyHandler mUiHandler;

    private TaskHandler mTaskHandler;
    private CarrierExpressServiceImplExt mCarrierExpressServiceExt;
    private NotificationManager mNotificationManager;
    private TelephonyManager mTelephonyManager;
    private String[] mOptrSlot = new String[2];
    private String[] mSlotSimState = new String[2];
    private boolean mDualSimPresent;
    // This variable is for phone full reboot
    private boolean mIsRebootRequired;
    private static final Bundle sNotificationBundle = new Bundle();

    /**
     * constructor of CarrierExpress service.
     * @param context from system server
     */
    public CarrierExpressServiceImpl(Context context) {
        Log.d(TAG, "CarrierExpressServiceImpl");
        mContext = checkNotNull(context, "missing Context");
        mOperatorResData = new HashMap<String, String>();
        //we always register SIM change as user may swap it
        IntentFilter filter = new IntentFilter();
        filter.addAction(CarrierExpressManager.ACTION_USER_NOTIFICATION_INTENT);
        filter.addAction(Intent.ACTION_USER_SWITCHED);
        filter.addAction(Intent.ACTION_SIM_STATE_CHANGED);
        mContext.registerReceiver(mIntentReceiver, filter);
        mUiHandler = new MyHandler();
        mTaskHandler = null;
        mDialog = null;
        mCarrierExpressServiceExt = new CarrierExpressServiceImplExt(context, this);
        mOptrSlot[0] = "";
        mOptrSlot[1] = "";
        mDualSimPresent = false;
        mIsRebootRequired = false;
        mSystemUserId = -1;
        mCheckAppStateCount = 0;
        mIPackageManager = AppGlobals.getPackageManager();
        init();
    }

    /**
     *
     * i function for service.
     */
    private void init() {
        Log.d(TAG, "start");
        /* start a new thread that handles configuration jobs */
        mNotificationManager =
                (NotificationManager) mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        createNotificationChannel();
        mTelephonyManager = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mPm = mContext.getPackageManager();
        HandlerThread ht = new HandlerThread("HandlerThread");
        ht.start();
        mTaskHandler = new TaskHandler(ht.getLooper());
        // set reboot flag false. i.e. reset this flag. as rebooted phone correctly
        setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT, false, null, null);

        if (getConfigCtrlFlag(PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID, null, null)) {
            // Here only comes when last configuration is invalid
            String opPack = getActiveOpPackInternal();
            String subId = SystemProperties.get("persist.vendor.operator.subid", "0");
            if (!isOperatorValid(opPack) || subId.equals("-1")) {
                Log.d(TAG, "Operator pack not valid: " + opPack);
                setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID, false, null, null);
            } else {
                Log.d(TAG, "start reconfiguring as last config was not complete");
                startConfiguringOpPack(opPack, subId, true);
                return;
            }
        }
        // Configure here for first time boot
        if (!getConfigCtrlFlag(PROP_CFG_CTRL_FLAG_NOT_FIRST_BOOT, null, null)) {
            Log.d(TAG, "send START_FIRST_CONFIG message");
            mTaskHandler.sendMessage(mTaskHandler.obtainMessage(TaskHandler.START_FIRST_CONFIG));
        }

        if(!isCarrierExpressSupported()) {
            Log.i(TAG, "Carrier Express not supported !");
            return;
        }

        String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
        if (simSwitchMode != null && simSwitchMode.equals("3")) {
            Log.d(TAG, "sim switch not allowed mode is: " + simSwitchMode);
            return;
        }
        // Configure here as early as possible from early telephony read (TER)
        int phoneCount = mTelephonyManager.getPhoneCount();
        if (phoneCount == 2) {
            Log.d(TAG, "Early detect configuration skipped as phoneCount: " + phoneCount);
            return;
        }
        String mccMnc = readMCCMNCFromProperty(true,0);
        if (mccMnc.length() < 5) {
            Log.d(TAG, "Invalid mccMnc from TER boot animation property:" + mccMnc);
            //Log.d(TAG, "Invalid mccMnc: " + mccMnc + "scheduled after some time");
            // reschedule it to read after some time, as failed from ter, may be ter not ready
            //mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
            //        TaskHandler.EARLY_READ_FAILED), 500);
            return;
        }
        String optr = getOperatorPackForSim(mccMnc);
        detectOperatorSwitching(getRefinedOperator(optr), mccMnc);
    }

    private static boolean isCarrierExpressSupported() {
        boolean result = false;

        if (!"no".equals(SystemProperties.get("ro.vendor.mtk_carrierexpress_pack", "no"))) {
            result = true;
        }
        return result;
    }

    boolean isDualSimSupported(String optr) {
        // Due to android O1 rule
        // CXP only support the OP switch without SIM confgiuration change
        return false;
    }

    private void setPhoneRebootFlag(String switchableOptr) {
        String activeOptr = getActiveOpPackInternal();
        //check if operator support dual SIM
        mIsRebootRequired = false;
        boolean newStatus = isDualSimSupported(switchableOptr);
        boolean oldStatus = isDualSimSupported(activeOptr);
        if ((newStatus == true && oldStatus == false) ||
                (newStatus == false && oldStatus == true)) {
            mIsRebootRequired = true;
            Log.d(TAG, "setPhoneRebootFlag TRUE: " + switchableOptr);
        }
    }

    void detectOperatorSwitching(String optr, String mccMnc) {
        Log.d(TAG, "detectOperatorSwitching: " + optr);
        // remove any notification if present
        mNotificationManager.cancel(CARRIEREXPRESS_NOTIFICATION);
        if (mCarrierExpressServiceExt.isToPreventOperatorSwitching(optr)) {
            Log.d(TAG, "isToPreventOperatorSwitching: true");
            return;
        }

        int opSwitchability = checkOperatorSwitchability(optr);
        if (opSwitchability < 0) {
            // operator switching prevented
            return;
        } else if (opSwitchability == 0) {
            // operator not valid, but needed for sub Id check for current operator
            String activeOptr = getActiveOpPackInternal();
            if (activeOptr != null && activeOptr.length() > 2) {
                mCarrierExpressServiceExt.instantiateOperatorClass(activeOptr, mccMnc);
                String subId = mCarrierExpressServiceExt.getOperatorSubId();
                if (subId.equals("-1")) {
                    // need to wait for subId to get ready
                    Log.d(TAG, "need to wait for subId to get ready");
                    return;
                }
                handleSwitchOperator(activeOptr, subId);
            }
        } else if (opSwitchability == 1) {
            // operator is valid for switching
            mCarrierExpressServiceExt.instantiateOperatorClass(optr, mccMnc);
            String subId = mCarrierExpressServiceExt.getOperatorSubId();
            if (subId.equals("-1")) {
                // need to wait for subId to get ready
                Log.d(TAG, "need to wait for subId to get ready");
                return;
            }
            handleSwitchOperator(optr, subId);
        }
    }

    void firstBootConfigure() {
        Log.d(TAG, "firstBootConfigure");
        // currently we are assuming that followings are default from load,
        // so no action required for them
        //   1. system properties from cip-build.prop
        //   2. sbp md from cip-build.prop
        //   3. lk / boot logo from default inbuilt behaviour
        //   4. install/uninstall apks from default behaviour*(TBD)

        // first config is either OM config or OP config
        // here only configure for enable/disable apks
        String optr = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);
        if (optr == null || optr.length() <= 0) {
            Log.d(TAG, "firstBootConfigure: OM config");
            enabledDisableApps("OP00");
        } else {
            Log.d(TAG, "firstBootConfigure: OP config" + optr);
            enabledDisableApps(optr);
            String subId = SystemProperties.get("persist.vendor.operator.subid", "0");
            // To handle when default op is set and user inserted  OM SIM
            setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_POPUP_HANDLED, true, optr, subId);
        }
    }

    synchronized boolean getConfigCtrlFlag(int prop, String optr, String subId) {
        int propValue = SystemProperties.getInt(PROP_CXP_CONFIG_CTRL, 0);
        Log.d(TAG,"getConfigCtrlFlag(),get config control:" + prop + ", value:" + propValue);
        switch (prop) {
        case PROP_CFG_CTRL_FLAG_NOT_FIRST_BOOT:
            return ((propValue & 0x1) == 1);
        case PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE:
            return ((propValue & 0x2) == 2);
        case PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID:
            return ((propValue & 0x4) == 4);
        case PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT:
            return ((propValue & 0x8) == 8);
        case PROP_CFG_CTRL_FLAG_POPUP_HANDLED:
            int numStored = (propValue & 0xFFFFFF00) >> 8;
            String numOptrStr = optr.substring(2, optr.length());
            try {
                int numOptr = Integer.parseInt(numOptrStr);
                int sub = Integer.parseInt(subId);
                int numCur = numOptr << 8 | sub;
                Log.d(TAG, "saved: " + numStored + "cur: " + numCur);
                if (numCur == numStored) {
                    return true;
                }
            } catch (NumberFormatException e) {
                Log.d(TAG, "getConfigCtrlFlag: 2" + e.toString());
            }
            break;
        default:
            Log.d(TAG, "getConfigCtrlFlag: default: " + prop);
            break;
        }
        return false;
    }

    synchronized void setConfigCtrlFlag(int prop, boolean flag, String optr, String subId) {
        int propValue = SystemProperties.getInt(PROP_CXP_CONFIG_CTRL, 0);
        switch (prop) {
        case PROP_CFG_CTRL_FLAG_NOT_FIRST_BOOT:
            propValue &= 0xFFFFFFFE; //reset this with 1st bit
            if (flag) {
                propValue |= 0x1;
            }
            break;
        case PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE:
            propValue &= 0xFFFFFFFD; //reset this with 2nd bit
            if (flag) {
                propValue |= 0x2;
            }
            break;
        case PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID:
            propValue &= 0xFFFFFFFB; //reset this with 3rd bit
            if (flag) {
                propValue |= 0x4;
            }
            break;
        case PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT:
            propValue &= 0xFFFFFFF7; //reset this with 4th bit
            if (flag) {
                propValue |= 0x8;
            }
            break;
        case PROP_CFG_CTRL_FLAG_POPUP_HANDLED:
            if (optr != null && optr.length() >= 3) {
                String numStr = optr.substring(2, optr.length());
                try {
                    int num = Integer.parseInt(numStr);
                    int sub = Integer.parseInt(subId);
                    propValue &= 0x000000FF; //reset this with last 24(16(optr) + 8(sub)) bits
                    propValue |= (num << 16 | sub << 8);
                } catch (NumberFormatException e) {
                    Log.d(TAG, "setConfigCtrlFlag: 2" + e.toString());
                }
            }
            break;
        default:
            Log.d(TAG, "setConfigCtrlFlag: default: " + prop);
            break;
        }
        Log.d(TAG, "setConfigCtrlFlag(),set config control:" + prop + ", value:" + propValue);
        SystemProperties.set(PROP_CXP_CONFIG_CTRL, "" + propValue);
    }

    boolean isFirstValidSimConfigured() {
        String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
        if (simSwitchMode != null && simSwitchMode.equals("2")
                && getConfigCtrlFlag(PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE, null, null)) {
            return true;
        }
        return false;
    }

    int checkOperatorSwitchability(String optr) {
        Log.d(TAG, "checkOperatorSwitchability: " + optr);
        if (isFirstValidSimConfigured()) {
            Log.d(TAG, "isFirstValidSimConfigured: true");
            return -1; // not needed further
        }
        if (!isOperatorValid(optr)) {
            Log.d(TAG, "Operator pack not valid: " + optr);
            // Need to check if current active operator sub id need to update
            return 0; // Operator is not valid
        }
        return 1; // operator is valid
    }

    void handleSwitchOperator(String optr, String subId) {
        Log.d(TAG, "handleSwitchOperator OPTR: " + optr + " SUBID: " + subId);
        // optr shall not be invalid (i.e. null or empty) and subid shall be >= 0
        if (optr.equals(getActiveOpPackInternal()) &&
                SystemProperties.get("persist.vendor.operator.subid", "0").equals(subId)) {
            Log.d(TAG, "same active operator: " + optr);
            // add first valid sim switch mod set, still as sim detected of same optr
            String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
            if (simSwitchMode != null && simSwitchMode.equals("2")) {
                setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE, true, null, null);
                Log.d(TAG, "set first valid sim configured");
            }
            // Check if only for secondary SIM is need to be configured
            Log.d(TAG, "mDualSimPresent: " + mDualSimPresent);
            if (mDualSimPresent == true) {
                int slotS = (getMainCapabilitySlotId() == 0) ? 1 : 0;
                String opS = mOptrSlot[slotS];
                if (opS != null && opS.length() > 2 &&
                        !opS.equals(SystemProperties.get("persist.vendor.operator.optr_1", ""))) {
                    Log.d(TAG, "only for secondary SIM is need to be configured optr" + opS);
                    mPrevOptr = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);
                    setDualSimOptrProperties(optr);
                    enablePluginApkDualSim();
                    sendBroadcastForConfigFinish(true);
                    // reset the list here as not registered the receiver
                    mPendingEnableDisableReq.clear();
                }
            }
            return;
        }
        if (mConfigState != CarrierExpressManager.SRV_CONFIG_STATE_WAIT) {
            // add for first valid sim switch mod set, it will execute only for first time
            String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
            if (simSwitchMode != null && simSwitchMode.equals("2")) {
                setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_FIRST_SIM_ONLY_DONE, true, null, null);
                Log.d(TAG, "set first valid sim configured");
                setPhoneRebootFlag(optr);
                startConfiguringOpPack(optr, subId, false);
            } else if ("4".equals(simSwitchMode)
                && getConfigCtrlFlag(PROP_CFG_CTRL_FLAG_POPUP_HANDLED, optr, subId)) {
                // The notification message was only used on "check SIM every time" switch mode
                if (!optr.equals(getActiveOpPackInternal())) {
                    showNotificationMessage(optr, subId);
                }
            } else {
                if (mDialog != null && mDialog.isShowing()) {
                    Log.d(TAG, "configuration dialog already being displayed");
                    return;
                }
                setPhoneRebootFlag(optr);
                // simSwitchMode = 1: auto switch mode
                if ("1".equals(simSwitchMode)) {
                    Log.d(TAG, "User popup skipped as auto switch mode");
                    startConfiguringOpPack(optr, subId, false);
                } else {
                    // simSwitchMode = 4: check SIM every time
                    new CarrierExpressUserDialog(optr, subId).showDialog();
                }
            }
        }
    }
    private void createNotificationChannel() {
        NotificationChannel channel = new NotificationChannel(CARRIEREXPRESS_CHANNEL_ID,
                "Alert",
                NotificationManager.IMPORTANCE_LOW);
        channel.setDescription("Notification for operator configuration switching");
        mNotificationManager.createNotificationChannel(channel);
    }
    void showNotificationMessage(String optr, String subId) {
        String title = mContext.getResources().getString(
                        R.string.usp_notif_content_title,
                        getOperatorNameFromPack(optr));
        String text = mContext.getResources().getString(
                        R.string.usp_notif_content_text,
                        getOperatorNameFromPack(optr));
        if (sNotificationBundle.isEmpty()) {
                sNotificationBundle.putString(Notification.EXTRA_SUBSTITUTE_APP_NAME,
                        "Operator configuration");
            }
        //Notification noti = new Notification.Builder(mContext, SystemNotificationChannels.ALERTS)
        Notification noti = new Notification.Builder(mContext)
                .setContentTitle(title)
                .addExtras(sNotificationBundle)
                .setContentText(text)
                .setSmallIcon(com.android.internal.R.drawable.stat_sys_adb)
                .setVisibility(Notification.VISIBILITY_SECRET)
                .setChannel(CARRIEREXPRESS_CHANNEL_ID)
                .build();
        Intent intent = new Intent(CarrierExpressManager.ACTION_USER_NOTIFICATION_INTENT);
        intent.putExtra("OPTR", optr);
        intent.putExtra("SUBID", subId);
        noti.contentIntent = PendingIntent.getBroadcast(mContext, 0, intent,
                PendingIntent.FLAG_CANCEL_CURRENT);
        noti.flags |= Notification.FLAG_NO_CLEAR;
        mNotificationManager.notify(CARRIEREXPRESS_NOTIFICATION, noti);
        Log.d(TAG, "showing CarrierExpress mismatch notification: " + optr);
    }

    @Override
    public String getActiveOpPack() {
        // Here we are using Md sbp value to check current operator not
        // persist.vendor.operator.optr for handling operator OM type behaviour
        String optr = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);
        String sbp = SystemProperties.get("persist.vendor.mtk_usp_md_sbp_code");
        if (optr != null && optr.length() > 0) {
            return optr;
        } else if (sbp != null && sbp.length() > 1) {
            // here we are caring if its single digit then
            // its not a valid sbp code or 0
            return "OP" + sbp;
        }
        return (optr == null) ? "" : optr;
    }

    @Override
    public String getOpPackFromSimInfo(String mccMnc) {
        if (mccMnc != null && mccMnc.length() > 0) {
            String optr = getOperatorPackForSim(mccMnc);
            return (isOperatorValid(optr) == true) ? optr : "";
        }
        return "";
    }

    @Override
    public void setOpPackActive(String op, String opSubId, int mainSlot) {
        Log.i(TAG, "setOpPackActive: " + op + " sub id: " + opSubId + " mainSlot: " + mainSlot);
        if(!isCarrierExpressSupported()) {
            Log.i(TAG, "Carrier Express not supported !");
            return;
        }
        if (op == null || opSubId == null || (mainSlot > mTelephonyManager.getPhoneCount())) {
            Log.e(TAG, "setOpPackActive: invalid input parameter, just return");
            return;
        }
        if (isInCall() == true) return;

        String opPack = getRefinedOperator(op);
        String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
        String curSubId = SystemProperties.get("persist.vendor.operator.subid", "0");

        if (simSwitchMode != null && simSwitchMode.equals("2")) {
            Log.d(TAG, "First valid sim is enabled: ");
            return;
        }
        if (!isOperatorValid(opPack)) {
            Log.d(TAG, "Operator pack not valid: " + opPack);
            return;
        }
        if (opPack.equals(getActiveOpPackInternal()) &&
                curSubId.equals(opSubId)) {
            int phoneCount = mTelephonyManager.getPhoneCount();
            int curMainSlot = SystemProperties.getInt("persist.vendor.mtk_usp_ds_main_slot", 0);
            if (phoneCount < 2 || mainSlot == curMainSlot) {
                Log.d(TAG, "same active operator: " + opPack + " phone count: " + phoneCount);
                return;
            }
        }
        if (mConfigState != CarrierExpressManager.SRV_CONFIG_STATE_WAIT) {
            // remove any notification if present
            mNotificationManager.cancel(CARRIEREXPRESS_NOTIFICATION);
            // set main slot
            SystemProperties.set("persist.vendor.mtk_usp_ds_main_slot", "" + mainSlot);
            // For dual SIM Support
            setDualSimSupportVariables(opPack);
            setPhoneRebootFlag(opPack);
            startConfiguringOpPack(opPack, opSubId, false);
        }
    }

    @Override
    public List getOperatorSubIdList(String opPack) {
        Log.d(TAG, "getOperatorSubIdList: " + opPack);
        return mCarrierExpressServiceExt.getOperatorSubIdList(opPack);
    }

    private String getActiveOpPackInternal() {
        String optr = getActiveOpPack();
        return getRefinedOperator(optr);
    }

    boolean isOperatorValid(String optr) {
        if (optr == null || optr.length() <= 0) {
            Log.d(TAG, "error in operator: " + optr);
            return false;
        } else if (optr.equals("OP00")) {
            return true;
        } else if (getAllOpList().contains(optr) == true) {
            // check for operator pack exists
            return true;
        }
        Log.d(TAG, "Operator not found in all op pack list");
        return false;
    }

    private String getRefinedOperator(String optr) {
        String omSwitchSupDisable = SystemProperties.get("persist.vendor.mtk_usp_om_dis_switch");
        if (omSwitchSupDisable == null || !omSwitchSupDisable.equals("1")) {
            // check for null optr
            if (optr == null || optr.length() <= 0 ||
                    getAllOpList().contains(optr) == false) {
                Log.d(TAG, "getRefinedOperator to OP00 (OM)");
                return new String("OP00");
            }
        }
        return optr;
    }

    List<String> getAllOpList() {
        if (mOpList.isEmpty()) {
            String ops = getRegionalOpPack();
            try {
                String[] opSplit = ops.split(" ");
                for (int count = 0; count < opSplit.length; ++count) {
                    if (opSplit[count] != null && opSplit[count].length() > 0) {
                        int firstUnderscoreIndex = opSplit[count].indexOf("_");
                        mOpList.add(opSplit[count].substring(0, firstUnderscoreIndex));
                    }
                }
            } catch (IndexOutOfBoundsException e) {
                Log.e(TAG, "illegal string passed to splitString: " + e.toString());
            }
        }
        return mOpList;
    }

    @Override
    public Map<String, String> getAllOpPackList() {
        List<String> opList = getAllOpList();
        Map<String, String> operatorMapInfo = new HashMap<String, String>();
        for (int count = 0; count < opList.size(); ++count) {
            operatorMapInfo.put(opList.get(count),
                    getOperatorNameFromPack(opList.get(count)));
        }
        // add OM operator
        String omSwitchSupDisable = SystemProperties.get("persist.vendor.mtk_usp_om_dis_switch");
        if (omSwitchSupDisable == null || !omSwitchSupDisable.equals("1")) {
            operatorMapInfo.put("", getOperatorNameFromPack("OP00"));
        }
        return operatorMapInfo;
    }

    private Map<String, String> getAllOpSpecSegList() {
        String ops = getRegionalOpPack();
        Map<String, String> operatorMapInfo = new HashMap<String, String>();
        try {
            String[] opSplit = ops.split(" ");
            for (int count = 0; count < opSplit.length; ++count) {
                if (opSplit[count] != null && opSplit[count].length() > 0) {
                    int firstUnderscoreIndex = opSplit[count].indexOf("_");
                    operatorMapInfo.put(opSplit[count].substring(0, firstUnderscoreIndex),
                            opSplit[count]);
                }
            }
        } catch (IndexOutOfBoundsException e) {
            Log.e(TAG, "illegal string passed to splitString: " + e.toString());
        }
        return operatorMapInfo;
    }

    private String getOperatorPackForSim(String mccMnc) {
        // fetch operator from framework res
        try {
            int mccMncNum = Integer.parseInt(mccMnc);
            String[] operatorList = mContext.getResources().getStringArray(
                R.array.operator_map_list);
            for (String item : operatorList) {
               String[] opSplit = item.split("\\s*,\\s*");
               if (mccMncNum >= Integer.parseInt(opSplit[0]) &&
                       mccMncNum <= Integer.parseInt(opSplit[1])) {
                   Log.d(TAG, "getOperatorPackForSim optr: " + opSplit[2]);
                   mOperatorResData.put("OP" + opSplit[2] + "_" + mccMnc, item);
                   return "OP" + opSplit[2];
               }
            }
        } catch (Resources.NotFoundException | IndexOutOfBoundsException |
                NumberFormatException e) {
            Log.e(TAG, "getOperatorPackForSim Exception: " + e.toString());
        }
        Log.d(TAG, "getOperatorPackForSim optr NOT FOUND");
        return "";
    }

    private boolean isInCall() {
        if (TelecomManager.from(mContext).isInCall() == true) {
            Log.d(TAG, "call is ongoing so rejecting config start");
            Toast.makeText(mContext, mContext.getResources().getString(
                    R.string.reject_config_during_inCall), Toast.LENGTH_SHORT).show();
            return true;
        }
        return false;
    }

    private void startConfiguringOpPack(String opPack, String subId, boolean isReconfig) {
        Log.d(TAG, "startConfiguringOpPack: " + opPack);
        if (isInCall() == true) return;

        // set state as wait
        mConfigState = CarrierExpressManager.SRV_CONFIG_STATE_WAIT;
        // show reboot dialog animation
        mUiHandler.sendMessage(mUiHandler.obtainMessage(MyHandler.REBOOT_DIALOG,
                (Boolean) isReconfig));

        // Do configuration
        Message startConnfig = mTaskHandler.obtainMessage(TaskHandler.START_CONFIG);
        Bundle data = new Bundle();
        data.putCharSequence("OPTR", opPack);
        data.putCharSequence("SUBID", subId);
        startConnfig.setData(data);
        mTaskHandler.sendMessage(startConnfig);
    }

    private boolean runningConfigurationTask(String opPack, String subId, boolean isSimSwitched) {
        Log.d(TAG, "runningConfigurationTask " + opPack + " subid: " + subId);

        setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_CONFIG_STATE_INVALID, true, null, null);
        if (mIsRebootRequired == true && !isSimSwitched) {
            if (true == triggerSimSwitchChange(opPack, subId)) {
                // wait to complete
                return false;
            }
        }

        // Store previously configured operator
        mPrevOptr = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);

        // Set current OP & sub ID property
        setProperties(opPack, subId);

        // finished
        return true;
    }

    private void setDualSimOptrProperties(String mainOp) {
        int phoneCount = mTelephonyManager.getPhoneCount();
        if (phoneCount == 2) {
            int slotS = (getMainCapabilitySlotId() == 0) ? 1 : 0;
            String optrS = mOptrSlot[slotS];
            Log.d(TAG, "setDualSimOptrProperties optr_1: " + optrS);

            Map<String, String> opSpecMap = getAllOpSpecSegList();
            // set properties for plugin
            if ("OP00".equals(mOptrSlot[0])) {
                SystemProperties.set("persist.vendor.mtk_usp_optr_slot_0", "");
            } else {
                SystemProperties.set("persist.vendor.mtk_usp_optr_slot_0", opSpecMap.get(mOptrSlot[0]));
            }
            if ("OP00".equals(mOptrSlot[1])) {
                SystemProperties.set("persist.vendor.mtk_usp_optr_slot_1", "");
            } else {
                SystemProperties.set("persist.vendor.mtk_usp_optr_slot_1", opSpecMap.get(mOptrSlot[1]));
            }
            Log.d(TAG, "set persist.vendor.mtk_usp_optr_slot_0: " +
                    SystemProperties.get("persist.vendor.mtk_usp_optr_slot_0"));
            Log.d(TAG, "set persist.vendor.mtk_usp_optr_slot_1: " +
                    SystemProperties.get("persist.vendor.mtk_usp_optr_slot_1"));

            // set properties for secondary SIM
            if (mDualSimPresent == true) {
                String optrSpec;
                String optrSeg;
                String opV = opSpecMap.get(optrS);

                if (opV == null || opV.length() <= 0) {
                    SystemProperties.set("persist.vendor.operator.optr_1", "");
                    SystemProperties.set("persist.vendor.operator.spec_1", "");
                    SystemProperties.set("persist.vendor.operator.seg_1", "");
                    Log.d(TAG, "secondary operator is null");
                    return;
                }

                if ("OP00".equals(optrS)) {
                    SystemProperties.set("persist.vendor.operator.optr_1", "");
                } else {
                    SystemProperties.set("persist.vendor.operator.optr_1", optrS);
                }

                try {
                    String[] opSplit = opV.split("_");
                    if (opSplit.length == 3) {
                        SystemProperties.set("persist.vendor.operator.spec_1", opSplit[1]);
                        SystemProperties.set("persist.vendor.operator.seg_1", opSplit[2]);
                    } else {
                        SystemProperties.set("persist.vendor.operator.spec_1", "");
                        SystemProperties.set("persist.vendor.operator.seg_1", "");
                    }

                } catch (IndexOutOfBoundsException e) {
                    Log.e(TAG, "illegal string passed to splitString: " + e.toString());
                    SystemProperties.set("persist.vendor.operator.spec_1", "");
                    SystemProperties.set("persist.vendor.operator.seg_1", "");
                }
            } else {
                SystemProperties.set("persist.vendor.operator.optr_1", "");
                SystemProperties.set("persist.vendor.operator.spec_1", "");
                SystemProperties.set("persist.vendor.operator.seg_1", "");
                Log.d(TAG, "secondary operator is null: mDualSimPresent = " + mDualSimPresent);
            }
        }
    }

    private void showWaitingScreen(boolean isReconfig) {
        ProgressDialog dialog = new ProgressDialog(mContext) {
            // This dialog will consume all events coming in to
            // it, to avoid it trying to do things too early in boot.
            @Override public boolean dispatchKeyEvent(KeyEvent event) {
                return true;
            }
            @Override public boolean dispatchKeyShortcutEvent(KeyEvent event) {
                return true;
            }
            @Override public boolean dispatchTouchEvent(MotionEvent ev) {
                return true;
            }
            @Override public boolean dispatchTrackballEvent(MotionEvent ev) {
                return true;
            }
            @Override public boolean dispatchGenericMotionEvent(MotionEvent ev) {
                return true;
            }
            @Override public boolean dispatchPopulateAccessibilityEvent(
                    AccessibilityEvent event) {
                return true;
            }
        };
        if (isReconfig) {
            dialog.setMessage(mContext.getResources()
                    .getString(R.string.reconfig_dialog_message));
        } else {
            dialog.setMessage(mContext.getResources()
                    .getString(R.string.reboot_dialog_message));
        }
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        //dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        //dialog.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
            //WindowManager.LayoutParams.FLAG_FULLSCREEN);
        //ColorDrawable dialogColor = new ColorDrawable(Color.GRAY);
        //dialogColor.setAlpha(100);
        //dialog.getWindow().setBackgroundDrawable(dialogColor);
        dialog.getWindow().setDimAmount(1);
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
        dialog.getWindow().getAttributes().privateFlags |=
                WindowManager.LayoutParams.PRIVATE_FLAG_SHOW_FOR_ALL_USERS;
        dialog.show();
        mWaitingDialog = dialog;
        Log.d(TAG, "showing WaitingScreen");
    }

    private void checkPendingEnableDisableApp() {
        //keep checking Pending Requests at every .5 seconds
        if(mCheckAppStateCount < 30){
            if (!mPendingEnableDisableReq.isEmpty()) {
                mCheckAppStateCount += 1;
                mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
                        TaskHandler.CHECK_APP_CONFIG_STATUS), 500);
            } else {
                Log.d(TAG, "All Enable Disable completed before " + mCheckAppStateCount +
                        "th check");
                Log.d(TAG, "Going to Reboot Android System");
                mCheckAppStateCount = 0;
                mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
                        TaskHandler.REBOOT_SYSTEM), 500);
            }
        } else {
            Log.e(TAG, "Enable Disable May Have Not Completed");
            mCheckAppStateCount = 0;
            mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
                TaskHandler.REBOOT_SYSTEM), 500);
            //Abort the check process Here
        }
    }

    private void rebootAndroidSystem() {
        mConfigState = CarrierExpressManager.SRV_CONFIG_STATE_IDLE;

        //set optr in native sys env for boot logo and restart zygote from init.rc
        // trigger to restart FW & MD Reset
        SystemProperties.set("persist.vendor.mtk_usp_native_start", "1");
        Log.d(TAG, "phone reboot- IsRebootRequired: " + mIsRebootRequired);
        if (mIsRebootRequired == true) {
            // remove dialog
            if (mWaitingDialog != null) {
                mWaitingDialog.dismiss();
                mWaitingDialog = null;
            }
            //reboot needed - full reboot phone
            PowerManager pm = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);
            // set reboot flag, this is used by native carrier express service
            setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT, true, null, null);
            pm.reboot("");
            Log.e(TAG, "rebooting phone failed");
            // set reboot flag false, here it comes when reboot is failed
            setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_PHONE_FULL_REBOOT, false, null, null);
        }

        // Send specific broadcast intent to reset modem through MtkRIL.resetRadio()
        sendBroadcastForResetModem();

        // Now configuration is done
        // Force stopping all running app processes may cause some unexpected issues.
        // Ex. force sttopping system app(appid=1000), it caused WTF.
        // We do not stop all the running app processes
        // We only stop the specific process which ask CXP help to stop it
        mAm = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningAppProcessInfo service : mAm.getRunningAppProcesses()) {
            for (String pkg : service.pkgList) {
                Log.d(TAG, "process name: " + service.processName
                        + ", package name: " + pkg
                        + ", uid: " + service.uid);
                //if (!checkIfIgnorePackage(pkg)) {
                if (checkNeedStopPackage(pkg)) {
                    Log.d(TAG, "Stop package name: " + pkg);
                    mAm.forceStopPackage(pkg);
                }
            }
        }

        // Activity Manager need a few seconds to dispatch the ordered broadcast intent
        // "OP configure switch finish", so we do not need to wait 10 sec.
        // Braodcast intent for other process to know sothat they can update:
        sendBroadcastForConfigFinish(false);
    }

    private void finishOPConfig() {
        // reset global variables
        mOptrSlot[0] = "";
        mOptrSlot[1] = "";
        if (mWaitingDialog != null) {
            mWaitingDialog.dismiss();
            mWaitingDialog = null;
        }
        Log.d(TAG, "Configuration is finished");
    }

    private boolean checkIfIgnorePackage(String pkgName) {
        // invalid pkgName, just ignore it
        if (pkgName == null || pkgName.length() == 0) {
            return true;
        }

        for (String checkPkgName : IGNORE_PACKAGE_LIST) {
            if (checkPkgName.equals(pkgName)) {
                return true;
            }
        }
        return false;
    }

    private boolean checkNeedStopPackage(String pkgName) {
        // invalid pkgName, just ignore it
        if (pkgName == null || pkgName.length() == 0) {
            return false;
        }

        for (String checkPkgName : NEED_STOP_PROCESS) {
            if (checkPkgName.equals(pkgName)) {
                return true;
            }
        }
        return false;
    }

    void sendBroadcastForConfigFinish(boolean isSecondary) {
        String opP = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);
        String opS = SystemProperties.get("persist.vendor.operator.optr_1");
        String subId = SystemProperties.get("persist.vendor.operator.subid", "0");
        //Broadcast this intent for configuration completion
        Intent intent = new Intent(CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED);
        intent.putExtra("previous_operator", mPrevOptr);
        intent.putExtra("operator", opP);
        intent.putExtra("operator_1", opS);
        intent.putExtra("operator_subid", subId);
        intent.putExtra("primary_slot", getMainCapabilitySlotId());
        intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        if (mDualSimPresent == true) {
            if (isSecondary == true) {
                intent.putExtra("operator_update", "secondary");
            } else {
                intent.putExtra("operator_update", "both");
            }
        } else {
            intent.putExtra("operator_update", "primary");
        }

        // Send ordered broadcast intent
        // Register own BroadcastReceiver as the final receiver of this intent
        mContext.sendOrderedBroadcast(intent, null, mIntentReceiver, null,
            Activity.RESULT_OK, null, null);
        Log.d(TAG, "sendBroadcastForConfigFinish is done");
    }

    private void sendBroadcastForResetModem() {
        Intent intent = new Intent(CarrierExpressManager.ACTION_CXP_RESET_MODEM);
            intent.setPackage("com.android.phone");
        mContext.sendBroadcast(intent);
        Log.d(TAG, "sendBroadcastForResetModem is done");
    }

    protected BroadcastReceiver mEnableDisableRespReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String[] data = intent.getData().toString().split(":");
            String packageName = data[data.length - 1];
            mPendingEnableDisableReq.remove(packageName);
            Log.d(TAG, "mEnableDisableRespReceiver, got response for package name="
                    + packageName);
            Log.d(TAG, "Dump mPendingEnableDisableReq List of Size:"
                    + mPendingEnableDisableReq.size() +
                    Arrays.toString(mPendingEnableDisableReq.toArray()));
            if (mPendingEnableDisableReq.isEmpty()) {
                Log.d(TAG, "mEnableDisableRespReceiver," +
                    "mPendingEnableDisableReq empty So Calling rebootAndroidSystem");
                //rebootAndroidSystem();
                try {
                    mContext.unregisterReceiver(mEnableDisableRespReceiver);
                } catch (IllegalArgumentException e) {
                    Log.d(TAG, "Receiver not registered: " + e.toString());
                }
            }
        }
    };

    protected BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "BroadcastReceiver(), action=" + action);
            //remove any pending message
            if (mTaskHandler.hasMessages(TaskHandler.EARLY_READ_FAILED)) {
                Log.d(TAG, "removeMessages, TaskHandler.EARLY_READ_FAILED");
                mTaskHandler.removeMessages(TaskHandler.EARLY_READ_FAILED);
            }
            // Not Allow Switching when switch mode is 3
            String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
            if (Intent.ACTION_SIM_STATE_CHANGED.equals(action) &&
                    simSwitchMode != null && simSwitchMode.equals("3")) {
                Log.d(TAG, "sim switch not allowed mode is: " + simSwitchMode);

                // Only for lab test, "manual switch mode" + "auto switch to specific OP"
                // optr = OP07, OP08, OP12...etc
                // optr = no => NOT switch OP
                String optr = SystemProperties.get("ro.vendor.mtk_cxp_lab_optr");
                Log.d(TAG, "get ro.vendor.mtk_cxp_lab_optr value is: " + optr);
                if (optr != null && optr.length() > 0 && !optr.equals("no")) {
                    setOpPackActive(optr,"0",0);
                }
                return;
            }
            if (Intent.ACTION_SIM_STATE_CHANGED.equals(action) &&
                    mConfigState != CarrierExpressManager.SRV_CONFIG_STATE_WAIT) {
                String newState = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                int slotId = intent.getIntExtra(PhoneConstants.PHONE_KEY, 0);
                Log.d(TAG, "BroadcastReceiver(), SIM state change, slotId:" + slotId
                    + ", new state:" + newState);

                int phoneCount = mTelephonyManager.getPhoneCount();
                if (phoneCount == 2) {
                    if (slotId < 0 || slotId >= 2) {
                        Log.d(TAG, "Invalid slot ID: " + slotId);
                        return;
                    }
                    mSlotSimState[slotId] = newState;

                    // We may miss the SIM state change intent for another SIM
                    // So we also update the last SIM state of another SIM here
                    int otherSlot = (slotId == 0) ? 1 : 0;
                    int anotherSimState = mTelephonyManager.getSimState(otherSlot);
                    switch(anotherSimState) {
                        case TelephonyManager.SIM_STATE_READY:
                            mSlotSimState[otherSlot] = IccCardConstants.INTENT_VALUE_ICC_READY;
                            break;
                        case TelephonyManager.SIM_STATE_LOADED:
                            mSlotSimState[otherSlot] = IccCardConstants.INTENT_VALUE_ICC_LOADED;
                            break;
                        case TelephonyManager.SIM_STATE_ABSENT:
                            mSlotSimState[otherSlot] = IccCardConstants.INTENT_VALUE_ICC_ABSENT;
                            break;
                        case TelephonyManager.SIM_STATE_CARD_IO_ERROR:
                            mSlotSimState[otherSlot] = IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR;
                            break;
                    }

                    Log.d(TAG, "phoneCount=2, mSlotSimState["+slotId+"]:" + mSlotSimState[slotId]
                        + ", mSlotSimState["+otherSlot+"]:" + mSlotSimState[otherSlot]);
                    if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(newState) ||
                        IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(newState)) {
                        simReadyDetectDualSwitch(intent, slotId);
                    }
                } else if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(newState)||
                        IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(newState)) {
                    // reset this (mDualSimPresent) so that if dsds chnaged to ss after switching
                    mDualSimPresent = false;
                    Log.d(TAG, "set mDualSimPresent = false");
                    String mccMnc = readMCCMNCFromProperty(false,0);
                    if (mccMnc.length() < 5) {
                        Log.d(TAG, "Invalid mccMnc " + mccMnc);
                        return;
                    }
                    String optr = getOperatorPackForSim(mccMnc);
                    detectOperatorSwitching(getRefinedOperator(optr), mccMnc);
                }
            } else if (CarrierExpressManager.ACTION_USER_NOTIFICATION_INTENT.equals(action)) {
                 if (mDialog != null && mDialog.isShowing()) {
                    Log.d(TAG, "configuration dialog already being displayed");
                    return;
                }
                // show user dialog for action
                String optr = intent.getStringExtra("OPTR");
                String subId = intent.getStringExtra("SUBID");
                Log.d(TAG, "in br, optr: " + optr + "subid:" + subId);
                setPhoneRebootFlag(optr);
                // simSwitchMode = 1: auto switch mode
                if ("1".equals(simSwitchMode)) {
                    Log.d(TAG, "User popup skipped as auto switch mode");
                    startConfiguringOpPack(optr, subId, false);
                } else {
                    new CarrierExpressUserDialog(optr, subId, true).showDialog();
                }
            } else if (Intent.ACTION_USER_SWITCHED.equals(action)) {
                // PMS will enable all existed packages when user account changed
                // CXP need to run firstBootConfigure() again to enable/disable app
                // according the current OP configuration again
                mTaskHandler.sendMessage(mTaskHandler.obtainMessage(TaskHandler.START_FIRST_CONFIG));
                mSystemUserId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, -1);
                Log.d(TAG,"ACTION_USER_SWITCHED: current user ID: " + mSystemUserId);
            } else if (CarrierExpressManager.ACTION_CXP_NOTIFY_FEATURE.equals(action)){
                // All receivers received the intent ACTION_CXP_NOTIFY_FEATURE

                // Start to enable/disable OP customization apps according to new operator
                mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
                        TaskHandler.ENABLE_DISABLE_APP), 1000);
            } else if (CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED.equals(action)) {
                // All receivers received the intent ACTION_OPERATOR_CONFIG_CHANGED
                // Take the final step of OP configuration switch flow
                finishOPConfig();
            }
        }
    };

    private String readMCCMNCFromProperty(boolean isFromBootanim, int phoneId) {
        String mccMncPropName = "";
        String mccMnc = "";

        if (isFromBootanim) {
            mccMncPropName = PROP_PERSIST_BOOTANIM_MNC;
            mccMnc = SystemProperties.get(mccMncPropName, "");
        } else {
            // North America operators use the same CSIM mcc/mnc 31000
            // So we should use USIM mcc/mnc first to decide OP ID
            // CDMA only SIM: We only use CSIM mccmnc when we cannot get USIM mccmnc
            if (phoneId == 0) {
                mccMncPropName = PROP_GSM_SIM_OPERATOR_NUMERIC;
            } else {
                mccMncPropName = PROP_GSM_SIM_OPERATOR_NUMERIC + "." + phoneId;
            }
            mccMnc = SystemProperties.get(mccMncPropName, "");

            if(mccMnc == null || mccMnc.length() == 0) {
                // USIM mcc/mnc is empty
                if (phoneId == 0) {
                    mccMncPropName = PROP_CDMA_SIM_OPERATOR_NUMERIC;
                } else {
                    mccMncPropName = PROP_CDMA_SIM_OPERATOR_NUMERIC + "." + phoneId;
                }
                mccMnc = SystemProperties.get(mccMncPropName, "");
            }
        }

        if (mccMnc != null && mccMnc.length() > 4) {
            Log.d(TAG, "read mcc mnc property from " + mccMncPropName + ": " + mccMnc);
            if (TESTING_PURPOSE == true) {
                // We are using a dummyMccMnc to verify specific string mcc_mnc
                // by setting a value from property
                String dummyMccMnc
                    = SystemProperties.get("persist.vendor.mtk_usp_simulate_cxp_sim");
                Log.d(TAG, "Simulate mcc mnc:" + dummyMccMnc);
                return (dummyMccMnc != null && dummyMccMnc.length() > 4) ? dummyMccMnc : mccMnc;
            } else {
                return mccMnc;
            }
        }

        Log.d(TAG, "failed to read mcc mnc from property");
        return "";
    }

    /**
    * CarrierExpressUserDialog.
    */
    private class CarrierExpressUserDialog implements DialogInterface.OnClickListener {
        private String mOptr;
        private String mSubId;
        private boolean mNotifDialog;

        CarrierExpressUserDialog(String optr, String subId) {
            mOptr = optr;
            mSubId = subId;
            mNotifDialog = false;
        }
        CarrierExpressUserDialog(String optr, String subId, boolean isNotifcation) {
            mOptr = optr;
            mSubId = subId;
            mNotifDialog = isNotifcation;
        }

        void showDialog() {
            StringBuilder message;
            if (mNotifDialog == true) {
                if (mIsRebootRequired == true) {
                    message = new StringBuilder(mContext.getResources().getString(
                            R.string.usp_config_notif_reboot_dialog,
                            getOperatorNameFromPack(mOptr)));
                } else {
                    message = new StringBuilder(mContext.getResources().getString(
                            R.string.usp_config_notif_dialog,
                            getOperatorNameFromPack(mOptr)));
                }
            } else {
                String content;
                if (mIsRebootRequired == true) {
                    content = mContext.getResources().getString(
                            R.string.usp_config_reboot_confirm);
                } else {
                    content = mContext.getResources().getString(
                            R.string.usp_config_confirm);
                }
                String operatorName = getOperatorNameFromPack(mOptr);
                message = new StringBuilder(
                        "[" + operatorName + "] " + content);
            }
            mDialog = new AlertDialog.Builder(mContext)
                    .setMessage(message.toString())
                    .setPositiveButton(android.R.string.yes, this)
                    .setNegativeButton(android.R.string.no, this)
                    .create();
            mDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ERROR);
            mDialog.getWindow().getAttributes().privateFlags |=
                    WindowManager.LayoutParams.PRIVATE_FLAG_SHOW_FOR_ALL_USERS;
            mDialog.setCanceledOnTouchOutside(false);
            mDialog.show();
            Log.d(TAG, "showDialog: " + mDialog + " optr: " + mOptr);
        }

        @Override
        public void onClick(DialogInterface dialog, int whichButton) {
            if (DialogInterface.BUTTON_POSITIVE == whichButton) {
                Log.d(TAG, "Click for yes");
                if (isInCall() == true) return;

                startConfiguringOpPack(mOptr, mSubId, false);
                mNotificationManager.cancel(CARRIEREXPRESS_NOTIFICATION);
            } else {
                showNotificationMessage(mOptr, mSubId);
            }
            setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_POPUP_HANDLED, true, mOptr, mSubId);
            mDialog.dismiss();
            mDialog = null;
        }
    }

    private String getOperatorNameFromPack(String optr) {
        //Get redirected operator name first
        String cxpPack = SystemProperties.get("ro.vendor.mtk_carrierexpress_pack");
        if (sOperatorMapInfo.containsKey(cxpPack + "_" + optr)) {
            Log.d(TAG, "getOperatorNameFromPack for optr: " + cxpPack + "_" + optr);
            return sOperatorMapInfo.get(cxpPack + "_" + optr);
        }
        if (sOperatorMapInfo.containsKey(optr)) {
            Log.d(TAG, "getOperatorNameFromPack for optr: " + optr);
            return sOperatorMapInfo.get(optr);
        }
        return new String("Unknown");
    }

    private String getMdSbpId(String optr) {
        String val = "";
        if (optr.equals("OP00")) {
            val = "0";
        } else {
            val = getSbpIdForOperator(optr);
        }
        Log.d(TAG, "getMdSbpId value: " + val);
        return val;
    }

    private void setProperties(String optr, String subId) {
        File customGlobalDir;
        Bundle featureSettings = new Bundle();
        Intent intentSetVendor = new Intent(CarrierExpressManager.ACTION_CXP_SET_VENDOR_PROP);
        Intent intent = new Intent(CarrierExpressManager.ACTION_CXP_NOTIFY_FEATURE);

        if (new File(CUSTOM_PATH).exists()) {
            customGlobalDir = new File(CUSTOM_PATH);
        } else if (new File(SYSTEM_PATH).exists()) {
            customGlobalDir = new File(SYSTEM_PATH);
        } else {
            Log.e(TAG, "none of custom/usp or vendor/usp exists");
            return;
        }
        String propFileName = "usp-content" + "-" + optr + ".txt";
        File customPropFile = new File(customGlobalDir, propFileName);
        List<String> opPropertyList = readFromFile(customPropFile,
                "[Property-start]", "[Property-end]");
        for (int i = 0; i < opPropertyList.size(); i++) {
            String key = getKey(opPropertyList.get(i).trim());
            String value = getValue(opPropertyList.get(i).trim());
            if (DEBUG) {
                Log.d(TAG, "Checking property " + key + ", setting value:" + value);
            }
            String customValue = mCarrierExpressServiceExt.getPropertyValue(key, value);

            featureSettings.putString(key,customValue);

            if (PROP_PERSIST_OPERATOR_OPTR.equals(key)) {
                intentSetVendor.putExtra("OPTR", customValue);
            }
            if (PROP_PERSIST_OPERATOR_SPEC.equals(key)) {
                intentSetVendor.putExtra("SPEC", customValue);
            }
            if (PROP_PERSIST_OPERATOR_SEG.equals(key)) {
                intentSetVendor.putExtra("SEG", customValue);
            }
            for (String checkPropertyName : PROPERTY_NEED_CXP_SET_SYSTEM_ONLY) {
                if (checkPropertyName.equals(key)) {
                    set(mContext, key, customValue);
                }
            }
        }

        // Set for modem SBP ID
        intentSetVendor.putExtra("SBP", getMdSbpId(optr));
        // Set for operator sub ID
        intentSetVendor.putExtra("SUBID", subId);
        intentSetVendor.setPackage("com.android.phone");
        mContext.sendBroadcast(intentSetVendor);
        Log.d(TAG, "send broadcast intent for setting vendor property");

        setDualSimOptrProperties(optr);

        intent.putExtras(featureSettings);
        intent.addFlags(Intent.FLAG_RECEIVER_INCLUDE_BACKGROUND);
        // Send ordered broadcast intent
        // Register own BroadcastReceiver as the final receiver of this intent
        mContext.sendOrderedBroadcast(intent, null, mIntentReceiver, null,
            Activity.RESULT_OK, null, null);
        Log.d(TAG, "Send ordered broadcast intent to notify feature setting");
    }

    private String getRegionalOpPack() {
        Log.d(TAG, "getRegionalOpPack " + CUSTOM_PATH +  " " + SYSTEM_PATH);
        File customGlobalDir;
        if (new File(CUSTOM_PATH).exists()) {
            customGlobalDir = new File(CUSTOM_PATH);
        } else if (new File(SYSTEM_PATH).exists()) {
            customGlobalDir = new File(SYSTEM_PATH);
        } else {
            Log.e(TAG, "none of custom/usp or vendor/usp exists");
            return "";
        }
        String fileName = CARRIEREXPRESS_INFO_FILE;
        File customFile = new File(customGlobalDir, fileName);
        List<String> data = readFromFile(customFile);
        for (int i = 0; i < data.size(); i++) {
            String key = getKey(data.get(i).trim());
            Log.d(TAG, "MTK_REGIONAL_OP_PACK = " + key);
            if (key.equals("MTK_REGIONAL_OP_PACK")) {
                String value = getValue(data.get(i).trim());
                Log.d(TAG, "MTK_REGIONAL_OP_PACK = " + value);
                return value;
            }
        }
        return "";
    }
    /**
     * Set the value for the given key.
     * @throws IllegalArgumentException if the key exceeds 32 characters
     * @throws IllegalArgumentException if the value exceeds 92 characters
     */
    private void set(Context context, String key, String val) throws IllegalArgumentException {
        try {
            if (DEBUG) {
                Log.d(TAG, "setting property " + key + "  TO  " + val);
            }
            SystemProperties.set(key, val);
        } catch (IllegalArgumentException iAE) {
            throw iAE;
        } catch (Exception e) {
            //TODO
        }
    }

    private void enabledDisableApps(String optr) {
        String isInstSupport = SystemProperties.get("ro.vendor.mtk_carrierexpress_inst_sup");
        if (isInstSupport != null && isInstSupport.equals("1")) {
            Log.d(TAG, "Install/uninstall apk is enabled");
            return;
        }
        File customGlobalDir;
        if (new File(CUSTOM_PATH).exists()) {
            customGlobalDir = new File(CUSTOM_PATH);
        } else if (new File(SYSTEM_PATH).exists()) {
            customGlobalDir = new File(SYSTEM_PATH);
        } else {
            Log.e(TAG, "none of custom/CarrierExpress or vendor/CarrierExpress exists");
            return;
        }
        String[] customGlobalFiles = customGlobalDir.list();
        String opFileName = "usp-content" + "-" + optr + ".txt";
        String allFileName = "usp-packages" + "-" + "all" + ".txt";
        File customAllFile = new File(customGlobalDir, allFileName);
        File customOpFile = new File(customGlobalDir, opFileName);

        List<String> allPackageList = readFromFile(customAllFile);
        List<String> opPackageList = readFromFile(customOpFile, "[Package-start]", "[Package-end]");
        Log.d(TAG, "enabledDisableApps, to check OP content file: " + opFileName);
        //Register broadcast receiver for tracking enable-disable Status
        IntentFilter packageFilter = new IntentFilter();
        packageFilter.addAction("android.intent.action.PACKAGE_CHANGED");
        packageFilter.addDataScheme("package");
        mContext.registerReceiver(mEnableDisableRespReceiver, packageFilter);
        //Disable Apps Present in All but not in OP File, Don't Disable if Already Disabled
        Log.d(TAG,"Carrier Express user ID: " + mContext.getUserId() + ", current user ID: " + mSystemUserId);
        for (int i = 0; i < allPackageList.size(); i++) {
            Log.d(TAG, allPackageList.get(i) + " is not in OP packages list: " +
                    !opPackageList.contains(allPackageList.get(i)) +
                    ", EnabledState: " + getPackageEnabledState(allPackageList.get(i), false));
            if ((!opPackageList.contains(allPackageList.get(i))) &&
                    getPackageEnabledState(allPackageList.get(i), false)) {
                mPendingEnableDisableReq.add(allPackageList.get(i));
                disableApps(allPackageList.get(i));
            }
        }
        //Enable Apps Present in OP File, Don't Enable if Already Enable
        for (int i = 0; i < opPackageList.size(); i++) {
            Log.d(TAG, opPackageList.get(i) + " EnabledState: "
                    + getPackageEnabledState(opPackageList.get(i), true));
            if (!getPackageEnabledState(opPackageList.get(i), true)) {
                mPendingEnableDisableReq.add(opPackageList.get(i));
                enableApps(opPackageList.get(i));
            }
        }
        // it does enable secondary sim when dual sim enabled
        enablePluginApkDualSim();

    }

    void enablePluginApkDualSim() {
        // Enable plugin apk for secondary SIM
        int phoneCount = mTelephonyManager.getPhoneCount();
        if (phoneCount == 2 && mDualSimPresent == true) {
            int slotS = (getMainCapabilitySlotId() == 0) ? 1 : 0;
            String opS = mOptrSlot[slotS];
            Log.d(TAG, "enablePluginApkDualSim optrS: " + opS);
            if (opS != null && opS.length() > 2) {
                // APP Plugin APK
                mPendingEnableDisableReq.add("com.mediatek." + opS.toLowerCase() + ".plugin");
                enableApps("com.mediatek." + opS.toLowerCase() + ".plugin");
                // FWK Plugin APK
                mPendingEnableDisableReq.add("com.mediatek.fwk." + opS + ".plugin");
                enableApps("com.mediatek.fwk." + opS + ".plugin");
            }
        }
    }

    //Takes a defaultState boolean to handle packages which are not installed
    private boolean getPackageEnabledState(String packageName, boolean defaultState) {
        ApplicationInfo ai = null;
        try {
            if (mSystemUserId >= 0) {
                ai = mPm.getApplicationInfoAsUser(packageName, 0, mSystemUserId);
            } else {
                ai = mPm.getApplicationInfo(packageName, 0);
            }
        } catch (NameNotFoundException e) {
            e.printStackTrace();
            Log.e(TAG, "getPackageEnabledState, packageNotFound: " + packageName);
            return defaultState;
        }

        return ai.enabled;
    }


    private String getKey(String toBeSplit) {
        int assignmentIndex = toBeSplit.indexOf("=");
        String key = null;
        try {
            key = toBeSplit.substring(0, assignmentIndex);
        } catch (IndexOutOfBoundsException e) {
            Log.e(TAG, "illegal property string: " + e.toString());
        }
        return key;
    }

    private String getValue(String toBeSplit) {
        int assignmentIndex = toBeSplit.indexOf("=");
        String value = null;
        try {
            value = toBeSplit.substring(assignmentIndex + 1, toBeSplit.length());
        } catch (IndexOutOfBoundsException e) {
            Log.e(TAG, "illegal property string: " + e.toString());
        }
        return value;
     }

    private List<String> readFromFile(File customGlobalFile) {
        int length = (int) customGlobalFile.length();
        byte[] bytes = new byte[length];
        List<String> fileContents = new ArrayList<String>();
        try {
            FileInputStream inputStream = new FileInputStream(customGlobalFile);
            if (inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                while ((receiveString = bufferedReader.readLine()) != null) {
                    fileContents.add(receiveString);
                }
                inputStream.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "File not found: " + e.toString());
        } catch (IOException e) {
            Log.e(TAG, "Can not read file: " + e.toString());
        }
        return fileContents;
    }

    private List<String> readFromFile(File customGlobalFile, String startTag, String endTag) {
        int length = (int) customGlobalFile.length();
        byte[] bytes = new byte[length];
        List<String> fileContents = new ArrayList<String>();
        try {
            FileInputStream inputStream = new FileInputStream(customGlobalFile);
            if (inputStream != null) {
                InputStreamReader inputStreamReader = new InputStreamReader(inputStream);
                BufferedReader bufferedReader = new BufferedReader(inputStreamReader);
                String receiveString = "";
                boolean isSect = false;
                while ((receiveString = bufferedReader.readLine()) != null) {
                    if (startTag.equals(receiveString)) {
                        isSect = true;
                        continue;
                    } else if (endTag.equals(receiveString)) {
                        isSect = false;
                        break;
                    }
                    if (isSect) {
                        fileContents.add(receiveString);
                    }
                }
                inputStream.close();
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "File not found: " + e.toString());
        } catch (IOException e) {
            Log.e(TAG, "Can not read file: " + e.toString());
        }
        return fileContents;
    }

    private void enableApps(String appPackage) {
        Log.d(TAG, "enablingApp :" + appPackage);
        try {
            mPm.setApplicationEnabledSetting(
                    appPackage, mPm.COMPONENT_ENABLED_STATE_ENABLED, mPm.DONT_KILL_APP);
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "enabling illegal package: " + e.toString());
        }
    }

    private void disableApps(String appPackage) {
        Log.d(TAG, "disablingApp :" + appPackage);
        try {
            if (mSystemUserId >= 0) {
                mIPackageManager.setApplicationEnabledSetting(
                        appPackage, mPm.COMPONENT_ENABLED_STATE_DISABLED, mPm.DONT_KILL_APP,
                        mSystemUserId, mContext.getBasePackageName());
            } else {
                mPm.setApplicationEnabledSetting(
                        appPackage, mPm.COMPONENT_ENABLED_STATE_DISABLED, mPm.DONT_KILL_APP);
            }
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "disabling illegal package: " + e.toString());
        } catch (RemoteException e) {
        }
    }


    /** Handler to handle UI related operations.
     */
    private class MyHandler extends Handler {

        static final int REBOOT_DIALOG = 0;

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case REBOOT_DIALOG:
                    Intent startMain = new Intent(Intent.ACTION_MAIN);
                    startMain.addCategory(Intent.CATEGORY_HOME);
                    startMain.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(startMain);
                    // show waiting screen
                    showWaitingScreen((Boolean) msg.obj);
                    break;
                default:
                    Log.d(TAG, "Wrong message reason");
                    break;
            }
        }
    }

    /** Handler to handle as worker thread related operations.
     */
    private class TaskHandler extends Handler {

        static final int START_FIRST_CONFIG = 0;
        static final int START_CONFIG = 1;
        static final int ENABLE_DISABLE_APP = 2;
        static final int CHECK_APP_CONFIG_STATUS = 3;
        static final int REBOOT_SYSTEM = 4;
        static final int EARLY_READ_FAILED = 5;

        public TaskHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "TaskHandler message:" + msg.what);
            switch (msg.what) {
                case START_FIRST_CONFIG:
                    firstBootConfigure();
                    setConfigCtrlFlag(PROP_CFG_CTRL_FLAG_NOT_FIRST_BOOT, true, null, null);
                    break;
                case START_CONFIG:
                    // Here we clear the pending list of enable/disable apps
                    if (!mPendingEnableDisableReq.isEmpty()) {
                        mPendingEnableDisableReq.clear();
                        try {
                            mContext.unregisterReceiver(mEnableDisableRespReceiver);
                        } catch (IllegalArgumentException e) {
                            Log.d(TAG, "Receiver not registered: " + e.toString());
                        }
                    }
                    Bundle data = msg.getData();
                    String opPack = (String) data.getCharSequence("OPTR");
                    String subId = (String) data.getCharSequence("SUBID");
                    boolean isSimSwitched =
                            "done".equals((String)data.getCharSequence("SIM_SWITCH")) ? true : false;
                    runningConfigurationTask(opPack, subId, isSimSwitched);
                    break;
                case ENABLE_DISABLE_APP:
                    String current_optr = SystemProperties.get(PROP_PERSIST_OPERATOR_OPTR);
                    Log.d(TAG,"Check current operator optr property:" + current_optr);
                    // Enable-disable op packages apps, plugins, resource overlays
                    enabledDisableApps(getRefinedOperator(current_optr));

                    // After enable/disable OP packages apps, check progress
                    checkPendingEnableDisableApp();
                    break;
                case CHECK_APP_CONFIG_STATUS:
                    checkPendingEnableDisableApp();
                    break;
                case REBOOT_SYSTEM:
                    rebootAndroidSystem();
                    break;
                case EARLY_READ_FAILED:
                    String mccMnc = readMCCMNCFromProperty(true,0);
                    if (mccMnc.length() < 5) {
                        Log.d(TAG, "Invalid mccMnc " + mccMnc);
                        mTaskHandler.sendMessageDelayed(mTaskHandler.obtainMessage(
                                TaskHandler.EARLY_READ_FAILED), 1000);
                        return;
                    }
                    String optr = getOperatorPackForSim(mccMnc);
                    detectOperatorSwitching(getRefinedOperator(optr), mccMnc);
                    break;
                default:
                    Log.d(TAG, "Wrong message reason");
                    break;
            }
        }
    }
    void simReadyDetectDualSwitch(Intent intent, int slotId) {
        Log.d(TAG, "simReadyDetectDualSwitch slot:" + slotId);
        int otherSlot = (slotId == 0) ? 1 : 0;
        String[] mccMncOnSlot = new String[2];

        int subId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY, -1);

        // Allow switch when both SIM state is confirmed, As current SIM is already detected
        // as loaded/ready, so need to check other SIM
        if (!isSimStateConfirmed(otherSlot)) {
            Log.d(TAG, "Need to wait another slot " + otherSlot + ", SIM state is in progress");
            return;
        }
        // set optr in slot for MPlugin
        mccMncOnSlot[slotId] = readMCCMNCFromProperty(false,slotId);
        mccMncOnSlot[otherSlot] = readMCCMNCFromProperty(false,otherSlot);

        mOptrSlot[slotId] = getOpPackFromSimInfo(mccMncOnSlot[slotId]);
        mOptrSlot[otherSlot] = getOpPackFromSimInfo(mccMncOnSlot[otherSlot]);

        Log.d(TAG, "simReadyDetectDualSwitch mccMncOnSlot[slotId]:" + mccMncOnSlot[slotId]
            + ", mccMncOnSlot[otherSlot]:" + mccMncOnSlot[otherSlot]
            + ", mOptrSlot[slotId]:" + mOptrSlot[slotId]
            + ", mOptrSlot[otherSlot]:" + mOptrSlot[otherSlot]);

        if (isSimStateReadyForDetect(slotId) &&
                isSimStateReadyForDetect(otherSlot)) {
            // Both SIMs are present and loaded
            mDualSimPresent = true;
            Log.d(TAG, "Both SIMs are present and loaded");
            if (getMainCapabilitySlotId() == slotId) {
                Log.d(TAG, "Main optr: " + mOptrSlot[slotId] +
                        " Secondary optr: " + mOptrSlot[otherSlot]);
                detectOperatorSwitching(getRefinedOperator(mOptrSlot[slotId]),
                        mccMncOnSlot[slotId]);
            } else {
                Log.d(TAG, "Main optr: " + mOptrSlot[otherSlot] +
                        " Secondary optr: " + mOptrSlot[slotId]);
                detectOperatorSwitching(getRefinedOperator(mOptrSlot[otherSlot]),
                        mccMncOnSlot[otherSlot]);
            }
        } else {
            // Only one SIM or None present
            mDualSimPresent = false;
            if (isSimStateReadyForDetect(slotId)) {
                Log.d(TAG, "Only one SIM is present OPTR: " + mOptrSlot[slotId]);
                detectOperatorSwitching(getRefinedOperator(mOptrSlot[slotId]),
                        mccMncOnSlot[slotId]);
            } else if (isSimStateReadyForDetect(otherSlot)) {
                Log.d(TAG, "Only one SIM is present OPTR: " + mOptrSlot[otherSlot]);
                detectOperatorSwitching(getRefinedOperator(mOptrSlot[otherSlot]),
                        mccMncOnSlot[otherSlot]);
            } else {
                Log.d(TAG, "Both SIMs are absent states - slot1: " + mSlotSimState[0] +
                        ", slot2: " + mSlotSimState[1]);
            }
        }
    }

    private boolean isSimStateConfirmed(int slot) {
        String simState = mSlotSimState[slot];
        Log.d(TAG, "isSimStateConfirmed slot: " + slot + " state: " + simState);
        if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simState) ||
                IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(simState) ||
                IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(simState) ||
                IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR.equals(simState)) {
            return true;
        }
        return false;
    }

    private boolean isSimStateReadyForDetect(int slot) {
        String simState = mSlotSimState[slot];
        Log.d(TAG, "isSimStateReadyForDetect slot: " + slot + " state: " + simState);
        if (IccCardConstants.INTENT_VALUE_ICC_READY.equals(simState) ||
                IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(simState)) {
            return true;
        }
        return false;
    }

    private int getMainCapabilitySlotId() {

      int id = SystemProperties.getInt("persist.vendor.mtk_usp_ds_main_slot", 0);
      // if main id is invalid then consider 1st slot as main slot.
      return (id == 0 || id == 1) ? id : 0;
    }

    private void setDualSimSupportVariables(String mainOptr) {
        int phoneCount = mTelephonyManager.getPhoneCount();
        Log.d(TAG, "setDualSimSupportVariables phonecount: " + phoneCount);
        if (phoneCount == 2) {
            int mainId = getMainCapabilitySlotId();
            int otherSlot =  (mainId == 0) ? 1 : 0;
            String mccMnc = readMCCMNCFromProperty(false,otherSlot);
            String optr = getOpPackFromSimInfo(mccMnc);
            mOptrSlot[mainId] = mainOptr;
            mOptrSlot[otherSlot] = optr;
            Log.d(TAG, "main optr:  " + mainOptr + " secondary optr: " + optr);
        }
    }
    private String getSbpIdForOperator(String mainOptr) {
        String DefaultVal = new String(mainOptr.substring(2, mainOptr.length()));
        int phoneCount = mTelephonyManager.getPhoneCount();
        Log.d(TAG, "getSbpIdForOperator phonecount: " + phoneCount);
        if (phoneCount == 2) {
            int mainId = getMainCapabilitySlotId();
            int otherSlot =  (mainId == 0) ? 1 : 0;
            String mccMnc = readMCCMNCFromProperty(false,mainId);
            if (mccMnc != null && mccMnc.length() >= 5) {
                if (mOperatorResData.containsKey(mainOptr + "_" + mccMnc)) {
                    String data = mOperatorResData.get(mainOptr + "_" + mccMnc);
                    String[] opSplit = data.split("\\s*,\\s*");
                    if (opSplit.length == 4) {
                        return opSplit[3];
                    }
                }
            } else {
                // when Single sim is present and configuring phone for 2nd slot
                mccMnc = readMCCMNCFromProperty(false,otherSlot);
                if (mccMnc != null && mccMnc.length() >= 5) {
                    if (mOperatorResData.containsKey(mainOptr + "_" + mccMnc)) {
                        String data = mOperatorResData.get(mainOptr + "_" + mccMnc);
                        String[] opSplit = data.split("\\s*,\\s*");
                        if (opSplit.length == 4) {
                            return opSplit[3];
                        }
                    }
                }
            }
        } else {
            String mccMnc = readMCCMNCFromProperty(false,0);
            if (mccMnc != null && mccMnc.length() >= 5) {
                if (mOperatorResData.containsKey(mainOptr + "_" + mccMnc)) {
                    String data = mOperatorResData.get(mainOptr + "_" + mccMnc);
                    String[] opSplit = data.split("\\s*,\\s*");
                    if (opSplit.length == 4) {
                        return opSplit[3];
                    }
                }
            }
        }
        return DefaultVal;
    }

    boolean triggerSimSwitchChange(String opPack, String subId) {
        boolean ret = false;
        int len = TelephonyManager.getDefault().getPhoneCount();
        if (len > 1) {
            try {
                RadioAccessFamily[] rafs = new RadioAccessFamily[len];
                IMtkTelephonyEx iTelEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
                if (null == iTelEx) {
                    Log.d(TAG, "Can not get phone service");
                    return ret;
                }
                for (int phoneId = 0; phoneId < len; phoneId++) {
                    int raf;
                    if (phoneId == 0) {
                        raf = RadioAccessFamily.RAF_GPRS | RadioAccessFamily.RAF_UMTS |
                                RadioAccessFamily.RAF_LTE | RadioAccessFamily.RAF_GSM;
                    } else {
                        raf = RadioAccessFamily.RAF_GSM;
                    }
                    rafs[phoneId] = new RadioAccessFamily(phoneId, raf);
                }
                try {
                    //we always register SIM change as user may swap it
                    IntentFilter filter = new IntentFilter();
                    filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE);
                    filter.addAction(TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED);
                    BroadcastReceiver receiver = new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        String action = intent.getAction();
                        if (TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_DONE.equals(action) ||
                                TelephonyIntents.ACTION_SET_RADIO_CAPABILITY_FAILED.equals(
                                action)) {
                            Log.d(TAG, "on receive SimSwitch change: " + action);
                            mContext.unregisterReceiver(this);

                            // continue configuration
                            Message startConnfig = mTaskHandler.obtainMessage(
                                    TaskHandler.START_CONFIG);
                            Bundle data = new Bundle();
                            data.putCharSequence("OPTR", opPack);
                            data.putCharSequence("SUBID", subId);
                            data.putCharSequence("SIM_SWITCH", "done");
                            startConnfig.setData(data);
                            mTaskHandler.sendMessage(startConnfig);
                        }
                      }
                    };
                    mContext.registerReceiver(receiver, filter);
                    if (false == iTelEx.setRadioCapability(rafs)) {
                        Log.d(TAG, "Set phone rat fail!!! MaxPhoneRat=");
                        mContext.unregisterReceiver(receiver);
                    } else {
                        Log.d(TAG, "setRadioCapability triggered now wait");
                        ret = true;
                    }
                } catch (RemoteException e) {
                  Log.d(TAG, "setRadioCapability: RemoteException " + e);
                }
            } catch (RuntimeException e) {
                Log.d(TAG, "setRadioCapability: Runtime Exception" + e);
            }
        }
        Log.d(TAG, "triggerSimSwitchChange: " + ret);
        return ret;
    }
}
