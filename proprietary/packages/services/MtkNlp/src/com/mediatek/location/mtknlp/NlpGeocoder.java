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

import android.content.Context;
import android.location.Address;
import android.location.GeocoderParams;
import android.location.IGeocodeProvider;
import android.os.Handler;
import android.os.RemoteException;
import android.util.Log;

import com.android.location.provider.GeocodeProvider;
import java.util.List;

public class NlpGeocoder extends GeocodeProvider {
    private static final String TAG = "NlpGeocoder";
    private static final String GMS_PACKAGE_NAME = "com.google.android.gms";
    public static final String GEOCODER_SERVICE_ACTION =
            "com.android.location.service.GeocodeProvider";

    private final Context mContext;
    private NlpServiceWatcher mGeocoderServiceWatcher;
    private String mVendorPackageName;
    private Handler mHandler = new Handler();

    public NlpGeocoder(Context context) {
        super();
        mContext = context;
    }

    public void init(String vendorPackageName) {
        Log.i(TAG, "init");
        mVendorPackageName = vendorPackageName;
        mGeocoderServiceWatcher = new NlpServiceWatcher(mContext, TAG,
                GEOCODER_SERVICE_ACTION,  // action
                GMS_PACKAGE_NAME, mVendorPackageName, GMS_PACKAGE_NAME, mHandler) {
            @Override
            protected void onBind() {
            }

            @Override
            protected void onUnbind() {
            }
        };
        mGeocoderServiceWatcher.start();
    }

    public void reBindGeocoder(boolean bindGmsPackage) {
        Log.d(TAG, "reBindGeocoder bindGmsPackage: " + bindGmsPackage);

        //unbind previous network provider
        if (mGeocoderServiceWatcher != null) {
            mGeocoderServiceWatcher.stop();
        }

        // try to bind to new network provider
        String preferPackageName;
        if (bindGmsPackage) {
            preferPackageName = GMS_PACKAGE_NAME;
        } else {
            preferPackageName = mVendorPackageName;
        }

        // bind to network provider
        mGeocoderServiceWatcher = new NlpServiceWatcher(mContext, "NlpGeocoder",
                GEOCODER_SERVICE_ACTION,  // action
                GMS_PACKAGE_NAME,
                mVendorPackageName,
                preferPackageName,
                mHandler){
            @Override
            protected void onBind() {
            }

            @Override
            protected void onUnbind() {
            }
        };

        if (mGeocoderServiceWatcher.start()) {
            Log.d(TAG, "Try to bind: " + preferPackageName);
        } else {
            Log.d(TAG, "Failed to bind specified package service");
        }
    }

    private IGeocodeProvider getService() {
        return IGeocodeProvider.Stub.asInterface(mGeocoderServiceWatcher.getBinder());
    }

    public String onGetFromLocation(double latitude, double longitude, int maxResults,
            GeocoderParams params, List<Address> addrs) {
        IGeocodeProvider provider = getService();
        if (provider != null) {
            try {
                return provider.getFromLocation(latitude, longitude, maxResults, params, addrs);
            } catch (RemoteException e) {
                Log.w(TAG, e);
            }
        }
        return null;
    }

    public String onGetFromLocationName(String locationName,
            double lowerLeftLatitude, double lowerLeftLongitude,
            double upperRightLatitude, double upperRightLongitude, int maxResults,
            GeocoderParams params, List<Address> addrs) {
        IGeocodeProvider provider = getService();
        if (provider != null) {
            try {
                return provider.getFromLocationName(locationName, lowerLeftLatitude,
                        lowerLeftLongitude, upperRightLatitude, upperRightLongitude,
                        maxResults, params, addrs);
            } catch (RemoteException e) {
                Log.w(TAG, e);
            }
        }
        return null;
    }

    public void onStop() {
        mGeocoderServiceWatcher.stop();
    }
}

