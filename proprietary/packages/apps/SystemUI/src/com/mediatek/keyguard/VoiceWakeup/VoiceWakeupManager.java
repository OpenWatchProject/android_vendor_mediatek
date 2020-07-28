/**
 * M : Used for Voice wakeup feature.
 *     - Dismiss keyguard (if authenticated).
 *     - Launch App.
 */

package com.mediatek.keyguard.VoiceWakeup;

import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.UserHandle;
import android.provider.MediaStore;
import android.provider.Settings;
import android.util.Log;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.widget.LockPatternUtils;
import com.android.keyguard.KeyguardSecurityModel;
import com.android.keyguard.KeyguardSecurityModel.SecurityMode;
//import com.android.systemui.keyguard.KeyguardViewMediator;
import com.android.keyguard.KeyguardUpdateMonitor;
import com.android.keyguard.KeyguardUpdateMonitorCallback;
import com.android.keyguard.KeyguardUtils;
import com.android.keyguard.ViewMediatorCallback;
import com.android.systemui.plugins.ActivityStarter.OnDismissAction;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.keyguard.AntiTheft.AntiTheftManager;
import com.mediatek.provider.MtkSettingsExt;

public class VoiceWakeupManager implements OnDismissAction {
    private static final boolean DEBUG = true;
    private static final String TAG = "VoiceWakeupManager";

    private static VoiceWakeupManager sInstance = null;
    private Context mContext = null;
    private LockPatternUtils mLockPatternUtils;
    private ViewMediatorCallback mViewMediatorCallback;
    private Handler mHandler;
    private PowerManager mPM;
    private KeyguardSecurityModel securityModel;

    private IVoiceCommandManagerService mVCmdMgrService;
    private boolean isRegistered = false;
    private String mPkgName;
    private String mLaunchApp;
    private LimitedModeApp[] limitedApps = {
        new LimitedModeApp("com.android.gallery3d/com.android.camera.CameraLauncher",
                MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE)
    };

    // Just for test. May support in future.
    private static final boolean VOW_SCREEN_ON_EARLIER = false;
    // Just for test. May support in future.
    private static final boolean VOW_ALWAYS_ON = false;

    // M: ALPS02384775 fixed voice search cannot display when swipe mode
    private static final String VOICE_SEARCH = "VoiceSearchActivity";

    public VoiceWakeupManager() {
        Log.d(TAG, "constructor is called.");
    }

    public static VoiceWakeupManager getInstance() {
        Log.d(TAG, "getInstance(...) is called.");
        if (sInstance == null) {
            Log.d(TAG, "getInstance(...) create one.");
            sInstance = new VoiceWakeupManager();
        }
        return sInstance;
    }

    @VisibleForTesting
    public boolean checkIfVowSupport(Context context) {
        boolean support = false;

        if (context != null) {
            /// temp fix in KK.AOSP because systemProperty is not yet created
            if (KeyguardUtils.isVoiceWakeupSupport(context)
                && !KeyguardUtils.isVoiceTriggerSupport(context)) {
                log("VoiceWakeup demo is enabled in this load.");
                support = true;
            } else {
                log("VoiceWakeup demo is NOT enabled in this load.");
            }
        } else {
            log("checkIfVowSupport() - context is still null, bypass the check...");
        }

        return support;
    }

    public void init(Context context, ViewMediatorCallback viewMediatorCallback) {
        log("init() is called.");

        mContext = context;

        if (!checkIfVowSupport(context)) {
            return;
        }

        mLockPatternUtils = new LockPatternUtils(context);
        mViewMediatorCallback = viewMediatorCallback;
        securityModel = new KeyguardSecurityModel(mContext);
        mPkgName =  mContext.getPackageName();
        mHandler = new Handler();

        mPM = (PowerManager) mContext.getSystemService(Context.POWER_SERVICE);

        KeyguardUpdateMonitor.getInstance(mContext).registerCallback(mUpdateCallback);
        start();
    }

    public void start() {
        log("start()");

        if (!checkIfVowSupport(mContext)) {
            return;
        }

        log("register to service");
        if (mVCmdMgrService == null) {
            bindVoiceService(mContext);
        } else {
            registerVoiceCommand(mPkgName);
        }

        return;
    }

    public void stop() {
        log("stop()");

        if (!checkIfVowSupport(mContext)) {
            return;
        }

        /*if (mHandler.getLooper() != Looper.myLooper()) {
            log("stop() called off of the UI thread");
        }*/

        if (mVCmdMgrService != null) {
            log("unregister to service");
            unregisterVoicecommand(mPkgName);
        }

        return;
    }

