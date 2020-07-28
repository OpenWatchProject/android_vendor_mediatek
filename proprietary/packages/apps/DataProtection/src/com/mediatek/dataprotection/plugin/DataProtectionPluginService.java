package com.mediatek.dataprotection.plugin;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class DataProtectionPluginService extends Service {
    @Override
    public IBinder onBind(Intent arg0) {
        return null;
    }
}