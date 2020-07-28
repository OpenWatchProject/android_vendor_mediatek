package com.mediatek.op12.settings;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsManager;


public class RoamingReceiverService extends Service {
    private static final String TAG = "RoamingReceiverService";
    private static final int PREFER_OTHER_CELLULAR_NETWORK = 1;
    private Context mContext;


    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int roamingMode = Settings.Global.getInt(getContentResolver(),
                "ROAMING_MODE", PREFER_OTHER_CELLULAR_NETWORK);

        TelephonyManager telephonyManager = (TelephonyManager)
                            getSystemService(Context.TELEPHONY_SERVICE);
        if (telephonyManager.isNetworkRoaming()) {
             Log.d(TAG, "PLMN changed, Phone is in Roaming");
             if (roamingMode == 1) {
                 ImsManager.setWfcMode(mContext,
                       ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED);
                 Log.d(TAG, "setWfcMode: Cellular Preferred");
             } else if (roamingMode == 0) {
                 ImsManager.setWfcMode(mContext,
                         ImsConfig.WfcModeFeatureValueConstants.WIFI_PREFERRED);
                 Log.d(TAG, "setWfcMode: WiFi Preferred");
             }
        } else {
             Log.d(TAG, "PLMN changed, Phone is in Local Network");
            ImsManager.setWfcMode(mContext,
                    ImsConfig.WfcModeFeatureValueConstants.CELLULAR_PREFERRED);
        }
        return Service.START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        // TODO Auto-generated method stub
        return null;
    }
}
