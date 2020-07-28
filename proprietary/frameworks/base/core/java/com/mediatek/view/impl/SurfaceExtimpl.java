package com.mediatek.view.impl;

import android.content.pm.IPackageManager;
import android.content.pm.ResolveInfo;
import android.os.Binder;
import android.os.Process;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.util.Log;

import com.mediatek.view.SurfaceExt;
import com.mediatek.appresolutiontuner.ResolutionTunerAppList;


public class SurfaceExtimpl extends SurfaceExt {
    private static final String TAG = "SurfaceExt";
    private static final boolean ENABLE_WHITE_LIST = SystemProperties.getBoolean(
            "debug.enable.whitelist", false);
    private static final int ENABLE_RESOLUTION_TUNING = SystemProperties.getInt(
            "ro.vendor.app_resolution_tuner", 0);
    private static final String WHITE_LIST[] = {"com.tencent.qqlive"};
    private String mPackageName;
    private boolean mIsContainPackageName = false;
    private float mXScaleValue = 1;
    private float mYScaleValue = 1;
    private static ResolutionTunerAppList mApplist = null;

    public SurfaceExtimpl() {
    }

    @Override
    public boolean isInWhiteList() {
        if (ENABLE_WHITE_LIST) {
            return true;
            }
        boolean isContained = false;
        String packageName = getCallerProcessName();
        if (WHITE_LIST != null && packageName != null) {
            for (String item : WHITE_LIST) {
                if (item.equals(packageName)) {
                    isContained = true;
                    break;
                }
            }
        }
        return isContained;
    }

    @Override
    public void initResolutionTunner() {
        if (ENABLE_RESOLUTION_TUNING == 1 && mApplist == null) {
            mPackageName = getCallerProcessName();
            mApplist = getTunerList();
            mIsContainPackageName = mApplist.isScaledBySurfaceView(mPackageName);
            mXScaleValue = mApplist.getScaleValue(mPackageName);
            mYScaleValue = mXScaleValue;
            Log.d(TAG, "initResolutionTunner, mPackageName:" + mPackageName +
                        ",mContainPackageName:" + mIsContainPackageName +
                        "mXScaleValue:" + mXScaleValue +
                        ",mYScaleValue:" + mYScaleValue);
        }
    }

    @Override
    public boolean isResolutionTuningPackage() {
        return mIsContainPackageName;
    }

    @Override
    public float getXScale() {
        return mXScaleValue;
    }

    @Override
    public float getYScale() {
        return mYScaleValue;
    }

    private ResolutionTunerAppList getTunerList() {
        ResolutionTunerAppList applist = ResolutionTunerAppList.getInstance();
        applist.loadTunerAppList();
        return applist;
    }

    private String getCallerProcessName() {
        int binderuid = Binder.getCallingUid();
        IPackageManager pm = IPackageManager.Stub.asInterface(ServiceManager.getService("package"));
        if (pm != null) {
            try {
                String callingApp = pm.getNameForUid(binderuid);
                // Log.d(TAG, "callingApp: " + callingApp);
                return callingApp;
            } catch (RemoteException e) {
                Log.e(TAG, "getCallerProcessName exception :" + e);
            }
        }
        return null;
    }
}
