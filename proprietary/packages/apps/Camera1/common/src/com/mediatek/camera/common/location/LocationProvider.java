/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.location;

import android.app.Activity;
import android.content.Context;
import android.location.Location;
import android.os.Bundle;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.permission.PermissionManager;

/**
 * This is to get location info.
 */

public class LocationProvider {

    private static final LogUtil.Tag TAG = new LogUtil.Tag(LocationProvider.class.getSimpleName());
    private static final int LOCATION_UPDATE_TIME = 5000;

    private Context mContext;
    private android.location.LocationManager mLocationManager;
    private boolean mRecordLocation;
    private PermissionManager mLocationPermission;

    LocationListener[] mLocationListeners = new LocationListener[] {
            new LocationListener(android.location.LocationManager.GPS_PROVIDER),
            new LocationListener(
                    android.location.LocationManager.NETWORK_PROVIDER) };

    /**
     * init the location provider.
     * @param context the activity context.
     */
    public LocationProvider(Context context) {
        mContext = context;
        mLocationPermission = new PermissionManager((Activity) context);
    }

    /**
     * get current location info.
     * @return the location.
     */
    public Location getCurrentLocation() {
        if (!mRecordLocation) {
            return null;
        }
        // go in best to worst order
        for (int i = 0; i < mLocationListeners.length; i++) {
            Location l = mLocationListeners[i].current();
            if (l != null) {
                return l;
            }
        }
        LogHelper.d(TAG, "No location received yet.");
        return null;
    }

    /**
     * To record location, the location info maybe updated.
     * @param recordLocation true to start record location.
     *                       false to stop record location.
     */
    public void recordLocation(boolean recordLocation) {
        if (!mLocationPermission.checkCameraLocationPermissions()) {
            return;
        }
        if (mRecordLocation != recordLocation) {
            mRecordLocation = recordLocation;
            if (recordLocation) {
                startReceivingLocationUpdates();
            } else {
                stopReceivingLocationUpdates();
            }
        }
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
                        LOCATION_UPDATE_TIME, 0F, mLocationListeners[1]);
            } catch (SecurityException ex) {
                LogHelper.e(TAG, "fail to request location update, ignore", ex);
            } catch (IllegalArgumentException ex) {
                LogHelper.e(TAG, "provider does not exist " + ex.getMessage());
            }

            try {
                mLocationManager.requestLocationUpdates(
                        android.location.LocationManager.GPS_PROVIDER,
                        LOCATION_UPDATE_TIME, 0F, mLocationListeners[0]);
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

    /**
     * the location listener to set to provider when request location.
     */
    private class LocationListener implements android.location.LocationListener {
        Location mLastLocation;
        boolean mValid = false;
        String mProvider;

        /**
         * init location listener.
         * @param provider the location provider.
         */
        public LocationListener(String provider) {
            mProvider = provider;
            mLastLocation = new Location(mProvider);
        }

        /**
         * the callback for location changed.
         * @param newLocation the location info.
         */
        @Override
        public void onLocationChanged(Location newLocation) {
            if (newLocation.getLatitude() == 0.0
                    && newLocation.getLongitude() == 0.0) {
                // Hack to filter out 0.0,0.0 locations
                return;
            }
            mLastLocation.set(newLocation);
            mValid = true;
        }

        /**
         * to notify provider enabled.
         * @param provider the location provider.
         */
        @Override
        public void onProviderEnabled(String provider) {
            // do -noting
        }

        /**
         * to notify provider disabled.
         * @param provider the location provider.
         */
        @Override
        public void onProviderDisabled(String provider) {
            mValid = false;
        }

        /**
         * to notify location provider status changed.
         * @param provider the location provider.
         * @param status the location provider status.
         * @param extras the msg Bundle.
         */
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

        /**
         * get the current location info.
         * @return
         */
        public Location current() {
            LogHelper.d(TAG, "[current],mValid = " + mValid);
            return mValid ? mLastLocation : null;
        }
    }
}
