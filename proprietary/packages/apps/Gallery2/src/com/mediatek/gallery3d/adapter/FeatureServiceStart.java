package com.mediatek.gallery3d.adapter;

import android.app.ActivityManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.Environment;
import android.util.Log;

import com.mediatek.galleryportable.SystemPropertyUtils;

import java.io.File;

/**
 * Auto start FeatureService when device boot completed.
 * TODO: Maybe this is temp solution for initialize FeatureManager
 */
public class FeatureServiceStart extends BroadcastReceiver {
    private static final String TAG = "MtkGallery2/FeatureServiceStart";
    private static final String GMO_RAM = "ro.vendor.mtk_gmo_ram_optimize";
    private static final String DEBUG_STARTSERVICE = "debug.gallery.startservice";
    private static boolean START_SERVICE = (new File("data/data/com.android.gallery3d/",
            DEBUG_STARTSERVICE)).exists();
    private static boolean GMO_PROJECT = "1".equals(SystemPropertyUtils.get(GMO_RAM));

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        boolean isLowRamDevice = isLowRamDevice(context);
        Log.d(TAG, "<onReceive> IsLowRamDevice = " + isLowRamDevice + " GMO_PROJECT = " +
                GMO_PROJECT);
        if (START_SERVICE && action != null &&
            action.equals("android.intent.action.BOOT_COMPLETED")) {
            Log.i(TAG, "<onReceive> START_SERVICE action = " + action);
            Intent startServiceIntent = new Intent(context, FeatureService.class);
            context.startService(startServiceIntent);
        }
    }

    private static boolean isLowRamDevice(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            return ((ActivityManager)
                    context.getSystemService(Context.ACTIVITY_SERVICE)).isLowRamDevice();
        } else {
            return "true".equals(SystemPropertyUtils.get("ro.config.low_ram", "false"));
        }
    }

}