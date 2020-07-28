package com.mediatek.op18.mms;

import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.R;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.widget.Toast;

public class PromptReceiver extends BroadcastReceiver {

    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/PromptReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub

        MyLogger.logI(CLASS_TAG, "intent is " + intent);

        if (intent != null) {
            String action = intent.getAction();
            if (action != null && action.equals("com.mediatek.mms.plugin.sd_removed_or_full")) {

                MyLogger.logI(CLASS_TAG, "show toast SD removed or full");
                Toast.makeText(context, R.string.toast_sd_removed_or_full, Toast.LENGTH_SHORT)
                        .show();
            }
        }
    }

}
