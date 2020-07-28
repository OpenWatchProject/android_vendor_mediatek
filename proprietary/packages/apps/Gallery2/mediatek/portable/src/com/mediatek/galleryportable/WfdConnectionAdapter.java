package com.mediatek.galleryportable;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.display.DisplayManager;
import android.hardware.display.WifiDisplayStatus;
import android.os.SystemClock;
import android.provider.Settings;

public class WfdConnectionAdapter {
    private static final String TAG = "Gallery2/WfdConnectionAdapter";
    private static final boolean DEBUG = false;

    public interface StateChangeListener {
        void stateNotConnected();
        void stateConnected();
    }

    private static boolean sIsWfdApiExisted = false;
    private static boolean sHasChecked = false;
    private static String sSavingOption = null;
    private static String sSavingDelay = null;
    private StateChangeListener mStateChangeListener;
    private BroadcastReceiver mWfdReceiver;

    private static boolean isWfdApiExisted() {
        if (!sHasChecked) {
            long checkStart = SystemClock.elapsedRealtime();
            try {
                // check com.mediatek.provider.MtkSettingsExt$Global#WIFI_DISPLAY_POWER_SAVING_OPTION
                Class<?> clazz =
                        WfdConnectionAdapter.class.getClassLoader().loadClass(
                                "com.mediatek.provider.MtkSettingsExt$Global");
                if (clazz != null) {
                    clazz.getDeclaredField("WIFI_DISPLAY_POWER_SAVING_OPTION");
                    Object name = clazz.getField("WIFI_DISPLAY_POWER_SAVING_OPTION").get(null);
                    if (name != null) {
                        sSavingOption = (String)name;
                    }
                }
                if (clazz != null) {
                    clazz.getDeclaredField("WIFI_DISPLAY_POWER_SAVING_DELAY");
                    Object name = clazz.getField("WIFI_DISPLAY_POWER_SAVING_DELAY").get(null);
                    if (name != null) {
                        sSavingDelay = (String)name;
                    }
                }

                // check android.hardware.display.DisplayManager
                clazz =
                        WfdConnectionAdapter.class.getClassLoader().loadClass(
                                "android.hardware.display.DisplayManager");
                if (clazz != null) {
                    clazz.getDeclaredMethod("getWifiDisplayStatus");
                }
                if (clazz != null) {
                    clazz.getDeclaredField("ACTION_WIFI_DISPLAY_STATUS_CHANGED");
                }
                if (clazz != null) {
                    clazz.getDeclaredField("EXTRA_WIFI_DISPLAY_STATUS");
                }

                // check android.hardware.display.WifiDisplayStatus
                clazz =
                        WfdConnectionAdapter.class.getClassLoader().loadClass(
                                "android.hardware.display.WifiDisplayStatus");
                if (clazz != null) {
                    clazz.getDeclaredMethod("getActiveDisplayState");
                }
                if (clazz != null) {
                    clazz.getDeclaredField("DISPLAY_STATE_CONNECTED");
                }
                if (clazz != null) {
                    clazz.getDeclaredField("DISPLAY_STATE_NOT_CONNECTED");
                }
                sIsWfdApiExisted = true;
            } catch (ClassNotFoundException e) {
                sIsWfdApiExisted = false;
                Log.e(TAG, e.toString());
            } catch (NoSuchMethodException e) {
                sIsWfdApiExisted = false;
                Log.e(TAG, e.toString());
            } catch (NoSuchFieldException e) {
                sIsWfdApiExisted = false;
                Log.e(TAG, e.toString());
            } catch (IllegalAccessException e) {
                sIsWfdApiExisted = false;
                Log.e(TAG, e.toString());
            }
            sHasChecked = true;
            Log.d(TAG, "isWfdApiExisted, mIsWfdApiExisted = " + sIsWfdApiExisted);
            if (DEBUG) {
                Log.d(TAG, "isWfdApiExisted elapsed time = "
                        + (SystemClock.elapsedRealtime() - checkStart));
            }
        }
        return sIsWfdApiExisted;
    }

    public WfdConnectionAdapter(StateChangeListener stateChangeListener) {
        this.mStateChangeListener = stateChangeListener;
        if (isWfdApiExisted()) {
            mWfdReceiver = new WfdBroadcastReceiver();
        }
    }

    public static int getPowerSavingMode(Context context) {
        int mode = 0;
        if (isWfdApiExisted() && sSavingOption != null) {
            mode = Settings.Global.getInt(context.getContentResolver(),
                    sSavingOption, 0);
        }
        Log.v(TAG, "getPowerSavingMode() mode = " + mode);
        return mode;
    }

    public static int getPowerSavingDelay(Context context) {
        int delayTime = 0;
        if (isWfdApiExisted() && sSavingDelay != null) {
            delayTime = Settings.Global.getInt(context.getContentResolver(),
                    sSavingDelay, 0);
        }
        Log.v(TAG, "getDelayTime(): " + delayTime);
        return delayTime;
    }

    public void registerReceiver(Context context) {
        if (isWfdApiExisted() && mWfdReceiver != null) {
            Log.v(TAG, "registerReceiver");
            IntentFilter filter = new IntentFilter(
                    DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED);
            context.registerReceiver(mWfdReceiver, filter);
        }
    }

    public void unRegisterReceiver(Context context) {
        if (isWfdApiExisted() && mWfdReceiver != null) {
            Log.v(TAG, "unRegisterReceiver");
            try {
                context.unregisterReceiver(mWfdReceiver);
            } catch (IllegalArgumentException ex) {
                // Ignore this exeption, This is exactly what is desired
                ex.printStackTrace();
            }
        }
    }

    public static boolean isWfdSupported(Context context) {
        boolean connected = false;
        if (isWfdApiExisted()) {
            int activityDisplayState = -1;
            DisplayManager mDisplayManager = (DisplayManager) context
                    .getSystemService(Context.DISPLAY_SERVICE);
            WifiDisplayStatus mWfdStatus = mDisplayManager.getWifiDisplayStatus();
            activityDisplayState = mWfdStatus.getActiveDisplayState();
            connected = activityDisplayState == WifiDisplayStatus.DISPLAY_STATE_CONNECTED;
        }
        Log.d(TAG, "isWfdSupported(): " + connected);
        return connected;
    }

    class WfdBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(TAG, "WfdBroadcastReceiver onReceive action: " + action);
            if (action != null && action
                    .equals(DisplayManager.ACTION_WIFI_DISPLAY_STATUS_CHANGED)) {
                WifiDisplayStatus status = (WifiDisplayStatus) intent
                        .getParcelableExtra(DisplayManager.EXTRA_WIFI_DISPLAY_STATUS);
                if (status != null) {
                    int state = status.getActiveDisplayState();
                    Log.v(TAG, "WfdBroadcastReceiver onReceive wfd State: " + state);
                    if (state == WifiDisplayStatus.DISPLAY_STATE_NOT_CONNECTED) {
                        if (mStateChangeListener != null) {
                            mStateChangeListener.stateNotConnected();
                        }
                    } else if (state == WifiDisplayStatus.DISPLAY_STATE_CONNECTED) {
                        if (mStateChangeListener != null) {
                            mStateChangeListener.stateConnected();
                        }
                    }
                }
            }
        }
    }

}
