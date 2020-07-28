/*
 * Copyright (C) 2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.fmradio;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemProperties;
import android.os.storage.StorageManager;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.View.MeasureSpec;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.io.File;
import java.util.Locale;

/**
 * This class provider interface to compute station and frequency, get project
 * string
 */
public class FmUtils {
    private static final String TAG = "FmUtils";

    // FM station variables
    public static final int DEFAULT_STATION = 1000;
    public static final int DEFAULT_STATION_50KHZ = 10000;
    public static final float DEFAULT_STATION_FLOAT = computeFrequency(DEFAULT_STATION);
    // maximum station frequency
    private static final int HIGHEST_STATION = 1080;
    // minimum station frequency
    private static final int LOWEST_STATION = 875;
    // maximum station frequency 50khz
    private static final int HIGHEST_STATION_50KHZ = 10800;
    // minimum station frequency 50khz
    private static final int LOWEST_STATION_50KHZ = 8750;
    // station step
    private static final int STEP = 1;
    // station step 50khz
    private static final int STEP_50KHZ = 5;
    // convert rate
    private static final int CONVERT_RATE = 10;
    private static final int CONVERT_RATE_50KHZ = 100;

    // minimum storage space for record (512KB).
    // Need to check before starting recording and during recording to avoid
    // recording keeps going but there is no free space in sdcard.
    public static final long LOW_SPACE_THRESHOLD = 512 * 1024;

    private static final String FM_IS_FIRST_TIME_PLAY = "fm_is_first_time_play";
    private static final String FM_IS_SPEAKER_MODE = "fm_is_speaker_mode";
    private static final String FM_IS_FIRST_ENTER_STATION_LIST = "fm_is_first_enter_station_list";
    // StorageManager For FM record
    private static StorageManager sStorageManager = null;

    // FM suspend feature
    private static Boolean sIsFmSuspendEnabled = null;

    /**
     * Whether the frequency is valid.
     *
     * @param station The FM station
     *
     * @return true if the frequency is in the valid scale, otherwise return
     *         false
     */
    public static boolean isValidStation(int station) {
        if (is50KhzSupportOn()) {
            return (station >= LOWEST_STATION_50KHZ
                    && station <= HIGHEST_STATION_50KHZ);
        } else {
            return (station >= LOWEST_STATION && station <= HIGHEST_STATION);
        }
    }

    /**
     * Compute increase station frequency
     *
     * @param station The station frequency
     *
     * @return station The frequency after increased
     */
    public static int computeIncreaseStation(int station) {
        int result;
        if (is50KhzSupportOn()) {
            result = station + STEP_50KHZ;
            if (result > HIGHEST_STATION_50KHZ) {
                result = LOWEST_STATION_50KHZ;
            }
        } else {
            result = station + STEP;
            if (result > HIGHEST_STATION) {
                result = LOWEST_STATION;
            }
        }
        return result;
    }

    /**
     * Compute decrease station frequency
     *
     * @param station The station frequency
     *
     * @return station The frequency after decreased
     */
    public static int computeDecreaseStation(int station) {
        int result;
        if (is50KhzSupportOn()) {
            result = station - STEP_50KHZ;
            if (result < LOWEST_STATION_50KHZ) {
                result = HIGHEST_STATION_50KHZ;
            }
        } else {
            result = station - STEP;
            if (result < LOWEST_STATION) {
                result = HIGHEST_STATION;
            }
        }
        return result;
    }

    /**
     * Compute station value with given frequency
     *
     * @param frequency The station frequency
     *
     * @return station The result value
     */
    public static int computeStation(float frequency) {
        if (is50KhzSupportOn()) {
            return (int) (frequency * CONVERT_RATE_50KHZ);
        } else {
            return (int) (frequency * CONVERT_RATE);
        }
    }

    /**
     * Compute frequency value with given station
     *
     * @param station The station value
     *
     * @return station The frequency
     */
    public static float computeFrequency(int station) {
        if (is50KhzSupportOn()) {
            return (float) station / CONVERT_RATE_50KHZ;
        } else {
            return (float) station / CONVERT_RATE;
        }
    }

    /**
     * According station to get frequency string
     *
     * @param station for 100KZ, range 875-1080
     *
     * @return string like 87.5
     */
    public static String formatStation(int station) {
        float frequency;
        String result;

        if (is50KhzSupportOn()) {
            frequency = (float) station / CONVERT_RATE_50KHZ;
            result = String.format(Locale.ENGLISH, "%.2f",
                Float.valueOf(frequency));
        } else {
            frequency = (float) station / CONVERT_RATE;
            result = String.format(Locale.ENGLISH, "%.1f",
                Float.valueOf(frequency));
        }

        return result;
    }

    /**
     * Get the phone storage path
     *
     * @return The phone storage path
     */
    public static String getDefaultStoragePath() {
        return Environment.getExternalStorageDirectory().getPath();
    }

    /**
     * Get the default storage state
     *
     * @return The default storage state
     */
    public static String getDefaultStorageState(Context context) {
        ensureStorageManager(context);
        String state = sStorageManager.getVolumeState(getDefaultStoragePath());
        return state;
    }

    private static void ensureStorageManager(Context context) {
        if (sStorageManager == null) {
            sStorageManager = (StorageManager) context.getSystemService(Context.STORAGE_SERVICE);
        }
    }

