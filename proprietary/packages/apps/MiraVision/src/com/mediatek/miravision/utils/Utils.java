package com.mediatek.miravision.utils;

import android.app.ActivityManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.BitmapFactory.Options;
import android.os.UserHandle;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.HashMap;

public class Utils {

    private static final String TAG = "Miravision/Utils";

    private static final String SHARED_PREFERENCES_NAME = "miravision_status";

    private Context mContext;

    public Utils(Context context) {
        mContext = context;
    }

    private SharedPreferences getSharedPreferences() {
        return mContext.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }

    public boolean getSharePrefBoolenValue(String key) {
        return getSharedPreferences().getBoolean(key, false);
    }

    public void setSharePrefBoolenValue(String key, boolean value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putBoolean(key, value);
        editor.commit();
    }

    public int getSharePrefIntValue(String key) {
        return getSharedPreferences().getInt(key, 0);
    }

    public void setSharePrefIntValue(String key, int value) {
        SharedPreferences.Editor editor = getSharedPreferences().edit();
        editor.putInt(key, value);
        editor.apply();
    }

    public static byte[] obtainByteArray(Resources res, int resId) {
        InputStream is = res.openRawResource(resId);
        ByteArrayOutputStream os = new ByteArrayOutputStream();
        byte[] buffer = new byte[1024];
        byte[] data = null;
        int readLength;
        try {
            while ((readLength = is.read(buffer)) != -1) {
                os.write(buffer, 0, readLength);
            }
            data = os.toByteArray();
            is.close();
            os.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return data;
    }

    public static String getIntentHiddenValues(String value) throws ClassNotFoundException,
            NoSuchFieldException, IllegalAccessException {
        Class cls = Intent.class;
        Field field = cls.getDeclaredField(value);
        field.setAccessible(true);
        return field.get(cls).toString();
    }

    public static int getActivityManagerHiddenMethod() throws NoSuchMethodException,
            IllegalAccessException, InvocationTargetException {
        Class cls = ActivityManager.class;
        Class noClass[] = {};
        Method method = cls.getDeclaredMethod("getCurrentUser", noClass);
        method.setAccessible(true);
        Object noObject[] = {};
        Object value = method.invoke(cls, noObject);
        return Integer.valueOf(value.toString());
    }

    public static int getUserHandleHiddenValues(String value) throws ClassNotFoundException,
            NoSuchFieldException, IllegalAccessException {
        Class cls = UserHandle.class;
        Field field = cls.getDeclaredField(value);
        field.setAccessible(true);
        return Integer.valueOf(field.get(cls).toString());
    }

    public static Options setInPostProc(Options options, boolean val) {
        Class cls = options.getClass();
        try {
            Field field = cls.getDeclaredField("inPostProc");
            field.setAccessible(true);
            field.set(options, val);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            Log.w(TAG, "setInPostProc exception", e);
        }
        return options;
    }

    private static Method getProp = null;

    public static String getSysProperty(final String key, String def) {
        try {
            if (getProp == null) {
                getProp = Class.forName("android.os.SystemProperties")
                        .getMethod("get", String.class, String.class);
            }
            return getProp.invoke(null, key, def).toString();
        } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException
                | ClassNotFoundException e) {
            Log.w(TAG, "getSysProperty exception", e);
        }
        return def;
    }

    private static Method setProp = null;

    public static void setSysProperty(String key, String val) {
        try {
            if (setProp == null) {
                setProp = Class.forName("android.os.SystemProperties")
                        .getMethod("set", String.class, String.class);
            }
            setProp.invoke(null, key, val);
        } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException
                | ClassNotFoundException e) {
            Log.w(TAG, "setSysProperty exception", e);
        }
    }

    private static Method getIntProp = null;

    public static int getIntProperty(String key, int def) {
        try {
            if (getIntProp == null) {
                getIntProp = Class.forName("android.os.SystemProperties")
                        .getMethod("getInt", String.class, int.class);
            }
            return Integer.valueOf(getIntProp.invoke(null, key, def).toString());
        } catch (NoSuchMethodException | IllegalAccessException | InvocationTargetException
                | ClassNotFoundException e) {
            Log.w(TAG, "setSysProperty exception", e);
        }
        return def;
    }
}
