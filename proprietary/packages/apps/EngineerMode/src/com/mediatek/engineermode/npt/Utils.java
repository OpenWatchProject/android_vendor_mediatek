package com.mediatek.engineermode.npt;

import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;
import android.os.PowerManager;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.ShellExe;

import java.io.IOException;

/**
 * Common functions.
 */
public class Utils {
    public static final int RETURN_FAIL = -1;
    public static final int RETURN_SUCCESS = 0;
    private static final String TAG = "NPT/Utils";
    public static MediaPlayer mMediaPlayer = null;
    private static PowerManager.WakeLock mWakeLock = null;
    private static PowerManager pm = null;

    public static void initMediaPlayer(Context context) {
        mMediaPlayer = new MediaPlayer();
        try {
            AssetFileDescriptor fd = context.getAssets().openFd("audio.ogg");
            mMediaPlayer.setDataSource(fd);
            mMediaPlayer.prepare();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public static void releaseMediaPlayer() {
        mMediaPlayer.stop();
        mMediaPlayer.release();
        mMediaPlayer = null;
    }

    public static void playMediaPlayer() {
        if (!mMediaPlayer.isPlaying()) {
            mMediaPlayer.start();
        }
    }

    public static void stopMediaPlayer() {
        if (mMediaPlayer.isPlaying()) {
            mMediaPlayer.pause();
        }
    }

    public static int execCmd(String cmd, boolean execOnLocal) {
        int result = 0;
        Elog.i(TAG, "exec " + cmd);
        try {
            result = ShellExe.execCommand(cmd, execOnLocal);
        } catch (IOException e) {
            e.printStackTrace();
        }
        return result;
    }

    public static String getCmdOut() {
        Elog.v(TAG, "getCmdOut: ");
        String outStr = ShellExe.getOutput();
        Elog.d(TAG, "[output]: " + outStr);
        return outStr;
    }

    public static void powerInit(Context context) {
        pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = ((PowerManager) context.getSystemService(Context.POWER_SERVICE)).newWakeLock(
                PowerManager.PARTIAL_WAKE_LOCK, TAG);
    }

    public static void switchScreanOnOrOff(boolean isScreanOn) {
        boolean ifOpen = pm.isScreenOn();
        Elog.i(TAG, "ifOpen = " + ifOpen);
        Elog.i(TAG, "switchScreanOnOrOff = " + isScreanOn);
        if (isScreanOn) {
            if (!ifOpen) {
                execCmd("input keyevent 26", true);//turn on screan
                //execCmd("input keyevent 82", true);//unlock screan
                // holdonWakelock(true);
            }
        } else {
            if (ifOpen) {
                execCmd("input keyevent 26", true); //turn off screan
            }
        }
    }

    public static void holdonWakelock(boolean holdon) {
        if (holdon == true) {
            if (!mWakeLock.isHeld()) {
                mWakeLock.acquire();
                Elog.i(TAG, "mWakeLock.acquire");
            }
        } else {
            if (mWakeLock.isHeld()) {
                mWakeLock.release();
                Elog.i(TAG, " mWakeLock.release");
            }
        }
    }

}
