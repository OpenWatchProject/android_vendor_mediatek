package com.mediatek.engineermode.desenseat;

import android.bluetooth.BluetoothAdapter;
import android.content.Context;
import android.net.wifi.WifiManager;
import android.provider.Settings;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

/**
 * Class to control test condition.
 * @author mtk80357
 *
 */
class TestCondition {
    private static final String TAG = "DesenseAT/TestCondition";
    private boolean mWifiEnable;
    private boolean mBtEnable;
    private WifiManager mWifiManager = null;
    private BluetoothAdapter mBtAdapter = null;

    private static final int TIME_OFF_TEST = 2 * 3600 * 1000;
    private static final int TIME_OFF_DEFAULT = 60 * 1000;
    private boolean mAirplaneMode;
    private int mScreenOffTime = -1;;

    void init(Context context) {
        mWifiManager = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
    }

    void setCondition(Context context) {
        //Disable wifi
        mWifiEnable = mWifiManager.getWifiState() == WifiManager.WIFI_STATE_ENABLED;
        if (mWifiEnable) {
            Util.switchWifi(context, false);
        }
        //Disable BT
        mBtEnable = mBtAdapter.getState() == BluetoothAdapter.STATE_ON;
        if (mBtEnable) {
            Util.switchBt(false);
        }
        EmUtils.initPoweroffmdMode(true, true);
        //Enable airplanemode
        mAirplaneMode = Settings.System.getInt(context.getContentResolver(),
                Settings.Global.AIRPLANE_MODE_ON, 0) != 0;
        if (!mAirplaneMode) {
            Util.switchAirplaneMode(context, true);
        }

        //Keep screen on
        mScreenOffTime = Settings.System.getInt(context.getContentResolver(),
                Settings.System.SCREEN_OFF_TIMEOUT, TIME_OFF_DEFAULT);
        Elog.i(TAG, "default off:" + mScreenOffTime);
        Settings.System.putInt(context.getContentResolver(),
                               Settings.System.SCREEN_OFF_TIMEOUT, TIME_OFF_TEST);

    }

    void resetCondition(Context context) {
        Util.switchWifi(context, mWifiEnable);
        Util.switchBt(mBtEnable);
        Util.switchAirplaneMode(context, mAirplaneMode);
        EmUtils.initPoweroffmdMode(false, true);
        if (mScreenOffTime != -1) {
            Settings.System.putInt(context.getContentResolver(),
                                   Settings.System.SCREEN_OFF_TIMEOUT, mScreenOffTime);
        }
    }

}
