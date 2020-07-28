package com.mediatek.mtklogger.c2klogger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

/**
 * @author MTK81255
 *
 */
public class OpenReceiver extends BroadcastReceiver {

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.v("saber", "I am open receiver in saber");

        if (intent.getAction().equals("android.provider.Telephony.SECRET_CODE")) {
            Log.i("saber", "User call saber");
            boolean isServiceRunning = C2KLogUtils.isServiceRunning(context,
                    C2KLogService.class.getName());
            if (isServiceRunning) {
                return;
            }
            Intent mIntentStartSaber = new Intent(context, C2KLogService.class);
            // intent_start_saber.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
            mIntentStartSaber.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(mIntentStartSaber);
        }
    }

}
