package com.mediatek.dm;

import android.content.Context;
import android.net.Network;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.util.Log;


/**
 * A class provides the DM service APIs.
 *
 * @hide
 */
public final class DmManager {
    private static final String TAG = "DmManager";

    private final Context mContext;
    private static IDmService mService;
    private static DmManager mDmManager = null;


    /**
     * Helpers to get the default DmManager.
     */
    public static DmManager getDefaultDmManager(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }

        synchronized (DmManager.class) {
            if (mDmManager == null) {
                IBinder b = ServiceManager.getService("GbaDmService");
                if (b == null) {
                    Log.i("debug", "[getDefaultDmManager]The binder is null");
                    return null;
                }
                mService = IDmService.Stub.asInterface(b);
                mDmManager = new DmManager(context);
            }
            return mDmManager;
        }
    }

    DmManager(Context context) {
        mContext = context;
    }

    /**
     * Check Dm is supported and support type or not.
     *
     * @return Dm Support Type
     */
    public int getDmSupported() {
        try {
            return mService.getDmSupported();
        } catch (RemoteException e) {
            return 0;
        }
    }

    public boolean getImcProvision(int phoneId, int feature) {
        Log.d(TAG, "DmManager getImcProvision for feature=" + feature);
        try {
            return mService.getImcProvision(phoneId, feature);
        } catch (RemoteException e) {
            return true; //prvisison default to enable on any exception
        }
    }

    public boolean setImcProvision(int phoneId, int feature, int pvs_en) {
        Log.d(TAG, "DmManager setImcProvision for feature=" + feature + ", en = " + pvs_en);
        try {
            return mService.setImcProvision(phoneId, feature, pvs_en);
        } catch (RemoteException e) {
            return false;
        }
    }

}
