package com.mediatek.location.mtknlp;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.ContentObserver;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkInfo;
import android.net.NetworkRequest;
import android.os.BatteryManager;
import android.os.Handler;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.ServiceState;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.telephony.TelephonyIntents;

import java.util.List;

public class NlpSwitcher {
    private static final boolean DEBUG = true;
    private static final String NETWORK_LOCATION_SERVICE_ACTION =
            "com.android.location.service.v3.NetworkLocationProvider";
    private static NlpSwitcher sInstance;

    private Context mContext;
    private PackageManager mPackageManager;
    private String mGmsLpPkg;
    private String[] mVendorLpPkgs;
    private int mUsbPlugged = 0;
    private int mWifiConnected = 0;
    private int mStayAwake = 0;
    private final ConnectivityManager mConnMgr;
    private String mMccMnc;
    private Handler mHandler = new Handler();
    private boolean mInTestMode = false;
    private String mVendorNlpPackageName;
    private NlpLocationProvider mNlpProvider;
    private NlpGeocoder mGeocoder;
    private boolean mNlpSwitchSupport = true;
    private BroadcastReceiver mBroadcastReceiver;
    private ContentObserver mContentObserver;
    private Object mLock = new Object();

    public NlpSwitcher(Context context, String gmsLpPkg, String[] vendorLpPkgs) {
        if (DEBUG) log("NlpSwitcher constructor");
        mContext = context;
        mGmsLpPkg = gmsLpPkg;
        mVendorLpPkgs = vendorLpPkgs;
        mPackageManager = mContext.getPackageManager();
        mConnMgr = (ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
    }

    public void startMonitor() {
        if (DEBUG) log("startMonitor");

        if(mNlpSwitchSupport) {
            registerAutoSwitchNlpFilter();
        }
    }

    public void stopMonitor() {
        if (DEBUG) log("stopMonitor");
        if(mNlpSwitchSupport) {
            mContext.unregisterReceiver(mBroadcastReceiver);
            mConnMgr.unregisterNetworkCallback(mNetworkConnectivityCallback);
            mContext.getContentResolver().unregisterContentObserver(mContentObserver);
        }
    }

    private void registerAutoSwitchNlpFilter() {
        IntentFilter intentSrvFilter = new IntentFilter();
        intentSrvFilter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        intentSrvFilter.addAction(Intent.ACTION_BATTERY_CHANGED);
        mBroadcastReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                String action = intent.getAction();
                if (TelephonyIntents.ACTION_SERVICE_STATE_CHANGED.equals(action)) {
                    serviceStateChanged(intent);
                } else if (Intent.ACTION_BATTERY_CHANGED.equals(action)) {
                    int usbPlugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, 0);
                    if (usbPlugged != mUsbPlugged) {
                        log("usb plugged state changed: " + usbPlugged);
                        mUsbPlugged = usbPlugged;
                        if (mUsbPlugged == 0) {
                            mInTestMode = false;
                        }
                        testModeConditionChanged();
                    }
                }
            }
        };
        mContext.registerReceiverAsUser(mBroadcastReceiver,
                UserHandle.ALL, intentSrvFilter, null, mHandler);

        // registering for CONNECTIVITY_ACTION broadcasts
        NetworkRequest.Builder networkRequestBuilder = new NetworkRequest.Builder();
        networkRequestBuilder.addTransportType(NetworkCapabilities.TRANSPORT_WIFI);
        NetworkRequest networkRequest = networkRequestBuilder.build();
        mConnMgr.registerNetworkCallback(networkRequest, mNetworkConnectivityCallback);

        try {
            mStayAwake = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.STAY_ON_WHILE_PLUGGED_IN);
        } catch (SettingNotFoundException e) {
            // TODO Auto-generated catch block
            log("settings not found exception");
        }

        // listen for settings changes
        mContentObserver = new ContentObserver(mHandler) {
            @Override
            public void onChange(boolean selfChange) {
                try {
                    int stayAwake = Settings.Global.getInt(mContext.getContentResolver(),
                    Settings.Global.STAY_ON_WHILE_PLUGGED_IN);
                    if (stayAwake != mStayAwake) {
                        log("Stay awake state changed: " + stayAwake);
                        mStayAwake = stayAwake;
                        testModeConditionChanged();
                    }
                } catch (SettingNotFoundException e) {
                    // TODO Auto-generated catch block
                    log("settings not found exception");
                }
            }
        };
        mContext.getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(Settings.Global.STAY_ON_WHILE_PLUGGED_IN), true,
                mContentObserver, UserHandle.USER_ALL);
    }

    public boolean isNlpSwitchingSupported() {
        Intent intent = new Intent(NETWORK_LOCATION_SERVICE_ACTION);
        List<ResolveInfo> rInfos = mPackageManager.queryIntentServicesAsUser(intent,
                PackageManager.GET_META_DATA, UserHandle.USER_OWNER);
        if (rInfos == null) {
            log("Installed NLP count= 0, skip switching");
            return false;
        } else if (mVendorLpPkgs[0] == null) {
            log("vendor package name is null, skip switching");
            return false;
        } else if (rInfos.size() < 3) {  // one is MtkNlp, at least 2 more vendor NLP
            log("Installed NLP size is less than 2, skip switching");
            return false;
        } else {
            for (ResolveInfo rInfo : rInfos) {
                final ComponentName component = rInfo.serviceInfo.getComponentName();
                final String packageName = component.getPackageName();
                if (mGmsLpPkg != null && mGmsLpPkg.equals(packageName)) {
                    return true;
                }
            }
        }
        log("GMS NLP is not installed, skip switching");
        return false;
    }

    private void serviceStateChanged(Intent intent) {
        synchronized (mLock) {
            ServiceState ss = ServiceState.newFromBundle(intent.getExtras());
            String mccMnc = ss != null ? ss.getOperatorNumeric() : null;
            /// for testing purpose extra data
            String testStr = intent.getStringExtra("testStr");
            log("received action ACTION_SERVICE_STATE_CHANGED, testStr=" + testStr);
            if (!TextUtils.isEmpty(mccMnc)) {
                mMccMnc = mccMnc;
            } else if(!TextUtils.isEmpty(testStr)) {
                mMccMnc = testStr;
            }

            if (!TextUtils.isEmpty(mMccMnc)) {
                log("Network mMccMnc is set");
                maybeRebindNetworkProvider();
            } else {
                log("Network mMccMnc is not yet set");
            }
        }
    }

    private void testModeConditionChanged() {
        maybeRebindNetworkProvider();
    }

    private String getNetworkProviderPackage() {
        if (mNlpProvider != null) {
            return mNlpProvider.getNetworkProviderPackage();
        }
        return null;
    }

    private void maybeRebindNetworkProvider() {
        synchronized (mLock) {
            if (!isNlpSwitchingSupported()) {
                return;
            }

            String nlpPackageName = getNetworkProviderPackage();
            if (nlpPackageName != null) {
                log("current NLP package name: " + nlpPackageName);
            } else {
                log("currently there is no NLP binded.");
            }

            boolean isUsingGmsNlp = (mGmsLpPkg != null && mGmsLpPkg.equals(nlpPackageName));
            if (!isUsingGmsNlp) {
                mVendorNlpPackageName = nlpPackageName;
            }

            if ((mUsbPlugged != 0 && mWifiConnected != 0 && mStayAwake != 0) || mInTestMode) {
                log("current in test mode, mInTestMode=" + mInTestMode + " mVendorNlpPackageName=" + mVendorNlpPackageName);
                if (!isUsingGmsNlp || nlpPackageName == null) {
                    reBindNetworkProvider(true);
                }
                if (!mInTestMode) {
                    revokePermissions();
                    mInTestMode = true;
                }
            } else if (mMccMnc != null && mMccMnc.startsWith("460")) {
                // in China area
                if (isUsingGmsNlp || nlpPackageName == null) {
                    reBindNetworkProvider(false);
                }
            } else {
                // in global area except China
                if (!isUsingGmsNlp || nlpPackageName == null) {
                    reBindNetworkProvider(true);
                }
            }
        }
    }

    private void revokePermissions() {
        if (mVendorNlpPackageName == null) {
            if (mVendorLpPkgs != null) {
                for (String vendorPkg : mVendorLpPkgs) {
                    revokePermissions(vendorPkg);
                }
            }
        } else {
            revokePermissions(mVendorNlpPackageName);
        }
    }

    private void revokePermissions(String packageName) {
        log("revokeRuntimePermission package: " + packageName);
        revokePermission(packageName, android.Manifest.permission.READ_PHONE_STATE);
        revokePermission(packageName, android.Manifest.permission.ACCESS_COARSE_LOCATION);
        revokePermission(packageName, android.Manifest.permission.ACCESS_FINE_LOCATION);
        revokePermission(packageName, android.Manifest.permission.ACCESS_BACKGROUND_LOCATION);
    }

    private void revokePermission(String packageName, String permission) {
        if (packageName != null) {
            try {
                mPackageManager.revokeRuntimePermission(packageName, permission,
                        new UserHandle(UserHandle.USER_SYSTEM));
            } catch (IllegalArgumentException e) {
                log("RevokeRuntimePermission IllegalArgumentException: " + permission);
            } catch (SecurityException e) {
                log("RevokeRuntimePermission SecurityException: " + permission);
            } catch (Exception e) {
                log("RevokeRuntimePermission Exception: " + permission);
            }
        }
    }

    private void reBindNetworkProvider(boolean bindGmsPackage) {
        log("reBindNetworkProvider bindGmsPackage: " + bindGmsPackage);
        if (mNlpProvider != null) {
            mNlpProvider.reBindNlp(bindGmsPackage);
        }
        if (mGeocoder != null) {
            mGeocoder.reBindGeocoder(bindGmsPackage);
        }
    }

    public static void log(String msg) {
        if (DEBUG) {
            Log.d("NlpSwitcher", msg);
        }
    }

    public void setNlp(NlpLocationProvider networkLocationProvider) {
        mNlpProvider = networkLocationProvider;
    }

    public void setGeocoder(NlpGeocoder geocoder) {
        mGeocoder = geocoder;
    }

    /**
     * Callback used to listen for data connectivity changes.
     */
    private final ConnectivityManager.NetworkCallback mNetworkConnectivityCallback =
            new ConnectivityManager.NetworkCallback() {
        @Override
        public void onAvailable(Network network) {
            if (mWifiConnected == 0) {
                mWifiConnected = 1;
                log("wifi connected state changed: " + mWifiConnected);
                testModeConditionChanged();
            }
        }
        @Override
        public void onLost(Network network) {
            if (mWifiConnected == 1) {
                mWifiConnected = 0;
                log("wifi connected state changed: " + mWifiConnected);
                testModeConditionChanged();
            }
        }

        @Override
        public void onUnavailable() {
            if (mWifiConnected == 1) {
                mWifiConnected = 0;
                log("wifi connected state changed: " + mWifiConnected);
                testModeConditionChanged();
            }
        }
    };

}
