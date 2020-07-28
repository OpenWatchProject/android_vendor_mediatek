/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.camera.v2.app.location;

import android.content.Context;
import android.location.Location;
import android.os.Bundle;

import com.mediatek.camera.debug.LogHelper;
import com.mediatek.camera.debug.LogHelper.Tag;

/**
 * A class that handles legacy (network, gps) location providers, in the event
 * the fused location provider from Google Play Services is unavailable.
 */
public class LocationProviderImpl implements ILocationProvider {
    private static final Tag TAG = new Tag(LocationProviderImpl.class.getSimpleName());

    private Context mContext;
    private android.location.LocationManager mLocationManager;
    private boolean mRecordLocation;

    LocationListener[] mLocationListeners = new LocationListener[] {
            new LocationListener(android.location.LocationManager.GPS_PROVIDER),
            new LocationListener(
                    android.location.LocationManager.NETWORK_PROVIDER) };

    public LocationProviderImpl(
            Context context) {
        mContext = context;
    }

    @Override
    public Location getCurrentLocation() {
        if (!mRecordLocation)
            return null;

        // go in best to worst order
        for (int i = 0; i < mLocationListeners.length; i++) {
            Location l = mLocationListeners[i].current();
            if (l != null)
                return l;
        }
        LogHelper.i(TAG, "No location received yet.");
        return null;
    }

    public void recordLocation(boolean recordLocation) {
        LogHelper.d(TAG, "[recordLocation], mRecordLocation = " + mRecordLocation
                + ",recordLocation = " + recordLocation);
        if (mRecordLocation != recordLocation) {
            mRecordLocation = recordLocation;
            if (recordLocation) {
                startReceivingLocationUpdates();
            } else {
                stopReceivingLocationUpdates();
            }
        }
    }

    @Override
    public void disconnect() {
        LogHelper.d(TAG, "disconnect");
        // The onPause() call to stopReceivingLocationUpdates is sufficient to
        // unregister the
        // Network/GPS listener.
    }

    private void startReceivingLocationUpdates() {
        LogHelper.d(TAG, "startReceivingLocationUpdates ++++");

        if (mLocationManager == null) {
            mLocationManager = (android.location.LocationManager) mContext
                    .getSystemService(Context.LOCATION_SERVICE);
        }
        if (mLocationManager != null) {
            try {
                mLocationManager.requestLocationUpdates(
                        android.location.LocationManager.NETWORK_PROVIDER,
                        1000, 0F, mLocationListeners[1]);
            } catch (SecurityException ex) {
                LogHelper.e(TAG, "fail to request location update, ignore", ex);
            } catch (IllegalArgumentException ex) {
                LogHelper.e(TAG, "provider does not exist " + ex.getMessage());
            }

            try {
                mLocationManager.requestLocationUpdates(
                        android.location.LocationManager.GPS_PROVIDER, 1000,
                        0F, mLocationListeners[0]);
            } catch (SecurityException ex) {
                LogHelper.e(TAG, "fail to request location update, ignore", ex);
            } catch (IllegalArgumentException ex) {
                LogHelper.e(TAG, "provider does not exist " + ex.getMessage());
            }
            LogHelper.d(TAG, "startReceivingLocationUpdates----");
        }
    }

    private void stopReceivingLocationUpdates() {
        if (mLocationManager != null) {
            LogHelper.d(TAG, "stopReceivingLocationUpdates++++");
            for (int i = 0; i < mLocationListeners.length; i++) {
                try {
                    mLocationManager.removeUpdates(mLocationListeners[i]);
                } catch (Exception ex) {
                    LogHelper.e(TAG, "fail to remove location listners, ignore", ex);
                }
            }
            LogHelper.d(TAG, "stopReceivingLocationUpdates----");
        }
    }

    private class LocationListener implements android.location.LocationListener {
        Location mLastLocation;
        boolean mValid = false;
        String mProvider;

        public LocationListener(
                String provider) {
            LogHelper.d(TAG, "[LocationListener] provider = " + provider);
            mProvider = provider;
            mLastLocation = new Location(mProvider);
        }

        @Override
        public void onLocationChanged(Location newLocation) {
            LogHelper.d(TAG, "[onLocationChanged]");

            if (newLocation.getLatitude() == 0.0
                    && newLocation.getLongitude() == 0.0) {
                // Hack to filter out 0.0,0.0 locations
                return;
            }
            if (!mValid) {
                LogHelper.d(TAG, "Got first location.");
            }
            mLastLocation.set(newLocation);
            mValid = true;
        }

        @Override
        public void onProviderEnabled(String provider) {
            // do -noting
        }

        @Override
        public void onProviderDisabled(String provider) {
            mValid = false;
        }

        @Override
        public void onStatusChanged(String provider, int status, Bundle extras) {
            switch (status) {
            case android.location.LocationProvider.OUT_OF_SERVICE:
            case android.location.LocationProvider.TEMPORARILY_UNAVAILABLE: {
                mValid = false;
                break;
            }
            }
        }

        public Location current() {
            LogHelper.d(TAG, "[current],mValid = " + mValid);
            return mValid ? mLastLocation : null;
        }
    }
}
