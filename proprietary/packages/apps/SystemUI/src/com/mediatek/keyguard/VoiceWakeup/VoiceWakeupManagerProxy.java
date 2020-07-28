package com.mediatek.keyguard.VoiceWakeup;

import android.os.Bundle;
import android.content.Context;
import android.util.Log;

import com.android.keyguard.KeyguardSecurityModel;
import com.android.keyguard.KeyguardSecurityModel.SecurityMode;
import com.android.keyguard.ViewMediatorCallback ;

import java.lang.reflect.Method;

public class VoiceWakeupManagerProxy {

    private static final String TAG = "VoiceWakeupManagerProxy";

    private static final String sVOICE_WAKEUP_MANAGER_PACKAGE_PATH
            = "com.mediatek.keyguard.VoiceWakeup.VoiceWakeupManager";

    private static VoiceWakeupManagerProxy sInstance = null;
    private static Class<?> mVoiceWakeupClsObj = null;
    private static Object mVoiceWakeupInstance = null;

    public VoiceWakeupManagerProxy() {
        Log.d(TAG, "constructor is called.");
    }

    public static VoiceWakeupManagerProxy getInstance() {
        Log.d(TAG, "getInstance(...) is called.");
        if (sInstance == null) {
            Log.d(TAG, "getInstance(...) create one.");
            sInstance = new VoiceWakeupManagerProxy();
            createVoiceWakeupManagerInstance();
        }
        return sInstance;
    }

    private static void createVoiceWakeupManagerInstance() {
        try {
            mVoiceWakeupClsObj = Class.forName(sVOICE_WAKEUP_MANAGER_PACKAGE_PATH);
            if (mVoiceWakeupClsObj != null) {
                Method vowInstance = mVoiceWakeupClsObj.getDeclaredMethod("getInstance");
                mVoiceWakeupInstance = (Object) vowInstance.invoke(mVoiceWakeupClsObj);
            }
        } catch (Exception e) {
            Log.e(TAG, "createVoiceWakeupManagerInstance error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public boolean isDismissAndLaunchApp() {
        boolean result = false;
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod(
                        "isDismissAndLaunchApp");
                result = (Boolean) method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect isDismissAndLaunchApp error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
        return result;
    }

    public boolean onDismiss() {
        boolean result = false;
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("onDismiss");
                result = (Boolean) method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect onDismiss error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
        return result;
    }

    public void notifySecurityModeChange(KeyguardSecurityModel.SecurityMode currentMode,
            KeyguardSecurityModel.SecurityMode nextMode) {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("notifySecurityModeChange",
                        KeyguardSecurityModel.SecurityMode.class,
                        KeyguardSecurityModel.SecurityMode.class);
                method.invoke(mVoiceWakeupInstance, currentMode, nextMode);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect notifySecurityModeChange error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void notifyKeyguardIsGone() {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("notifyKeyguardIsGone");
                method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect notifyKeyguardIsGone error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void init(Context context, ViewMediatorCallback viewMediatorCallback) {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("init",
                        Context.class, ViewMediatorCallback.class);
                method.invoke(mVoiceWakeupInstance, context, viewMediatorCallback);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect init error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void handleVoiceCommandNotified(Bundle data, boolean calledFromTest) {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("handleVoiceCommandNotified",
                        Bundle.class, Boolean.class);
                method.invoke(mVoiceWakeupInstance, data, calledFromTest);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect handleVoiceCommandNotified error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public boolean isRegisted() {
        boolean result = false;
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("isRegisted",
                        Bundle.class, Boolean.class);
                result = (Boolean) method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect isRegisted error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
        return result;
    }

    public void setPackageName(String pkgName) {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("setPackageName",
                        String.class);
                method.invoke(mVoiceWakeupInstance, pkgName);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect setPackageName error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void start() {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getMethod("start");
                method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect start error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void stop() {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getMethod("stop");
                method.invoke(mVoiceWakeupInstance);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect stop error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }

    public void sendVoiceCommand(String pkgName, int mainAction, int subAction,
            Bundle extraData) {
        try {
            if (mVoiceWakeupInstance != null) {
                Method method = mVoiceWakeupClsObj.getDeclaredMethod("sendVoiceCommand",
                        String.class, Integer.class, Integer.class, Bundle.class);
                method.invoke(mVoiceWakeupInstance, pkgName, mainAction, subAction, extraData);
            }
        } catch (Exception e) {
            Log.e(TAG, "reflect sendVoiceCommand error");
            Log.e(TAG, Log.getStackTraceString(e));
        }
    }
}
