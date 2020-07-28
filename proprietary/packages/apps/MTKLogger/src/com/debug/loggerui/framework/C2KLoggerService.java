package com.debug.loggerui.framework;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import com.debug.loggerui.utils.Utils;

/**
 * @author MTK81255
 *
 */
public class C2KLoggerService extends Service {

    private static final String TAG = Utils.TAG + "/C2KLoggerService";
    public static Service sService;

    @Override
    public void onCreate() {
        sService = this;
        Utils.logi(TAG, "============>  onCreate");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Utils.logi(TAG, "============>  onStartCommand");
        return START_REDELIVER_INTENT;
    }

    @Override
    public void onDestroy() {
        Utils.logi(TAG, "============>  onDestroy");
    }

    @Override
    public IBinder onBind(Intent intent) {
        Utils.logi(TAG, "============>  onBind");
        return null;
    }

}
