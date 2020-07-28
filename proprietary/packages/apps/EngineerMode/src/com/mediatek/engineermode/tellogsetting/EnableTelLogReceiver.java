package com.mediatek.engineermode.tellogsetting;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.engineermode.Elog;

/**
 * Broadcast receiver to receive command to enable telephony log.
 *
 */
public class EnableTelLogReceiver extends BroadcastReceiver {
    private static final String TAG = "telLogSetting";
    private static final String ACTION_ENABLE_TELLOG_START =
            "com.mediatek.engineermode.tellog.enable.start";
    private static final String ACTION_ENABLE_TELLOG_DONE =
            "com.mediatek.engineermode.tellog.enable.done";
    private static final String ENABLE_TELLOG_RESULT = "result";
    private static final String TAR_PKG = "com.mediatek.issuesubmitter";
    private static final String TAR_CLASS_NAME = "com.mediatek.issuesubmitter.MainActivity";
    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (ACTION_ENABLE_TELLOG_START.equals(intent.getAction())) {
            Elog.i(TAG, "receive ACTION_ENABLE_TELLOG_START");
            TelLogController telLogCtr = new TelLogController();
            boolean result = telLogCtr.switchTelLog(context, true);
            Intent intentDone = new Intent(ACTION_ENABLE_TELLOG_DONE);
            intentDone.setClassName(TAR_PKG, TAR_CLASS_NAME);
            intentDone.putExtra(ENABLE_TELLOG_RESULT, result);
            context.sendBroadcast(intentDone);
        }
    }

}
