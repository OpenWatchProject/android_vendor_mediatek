/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.mediatek.providers.drm;

import static com.mediatek.providers.drm.OmaDrmHelper.DEBUG;
import android.app.job.JobParameters;
import android.app.job.JobService;
import android.app.Notification;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.SntpClient;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.provider.Settings;
import android.util.Log;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.List;

/**
 * Using service to sync time, and start with foreground to avoid low memory kill.
 * 1. sync secure timer
 * 2. update time offset
 */
public class DrmSyncTimeService extends JobService {
    private static final String TAG = "DRM/DrmSyncTimeService";
    // Message which handle by DrmSyncTimeHandler
    private static final int MSG_SYNC_SECURE_TIMER = 1;
    private static final int MSG_UPDATE_TIME_OFFSET = 2;
    public static final int INVALID_OFFSET = 0x7fffffff;
    public static final int SERVER_TIMEOUT = 3000;
    private DrmSyncTimeHandler mDrmSyncTimeHandler = null;
    private Context mContext;
    private DrmManagerClient mDrmManagerClient;
    private List<String> mNtpServers;

    // Sync with these SNTP host servers for different countries.
    private static String[] sHostList = new String[] {
        "pool.ntp.org",
        "2.android.pool.ntp.org",
        "time-a.nist.gov",
        "t1.hshh.org",
        "t2.hshh.org",
        "t3.hshh.org",
        "clock.via.net",
        "asia.pool.ntp.org",
        "europe.pool.ntp.org",
        "north-america.pool.ntp.org",
        "oceania.pool.ntp.org",
        "south-america.pool.ntp.org",
        "hshh.org"
    };

    private JobParameters mSyncTimeParams;
    private JobParameters mUpdateTimeParams;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        mContext = getApplicationContext();
        mDrmManagerClient = new DrmManagerClient(mContext);
        // Set as foreground process to avoid LMK when sync secure timer
        // use 0 does not show notification
        Notification notification = new Notification.Builder(mContext).build();
        startForeground(0, notification);

        // start sub thread to access network
        HandlerThread handlerThread = new HandlerThread("DrmSyncTimeThread");
        handlerThread.start();
        mDrmSyncTimeHandler = new DrmSyncTimeHandler(handlerThread.getLooper());

        // Get default ntp server from system and init sync secure time servers list
        String defaultServer = Settings.Global.getString(mContext.getContentResolver(),
                Settings.Global.NTP_SERVER);
        if (DEBUG) Log.d(TAG, "defaultServer: " + defaultServer);
        mNtpServers = new ArrayList<String>(sHostList.length + 1);
        for (String host : sHostList) {
            mNtpServers.add(host);
            if (host.equals(defaultServer)) {
                defaultServer = null;
            }
        }
      
