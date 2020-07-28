package com.mediatek.mtklogger.c2klogger;

import android.app.Service;
import android.util.Log;

/**
 * @author MTK81255
 *
 */
public class C2KLoggerProxy {

    private static C2KLogService sC2KLogService;

    /**
     * @param service Service
     */
    public static void startService(Service service) {
        Log.d("saber", "-->>>>>C2KLoggerStart");
        sC2KLogService = new C2KLogService(service);
        sC2KLogService.onCreate();
        sC2KLogService.onStartCommand();
        Log.d("saber", "<<<<<<<<<<---------C2KLoggerStart end!");
    }

    /**
     * @param service Service
     */
    public static void stopService(Service service) {
        Log.d("saber", "-->>>>>stopService");
        sC2KLogService.onDestroy();
        Log.d("saber", "<<<<<<<<<<---------stopService end!");
    }

}