    // ===== Voice Service Related : start =====
    @VisibleForTesting
    public void sendVoiceCommand(String pkgName, int mainAction, int subAction,
            Bundle extraData) {
        if (isRegistered) {
            try {
                int errorid = mVCmdMgrService.sendCommand(pkgName, mainAction, subAction,
                        extraData);
                if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                    log("send voice Command fail ");
                } else {
                    log("send voice Command success ");
                }
            } catch (RemoteException e) {
                isRegistered = false;
                mVCmdMgrService = null;
                log("send voice Command RemoteException =  " + e.getMessage());
            }
        } else {
            log("didn't register , can not send voice Command  ");
        }
    }

    private void registerVoiceCommand(String pkgName) {
        if (!isRegistered) {
            try {
                int errorid = mVCmdMgrService.registerListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    isRegistered = true;

                    log("register voiceCommand successfuly, now send VOICE_WAKEUP_START");

                    sendVoiceCommand(mPkgName,
                        VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP,
                        VoiceCommandListener.ACTION_VOICE_WAKEUP_START, null);
                } else {
                    log("register voiceCommand fail errorid=" + errorid
                            + " with pkgName="+pkgName);
                }
            } catch (RemoteException e) {
                isRegistered = false;
                mVCmdMgrService = null;
                log("register voiceCommand RemoteException =  " + e.getMessage());
            }
        } else {
            log("register voiceCommand success ");
        }
        log("register voiceCommand end ");
    }

    private void unregisterVoicecommand(String pkgName) {
        if (mVCmdMgrService != null) {
            try {
                int errorid = mVCmdMgrService.unregisterListener(pkgName, mVoiceCallback);
                if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
                    isRegistered = false;
                }
            } catch (RemoteException e) {
                log("unregisteVoiceCmd voiceCommand RemoteException = " + e.getMessage());
                isRegistered = false;
                mVCmdMgrService = null;
            }
            log("unregisteVoiceCmd end ");
            mContext.unbindService(mVoiceServiceConnection);
            mVCmdMgrService = null;
            isRegistered = false;
        }
    }

    private void bindVoiceService(Context context) {
        log("bindVoiceService begin  ");
        Intent mVoiceServiceIntent = new Intent();
        mVoiceServiceIntent.setAction(VoiceCommandListener.VOICE_SERVICE_ACTION);
        mVoiceServiceIntent.addCategory(VoiceCommandListener.VOICE_SERVICE_CATEGORY);
        mVoiceServiceIntent.setPackage(VoiceCommandListener.VOICE_SERVICE_PACKAGE_NAME);
        context.bindService(mVoiceServiceIntent, mVoiceServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    private ServiceConnection mVoiceServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            log("onServiceConnected   ");
            mVCmdMgrService = IVoiceCommandManagerService.Stub.asInterface(service);
            registerVoiceCommand(mPkgName);
        }
        @Override
        public void onServiceDisconnected(ComponentName name) {
            log("onServiceDisconnected  ");
            isRegistered = false;
            mVCmdMgrService = null;
        }

    };

    private IVoiceCommandListener mVoiceCallback = new IVoiceCommandListener.Stub() {
        public void onVoiceCommandNotified(int mainAction, int subAction, Bundle extraData)
                throws RemoteException {
            int result = extraData.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT);
            log("onNotified result=" + result + " mainAction = " + mainAction
                    + " subAction = " + subAction);

            if (result == VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS) {
                if (mainAction == VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP &&
                        subAction == VoiceCommandListener.ACTION_VOICE_WAKEUP_NOTIFY) {
                    Message.obtain(mVoiceCommandHandler, mainAction, subAction, 0,
                    extraData).sendToTarget();
                }
            }
        }
    };

    private Handler mVoiceCommandHandler = new Handler() {
        public void handleMessage(Message msg) {
            handleVoiceCommandNotified((Bundle) msg.obj);
         }
    };

    @VisibleForTesting
    public void handleVoiceCommandNotified(Bundle data) {
        int commandId = data.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO);
        boolean isUserDependentMode = (data.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1)
                == VoiceCommandListener.VOICE_WAKEUP_MODE_SPEAKER_DEPENDENT);

        log("data.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1) = "
                + data.getInt(VoiceCommandListener.ACTION_EXTRA_RESULT_INFO1));
        log("handleVoiceCommandNotified() commandId = " + commandId + " isUserDependentMode = "
                + isUserDependentMode);

        doLaunchAppAndDismissKeyguard(commandId, isUserDependentMode);
    }
    // ===== Voice Service Related : end =====


    // ===== Launch App & Dismiss Keyguard : start =====
    private boolean mIsDismissAndLaunchApp = false;
    public boolean isDismissAndLaunchApp() {
        log("isDismissAndLaunchApp() mIsDismissAndLaunchApp = " + mIsDismissAndLaunchApp);
        return mIsDismissAndLaunchApp ;
    }

    private class LimitedModeApp {
        public String normalModeAppName;
        public String limtedModeAppName;

        public LimitedModeApp(String normalName, String limitedName) {
            normalModeAppName = normalName;
            limtedModeAppName = limitedName;
        }
    }

    private String getLimtiedModeActionNameOfApp(String appName) {
        String actionName = null;

        /*if(appName.equals("com.android.gallery3d/com.android.camera.CameraLauncher")) {
            actionName = MediaStore.INTENT_ACTION_STILL_IMAGE_CAMERA_SECURE ;
        }*/

        for (int i = 0; i < limitedApps.length; i++) {
            if (appName.equals(limitedApps[i].normalModeAppName)) {
                actionName = limitedApps[i].limtedModeAppName;
                break;
            }
        }

        return actionName;
    }

    private String getLaunchAppNameFromSettings(int commandId) {
        String appName = MtkSettingsExt.System.getVoiceCommandValue(mContext.getContentResolver(),
                MtkSettingsExt.System.BASE_VOICE_WAKEUP_COMMAND_KEY, commandId);
        Log.d(TAG, "getLaunchAppNameFromSettings() - appName = " + appName);
        return appName;
    }

    private void doLaunchAppAndDismissKeyguard(int commandId, boolean isUserDependentMode) {
        mIsDismissAndLaunchApp = false;

        mLaunchApp = getLaunchAppNameFromSettings(commandId);
        if (mLaunchApp == null) {
            //return;
            //Sometimes we need to do verifications even if GMS does not exist.
            //In this case, mLaunchApp will be null and cannot do any futher test.
            //So we give a default App to launch to proceed the verifications.
            Log.d(TAG, "AppName does not exist in Setting DB, give it a default value.");
            mLaunchApp = "com.android.contacts/com.android.contacts.activities.PeopleActivity";
        }

        boolean isAntitheftMode =
                AntiTheftManager.getInstance(null, null, null).isAntiTheftLocked();
        boolean isKeyguardExternallyDisabled =
                !mViewMediatorCallback.isKeyguardExternallyEnabled();
        if (isAntitheftMode || isKeyguardExternallyDisabled) {
            log("Give up launching since isAntitheftMode = " + isAntitheftMode +
                    " isKeyguardExternallyDisabled = " + isKeyguardExternallyDisabled);
            return;
        }

        if (mPM.isScreenOn() && !VOW_ALWAYS_ON) {
            log("Give up launching since screen is on but we do not allow this case.");
            return;
        }

        mIsDismissAndLaunchApp = true ;

        //if keyguard is in the "later locked" status and VoiceWakeup flow starts,
        //we should show lock screen immediately for the rest flow.
        boolean isInLaterLocked = !mLockPatternUtils.isLockScreenDisabled(
                KeyguardUpdateMonitor.getCurrentUser()) && !mPM.isScreenOn()
                && !mViewMediatorCallback.isShowing();
        if (isInLaterLocked) {
            log("doLaunchAppAndDismissKeyguard() : call showLocked() due to keyguard is"
                    + "in the later locked status");
            mViewMediatorCallback.showLocked(null);
        }

        if (VOW_SCREEN_ON_EARLIER) {
            log("doLaunchAppAndDismissKeyguard() : VOW_SCREEN_ON_EARLIER is true."
                    + "(try to act like Moto behavior)");
            mPM.wakeUp(SystemClock.uptimeMillis());
        }

        if (!mViewMediatorCallback.isShowing() && mPM.isScreenOn() && VOW_ALWAYS_ON) {
            log("doLaunchAppAndDismissKeyguard() : screen is on and no keyguard shows,"
                    +"launch full-access mode APP(VOW_ALWAYS_ON)");
            // 1. launch app
            ComponentName cn = ComponentName.unflattenFromString(mLaunchApp);
            final Intent intent = new Intent();
            intent.setComponent(cn);
            intent.setAction(Intent.ACTION_MAIN);
            launchApp(intent);

            mIsDismissAndLaunchApp = false ;
        } else if (mLockPatternUtils.isLockScreenDisabled(KeyguardUpdateMonitor.getCurrentUser())
                && securityModel.getSecurityMode(KeyguardUpdateMonitor.getCurrentUser())
                        == KeyguardSecurityModel.SecurityMode.None) {
            log("doLaunchAppAndDismissKeyguard() : Keyguard is DISABLED, launch full-access mode"
                    + " APP and dismiss keyguard.");
            // 1. launch app
            ComponentName cn = ComponentName.unflattenFromString(mLaunchApp);
            final Intent intent = new Intent();
            intent.setComponent(cn);
            intent.setAction(Intent.ACTION_MAIN);
            launchApp(intent);
            // 2. light up the screen.
            lightUpScreen();

            mIsDismissAndLaunchApp = false;
        } else if (!mLockPatternUtils.isSecure(KeyguardUpdateMonitor.getCurrentUser())
                && securityModel.getSecurityMode(KeyguardUpdateMonitor.getCurrentUser())
                        == KeyguardSecurityModel.SecurityMode.None) {
            log("doLaunchAppAndDismissKeyguard() : Keyguard is SLIDE mode, launch full-access mode"
                    + " APP and dismiss keyguard.");

            // M: ALPS02384775 fixed voice search cannot display when swipe mode
            ComponentName cn = ComponentName.unflattenFromString(mLaunchApp);
            if (cn.getClassName().indexOf(VOICE_SEARCH) != -1) {
                lightUpScreen();
            }

            // 1. force to dismiss keyguard
            mViewMediatorCallback.dismiss(true);
            // 2. launch specified APP -- The app is launched by onDismiss() called in
            //         KeyguardHostView.showNextSecurityScreenOrFinish().
            // 3. light up screen -- The screen will be lighted up by lightUpScreen()
            //         called in KeyguardViewMediator.keyguardGone().
        } else {
            log("doLaunchAppAndDismissKeyguard() : Keyguard is secured.");

            if (isUserDependentMode) {
                if (isSimPinPukMeModeNow()) {
                    log("doLaunchAppAndDismissKeyguard() : isUserDependentMode = TRUE but SIM "
                            + "PIN/PUK/ME screen shows, light up to request the password.");
                    lightUpScreen();
                    //mViewMediatorCallback.setShowNextViewAfterSimLock(false);
                }
                else {
                    log("doLaunchAppAndDismissKeyguard() : isUserDependentMode = TRUE, launch "
                            + "full-access mode APP and dismiss keyguard.");

                    // 1. force to dismiss keyguard
                    mViewMediatorCallback.dismiss(true);
                    // 2. launch specified APP -- The app is launched by onDismiss() called
                    //          in KeyguardHostView.showNextSecurityScreenOrFinish().
                    // 3. light up screen -- The screen will be lighted up by lightUpScreen()
                    //          called in KeyguardViewMediator.keyguardGone().
                }
            } else {
                String limitedModeName = getLimtiedModeActionNameOfApp(mLaunchApp);
                if (limitedModeName != null) {
                    log("doLaunchAppAndDismissKeyguard() : isUserDependentMode = FALSE "
                            + "& APP has limited mode, launch limited-access mode APP");

                    /// 1. launch activity
                    final Intent intent = new Intent(limitedModeName).addFlags(
                            Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
                    launchApp(intent);
                    /// 2. light up screen (we cannot light up screen until the activity
                    ///         is launched.)
                    delayToLightUpScreen = true;
                } else {
                    log("doLaunchAppAndDismissKeyguard() : isUserDependentMode = FALSE "
                            + "& APP does not have limited mode, light up to request the password");

                    /// 1. light up screen(we should light up screen since we ask user
                    ///         to enter PW at first.)
                    lightUpScreen();
                    /// 2. always ask user to enter pw to launch activity
                    mViewMediatorCallback.dismiss(false);
                }
            }
        }
    }

    /*private void dismissKeyguardOnNextActivity() {
        try {
            ActivityManagerNative.getDefault().keyguardWaitingForActivityDrawn();
        } catch (RemoteException e) {
            Log.w(TAG, "can't dismiss keyguard on launch");
        }
    }*/

    private void launchApp(final Intent intent) {
        log("launchApp() enters.");

        //dismissKeyguardOnNextActivity();

        intent.setFlags(
                Intent.FLAG_ACTIVITY_NEW_TASK
                | Intent.FLAG_ACTIVITY_SINGLE_TOP
                | Intent.FLAG_ACTIVITY_CLEAR_TOP);

        mHandler.postDelayed(new Runnable() {
            @Override
            public void run() {
                try {
                    mContext.startActivityAsUser(intent, new UserHandle(UserHandle.USER_CURRENT));
                    log("startActivity intent = " + intent.toString());
                } catch (ActivityNotFoundException e) {
                    log("Activity not found for intent + " + intent.getAction());
                }
            }
        }, 500);
    }
    // ===== Launch App & Dismiss Keyguard : end =====

    // ===== KeyguardHostView.OnDismissAction : start =====
    @Override
    public boolean onDismiss() {
        log("onDismiss() is called.");

        if (!checkIfVowSupport(mContext)) {
            return false;
        }

        ComponentName cn = ComponentName.unflattenFromString(mLaunchApp);
        final Intent intent = new Intent();
        intent.setComponent(cn);
        intent.setAction(Intent.ACTION_MAIN);
        launchApp(intent);

        return true;
    }
    // ===== KeyguardHostView.OnDismissAction : end =====

    // ===== Misc : start =====
    private boolean isSimPinPukMeModeNow() {
        int userId = KeyguardUpdateMonitor.getCurrentUser();
        if ((securityModel.getSecurityMode(userId) == SecurityMode.SimPinPukMe1) ||
            (securityModel.getSecurityMode(userId) == SecurityMode.SimPinPukMe2) ||
            (securityModel.getSecurityMode(userId) == SecurityMode.SimPinPukMe3) ||
            (securityModel.getSecurityMode(userId) == SecurityMode.SimPinPukMe4)
        ) {
            return true;
        }

        return false;

    }

    private static boolean delayToLightUpScreen = false;
    private KeyguardUpdateMonitorCallback mUpdateCallback = new KeyguardUpdateMonitorCallback() {
        @Override
        public void onKeyguardVisibilityChanged(boolean showing) {
            log("onKeyguardVisibilityChanged(" + showing + ")");
            if (delayToLightUpScreen && !showing) {
                lightUpScreen();
                delayToLightUpScreen = false;
                mIsDismissAndLaunchApp = false; //since the job is done.
            } //end if
            else if (mIsDismissAndLaunchApp == true && mPM.isScreenOn() && !showing) {
                log("onKeyguardVisibilityChanged() : Keyguard is hidden now, set "
                        + "mIsDismissAndLaunchApp = false(ex:phone call screen shows)");
                mIsDismissAndLaunchApp = false;
            }
        }

        @Override
        public void onFinishedGoingToSleep(int why) {
            log("onFinishedGoingToSleep - we should reset mIsDismissAndLaunchApp "
                    +"when screen is off.");
            // To avoid we call VoiceWakeManager.onDismiss() incorrectly, we should reset this
            // value in case user decide to give up the voice wakeup operation.
            // Ex, in user-independent & sercure & full-access mode, user may
            // decide to give up launching and press power-key to leave.
            mIsDismissAndLaunchApp = false;
            // By default, we will bind Voice Service in KeyguardViewMediator constructor.
            // To avoid failing to bind at that moment, we should give VoiceWakeupManager
            // more chance.
            start();

            //sendVoiceCommand(mPkgName,
            //            VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP,
            //            VoiceCommandListener.ACTION_VOICE_WAKEUP_START, null);
        }
    };

    private void lightUpScreen() {
        log("lightUpScreen() is called.");
        // check if need to light up screen or not.
        if (mIsDismissAndLaunchApp && !mPM.isScreenOn()) {
            log("lightUpScreen(), call PowerManager.wakeUp()");
            mPM.wakeUp(SystemClock.uptimeMillis());
        }
        //mKeyguardViewMediator.getSecurityCallback().userActivity(millis);
    }

    public void notifyKeyguardIsGone() {
        log("notifyKeyguardGoneAndLightUpScreen() enters");

        if (!checkIfVowSupport(mContext)) {
            return;
        }

        lightUpScreen();
        mIsDismissAndLaunchApp = false;
    }

    public void notifySecurityModeChange(KeyguardSecurityModel.SecurityMode currentMode,
            KeyguardSecurityModel.SecurityMode nextMode) {
        if (!checkIfVowSupport(mContext)) {
            return;
        }

        log("notifySecurityModeChange curr = " + currentMode + ", next = " + nextMode);
        log("notifySecurityModeChange original mIsDismissAndLaunchApp = "
                + mIsDismissAndLaunchApp);

        if (mPM.isScreenOn() && mIsDismissAndLaunchApp) {
            if (nextMode == KeyguardSecurityModel.SecurityMode.AntiTheft) {
                log("notifySecurityModeChange(): mIsDismissAndLaunchApp = false");
                mIsDismissAndLaunchApp = false;
            }
        }
    }

    private void log(String msg) {
        if (DEBUG) {
            Log.d(TAG, msg);
        }
    }
    // ===== Misc : end =====

    // === For AT test === begin
    public boolean isRegisted() {
        return isRegistered;
    }

    public void setPackageName(String pkgName) {
        mPkgName = pkgName;
    }
}
