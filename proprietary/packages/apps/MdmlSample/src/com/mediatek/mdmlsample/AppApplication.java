package com.mediatek.mdmlsample;

import android.app.Application;
import android.util.Log;
import android.content.Context;

import com.mediatek.mdml.MonitorTrapReceiver;
import com.mediatek.mdml.PlainDataDecoder;

/**
 * Created by MTK03685 on 2015/9/14.
 */
public class AppApplication extends Application {
    private static final String TAG = "AppApplication";

    // Trap decoder objects
    private PlainDataDecoder m_plainDataDecoder;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "AppApplication onCreate()");
        try {
            m_plainDataDecoder = PlainDataDecoder.getInstance(null, getApplicationContext());
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }

    synchronized PlainDataDecoder GetPlainDataDecoder() {
        // maybe when first init, MDM server did not run, then m_plainDataDecoder will be null
        if (m_plainDataDecoder == null) {
            m_plainDataDecoder = PlainDataDecoder.getInstance(null, getApplicationContext());
        }
        return m_plainDataDecoder;
    }
}
