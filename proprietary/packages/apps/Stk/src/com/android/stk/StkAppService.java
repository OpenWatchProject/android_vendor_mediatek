/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.stk;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningAppProcessInfo;
import android.app.ActivityManager.RunningTaskInfo;
import android.app.AlertDialog;
import android.app.KeyguardManager;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.app.Activity;
import android.app.ActivityManagerNative;
import android.content.ActivityNotFoundException;
import android.app.IProcessObserver;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.PersistableBundle;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.Vibrator;
import android.provider.Settings;
import android.support.v4.content.LocalBroadcastManager;
import android.telephony.CarrierConfigManager;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.IWindowManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManagerPolicyConstants;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.content.IntentFilter;

import com.android.internal.telephony.cat.AppInterface;
import com.android.internal.telephony.cat.Input;
import com.android.internal.telephony.cat.LaunchBrowserMode;
import com.android.internal.telephony.cat.Menu;
import com.android.internal.telephony.cat.Item;
import com.android.internal.telephony.cat.ResultCode;
import com.android.internal.telephony.cat.CatCmdMessage;
import com.android.internal.telephony.cat.CatCmdMessage.BrowserSettings;
import com.android.internal.telephony.cat.CatCmdMessage.SetupEventListSettings;
import com.android.internal.telephony.cat.CatLog;
import com.android.internal.telephony.cat.CatResponseMessage;
import com.android.internal.telephony.cat.TextMessage;
import com.android.internal.telephony.cat.ToneSettings;
import com.android.internal.telephony.ITelephony;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.GsmAlphabet;
import com.android.internal.telephony.cat.CatService;

/// M:  @{
import android.database.ContentObserver;
import com.android.internal.telephony.Call;
import com.android.internal.telephony.CallStateException;
import com.android.internal.telephony.PhoneFactory;

import com.mediatek.internal.telephony.cat.MtkCatResponseMessage;
import com.mediatek.internal.telephony.cat.MtkAppInterface;
import com.mediatek.internal.telephony.cat.MtkCatCmdMessage;
import com.mediatek.internal.telephony.cat.MtkCatService;
import com.mediatek.internal.telephony.cat.MtkMenu;
import com.mediatek.internal.telephony.cat.MtkCatLog;

import com.mediatek.internal.telephony.RadioManager;

import com.mediatek.provider.MtkSettingsExt;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsRegistrationImplBase;
import com.android.ims.ImsManager;
import com.android.ims.ImsException;
import com.android.internal.telephony.SubscriptionController;
import com.mediatek.telephony.MtkTelephonyManagerEx;
import com.mediatek.ims.MtkImsConnectionStateListener;
import com.mediatek.ims.internal.MtkImsManager;

/// @}


import java.util.Iterator;
import java.util.LinkedList;
import java.lang.System;
import java.util.List;
import java.util.Locale;
import java.util.regex.Pattern;


import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.IDLE_SCREEN_AVAILABLE_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.LANGUAGE_SELECTION_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.USER_ACTIVITY_EVENT;

/// M: to handle browse status and termination event download. @{
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.BROWSER_TERMINATION_EVENT;
import static com.android.internal.telephony.cat.CatCmdMessage.
                   SetupEventListConstants.BROWSING_STATUS_EVENT;
/// @}


/**
 * SIM toolkit application level service. Interacts with Telephopny messages,
 * application's launch and user input from STK UI elements.
 *
 */
public class StkAppService extends Service implements Runnable {

    // members
    protected class StkContext {
        protected MtkCatCmdMessage mMainCmd = null;
        protected MtkCatCmdMessage mCurrentCmd = null;
        protected MtkCatCmdMessage mCurrentMenuCmd = null;
        protected Menu mCurrentMenu = null;
        protected String lastSelectedItem = null;
        protected boolean mMenuIsVisible = false;
        protected boolean mIsInputPending = false;
        protected boolean mIsMenuPending = false;
        protected boolean mIsDialogPending = false;
        protected boolean mNotificationOnKeyguard = false;
        protected boolean mNoResponseFromUser = false;
        protected boolean launchBrowser = false;
        protected BrowserSettings mBrowserSettings = null;
        protected LinkedList<DelayedCmd> mCmdsQ = null;
        protected boolean mCmdInProgress = false;
        protected int mStkServiceState = STATE_UNKNOWN;
        protected int mSetupMenuState = STATE_NOT_EXIST;
        protected int mMenuState = StkMenuActivity.STATE_INIT;
        protected int mOpCode = -1;
        private Activity mActivityInstance = null;
        private Activity mDialogInstance = null;
        private Activity mImmediateDialogInstance = null;
        private int mSlotId = 0;
        private SetupEventListSettings mSetupEventListSettings = null;
        private boolean mClearSelectItem = false;
        private boolean mDisplayTextDlgIsVisibile = false;
        private CatCmdMessage mCurrentSetupEventCmd = null;
        private CatCmdMessage mIdleModeTextCmd = null;
        private boolean mIdleModeTextVisible = false;
        // Determins whether the current session was initiated by user operation.
        protected boolean mIsSessionFromUser = false;

        /// M:  @{

        //stk available or not
        protected int mAvailable = STK_AVAIL_INIT;

        //add for set up call
        protected boolean mSetupCallInProcess = false; // true means in process.
        protected int mDelayCheckedCount = 0;

        //pending main menu activity, to finish when new setupmenu come
        private Activity mMainActivityInstance = null;
        /// @}

        final synchronized void setPendingActivityInstance(Activity act) {
            MtkCatLog.d(this, "setPendingActivityInstance act : " + mSlotId + ", " + act);
            callSetActivityInstMsg(OP_SET_ACT_INST, mSlotId, act);
        }
        final synchronized Activity getPendingActivityInstance() {
            MtkCatLog.d(this, "getPendingActivityInstance act : " + mSlotId + ", " +
                    mActivityInstance);
            return mActivityInstance;
        }
        final synchronized void setPendingDialogInstance(Activity act) {
            MtkCatLog.d(this, "setPendingDialogInstance act : " + mSlotId + ", " + act);
            callSetActivityInstMsg(OP_SET_DAL_INST, mSlotId, act);
        }
        final synchronized Activity getPendingDialogInstance() {
            MtkCatLog.d(this, "getPendingDialogInstance act : " + mSlotId + ", " +
                    mDialogInstance);
            return mDialogInstance;
        }
        final synchronized void setImmediateDialogInstance(Activity act) {
            MtkCatLog.d(this, "setImmediateDialogInstance act : " + mSlotId + ", " + act);
            callSetActivityInstMsg(OP_SET_IMMED_DAL_INST, mSlotId, act);
        }
        final synchronized Activity getImmediateDialogInstance() {
            MtkCatLog.d(this, "getImmediateDialogInstance act : " + mSlotId + ", " +
                    mImmediateDialogInstance);
            return mImmediateDialogInstance;
        }

        /// M:  set and get pending Mainactivity@{
        final synchronized void setMainActivityInstance(Activity act) {
            MtkCatLog.d(this, "setMainActivityInstance act : " + mSlotId + ", " + act);
            callSetActivityInstMsg(OP_SET_MAINACT_INST, mSlotId, act);
        }
        final synchronized Activity getMainActivityInstance() {
            MtkCatLog.d(this, "getMainActivityInstance act : " + mSlotId + ", " +
                    mMainActivityInstance);
            return mMainActivityInstance;
        }
        /// @}
    }

    private Thread serviceThread = null;
    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private Context mContext = null;
    private NotificationManager mNotificationManager = null;
    static StkAppService sInstance = null;
    private MtkAppInterface[] mStkService = null;
    private StkContext[] mStkContext = null;
    private int mSimCount = 0;
    private IProcessObserver.Stub mProcessObserver = null;
    private BroadcastReceiver mLocaleChangeReceiver = null;
    private TonePlayer mTonePlayer = null;
    private Vibrator mVibrator = null;
    private BroadcastReceiver mUserActivityReceiver = null;

    /// M:  @{
    private Phone[] mPhone = null;
    static boolean mIsLauncherAcceptInput = false;
    private Toast mToast = null;
    private ImsManager[] mImsManager = null;

    private static final String[] MCCMNC_TABLE_TYPE_CU = {
        "46001", "46006", "46009", "45407", "46005"};

    public static final int STK_AVAIL_INIT = -1;
    public static final int STK_AVAIL_NOT_AVAILABLE = 0;
    public static final int STK_AVAIL_AVAILABLE = 1;

    //for delay to check idle to show display text
    private static final int DELAY_TO_CHECK_IDLE_TIMEOUT = 3 * 1000;
    private static final int DELAY_TO_CHECK_NUM = 2;

    //for browser termination
    private static final String ACTION_BROWSER_TERMINATION =
            "mediatek.intent.action.stk.BROWSER_TERMINATION";

    //for browser status event download
    private String mLaunchBrowserUrl = null;
    int mLaunchBrowserUrlType = UNKNOWN_URL; // -1: unknown, 1:default url, 2:dedicated url
    private static final int UNKNOWN_URL = -1;
    private static final int DEFAULT_URL = 1;
    private static final int DEDICATED_URL = 2;

    private static final String ACTION_BROWSER_SEND_ERROR = "com.android.browser.action.SEND_ERROR";
    private static final String EXTRA_ERROR_CODE = "com.android.browser.error_code_key";
    private static final String EXTRA_URL = "com.android.browser.url_key";
    private static final String EXTRA_HOMEPAGE = "com.android.browser.homepage_key";

    protected static final String ACTION_SIM_ABSENT = "mediatek.intent.action.SIM_ABSENT";
    protected static final String ACTION_ALL_SIM_ABSENT = "mediatek.intent.action.ALL_SIM_ABSENT";
    /// @}

    // Used for setting FLAG_ACTIVITY_NO_USER_ACTION when
    // creating an intent.
    private enum InitiatedByUserAction {
        yes,            // The action was started via a user initiated action
        unknown,        // Not known for sure if user initated the action
    }

    // constants
    static final String OPCODE = "op";
    static final String CMD_MSG = "cmd message";
    static final String RES_ID = "response id";
    static final String MENU_SELECTION = "menu selection";
    static final String INPUT = "input";
    static final String HELP = "help";
    static final String CONFIRMATION = "confirm";
    static final String CHOICE = "choice";
    static final String SLOT_ID = "SLOT_ID";
    static final String STK_CMD = "STK CMD";
    static final String STK_DIALOG_URI = "stk://com.android.stk/dialog/";
    static final String STK_MENU_URI = "stk://com.android.stk/menu/";
    static final String STK_INPUT_URI = "stk://com.android.stk/input/";
    static final String STK_TONE_URI = "stk://com.android.stk/tone/";
    static final String FINISH_TONE_ACTIVITY_ACTION =
                                "android.intent.action.stk.finish_activity";

    // These below constants are used for SETUP_EVENT_LIST
    static final String SETUP_EVENT_TYPE = "event";
    static final String SETUP_EVENT_CAUSE = "cause";

    // operations ids for different service functionality.
    static final int OP_CMD = 1;
    static final int OP_RESPONSE = 2;
    static final int OP_LAUNCH_APP = 3;
    static final int OP_END_SESSION = 4;
    static final int OP_BOOT_COMPLETED = 5;
    private static final int OP_DELAYED_MSG = 6;
    static final int OP_CARD_STATUS_CHANGED = 7;
    static final int OP_SET_ACT_INST = 8;
    static final int OP_SET_DAL_INST = 9;
    static final int OP_LOCALE_CHANGED = 10;
    static final int OP_ALPHA_NOTIFY = 11;
    static final int OP_IDLE_SCREEN = 12;
    static final int OP_SET_IMMED_DAL_INST = 13;

    //Invalid SetupEvent
    static final int INVALID_SETUP_EVENT = 0xFF;

    // Message id to signal stop tone due to play tone timeout.
    private static final int OP_STOP_TONE = 16;

    // Message id to signal stop tone on user keyback.
    static final int OP_STOP_TONE_USER = 17;

    // Message id to remove stop tone message from queue.
    private static final int STOP_TONE_WHAT = 100;

    // Message id to send user activity event to card.
    private static final int OP_USER_ACTIVITY = 20;

    private static final int OP_DELAY_TO_CHECK_IDLE_SIM1 = 31;
    private static final int OP_DELAY_TO_CHECK_IDLE_SIM2 = 32;
    private static final int OP_DELAY_TO_CHECK_IDLE_SIM3 = 33;
    private static final int OP_DELAY_TO_CHECK_IDLE_SIM4 = 34;

    /// M:  @{

    //Message id to remove cached setup menu
    static final int OP_REMOVE_CACHED_SETUP_MENU = 21;
    //Message id to load cached setup menu
    static final int OP_LAUNCH_CACHED_SETUP_MENU = 22;

    //Message id to save mainmenu activity
    static final int OP_SET_MAINACT_INST = 23;

    //Message id to delay check idle screen
    private static final int OP_DELAY_TO_CHECK_IDLE = 14;

    // Message id to handle event download
    private static final int OP_BROWSER_TERMINATION = 25;
    private static final int OP_BROWSER_STATUS = 26;
    /// @}

    // mtk:For Acitvity restart after phone process killed
    public static final int APP_STATUS_FOCE_KILLED = -1;
    public static final int APP_STATUS_LAUNCHED_NORMAL = 0;
    public static int mAppStatus = APP_STATUS_FOCE_KILLED;

    // Response ids
    static final int RES_ID_MENU_SELECTION = 11;
    static final int RES_ID_INPUT = 12;
    static final int RES_ID_CONFIRM = 13;
    static final int RES_ID_DONE = 14;
    static final int RES_ID_CHOICE = 15;

    static final int RES_ID_TIMEOUT = 20;
    static final int RES_ID_BACKWARD = 21;
    static final int RES_ID_END_SESSION = 22;
    static final int RES_ID_EXIT = 23;
    static final int RES_ID_ERROR = 24;

    static final int YES = 1;
    static final int NO = 0;

    static final int STATE_UNKNOWN = -1;
    static final int STATE_NOT_EXIST = 0;
    static final int STATE_EXIST = 1;

    static int STK_GEMINI_SIM_NUM = 4;


    private static final String PACKAGE_NAME = "com.android.stk";
    private static final String STK_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivity";
    private static final String STK_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivity";
    private static final String STK_DIALOG_ACTIVITY_NAME = PACKAGE_NAME + ".StkDialogActivity";
     /// M: OP02 activity@{
    private static final String STK1_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivity";
    private static final String STK2_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityII";
    private static final String STK3_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityIII";
    private static final String STK4_MENU_ACTIVITY_NAME = PACKAGE_NAME + ".StkMenuActivityIV";
    private static final String STK1_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivity";
    private static final String STK2_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityII";
    private static final String STK3_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityIII";
    private static final String STK4_INPUT_ACTIVITY_NAME = PACKAGE_NAME + ".StkInputActivityIV";
    private static final String STK1_DIALOG_ACTIVITY_NAME = PACKAGE_NAME + ".StkDialogActivity";
    private static final String STK2_DIALOG_ACTIVITY_NAME = PACKAGE_NAME + ".StkDialogActivityII";
    private static final String STK3_DIALOG_ACTIVITY_NAME = PACKAGE_NAME + ".StkDialogActivityIII";
    private static final String STK4_DIALOG_ACTIVITY_NAME = PACKAGE_NAME + ".StkDialogActivityIV";
    /// @}
    // Notification id used to display Idle Mode text in NotificationManager.
    private static final int STK_NOTIFICATION_ID = 333;
    // Notification channel containing all mobile service messages notifications.
    private static final String STK_NOTIFICATION_CHANNEL_ID = "mobileServiceMessages";