        if (defaultServer != null) {
            mNtpServers.add(0, defaultServer);
        }
        if (null != mUpdateTimeParams) {
            Log.d(TAG, "onCreate unfinished updateTime JOB FINISH mParams[" + mUpdateTimeParams +
            "] id[" + mUpdateTimeParams.getJobId());
            // jobFinished(mUpdateTimeParams, false);
            mUpdateTimeParams = null;
        }
        if (null != mSyncTimeParams) {
            Log.d(TAG, "onCreate unfinished sync JOB FINISH mParams = " + mSyncTimeParams +
            "] id[" + mSyncTimeParams.getJobId());
            // jobFinished(mSyncTimeParams, false);
            mSyncTimeParams = null;
        }
        mSyncTimeParams = null;
        mUpdateTimeParams = null;
    }

    @Override
        public boolean onStartJob(final JobParameters params) {
        Log.d(TAG, "onStartJob: params = " + params);
         if (params == null) {
            // jobFinished(params, false);
            return false;
        }

        String action = params.getExtras().getString(OmaDrmHelper.KEY_ACTION);
        int startId = params.getJobId();
        int what = -1;
        if (OmaDrmHelper.ACTION_SYNC_SECURE_TIME.equals(action)) {
            Log.d(TAG, "onStartJob: sync secure time parm[" + params + "]id[" + startId);
            what = MSG_SYNC_SECURE_TIMER;
            if (null != mSyncTimeParams) {
                Log.d(TAG, "unfinished sync JOB FINISH mParams = " + mSyncTimeParams +
                "] id[" + mSyncTimeParams.getJobId());
                jobFinished(mSyncTimeParams, false);
            }
            mSyncTimeParams = params;
        } else if (OmaDrmHelper.ACTION_UPDATE_TIME_OFFSET.equals(action)) {
            Log.d(TAG, "onStartJob: update time parm[" + params + "] id[" + startId);
            what = MSG_UPDATE_TIME_OFFSET;
            if (null != mUpdateTimeParams) {
                Log.d(TAG, "unfinished updateTime JOB FINISH mParams = " + mUpdateTimeParams +
                "] id[" + mUpdateTimeParams.getJobId());
                jobFinished(mUpdateTimeParams, false);
            }
            mUpdateTimeParams = params;
        }
        mDrmSyncTimeHandler.removeMessages(what);
        Message msg = mDrmSyncTimeHandler.obtainMessage(what, startId, -1);
        msg.sendToTarget();
        return true;
    }

    @Override
    public boolean onStopJob(JobParameters params) {
        Log.d(TAG, "onStopJob: params = " + params);
        if (null != params) {
            inputID = params.getJobId();
        }
        if ((null != mUpdateTimeParams) && (inputID == mUpdateTimeParams.getJobId())) {
            Log.d(TAG, "onStopJob: updateTime params[" + params + "] id[" + inputID);
            mUpdateTimeParams = null;
        } else if ((null != mSyncTimeParams) && (inputID == mSyncTimeParams.getJobId())) {
            Log.d(TAG, "onStopJob: synctime params[" + params + "] id[" + inputID);
            mSyncTimeParams = null;
        } else {
            if ((null != mSyncTimeParams) && (null != mUpdateTimeParams)) {
            Log.d(TAG, "onStopJob: 1 params[" + params + "] id[" + inputID +
                "]sync[" + mSyncTimeParams + "] syncid[" + mSyncTimeParams.getJobId() +
                "]updt[" + mUpdateTimeParams + "]updtid[" + mUpdateTimeParams.getJobId());
            } else if (null != mSyncTimeParams) {
            Log.d(TAG, "onStopJob: 2 params[" + params + "] id[" + inputID +
                "]sync[" + mSyncTimeParams + "] syncid[" + mSyncTimeParams.getJobId() +
                "]updt[" + mUpdateTimeParams);
            } else if (null != mUpdateTimeParams) {
            Log.d(TAG, "onStopJob: 3 params[" + params + "] id[" + inputID +
                "]sync[" + mSyncTimeParams + "]updt[" + mUpdateTimeParams +
                "]updtid[" + mUpdateTimeParams.getJobId());
            } else {
                Log.d(TAG, "onStopJob: 4 params[" + params + "] id[" + inputID);
            }
        }
        return false;
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "onDestroy");
        // Set process as background and stop handler thread
        if (null != mUpdateTimeParams) {
            Log.d(TAG, "onDestroy unfinished updateTime JOB FINISH mParams[" + mUpdateTimeParams +
                "] id[" + mUpdateTimeParams.getJobId());
            jobFinished(mUpdateTimeParams, false);
            mUpdateTimeParams = null;
        }
        if (null != mSyncTimeParams) {
            Log.d(TAG, "onDestroy unfinished sync JOB FINISH mParams[" + mSyncTimeParams +
                "] id[" + mSyncTimeParams.getJobId());
            jobFinished(mSyncTimeParams, false);
            mSyncTimeParams = null;
        }
        stopForeground(true);
        mDrmSyncTimeHandler.getLooper().quit();
        mDrmSyncTimeHandler = null;
        mContext = null;
        mDrmManagerClient.close();
        super.onDestroy();
    }


    /**
     * Use handler thread to access network to avoid blocking main thread.
     */
    private class DrmSyncTimeHandler extends Handler {

        DrmSyncTimeHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_SYNC_SECURE_TIMER:
                    syncSecureTime();
                    break;

                case MSG_UPDATE_TIME_OFFSET:
                    updateTimeOffset();
                    break;

                default:
                    break;
            }
            // stop with start id of you self, so that service will be stopped only when
            // all job finish
            //if (DEBUG) Log.d(TAG, "Stop DrmSyncTimeService with startId = " + msg.arg1);
            //stopSelf(msg.arg1);
            Log.d(TAG, "Exit handleMessage");
        }
    }

    private void updateTimeOffset() {
        OmaDrmHelper.updateOffset(mDrmManagerClient);
        Log.d(TAG, "JOB FINISHED from updateTimeOffset mParams[" + mUpdateTimeParams +
            "] id[" + ((null != mUpdateTimeParams) ? mUpdateTimeParams.getJobId() : null));
        if (null != mUpdateTimeParams) jobFinished(mUpdateTimeParams, false);
        mUpdateTimeParams = null;
    }

    private void syncSecureTime() {
        if (null == mSyncTimeParams) {
            Log.d(TAG, "JOB prematurely finished from syncSecureTime");
            return;
        }
        // If test on CT case, disable to sync secure timer to avoid send ntp package
        if (OmaDrmHelper.isRequestDisableSyncSecureTime()) {
            if (DEBUG) Log.d(TAG, "workaround for OP case, disable sync secure time");
            Log.d(TAG, "JOB FINISHED from syncSecureTime 1 mParams[" + mSyncTimeParams +
                "] id[" + ((null != mSyncTimeParams) ? mSyncTimeParams.getJobId() : null));
            if (null != mSyncTimeParams) jobFinished(mSyncTimeParams, false);
            mSyncTimeParams = null;
            return;
        }

        for (String serverName: mNtpServers) {
            // 1. If current server not available, check next server
            if (null == mSyncTimeParams) {
                Log.d(TAG, "JOB prematurely finished 2 from syncSecureTime");
                return;
            }

            if (!isServerAvailable(serverName)) {
                Log.d(TAG, "syncSecureTime with server not available for " + serverName);
                continue;
            }

            ConnectivityManager cm =
                mContext == null ? null : mContext.getSystemService(ConnectivityManager.class);
            Network nt = cm == null ? null : cm.getActiveNetwork();
            if (nt == null) {
                Log.d(TAG, "syncSecureTime failed with server " + serverName + "network is null");
                continue;
            }
            // 2. If sync offset is invalid, check next server
            SntpClient sntpClient = new SntpClient();
            Long sntpTime = (long) INVALID_OFFSET;
            if (!sntpClient.requestTime(serverName, SERVER_TIMEOUT, nt)) {
                Log.d(TAG, "syncSecureTime failed with server " + serverName);
                continue;
            }
            sntpTime = (long) (sntpClient.getNtpTime() + SystemClock.elapsedRealtime()
                    - sntpClient.getNtpTimeReference());

            int offset = (int) ((sntpTime - System.currentTimeMillis()) / 1000);
            Log.d(TAG, "syncSecureTime: offset = " + offset);

            // 3. If secure time is not valid after update clock with offset, check next server
            if (!OmaDrmHelper.saveSecureTime(mDrmManagerClient, offset)) {
                Log.d(TAG, "syncSecureTime save failed with " + serverName);
                continue;
            }

            // Success sync secure time, break loop
            Log.d(TAG, "syncSecureTime success with host server " + serverName);
            break;
        }
         Log.d(TAG, "JOB FINISHED from syncSecureTime 2 mParams[" + mSyncTimeParams +
            "] id[" + ((null != mSyncTimeParams) ? mSyncTimeParams.getJobId() : null));
         if (null != mSyncTimeParams) jobFinished(mSyncTimeParams, false);
         mSyncTimeParams = null;
    }

    private boolean isServerAvailable(String serverName) {
        InetAddress addr = null;
        try {
            addr = InetAddress.getByName(serverName);
        } catch (UnknownHostException e) {
            Log.w(TAG, "isServerAvailable with " + e);
        }
        if (DEBUG) Log.d(TAG, "isServerAvailable:serverName = " + serverName + ", addr = " + addr);
        return addr != null;
    }
}
