package com.mediatek.usbchecker;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

public class UsbCheckerService extends Service{

    private static final String TAG = "UsbChecker.Service";

    public static final String SIM_INTENT = "android.intent.action.SIM_STATE_CHANGED";
    public static final String USB_INTENT = "android.hardware.usb.action.USB_STATE";

    public static final String PROPERTY_FEATURE_SUPPORT = "ro.vendor.mtk_usb_cba_support";
    public static final String PROPERTY_USB_ACTIVATION = "persist.vendor.sys.usb.activation";
    public static final String PROPERTY_USB_CHARGING = "sys.usb.config";

    private static final String CHARGING_YES = "charging_yes";
    private static final String CHARGING_NO = "charging_no";

    // whether plugged in with USB cable
    private boolean mUsbPlugin = false;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        if (getActivateState()) {
            Log.d(TAG, "onStartCommand() already activation, kill self.");
            killProcess();
            return START_NOT_STICKY;
        }

        if (intent == null) {
            Log.d(TAG, "onStartCommand() null intent, ignore");
            return START_NOT_STICKY;
        }

        String action = intent.getAction();
        Log.d(TAG, "startService with intent:" + action);

        if (SIM_INTENT.equals(action)) {

            String state = intent.getStringExtra("ss");
            int slotId = intent.getIntExtra("slot", -1);
            Log.d(TAG, "Sim Intent extra. state=" + state + ". slotId=" + slotId);
            if (slotId == -1 || state.equals("NOT_READY")) {
                //do nothing.
                Log.d(TAG, "Do noting");
            } else if (!state.equals("ABSENT") && !state.equals("UNKNOWN")) {
                if (true == mUsbPlugin) {
                    setActivateState();
                    enableUsb();
                    finishAlertActivity();
                }
            }
        } else if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
            int plugType = intent.getIntExtra("plugged", 0);
            Log.d(TAG, "Usb Intent extra. plugType=" + plugType);

            boolean isPluggin = plugType == 0 ? false : true;

            if (isPluggin == mUsbPlugin) {
                //Do nothing for USB plug in state not change.
            } else if (0 == plugType) {
                //close activity.
                mUsbPlugin = false;
                finishAlertActivity();
            } else if (2 == plugType) {
                mUsbPlugin = true;

                if (hasIccCard()) {
                    setActivateState();
                    enableUsb();
                } else {
                    startAlertActivity();
                    disableUsb();
                }
            } else {
                Log.i(TAG, "Not connected to PC.");
            }
        } else if (USB_INTENT.equals(action)) {
            if (isUsbEnabled()) {
                disableUsb();
            }
        } else if (UsbCheckerConstants.INTENT_ENGINEER_ACTIVATE.equals(action)) {
            String hint = "Activate device for engineer.";
            Log.i(TAG, hint);
            Toast.makeText(this, hint, Toast.LENGTH_SHORT).show();
            setActivateState();
            enableUsb();
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate()");

        BroadcastReceiver mPluginFilterReceiver = new UsbCheckerReceiver();

        IntentFilter pluginFilter = new IntentFilter();
        pluginFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(mPluginFilterReceiver, pluginFilter);

    }

    /**
    * Activate when pluged in and also has sim
    */
    private void setActivateState() {
        SystemProperties.set(PROPERTY_USB_ACTIVATION, "yes");
        Intent intent = new Intent(UsbCheckerConstants.INTENT_USB_ACTIVATION);
        sendBroadcast(intent);
        Log.d(TAG, "setActivateState() yes and sendBroadCast:" + UsbCheckerConstants.INTENT_USB_ACTIVATION);
    }

    /**
     * PROPERTY_USB_ACTIVATION: whether has sim when plugged in with USB cable before.
     * Property will be reset if factory reset or reinstall system.
     * @return yes - has sim once; no - never has
     */
    static public boolean getActivateState() {
        String activate = SystemProperties.get(PROPERTY_USB_ACTIVATION, "no");
        Log.d(TAG, "getActivateState=" + activate);
        if (activate.equals("yes")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Set property charging_no to enable communication
     */
    private void enableUsb() {
        Log.d(TAG, "enableUsb()");
        SystemProperties.set(PROPERTY_USB_CHARGING, CHARGING_NO);
    }

    /**
     * Set property charging_yes to disable communication
     */
    private void disableUsb() {
        Log.d(TAG, "disableUsb()");
        SystemProperties.set(PROPERTY_USB_CHARGING, CHARGING_YES);
    }

    public static boolean isUsbEnabled() {
        String ret = SystemProperties.get(PROPERTY_USB_CHARGING, CHARGING_NO);
        Log.d(TAG, "isUsbEnabled()=" + ret);
        if (ret.equals(CHARGING_NO)) {
            return true;
        } else {
            return false;
        }
    }

    public static void killProcess() {
        Log.i(TAG, "Kill process via System.exit(0)");
        System.exit(0);
    }

    public static boolean isFeatureSupported() {
        int property = SystemProperties.getInt(PROPERTY_FEATURE_SUPPORT, -1);
        if (property != 1) {
            Log.i(TAG, "[isFeatureSupported] not support");
            return false;
        }
        return true;
    }

    private void startAlertActivity() {
        Log.d(TAG, "startAlertActivity()");
        Intent activityIntent  = new Intent();
        activityIntent.setClassName("com.mediatek.usbchecker",
            "com.mediatek.usbchecker.UsbCheckerActivity");
        activityIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(activityIntent);
    }

    private void finishAlertActivity() {
        Log.d(TAG, "finishAlertActivity()");
        Intent intent = new Intent(UsbCheckerConstants.INTENT_USB_CHECKER_FINISH);
        sendBroadcast(intent);
    }

    private boolean hasIccCard() {
        TelephonyManager telephonyManager = TelephonyManager.getDefault();
        if (telephonyManager == null) {
            Log.d(TAG, "TelephonyManagerEx is null");
            return false;
        }

        int simNumber = telephonyManager.getSimCount();
        for (int i = 0 ; i <  simNumber ; i++) {
            if (telephonyManager.hasIccCard(i)) {
                Log.d(TAG, "Slot(" + i + ") has iccCard");
                return true;
            }
        }
        Log.d(TAG, "No card plug ined: " + simNumber);
        return false;
    }
}
