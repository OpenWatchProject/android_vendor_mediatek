package com.mediatek.rcse.receiver;

import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;


import com.mediatek.rcse.activities.RegistrationStatusActivity;
import com.mediatek.rcse.api.Logger;

/**
 * @author MTK33296 This Class used to receive broadcast when registration is
 *         failed first time on wifi.
 */
public class RegistrationStatusReceiver extends BroadcastReceiver {

    //private Logger mLogger = Logger.getLogger(this.getClass().getName());
    public static final String TAG = "RegistrationStatusReceiver";
    @Override
    public void onReceive(Context context, Intent intent) {
        if (Logger.isActivated()) {
            Logger.d(TAG,"Registration before config Broadcast Receievd");
        }
        try {
            Intent intentStatus = new Intent(context,
                    RegistrationStatusActivity.class);
            intentStatus.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            context.startActivity(intentStatus);
        } catch (ActivityNotFoundException e) {
            e.printStackTrace();
            Logger.d(TAG,e.getMessage());
        }
    }
}
