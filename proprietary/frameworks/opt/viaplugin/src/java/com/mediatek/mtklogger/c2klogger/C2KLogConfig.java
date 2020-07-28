package com.mediatek.mtklogger.c2klogger;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.SystemProperties;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 * @author MTK81255
 *
 */
public class C2KLogConfig {
    private static final String TAG = "saber" + "/C2KLogConfig";
    private File mConfigFile = null;
    private Context mContext = null;
    private SharedPreferences mSharedPreferences;
    /**
     * @param context Context
     */
    public C2KLogConfig(Context context) {
        mContext = context;
    }

    /**
     * Initiate log related configuration. If configure file(SharedPreference)
     * already, do nothing.
     */
    public void checkConfig() {
        mSharedPreferences = mContext.getSharedPreferences(C2KLogUtils.CONFIG_FILE_NAME,
                Context.MODE_PRIVATE);
        Log.d(TAG, "-->checkConfig()");
        if (checkConfigFile(C2KLogUtils.LOG_PATH_TYPE_INTERNAL_SD_KEY)
                || checkConfigFile(C2KLogUtils.LOG_PATH_TYPE_EXTERNAL_SD_KEY)) {
            initConfig();
        } else {
            initDefaultConfig();
        }
    }

    private boolean checkConfigFile(String sdType) {
        boolean isExists = true;
        String storagePath = mSharedPreferences.getString(sdType, "");
        mConfigFile = new File(storagePath + "/" + C2KLogUtils.C2KLOG_CUSTOMIZE_CONFIG_FOLDER + "/"
                + C2KLogUtils.C2KLOG_CUSTOMIZE_CONFIG_FILE);
        if (mConfigFile == null || !mConfigFile.exists()) {
            Log.i(TAG, "The configfile in flow path is not exist : " + mConfigFile);
            isExists = false;
        } else {
            Log.d(TAG, "The configfile in flow path is exist : " + mConfigFile);
            isExists = true;
        }
        return isExists;
    }

    /**
     * Shared preference have not be initialized yet, try to initialize it with
     * customer config file.
     */
    private void initConfig() {
        Log.d(TAG, "-->initConfig()");
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        Properties customizeProp = new Properties();
        FileInputStream customizeInputStream = null;
        try {
            customizeInputStream = new FileInputStream(mConfigFile);
            customizeProp.load(customizeInputStream);
//            String devPath = customizeProp.getProperty(C2KLogUtils.CONIFG_DEVICEPATH);
            String perLogsize = customizeProp.getProperty(C2KLogUtils.CONIFG_PERLOGSIZE);
            String filterFile = customizeProp.getProperty(C2KLogUtils.CONIFG_FILTERFILE);
            String totalLogsize = customizeProp.getProperty(C2KLogUtils.CONIFG_TOTALLOGSIZE);
            int perLogsizeInt = C2KLogUtils.DEFAULT_CONIFG_PERLOGSIZE;
            int totalLogsizeInt = C2KLogUtils.DEFAULT_CONIFG_TOTALLOGSIZE;
            if (perLogsize != null) {
                try {
                    perLogsizeInt = Integer.parseInt(perLogsize);
                } catch (NumberFormatException e) {
                    Log.w(TAG, "perLogsize " + perLogsize + " in config file is invalid");
                }
            }
            if (totalLogsize != null) {
                try {
                    totalLogsizeInt = Integer.parseInt(totalLogsize);
                } catch (NumberFormatException e) {
                    Log.w(TAG, "totalLogsize " + totalLogsize + " in config file is invalid");
                }
            }
            Log.d(TAG, "initConfig perLogsizeInt = " + perLogsizeInt
                    + "; totalLogsizeInt = " + totalLogsizeInt
                    + "; filterFile = " + filterFile);
            editor.putString(C2KLogUtils.CONIFG_PATH,
                    mConfigFile.getParentFile().getCanonicalPath())
                    .putString(C2KLogUtils.CONIFG_DEVICEPATH, getDevicePath())
                    .putInt(C2KLogUtils.CONIFG_PERLOGSIZE, perLogsizeInt)
                    .putInt(C2KLogUtils.CONIFG_TOTALLOGSIZE, totalLogsizeInt)
                    .putString(C2KLogUtils.CONIFG_FILTERFILE, filterFile);
            editor.apply();
        } catch (IOException e) {
            Log.e(TAG, "read customize config file error!" + e.toString());
            initDefaultConfig();
        } finally {
            if (customizeInputStream != null) {
                try {
                    customizeInputStream.close();
                } catch (IOException e2) {
                    Log.e(TAG, "Fail to close opened customization file.");
                }
            }
        }
    }

    /**
     * If found no configuration file in device, just initiate shared preference
     * with default value in code.
     */
    private void initDefaultConfig() {
        Log.w(TAG, "-->initDefaultConfig()");
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putString(C2KLogUtils.CONIFG_PATH, C2KLogUtils.DEFAULT_CONIFG_PATH)
                .putString(C2KLogUtils.CONIFG_DEVICEPATH, getDevicePath())
                .putInt(C2KLogUtils.CONIFG_PERLOGSIZE, C2KLogUtils.DEFAULT_CONIFG_PERLOGSIZE)
                .putString(C2KLogUtils.CONIFG_FILTERFILE, C2KLogUtils.DEFAULT_CONIFG_FILTERFILE);
        editor.apply();
    }

    /**
     * Demo : viatel.device.ets sdio.1.ttySDIO.
     * @return String
     */
    private String getDevicePath() {
        Log.d(TAG, "-->getDevicePath()");
        String devicePath = C2KLogUtils.DEFAULT_CONIFG_DEVICEPATH;
        String pathFromPro = SystemProperties.get("viatel.device.ets", "sdio.1.ttySDIO");
        Log.d(TAG, "getDevicePath() pathFromPro = " + pathFromPro);
        String[] paths = pathFromPro.split("\\.");
        if (paths.length == 3) {
            devicePath = "/dev/" + paths[2] + paths[1];
        }
        return devicePath;
    }
}
