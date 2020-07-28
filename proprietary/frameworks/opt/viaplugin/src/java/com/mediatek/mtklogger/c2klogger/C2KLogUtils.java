package com.mediatek.mtklogger.c2klogger;

import android.app.ActivityManager;
import android.app.ActivityManager.RunningServiceInfo;
import android.content.Context;
import android.content.Intent;
import android.os.UserHandle;
import android.util.Log;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;

/**
 * @author MTK81255
 *
 */
public class C2KLogUtils {

    static public final String TAG_APP = "saber";

    public static final String C2KLOG_CUSTOMIZE_CONFIG_FOLDER = "c2klog_config";
    public static final String C2KLOG_CUSTOMIZE_CONFIG_FILE = "c2klog-config.prop";
    public static final String DEFAULT_CONIFG_PATH = "/mnt/sdcard/"
            + C2KLOG_CUSTOMIZE_CONFIG_FOLDER + "/";

    public static final String LOG_PATH_TYPE_INTERNAL_SD_KEY = "InternalLogPath";
    public static final String LOG_PATH_TYPE_EXTERNAL_SD_KEY = "ExternalLogPath";
    public static final String CONFIG_FILE_NAME = "log_settings";
    public static final String C2K_MODEM_LOG_PATH = "c2kmdlog";
    public static final String KEY_C2K_MODEM_LOGGING_PATH = "c2k_modem_logging_path";
    public static final String LOG_TREE_FILE = "file_tree.txt";

    public static final String C2KLOG_RECYCLE_CONFIG_FILE = "recycle_file_tree.txt";
    /*
     * com.mediatek.mtklogger.c2klogger.devicepath = /dev/ttySDIO1
     * com.mediatek.mtklogger.c2klogger.perlogsize = 6
     * com.mediatek.mtklogger.c2klogger.filterfile =
     * ets_1x_do_hybrid_default_v5.bcfg
     */
    public static final String CONIFG_PATH = "com.mediatek.mtklogger.c2klogger.configpath";
    public static final String CONIFG_DEVICEPATH = "com.mediatek.mtklogger.c2klogger.devicepath";
    public static final String CONIFG_PERLOGSIZE = "com.mediatek.mtklogger.c2klogger.perlogsize";
    public static final String CONIFG_TOTALLOGSIZE =
            "com.mediatek.mtklogger.c2klogger.totallogsize";
    public static final String CONIFG_FILTERFILE = "com.mediatek.mtklogger.c2klogger.filterfile";
    public static final String DEFAULT_CONIFG_DEVICEPATH = "/dev/ttySDIO1";
    public static final int DEFAULT_CONIFG_PERLOGSIZE = 6;
    public static final int DEFAULT_CONIFG_TOTALLOGSIZE = 3 * 1024;
    public static final String DEFAULT_CONIFG_FILTERFILE = "ets_1x_do_hybrid_default_v5.bcfg";

    public static final String ACTION_VIA_SET_ETS_DEV = "via.cdma.action.set.ets.dev.c2klogger";
    public static final String EXTRAL_VIA_ETS_DEV = "via.cdma.extral.ets.dev";
    public static final String EXTRAL_VIA_ETS_DEV_RESULT = "set.ets.dev.result";

    public int sWaitCbpS = 10; // wait to open the cbp in seconds afater cbp
                               // ready message reached

    /**
     * Add for multi user.
     */
    public static final int USER_ID_OWNER = 0;
    public static final int USER_ID_UNDEFINED = -1;
    public static final int USER_ID = UserHandle.myUserId();
    /**
     * @return false if is Guest or new user.
     */
    public static boolean isDeviceOwner() {
        return USER_ID == USER_ID_OWNER || USER_ID == USER_ID_UNDEFINED;
    }

    /**
     * @param context Context
     * @param intent Intent
     */
    public static void sendBroadCast(Context context, Intent intent) {
        if (isDeviceOwner() && context != null) {
            context.sendBroadcast(intent);
        } else {
            Log.w("saber", "Is not device owner, no permission to send broadcast!");
        }
    }

    /**
     * @param context Context
     * @param serviceClassName String
     * @return boolean
     */
    public static boolean isServiceRunning(Context context, String serviceClassName) {
        ActivityManager manager = (ActivityManager) context
                .getSystemService(Context.ACTIVITY_SERVICE);
        for (RunningServiceInfo service : manager.getRunningServices(Integer.MAX_VALUE)) {
            if (serviceClassName.equals(service.service.getClassName())) {
                return true;
            }
        }
        return false;
    }

    /**
     * @param fileTreePath String
     * @param logPath String
     */
    public static void writerToFileTree(String fileTreePath, String logPath) {
        Log.i("saber", "writerToFileTree() logPath = " + logPath);
        if (!new File(logPath).exists()) {
            Log.w("saber", "The logPath:" + logPath + " is not exists.");
            return;
        }
        File treeFile = new File(fileTreePath);
        try {
            FileWriter fw = new FileWriter(treeFile, true);
            BufferedWriter bw = new BufferedWriter(fw);
            bw.write(logPath + "\r\n");
            bw.close();
            fw.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

}