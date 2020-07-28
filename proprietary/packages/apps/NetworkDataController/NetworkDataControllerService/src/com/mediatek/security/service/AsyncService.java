/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.security.service;

import android.app.Service;
import android.content.Intent;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;

/**
 * AsyncService is a base class for {@link Service}s that handle asynchronous
 * requests (expressed as {@link Intent}s) on demand. Clients send requests
 * through {@link android.content.Context#startService(Intent)} calls; the
 * service is started as needed, handles each Intent in turn using a worker
 * thread. However the different between IntentService and AsyncService is where
 * to stop. IntentService stop itself when it runs out of work
 */
public abstract class AsyncService extends Service {
    /**
     * All message send from service main thread to asynservice's working thread
     * */
    public final int MSG_INTENT = 1;
    public final int NETWORK_DATA_REQUEST = 2;
    public final int NETWORK_DATA_MODIFY = 3;
    public final int MSG_PACKAGE_CHANGED = 4;
    public final int MSG_SHOW_CONF_DLG = 5;

    private volatile Looper mServiceLooper;
    private volatile ServiceHandler mServiceHandler;
    private String mName;
    private boolean mRedelivery;

    private final class ServiceHandler extends Handler {
        public ServiceHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            onHandleMessage(msg);
        }
    }

    /**
     * Creates an IntentService. Invoked by your subclass's constructor.
     *
     * @param name
     *            Used to name the worker thread, important only for debugging.
     */
    public AsyncService(String name) {
        super();
        mName = name;
    }

    /**
     * Sets intent redelivery preferences. Usually called from the constructor
     * with your preferred semantics.
     *
     * <p>
     * If enabled is true, {@link #onStartCommand(Intent, int, int)} will return
     * {@link Service#START_REDELIVER_INTENT}, so if this process dies before
     * {@link #onHandleIntent(Intent)} returns, the process will be restarted
     * and the intent redelivered. If multiple Intents have been sent, only the
     * most recent one is guaranteed to be redelivered.
     *
     * <p>
     * If enabled is false (the default),
     * {@link #onStartCommand(Intent, int, int)} will return
     * {@link Service#START_NOT_STICKY}, and if the process dies, the Intent
     * dies along with it.
     */
    public void setIntentRedelivery(boolean enabled) {
        mRedelivery = enabled;
    }

    @Override
    public void onCreate() {
        // It would be nice to have an option to hold a partial wakelock
        // during processing, and to have a static startService(Context, Intent)
        // method that would launch the service & hand off a wakelock.

        super.onCreate();
        HandlerThread thread = new HandlerThread("AsyService[" + mName + "]");
        thread.start();

        mServiceLooper = thread.getLooper();
        mServiceHandler = new ServiceHandler(mServiceLooper);
    }

    @Override
    public void onStart(Intent intent, int startId) {
        Message msg = mServiceHandler.obtainMessage(MSG_INTENT);
        msg.arg1 = startId;
        msg.obj = intent;
        mServiceHandler.sendMessage(msg);
    }

    public void handleMessage(int what, Object obj) {
        Message msg = mServiceHandler.obtainMessage(what);
        msg.obj = obj;
        mServiceHandler.sendMessage(msg);
    }

    /**
     * You should not override this method for your IntentService. Instead,
     * override {@link #onHandleIntent}, which the system calls when the
     * IntentService receives a start request.
     *
     * @see android.app.Service#onStartCommand
     */
    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        onStart(intent, startId);
        return mRedelivery ? START_REDELIVER_INTENT : START_STICKY;
    }

    @Override
    public void onDestroy() {
        mServiceLooper.quit();
    }

    /**
     * Unless you provide binding for your service, you don't need to implement
     * this method, because the default implementation returns null.
     *
     * @see android.app.Service#onBind
     */
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    /**
     * This method is invoked on the worker thread with a request to process.
     * Only one Intent is processed at a time, but the processing happens on a
     * worker thread that runs independently from other application logic. So,
     * if this code takes a long time, it will hold up other requests to the
     * same IntentService, but it will not hold up anything else. When all
     * requests have been handled, the IntentService stops itself, so you should
     * not call {@link #stopSelf}.
     *
     * @param intent
     *            The value passed to
     *            {@link android.content.Context#startService(Intent)}.
     */
    protected abstract void onHandleMessage(Message message);
}
