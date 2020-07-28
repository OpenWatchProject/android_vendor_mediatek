package com.mediatek.galleryportable;

import android.media.MediaPlayer;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class MediaPlayerUtils {

    private static final String TAG = "VP_MediaPlayerUtils";

    private static boolean sHasChecked = false;
    private static boolean sHasSetParameterFunction = false;
    private static Method sMethodII = null;
    private static Method sMethodIS = null;
    private static Method sMethodI = null;

    private static boolean hasSetGetParameterFunction() {
        if (!sHasChecked) {
            try {
                sMethodII = MediaPlayer.class.getDeclaredMethod(
                        "setParameter", int.class, int.class);
                sMethodIS = MediaPlayer.class.getDeclaredMethod(
                        "setParameter", int.class, String.class);
                sMethodI = MediaPlayer.class.getDeclaredMethod(
                        "getStringParameter", int.class);
                sHasSetParameterFunction = (sMethodII != null) && (sMethodIS != null)
                        && (sMethodI != null);
            } catch (NoSuchMethodException e) {
                Log.e(TAG, "MediaPlayer#setParameter() or getStringParameter() is not found");
                sHasSetParameterFunction = false;
            }
            sHasChecked = true;
            Log.d(TAG, "hasSetGetParameterFunction = " + sHasSetParameterFunction);
        }
        return sHasSetParameterFunction;
    }

    public static void setParameter(MediaPlayer mp, int key, int value) {
        if (hasSetGetParameterFunction() && mp != null) {
            try {
                sMethodII.invoke(mp, key, value);
            } catch (IllegalAccessException e) {
                android.util.Log.e(TAG, "setParameter", e);
            } catch (InvocationTargetException e) {
                android.util.Log.e(TAG, "setParameter", e);
            }

        }
    }

    public static void setParameter(MediaPlayer mp, int key, String value) {
        if (hasSetGetParameterFunction() && mp != null) {
            try {
                sMethodIS.invoke(mp, key, value);
            } catch (IllegalAccessException e) {
                android.util.Log.e(TAG, "setParameter", e);
            } catch (InvocationTargetException e) {
                android.util.Log.e(TAG, "setParameter", e);
            }
        }
    }

    public static String getStringParameter(MediaPlayer mp, int key) {
        if (hasSetGetParameterFunction() && mp != null) {
            try {
                return (String)sMethodI.invoke(mp, key);
            } catch (IllegalAccessException e) {
                android.util.Log.e(TAG, "getStringParameter", e);
            } catch (InvocationTargetException e) {
                android.util.Log.e(TAG, "getStringParameter", e);
            }
        }
        return null;
    }
}
