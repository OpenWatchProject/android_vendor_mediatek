/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.location;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.database.ContentObserver;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.os.IBinder;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.UserHandle;
import android.util.Log;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.widget.Toast;
import android.os.Handler;
import android.os.HandlerThread;

import android.os.Bundle;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.widget.Toast;

import java.util.Calendar;

import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
import com.mediatek.cta.CtaManager.KeywordType;
import com.mediatek.cta.CtaManager.ActionType;

import static com.mediatek.provider.MtkSettingsExt.Global.AUTO_TIME_GPS;

public class MtkLocationExt {
    private static final String TAG = "MtkLocationExt";
    private static final boolean DEBUG = true;

    // ====================================================================
    // APIs for mtk GnssLocationProvider
    // ====================================================================
    public static class GnssLocationProvider {

        // / mtk added deleting aiding data flags
        private final Context mContext;
        private final Handler mHandler;

        private static final int UPDATE_LOCATION = 7;
        private static final int EVENT_GPS_TIME_SYNC_CHANGED = 4;
        private Handler mGpsHandler;
        private Location mLastLocation;

        public GnssLocationProvider(Context context, Handler handler) {
            if (DEBUG)
                Log.d(TAG, "MtkLocationExt GnssLocationProvider()");
            mContext = context;
            mHandler = handler;

            registerIntentReceiver();

            Log.d(TAG, "add GPS time sync handler and looper");
            mGpsHandler = new MyHandler(mHandler.getLooper());

            mLocationManager = (LocationManager) mContext
                    .getSystemService(mContext.LOCATION_SERVICE);
            mGpsTimeSyncObserver = new GpsTimeSyncObserver(mGpsHandler,
                    EVENT_GPS_TIME_SYNC_CHANGED);
            mGpsTimeSyncObserver.observe(mContext);
        }

        private void launchLPPeService() {
            // start LPPe service to run in system server process
            Intent intent = new Intent();
            intent.setComponent(new ComponentName(
                    "com.mediatek.location.lppe.main",
                    "com.mediatek.location.lppe.main.LPPeServiceWrapper"));
            mContext.startService(intent);
        }

        private void registerIntentReceiver() {
            IntentFilter intentFilter = new IntentFilter();
            intentFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
            mContext.registerReceiverAsUser(new BroadcastReceiver() {
                @Override
                public void onReceive(Context context, Intent intent) {
                    final String action = intent.getAction();
                    if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
                        boolean gpsTimeSyncStatus = getGpsTimeSyncState();
                        Log.d(TAG, "GPS Time sync is set to " + gpsTimeSyncStatus);
                        setGpsTimeSyncFlag(gpsTimeSyncStatus);

                        Log.d(TAG, "Skip luaunch lppe service");
                        ///launchLPPeService();
                    }
                }
            }, UserHandle.ALL, intentFilter, null, mHandler);
        }

        // / M: comment @{ add GPS Time Sync Service
        private class MyHandler extends Handler {

