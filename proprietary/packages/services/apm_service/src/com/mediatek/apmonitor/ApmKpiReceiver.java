package com.mediatek.apmonitor;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

public class ApmKpiReceiver extends BroadcastReceiver {
    private static final String TAG = ApmService.TAG;

    private static Handler mHandler = null;
    private ApmService mService = null;

    private ApmKpiReceiver() {}

    public ApmKpiReceiver(ApmService service, Handler handler) {
        super();
        mHandler = handler;
        mService = service;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        switch (action) {
            case IApmKpiMonitor.ACTION_APM_INIT_EVENT:
                if (!mService.mApmSession.tryCreateApmSession(false)) {
                    Log.e(TAG, "Init APM failed!");
                } else {
                    Log.d(TAG, "Init APM success!");
                }
                break;
            case IApmKpiMonitor.ACTION_APM_KPI_EVENT:
                Message msg = mHandler.obtainMessage(KpiMonitor.MSG_ID_APM_KPI_BROADCAST, intent);
                mHandler.sendMessage(msg);
                break;
            case CarrierExpressManager.ACTION_OPERATOR_CONFIG_CHANGED:
                // Do nothing now
                String op = intent.getStringExtra("operator");
                Log.d(TAG, "Switch to " + op);
                break;
            default:
                break;
        }
    }
}