    /**
     * Get the FM play list path
     *
     * @param context The context
     *
     * @return The FM play list path
     */
    public static String getPlaylistPath(Context context) {
        ensureStorageManager(context);
        String[] externalStoragePaths = sStorageManager.getVolumePaths();
        String path = externalStoragePaths[0] + "/Playlists/";
        return path;
    }

    /**
     * Check if has enough space for record
     *
     * @param recordingSdcard The recording sdcard path
     *
     * @return true if has enough space for record
     */
    public static boolean hasEnoughSpace(String recordingSdcard) {
        boolean ret = false;
        try {
            StatFs fs = new StatFs(recordingSdcard);
            long blocks = fs.getAvailableBlocks();
            long blockSize = fs.getBlockSize();
            long spaceLeft = blocks * blockSize;
            ret = spaceLeft > LOW_SPACE_THRESHOLD ? true : false;
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "hasEnoughSpace, sdcard may be unmounted:" + recordingSdcard);
        }
        return ret;
    }

    /**
     * check it is the first time to use Fm
     */
    public static boolean isFirstTimePlayFm(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean isFirstTime = prefs.getBoolean(FM_IS_FIRST_TIME_PLAY, true);
        return isFirstTime;
    }

    /**
     * Called when first time play FM.
     * @param context The context
     */
    public static void setIsFirstTimePlayFm(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(FM_IS_FIRST_TIME_PLAY, false);
        editor.commit();
    }

    /**
     * check it is the first time enter into station list page
     */
    public static boolean isFirstEnterStationList(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        boolean isFirstEnter = prefs.getBoolean(FM_IS_FIRST_ENTER_STATION_LIST, true);
        if (isFirstEnter) {
            SharedPreferences.Editor editor = prefs.edit();
            editor.putBoolean(FM_IS_FIRST_ENTER_STATION_LIST, false);
            editor.commit();
        }
        return isFirstEnter;
    }

    /**
     * Create the notification large icon bitmap from layout
     * @param c The context
     * @param text The frequency text
     * @return The large icon bitmap with frequency text
     */
    public static Bitmap createNotificationLargeIcon(Context c, String text) {
        Resources res = c.getResources();
        int width = (int) res.getDimension(android.R.dimen.notification_large_icon_width);
        int height = (int) res.getDimension(android.R.dimen.notification_large_icon_height);
        LinearLayout iconLayout = new LinearLayout(c);
        iconLayout.setOrientation(LinearLayout.VERTICAL);
        iconLayout.setBackgroundColor(c.getResources().getColor(R.color.theme_primary_color));
        iconLayout.setDrawingCacheEnabled(true);
        iconLayout.layout(0, 0, width, height);
        TextView iconText = new TextView(c);
        boolean isTablet = false;
        isTablet = res.getBoolean(R.bool.is_tablet);
        if(true == isTablet){
            iconText.setTextSize(13.0f);
        }else{
            iconText.setTextSize(18.0f);
        }
        iconText.setTextColor(res.getColor(R.color.theme_title_color));
        iconText.setText(text);
        iconText.measure(MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED),
                MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED));
        int left = (int) ((width - iconText.getMeasuredWidth()) * 0.5);
        int top = (int) ((height - iconText.getMeasuredHeight()) * 0.5);
        iconText.layout(left, top, iconText.getMeasuredWidth() + left,
                iconText.getMeasuredHeight() + top);
        iconLayout.addView(iconText);
        iconLayout.layout(0, 0, width, height);

        iconLayout.buildDrawingCache();
        Bitmap largeIcon = Bitmap.createBitmap(iconLayout.getDrawingCache());
        iconLayout.destroyDrawingCache();
        return largeIcon;
    }

    /**
     * Check if support FM suspend feature. If true, will not aquire wakelock.
     * @return true if support, otherwise false
     */
    public static boolean isFmSuspendSupport() {
        if (sIsFmSuspendEnabled == null)
            sIsFmSuspendEnabled = true;
        return sIsFmSuspendEnabled.booleanValue();
    }

    /**
     *
     * @param millis to format time in milliseconds
     * @return formated time, eg "01:02:03" means 1 hour 2 minutes 3 seconds
     */
    public static String timeFormat(long millis) {
        millis = millis / 1000;
        long hour = millis / (60 * 60);
        millis %= (60 * 60);
        long min = millis / 60;
        long sec = millis % 60;
        StringBuilder timeText = new StringBuilder();
        timeText.append(addPaddingForString(hour)).append(":").
                append(addPaddingForString(min)).append(":").append(addPaddingForString(sec));
        return timeText.toString();
    }

    private static String addPaddingForString(long time) {
        StringBuilder builder = new StringBuilder();
        if (time >= 0 && time < 10) {
            builder.append("0");
        }
        return builder.append(time).toString();
    }

    /**
     * Get whether speaker mode is in use when audio focus lost.
     * @param context the Context
     * @return true for speaker mode, false for non speaker mode
     */
    public static boolean getIsSpeakerModeOnFocusLost(Context context) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);

        return prefs.getBoolean(FM_IS_SPEAKER_MODE, false);
    }

    /**
     * Set whether speaker mode is in use.
     * @param context the Context
     * @param isSpeaker speaker state
     */
    public static void setIsSpeakerModeOnFocusLost(Context context, boolean isSpeaker) {
        SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(FM_IS_SPEAKER_MODE, isSpeaker);
        editor.commit();
    }

    public static boolean is50KhzSupportOn() {
        boolean ret = SystemProperties.getBoolean("persist.vendor.connsys.fm_50khz_support", false);
        Log.d(TAG, "50Khz support: " + ret);
        return ret;
    }
}
