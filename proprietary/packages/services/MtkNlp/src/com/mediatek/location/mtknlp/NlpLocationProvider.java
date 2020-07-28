/*
* Copyright (C) 2015 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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
package com.mediatek.location.mtknlp;


import com.android.internal.location.ProviderProperties;
import com.android.internal.location.ILocationProvider;
import com.android.internal.location.ILocationProviderManager;
import com.android.internal.location.ProviderRequest;

import com.android.location.provider.LocationProviderBase;
import com.android.location.provider.ProviderPropertiesUnbundled;
import com.android.location.provider.LocationRequestUnbundled;
import com.android.location.provider.ProviderRequestUnbundled;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserHandle;
import android.os.WorkSource;
import android.util.Log;
import android.widget.Toast;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

public class NlpLocationProvider extends LocationProviderBase {
    private static final String TAG = "NlpLocationProvider";
    private static final String GMS_PACKAGE_NAME = "com.google.android.gms";
    public static final String NETWORK_LOCATION_SERVICE_ACTION =
            "com.android.location.service.v3.NetworkLocationProvider";

    private static ProviderPropertiesUnbundled PROPERTIES = ProviderPropertiesUnbundled.create(
            false, false, false, false, true, true, true, Criteria.POWER_LOW,
            Criteria.ACCURACY_FINE);

    private static final int MSG_ENABLE = 1;
    private static final int MSG_DISABLE = 2;
    private static final int MSG_SET_REQUEST = 3;
    private static final int MSG_CONNECTED = 4;
    private static final int MSG_REPORT_LAST_LOC = 5;
    private static final int MSG_REBIND_NLP = 6;

    private static final int DELAY_REPORT_LAST_LOCATION = (1*1000); //ms
    private static final int LAST_LOCATION_EXPIRED_TIMEOUT = (10*60*1000); //ms

    private final Context mContext;
    private PackageManager mPackageManager;
    private NlpServiceWatcher mNlpServiceWatcher;
    private boolean mEnabled = false;
    private boolean mDelay2ReportLastLocation = false;
    private ProviderRequest mRequest = null;
    private WorkSource mWorkSource;
    private String mVendorPackageName;
    private LocationManager mLocationManager;
    private HandlerThread mHandlerThread;
    private LocationWorkerHandler mHandler;
    private boolean mNlpInstalled = false;

    private static class RequestWrapper {
        public ProviderRequestUnbundled request;
        public WorkSource source;
        public RequestWrapper(ProviderRequestUnbundled request, WorkSource source) {
            this.request = request;
            this.source = source;
        }
    }

    public NlpLocationProvider(Context context) {
        super(TAG, PROPERTIES);
        mContext = context;
        mPackageManager = mContext.getPackageManager();
    }

    public void init(String vendorPackageName) {
        Log.i(TAG, "init");
        mVendorPackageName = vendorPackageName;
        mHandlerThread = new HandlerThread("MtkNlpThread");
        mHandlerThread.start();
        mHandler = new LocationWorkerHandler(mHandlerThread.getLooper());

        mNlpServiceWatcher = new NlpServiceWatcher(mContext, TAG,
                NETWORK_LOCATION_SERVICE_ACTION,  // action
                GMS_PACKAGE_NAME, mVendorPackageName, GMS_PACKAGE_NAME, mHandler) {
            @Override
            protected void onBind() {
                runOnBinder(NlpLocationProvider.this::initializeService);
            }

            @Override
            protected void onUnbind() {
                NlpLocationProvider.this.updateAdditionalProviderPackages(null);
                NlpLocationProvider.this.setEnabled(false);
            }
        };
        mNlpInstalled = mNlpServiceWatcher.start();
        if (!mNlpInstalled) {
            showNotInstalledToast();
        }

        mLocationManager = (LocationManager) mContext.getSystemService(Context.LOCATION_SERVICE);
    }

    private class LocationWorkerHandler extends Handler {
        public LocationWorkerHandler(Looper looper) {
            super(looper, null, true);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ENABLE:
                    handleEnabled();
                    break;
                case MSG_DISABLE:
                    handleDisabled();
                    break;
                case MSG_SET_REQUEST: {
                    RequestWrapper wrapper = (RequestWrapper) msg.obj;
                    handleSetRequest(wrapper.request, wrapper.source);
                    break;
                }
                case MSG_REPORT_LAST_LOC:
                    reportLastLocation();
                    break;
                case MSG_REBIND_NLP:
                    handleRebindNlp(msg.arg1 == 1);
                    break;
            }
        }
    };

    @Override
    public void onEnable() {
        mHandler.sendEmptyMessage(MSG_ENABLE);
    }

    private void handleEnabled() {
        Log.i(TAG, "handleEnabled() mEnabled=" + mEnabled);
        mEnabled = true;
    }

    @Override
    public void onDisable() {
        mHandler.sendEmptyMessage(MSG_DISABLE);
    }

    private void handleDisabled() {
        Log.i(TAG, "handleDisabled() mEnabled=" + mEnabled);
        mEnabled = false;
    }

    @Override
    public void onSetRequest(ProviderRequestUnbundled requestUnbundled, WorkSource source) {
        mHandler.obtainMessage(MSG_SET_REQUEST, new RequestWrapper(requestUnbundled, source)).sendToTarget();
    }

    private void handleSetRequest(ProviderRequestUnbundled requestUnbundled, WorkSource source) {
        Log.i(TAG, "handleSetRequest() request=" + requestUnbundled);
        ProviderRequest providerRequest = new ProviderRequest();

        for (LocationRequestUnbundled locRequestUnbundled : requestUnbundled.getLocationRequests()) {
            LocationRequest locationRequest = LocationRequest.createFromDeprecatedProvider(
                    LocationManager.NETWORK_PROVIDER,
                    locRequestUnbundled.getInterval(),
                    locRequestUnbundled.getSmallestDisplacement(),
                    false);
            providerRequest.locationRequests.add(locationRequest);
            if (locationRequest.getInterval() < providerRequest.interval) {
                providerRequest.reportLocation = requestUnbundled.getReportLocation();
                providerRequest.interval = locationRequest.getInterval();
            }
        }

        if (mRequest != null && mRequest.reportLocation == false
                && providerRequest.reportLocation == false) {
            return;
        }
        mRequest = providerRequest;
        mWorkSource = source;

        ILocationProvider service = getService();
        if (service == null) {
            Log.i(TAG, "handleSetRequest() service is null");
            if (!mNlpInstalled) {
                showNotInstalledToast();
            }
            return;
        }

        try {
            service.setRequest(providerRequest, source);
        } catch (RemoteException e) {
            Log.w(TAG, e);
        } catch (Exception e) {
            // never let remote service crash system server
            Log.e(TAG, "Exception from " + mNlpServiceWatcher.getCurrentPackageName(), e);
        }

        if (mRequest.reportLocation) {
            if (!mDelay2ReportLastLocation) {
                sendCommandDelayed(MSG_REPORT_LAST_LOC, DELAY_REPORT_LAST_LOCATION);
                mDelay2ReportLastLocation = true;
            }
        } else {
            if (mDelay2ReportLastLocation) {
                mHandler.removeMessages(MSG_REPORT_LAST_LOC);
                mDelay2ReportLastLocation = false;
            }
        }
    }

    @Override
    public void onDump(FileDescriptor fd, PrintWriter pw, String[] args) {
    }

    @Override
    public int onGetStatus(Bundle extras) {
        return LocationProvider.AVAILABLE;
    }

    @Override
    public long onGetStatusUpdateTime() {
        return 0;
    }

    private final ILocationProviderManager.Stub mManager = new ILocationProviderManager.Stub() {
        // executed on binder thread
        @Override
        public void onSetAdditionalProviderPackages(List<String> packageNames) {
            Log.d(TAG, "get onSetAdditionalProviderPackages packageNames:" + packageNames);
            NlpLocationProvider.this.updateAdditionalProviderPackages(packageNames);
        }

        // executed on binder thread
        @Override
        public void onSetEnabled(boolean enabled) {
            Log.d(TAG, "get onSetEnabled enabled:" + enabled);
            NlpLocationProvider.this.setEnabled(enabled);
        }

        // executed on binder thread
        @Override
        public void onSetProperties(ProviderProperties properties) {
            Log.d(TAG, "get onSetProperties properties:" + properties);
            NlpLocationProvider.this.setProperties(PROPERTIES);
            NlpLocationProvider.this.updateAdditionalProviderPackages(null);
        }

        // executed on binder thread
        @Override
        public void onReportLocation(Location location) {
            Log.d(TAG, "onReportLocation");
            NlpLocationProvider.this.reportLocation(location);
        }
    };


    private void initializeService(IBinder binder) {// throws RemoteException {
        ILocationProvider service = ILocationProvider.Stub.asInterface(binder);
        Log.d(TAG, "applying state to connected service " + service);

        //resetProviderPackages(Collections.emptyList());
        String packageName = getNetworkProviderPackage();

        if (service == null) return;

        if (!GMS_PACKAGE_NAME.equals(packageName)) {
            grantPermissions(packageName);
        }

        try {
            service.setLocationProviderManager(mManager);
            if (mRequest != null) {
                service.setRequest(mRequest, mWorkSource);
                sendCommandDelayed(MSG_REPORT_LAST_LOC, DELAY_REPORT_LAST_LOCATION);
                mDelay2ReportLastLocation = true;
            }
        } catch (RemoteException e) {
            Log.w(TAG, e);
        } catch (Exception e) {
            // never let remote service crash system server
            Log.e(TAG, "Exception from " + mNlpServiceWatcher.getCurrentPackageName(), e);
        }
    }

    public void onStop() {
        mNlpServiceWatcher.stop();
    }

    public void reBindNlp(boolean bindGmsPackage) {
        mHandler.obtainMessage(MSG_REBIND_NLP, bindGmsPackage ? 1 : 0, 0, null).sendToTarget();
    }

    private void handleRebindNlp(boolean bindGmsPackage) {
        Log.d(TAG, "reBindNetworkProviderLock bindGmsPackage: " + bindGmsPackage);

        //unbind previous network provider
        if (mNlpServiceWatcher != null) {
            mNlpServiceWatcher.stop();
        }

        // try to bind to new network provider
        String preferPackageName;
        if (bindGmsPackage) {
            preferPackageName = GMS_PACKAGE_NAME;
        } else {
            preferPackageName = mVendorPackageName;
        }

        // bind to network provider
        mNlpServiceWatcher = new NlpServiceWatcher(mContext, "MtkNlp",
                NETWORK_LOCATION_SERVICE_ACTION,  // action
                GMS_PACKAGE_NAME,
                mVendorPackageName,
                preferPackageName,
                mHandler)  {
            @Override
            protected void onBind() {
                runOnBinder(NlpLocationProvider.this::initializeService);
            }

            @Override
            protected void onUnbind() {
                NlpLocationProvider.this.updateAdditionalProviderPackages(null);
                NlpLocationProvider.this.setEnabled(false);
            }
        };

        if (mNlpServiceWatcher.start()) {
            Log.d(TAG, "Try to bind: " + preferPackageName);
        } else {
            Log.d(TAG, "Failed to bind specified package service");
        }
    }

    private ILocationProvider getService() {
        return ILocationProvider.Stub.asInterface(mNlpServiceWatcher.getBinder());
    }

    public String getNetworkProviderPackage() {
        return mNlpServiceWatcher.getCurrentPackageName();
    }

    private void sendCommandDelayed(int cmd, long delayMs) {
        Message msg = Message.obtain();
        msg.what = cmd;
        mHandler.sendMessageDelayed(msg, delayMs);
    }

    private void reportLastLocation() {
        if (mRequest != null && mRequest.reportLocation) {
            Location lastLocation = mLocationManager.
                    getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
            if (lastLocation != null) {
                long currentUtcTime = System.currentTimeMillis();
                long nlpTime = lastLocation.getTime();
                long deltaMs = currentUtcTime - nlpTime;
                if (deltaMs < LAST_LOCATION_EXPIRED_TIMEOUT
                        && deltaMs > (DELAY_REPORT_LAST_LOCATION * 2)) {
                    reportLocation(lastLocation);
                    Log.d(TAG, "reportLastLocation crTime: " + currentUtcTime + " nlpTime: "
                            + nlpTime + " delta: " + deltaMs);
                }
            }
        }
        mDelay2ReportLastLocation = false;
    }

    private void grantPermissions(String packageName) {
        Log.d(TAG, "GrantRuntimePermission package: " + packageName);
        grantPermission(packageName, android.Manifest.permission.READ_PHONE_STATE);
        grantPermission(packageName, android.Manifest.permission.ACCESS_COARSE_LOCATION);
        grantPermission(packageName, android.Manifest.permission.ACCESS_FINE_LOCATION);
        grantPermission(packageName, android.Manifest.permission.ACCESS_BACKGROUND_LOCATION);
    }

    private void grantPermission(String packageName, String permission) {
        if (packageName != null) {
            try {
                mPackageManager.grantRuntimePermission(packageName, permission,
                        new UserHandle(UserHandle.USER_SYSTEM));
            } catch (IllegalArgumentException e) {
                Log.w(TAG, "GrantRuntimePermission IllegalArgumentException: " + permission);
            } catch (SecurityException e) {
                Log.w(TAG, "GrantRuntimePermission SecurityException: " + permission);
            } catch (Exception e) {
                Log.w(TAG, "GrantRuntimePermission Exception: " + permission);
            }
        }
    }

    private void updateAdditionalProviderPackages(List<String> packageNames) {
        List<String> pNames;
        if (packageNames != null) {
            pNames = new ArrayList<>(packageNames);
        } else {
            pNames = new ArrayList<>();
        }
        ILocationProvider service = getService();
        if (service != null) {
            pNames.add(getNetworkProviderPackage());
        }
        Log.d(TAG, "call setAdditionalProviderPackages pName:" + pNames);
        setAdditionalProviderPackages(pNames);
    }

    private void showNotInstalledToast() {
        Log.d(TAG, "showNotInstalledToast");
        Toast.makeText(mContext,
                "No Network Location Provider is installed!" +
                "NLP is necessary for network location fixes.",
                Toast.LENGTH_LONG).show();
    }
}
