package com.mediatek.engineermode.dm;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

import com.mediatek.engineermode.Elog;

/**
 * Broadcast receiver to receive command to enable DM log.
 *
 */
public class DmEnableLogReceiver extends BroadcastReceiver {
    private static final String TAG = "DmEnableLogReceiver";
    private static final String ACTION_ENABLE_DM_LOG =
            "com.mediatek.engineermode.dm.ACTION_ENABLE_DM_LOG";
    private static final String EXTRA_TARGET = "target";
    private static final String EXTRA_ENABLED = "enabled";

    private static final String TARGET_ALL = "all";
    private static final String TARGET_DM = "dm";
    private static final String TARGET_PKM = "pkm";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (ACTION_ENABLE_DM_LOG.equals(intent.getAction())) {
            String target = intent.getStringExtra(EXTRA_TARGET);
            if (target == null) {
                target = "none";
            }
            boolean enabled = (intent.getIntExtra(EXTRA_ENABLED, 0) > 0) ? true : false;

            Elog.i(TAG, "receive ACTION_ENABLE_DM_LOG, target = " +
                    target + ", enabled = " + enabled);
            DmSettingController dmLogCtrl = new DmSettingController(context);

            switch (target) {
                case TARGET_ALL:
                    dmLogCtrl.enableDmLog(enabled);
                    dmLogCtrl.enablePkmLog(enabled);
                    break;
                case TARGET_DM:
                    dmLogCtrl.enableDmLog(enabled);
                    break;
                case TARGET_PKM:
                    dmLogCtrl.enablePkmLog(enabled);
                    break;
                default:
                    // Do nothing
                    break;
            }
        }
    }
}