    private static final String LOG_TAG = new Object(){}.getClass().getEnclosingClass().getName();
    static final boolean isOP09
            = "OP09".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    static final String SESSION_ENDED = "session_ended";
    static final boolean isOP02
            = "OP02".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));
    static final boolean isOP154
            = "OP154".equalsIgnoreCase(SystemProperties.get("persist.vendor.operator.optr", ""));

    // Inner class used for queuing telephony messages (proactive commands,
    // session end) while the service is busy processing a previous message.
    private class DelayedCmd {
        // members
        int id;
        MtkCatCmdMessage msg;
        int slotId;

        DelayedCmd(int id, MtkCatCmdMessage msg, int slotId) {
            this.id = id;
            this.msg = msg;
            this.slotId = slotId;
        }
    }

    // system property to set the STK specific default url for launch browser proactive cmds
    private static final String STK_BROWSER_DEFAULT_URL_SYSPROP = "persist.radio.stk.default_url";

    private static final int NOTIFICATION_ON_KEYGUARD = 1;
    private static final long[] VIBRATION_PATTERN = new long[] { 0, 350, 250, 350 };
    private BroadcastReceiver mUserPresentReceiver = null;

    /// M:  @{
    //device Id
    static final int DEV_ID_KEYPAD = 0x01;
    static final int DEV_ID_DISPLAY = 0x02;
    static final int DEV_ID_EARPIECE = 0x03;
    static final int DEV_ID_UICC = 0x81;
    static final int DEV_ID_TERMINAL = 0x82;
    static final int DEV_ID_NETWORK = 0x83;

    //SET UP CALL qualifier
    static final int SETUP_CALL_NO_CALL_1 = 0x00;
    static final int SETUP_CALL_NO_CALL_2 = 0x01;
    static final int SETUP_CALL_HOLD_CALL_1 = 0x02;
    static final int SETUP_CALL_HOLD_CALL_2 = 0x03;
    static final int SETUP_CALL_END_CALL_1 = 0x04;
    static final int SETUP_CALL_END_CALL_2 = 0x05;
    /// @}


    @Override
    public void onCreate() {
        MtkCatLog.d(LOG_TAG, "onCreate()+");
        // Initialize members
        int i = 0;
        mContext = getBaseContext();
        mSimCount = TelephonyManager.from(mContext).getSimCount();
        MtkCatLog.d(LOG_TAG, "simCount: " + mSimCount);
        mStkService = new MtkAppInterface[mSimCount];
        mStkContext = new StkContext[mSimCount];

        //MTK add begin
        mPhone = new Phone[mSimCount];
        mImsManager = new ImsManager[mSimCount];
        //MTK add end

        for (i = 0; i < mSimCount; i++) {
            MtkCatLog.d(LOG_TAG, "slotId: " + i);
            mStkService[i] = com.mediatek.internal.telephony.cat.MtkCatService.getInstance(i);
            mStkContext[i] = new StkContext();
            mStkContext[i].mSlotId = i;
            mStkContext[i].mCmdsQ = new LinkedList<DelayedCmd>();

            //MTK add begin
            //make phone by slot id, init it as null firstly
            mPhone[i] = null;
            mImsManager[i] = null;
            if (TelephonyManager.getDefault().hasIccCard(i)) {
                getPhone(i);
                getImsManager(i);
            } else {
                MtkCatLog.w(LOG_TAG, "no insert sim: " + i);
                mPhone[i] = null;
                mImsManager[i] = null;
            }
            //MTK add begin

        }

        serviceThread = new Thread(null, this, "Stk App Service");
        serviceThread.start();
        mNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        sInstance = this;

        //MTK add for regiter receiver begin
        registerReceiver(mBrowserTerminationReceiver, new IntentFilter(ACTION_BROWSER_TERMINATION));
        registerReceiver(mBrowsingStatusReceiver, new IntentFilter(ACTION_BROWSER_SEND_ERROR));

        registerReceiver(mAirplaneModeReceiver, new IntentFilter(
                Intent.ACTION_AIRPLANE_MODE_CHANGED));

        registerMSIMModeObserver();

        if(isOP154){
            registerImsListener();
        }
        //MTK add for regiter receiver end
    }

    @Override
    public void onStart(Intent intent, int startId) {
        if (intent == null) {
            MtkCatLog.d(LOG_TAG, "StkAppService onStart intent is null so return");
            return;
        }

        Bundle args = intent.getExtras();
        if (args == null) {
            MtkCatLog.d(LOG_TAG, "StkAppService onStart args is null so return");
            return;
        }

        int op = args.getInt(OPCODE);
        int slotId = 0;
        int i = 0;
        if (op != OP_BOOT_COMPLETED) {
            slotId = args.getInt(SLOT_ID);
        }

        /// M: to handle setupcall. @{
        StkAppInstaller appInstaller = StkAppInstaller.getInstance();
        /// @}
        MtkCatLog.d(LOG_TAG, "onStart sim id: " + slotId + ", op: " + op + ", *****");
        if ((slotId >= 0 && slotId < mSimCount) && mStkService[slotId] == null) {
            mStkService[slotId] = com.mediatek.internal.telephony.cat.MtkCatService
                    .getInstance(slotId);
            if (mStkService[slotId] == null) {
                MtkCatLog.d(LOG_TAG, "mStkService is: " + mStkContext[slotId].mStkServiceState);
                mStkContext[slotId].mStkServiceState = STATE_NOT_EXIST;
                //Check other StkService state.
                //If all StkServices are not available, stop itself and uninstall apk.
                for (i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                    if (i != slotId
                            && (mStkService[i] != null)
                            && (mStkContext[i].mStkServiceState == STATE_UNKNOWN
                            || mStkContext[i].mStkServiceState == STATE_EXIST)) {
                       break;
                   }
                }
            } else {
                mStkContext[slotId].mStkServiceState = STATE_EXIST;
            }
            if (i == mSimCount) {
                stopSelf();
                //will never unInstall stkMain
                //appInstaller.unInstall(mContext);
                return;
            }
        }

        waitForLooper();

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = op;
        msg.arg2 = slotId;
        switch(msg.arg1) {
        case OP_CMD:
            msg.obj = args.getParcelable(CMD_MSG);
            break;
        case OP_RESPONSE:
        case OP_CARD_STATUS_CHANGED:
        case OP_LOCALE_CHANGED:
        case OP_ALPHA_NOTIFY:
        case OP_IDLE_SCREEN:
            msg.obj = args;
            /* falls through */
        case OP_LAUNCH_APP:
        case OP_END_SESSION:
        case OP_BOOT_COMPLETED:
            break;
        case OP_STOP_TONE_USER:
            msg.obj = args;
            msg.what = STOP_TONE_WHAT;
            break;

        case OP_REMOVE_CACHED_SETUP_MENU:
        case OP_LAUNCH_CACHED_SETUP_MENU:
            break;

        default:
            return;
        }
        mServiceHandler.sendMessage(msg);
    }

    @Override
    public void onDestroy() {
        MtkCatLog.d(LOG_TAG, "onDestroy()");
        unregisterUserActivityReceiver();
        unregisterProcessObserver();
        unregisterLocaleChangeReceiver();
        sInstance = null;
        waitForLooper();
        mServiceLooper.quit();

        //MTK add for unregister receiver begin
        unregisterReceiver(mBrowserTerminationReceiver);
        unregisterReceiver(mBrowsingStatusReceiver);
        unregisterReceiver(mAirplaneModeReceiver);
        unRegisterMSIMModeObserver();
        if(isOP154) {
            unregisterImsListener();
        }
        //MTK add for unregister receiver begin
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    public void run() {
        Looper.prepare();

        mServiceLooper = Looper.myLooper();
        mServiceHandler = new ServiceHandler();

        Looper.loop();
    }

    /*
     * Package api used by StkMenuActivity to indicate if its on the foreground.
     */
    void indicateMenuVisibility(boolean visibility, int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            mStkContext[slotId].mMenuIsVisible = visibility;
        }
    }

    /*
     * Package api used by StkDialogActivity to indicate if its on the foreground.
     */
    void setDisplayTextDlgVisibility(boolean visibility, int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            mStkContext[slotId].mDisplayTextDlgIsVisibile = visibility;
        }
    }

    boolean isInputPending(int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            MtkCatLog.d(LOG_TAG, "isInputFinishBySrv: " + mStkContext[slotId].mIsInputPending);
            return mStkContext[slotId].mIsInputPending;
        }
        return false;
    }

    boolean isMenuPending(int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            MtkCatLog.d(LOG_TAG, "isMenuPending: " + mStkContext[slotId].mIsMenuPending);
            return mStkContext[slotId].mIsMenuPending;
        }
        return false;
    }

    boolean isDialogPending(int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            MtkCatLog.d(LOG_TAG, "isDialogPending: " + mStkContext[slotId].mIsDialogPending);
            return mStkContext[slotId].mIsDialogPending;
        }
        return false;
    }

    boolean isMainMenuAvailable(int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            // The main menu can handle the next user operation if the previous session finished.
            return (mStkContext[slotId].lastSelectedItem == null) ? true : false;
        }
        return false;
    }

    /*
     * Package api used by StkMenuActivity to get its Menu parameter.
     */
    Menu getMenu(int slotId) {
        MtkCatLog.d(LOG_TAG, "StkAppService, getMenu, sim id: " + slotId);
        if (slotId >=0 && slotId < mSimCount) {
            return mStkContext[slotId].mCurrentMenu;
        } else {
            return null;
        }
    }

    /*
     * Package api used by StkMenuActivity to get its Main Menu parameter.
     */
    Menu getMainMenu(int slotId) {
        MtkCatLog.d(LOG_TAG, "StkAppService, getMainMenu, sim id: " + slotId);
        if (slotId >=0 && slotId < mSimCount && (mStkContext[slotId].mMainCmd != null)) {
            Menu menu = mStkContext[slotId].mMainCmd.getMenu();
            if (menu != null && mSimCount > PhoneConstants.MAX_PHONE_COUNT_SINGLE_SIM) {
                // If alpha identifier or icon identifier with the self-explanatory qualifier is
                // specified in SET-UP MENU command, it should be more prioritized than preset ones.
                MtkCatLog.d(LOG_TAG, "StkAppService, getMainMenu, menu.title: " + menu.title);
                if (menu.title == null
                        && (menu.titleIcon == null || !menu.titleIconSelfExplanatory)) {
                    StkMenuConfig config = StkMenuConfig.getInstance(getApplicationContext());
                    String label = config.getLabel(slotId);
                    Bitmap icon = config.getIcon(slotId);
                    if (label != null || icon != null) {
                        Parcel parcel = Parcel.obtain();
                        menu.writeToParcel(parcel, 0);
                        parcel.setDataPosition(0);
                        menu = Menu.CREATOR.createFromParcel(parcel);
                        parcel.recycle();
                        menu.title = label;
                        menu.titleIcon = icon;
                        menu.titleIconSelfExplanatory = false;
                    }
                }
            }
            return menu;
        } else {
            return null;
        }
    }

    /*
     * Package api used by UI Activities and Dialogs to communicate directly
     * with the service to deliver state information and parameters.
     */
    static StkAppService getInstance() {
        return sInstance;
    }

    private void waitForLooper() {
        while (mServiceHandler == null) {
            if (serviceThread == null || serviceThread.isAlive() == false) {
                MtkCatLog.w(LOG_TAG, "do re-init");
                init();
            }
            synchronized (this) {
                try {
                    wait(100);
                } catch (InterruptedException e) {
                }
            }
        }
    }

    private void init() {

        mContext = getBaseContext();
        mSimCount = TelephonyManager.from(mContext).getSimCount();
        MtkCatLog.v(LOG_TAG, "init()  simCount: " + mSimCount);
        // TODO:
        serviceThread = new Thread(null, this, "Stk App Service");
        serviceThread.start();

        registerReceiver(mBrowserTerminationReceiver, new IntentFilter(ACTION_BROWSER_TERMINATION));
        registerReceiver(mBrowsingStatusReceiver, new IntentFilter(ACTION_BROWSER_SEND_ERROR));

        registerMSIMModeObserver();
        registerReceiver(mAirplaneModeReceiver, new IntentFilter(
                Intent.ACTION_AIRPLANE_MODE_CHANGED));

        MtkCatLog.v(LOG_TAG, " init()-");
    }

    private final class ServiceHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            if(null == msg) {
                MtkCatLog.d(LOG_TAG, "ServiceHandler handleMessage msg is null");
                return;
            }
            int opcode = msg.arg1;
            int slotId = msg.arg2;

            MtkCatLog.d(LOG_TAG, "handleMessage opcode[" + opcode + "], sim id[" + slotId + "]");
            if (opcode == OP_CMD && msg.obj != null &&
                    ((CatCmdMessage)msg.obj).getCmdType()!= null) {
                MtkCatLog.d(LOG_TAG, "cmdName[" +
                        ((CatCmdMessage)msg.obj).getCmdType().name() + "]");
            }

            if (slotId >= PhoneConstants.SIM_ID_1 && slotId < mSimCount) {
                mStkContext[slotId].mOpCode = opcode;
            }

            switch (opcode) {
            case OP_LAUNCH_APP:
                if (mStkContext[slotId].mMainCmd == null) {
                    MtkCatLog.d(LOG_TAG, "mMainCmd is null");
                    // nothing todo when no SET UP MENU command didn't arrive.

                    //MTK add begin
                    mToast = Toast.makeText(mContext.getApplicationContext(),
                            R.string.main_menu_not_initialized, Toast.LENGTH_LONG);
                    mToast.setGravity(Gravity.BOTTOM, 0, 0);
                    mToast.show();
                    StkAppService.mIsLauncherAcceptInput = true;
                    //MTK add end
                    return;
                }

                //MTK add begin
                if (mStkContext[slotId].mAvailable != STK_AVAIL_AVAILABLE) {
                    mToast = Toast.makeText(mContext.getApplicationContext(),
                            R.string.lable_not_available, Toast.LENGTH_LONG);
                    mToast.setGravity(Gravity.BOTTOM, 0, 0);
                    mToast.show();
                    StkAppService.mIsLauncherAcceptInput = true;
                    return;
                }
                StkAppService.mAppStatus = StkAppService.APP_STATUS_LAUNCHED_NORMAL;
                //MTK add end

                MtkCatLog.d(LOG_TAG, "handleMessage OP_LAUNCH_APP - mCmdInProgress[" +
                        mStkContext[slotId].mCmdInProgress + "]");

                //If there is a pending activity for the slot id,
                //just finish it and create a new one to handle the pending command.
                cleanUpInstanceStackBySlot(slotId);

                MtkCatLog.d(LOG_TAG, "Current cmd type: " +
                        mStkContext[slotId].mCurrentCmd.getCmdType());
                //Restore the last command from stack by slot id.
                restoreInstanceFromStackBySlot(slotId);

                if (isOP02) {
                    showSimSwitchDialog(slotId);
                }
                break;
                // MTK add begin Show SIM switch dialog for CU

            case OP_CMD:
                MtkCatLog.d(LOG_TAG, "[OP_CMD]");
                MtkCatCmdMessage cmdMsg = (MtkCatCmdMessage) msg.obj;
                // There are two types of commands:
                // 1. Interactive - user's response is required.
                // 2. Informative - display a message, no interaction with the user.
                //
                // Informative commands can be handled immediately without any delay.
                // Interactive commands can't override each other. So if a command
                // is already in progress, we need to queue the next command until
                // the user has responded or a timeout expired.

                if (cmdMsg == null) {
                    /* In EMMA test case, cmdMsg may be null */
                    MtkCatLog.d(LOG_TAG, "cmdMsg null");
                    return;
                }

                if (!isCmdInteractive(cmdMsg)) {
                    MtkCatLog.d(LOG_TAG, "[OP_CMD][Normal][Not Interactive]");
                    handleCmd(cmdMsg, slotId);
                } else {
                    //No need check peding immediately display text here,
                    //becase it is check in startActivity
                    if (!mStkContext[slotId].mCmdInProgress) {
                        mStkContext[slotId].mCmdInProgress = true;
                        handleCmd((MtkCatCmdMessage) msg.obj, slotId);
                    } else {
                        MtkCatLog.d(LOG_TAG, "[Interactive][in progress]");
                        mStkContext[slotId].mCmdsQ.addLast(new DelayedCmd(OP_CMD,
                                (MtkCatCmdMessage) msg.obj, slotId));
                    }
                }
                break;
            case OP_RESPONSE:
                //no need to check if response needed because check in stkdialog
                //if immediately reponse, the dialog would not sent response to here
                handleCmdResponse((Bundle) msg.obj, slotId);
                // call delayed commands if needed.
                if (mStkContext[slotId].mCmdsQ.size() != 0) {
                    callDelayedMsg(slotId);
                } else {
                    mStkContext[slotId].mCmdInProgress = false;
                }
                break;
            case OP_END_SESSION:
                MtkCatLog.v(LOG_TAG, "OP_END_SESSION: mCmdInProgress: " +
                    mStkContext[slotId].mCmdInProgress);
                if (!mStkContext[slotId].mCmdInProgress) {
                    mStkContext[slotId].mCmdInProgress = true;
                    handleSessionEnd(slotId);
                } else {
                    mStkContext[slotId].mCmdsQ.addLast(
                            new DelayedCmd(OP_END_SESSION, null, slotId));
                }
                break;
            case OP_BOOT_COMPLETED:
                MtkCatLog.d(LOG_TAG, " OP_BOOT_COMPLETED");
                StkAppInstaller appInstaller = StkAppInstaller.getInstance();
                int i = 0;
                for (i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                    if (mStkContext[i].mMainCmd != null) {
                        break;
                    } else {
                        if (isOP02) {
                            if (MtkCatService.getSaveNewSetUpMenuFlag(i)) {
                                MtkCatLog.d(LOG_TAG, "OP_BOOT_COMPLETED, no need uninstall op02 STK");
                                return;
                            }
                        }
                        MtkCatLog.d(LOG_TAG, "OP_BOOT_COMPLETED, but not menu ccoming, unInstall");
                        appInstaller.unInstall(mContext, i);
                        StkAvailable(slotId, STK_AVAIL_NOT_AVAILABLE);
                    }
                }
                if (i == mSimCount) {
                    //Never unInstall StkMain
                    //appInstaller.unInstall(mContext);
                }
                break;
            case OP_DELAYED_MSG:
                handleDelayedCmd(slotId);
                break;
            case OP_DELAY_TO_CHECK_IDLE_SIM1:
            case OP_DELAY_TO_CHECK_IDLE_SIM2:
            case OP_DELAY_TO_CHECK_IDLE_SIM3:
            case OP_DELAY_TO_CHECK_IDLE_SIM4:
                launchTextDialog(slotId, false);
                break;
            case OP_CARD_STATUS_CHANGED:
                MtkCatLog.d(LOG_TAG, "Card/Icc Status change received");
                handleCardStatusChangeAndIccRefresh((Bundle) msg.obj, slotId);
                break;
            case OP_SET_ACT_INST:
                Activity act = (Activity) msg.obj;
                if (mStkContext[slotId].mActivityInstance != act) {
                    MtkCatLog.d(LOG_TAG, "Set pending activity instance - " + act);
                    Activity previous = mStkContext[slotId].mActivityInstance;
                    mStkContext[slotId].mActivityInstance = act;
                    // Finish the previous one if it was replaced with new one
                    // but it has not been finished yet somehow.
                    if (act != null && previous != null && !previous.isDestroyed()
                            && !previous.isFinishing()) {
                        MtkCatLog.d(LOG_TAG, "Finish the previous pending activity - " + previous);
                        previous.finish();
                    }
                    // Pending activity is registered in the following 2 scnarios;
                    // A. TERMINAL RESPONSE was sent to the card.
                    // B. Activity was moved to the background before TR is sent to the card.
                    // No need to observe idle screen for the pending activity in the scenario A.
                    if (act != null && mStkContext[slotId].mCmdInProgress) {
                        startToObserveIdleScreen(slotId);
                    } else {
                        if (mStkContext[slotId].mCurrentCmd != null) {
                            unregisterProcessObserver(
                                    mStkContext[slotId].mCurrentCmd.getCmdType(), slotId);
                        }
                    }
                }
                break;
            case OP_SET_DAL_INST:
                Activity dal = (Activity) msg.obj;
                if (mStkContext[slotId].mDialogInstance != dal) {
                    MtkCatLog.d(LOG_TAG, "Set pending dialog instance - " + dal);
                    mStkContext[slotId].mDialogInstance = dal;
                    if (dal != null) {
                        startToObserveIdleScreen(slotId);
                    } else {
                        if (mStkContext[slotId].mCurrentCmd != null) {
                            unregisterProcessObserver(
                                    mStkContext[slotId].mCurrentCmd.getCmdType(), slotId);
                        }
                    }
                }
                break;
            case OP_SET_MAINACT_INST:
                Activity mainAct = (Activity) msg.obj;
                MtkCatLog.d(LOG_TAG, "Set Main activity instance. " + mainAct);
                mStkContext[slotId].mMainActivityInstance = mainAct;
                break;
            case OP_SET_IMMED_DAL_INST:
                Activity immedDal = (Activity) msg.obj;
                MtkCatLog.d(LOG_TAG, "Set dialog instance for immediate response. " + immedDal);
                mStkContext[slotId].mImmediateDialogInstance = immedDal;
                break;
            case OP_LOCALE_CHANGED:
                MtkCatLog.d(this, "Locale Changed");
                for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
                    checkForSetupEvent(LANGUAGE_SELECTION_EVENT, (Bundle) msg.obj, slot);
                }
                // rename all registered notification channels on locale change
                createAllChannels();
                break;
            case OP_ALPHA_NOTIFY:
                handleAlphaNotify((Bundle) msg.obj);
                break;
            case OP_IDLE_SCREEN:
               for (int slot = 0; slot < mSimCount; slot++) {
                    if (mStkContext[slot] != null) {
                        handleIdleScreen(slot);
                    }
                }
                break;
            case OP_STOP_TONE_USER:
            case OP_STOP_TONE:
                MtkCatLog.d(this, "Stop tone");
                handleStopTone(msg, slotId);
                break;
            case OP_USER_ACTIVITY:
                for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
                    checkForSetupEvent(USER_ACTIVITY_EVENT, null, slot);
                }
                break;

            //MTK add begin
            case OP_BROWSER_TERMINATION:
                for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
                    checkForSetupEvent(BROWSER_TERMINATION_EVENT, null, slot);
                }
                break;

            case OP_BROWSER_STATUS:
                for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
                    checkForSetupEvent(BROWSING_STATUS_EVENT, null, slot);
                }
                break;

            case OP_REMOVE_CACHED_SETUP_MENU:
                StkAvailable(slotId, STK_AVAIL_NOT_AVAILABLE);
                if (mStkContext[slotId] != null) {
                    mStkContext[slotId].mCurrentMenu = null;
                }

                //no need uninstall because unInstall when handling OP_BOOT_COMPLETE
                if (mStkService[slotId] != null) {
                    mStkService[slotId].onDBHandler(slotId);
                }
                break;

            case OP_LAUNCH_CACHED_SETUP_MENU:
                if (mStkService[slotId] != null) {
                    mStkService[slotId].onLaunchCachedSetupMenu();
                }
                break;

            case OP_DELAY_TO_CHECK_IDLE:
                launchTextDialog(slotId, false);
                break;

            //MTK add end

            }
        }

        private void handleCardStatusChangeAndIccRefresh(Bundle args, int slotId) {
            boolean cardStatus = args.getBoolean(AppInterface.CARD_STATUS);

            MtkCatLog.d(LOG_TAG, "CardStatus: " + cardStatus);

            //MTK add begin
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();
            //MTK add end

            if (cardStatus == false) {
                MtkCatLog.d(LOG_TAG, "CARD is ABSENT");
                // Uninstall STKAPP, Clear Idle text, Stop StkAppService
                cancelIdleText(slotId);

                //MTK add begin
                boolean airPlane = false;
                String optr = SystemProperties.get("persist.operator.optr");

                if (!airPlane) {
                    appInstaller.unInstall(mContext, slotId);
                }

                if (slotId >= mSimCount) {
                    MtkCatLog.d(LOG_TAG, "SAS recreate with different sim count, skip");
                    return;
                }
                //StkAvailable(slotId, false);

                if (mStkContext[slotId] != null) {
                    mStkContext[slotId].mCurrentMenu = null;
                    mStkContext[slotId].mCurrentMenuCmd = null;
                    mStkContext[slotId].mMainCmd = null;
                    mStkContext[slotId].mCmdInProgress = false;
                    if (mStkContext[slotId].mCmdsQ != null &&
                        mStkContext[slotId].mCmdsQ.size() != 0) {
                        MtkCatLog.v(LOG_TAG, "There are commands in queue. size: " +
                        mStkContext[slotId].mCmdsQ.size());
                        mStkContext[slotId].mCmdsQ.clear();
                    }
                    mStkContext[slotId].mSetupMenuState = STATE_NOT_EXIST;
                    mStkContext[slotId].mStkServiceState = STATE_UNKNOWN;
                    mStkContext[slotId].mDelayCheckedCount = 0;
                    mStkContext[slotId].mMenuState = StkMenuActivity.STATE_MAIN;
                }

                if (mServiceHandler != null) {
                    if (PhoneConstants.SIM_ID_1 == slotId) {
                        mServiceHandler.removeMessages(OP_DELAY_TO_CHECK_IDLE_SIM1);
                    } else if (PhoneConstants.SIM_ID_2 == slotId) {
                        mServiceHandler.removeMessages(OP_DELAY_TO_CHECK_IDLE_SIM2);
                    } else if (PhoneConstants.SIM_ID_3 == slotId) {
                        mServiceHandler.removeMessages(OP_DELAY_TO_CHECK_IDLE_SIM3);
                    } else if (PhoneConstants.SIM_ID_4 == slotId) {
                        mServiceHandler.removeMessages(OP_DELAY_TO_CHECK_IDLE_SIM4);
                    }
                }
                //Reset CatService instance.
                mStkService[slotId] = null;

                if (isAllOtherCardsAbsent(slotId)) {
                    MtkCatLog.d(LOG_TAG, "All CARDs are ABSENT");

                    //cause all SIMs maybe absent at same time due to TRM or card slot plug out
                    //Then checking other card absent is true
                    //but the 2nd absent message could not received because of stopping stkappservice here.
                    //So send a new intent for stk pending activities of the other cards
                    Intent intent = new Intent(ACTION_ALL_SIM_ABSENT);
                    LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);

                    //for OP02, remove all STK icon when all cards absent. seldom,
                    //one of STK icon cannot be removed for OP_CARD_STATUS_CHANGED broadcase delay.
                    if (isOP02) {
                        for (int index = 0; index < mSimCount; ++index)
                        {
                            if (index != slotId) {
                                appInstaller.unInstall(mContext, index);
                            }
                        }
                    }
                    //never unInstall Stkmain
                    //appInstaller.unInstall(mContext);
                    stopSelf();
                } else {
                    //StkAvailable(slotId, false);
                    Intent intent = new Intent(ACTION_SIM_ABSENT);
                    intent.putExtra(StkAppService.SLOT_ID, slotId);
                    LocalBroadcastManager.getInstance(mContext).sendBroadcast(intent);
                }
                //MTK add end
            } else {
                IccRefreshResponse state = new IccRefreshResponse();
                state.refreshResult = args.getInt(AppInterface.REFRESH_RESULT);

                MtkCatLog.d(LOG_TAG, "Icc Refresh Result: "+ state.refreshResult);
                if ((state.refreshResult == IccRefreshResponse.REFRESH_RESULT_INIT) ||
                    (state.refreshResult == IccRefreshResponse.REFRESH_RESULT_RESET)) {
                    // Clear Idle Text
                    cancelIdleText(slotId);
                }
            }
        }
    }
    /*
     * Check if all SIMs are absent except the id of slot equals "slotId".
     */
    private boolean isAllOtherCardsAbsent(int slotId) {
        TelephonyManager mTm = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        int i = 0;

        for (i = 0; i < mSimCount; i++) {
            if (i != slotId && mTm.hasIccCard(i)) {
                break;
            }
        }
        if (i == mSimCount) {
            return true;
        } else {
            return false;
        }
    }

    /* package */ boolean isScreenIdle() {
        ActivityManager am = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
        List<RunningTaskInfo> tasks = am.getRunningTasks(1);
        if (tasks == null || tasks.isEmpty()) {
            MtkCatLog.d(this, "isScreenIdle: no task");
            return false;
        }

        String top = tasks.get(0).topActivity.getPackageName();
        if (top == null) {
            MtkCatLog.d(this, "isScreenIdle: no top task");
            return false;
        }

        // We can assume that the screen is idle if the home application is in the foreground.
        final Intent intent = new Intent(Intent.ACTION_MAIN, null);
        intent.addCategory(Intent.CATEGORY_HOME);

        ResolveInfo info = getPackageManager().resolveActivity(intent,
                PackageManager.MATCH_DEFAULT_ONLY);
        if (info != null) {
            if (top.equals(info.activityInfo.packageName)) {
                MtkCatLog.d(this, "isScreenIdle: home application in the foreground");
                return true;
            }
        }

        return false;
    }

    private void startToObserveIdleScreen(int slotId) {
        if (!mStkContext[slotId].mIsSessionFromUser) {
            if (!isScreenIdle()) {
                synchronized (this) {
                    if (mProcessObserver == null && !mServiceHandler.hasMessages(OP_IDLE_SCREEN)) {
                        registerProcessObserver();
                    }
                }
            } else {
                handleIdleScreen(slotId);
            }
        }
    }

    private void handleIdleScreen(int slotId) {
        // It might be hard for user to recognize that the dialog or screens belong to SIM Toolkit
        // application if the current session was not initiated by user but by the SIM card,
        // so it is recommended to send TERMINAL RESPONSE if user goes to the idle screen.
        if (!mStkContext[slotId].mIsSessionFromUser) {
            Activity dialog = mStkContext[slotId].getPendingDialogInstance();
            if (dialog != null) {
                dialog.finish();
                mStkContext[slotId].mDialogInstance = null;
            }
            Activity activity = mStkContext[slotId].getPendingActivityInstance();
            if (activity != null) {
                activity.finish();
                mStkContext[slotId].mActivityInstance = null;
            }
        }
        // If the idle screen event is present in the list need to send the
        // response to SIM.
        MtkCatLog.d(this, "Need to send IDLE SCREEN Available event to SIM");
        checkForSetupEvent(IDLE_SCREEN_AVAILABLE_EVENT, null, slotId);

        if (mStkContext[slotId].mIdleModeTextCmd != null
                && !mStkContext[slotId].mIdleModeTextVisible) {
            launchIdleText(slotId);
        }
    }

    private void sendScreenBusyResponse(int slotId) {
        if (mStkContext[slotId].mCurrentCmd == null) {
            return;
        }
        MtkCatResponseMessage resMsg = new MtkCatResponseMessage(mStkContext[slotId].mCurrentCmd);
        MtkCatLog.d(this, "SCREEN_BUSY");
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        //MTK add begin
        if (mStkService[slotId] == null) {
            MtkCatLog.d(this, "no catservice, return");
            return;
        }
        byte[] additionalInfo = new byte[1];
        additionalInfo[0] = (byte) 0x01; //busy on call
        resMsg.setAdditionalInfo(additionalInfo);
        //MTK add end
        mStkService[slotId].onCmdResponse(resMsg);
        if (mStkContext[slotId].mCmdsQ.size() != 0) {
            callDelayedMsg(slotId);
        } else {
            mStkContext[slotId].mCmdInProgress = false;
        }
    }

    /**
     * Sends TERMINAL RESPONSE or ENVELOPE
     *
     * @param args detailed parameters of the response
     * @param slotId slot identifier
     */
    public void sendResponse(Bundle args, int slotId) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_RESPONSE;
        msg.arg2 = slotId;
        msg.obj = args;
        mServiceHandler.sendMessage(msg);
    }

    private void sendResponse(int resId, int slotId, boolean confirm) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_RESPONSE;
        msg.arg2 = slotId;
        Bundle args = new Bundle();
        args.putInt(StkAppService.RES_ID, resId);
        args.putBoolean(StkAppService.CONFIRMATION, confirm);
        msg.obj = args;
        mServiceHandler.sendMessage(msg);
    }

    private boolean isCmdInteractive(CatCmdMessage cmd) {
        switch (cmd.getCmdType()) {
        case SEND_DTMF:
        case SEND_SMS:
        case REFRESH:
        case RUN_AT:
        case SEND_SS:
        case SEND_USSD:
        case SET_UP_IDLE_MODE_TEXT:
        case SET_UP_MENU:
        case CLOSE_CHANNEL:
        case RECEIVE_DATA:
        case SEND_DATA:
        case SET_UP_EVENT_LIST:
            return false;
        }

        return true;
    }

    private void handleDelayedCmd(int slotId) {
        MtkCatLog.d(LOG_TAG, "handleDelayedCmd, slotId: " + slotId);
        if (mStkContext[slotId].mCmdsQ.size() != 0) {
            DelayedCmd cmd = mStkContext[slotId].mCmdsQ.poll();
            if (cmd != null) {
                MtkCatLog.d(LOG_TAG, "handleDelayedCmd - queue size: " +
                        mStkContext[slotId].mCmdsQ.size() +
                        " id: " + cmd.id + "sim id: " + cmd.slotId);
                switch (cmd.id) {
                case OP_CMD:
                    handleCmd(cmd.msg, cmd.slotId);
                    break;
                case OP_END_SESSION:
                    handleSessionEnd(cmd.slotId);
                    break;
                }
            }
        }
    }

    private void callDelayedMsg(int slotId) {
        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_DELAYED_MSG;
        msg.arg2 = slotId;
        mServiceHandler.sendMessage(msg);
    }

    private void callSetActivityInstMsg(int inst_type, int slotId, Object obj) {
        Message msg = mServiceHandler.obtainMessage();
        msg.obj = obj;
        msg.arg1 = inst_type;
        msg.arg2 = slotId;
        mServiceHandler.sendMessage(msg);
    }

    private void handleSessionEnd(int slotId) {
        // We should finish all pending activity if receiving END SESSION command.
        cleanUpInstanceStackBySlot(slotId);

        mStkContext[slotId].mCurrentCmd = mStkContext[slotId].mMainCmd;
        MtkCatLog.d(LOG_TAG, "[handleSessionEnd] - mCurrentCmd changed to mMainCmd!.");
        mStkContext[slotId].mCurrentMenuCmd = mStkContext[slotId].mMainCmd;
        MtkCatLog.d(LOG_TAG, "slotId: " + slotId + ", mMenuState: " +
                mStkContext[slotId].mMenuState);

        mStkContext[slotId].mIsInputPending = false;
        mStkContext[slotId].mIsMenuPending = false;
        mStkContext[slotId].mIsDialogPending = false;
        mStkContext[slotId].mNoResponseFromUser = false;

        if (mStkContext[slotId].mMainCmd == null) {
            MtkCatLog.d(LOG_TAG, "[handleSessionEnd][mMainCmd is null!]");
        }
        mStkContext[slotId].lastSelectedItem = null;
        mStkContext[slotId].mIsSessionFromUser = false;
        // In case of SET UP MENU command which removed the app, don't
        // update the current menu member.
        if (mStkContext[slotId].mCurrentMenu != null && mStkContext[slotId].mMainCmd != null) {
            mStkContext[slotId].mCurrentMenu = mStkContext[slotId].mMainCmd.getMenu();
        }
        MtkCatLog.d(LOG_TAG, "[handleSessionEnd][mMenuState]" + mStkContext[slotId].mMenuIsVisible);

        //AOSP update in P version, only set menu state, but not launcher mainmenu here
        if (StkMenuActivity.STATE_SECONDARY == mStkContext[slotId].mMenuState) {
            mStkContext[slotId].mMenuState = StkMenuActivity.STATE_MAIN;
        }
        //some card don't wait for the TR of display text before
        //so if have pending dialog delay to show, ommit it cause there is session end.
        if (mServiceHandler != null) {
            mServiceHandler.removeMessages(OP_DELAY_TO_CHECK_IDLE);
        }

        // Send a local broadcast as a notice that this service handled the session end event.
        Intent intent = new Intent(SESSION_ENDED);
        intent.putExtra(SLOT_ID, slotId);
        LocalBroadcastManager.getInstance(this).sendBroadcast(intent);

        if (mStkContext[slotId].mCmdsQ.size() != 0) {
            callDelayedMsg(slotId);
        } else {
            mStkContext[slotId].mCmdInProgress = false;
        }
        // In case a launch browser command was just confirmed, launch that url.
        if (mStkContext[slotId].launchBrowser) {
            mStkContext[slotId].launchBrowser = false;
            launchBrowser(mStkContext[slotId].mBrowserSettings);
        }
    }

    // returns true if any Stk related activity already has focus on the screen
    boolean isTopOfStack() {
        ActivityManager mActivityManager = (ActivityManager) mContext
                .getSystemService(ACTIVITY_SERVICE);
        String currentPackageName = null;
        List<RunningTaskInfo> tasks = mActivityManager.getRunningTasks(1);
        if (tasks == null || tasks.size() == 0 || tasks.get(0).topActivity == null) {
            return false;
        }
        currentPackageName = tasks.get(0).topActivity.getPackageName();
        if (null != currentPackageName) {
            return currentPackageName.equals(PACKAGE_NAME);
        }
        return false;
    }

    /**
     * Get the boolean config from carrier config manager.
     *
     * @param key config key defined in CarrierConfigManager
     * @param slotId slot ID.
     * @return boolean value of corresponding key.
     */
    private boolean getBooleanCarrierConfig(String key, int slotId) {
        CarrierConfigManager ccm = (CarrierConfigManager) getSystemService(CARRIER_CONFIG_SERVICE);
        SubscriptionManager sm = (SubscriptionManager) getSystemService(
                Context.TELEPHONY_SUBSCRIPTION_SERVICE);
        PersistableBundle b = null;
        if (ccm != null && sm != null) {
            SubscriptionInfo info = sm.getActiveSubscriptionInfoForSimSlotIndex(slotId);
            if (info != null) {
                b = ccm.getConfigForSubId(info.getSubscriptionId());
            }
        }
        if (b != null) {
            return b.getBoolean(key);
        }
        // Return static default defined in CarrierConfigManager.
        return CarrierConfigManager.getDefaultConfig().getBoolean(key);
    }

    private void handleCmd(MtkCatCmdMessage cmdMsg, int slotId) {

        if (cmdMsg == null) {
            return;
        }
        // save local reference for state tracking.
        mStkContext[slotId].mCurrentCmd = cmdMsg;
        boolean waitForUsersResponse = true;

        mStkContext[slotId].mIsInputPending = false;
        mStkContext[slotId].mIsMenuPending = false;
        mStkContext[slotId].mIsDialogPending = false;

        MtkCatLog.d(LOG_TAG,"[handleCmd]" + cmdMsg.getCmdType().name());
        switch (cmdMsg.getCmdType()) {
        case DISPLAY_TEXT:
            TextMessage msg = cmdMsg.geTextMessage();
            waitForUsersResponse = msg.responseNeeded;
            if (mStkContext[slotId].lastSelectedItem != null) {
                msg.title = mStkContext[slotId].lastSelectedItem;
            } else if (mStkContext[slotId].mMainCmd != null){
                if (!getResources().getBoolean(R.bool.show_menu_title_only_on_menu)) {
                    msg.title = mStkContext[slotId].mMainCmd.getMenu().title;
                }
            }
            //MTK add begin
            launchTextDialog(slotId, false);
            //MTK add end

            break;
        case SELECT_ITEM:
            MtkCatLog.d(LOG_TAG, "SELECT_ITEM +");
            mStkContext[slotId].mCurrentMenuCmd = mStkContext[slotId].mCurrentCmd;
            mStkContext[slotId].mCurrentMenu = cmdMsg.getMenu();

            //MTK add begin
            //don't allow handle stk if radio off or sim off
            if (checkSimRadioStateByModeSetting(slotId, false)) {
                launchMenuActivity(cmdMsg.getMenu(), slotId, false);
            } else {
                MtkCatLog.d(LOG_TAG, "radio off or sim off, send TR directly.");
                mStkContext[slotId].mCmdInProgress = false;
                MtkCatResponseMessage resMsg = new MtkCatResponseMessage(
                        mStkContext[slotId].mCurrentCmd);
                resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
                if(null != mStkService[slotId]) {
                    mStkService[slotId].onCmdResponse(resMsg);
                }
            }
            //MTK add end

            break;
        case SET_UP_MENU:

            //MTK add begin
            MtkCatLog.v(LOG_TAG, "[handleCmd][SET_UP_MENU] +, from modem: "
                    + ((MtkMenu)(cmdMsg.getMenu())).getSetUpMenuFlag());

            //because the setupmenu maybe come form cache but not modem, need to check it
            if (((MtkMenu)(cmdMsg.getMenu())).getSetUpMenuFlag() == 1) {
                if (mStkContext[slotId].mCmdsQ.size() != 0) {
                    mStkContext[slotId].mCmdsQ.clear();
                }
            }
            //MTK add end

            mStkContext[slotId].mCmdInProgress = false;
            mStkContext[slotId].mMainCmd = mStkContext[slotId].mCurrentCmd;
            mStkContext[slotId].mCurrentMenuCmd = mStkContext[slotId].mCurrentCmd;
            mStkContext[slotId].mCurrentMenu = cmdMsg.getMenu();

            //MTK add begin
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();

            boolean sim_radio_on = true;
            boolean sim_locked = false;
            boolean isWfcEnabled = ((TelephonyManager) mContext
                    .getSystemService(Context.TELEPHONY_SERVICE)).isWifiCallingAvailable();
            sim_radio_on = checkSimRadioStateByModeSetting(slotId, true);

            MtkCatLog.v(LOG_TAG, "StkAppService - SET_UP_MENU sim_radio_on[" + sim_radio_on + "]"
                    + ", [" + removeMenu(slotId) + "]");
            //MTK add end

            if (removeMenu(slotId)) {
                int i = 0;
                MtkCatLog.d(LOG_TAG, "removeMenu() - Uninstall App");
                mStkContext[slotId].mCurrentMenu = null;
                mStkContext[slotId].mMainCmd = null;
                appInstaller.unInstall(mContext, slotId);
                StkAvailable(slotId, STK_AVAIL_NOT_AVAILABLE);

                //Check other setup menu state. If all setup menu are removed, uninstall apk.
                for (i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                    if (i != slotId && mStkContext[i].mSetupMenuState != STATE_NOT_EXIST) {
                        MtkCatLog.d(LOG_TAG, "Not Uninstall App:" + i + ","
                                + mStkContext[i].mSetupMenuState);
                        break;
                    }
                }
                if (i == mSimCount) {
                    MtkCatLog.v(LOG_TAG, "All Stk menu are removed.");
                    //appInstaller.unInstall(mContext);
                }
            } else if (!sim_radio_on || sim_locked) {
                if (!isOP02) {
                    appInstaller.unInstall(mContext, slotId);
                } else {
                    // For op02
                    if (MtkCatService.getSaveNewSetUpMenuFlag(slotId)) {
                        appInstaller.install(mContext, slotId);
                    }
                }
                StkAvailable(slotId, STK_AVAIL_NOT_AVAILABLE);
            } else {
                MtkCatLog.d(LOG_TAG, "install App, slotId: " + slotId);
                appInstaller.install(mContext, slotId);
                StkAvailable(slotId, STK_AVAIL_AVAILABLE);
            }
            if (mStkContext[slotId].mMenuIsVisible) {
                //MTK add begin
                MtkCatLog.v(LOG_TAG,
                        "finish pending menu/mainMenu because of new set up menu coming");
                Activity PendingMenu = mStkContext[slotId].getPendingActivityInstance();
                Activity PendingMainMenu = mStkContext[slotId].getMainActivityInstance();
                if (PendingMenu != null) {
                    PendingMenu.finish();
                    mStkContext[slotId].mActivityInstance = null;
                }
                if (PendingMainMenu != null) {
                    PendingMainMenu.finish();
                    mStkContext[slotId].mMainActivityInstance = null;
                }
                //MTK add begin
                launchMenuActivity(null, slotId, false);
            }
            break;
        case GET_INPUT:
        case GET_INKEY:
            //clean up previous activity instance.
            //e.g. when stk input times out and get input is coming later.
            cleanUpInstanceStackBySlot(slotId);
            launchInputActivity(slotId, false);
            break;
        case SET_UP_IDLE_MODE_TEXT:
            waitForUsersResponse = false;
            mStkContext[slotId].mIdleModeTextCmd = mStkContext[slotId].mCurrentCmd;
            TextMessage idleModeText = mStkContext[slotId].mCurrentCmd.geTextMessage();
            if (idleModeText == null || TextUtils.isEmpty(idleModeText.text)) {
                cancelIdleText(slotId);
            }
            mStkContext[slotId].mCurrentCmd = mStkContext[slotId].mMainCmd;
            if (mStkContext[slotId].mIdleModeTextCmd != null) {
                if (mStkContext[slotId].mIdleModeTextVisible || isScreenIdle()) {
                    MtkCatLog.d(this, "set up idle mode, mIdleModeTextVisible: " +
                        mStkContext[slotId].mIdleModeTextVisible + "isScreenIdle: " +
                        isScreenIdle());
                    launchIdleText(slotId);
                } else {
                    registerProcessObserver();
                }
            }
            break;
        case SEND_DTMF:
        case SEND_SMS:
        case REFRESH:
        case RUN_AT:
        case SEND_SS:
        case SEND_USSD:
        case GET_CHANNEL_STATUS:
            waitForUsersResponse = false;
            launchEventMessage(slotId);
            break;
        case LAUNCH_BROWSER:
            // The device setup process should not be interrupted by launching browser.
            if (Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.DEVICE_PROVISIONED, 0) == 0) {
                MtkCatLog.d(this, "The command is not performed if setup has not been completed.");
                sendScreenBusyResponse(slotId);
                break;
            }

            /* Check if Carrier would not want to launch browser */
            if (getBooleanCarrierConfig(CarrierConfigManager.KEY_STK_DISABLE_LAUNCH_BROWSER_BOOL,
                    slotId)) {
                MtkCatLog.d(this, "Browser is not launched as per carrier.");
                sendResponse(RES_ID_DONE, slotId, true);
                break;
            }

            mStkContext[slotId].mBrowserSettings =
                    mStkContext[slotId].mCurrentCmd.getBrowserSettings();
            if (!isUrlAvailableToLaunchBrowser(mStkContext[slotId].mBrowserSettings)) {
                MtkCatLog.d(this, "Browser url property is not set - send error");
                sendResponse(RES_ID_ERROR, slotId, true);
            } else {
                TextMessage alphaId = mStkContext[slotId].mCurrentCmd.geTextMessage();
                if ((alphaId == null) || TextUtils.isEmpty(alphaId.text)) {
                    // don't need user confirmation in this case
                    // just launch the browser or spawn a new tab
                    MtkCatLog.d(this, "user confirmation is not currently needed.\n" +
                            "supressing confirmation dialogue and confirming silently...");
                    mStkContext[slotId].launchBrowser = true;
                    sendResponse(RES_ID_CONFIRM, slotId, true);
                } else {
                    launchConfirmationDialog(alphaId, slotId, false);
                }
            }
            break;
        case SET_UP_CALL:
            TextMessage mesg = mStkContext[slotId].mCurrentCmd.getCallSettings().confirmMsg;
            if((mesg != null) && (mesg.text == null || mesg.text.length() == 0)) {
                mesg.text = getResources().getString(R.string.default_setup_call_msg);
            }
            CatLog.d(this, "SET_UP_CALL mesg.text " +
                    (Build.IS_DEBUGGABLE ? mesg.text : "********"));
            launchConfirmationDialog(mesg, slotId, false);
            break;
        case PLAY_TONE:
            handlePlayTone(slotId);
            break;
        case OPEN_CHANNEL:
            processOpenChannel(slotId);
            break;
        case CLOSE_CHANNEL:
        case RECEIVE_DATA:
        case SEND_DATA:
            //MTK add begin
            waitForUsersResponse = false;
            //MTK add begin
            TextMessage m = mStkContext[slotId].mCurrentCmd.geTextMessage();

            if ((m != null) && (m.text == null)) {
                switch(cmdMsg.getCmdType()) {
                case CLOSE_CHANNEL:
                    m.text = getResources().getString(R.string.default_close_channel_msg);
                    break;
                case RECEIVE_DATA:
                    m.text = getResources().getString(R.string.default_receive_data_msg);
                    break;
                case SEND_DATA:
                    m.text = getResources().getString(R.string.default_send_data_msg);
                    break;
                }
            }
            /*
             * Display indication in the form of a toast to the user if required.
             */
            if (!isOP09) {
                launchEventMessage(slotId);
            }
            break;
        case SET_UP_EVENT_LIST:
            replaceEventList(slotId);
            if (isScreenIdle()) {
                MtkCatLog.d(this," Check if IDLE_SCREEN_AVAILABLE_EVENT is present in List");
                checkForSetupEvent(IDLE_SCREEN_AVAILABLE_EVENT, null, slotId);
            }
            break;
        }

        if (!waitForUsersResponse) {
            if (mStkContext[slotId].mCmdsQ.size() != 0) {
                callDelayedMsg(slotId);
            } else {
                mStkContext[slotId].mCmdInProgress = false;
            }
        }
    }

    @SuppressWarnings("FallThrough")
    private void handleCmdResponse(Bundle args, int slotId) {
        MtkCatLog.d(LOG_TAG, "handleCmdResponse, sim id: " + slotId);
        if (mStkContext[slotId].mCurrentCmd == null) {
            return;
        }

        if (mStkService[slotId] == null) {
            mStkService[slotId] = com.mediatek.internal.telephony.cat.MtkCatService
                    .getInstance(slotId);
            if (mStkService[slotId] == null) {
                // This should never happen (we should be responding only to a message
                // that arrived from StkService). It has to exist by this time
                MtkCatLog.d(LOG_TAG, "Exception! mStkService null when we need to send response.");
                //throw new RuntimeException("mStkService is null when we need to send response");
            }
        }

        CatResponseMessage resMsg = new CatResponseMessage(mStkContext[slotId].mCurrentCmd);

        //MTK add begin
        boolean skip_timeout = false;
        if (null != mStkContext[slotId].mCurrentCmd
                && null != mStkContext[slotId].mCurrentCmd.getCmdType()) {
            MtkCatLog.d(LOG_TAG, "handleCmdResponse+ cmdName["
                + mStkContext[slotId].mCurrentCmd.getCmdType().name() + "]");
            if (mStkContext[slotId].mCurrentCmd.getCmdType()
                    == MtkAppInterface.CommandType.SEND_DATA
                    || mStkContext[slotId].mCurrentCmd.getCmdType()
                    == MtkAppInterface.CommandType.RECEIVE_DATA
                    || mStkContext[slotId].mCurrentCmd.getCmdType()
                    == MtkAppInterface.CommandType.CLOSE_CHANNEL
                    || mStkContext[slotId].mCurrentCmd.getCmdType()
                    == MtkAppInterface.CommandType.SET_UP_MENU) {
                skip_timeout = true;
            }
        }
        //MTK add end


        // set result code
        boolean helpRequired = args.getBoolean(HELP, false);
        boolean confirmed    = false;

        switch(args.getInt(RES_ID)) {
        case RES_ID_MENU_SELECTION:
            if (null == mStkContext[slotId].mCurrentMenuCmd) {
                MtkCatLog.d(LOG_TAG, "mCurrentMenuCmd == null");
                return;
            }
            MtkCatLog.d(LOG_TAG, "MENU_SELECTION=" + mStkContext[slotId].
                    mCurrentMenuCmd.getCmdType());

            if (isBipCommand(mStkContext[slotId].mCurrentCmd)) {
                Toast toast = Toast.makeText(mContext.getApplicationContext(),
                        R.string.lable_busy_on_bip, Toast.LENGTH_LONG);
                toast.setGravity(Gravity.BOTTOM, 0, 0);
                toast.show();
                return;
            }

            int menuSelection = args.getInt(MENU_SELECTION);
            switch(mStkContext[slotId].mCurrentMenuCmd.getCmdType()) {
            case SET_UP_MENU:
                mStkContext[slotId].mIsSessionFromUser = true;
                // Fall through
            case SELECT_ITEM:
                mStkContext[slotId].lastSelectedItem = getItemName(menuSelection, slotId);
                //MTK add begin
                resMsg = new MtkCatResponseMessage(mStkContext[slotId].mCurrentMenuCmd);
                //MTK add end
                if (helpRequired) {
                    resMsg.setResultCode(ResultCode.HELP_INFO_REQUIRED);
                } else {
                    resMsg.setResultCode(mStkContext[slotId].mCurrentCmd.hasIconLoadFailed() ?
                            ResultCode.PRFRMD_ICON_NOT_DISPLAYED : ResultCode.OK);
                }
                resMsg.setMenuSelection(menuSelection);
                break;
            }
            break;
        case RES_ID_INPUT:
            MtkCatLog.d(LOG_TAG, "RES_ID_INPUT");
            String input = args.getString(INPUT);
            if (input != null && (null != mStkContext[slotId].mCurrentCmd.geInput()) &&
                    (mStkContext[slotId].mCurrentCmd.geInput().yesNo)) {
                boolean yesNoSelection = input
                        .equals(StkInputActivity.YES_STR_RESPONSE);
                resMsg.setYesNo(yesNoSelection);
            } else {
                if (helpRequired) {
                    resMsg.setResultCode(ResultCode.HELP_INFO_REQUIRED);
                } else {
                    resMsg.setResultCode(mStkContext[slotId].mCurrentCmd.hasIconLoadFailed() ?
                            ResultCode.PRFRMD_ICON_NOT_DISPLAYED : ResultCode.OK);
                    resMsg.setInput(input);
                }
            }
            break;
        case RES_ID_CONFIRM:
            MtkCatLog.d(this, "RES_ID_CONFIRM");
            confirmed = args.getBoolean(CONFIRMATION);
            switch (mStkContext[slotId].mCurrentCmd.getCmdType()) {
            case DISPLAY_TEXT:
                if (confirmed) {
                    resMsg.setResultCode(mStkContext[slotId].mCurrentCmd.hasIconLoadFailed() ?
                            ResultCode.PRFRMD_ICON_NOT_DISPLAYED : ResultCode.OK);
                } else {
                    resMsg.setResultCode(ResultCode.UICC_SESSION_TERM_BY_USER);
                }
                break;
            case LAUNCH_BROWSER:
                //MTK add begin
                resMsg.setResultCode(confirmed ? ResultCode.OK : ResultCode.USER_NOT_ACCEPT);
                //MTK add end
                if (confirmed) {
                    mStkContext[slotId].launchBrowser = true;
                    mStkContext[slotId].mBrowserSettings =
                            mStkContext[slotId].mCurrentCmd.getBrowserSettings();
                }
                break;
            case SET_UP_CALL:
                resMsg.setResultCode(ResultCode.OK);
                resMsg.setConfirmation(confirmed);

                if (confirmed) {
                    launchEventMessage(slotId,
                            mStkContext[slotId].mCurrentCmd.getCallSettings().callMsg);
                }
                break;
            case OPEN_CHANNEL:
                if (confirmed) {
                    processNormalOpenChannelResponse(slotId);
                    return;
                }
                resMsg.setResultCode(ResultCode.USER_NOT_ACCEPT);
                resMsg.setConfirmation(confirmed);
                break;
            }
            break;
        case RES_ID_DONE:
            resMsg.setResultCode(ResultCode.OK);
            break;
        case RES_ID_BACKWARD:
            MtkCatLog.d(LOG_TAG, "RES_ID_BACKWARD");
            //MTK add begin
            switch (mStkContext[slotId].mCurrentCmd.getCmdType()) {
                case OPEN_CHANNEL:
                    MtkCatLog.d(LOG_TAG, "RES_ID_BACKWARD - OPEN_CHANNEL");
                    resMsg.setResultCode(ResultCode.UICC_SESSION_TERM_BY_USER);
                    break;

                default:
                    resMsg.setResultCode(ResultCode.BACKWARD_MOVE_BY_USER);
                    break;
            }
            //resMsg.setResultCode(ResultCode.BACKWARD_MOVE_BY_USER);
            //MTK add end
            break;
        case RES_ID_END_SESSION:
            MtkCatLog.d(LOG_TAG, "RES_ID_END_SESSION");
            resMsg.setResultCode(ResultCode.UICC_SESSION_TERM_BY_USER);
            break;
        case RES_ID_TIMEOUT:
            MtkCatLog.d(LOG_TAG, "RES_ID_TIMEOUT");
            if (!skip_timeout) {
                // GCF test-case 27.22.4.1.1 Expected Sequence 1.5 (DISPLAY TEXT,
                // Clear message after delay, successful) expects result code OK.
                // If the command qualifier specifies no user response is required
                // then send OK instead of NO_RESPONSE_FROM_USER
                if ((mStkContext[slotId].mCurrentCmd.getCmdType().value() ==
                        AppInterface.CommandType.DISPLAY_TEXT.value())
                        && (mStkContext[slotId].mCurrentCmd.geTextMessage().userClear == false)) {
                    resMsg.setResultCode(ResultCode.OK);
                } else {
                    resMsg.setResultCode(ResultCode.NO_RESPONSE_FROM_USER);
                }
            } else {
                MtkCatLog.v(LOG_TAG, "Skip timeout because the command no need");
            }
            break;
        case RES_ID_CHOICE:
            int choice = args.getInt(CHOICE);
            MtkCatLog.d(this, "User Choice=" + choice);
            switch (choice) {
                case YES:
                    resMsg.setResultCode(ResultCode.OK);
                    confirmed = true;
                    break;
                case NO:
                    resMsg.setResultCode(ResultCode.USER_NOT_ACCEPT);
                    break;
            }

            if (mStkContext[slotId].mCurrentCmd.getCmdType().value() ==
                    AppInterface.CommandType.OPEN_CHANNEL.value()) {
                resMsg.setConfirmation(confirmed);
            }
            break;
        case RES_ID_ERROR:
            MtkCatLog.d(LOG_TAG, "RES_ID_ERROR");
            switch (mStkContext[slotId].mCurrentCmd.getCmdType()) {
            case LAUNCH_BROWSER:
                resMsg.setResultCode(ResultCode.LAUNCH_BROWSER_ERROR);
                break;
            }
            break;
        default:
            MtkCatLog.d(LOG_TAG, "Unknown result id");
            return;
        }

        switch (args.getInt(RES_ID)) {
            case RES_ID_MENU_SELECTION:
            case RES_ID_INPUT:
            case RES_ID_CONFIRM:
            case RES_ID_CHOICE:
            case RES_ID_BACKWARD:
            case RES_ID_END_SESSION:
                mStkContext[slotId].mNoResponseFromUser = false;
                break;
            case RES_ID_TIMEOUT:
                cancelNotificationOnKeyguard(slotId);
                mStkContext[slotId].mNoResponseFromUser = true;
                break;
            default:
                // The other IDs cannot be used to judge if there is no response from user.
                break;
        }

        if (null != mStkContext[slotId].mCurrentCmd &&
                null != mStkContext[slotId].mCurrentCmd.getCmdType()) {
            MtkCatLog.d(LOG_TAG, "handleCmdResponse- cmdName[" +
                    mStkContext[slotId].mCurrentCmd.getCmdType().name() + "]");
        }
        if (mStkService[slotId] != null) {
            mStkService[slotId].onCmdResponse(resMsg);
        }
    }

    /**
     * Returns 0 or FLAG_ACTIVITY_NO_USER_ACTION, 0 means the user initiated the action.
     *
     * @param userAction If the userAction is yes then we always return 0 otherwise
     * mMenuIsVisible is used to determine what to return. If mMenuIsVisible is true
     * then we are the foreground app and we'll return 0 as from our perspective a
     * user action did cause. If it's false than we aren't the foreground app and
     * FLAG_ACTIVITY_NO_USER_ACTION is returned.
     *
     * @return 0 or FLAG_ACTIVITY_NO_USER_ACTION
     */
    private int getFlagActivityNoUserAction(InitiatedByUserAction userAction, int slotId) {
        return ((userAction == InitiatedByUserAction.yes) | mStkContext[slotId].mMenuIsVisible)
                ? 0 : Intent.FLAG_ACTIVITY_NO_USER_ACTION;
    }
    /**
     * This method is used for cleaning up pending instances in stack.
     * No terminal response will be sent for pending instances.
     */
    private void cleanUpInstanceStackBySlot(int slotId) {
        Activity activity = mStkContext[slotId].getPendingActivityInstance();
        Activity dialog = mStkContext[slotId].getPendingDialogInstance();
        MtkCatLog.d(LOG_TAG, "cleanUpInstanceStackBySlot slotId: " + slotId);
        if (activity != null) {
            if (mStkContext[slotId].mCurrentCmd != null) {
                MtkCatLog.d(LOG_TAG, "current cmd type: " +
                        mStkContext[slotId].mCurrentCmd.getCmdType());
                if (mStkContext[slotId].mCurrentCmd.getCmdType().value()
                        == AppInterface.CommandType.GET_INPUT.value()
                        || mStkContext[slotId].mCurrentCmd.getCmdType().value()
                        == AppInterface.CommandType.GET_INKEY.value()) {
                    mStkContext[slotId].mIsInputPending = true;
                } else if (mStkContext[slotId].mCurrentCmd.getCmdType().value()
                        == AppInterface.CommandType.SET_UP_MENU.value()
                        || mStkContext[slotId].mCurrentCmd.getCmdType().value()
                        == AppInterface.CommandType.SELECT_ITEM.value()) {
                    mStkContext[slotId].mIsMenuPending = true;
                }
            }
            MtkCatLog.d(LOG_TAG, "finish pending activity.");
            activity.finish();
            mStkContext[slotId].mActivityInstance = null;
        }
        if (dialog != null) {
            MtkCatLog.d(LOG_TAG, "finish pending dialog.");
            mStkContext[slotId].mIsDialogPending = true;
            dialog.finish();
            mStkContext[slotId].mDialogInstance = null;
        }
    }
    /**
     * This method is used for restoring pending instances from stack.
     */
    private void restoreInstanceFromStackBySlot(int slotId) {
        if (null == mStkContext || null == mStkContext[slotId].mCurrentCmd) {
            MtkCatLog.w(LOG_TAG, "Null mStkContext / mCurrentCmd : " + mStkContext);
            return;
        }
        AppInterface.CommandType cmdType = mStkContext[slotId].mCurrentCmd.getCmdType();

        MtkCatLog.d(LOG_TAG, "restoreInstanceFromStackBySlot cmdType : " + cmdType);
        switch(cmdType) {
            case GET_INPUT:
            case GET_INKEY:
                launchInputActivity(slotId, true);
                //Set mMenuIsVisible to true for showing main menu for
                //following session end command.
                mStkContext[slotId].mMenuIsVisible = true;
            break;
            case DISPLAY_TEXT:
                launchTextDialog(slotId, true);
            break;
            case LAUNCH_BROWSER:
                launchConfirmationDialog(mStkContext[slotId].mCurrentCmd.geTextMessage(),
                    slotId, true);
            break;
            case OPEN_CHANNEL:
                TextMessage txtMsg = mStkContext[slotId].mCurrentCmd.geTextMessage();
                if ((null != txtMsg.text) && (0 != txtMsg.text.length())) {
                    launchOpenChannelDialog(slotId);
                }
            break;
            case SET_UP_CALL:
                launchConfirmationDialog(mStkContext[slotId].mCurrentCmd.getCallSettings().
                        confirmMsg, slotId, true);
            break;
            case SET_UP_MENU:
            case SELECT_ITEM:
                launchMenuActivity(null, slotId, true);
            break;
        default:
            MtkCatLog.d(LOG_TAG, "don't launch any stk activity, reset mIsLauncherAcceptInput");
            StkAppService.mIsLauncherAcceptInput = true;
            break;
        }
    }

    @Override
    public void startActivity(Intent intent) {
        int slotId = intent.getIntExtra(SLOT_ID, SubscriptionManager.INVALID_SIM_SLOT_INDEX);
        // Close the dialog displayed for DISPLAY TEXT command with an immediate response object
        // before new dialog is displayed.
        if (SubscriptionManager.isValidSlotIndex(slotId)) {
            Activity dialog = mStkContext[slotId].getImmediateDialogInstance();
            if (dialog != null) {
                MtkCatLog.d(LOG_TAG, "finish dialog for immediate response.");
                dialog.finish();
            }
        }
        super.startActivity(intent);
    }

    private void launchMenuActivity(Menu menu, int slotId, boolean isRestore) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        String targetActivity = STK_MENU_ACTIVITY_NAME;
        /// M: OP02 customization, to update menu activity name. @{
        if (isOP02) {
            if (slotId == PhoneConstants.SIM_ID_1) {
                targetActivity = STK1_MENU_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_2) {
                targetActivity = STK2_MENU_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_3) {
                targetActivity = STK3_MENU_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_4) {
                targetActivity = STK4_MENU_ACTIVITY_NAME;
            }
        }
        /// @}
        String uriString = STK_MENU_URI + System.currentTimeMillis();
        //Set unique URI to create a new instance of activity for different slotId.
        Uri uriData = Uri.parse(uriString);

        MtkCatLog.d(LOG_TAG, "launchMenuActivity, slotId: " + slotId + " , " +
                uriData.toString() + " , " + mStkContext[slotId].mOpCode + ", "
                + mStkContext[slotId].mMenuState);
        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        int intentFlags = Intent.FLAG_ACTIVITY_NEW_TASK;

        if (menu == null) {
            // We assume this was initiated by the user pressing the tool kit icon
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.yes, slotId);

            //MTK add begin
            if (true == isRestore) {
                intentFlags |= Intent.FLAG_ACTIVITY_SINGLE_TOP;
            }
            //MTK add end

            //If the last pending menu is secondary menu, "STATE" should be "STATE_SECONDARY".
            //Otherwise, it should be "STATE_MAIN".
            if (mStkContext[slotId].mOpCode == OP_LAUNCH_APP &&
                    mStkContext[slotId].mMenuState == StkMenuActivity.STATE_SECONDARY) {
                newIntent.putExtra("STATE", StkMenuActivity.STATE_SECONDARY);
            } else {
                newIntent.putExtra("STATE", StkMenuActivity.STATE_MAIN);
                mStkContext[slotId].mMenuState = StkMenuActivity.STATE_MAIN;
            }
        } else {
            // We don't know and we'll let getFlagActivityNoUserAction decide.
            intentFlags |= getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId);
            newIntent.putExtra("STATE", StkMenuActivity.STATE_SECONDARY);
            mStkContext[slotId].mMenuState = StkMenuActivity.STATE_SECONDARY;
        }
        newIntent.putExtra(SLOT_ID, slotId);
        newIntent.setData(uriData);
        newIntent.setFlags(intentFlags);
        startActivity(newIntent);
    }

    private void launchInputActivity(int slotId, boolean isRestore) {
        Intent newIntent = new Intent(Intent.ACTION_VIEW);
        String targetActivity = STK_INPUT_ACTIVITY_NAME;
        /// M: OP02 customization, to update input activity name. @{
        if (isOP02) {
            if (slotId == PhoneConstants.SIM_ID_1) {
                targetActivity = STK1_INPUT_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_2) {
                targetActivity = STK2_INPUT_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_3) {
                targetActivity = STK3_INPUT_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_4) {
                targetActivity = STK4_INPUT_ACTIVITY_NAME;
            }
        }
        /// @}
        String uriString = STK_INPUT_URI + System.currentTimeMillis();
        //Set unique URI to create a new instance of activity for different slotId.
        Uri uriData = Uri.parse(uriString);
        Input input = mStkContext[slotId].mCurrentCmd.geInput();
        String title = null;
        if (input != null) { title = input.text;}

        MtkCatLog.d(LOG_TAG, "launchInputActivity, slotId: " + slotId);

        if (true == isRestore) {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));
        } else {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));
        }
        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        newIntent.putExtra("INPUT", input);
        newIntent.putExtra(SLOT_ID, slotId);
        newIntent.setData(uriData);

        if (input != null) {
            notifyUserIfNecessary(slotId, input.text);
        }
        startActivity(newIntent);
    }

    private void launchTextDialog(int slotId, boolean isRestore) {
        MtkCatLog.d(LOG_TAG, "launchTextDialog, slotId: " + slotId + " isRestore: " +
            isRestore + " mDelayCheckedCount: " + mStkContext[slotId].mDelayCheckedCount);

        //MTK add begin
        if (mStkContext[slotId].mCurrentCmd.getCmdType().value() ==
                    AppInterface.CommandType.SET_UP_MENU.value()) {
            MtkCatLog.d(LOG_TAG, "maybe delay check, but current command is setupmenu, ignore");
            return;
        }
        TextMessage textMessage = mStkContext[slotId].mCurrentCmd.geTextMessage();

        //If we receive a low priority Display Text and the device is
        // not displaying any STK related activity and the screen is not idle
        // ( that is, device is in an interactive state), then send a screen busy
        // terminal response. Otherwise display the message. The existing
        // displayed message shall be updated with the new display text
        // proactive command (Refer to ETSI TS 102 384 section 27.22.4.1.4.4.2).

        if (canShowTextDialog(textMessage, slotId) == false) {
            if (!textMessage.responseNeeded ||
                    DELAY_TO_CHECK_NUM <= mStkContext[slotId].mDelayCheckedCount) {
                mStkContext[slotId].mDelayCheckedCount = 0;
                sendScreenBusyResponse(slotId);
            } else {
                mStkContext[slotId].mDelayCheckedCount++;
                delayToCheckIdle(slotId);
            }
            return;
        }
        mStkContext[slotId].mDelayCheckedCount = 0;
        //MTK add end

        Intent newIntent = new Intent();
        String targetActivity = STK_DIALOG_ACTIVITY_NAME;
        /// M: OP02 customization, to update dialog activity name. @{
        if (isOP02) {
            if (slotId == PhoneConstants.SIM_ID_1) {
                targetActivity = STK1_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_2) {
                targetActivity = STK2_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_3) {
                targetActivity = STK3_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_4) {
                targetActivity = STK4_DIALOG_ACTIVITY_NAME;
            }
        }
        /// @}
        int action = getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId);
        String uriString = STK_DIALOG_URI + System.currentTimeMillis();
        //Set unique URI to create a new instance of activity for different slotId.
        Uri uriData = Uri.parse(uriString);

        newIntent.setClassName(PACKAGE_NAME, targetActivity);
        if (isRestore) {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));

        } else {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));

        }
        newIntent.setData(uriData);
        newIntent.putExtra("TEXT", textMessage);
        newIntent.putExtra(SLOT_ID, slotId);

        if (textMessage != null) {
            notifyUserIfNecessary(slotId, textMessage.text);
        }
        startActivity(newIntent);
        // For display texts with immediate response, send the terminal response
        // immediately. responseNeeded will be false, if display text command has
        // the immediate response tlv.
        if (!mStkContext[slotId].mCurrentCmd.geTextMessage().responseNeeded) {
            sendResponse(RES_ID_CONFIRM, slotId, true);
        }
    }

    private void notifyUserIfNecessary(int slotId, String message) {
        createAllChannels();

        if (mStkContext[slotId].mNoResponseFromUser) {
            // No response from user was observed in the current session.
            // Do nothing in that case in order to avoid turning on the screen again and again
            // when the card repeatedly sends the same command in its retry procedure.
            return;
        }

        PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);

        if (((KeyguardManager) getSystemService(Context.KEYGUARD_SERVICE)).isKeyguardLocked()) {
            // Display the notification on the keyguard screen
            // if user cannot see the message from the card right now because of it.
            // The notification can be dismissed if user removed the keyguard screen.
            launchNotificationOnKeyguard(slotId, message);
        } else if (!(pm.isInteractive() && isTopOfStack())) {
            // User might be doing something but it is not related to the SIM Toolkit.
            // Play the tone and do vibration in order to attract user's attention.
            // User will see the input screen or the dialog soon in this case.
            NotificationChannel channel = mNotificationManager
                    .getNotificationChannel(STK_NOTIFICATION_CHANNEL_ID);
            if(channel != null) {
                Uri uri = channel.getSound();
                if (uri != null && !Uri.EMPTY.equals(uri)
                        && (NotificationManager.IMPORTANCE_LOW) < channel.getImportance()) {
                    RingtoneManager.getRingtone(getApplicationContext(), uri).play();
                }
            }
            long[] pattern = channel.getVibrationPattern();
            if (pattern != null && channel.shouldVibrate()) {
                ((Vibrator) this.getSystemService(Context.VIBRATOR_SERVICE))
                        .vibrate(pattern, -1);
            }
        }

        // Turn on the screen.
        PowerManager.WakeLock wakelock = pm.newWakeLock(PowerManager.FULL_WAKE_LOCK
                | PowerManager.ACQUIRE_CAUSES_WAKEUP | PowerManager.ON_AFTER_RELEASE, LOG_TAG);
        wakelock.acquire();
        wakelock.release();
    }

    private void launchNotificationOnKeyguard(int slotId, String message) {
        Notification.Builder builder = new Notification.Builder(this, STK_NOTIFICATION_CHANNEL_ID);

        builder.setStyle(new Notification.BigTextStyle(builder).bigText(message));
        builder.setContentText(message);

        Menu menu = getMainMenu(slotId);
        if (menu == null || TextUtils.isEmpty(menu.title)) {
            builder.setContentTitle(getResources().getString(R.string.app_name));
        } else {
            builder.setContentTitle(menu.title);
        }

        builder.setSmallIcon(com.android.internal.R.drawable.stat_notify_sim_toolkit);
        builder.setOngoing(true);
        builder.setOnlyAlertOnce(true);
        builder.setColor(getResources().getColor(
                com.android.internal.R.color.system_notification_accent_color));

        registerUserPresentReceiver();
        mNotificationManager.notify(getNotificationId(NOTIFICATION_ON_KEYGUARD, slotId),
                builder.build());
        mStkContext[slotId].mNotificationOnKeyguard = true;
    }

    private void cancelNotificationOnKeyguard(int slotId) {
        mNotificationManager.cancel(getNotificationId(NOTIFICATION_ON_KEYGUARD, slotId));
        mStkContext[slotId].mNotificationOnKeyguard = false;
        unregisterUserPresentReceiver(slotId);
    }

    private synchronized void registerUserPresentReceiver() {
        if (mUserPresentReceiver == null) {
            mUserPresentReceiver = new BroadcastReceiver() {
                @Override public void onReceive(Context context, Intent intent) {
                    if (Intent.ACTION_USER_PRESENT.equals(intent.getAction())) {
                        for (int slot = 0; slot < mSimCount; slot++) {
                            cancelNotificationOnKeyguard(slot);
                        }
                    }
                }
            };
            registerReceiver(mUserPresentReceiver, new IntentFilter(Intent.ACTION_USER_PRESENT));
        }
    }

    private synchronized void unregisterUserPresentReceiver(int slotId) {
        if (mUserPresentReceiver != null) {
            for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
                if (slot != slotId) {
                    if (mStkContext[slot].mNotificationOnKeyguard) {
                        // The broadcast receiver is still necessary for other SIM card.
                        return;
                    }
                }
            }
            unregisterReceiver(mUserPresentReceiver);
            mUserPresentReceiver = null;
        }
    }

    private int getNotificationId(int notificationType, int slotId) {
        return getNotificationId(slotId) + (notificationType * mSimCount);
    }

    /**
     * Checks whether the dialog exists as the top activity of this task.
     *
     * @return true if the top activity of this task is the dialog.
     */
    public boolean isStkDialogActivated() {
        ActivityManager am = (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        if (am.getAppTasks().size() > 0) {
            ComponentName componentName = am.getAppTasks().get(0).getTaskInfo().topActivity;
            if (componentName != null) {
                String[] split = componentName.getClassName().split(Pattern.quote("."));
                String topActivity = split[split.length - 1];
                CatLog.d(LOG_TAG, "Top activity: " + topActivity);
                if (TextUtils.equals(topActivity, StkDialogActivity.class.getSimpleName())) {
                    return true;
                }
            }
        }
        return false;
    }

    private void replaceEventList(int slotId) {
        if (mStkContext[slotId].mSetupEventListSettings != null) {
            for (int current : mStkContext[slotId].mSetupEventListSettings.eventList) {
                if (current != INVALID_SETUP_EVENT) {
                    // Cancel the event notification if it is not listed in the new event list.
                    if ((mStkContext[slotId].mCurrentCmd.getSetEventList() == null)
                            || !findEvent(current, mStkContext[slotId].mCurrentCmd
                            .getSetEventList().eventList)) {
                        unregisterEvent(current, slotId);
                    }
                }
            }
        }
        mStkContext[slotId].mSetupEventListSettings
                = mStkContext[slotId].mCurrentCmd.getSetEventList();
        mStkContext[slotId].mCurrentSetupEventCmd = mStkContext[slotId].mCurrentCmd;
        mStkContext[slotId].mCurrentCmd = mStkContext[slotId].mMainCmd;
        registerEvents(slotId);
    }

    private boolean findEvent(int event, int[] eventList) {
        for (int content : eventList) {
            if (content == event) return true;
        }
        return false;
    }

    private void unregisterEvent(int event, int slotId) {
        for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
            if (slot != slotId) {
                if (mStkContext[slot].mSetupEventListSettings != null) {
                    if (findEvent(event, mStkContext[slot].mSetupEventListSettings.eventList)) {
                        // The specified event shall never be canceled
                        // if there is any other SIM card which requests the event.
                        return;
                    }
                }
            }
        }

        switch (event) {
            case USER_ACTIVITY_EVENT:
                unregisterUserActivityReceiver();
                break;
            case IDLE_SCREEN_AVAILABLE_EVENT:
                unregisterProcessObserver(AppInterface.CommandType.SET_UP_EVENT_LIST, slotId);
                break;
            case LANGUAGE_SELECTION_EVENT:
                unregisterLocaleChangeReceiver();
                break;
            default:
                break;
        }
    }

    private void registerEvents(int slotId) {
        if (mStkContext[slotId].mSetupEventListSettings == null) {
            return;
        }
        for (int event : mStkContext[slotId].mSetupEventListSettings.eventList) {
            switch (event) {
                case USER_ACTIVITY_EVENT:
                    registerUserActivityReceiver();
                    break;
                case IDLE_SCREEN_AVAILABLE_EVENT:
                    registerProcessObserver();
                    break;
                case LANGUAGE_SELECTION_EVENT:
                    registerLocaleChangeReceiver();
                    break;
                default:
                    break;
            }
        }
    }

    private synchronized void registerUserActivityReceiver() {
        if (mUserActivityReceiver == null) {
            mUserActivityReceiver = new BroadcastReceiver() {
                @Override public void onReceive(Context context, Intent intent) {
                    if (WindowManagerPolicyConstants.ACTION_USER_ACTIVITY_NOTIFICATION.equals(
                            intent.getAction())) {
                        Message message = mServiceHandler.obtainMessage();
                        message.arg1 = OP_USER_ACTIVITY;
                        mServiceHandler.sendMessage(message);
                        unregisterUserActivityReceiver();
                    }
                }
            };
            registerReceiver(mUserActivityReceiver, new IntentFilter(
                    WindowManagerPolicyConstants.ACTION_USER_ACTIVITY_NOTIFICATION));
            try {
                IWindowManager wm = IWindowManager.Stub.asInterface(
                        ServiceManager.getService(Context.WINDOW_SERVICE));
                wm.requestUserActivityNotification();
            } catch (RemoteException e) {
                MtkCatLog.e(this, "failed to init WindowManager:" + e);
            }
        }
    }

    private synchronized void unregisterUserActivityReceiver() {
        if (mUserActivityReceiver != null) {
            unregisterReceiver(mUserActivityReceiver);
            mUserActivityReceiver = null;
        }
    }

    private synchronized void registerProcessObserver() {
        if (mProcessObserver == null) {
            try {
                IProcessObserver.Stub observer = new IProcessObserver.Stub() {
                    @Override
                    public void onForegroundActivitiesChanged(int pid, int uid, boolean fg) {
                        if (isScreenIdle()) {
                            Message message = mServiceHandler.obtainMessage();
                            message.arg1 = OP_IDLE_SCREEN;
                            mServiceHandler.sendMessage(message);
                            unregisterProcessObserver();
                        }
                    }

                    @Override
                    public void onForegroundServicesChanged(int pid, int uid, int fgServicetypes) {
                    }

                    @Override
                    public void onProcessDied(int pid, int uid) {
                    }
                };
                ActivityManagerNative.getDefault().registerProcessObserver(observer);
                MtkCatLog.d(this, "Started to observe the foreground activity");
                mProcessObserver = observer;
            } catch (RemoteException e) {
                MtkCatLog.d(this, "Failed to register the process observer");
            }
        }
    }

    private void unregisterProcessObserver(AppInterface.CommandType command, int slotId) {
        // Check if there is any pending command which still needs the process observer
        // except for the current command and slot.
        for (int slot = PhoneConstants.SIM_ID_1; slot < mSimCount; slot++) {
            if (command != AppInterface.CommandType.SET_UP_IDLE_MODE_TEXT || slot != slotId) {
                if (mStkContext[slot].mIdleModeTextCmd != null
                        && !mStkContext[slot].mIdleModeTextVisible) {
                    // Keep the process observer registered
                    // as there is an idle mode text which has not been visible yet.
                    return;
                }
            }
            if (command != AppInterface.CommandType.SET_UP_EVENT_LIST || slot != slotId) {
                if (mStkContext[slot].mSetupEventListSettings != null) {
                    if (findEvent(IDLE_SCREEN_AVAILABLE_EVENT,
                                mStkContext[slot].mSetupEventListSettings.eventList)) {
                        // Keep the process observer registered
                        // as there is a SIM card which still want IDLE SCREEN AVAILABLE event.
                        return;
                    }
                }
            }
        }
        unregisterProcessObserver();
    }

    private synchronized void unregisterProcessObserver() {
        if (mProcessObserver != null) {
            try {
                ActivityManagerNative.getDefault().unregisterProcessObserver(mProcessObserver);
                MtkCatLog.d(this, "Stopped to observe the foreground activity");
                mProcessObserver = null;
            } catch (RemoteException e) {
                MtkCatLog.d(this, "Failed to unregister the process observer");
            }
        }
    }

    private synchronized void registerLocaleChangeReceiver() {
        if (mLocaleChangeReceiver == null) {
            mLocaleChangeReceiver = new BroadcastReceiver() {
                @Override public void onReceive(Context context, Intent intent) {
                    if (Intent.ACTION_LOCALE_CHANGED.equals(intent.getAction())) {
                        Message message = mServiceHandler.obtainMessage();
                        message.arg1 = OP_LOCALE_CHANGED;
                        mServiceHandler.sendMessage(message);
                    }
                }
            };
            registerReceiver(mLocaleChangeReceiver, new IntentFilter(Intent.ACTION_LOCALE_CHANGED));
        }
    }

    private synchronized void unregisterLocaleChangeReceiver() {
        if (mLocaleChangeReceiver != null) {
            unregisterReceiver(mLocaleChangeReceiver);
            mLocaleChangeReceiver = null;
        }
    }

    private void sendSetUpEventResponse(int event, byte[] addedInfo,
            int srcId, int desId, boolean oneShot, int slotId) {
        MtkCatLog.d(this, "sendSetUpEventResponse: event : " + event + "slotId = " + slotId);

        if (mStkContext[slotId].mCurrentSetupEventCmd == null){
            MtkCatLog.e(this, "mCurrentSetupEventCmd is null");
            return;
        }

        MtkCatResponseMessage resMsg =
                new MtkCatResponseMessage(mStkContext[slotId].mCurrentSetupEventCmd);

        resMsg.setResultCode(ResultCode.OK);

        //MTK add begin
        resMsg.setSourceId(srcId);
        resMsg.setDestinationId(desId);
        resMsg.setAdditionalInfo(addedInfo);
        resMsg.setOneShot(oneShot);
        //MTK add end
        resMsg.setEventDownload(event, addedInfo);
        if (mStkService[slotId] != null) {
            mStkService[slotId].onCmdResponse(resMsg);
        } else {
            MtkCatLog.e(LOG_TAG, "mStkService is null!!");
        }
    }

    private void checkForSetupEvent(int event, Bundle args, int slotId) {
        boolean eventPresent = false;
        byte[] addedInfo = null;

        //MTK add
        byte[] additionalInfo = null;
        boolean oneShot = false;
        int sourceId = 0;
        int destinationId = 0;

        MtkCatLog.d(this, "Event :" + event);

        if (mStkContext[slotId].mSetupEventListSettings != null) {
            /* Checks if the event is present in the EventList updated by last
             * SetupEventList Proactive Command */
            for (int i : mStkContext[slotId].mSetupEventListSettings.eventList) {
                 if (event == i) {
                     eventPresent =  true;
                     break;
                 }
            }

            /* If Event is present send the response to ICC */
            if (eventPresent == true) {
                MtkCatLog.d(this, " Event " + event + "exists in the EventList");

                switch (event) {
                    case USER_ACTIVITY_EVENT:
                        sourceId = DEV_ID_TERMINAL;
                        destinationId = DEV_ID_UICC;
                        oneShot = true;
                        sendSetUpEventResponse(event, additionalInfo,
                                sourceId, destinationId, oneShot, slotId);
                        removeSetUpEvent(event, slotId);
                        break;
                    case IDLE_SCREEN_AVAILABLE_EVENT:
                        sourceId = DEV_ID_DISPLAY;
                        destinationId = DEV_ID_UICC;
                        oneShot = true;
                        sendSetUpEventResponse(event, additionalInfo,
                                sourceId, destinationId, oneShot,slotId);
                        removeSetUpEvent(event, slotId);
                        break;
                    case LANGUAGE_SELECTION_EVENT:
                        //locale changed, rename all registered
                        //notification channels on locale change
                        createAllChannels();

                        sourceId = DEV_ID_TERMINAL;
                        destinationId = DEV_ID_UICC;
                        String languageInfo = Locale.getDefault().getLanguage();
                        byte[] language = languageInfo.getBytes();
                        sendSetUpEventResponse(event, language,
                                sourceId, destinationId, oneShot, slotId);
                        break;

                    case BROWSER_TERMINATION_EVENT:
                        sourceId = DEV_ID_TERMINAL;
                        destinationId = DEV_ID_UICC;
                        //browser termination cause tag
                        additionalInfo = new byte[3];
                        additionalInfo[0] = (byte) 0xB4;
                        additionalInfo[1] = 0x01;
                        additionalInfo[2] = 0x00;
                        sendSetUpEventResponse(event, additionalInfo,
                                sourceId, destinationId, oneShot, slotId);
                        break;

                    case BROWSING_STATUS_EVENT:
                        sourceId = DEV_ID_TERMINAL;
                        destinationId = DEV_ID_UICC;
                        //browsing status tag
                        additionalInfo = new byte[4];
                        additionalInfo[0] = (byte) 0xE4;
                        additionalInfo[1] = 0x02;
                        //0x0194 = 404, Not found.
                        additionalInfo[2] = (byte) 0x01;
                        additionalInfo[3] = (byte) 0x94;
                        sendSetUpEventResponse(event, additionalInfo,
                                sourceId, destinationId, oneShot, slotId);
                        break;

                    default:
                        break;
                }
            } else {
                MtkCatLog.e(this, " Event does not exist in the EventList");
            }
        } else {
            MtkCatLog.e(this, "SetupEventList is not received. Ignoring the event: " + event);
        }
    }

    private void removeSetUpEvent(int event, int slotId) {
        MtkCatLog.d(this, "Remove Event :" + event);

        if (mStkContext[slotId].mSetupEventListSettings != null) {
            /*
             * Make new  Eventlist without the event
             */
            for (int i = 0; i < mStkContext[slotId].mSetupEventListSettings.eventList.length; i++) {
                if (event == mStkContext[slotId].mSetupEventListSettings.eventList[i]) {
                    mStkContext[slotId].mSetupEventListSettings.eventList[i] = INVALID_SETUP_EVENT;

                    switch (event) {
                        case USER_ACTIVITY_EVENT:
                            // The broadcast receiver can be unregistered
                            // as the event has already been sent to the card.
                            unregisterUserActivityReceiver();
                            break;
                        case IDLE_SCREEN_AVAILABLE_EVENT:
                            // The process observer can be unregistered
                            // as the idle screen has already been available.
                            unregisterProcessObserver();
                            break;
                        default:
                            break;
                    }
                    break;
                }
            }
        }
    }

    private void launchEventMessage(int slotId) {
        launchEventMessage(slotId, mStkContext[slotId].mCurrentCmd.geTextMessage());
    }

    private void launchEventMessage(int slotId, TextMessage msg) {
        if (msg == null || msg.text == null || (msg.text != null && msg.text.length() == 0)) {
            MtkCatLog.d(LOG_TAG, "launchEventMessage return");
            return;
        }

        Toast toast = new Toast(mContext.getApplicationContext());
        LayoutInflater inflate = (LayoutInflater) mContext
                .getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View v = inflate.inflate(R.layout.stk_event_msg, null);
        TextView tv = (TextView) v
                .findViewById(com.android.internal.R.id.message);
        ImageView iv = (ImageView) v
                .findViewById(com.android.internal.R.id.icon);
        if (msg.icon != null) {
            iv.setImageBitmap(msg.icon);
        } else {
            iv.setVisibility(View.GONE);
        }
        /* In case of 'self explanatory' stkapp should display the specified
         * icon in proactive command (but not the alpha string).
         * If icon is non-self explanatory and if the icon could not be displayed
         * then alpha string or text data should be displayed
         * Ref: ETSI 102.223,section 6.5.4
         */
        if (mStkContext[slotId].mCurrentCmd.hasIconLoadFailed() ||
                msg.icon == null || !msg.iconSelfExplanatory) {
            tv.setText(msg.text);
        }

        toast.setView(v);
        toast.setDuration(Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    private void launchConfirmationDialog(TextMessage msg, int slotId, boolean isRestore) {
        msg.title = mStkContext[slotId].lastSelectedItem;
        Intent newIntent = new Intent();
        String targetActivity = STK_DIALOG_ACTIVITY_NAME;
        /// M: OP02 customization, to update dialog activity name. @{
        if (isOP02) {
            if (slotId == PhoneConstants.SIM_ID_1) {
                targetActivity = STK1_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_2) {
                targetActivity = STK2_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_3) {
                targetActivity = STK3_DIALOG_ACTIVITY_NAME;
            } else if (slotId == PhoneConstants.SIM_ID_4) {
                targetActivity = STK4_DIALOG_ACTIVITY_NAME;
            }
        }
        /// @}
        String uriString = STK_DIALOG_URI + System.currentTimeMillis();
        //Set unique URI to create a new instance of activity for different slotId.
        Uri uriData = Uri.parse(uriString);

        newIntent.setClassName(this, targetActivity);
        if (true == isRestore) {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_NO_HISTORY
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));

        } else {
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_NO_HISTORY
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));
        }
        newIntent.putExtra("TEXT", msg);
        newIntent.putExtra(SLOT_ID, slotId);
        newIntent.setData(uriData);
        startActivity(newIntent);
    }

    private void launchBrowser(BrowserSettings settings) {
        if (settings == null) {
            return;
        }

        Uri data = null;
        String url;
        if (settings.url == null) {
            // if the command did not contain a URL,
            // launch the browser to the default homepage.
            MtkCatLog.d(this, "no url data provided by proactive command." +
                       " launching browser with stk default URL ... ");
            url = SystemProperties.get(STK_BROWSER_DEFAULT_URL_SYSPROP,
                    "http://www.google.com");
            mLaunchBrowserUrlType = DEFAULT_URL;
        } else {
            MtkCatLog.d(this, "launch browser command has attached url = " + settings.url);
            url = settings.url;
            mLaunchBrowserUrlType = DEDICATED_URL;
        }

        if (url.startsWith("http://") || url.startsWith("https://")) {
            data = Uri.parse(url);
            MtkCatLog.d(this, "launching browser with url = " + url);
        } else {
            String modifiedUrl = "http://" + url;
            data = Uri.parse(modifiedUrl);
            MtkCatLog.d(this, "launching browser with modified url = " + modifiedUrl);
        }

        if (DEDICATED_URL == mLaunchBrowserUrlType) {
            mLaunchBrowserUrl = data.getAuthority();
            MtkCatLog.v(LOG_TAG, "authority of Uri: " + mLaunchBrowserUrl);
        } else {
            mLaunchBrowserUrl = null;
        }


        Intent intent = new Intent(Intent.ACTION_VIEW);
        intent.setData(data);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        switch (settings.mode) {
        case USE_EXISTING_BROWSER:
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            break;
        case LAUNCH_NEW_BROWSER:
            intent.addFlags(Intent.FLAG_ACTIVITY_MULTIPLE_TASK);
            break;
        case LAUNCH_IF_NOT_ALREADY_LAUNCHED:
            intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            break;
        }
        // start browser activity
        try {
            startActivity(intent);
        } catch (ActivityNotFoundException e) {
            MtkCatLog.e(LOG_TAG, "Browser activity is not found.");
        }
        // a small delay, let the browser start, before processing the next command.
        // this is good for scenarios where a related DISPLAY TEXT command is
        // followed immediately.
        try {
            Thread.sleep(3000);
        } catch (InterruptedException e) {}
    }

    private void cancelIdleText(int slotId) {
        unregisterProcessObserver(AppInterface.CommandType.SET_UP_IDLE_MODE_TEXT, slotId);
        mNotificationManager.cancel(getNotificationId(slotId));
        mStkContext[slotId].mIdleModeTextCmd = null;
        mStkContext[slotId].mIdleModeTextVisible = false;
    }

    private void launchIdleText(int slotId) {
        TextMessage msg = mStkContext[slotId].mIdleModeTextCmd.geTextMessage();

        if (msg == null || msg.text == null || TextUtils.isEmpty(msg.text)) {
            MtkCatLog.d(LOG_TAG,  msg == null ? "mCurrent.getTextMessage is NULL"
                    : "mCurrent.getTextMessage.text invailid");
            mNotificationManager.cancel(getNotificationId(slotId));
        } else {
            MtkCatLog.d(LOG_TAG, "launchIdleText - text["
                    + (Build.IS_DEBUGGABLE ? msg.text : "********")
                    + "] iconSelfExplanatory[" + msg.iconSelfExplanatory
                    + "] icon[" + msg.icon + "], sim id: " + slotId);
            MtkCatLog.d(LOG_TAG, "Add IdleMode text");
            PendingIntent pendingIntent = PendingIntent.getService(mContext, 0,
                    new Intent(mContext, StkAppService.class), 0);
            createAllChannels();
            final Notification.Builder notificationBuilder = new Notification.Builder(
                    StkAppService.this, STK_NOTIFICATION_CHANNEL_ID);
            if (mStkContext[slotId].mMainCmd != null &&
                    mStkContext[slotId].mMainCmd.getMenu() != null) {
                notificationBuilder.setContentTitle(mStkContext[slotId].mMainCmd.getMenu().title);
            } else {
                notificationBuilder.setContentTitle("");
            }
            notificationBuilder
                    .setSmallIcon(com.android.internal.R.drawable.stat_notify_sim_toolkit);
            notificationBuilder.setContentIntent(pendingIntent);
            notificationBuilder.setOngoing(true);
            notificationBuilder.setOnlyAlertOnce(true);
            // Set text and icon for the status bar and notification body.
            if (mStkContext[slotId].mIdleModeTextCmd.hasIconLoadFailed() ||
                    !msg.iconSelfExplanatory) {
                notificationBuilder.setContentText(msg.text);
                notificationBuilder.setTicker(msg.text);
            }
            if (msg.icon != null) {
                notificationBuilder.setLargeIcon(msg.icon);
            } else {
                Bitmap bitmapIcon = BitmapFactory.decodeResource(StkAppService.this
                    .getResources().getSystem(),
                    com.android.internal.R.drawable.stat_notify_sim_toolkit);
                notificationBuilder.setLargeIcon(bitmapIcon);
            }
            notificationBuilder.setColor(mContext.getResources().getColor(
                    com.android.internal.R.color.system_notification_accent_color));
            mNotificationManager.notify(getNotificationId(slotId), notificationBuilder.build());
            mStkContext[slotId].mIdleModeTextVisible = true;
        }
    }

    /** Creates the notification channel and registers it with NotificationManager.
     * If a channel with the same ID is already registered, NotificationManager will
     * ignore this call.
     */
    private void createAllChannels() {
        NotificationChannel notificationChannel = new NotificationChannel(
                STK_NOTIFICATION_CHANNEL_ID,
                getResources().getString(R.string.stk_channel_name),
                NotificationManager.IMPORTANCE_DEFAULT);

        notificationChannel.enableVibration(true);
        notificationChannel.setVibrationPattern(VIBRATION_PATTERN);

        mNotificationManager.createNotificationChannel(notificationChannel);
    }

    private void launchToneDialog(int slotId) {
        Intent newIntent = new Intent(this, ToneDialog.class);
        String uriString = STK_TONE_URI + slotId;
        Uri uriData = Uri.parse(uriString);
        //Set unique URI to create a new instance of activity for different slotId.
        MtkCatLog.d(LOG_TAG, "launchToneDialog, slotId: " + slotId);
        newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_NO_HISTORY
                | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));
        newIntent.putExtra("TEXT", mStkContext[slotId].mCurrentCmd.geTextMessage());
        newIntent.putExtra("TONE", mStkContext[slotId].mCurrentCmd.getToneSettings());
        newIntent.putExtra(SLOT_ID, slotId);
        newIntent.setData(uriData);
        startActivity(newIntent);
    }

    private void handlePlayTone(int slotId) {
        TextMessage toneMsg = mStkContext[slotId].mCurrentCmd.geTextMessage();

        boolean showUser = true;
        boolean displayDialog = true;
        Resources resource = Resources.getSystem();
        try {
            displayDialog = !resource.getBoolean(
                    com.android.internal.R.bool.config_stkNoAlphaUsrCnf);
        } catch (NotFoundException e) {
            displayDialog = true;
        }

        // As per the spec 3GPP TS 11.14, 6.4.5. Play Tone.
        // If there is no alpha identifier tlv present, UE may show the
        // user information. 'config_stkNoAlphaUsrCnf' value will decide
        // whether to show it or not.
        // If alpha identifier tlv is present and its data is null, play only tone
        // without showing user any information.
        // Alpha Id is Present, but the text data is null.
        if ((toneMsg.text != null ) && (toneMsg.text.equals(""))) {
            MtkCatLog.d(this, "Alpha identifier data is null, play only tone");
            showUser = false;
        }
        // Alpha Id is not present AND we need to show info to the user.
        if (toneMsg.text == null && displayDialog) {
            MtkCatLog.d(this, "toneMsg.text " + toneMsg.text
                    + " Starting ToneDialog activity with default message.");
            toneMsg.text = getResources().getString(R.string.default_tone_dialog_msg);
            showUser = true;
        }
        // Dont show user info, if config setting is true.
        if (toneMsg.text == null && !displayDialog) {
            MtkCatLog.d(this, "config value stkNoAlphaUsrCnf is true");
            showUser = false;
        }

        MtkCatLog.d(this, "toneMsg.text: " + toneMsg.text + "showUser: " +showUser +
                "displayDialog: " +displayDialog);
        playTone(showUser, slotId);
    }

    private void playTone(boolean showUserInfo, int slotId) {
        // Start playing tone and vibration
        ToneSettings settings = mStkContext[slotId].mCurrentCmd.getToneSettings();
        if (null == settings) {
            MtkCatLog.d(this, "null settings, not playing tone.");
            return;
        }

        mVibrator = (Vibrator)getSystemService(VIBRATOR_SERVICE);
        mTonePlayer = new TonePlayer();
        mTonePlayer.play(settings.tone);
        int timeout = StkApp.calculateDurationInMilis(settings.duration);
        if (timeout == 0) {
            timeout = StkApp.TONE_DEFAULT_TIMEOUT;
        }

        Message msg = mServiceHandler.obtainMessage();
        msg.arg1 = OP_STOP_TONE;
        msg.arg2 = slotId;
        msg.obj = (Integer)(showUserInfo ? 1 : 0);
        msg.what = STOP_TONE_WHAT;
        mServiceHandler.sendMessageDelayed(msg, timeout);
        if (settings.vibrate) {
            mVibrator.vibrate(timeout);
        }

        // Start Tone dialog Activity to show user the information.
        if (showUserInfo) {
            Intent newIntent = new Intent(sInstance, ToneDialog.class);
            String uriString = STK_TONE_URI + slotId;
            Uri uriData = Uri.parse(uriString);
            newIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK
                    | Intent.FLAG_ACTIVITY_NO_HISTORY
                    | Intent.FLAG_ACTIVITY_SINGLE_TOP
                    | Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS
                    | getFlagActivityNoUserAction(InitiatedByUserAction.unknown, slotId));
            newIntent.putExtra("TEXT", mStkContext[slotId].mCurrentCmd.geTextMessage());
            newIntent.putExtra(SLOT_ID, slotId);
            newIntent.setData(uriData);
            startActivity(newIntent);
        }
    }

    private void finishToneDialogActivity() {
        Intent finishIntent = new Intent(FINISH_TONE_ACTIVITY_ACTION);
        sendBroadcast(finishIntent);
    }

    private void handleStopTone(Message msg, int slotId) {
        int resId = 0;

        // Stop the play tone in following cases:
        // 1.OP_STOP_TONE: play tone timer expires.
        // 2.STOP_TONE_USER: user pressed the back key.
        if (msg.arg1 == OP_STOP_TONE) {
            resId = RES_ID_DONE;
            // Dismiss Tone dialog, after finishing off playing the tone.
            int finishActivity = (Integer) msg.obj;
            if (finishActivity == 1) finishToneDialogActivity();
        } else if (msg.arg1 == OP_STOP_TONE_USER) {
            resId = RES_ID_END_SESSION;
        }

        sendResponse(resId, slotId, true);
        mServiceHandler.removeMessages(STOP_TONE_WHAT);
        if (mTonePlayer != null)  {
            mTonePlayer.stop();
            mTonePlayer.release();
            mTonePlayer = null;
        }
        if (mVibrator != null) {
            mVibrator.cancel();
            mVibrator = null;
        }
    }

    private void launchOpenChannelDialog(final int slotId) {
        MtkCatLog.d(LOG_TAG, "launchOpenChannelDialog");

        TextMessage msg = mStkContext[slotId].mCurrentCmd.geTextMessage();
        if (msg == null) {
            MtkCatLog.d(LOG_TAG, "msg is null, return here");
            return;
        }

        if (msg.title == null || msg.title.length() == 0) {
            msg.title = getResources().getString(R.string.stk_dialog_title);
        }

        //should not null here, because checking already
        if (msg.text == null) {
            msg.text = getResources().getString(R.string.default_open_channel_msg);
        }

        MtkCatLog.d(LOG_TAG, "title: " + (Build.IS_DEBUGGABLE ? msg.title : "********")
                + " text: " + (Build.IS_DEBUGGABLE ? msg.text : "********"));

        final AlertDialog dialog = new AlertDialog.Builder(mContext)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(msg.title)
                    .setMessage(msg.text)
                    .setCancelable(false)
                    .setPositiveButton(getResources().getString(R.string.stk_dialog_accept),
                                       new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Bundle args = new Bundle();
                            args.putInt(RES_ID, RES_ID_CHOICE);
                            args.putInt(CHOICE, YES);
                            Message message = mServiceHandler.obtainMessage();
                            message.arg1 = OP_RESPONSE;
                            message.arg2 = slotId;
                            message.obj = args;
                            mServiceHandler.sendMessage(message);
                        }
                    })
                    .setNegativeButton(getResources().getString(R.string.stk_dialog_reject),
                                       new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            Bundle args = new Bundle();
                            args.putInt(RES_ID, RES_ID_CHOICE);
                            args.putInt(CHOICE, NO);
                            Message message = mServiceHandler.obtainMessage();
                            message.arg1 = OP_RESPONSE;
                            message.arg2 = slotId;
                            message.obj = args;
                            mServiceHandler.sendMessage(message);
                        }
                    })
                    .create();

        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        if (!mContext.getResources().getBoolean(
                com.android.internal.R.bool.config_sf_slowBlur)) {
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        }

        dialog.show();
    }

    private void launchTransientEventMessage(int slotId) {
        TextMessage msg = mStkContext[slotId].mCurrentCmd.geTextMessage();
        if (msg == null) {
            MtkCatLog.d(LOG_TAG, "msg is null, return here");
            return;
        }

        msg.title = getResources().getString(R.string.stk_dialog_title);

        final AlertDialog dialog = new AlertDialog.Builder(mContext)
                    .setIconAttribute(android.R.attr.alertDialogIcon)
                    .setTitle(msg.title)
                    .setMessage(msg.text)
                    .setCancelable(false)
                    .setPositiveButton(getResources().getString(android.R.string.ok),
                                       new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                        }
                    })
                    .create();

        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        if (!mContext.getResources().getBoolean(
                com.android.internal.R.bool.config_sf_slowBlur)) {
            dialog.getWindow().addFlags(WindowManager.LayoutParams.FLAG_BLUR_BEHIND);
        }

        dialog.show();
    }

    private int getNotificationId(int slotId) {
        int notifyId = STK_NOTIFICATION_ID;
        if (slotId >= 0 && slotId < mSimCount) {
            notifyId += slotId;
        } else {
            MtkCatLog.d(LOG_TAG, "invalid slotId: " + slotId);
        }
        MtkCatLog.d(LOG_TAG, "getNotificationId, slotId: " + slotId + ", notifyId: " + notifyId);
        return notifyId;
    }

    private String getItemName(int itemId, int slotId) {
        Menu menu = mStkContext[slotId].mCurrentCmd.getMenu();
        if (menu == null) {
            return null;
        }
        for (Item item : menu.items) {
            if (item.id == itemId) {
                return item.text;
            }
        }
        return null;
    }

    private boolean removeMenu(int slotId) {
        try {
            if (mStkContext[slotId].mCurrentMenu.items.size() == 1 &&
                mStkContext[slotId].mCurrentMenu.items.get(0) == null) {
                mStkContext[slotId].mSetupMenuState = STATE_NOT_EXIST;
                return true;
            }
        } catch (NullPointerException e) {
            MtkCatLog.d(LOG_TAG, "Unable to get Menu's items size");
            mStkContext[slotId].mSetupMenuState = STATE_NOT_EXIST;
            return true;
        }
        mStkContext[slotId].mSetupMenuState = STATE_EXIST;
        return false;
    }

    StkContext getStkContext(int slotId) {
        if (slotId >= 0 && slotId < mSimCount) {
            return mStkContext[slotId];
        } else {
            MtkCatLog.d(LOG_TAG, "invalid slotId: " + slotId);
            return null;
        }
    }

    private void handleAlphaNotify(Bundle args) {
        String alphaString = args.getString(AppInterface.ALPHA_STRING);

        MtkCatLog.d(this, "Alpha string received from card: " +
                (Build.IS_DEBUGGABLE ? alphaString : "********"));
        Toast toast = Toast.makeText(sInstance, alphaString, Toast.LENGTH_LONG);
        toast.setGravity(Gravity.TOP, 0, 0);
        toast.show();
    }

    private boolean isUrlAvailableToLaunchBrowser(BrowserSettings settings) {
        String url = SystemProperties.get(STK_BROWSER_DEFAULT_URL_SYSPROP, "");
        if (url == "" && settings.url == null) {
            return false;
        }
        return true;
    }


    /// M:  @{



    //BIP releated begin
    private void processOpenChannel(int slotId) {
        MtkCatLog.d(LOG_TAG, "processOpenChannel()+ " + slotId);

        PhoneConstants.State callState = PhoneConstants.State.IDLE;
        TextMessage txtMsg = mStkContext[slotId].mCurrentCmd.geTextMessage();
        int subId[] = SubscriptionManager.getSubId(slotId);
        int phoneId = SubscriptionManager.getPhoneId(subId[0]);
        Phone myPhone = null;
        try {
            myPhone = PhoneFactory.getPhone(phoneId);
        } catch (IllegalStateException e) {
            MtkCatLog.e(LOG_TAG, "processOpenChannel(): IllegalStateException, get phone fail.");
            e.printStackTrace();
            myPhone = null;
        }

        if (myPhone == null) {
            MtkCatLog.w("CatService", "myPhone is null");
        } else {
        //if (myPhone.getServiceState().getNetworkType() <= TelephonyManager.NETWORK_TYPE_EDGE) {
            if (false) {
                callState = getPhoneCallState();
            }
        }

        if (callState == PhoneConstants.State.IDLE) {
            if ((null != txtMsg.text) && (0 != txtMsg.text.length())) {
                /* Alpha identifier with data object */
                //launchConfirmationDialog(txtMsg, slotId, false);
                //change to AOSP
                launchOpenChannelDialog(slotId);
            } else {
                 /* Alpha identifier with null data object Chap 6.4.27.1 ME
                    should not give any information to the user or ask for user confirmation */
                processNormalOpenChannelResponse(slotId);
            }
        } else {
            processBusyOnCallOpenChannelResponse(slotId);
        }
        MtkCatLog.v(LOG_TAG, "processOpenChannel()-");
    }

    private void processNormalOpenChannelResponse(int slotId) {
        MtkCatLog.d(LOG_TAG, "Normal OpenChannel Response PROCESS Start");

        mStkContext[slotId].mCmdInProgress = false;
        if (mStkContext[slotId].mCurrentCmd == null) {
            MtkCatLog.w(LOG_TAG, "Normal OpenChannel Response PROCESS mCurrentCmd changed to null");
            return;
        }

        TextMessage txtMsg = mStkContext[slotId].mCurrentCmd.geTextMessage();
        if (mStkContext[slotId].mCurrentCmd.getCmdType() != null) {
            MtkCatLog.v(LOG_TAG, "Normal OpenChannel Response PROCESS end! cmdName["
                    + mStkContext[slotId].mCurrentCmd.getCmdType().name() + "]");
        }
        MtkCatResponseMessage resMsg = new MtkCatResponseMessage(mStkContext[slotId].mCurrentCmd);
        resMsg.setResultCode(ResultCode.OK);
        resMsg.setConfirmation(true);
        if (!isOP09) {
            displayAlphaIcon(txtMsg, slotId);
        }
        if (mStkService != null && mStkService[slotId] != null) {
            mStkService[slotId].onCmdResponse(resMsg);
        } else {
            MtkCatLog.e(LOG_TAG, "mStkService is null!!");
        }
        mStkContext[slotId].mCurrentCmd = mStkContext[slotId].mMainCmd;
    }

    private void processBusyOnCallOpenChannelResponse(int slotId) {
        MtkCatLog.d(LOG_TAG, "Busy on Call OpenChannel Response PROCESS cmdName["
                    + mStkContext[slotId].mCurrentCmd.getCmdType().name() + "]");
        mStkContext[slotId].mCmdInProgress = false;
        MtkCatResponseMessage resMsg = new MtkCatResponseMessage(mStkContext[slotId].mCurrentCmd);
        resMsg.setResultCode(ResultCode.TERMINAL_CRNTLY_UNABLE_TO_PROCESS);
        byte[] additionalInfo = new byte[1];
        additionalInfo[0] = (byte) 0x02; //busy on call
        resMsg.setAdditionalInfo(additionalInfo);
        if (mStkService != null && mStkService[slotId] != null) {
            mStkService[slotId].onCmdResponse(resMsg);
        } else {
            MtkCatLog.e(LOG_TAG, "mStkService is null!!");
        }
    }

    private void displayAlphaIcon(TextMessage msg, int slotId) {
        if (msg == null) {
            MtkCatLog.w(LOG_TAG, "[displayAlphaIcon] msg is null");
            return;
        }
        MtkCatLog.v(LOG_TAG, "launchAlphaIcon - IconSelfExplanatory[" +
                    msg.iconSelfExplanatory + "]" +
                    "icon[" + msg.icon + "]" +
                    "text[" + (Build.IS_DEBUGGABLE ? msg.text : "********") + "]");
        TextMessage dispTxt = msg;

        correctTextMessage(dispTxt, slotId);
        if (msg.iconSelfExplanatory == true) {
            // only display Icon.
            if (msg.icon != null) {
                showIconToast(msg);
            } else {
                // do nothing.
                MtkCatLog.w(LOG_TAG, "launchAlphaIcon - null icon!");
                return;
            }
        } else {
            // show text & icon.
            if (msg.icon != null) {
                if (msg.text == null || msg.text.length() == 0) {
                    // show Icon only.
                    showIconToast(msg);
                }
                else {
                    showIconAndTextToast(msg);
                }
            } else {
                if (msg.text == null || msg.text.length() == 0) {
                    // do nothing
                    MtkCatLog.w(LOG_TAG, "launchAlphaIcon - null txt!");
                    return;
                } else {
                    showTextToast(msg, slotId);
                }
            }
        }
    }

    private boolean isBipCommand(MtkCatCmdMessage cmd) {
        switch (cmd.getCmdType()) {
        case OPEN_CHANNEL:
        case CLOSE_CHANNEL:
        case SEND_DATA:
        case RECEIVE_DATA:
        case GET_CHANNEL_STATUS:
            MtkCatLog.v(this, "BIP command");
            return true;
        }

        MtkCatLog.v(this, "non-BIP command");
        return false;
    }
    //BIP releated end


    //call state related
    private PhoneConstants.State getPhoneCallState() {
        Phone phone = null;
        for (int i = 0; i < mSimCount; i++) {
            phone = getPhone(i);
            if (phone == null) {
                MtkCatLog.w(LOG_TAG, "Phone is null.");
                continue;
            }
            PhoneConstants.State ps = phone.getState();
            MtkCatLog.d(LOG_TAG, "Phone " + i + " state: " + ps);
            if (ps == PhoneConstants.State.RINGING || ps == PhoneConstants.State.OFFHOOK) {
                return ps;
            }
        }
        return PhoneConstants.State.IDLE;
    }

    //browser termination event begin
    private final BroadcastReceiver mBrowserTerminationReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String Action = intent.getAction();
            MtkCatLog.v(LOG_TAG, "mBrowsingTerminationReceiver() - Action[" + Action + "]");
            if (Action.equals(ACTION_BROWSER_TERMINATION)) {
                Message message = mServiceHandler.obtainMessage();
                message.arg1 = OP_BROWSER_TERMINATION;
                mServiceHandler.sendMessage(message);
            }
        }
    };
    //browser termination event end

    //browser status event begin
    private final BroadcastReceiver mBrowsingStatusReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String evtAction = intent.getAction();

            String url = intent.getStringExtra(EXTRA_URL);
            String defaultUrl = intent.getStringExtra(EXTRA_HOMEPAGE);
            int errorCode = intent.getIntExtra(EXTRA_ERROR_CODE, 0);
            MtkCatLog.v(LOG_TAG, "mBrowsingStatusReceiver() - evtAction["
                    + evtAction + "," + mLaunchBrowserUrl +
                    "," + url + "," + defaultUrl + "," + errorCode + "]");
            int i = 0;
            Uri defaultUri = null;
            String authDefaultUrl = null;

            if (null != url) {
                if (DEDICATED_URL == mLaunchBrowserUrlType
                        && null != mLaunchBrowserUrl
                        && url.contains(mLaunchBrowserUrl)) {
                    MtkCatLog.v(LOG_TAG, "contain dedicated url.");
                } else if (DEFAULT_URL == mLaunchBrowserUrlType) {
                    if (null != defaultUrl) {
                        defaultUri = Uri.parse(defaultUrl);
                        if (null == defaultUri) {
                            return;
                        }
                        authDefaultUrl = defaultUri.getAuthority();
                    } else {
                        return;
                    }
                    if (null != authDefaultUrl &&
                        url.contains(authDefaultUrl)) {
                        MtkCatLog.v(LOG_TAG, "contain default url.");
                    } else {
                        return;
                    }
                } else {
                    MtkCatLog.e(LOG_TAG, "unknown url type.");
                    return;
                }
                Message message = mServiceHandler.obtainMessage();
                message.arg1 = OP_BROWSER_STATUS;
                mServiceHandler.sendMessage(message);
            } else {
                MtkCatLog.e(LOG_TAG, "null url.");
            }
        }
    };
    //browser status event end

    //SIM radion on/off begin
    private void registerMSIMModeObserver() {
        MtkCatLog.v(this, "call registerMSIMModeObserver");
        Uri uri = Settings.System.getUriFor(MtkSettingsExt.System.MSIM_MODE_SETTING);
        mContext.getContentResolver().registerContentObserver(
                uri, false, mMSIMModeObserver);
    }

    private void unRegisterMSIMModeObserver() {
        MtkCatLog.v(this, "call unRegisterMSIMModeObserver");
        mContext.getContentResolver().unregisterContentObserver(
                mMSIMModeObserver);
    }

    private ContentObserver mMSIMModeObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();
            for (int i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                boolean radioOn = checkSimRadioStateByModeSetting(i, true);
                MtkCatLog.d(this, "mMSIMModeObserver radioOn: " + radioOn);
                if (radioOn) {
                    if (MtkCatService.getSaveNewSetUpMenuFlag(i) &&
                        mStkContext[i].mMainCmd != null &&
                        isMenuValid(mStkContext[i].mMainCmd.getMenu())) {
                        appInstaller.install(mContext, i);
                        StkAvailable(i, STK_AVAIL_AVAILABLE);
                    }
                } else {
                    if (!isOP02) {
                        appInstaller.unInstall(mContext, i);
                    }
                    StkAvailable(i, STK_AVAIL_NOT_AVAILABLE);
                }
            }
        }
    };
    //SIM radion on/off begin

    //fligt mode on/off begin

    private BroadcastReceiver mAirplaneModeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            boolean airplaneModeEnabled = isAirplaneModeOn(mContext);
            StkAppInstaller appInstaller = StkAppInstaller.getInstance();
            MtkCatLog.v(LOG_TAG, "mAirplaneModeReceiver AIRPLANE_MODE_CHANGED...[" +
                    airplaneModeEnabled + "]");
            if (airplaneModeEnabled) {
                for (int i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                    if(isOP154 && isWifiCallingAvailable(i))
                        continue;

                    if (RadioManager.isFlightModePowerOffModemEnabled()) {
                        if (null != mStkContext) {
                            //Reset all stk context info, since modem will be reset by flight mode.
                            //To avoid user uses the old or null context after receiving card is
                            //absent that results from Cat service is disposed.
                            mStkContext[i].mCurrentMenu = null;
                            mStkContext[i].mCurrentMenuCmd = null;
                            mStkContext[i].mMainCmd = null;
                            if (mStkContext[i].mCmdsQ != null &&
                                mStkContext[i].mCmdsQ.size() != 0) {
                                mStkContext[i].mCmdsQ.clear();
                            }
                        }
                    }

                    if (!isOP02) {
                        appInstaller.unInstall(mContext, i);
                    }
                    StkAvailable(i, STK_AVAIL_NOT_AVAILABLE);
                }
            } else {
                for (int i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                    //Check if the card is inserted and if need show entrance.
                    if (TelephonyManager.getDefault().hasIccCard(i)) {
                        if (MtkCatService.getSaveNewSetUpMenuFlag(i) &&
                            mStkContext[i].mMainCmd != null &&
                            isMenuValid(mStkContext[i].mMainCmd.getMenu())) {
                            appInstaller.install(mContext, i);
                            StkAvailable(i, StkAppService.STK_AVAIL_AVAILABLE);
                        }
                    }
                }
            }
        }
    };
    //fligt mode on/off begin

    //wifi calling mode on/off begein
    private MtkImsConnectionStateListener mImsConnectionStateListener =
            new MtkImsConnectionStateListener() {
                @Override
                public void onImsConnected(int imsRadioTech) {
                    MtkCatLog.d(LOG_TAG, "onImsConnected imsRadioTech=" + imsRadioTech);
                    for (int i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                        boolean isWfcEnabled = MtkTelephonyManagerEx.getDefault()
                                .isWifiCallingEnabled(i);

                        MtkCatLog.d(LOG_TAG, "sim[" + i + "], isWfcEnabled:" + isWfcEnabled);
                        updateStkState(i, imsRadioTech);
                    }
                }

                @Override
                public void onImsDisconnected(ImsReasonInfo imsReasonInfo) {
                    MtkCatLog.d(LOG_TAG, "onImsDisconnected imsReasonInfo=" + imsReasonInfo);
                    for (int i = PhoneConstants.SIM_ID_1; i < mSimCount; i++) {
                        boolean isWfcEnabled = MtkTelephonyManagerEx.getDefault()
                                .isWifiCallingEnabled(i);

                        MtkCatLog.d(LOG_TAG, "sim[" + i + "], isWfcEnabled:" + isWfcEnabled);
                        updateStkState(i, -1);// -1 means to unInstall stk
                    }
                }
            };


    public void registerImsListener() {
        MtkCatLog.d(LOG_TAG, "registerImsListener >>>");
        for (int i=0; i<mSimCount; i++) {
            mImsManager[i] = ImsManager.getInstance(mContext, i);
            unregisterImsListener();
            try {
                ((MtkImsManager) mImsManager[i]).addImsConnectionStateListener(
                        mImsConnectionStateListener);
                MtkCatLog.d(LOG_TAG, "register ims succeed, " + mImsConnectionStateListener);
            } catch (ImsException e) {
                // Could not get the ImsService.
                MtkCatLog.w(LOG_TAG, "register ims fail!");
            }
        }
    }

    public void unregisterImsListener() {
        for(int i=0; i<mSimCount; i++) {
            if (mImsManager[i] != null) {
                try {
                    ((MtkImsManager) mImsManager[i]).removeImsConnectionStateListener(
                            mImsConnectionStateListener);
                    MtkCatLog.d(LOG_TAG, "unregister ims succeed, " + mImsConnectionStateListener);
                } catch (ImsException e) {
                    // Could not get the ImsService.
                    MtkCatLog.w(LOG_TAG, "unregister ims fail!");
                }
            }
        }
    }

    private void updateStkState(int subId, int imsRadioTech)
    {
        if(!isOP154)
            return;

        StkAppInstaller appInstaller = StkAppInstaller.getInstance();
        MtkCatLog.d(LOG_TAG, "subId: " + subId + ", imsRadioTech: " + imsRadioTech);
        switch(imsRadioTech)
        {
            case ImsRegistrationImplBase.REGISTRATION_TECH_LTE:
            case ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN:
                appInstaller.install(mContext, subId);
                StkAvailable(subId, STK_AVAIL_AVAILABLE);
                break;
            default:
                if(isAirplaneModeOn(mContext)) {
                    appInstaller.unInstall(mContext, subId);
                    StkAvailable(subId, STK_AVAIL_NOT_AVAILABLE);
                }
                break;
        }
    }
    //wifi calling mode on/off end

    //for CU spec begin
    private void showSimSwitchDialog(int slotId) {
        MtkCatLog.d(this, " start showSimSwitchDialog for CU");
        final int subId[] = SubscriptionManager.getSubId(slotId);
        if (subId == null) {
            MtkCatLog.e(this, "invalid sub id!");
            return;
        }
        final SubscriptionManager subscriptionManager = SubscriptionManager.from(mContext);
        final TelephonyManager tm = TelephonyManager.from(mContext);

        int activeCount = subscriptionManager.getActiveSubscriptionInfoCount();
        int defaultSubId = SubscriptionManager.getDefaultDataSubscriptionId();;
        int defaultSlotId = SubscriptionManager.getSlotIndex(defaultSubId);

        MtkCatLog.v(this, "isCUCard(slotId) == " + isCUCard(subId[0])
                + "sim count = " + activeCount);

        if (isCUCard(subId[0]) && activeCount > 1 && slotId != defaultSlotId) {
            final int anotherSubId = getSubIdBySlot(1 - slotId);

            MtkCatLog.d(this, "isCUCard(anotherSubId) == " + isCUCard(anotherSubId));
            if (!isCUCard(anotherSubId)) {
                SubscriptionInfo activeSub = subscriptionManager
                        .getActiveSubscriptionInfo(subId[0]);
                if (activeSub == null) {
                    MtkCatLog.e(this, "invalid active sub id!");
                    return;
                }
                CharSequence currentSimName = activeSub.getDisplayName();
                if (currentSimName == null) {
                    currentSimName = "SIM " + (activeSub.getSimSlotIndex() + 1);
                }
                String message = String.format(getResources().getString(R.string.sim_switch),
                        currentSimName);
                final AlertDialog SimSwitchDialog = new AlertDialog.Builder(mContext,
                        AlertDialog.THEME_DEVICE_DEFAULT_DARK)
                        .setMessage(message)
                        .setPositiveButton(getResources().getString(R.string.stk_dialog_accept),
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {
                                        subscriptionManager.setDefaultDataSubId(subId[0]);
                                    }
                                })
                        .setNegativeButton(getResources().getString(R.string.stk_dialog_reject),
                                new DialogInterface.OnClickListener() {
                                    public void onClick(DialogInterface dialog, int which) {

                                    }
                                }).create();
                SimSwitchDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
                SimSwitchDialog.show();
            }
        }
    }

    private boolean isCUCard(int subId) {
        MtkCatLog.v(this, "isCUCard, subId = " + subId);
        String simOperator = null;
        simOperator = getSimOperator(subId);
        if (simOperator != null) {
            MtkCatLog.v(this, "isCUCard, simOperator =" + simOperator);
            for (String mccmnc : MCCMNC_TABLE_TYPE_CU) {
                if (simOperator.equals(mccmnc)) {
                    return true;
                }
            }
        }
        return false;
    }

    private String getSimOperator(int subId) {
        if (subId < 0) {
            return null;
        }
        String simOperator = null;
        int status = TelephonyManager.SIM_STATE_UNKNOWN;
        int slotId = SubscriptionManager.getSlotIndex(subId);
        if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            status = TelephonyManager.getDefault().getSimState(slotId);
        }
        if (status == TelephonyManager.SIM_STATE_READY) {
            simOperator = TelephonyManager.getDefault().getSimOperator(subId);
        }
        MtkCatLog.v(this, "getSimOperator, simOperator = " + simOperator + " subId = " + subId);
        return simOperator;
    }

    private int getSubIdBySlot(int slotId) {
        MtkCatLog.v(this, "SlotId = " + slotId);
        if (slotId < 0 || slotId > 1) {
            return -1;
        }
        int[] subids = SubscriptionManager.getSubId(slotId);
        int subid = -1;
        if (subids != null && subids.length >= 1) {
            subid = subids[0];
        }
        MtkCatLog.v(this, "GetSimIdBySlot: sub id = " + subid + "sim Slot = " + slotId);
        return subid;
    }

    //for CU spec begin

    //get menu title form EF begin
    private void updateMenuTitleFromEf(String appName, int slotId) {
        String newAppName = appName;
        MtkCatLog.d(this, "updateMenuTitleFromEf, appName: " + appName);
        if (null == newAppName || newAppName.isEmpty() || 0 == newAppName.length()) {
            MtkCatLog.d(this, "appName is invalid valule");
            // TODO: read SIM EF(Elementary File) SUME(Setup Menu
            // Entity) Address 6F54
            newAppName = getMenuTitleFromEf(slotId);
        }
        if (null != newAppName && !newAppName.isEmpty()) {
            MtkCatLog.d(this, "update appName: " + newAppName);
            updateAppName(newAppName);
        }
    }

    private String getMenuTitleFromEf(int slotId) {
        String title = null;
        IccRecords ic = null;
        //ic = mStkService[slotId].getIccRecords();
        if (ic != null) {
            //title = ic.getMenuTitleFromEf();
            title = "testTitle";
        }
        if (title != null) {
            MtkCatLog.d(this, "getMenuTitleFromEf, Title:" + title);
        } else {
            MtkCatLog.d(this, "getMenuTitleFromEf, Title is null");
        }
        return title;
    }

    final static String STK_TITLE_KEY = "gsm.setupmenu.title";
    final static String STK_TITLE_KEY2 = "gsm.setupmenu.title2";

    private void updateAppName(String appName) {
        if (appName.length() >= 30) {
            MtkCatLog.d(this, "title too long");
            return;
        }
        MtkCatLog.d(this, "set menu title in SystemProperties to " + appName);
        SystemProperties.set(STK_TITLE_KEY, appName);
    }
    //get menu title form EF begin


    //common function begin

    private boolean isMenuValid(Menu menu) {
        if (menu == null) {
            MtkCatLog.w(this, "Null menu.");
            return false;
        }
        if (menu.items.size() == 1 &&
            menu.items.get(0) == null) {
            return false;
        }
        return true;
    }

    public void StkAvailable(int slotId, int available) {
        if (mStkContext[slotId] != null) {
            mStkContext[slotId].mAvailable = available;
        }
        MtkCatLog.v(LOG_TAG, "slotId: " + slotId + ", available: " + available + ", StkAvailable: "
                + ((mStkContext[slotId] != null) ? mStkContext[slotId].mAvailable : -1));
    }

    public int StkQueryAvailable(int slotId) {
        int result = ((mStkContext[slotId] != null) ? mStkContext[slotId].mAvailable : -1);

        MtkCatLog.v(LOG_TAG, "slotId: " + slotId + ", StkQueryAvailable: " + result);
        return result;
    }

    private boolean checkSimRadioStateByModeSetting(int slotId, boolean by_mode_setting) {
        boolean isSimOnOffEnabled = MtkTelephonyManagerEx.getDefault().isSimOnOffEnabled();
        MtkCatLog.d(LOG_TAG, "checkSimRadioState - slotId[" + slotId +
                "], isSimOnOffEnabled[" + isSimOnOffEnabled + "]");
        if (isSimOnOffEnabled) {
            return isSimOnState(slotId);
        } else {
            if (by_mode_setting) {
                return isRadioOnStateByModeSetting(slotId);
            } else {
                return isRadioOnState(slotId);
            }
        }
    }

    private boolean isRadioOnStateByModeSetting(int slotId) {
        boolean radioOn = true;
        int multiSimMode = -1;

        /* multiSimMode, bit used: 0 => all sim are off, 1 => SIM1 is on, 2 => SIM2 is on
        3 => SIM1 & SIM2 both on, 4 => SIM 3 is on*/
        multiSimMode = Settings.System.getInt(mContext.getContentResolver(),
                MtkSettingsExt.System.MSIM_MODE_SETTING, -1);

        int curRadioOnSim = (multiSimMode & (0x01 << slotId));
        MtkCatLog.d(LOG_TAG, "multiSimMode: " + multiSimMode + ", slotId: " + slotId
                + ", curRadioOnSim: " + curRadioOnSim);
        if (curRadioOnSim != 0) {
            return true;
        } else {
            return false;
        }
    }

    private boolean isRadioOnState(int slotId) {
        boolean radioOn = true;
        MtkCatLog.d(LOG_TAG, "isRadioOnState check = " + slotId);

        try {
            ITelephony phone = ITelephony.Stub.asInterface(
                    ServiceManager.getService(Context.TELEPHONY_SERVICE));
            if (phone != null) {
                int subId[] = SubscriptionManager.getSubId(slotId);
                radioOn = phone.isRadioOnForSubscriber(subId[0],
                        getApplicationContext().getOpPackageName());
            }
            MtkCatLog.d(LOG_TAG, "isRadioOnState - radio_on[" + radioOn + "]");
        } catch (RemoteException e) {
            e.printStackTrace();
            MtkCatLog.d(LOG_TAG, "isRadioOnState - Exception happen ====");
        }
        return radioOn;
    }

    private boolean isSimOnState(int slotId) {
        boolean simOn = (MtkTelephonyManagerEx.getDefault().getSimOnOffState(slotId)
                == MtkTelephonyManagerEx.SIM_POWER_STATE_SIM_ON );
        MtkCatLog.d(LOG_TAG, "isSimOnState - slotId[" + slotId + "], sim_on[" + simOn + "]");

        return simOn;
    }


    private Phone getPhone(int slotId) {
        MtkCatLog.v(LOG_TAG, "getPhone slotId: " + slotId);
        if (mPhone[slotId] != null) {
            return mPhone[slotId];
        }

        int subId[] = SubscriptionManager.getSubId(slotId);
        int phoneId = 0;
        if (subId != null && SubscriptionManager.isValidSubscriptionId(subId[0])) {
            phoneId = SubscriptionManager.getPhoneId(subId[0]);
            MtkCatLog.v(LOG_TAG, "subId[0] " + subId[0] + " phoneId: " + phoneId);
            if (!SubscriptionManager.isValidPhoneId(phoneId)) {
                MtkCatLog.w(LOG_TAG, "ERROR: invalid phone id.");
                return null;
            }
        } else {
            //FIXME
            if (subId == null) {
                MtkCatLog.w(LOG_TAG, "ERROR: sub array is null.");
            } else {
                MtkCatLog.w(LOG_TAG, "ERROR: sub id is invalid. sub id: " + subId[0]);
            }
            return null; //should return null and caller should handle null object case.
        }
        try {
            mPhone[slotId] = PhoneFactory.getPhone(phoneId);
        } catch (IllegalStateException e) {
            MtkCatLog.e(LOG_TAG, "IllegalStateException, get phone fail.");
            e.printStackTrace();
            return null;
        }
        return mPhone[slotId];
    }

    private ImsManager getImsManager(int subId) {
        return ImsManager.getInstance(mContext, SubscriptionManager.getPhoneId(subId));
    }

    private boolean canShowTextDialog(TextMessage msg, int slotId) {
        if (msg == null) {
            return true;
        }
        MtkCatLog.v(LOG_TAG, "canShowTextDialog: mMenuIsVisible: " +
                mStkContext[slotId].mMenuIsVisible + " mDisplayTextDlgIsVisibile: " +
                mStkContext[slotId].mDisplayTextDlgIsVisibile + "isHighPriority: " +
                msg.isHighPriority);

        if (msg.isHighPriority || mStkContext[slotId].mMenuIsVisible
                            || mStkContext[slotId].mDisplayTextDlgIsVisibile || isTopOfStack()) {
            return true;
        }
        if (isScreenIdle()) {
            return true;
        }
        return false;
    }

    private void delayToCheckIdle(int slotId) {
        MtkCatLog.d(LOG_TAG, "delayToCheckIdle, sim id: " + slotId);
        Message msg1 = null;
        if (PhoneConstants.SIM_ID_1 == slotId) {
            msg1 = mServiceHandler.obtainMessage(OP_DELAY_TO_CHECK_IDLE_SIM1);
            msg1.arg1 = OP_DELAY_TO_CHECK_IDLE_SIM1;
        } else if (PhoneConstants.SIM_ID_2 == slotId) {
            msg1 = mServiceHandler.obtainMessage(OP_DELAY_TO_CHECK_IDLE_SIM2);
            msg1.arg1 = OP_DELAY_TO_CHECK_IDLE_SIM2;
        } else if (PhoneConstants.SIM_ID_3 == slotId) {
            msg1 = mServiceHandler.obtainMessage(OP_DELAY_TO_CHECK_IDLE_SIM3);
            msg1.arg1 = OP_DELAY_TO_CHECK_IDLE_SIM3;
        } else if (PhoneConstants.SIM_ID_4 == slotId) {
            msg1 = mServiceHandler.obtainMessage(OP_DELAY_TO_CHECK_IDLE_SIM4);
            msg1.arg1 = OP_DELAY_TO_CHECK_IDLE_SIM4;
        } else {
            MtkCatLog.v(LOG_TAG, "invalid slotId");
            return;
        }
        msg1.arg2 = slotId;
        mServiceHandler.sendMessageDelayed(msg1, DELAY_TO_CHECK_IDLE_TIMEOUT);
    }

    private void showIconToast(TextMessage msg) {
        MtkCatLog.v(LOG_TAG, "showIconToast");
        Toast t = new Toast(this);
        ImageView v = new ImageView(this);
        v.setImageBitmap(msg.icon);
        t.setView(v);
        t.setDuration(Toast.LENGTH_LONG);
        t.show();
    }

    private void showTextToast(TextMessage msg, int slotId) {
        MtkCatLog.v(LOG_TAG, "showTextToast msg: " + msg.text);
        msg.title = mStkContext[slotId].lastSelectedItem;

        Toast toast = Toast.makeText(mContext.getApplicationContext(), msg.text,
                Toast.LENGTH_LONG);
        toast.setGravity(Gravity.BOTTOM, 0, 0);
        toast.show();
    }

    // TODO should show text and Icon
    private void showIconAndTextToast(TextMessage msg) {
        MtkCatLog.v(LOG_TAG, "showIconAndTextToast");
        Toast t = new Toast(this);
        ImageView v = new ImageView(this);
        v.setImageBitmap(msg.icon);
        t.setView(v);
        t.setDuration(Toast.LENGTH_LONG);
        t.show();
    }

    private boolean isAirplaneModeOn(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
    }

    private void correctTextMessage(TextMessage msg, int slotId) {
        switch (mStkContext[slotId].mCurrentCmd.getCmdType()) {
            case OPEN_CHANNEL:
                if (msg.text == null) {
                    msg.text = getDefaultText(slotId);
                }
                break;
            default:
                if (msg.text == null || msg.text.length() == 0) {
                    msg.text = getDefaultText(slotId);
                }
        }
    }

    private String getDefaultText(int slotId) {
        String str = "";
        MtkCatLog.v(LOG_TAG, "getDefaultText: " + mStkContext[slotId].mCurrentCmd.getCmdType());
        switch (mStkContext[slotId].mCurrentCmd.getCmdType()) {
        case LAUNCH_BROWSER:
            str = getResources().getString(R.string.action_launch_browser);
            break;
        case SET_UP_CALL:
            str = getResources().getString(R.string.action_setup_call);
            break;
        case OPEN_CHANNEL:
            str = getResources().getString(R.string.lable_open_channel);
            break;
        }
        return str;
    }

    private boolean isWifiCallingAvailable(int phoneId) {
        final MtkTelephonyManagerEx tm = MtkTelephonyManagerEx.getDefault();
        int subId = getSubIdUsingPhoneId(phoneId);
        return tm.isWifiCallingEnabled(subId);
    }

    private int getSubIdUsingPhoneId(int phoneId) {
        SubscriptionController subCon = SubscriptionController.getInstance();
        int subId = (subCon != null)?
                subCon.getSubIdUsingPhoneId(phoneId) : SubscriptionManager.INVALID_SUBSCRIPTION_ID;

        MtkCatLog.d(LOG_TAG, "[getSubIdUsingPhoneId] subId " + subId + ", phoneId " + phoneId);
        return subId;
    }
    //common function end
    /// @}

}