            public MyHandler(Looper l) {
                super(l);
            }

            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                case EVENT_GPS_TIME_SYNC_CHANGED:
                    boolean gpsTimeSyncStatus = getGpsTimeSyncState();
                    ;
                    Log.d(TAG, "GPS Time sync is changed to "
                            + gpsTimeSyncStatus);
                    onGpsTimeChanged(gpsTimeSyncStatus);
                    break;
                // / @}
                }
            }
        }

        /** ================ Gps Time Sync part ================ */

        private Thread mGpsTimerThread; // for interrupt
        private LocationManager mLocationManager;
        private boolean mIsGpsTimeSyncRunning = false;
        private GpsTimeSyncObserver mGpsTimeSyncObserver;

        private boolean getGpsTimeSyncState() {
            try {
                return Settings.Global.getInt(mContext.getContentResolver(), AUTO_TIME_GPS) > 0;
            } catch (SettingNotFoundException snfe) {
                return false;
            }
        }

        private static class GpsTimeSyncObserver extends ContentObserver {

            private int mMsg;
            private Handler mHandler;

            GpsTimeSyncObserver(Handler handler, int msg) {
                super(handler);
                mHandler = handler;
                mMsg = msg;
            }

            void observe(Context context) {
                ContentResolver resolver = context.getContentResolver();
                resolver.registerContentObserver(Settings.Global
                        .getUriFor(AUTO_TIME_GPS), false, this);
            }

            @Override
            public void onChange(boolean selfChange) {
                mHandler.obtainMessage(mMsg).sendToTarget();
            }
        }

        public void onGpsTimeChanged(boolean enable) {
            if (enable) {
                startUsingGpsWithTimeout(180000,
                mContext.getString(com.mediatek.internal.R.string.gps_time_sync_fail_str));
            } else {
                if (mGpsTimerThread != null) {
                    mGpsTimerThread.interrupt();
                }
            }
            setGpsTimeSyncFlag(enable);
        }

        private void setGpsTimeSyncFlag(boolean flag) {
            if (DEBUG)
                Log.d(TAG, "setGpsTimeSyncFlag: " + flag);

            if (flag) {
                mLocationManager.requestLocationUpdates(
                        LocationManager.PASSIVE_PROVIDER, 0, 0,
                        mPassiveLocationListener);
            } else {
                mLocationManager.removeUpdates(mPassiveLocationListener);
            }
        }

        private LocationListener mPassiveLocationListener = new LocationListener() {
            @Override
            public void onLocationChanged(Location location) {
                if (LocationManager.GPS_PROVIDER.equals(location.getProvider())) {
                    boolean hasLatLong = (location.getLatitude() != 0)
                            && (location.getLongitude() != 0);
                    doSystemTimeSyncByGps(hasLatLong, location.getTime());
                }
            }

            @Override
            public void onProviderDisabled(String provider) {
            }

            @Override
            public void onProviderEnabled(String provider) {
            }

            @Override
            public void onStatusChanged(String provider, int status, Bundle extras) {
            }

        };

        private void doSystemTimeSyncByGps(boolean hasLatLong, long timestamp) {
            if (hasLatLong) {
                if (DEBUG)
                    Log.d(TAG,
                            " ########## Auto-sync time with GPS: timestamp = "
                                    + timestamp + " ########## ");
                Calendar c = Calendar.getInstance();
                c.setTimeInMillis(timestamp);
                long when = c.getTimeInMillis();
                if (when / 1000 < Integer.MAX_VALUE) {
                    SystemClock.setCurrentTimeMillis(when);
                }
                mLocationManager.removeUpdates(mPassiveLocationListener);
            }
        }

        public void startUsingGpsWithTimeout(final int milliseconds,
                final String timeoutMsg) {

            if (mIsGpsTimeSyncRunning == true) {
                Log.d(TAG, "WARNING: Gps Time Sync is already run");
                return;
            } else {
                mIsGpsTimeSyncRunning = true;
            }

            Log.d(TAG, "start using GPS for GPS time sync timeout="
                    + milliseconds + " timeoutMsg=" + timeoutMsg);
            mLocationManager.requestLocationUpdates(
                    LocationManager.GPS_PROVIDER, 1000, 0, mLocationListener);
            mGpsTimerThread = new Thread() {
                public void run() {
                    boolean isTimeout = false;
                    try {
                        Thread.sleep(milliseconds);
                        isTimeout = true;
                    } catch (InterruptedException e) {
                    }
                    Log.d(TAG, "isTimeout=" + isTimeout);
                    if (isTimeout == true) {
                        Message m = new Message();
                        m.obj = timeoutMsg;
                        mGpsToastHandler.sendMessage(m);
                    }
                    mLocationManager.removeUpdates(mLocationListener);
                    mIsGpsTimeSyncRunning = false;
                }
            };
            mGpsTimerThread.start();
        }

        private Handler mGpsToastHandler = new Handler() {
            public void handleMessage(Message msg) {
                String timeoutMsg = (String) msg.obj;
                Toast.makeText(mContext, timeoutMsg, Toast.LENGTH_LONG).show();
            }
        };

        private LocationListener mLocationListener = new LocationListener() {
            @Override
            public void onLocationChanged(Location location) {
                mGpsTimerThread.interrupt();
            }

            @Override
            public void onProviderDisabled(String provider) {
            }

            @Override
            public void onProviderEnabled(String provider) {
            }

            @Override
            public void onStatusChanged(String provider, int status,
                    Bundle extras) {
            }
        };
        // / @}

    }

    // ====================================================================
    // APIs for mtk LocationManagerService
    // ====================================================================
    public static class LocationManagerService {
        private final Context mContext;
        private final Handler mHandler;
        private CtaManager mCtaManager;
        private LocationManager mLocationManager;

        public LocationManagerService(Context context, Handler handler) {
            if (DEBUG)
                Log.d(TAG, "MtkLocationExt LocationManagerService()");
            mContext = context;
            mHandler = handler;
            mCtaManager = CtaManagerFactory.getInstance().makeCtaManager();
            mLocationManager = (LocationManager) mContext
                    .getSystemService(mContext.LOCATION_SERVICE);
        }

        public boolean isCtaFeatureSupport() {
            return mCtaManager.isCtaSupported();
        }

         /**
             * CTA required to print the preset application to print information like:
             * Time < application Chinese name >[keyword] [process name]:[function name]
             * to do the operation..parameter, it is used for framework API directly call.
             * @param callingPid
             * @param callingUid
             * @param keyWordType
             * @param functionName
             * @param actionType
             * @param parameter
             */
        public void printCtaLog(
                int callingPid, int callingUid, String functionName,
                String strActionType, String parameter) {

            ActionType actionType = ActionType.USE_LOCATION;
            if ("USE_LOCATION".equals(strActionType)) {
                actionType = ActionType.USE_LOCATION;
            } else if ("READ_LOCATION_INFO".equals(strActionType)) {
                actionType = ActionType.READ_LOCATION_INFO;
            }
            /*
                Log.d(TAG, "MtkLocationExt printCtaLog callingPid = " + callingPid
                    + " callingUid: " + callingUid
                    + " functionName: " + functionName
                    + " strActionType: " + strActionType
                    + " actionType: " + actionType
                    + " parameter: " + parameter);
            */
            mCtaManager.printCtaInfor(callingPid, callingUid, KeywordType.LOCATION, functionName,
                actionType, parameter);
        }

        public void showNlpNotInstalledToast(String provider) {
            try {
                Log.d(TAG, "showNlpNotInstalledToast provider: " + provider);
                if (LocationManager.NETWORK_PROVIDER.equals(provider)) {
                    Toast.makeText(mContext,
                            "No Network Location Provider is installed!" +
                            "NLP is necessary for network location fixes.",
                            Toast.LENGTH_LONG).show();
                }
            } catch (Exception e) {
                Log.w(TAG, "Failed to show toast ", e);
            }
        }
    }
}
