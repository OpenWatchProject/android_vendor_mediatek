package com.mediatek.common.multiwindow;

import android.app.PendingIntent;
import android.os.IBinder;
import android.os.WorkSource;
import android.content.ComponentName;


/**
 * System private API for talking with the alarm manager service.
 *
 * {@hide}
 */
interface IMWSystemUiCallback {
  ///for System 
  void showRestoreButton(in boolean flag);     
}


