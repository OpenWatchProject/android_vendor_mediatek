package com.mediatek.dataprotection.utils;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.pm.PackageManager;
import android.support.v4.app.ActivityCompat;

import com.mediatek.dataprotection.utils.Log;

public class PermissionUtils {
    private static final String TAG = "PermissionUtil";
    public static boolean hasStorageWritePermission(Context ctx) {
        return (ctx.checkSelfPermission(Manifest.permission.
                WRITE_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED);
    }

    public static boolean hasStorageReadPermission(Context ctx) {
        return (ctx.checkSelfPermission(Manifest.permission.
                READ_EXTERNAL_STORAGE) == PackageManager.PERMISSION_GRANTED);
    }

    public static boolean hasStorageReadWritePermission(Context ctx) {
        return hasStorageReadPermission(ctx) && hasStorageWritePermission(ctx);
    }

    public static void requestPermission(Activity ctx, String permission, int requestCode){
        ctx.requestPermissions(new String[]{permission}, requestCode);
    }

    public static void requestReadWriteStoragePermission(Activity ctx, int requestCode){
        ctx.requestPermissions(new String[]{Manifest.permission.
                         READ_EXTERNAL_STORAGE, Manifest.permission.
                         WRITE_EXTERNAL_STORAGE}, requestCode);
    }

    public static boolean showWriteRational(Activity ctx){
        return ctx.shouldShowRequestPermissionRationale(
                Manifest.permission.WRITE_EXTERNAL_STORAGE);
    }

    public static boolean showReadRational(Activity ctx){
        return ctx.shouldShowRequestPermissionRationale(
                Manifest.permission.READ_EXTERNAL_STORAGE);
    }

    public static boolean showReadOrWriteRational(Activity ctx) {
        boolean showRead = ActivityCompat.shouldShowRequestPermissionRationale(ctx,
                Manifest.permission.READ_EXTERNAL_STORAGE);
        boolean showWrite = ActivityCompat.shouldShowRequestPermissionRationale(ctx,
                Manifest.permission.WRITE_EXTERNAL_STORAGE);
        Log.d(TAG, "showRead: " + showRead + " showWrite: " + showWrite);
        return showWrite || showRead;
    }
}
